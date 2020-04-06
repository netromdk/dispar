#include "widgets/OmniSearchDialog.h"
#include "Util.h"
#include "cxx.h"
#include "widgets/BinaryWidget.h"
#include "widgets/LineEdit.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QHeaderView>
#include <QListWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cassert>
#include <future>
#include <thread>
#include <vector>

namespace dispar {

OmniSearchDialog::OmniSearchDialog(QWidget *parent) : QDialog(parent, Qt::Popup)
{
  setWindowModality(Qt::ApplicationModal);

  searchTimer.setSingleShot(true);
  searchTimer.setInterval(500);
  connect(&searchTimer, &QTimer::timeout, this, &OmniSearchDialog::search);

  setupLayout();
}

OmniSearchDialog::~OmniSearchDialog()
{
}

void OmniSearchDialog::setBinaryWidget(BinaryWidget *widget)
{
  assert(widget);
  binaryWidget = widget;
}

void OmniSearchDialog::done(int result)
{
  inputEdit->clearFocus();
  inputEdit->releaseKeyboard();

  QDialog::done(result);
}

void OmniSearchDialog::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);

  Util::centerWidget(this);
  inputEdit->setFocus();

  // Keep keyboard focus in input field even though focus might be put in the candidates list.
  inputEdit->grabKeyboard();
}

void OmniSearchDialog::inputEdited(const QString &text)
{
  input = text;

  // Delay search if inputting in quick succession.
  searchTimer.start();
}

void OmniSearchDialog::inputKeyDown()
{
  navigateCandidates(Navigation::DOWN);
}

void OmniSearchDialog::inputKeyUp()
{
  navigateCandidates(Navigation::UP);
}

void OmniSearchDialog::inputReturnPressed()
{
  const auto selected = candidatesWidget->selectedItems();
  if (selected.isEmpty()) {
    return;
  }

  const auto *item = selected.first();
  const auto type = EntryType(item->data(1, Qt::UserRole).toInt());
  void *data = item->data(0, Qt::UserRole).value<void *>();

  switch (type) {
  case EntryType::SECTION: {
    const auto *section = static_cast<Section *>(data);
    binaryWidget->selectSection(section);
    break;
  }

  case EntryType::SYMBOL: {
    auto *listItem = static_cast<QListWidgetItem *>(data);
    auto *list = binaryWidget->symbolList_;

    // Select nothing first to ensure the seleciton is changed even though it's already on that
    // item, such that it changes to the relevant tab widget.
    list->setCurrentItem(nullptr);
    list->setCurrentItem(listItem);
    break;
  }

  case EntryType::STRING: {
    auto *listItem = static_cast<QListWidgetItem *>(data);
    auto *list = binaryWidget->stringList_;
    list->setCurrentItem(nullptr);
    list->setCurrentItem(listItem);
    break;
  }

  case EntryType::TAG: {
    auto *listItem = static_cast<QListWidgetItem *>(data);
    auto *list = binaryWidget->tagList_;
    list->setCurrentItem(nullptr);
    list->setCurrentItem(listItem);
    break;
  }
  }
}

void OmniSearchDialog::setupLayout()
{
  inputEdit = new LineEdit;
  inputEdit->setPlaceholderText(tr("Omni regex search.."));
  inputEdit->setMinimumWidth(500);

  connect(inputEdit, &QLineEdit::textEdited, this, &OmniSearchDialog::inputEdited);
  connect(inputEdit, &LineEdit::keyDown, this, &OmniSearchDialog::inputKeyDown);
  connect(inputEdit, &LineEdit::keyUp, this, &OmniSearchDialog::inputKeyUp);
  connect(inputEdit, &QLineEdit::returnPressed, this, &OmniSearchDialog::inputReturnPressed);

  candidatesWidget = new QTreeWidget;
  candidatesWidget->setMinimumHeight(200);
  candidatesWidget->setHeaderLabels({tr("Match"), tr("Type"), tr("Similarity")});
  candidatesWidget->sortItems(2, Qt::DescendingOrder); // High similarity at the top.

  auto *header = candidatesWidget->header();
  header->resizeSection(0, 300);
  header->resizeSection(1, 100);

  auto *layout = new QVBoxLayout;
  layout->addWidget(inputEdit);
  layout->addWidget(candidatesWidget);

  setLayout(layout);
}

void OmniSearchDialog::search()
{
  if (input.isEmpty()) {
    candidatesWidget->clear();
    return;
  }

  regex.setPattern(input);
  input.clear();

  regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

  if (!regex.isValid()) {
    candidatesWidget->clear();
    return;
  }

  const auto *object = binaryWidget->object_;
  assert(object);

  candidatesWidget->clear();
  candidatesWidget->setUpdatesEnabled(false);
  candidatesWidget->setSortingEnabled(false);

  std::vector<std::future<QList<QTreeWidgetItem *>>> futures;
  QList<QTreeWidgetItem *> items;

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  futures.emplace_back(
    std::async(std::launch::async, &OmniSearchDialog::flexMatchSections, this, object->sections()));
  futures.emplace_back(std::async(std::launch::async, &OmniSearchDialog::flexMatchList, this,
                                  binaryWidget->symbolList_, EntryType::SYMBOL));
  futures.emplace_back(std::async(std::launch::async, &OmniSearchDialog::flexMatchList, this,
                                  binaryWidget->stringList_, EntryType::STRING));
  futures.emplace_back(std::async(std::launch::async, &OmniSearchDialog::flexMatchList, this,
                                  binaryWidget->tagList_, EntryType::TAG));

  for (auto &future : futures) {
    future.wait();
    items += future.get();
  }

  qDebug() << "Searched in" << elapsedTimer.restart() << "ms";

  // Pre-sorting speeds up insertion+sorting inside the QTreeWidget, and it is important when
  // throwing away candiates that are least similar.
  cxx::sort(items, [](const auto *a, const auto *b) {
    return a->text(2).toFloat() > b->text(2).toFloat();
  });

  // Only use a subset of most similar candidates.
  const auto totalItems = items.size();
  static constexpr int limit = 1000; // TODO: maybe this number goes into preferences UI?
  const auto throwAway = items.mid(limit);
  items = items.mid(0, limit);

  candidatesWidget->addTopLevelItems(items);
  candidatesWidget->setSortingEnabled(true);
  candidatesWidget->setUpdatesEnabled(true);

  // Delete excess items after UI has been updated as slight perceived speedup.
  qDeleteAll(throwAway);

  // Select first candidate, if any.
  inputKeyDown();
}

float OmniSearchDialog::flexMatch(const QString &haystack) const
{
  if (!regex.isValid()) {
    return 0.0f;
  }

  // White space hay stack is ignored.
  const auto lh = haystack.trimmed().toLower();
  if (lh.trimmed().isEmpty()) {
    return 0.0f;
  }

  if (const auto m = regex.match(lh); m.hasMatch()) {
    return float(m.captured().size()) / float(lh.size());
  }

  // Check if individual characters of needle matching starting letters of words. Underscores are
  // turned into spaces in order to match words from "LC_VERSION_MIN_MACOSX", for instance. It tries
  // to match needle letters to word 1,2,3.., then 2,3.. etc. such that "lvm" matches
  // "[L]C_[V]ERSION_[M]IN_MACOSX" but so does "vmm" for "LC_[V]ERSION_[M]IN_[M]ACOSX".
  const auto ln = regex.pattern().trimmed().simplified().toLower();
  static const QRegularExpression spaceRegex("\\s+");
  const auto words = QString(lh).replace("_", " ").split(spaceRegex, QString::SkipEmptyParts);
  if (ln.size() <= words.size()) {
    for (int i = 0; i < words.size() && !words.isEmpty(); ++i) {
      int wordMatches = 0;
      for (int j = 0; j < ln.size() && j + i < words.size(); ++j) {
        if (ln[j] == words[j + i][0]) {
          wordMatches++;
        }
      }
      if (wordMatches == ln.size()) {
        return float(wordMatches) / float(lh.size());
      }
    }
  }

  return 0.0f;
}

QList<QTreeWidgetItem *> OmniSearchDialog::flexMatchSections(const QList<Section *> &sections) const
{
  QList<QTreeWidgetItem *> items;
  for (const auto *section : sections) {
    const float sim = flexMatch(section->name()),
                sim2 = flexMatch(Section::typeName(section->type()));
    if (sim > 0.0f || sim2 > 0.0f) {
      items << createCandidate(section->toString(), EntryType::SECTION, std::max(sim, sim2),
                               QVariant::fromValue((void *) section));
    }
  }
  return items;
}

QList<QTreeWidgetItem *> OmniSearchDialog::flexMatchList(const QListWidget *list,
                                                         const EntryType type) const
{
  QList<QTreeWidgetItem *> items;
  for (int row = 0; row < list->count(); ++row) {
    const auto *item = list->item(row);
    if (!item) continue;
    auto itemText = item->text().trimmed();
    if (type == EntryType::SYMBOL && itemText.endsWith(" *")) {
      itemText.chop(2);
    }
    if (const float sim = flexMatch(itemText); sim > 0.0f) {
      items << createCandidate(itemText, type, sim, QVariant::fromValue((void *) item));
    }
  }
  return items;
}

QTreeWidgetItem *OmniSearchDialog::createCandidate(const QString &text, const EntryType type,
                                                   const float similarity,
                                                   const QVariant data) const
{
  const auto typeString = [&type]() -> QString {
    switch (type) {
    case EntryType::SECTION:
      return tr("Section");
    case EntryType::SYMBOL:
      return tr("Symbol");
    case EntryType::STRING:
      return tr("String");
    case EntryType::TAG:
      return tr("Tag");
    }
    return {};
  }();

  auto *item = new QTreeWidgetItem({text, typeString, QString::number(double(similarity))});
  item->setData(0, Qt::UserRole, data);
  item->setData(1, Qt::UserRole, int(type));
  return item;
}

void OmniSearchDialog::navigateCandidates(const Navigation nav)
{
  if (candidatesWidget->topLevelItemCount() == 0) {
    return;
  }
  auto *item = candidatesWidget->currentItem();
  if (item == nullptr) {
    item = candidatesWidget->topLevelItem(0);
  }
  else {
    switch (nav) {
    case Navigation::DOWN:
      item = candidatesWidget->itemBelow(item);
      break;
    case Navigation::UP:
      item = candidatesWidget->itemAbove(item);
      break;
    }
  }
  candidatesWidget->setCurrentItem(item);
}

} // namespace dispar

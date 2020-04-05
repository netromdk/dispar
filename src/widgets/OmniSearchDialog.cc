#include "widgets/OmniSearchDialog.h"
#include "Util.h"
#include "widgets/BinaryWidget.h"
#include "widgets/LineEdit.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QHeaderView>
#include <QListWidget>
#include <QRegularExpression>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cassert>

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

void OmniSearchDialog::showEvent(QShowEvent *event)
{
  Util::centerWidget(this);
  inputEdit->setFocus();
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
  inputEdit->setPlaceholderText(tr("Omni search.."));
  inputEdit->setMinimumWidth(500);

  // Keep keyboard focus in input field even though focus might be put in the candidates list.
  inputEdit->grabKeyboard();

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

  const auto *object = binaryWidget->object_;
  assert(object);

  candidatesWidget->clear();
  candidatesWidget->setSortingEnabled(false);

  QList<QTreeWidgetItem *> items;

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  for (const auto *section : object->sections()) {
    const float sim = flexMatch(section->name(), input),
                sim2 = flexMatch(Section::typeName(section->type()), input);
    if (sim > 0.0f || sim2 > 0.0f) {
      items << createCandidate(section->toString(), EntryType::SECTION, std::max(sim, sim2),
                               QVariant::fromValue((void *) section));
    }
  }

  // TODO: reuse the QListWidget searching for these three widgets?!
  const auto *symbolList = binaryWidget->symbolList_;
  for (int row = 0; row < symbolList->count(); ++row) {
    const auto *item = symbolList->item(row);
    if (!item) continue;
    auto itemText = item->text();
    if (itemText.endsWith(" *")) {
      itemText.chop(2);
    }
    if (const float sim = flexMatch(itemText, input); sim > 0.0f) {
      items << createCandidate(itemText, EntryType::SYMBOL, sim,
                               QVariant::fromValue((void *) item));
    }
  }
  const auto *stringList = binaryWidget->stringList_;
  for (int row = 0; row < stringList->count(); ++row) {
    const auto *item = stringList->item(row);
    if (!item) continue;
    if (const float sim = flexMatch(item->text(), input); sim > 0.0f) {
      items << createCandidate(item->text(), EntryType::STRING, sim,
                               QVariant::fromValue((void *) item));
    }
  }
  const auto *tagList = binaryWidget->tagList_;
  for (int row = 0; row < tagList->count(); ++row) {
    const auto *item = tagList->item(row);
    if (!item) continue;
    if (const float sim = flexMatch(item->text(), input); sim > 0.0f) {
      items << createCandidate(item->text(), EntryType::TAG, sim,
                               QVariant::fromValue((void *) item));
    }
  }

  qDebug() << "Searched in" << elapsedTimer.restart() << "ms";

  candidatesWidget->addTopLevelItems(items);
  candidatesWidget->setSortingEnabled(true);

  // Select first candidate, if any.
  inputKeyDown();

  input.clear();
}

float OmniSearchDialog::flexMatch(const QString &haystack, const QString &needle)
{
  // TODO: keep QSet of <haystack, needle> on successful matches so result is known immediately,
  // after one search match; writing the same again will query dictionary.

  const auto lh = haystack.toLower();
  const auto ln = needle.toLower();

  // White space hay stack is ignored.
  if (lh.trimmed().isEmpty()) {
    return 0.0f;
  }

  // Check if contained.
  if (lh.contains(ln)) {
    return float(needle.size()) / float(haystack.size());
  }

  // Check if individual characters of needle matching starting letters of words. Underscores are
  // turned into spaces in order to match words from "LC_VERSION_MIN_MACOSX", for instance. It tries
  // to match needle letters to word 1,2,3.., then 2,3.. etc. such that "lvm" matches
  // "[L]C_[V]ERSION_[M]IN_MACOSX" but so does "vmm" for "LC_[V]ERSION_[M]IN_[M]ACOSX".
  const auto words =
    QString(lh).replace("_", " ").split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
  for (int i = 0; i < words.size() && !words.isEmpty(); ++i) {
    int wordMatches = 0;
    for (int j = 0; j < ln.size() && j + i < words.size(); ++j) {
      if (ln[j] == words[j + i][0]) {
        wordMatches++;
      }
    }
    if (wordMatches > 1) {
      return float(wordMatches) / float(haystack.size());
    }
  }

  // TODO: check as regular expression input?

  return 0.0f;
}

QTreeWidgetItem *OmniSearchDialog::createCandidate(const QString &text, const EntryType type,
                                                   const float similarity, const QVariant data)
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

#include "widgets/OmniSearchDialog.h"
#include "Context.h"
#include "Util.h"
#include "cxx.h"
#include "widgets/BinaryWidget.h"
#include "widgets/LineEdit.h"

#include <QThread>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDebug>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QThread>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cassert>
#include <future>
#include <thread>
#include <vector>

namespace dispar {

OmniSearchItem::OmniSearchItem(const QStringList &values) : QTreeWidgetItem(values)
{
}

bool OmniSearchItem::operator<(const QTreeWidgetItem &rhs) const
{
  const auto col = treeWidget()->sortColumn();

  // Similarity.
  if (col == 2) {
    return text(col).toFloat() < rhs.text(col).toFloat();
  }

  return QTreeWidgetItem::operator<(rhs);
}

OmniSearchDialog::OmniSearchDialog(QWidget *parent) : QDialog(parent)
{
  setWindowTitle(tr("Omni Search"));
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
  Context::get().setValue("OmniSearchDialog.geometry", Util::byteArrayString(saveGeometry()));

  inputEdit->clearFocus();
  inputEdit->releaseKeyboard();

  QDialog::done(result);
}

void OmniSearchDialog::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);

  if (!restoreGeometry(
        Util::byteArray(Context::get().value("OmniSearchDialog.geometry").toString()))) {
    Util::centerWidget(this);
  }

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

void OmniSearchDialog::inputKeyCtrlNumber(int num)
{
  auto *item = candidatesWidget->topLevelItem(num - 1);
  if (!item) return;
  candidatesWidget->setCurrentItem(item);
  activateCurrentItem();
}

void OmniSearchDialog::setupLayout()
{
  inputEdit = new LineEdit;
  inputEdit->setPlaceholderText(tr("Regex search pattern.."));
  inputEdit->setMinimumWidth(500);

  connect(inputEdit, &QLineEdit::textEdited, this, &OmniSearchDialog::inputEdited);
  connect(inputEdit, &LineEdit::keyDown, this, &OmniSearchDialog::inputKeyDown);
  connect(inputEdit, &LineEdit::keyUp, this, &OmniSearchDialog::inputKeyUp);
  connect(inputEdit, &LineEdit::keyCtrlNumber, this, &OmniSearchDialog::inputKeyCtrlNumber);
  connect(inputEdit, &QLineEdit::returnPressed, this, &OmniSearchDialog::activateCurrentItem);

  candidatesWidget = new QTreeWidget;
  candidatesWidget->setMinimumHeight(200);
  candidatesWidget->setIndentation(0);
  candidatesWidget->setHeaderLabels({tr("Match"), tr("Type"), tr("Similarity")});
  candidatesWidget->sortItems(2, Qt::DescendingOrder); // High similarity at the top.

  auto *header = candidatesWidget->header();
  header->resizeSection(0, 300);
  header->resizeSection(1, 100);
  header->resizeSection(2, 80);

  connect(candidatesWidget, &QTreeWidget::itemDoubleClicked, this,
          &OmniSearchDialog::activateCurrentItem);

  candidatesWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(candidatesWidget, &QTreeWidget::customContextMenuRequested, this,
          &OmniSearchDialog::candidateContextMenu);

  statusLabel = new QLabel;

  searchTextChk = new QCheckBox(tr("Search binary text"));
  searchTextChk->setToolTip(tr("This can be a slow operation."));
  connect(searchTextChk, &QCheckBox::stateChanged, this, &OmniSearchDialog::search);

  auto *bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(statusLabel);
  bottomLayout->addStretch();
  bottomLayout->addWidget(searchTextChk);

  auto *layout = new QVBoxLayout;
  layout->addWidget(inputEdit);
  layout->addWidget(candidatesWidget);
  layout->addLayout(bottomLayout);

  setLayout(layout);
}

void OmniSearchDialog::search()
{
  if (input.isEmpty()) {
    candidatesWidget->clear();
    statusLabel->clear();
    return;
  }

  regex.setPattern(input);
  regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
  regex.optimize();

  if (!regex.isValid()) {
    candidatesWidget->clear();
    statusLabel->setText(tr("Invalid regex") + ": " + regex.errorString());
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

  const int threads = QThread::idealThreadCount();
  const int chunkSize = threads * 2048;
  for (const auto listPair :
       QList<QPair<QListWidget *, EntryType>>{{binaryWidget->symbolList_, EntryType::SYMBOL},
                                              {binaryWidget->stringList_, EntryType::STRING},
                                              {binaryWidget->tagList_, EntryType::TAG}}) {
    const auto *list = listPair.first;
    const auto type = listPair.second;
    for (int row = 0, elms = list->count(); row < elms; row += chunkSize) {
      futures.emplace_back(std::async(std::launch::async, &OmniSearchDialog::flexMatchListRows,
                                      this, list, row, chunkSize, type));
    }
  }

  if (searchTextChk->isChecked()) {
    items += flexMatchText();
  }

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
  const int limit = Context::get().omniSearchLimit();
  const auto throwAway = items.mid(limit);
  items = items.mid(0, limit);

  candidatesWidget->addTopLevelItems(items);
  candidatesWidget->setSortingEnabled(true);
  candidatesWidget->setUpdatesEnabled(true);

  if (candidatesWidget->topLevelItemCount() == 0) {
    statusLabel->setText(tr("No results found.."));
  }
  else if (items.size() == totalItems) {
    statusLabel->setText(tr("%1 results found.").arg(items.size()));
  }
  else {
    statusLabel->setText(tr("Showing %1 of %2 results.").arg(items.size()).arg(totalItems));
  }

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

  if (const auto m = regex.match(haystack); m.hasMatch()) {
    return float(m.captured().size()) / float(haystack.size());
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

QList<QTreeWidgetItem *> OmniSearchDialog::flexMatchListRows(const QListWidget *list,
                                                             const int startRow, const int amount,
                                                             const EntryType type) const
{
  QList<QTreeWidgetItem *> items;
  for (int row = startRow, count = list->count(); row < startRow + amount && row < count; ++row) {
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

QList<QTreeWidgetItem *> OmniSearchDialog::flexMatchText() const
{
  auto *view = binaryWidget->mainView;
  view->setUpdatesEnabled(false);

  const auto oldCursor = view->textCursor();
  view->moveCursor(QTextCursor::Start);

  QList<QTreeWidgetItem *> items;

  const int contextChars = 5;

  static const QRegularExpression spacesBefore("^\\s+");
  static const QRegularExpression spacesAfter("\\s+$");

  // find() can only be run in GUI thread?!
  while (regex.pattern().size() >= 3 && view->find(regex)) {
    const auto cursor = view->textCursor();
    const int endCol = cursor.columnNumber(); // This is the end column!
    const int col = endCol - cursor.selectedText().size();

    auto textBefore = cursor.block().text().mid(col - contextChars, contextChars);
    textBefore.remove(spacesBefore);

    auto textAfter = cursor.block().text().mid(endCol, contextChars);
    textAfter.remove(spacesAfter);

    const auto text = QString("%1[%2]%3     (line %4, col %5)")
                        .arg(textBefore)
                        .arg(cursor.selectedText())
                        .arg(textAfter)
                        .arg(cursor.block().firstLineNumber())
                        .arg(col);

    items << createCandidate(text, EntryType::TEXT,
                             float(cursor.selectedText().size()) /
                               float(cursor.block().text().trimmed().size()),
                             QVariant::fromValue(cursor.blockNumber()));
  }

  view->setTextCursor(oldCursor);
  view->setUpdatesEnabled(true);
  return items;
}

QTreeWidgetItem *OmniSearchDialog::createCandidate(const QString &text, const EntryType type,
                                                   const float similarity,
                                                   const QVariant data) const
{
  // Spaces are only removed on left and right, not internally.
  static const QRegularExpression whiteSpace("[\\n\\r\\t\\v]");
  const auto title = text.trimmed().remove(whiteSpace);

  auto *item = new OmniSearchItem(
    {title, entryTypeString(type), QString::number(double(similarity) * 100.0, 'f', 1)});
  item->setData(0, Qt::UserRole, data);
  item->setData(0, Qt::UserRole + 1, text);
  item->setToolTip(0, text);
  item->setData(1, Qt::UserRole, int(type));
  item->setTextAlignment(2, Qt::AlignRight);
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

void OmniSearchDialog::activateItem(const QTreeWidgetItem *item)
{
  const auto type = EntryType(item->data(1, Qt::UserRole).toInt());
  const auto data = item->data(0, Qt::UserRole);

  switch (type) {
  case EntryType::SECTION: {
    const auto *section = static_cast<Section *>(data.value<void *>());
    binaryWidget->selectSection(section);
    break;
  }

  case EntryType::SYMBOL: {
    auto *listItem = static_cast<QListWidgetItem *>(data.value<void *>());
    auto *list = binaryWidget->symbolList_;

    // Select nothing first to ensure the seleciton is changed even though it's already on that
    // item, such that it changes to the relevant tab widget.
    list->setCurrentItem(nullptr);
    list->setCurrentItem(listItem);
    break;
  }

  case EntryType::STRING: {
    auto *listItem = static_cast<QListWidgetItem *>(data.value<void *>());
    auto *list = binaryWidget->stringList_;
    list->setCurrentItem(nullptr);
    list->setCurrentItem(listItem);
    break;
  }

  case EntryType::TAG: {
    auto *listItem = static_cast<QListWidgetItem *>(data.value<void *>());
    auto *list = binaryWidget->tagList_;
    list->setCurrentItem(nullptr);
    list->setCurrentItem(listItem);
    break;
  }

  case EntryType::TEXT: {
    bool ok;
    const auto blockNumber = data.toInt(&ok);
    if (ok) {
      binaryWidget->selectBlock(blockNumber);
    }
    break;
  }
  }
}

void OmniSearchDialog::activateCurrentItem()
{
  const auto *item = candidatesWidget->currentItem();
  if (!item) return;

  // Delay activation until after scope such that dialog is closed before activation, which ensures
  // the blue activation line is shown in the binary view.
  QTimer::singleShot(1, [this, item] { activateItem(item); });

  close();
}

void OmniSearchDialog::candidateContextMenu(const QPoint &pos)
{
  const auto *selectedItem = candidatesWidget->currentItem();

  // Currently no other actions apply, so don't show context menu if no item is selected.
  if (!selectedItem) return;

  QMenu menu;

  if (selectedItem) {
    menu.addAction(tr("Jump to match"), this, &OmniSearchDialog::activateCurrentItem);
    menu.addAction(tr("Copy matched text"), this, &OmniSearchDialog::copyCurrentText);
  }

  menu.exec(QCursor::pos());
}

void OmniSearchDialog::copyCurrentText()
{
  const auto *selectedItem = candidatesWidget->currentItem();
  if (!selectedItem) return;

  const auto fullText = selectedItem->data(0, Qt::UserRole + 1).toString();
  QApplication::clipboard()->setText(fullText);
}

QString OmniSearchDialog::entryTypeString(const EntryType type)
{
  switch (type) {
  case EntryType::SECTION:
    return tr("Section");
  case EntryType::SYMBOL:
    return tr("Symbol");
  case EntryType::STRING:
    return tr("String");
  case EntryType::TAG:
    return tr("Tag");
  case EntryType::TEXT:
    return tr("Text");
  }
  return {};
}

} // namespace dispar

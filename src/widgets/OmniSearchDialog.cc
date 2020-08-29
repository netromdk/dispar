#include "widgets/OmniSearchDialog.h"
#include "Context.h"
#include "Util.h"
#include "cxx.h"
#include "widgets/BinaryWidget.h"
#include "widgets/LineEdit.h"

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

OmniSearchDialog::~OmniSearchDialog() = default;

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
  if (item == nullptr) return;
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
    const auto plainText = binaryWidget->mainView->toPlainText();
    const int size = plainText.size();

    // Create half as many chunks to search that there are threads.
    const int textChunkSize = size / std::max(threads / 2, 1);

    int offset = 0;
    while (offset < size) {
      auto textChunk = plainText.mid(offset, textChunkSize);

      // Include until next newline such that line boundaries aren't broken in seaches.
      const auto end = offset + textChunkSize;
      const auto nextNewline = plainText.indexOf('\n', end);
      if (nextNewline != -1) {
        textChunk.append(plainText.mid(end, nextNewline - end + 1));
      }

      futures.emplace_back(std::async(std::launch::async, &OmniSearchDialog::flexMatchTextOffset,
                                      this, textChunk, offset));
      offset += textChunk.size();
    }
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
    return 0.0F;
  }

  if (const auto m = regex.match(haystack); m.hasMatch()) {
    return float(m.captured().size()) / float(haystack.size());
  }

  return 0.0F;
}

QList<QTreeWidgetItem *> OmniSearchDialog::flexMatchSections(const QList<Section *> &sections) const
{
  QList<QTreeWidgetItem *> items;
  for (const auto *section : sections) {
    const float sim = flexMatch(section->name()),
                sim2 = flexMatch(Section::typeName(section->type()));
    if (sim > 0.0F || sim2 > 0.0F) {
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
    if (item == nullptr) continue;
    auto itemText = item->text().trimmed();
    if (type == EntryType::SYMBOL && itemText.endsWith(" *")) {
      itemText.chop(2);
    }
    if (const float sim = flexMatch(itemText); sim > 0.0F) {
      items << createCandidate(itemText, type, sim, QVariant::fromValue((void *) item));
    }
  }

  return items;
}

QList<QTreeWidgetItem *> OmniSearchDialog::flexMatchTextOffset(const QString &text,
                                                               const int offset) const
{
  QList<QTreeWidgetItem *> items;

  auto matchs = regex.globalMatch(text);
  while (matchs.hasNext()) {
    const auto m = matchs.next();
    if (!m.hasMatch()) continue;

    const auto start = m.capturedStart(), end = m.capturedEnd(), len = m.capturedLength();

    // Find previous newline or start of text.
    int previousNewline = start - 1;
    while (previousNewline > 0) {
      if (text[previousNewline] == '\n') {
        previousNewline++;
        break;
      }
      previousNewline--;
    }

    // Find next newline or end of text.
    auto nextNewline = text.indexOf('\n', end);
    if (nextNewline == -1) {
      nextNewline = text.size() - 1;
    }

    const auto lineLength = nextNewline - previousNewline;
    const auto line = text.mid(previousNewline, lineLength);

    // Show search context with up to 10 characters on each side, but stopping before previous
    // newline and before next newline.
    static const int contextChars = 10;
    const auto textCtxStart = std::max(start - contextChars, previousNewline);
    const auto textCtxLen = std::min(len + contextChars * 2, nextNewline - textCtxStart);
    auto textContext = text.mid(textCtxStart, textCtxLen);

    // Show in search context whether the start/end of line was reached or whether more content is
    // available.
    if (textCtxStart != previousNewline) {
      textContext.prepend("[..] ");
    }
    if (textCtxLen != nextNewline - textCtxStart) {
      textContext.append(" [..]");
    }

    const auto sim = float(m.capturedLength()) / float(lineLength);
    items << createCandidate(textContext, EntryType::TEXT, sim, QVariant::fromValue(start + offset),
                             line);
  }

  return items;
}

QTreeWidgetItem *OmniSearchDialog::createCandidate(const QString &text, const EntryType type,
                                                   const float similarity, const QVariant &data,
                                                   const QString &fullText)
{
  // Spaces are only removed on left and right, not internally.
  static const QRegularExpression whiteSpace(R"([\n\r\t\v])");
  const auto title = text.trimmed().remove(whiteSpace);

  const QString &line = fullText.isEmpty() ? text : fullText;

  auto *item = new OmniSearchItem(
    {title, entryTypeString(type), QString::number(double(similarity) * 100.0, 'f', 1)});
  item->setData(0, Qt::UserRole, data);
  item->setData(0, Qt::UserRole + 1, line);
  item->setToolTip(0, line);
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
    bool ok = false;
    const auto pos = data.toInt(&ok);
    if (ok) {
      binaryWidget->selectPosition(pos);
    }
    break;
  }
  }
}

void OmniSearchDialog::activateCurrentItem()
{
  const auto *item = candidatesWidget->currentItem();
  if (item == nullptr) return;

  // Delay activation until after scope such that dialog is closed before activation, which ensures
  // the blue activation line is shown in the binary view.
  QTimer::singleShot(1, [this, item] { activateItem(item); });

  close();
}

void OmniSearchDialog::candidateContextMenu(const QPoint &pos)
{
  const auto *selectedItem = candidatesWidget->currentItem();

  // Currently no other actions apply, so don't show context menu if no item is selected.
  if (selectedItem == nullptr) return;

  QMenu menu;

  if (selectedItem != nullptr) {
    menu.addAction(tr("Jump to match"), this, &OmniSearchDialog::activateCurrentItem);
    menu.addAction(tr("Copy matched text"), this, &OmniSearchDialog::copyCurrentText);
  }

  menu.exec(QCursor::pos());
}

void OmniSearchDialog::copyCurrentText()
{
  const auto *selectedItem = candidatesWidget->currentItem();
  if (selectedItem == nullptr) return;

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

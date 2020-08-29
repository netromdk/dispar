#include "widgets/TreeWidget.h"
#include "Constants.h"
#include "widgets/ConversionHelper.h"
#include "widgets/DisassemblerDialog.h"
#include "widgets/LineEdit.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>

namespace dispar {

TreeWidget::TreeWidget(QWidget *parent)
  : QTreeWidget(parent), cpuType(CpuType::X86), ctxItem(nullptr), ctxCol(-1), addrColumn(-1),
    curCol(0), curItem(0), cur(0), total(0)
{
  setSelectionBehavior(QAbstractItemView::SelectItems);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setEditTriggers(QAbstractItemView::DoubleClicked);
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QTreeWidget::customContextMenuRequested, this, &TreeWidget::onShowContextMenu);

  setFont(Constants::FIXED_FONT);

  searchEdit = new LineEdit(this);
  searchEdit->setVisible(false);
  searchEdit->setFixedWidth(150);
  searchEdit->setFixedHeight(21);
  searchEdit->setPlaceholderText(tr("Search query"));
  connect(searchEdit, &LineEdit::focusLost, this, &TreeWidget::onSearchLostFocus);
  connect(searchEdit, &LineEdit::keyDown, this, &TreeWidget::nextSearchResult);
  connect(searchEdit, &LineEdit::keyUp, this, &TreeWidget::prevSearchResult);
  connect(searchEdit, &LineEdit::returnPressed, this, &TreeWidget::onSearchReturnPressed);
  connect(searchEdit, &LineEdit::textEdited, this, &TreeWidget::onSearchEdited);

  searchLabel = new QLabel(this);
  searchLabel->setVisible(false);
  searchLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  searchLabel->setFixedHeight(searchEdit->height());
  searchLabel->setStyleSheet("QLabel { "
                             "background-color: #EEEEEE; "
                             "border-top: 1px solid #CCCCCC; "
                             "}");
}

void TreeWidget::setMachineCodeColumns(const QList<int> &columns)
{
  if (columns.isEmpty()) {
    machineCodeColumns.clear();
    return;
  }

  int cols = columnCount();
  for (const int col : columns) {
    if (col < cols) {
      machineCodeColumns << col;
    }
  }

  // Remove duplicates.
  machineCodeColumns = QSet<int>(machineCodeColumns.begin(), machineCodeColumns.end()).values();

  if (machineCodeColumns.size() > cols) {
    machineCodeColumns.clear();
  }
}

void TreeWidget::setAddressColumn(int column)
{
  if (column < 0 || column > columnCount() - 1) {
    addrColumn = -1;
    return;
  }

  addrColumn = column;
}

void TreeWidget::keyPressEvent(QKeyEvent *event)
{
  QTreeWidget::keyPressEvent(event);

  bool ctrl{false};
#ifdef MAC
  ctrl = ((event->modifiers() | Qt::MetaModifier) != 0U);
#else
  ctrl = event->modifiers() | Qt::ControlModifier;
#endif
  if (ctrl && event->key() == Qt::Key_F) {
    doSearch();
  }
  else if (event->key() == Qt::Key_Escape) {
    endSearch();
  }
}

void TreeWidget::resizeEvent(QResizeEvent *event)
{
  QTreeWidget::resizeEvent(event);

  if (searchEdit->isVisible()) {
    searchEdit->move(width() - searchEdit->width() - 1, height() - searchEdit->height() - 1);
    searchLabel->setFixedWidth(width() - searchEdit->width());
    searchLabel->move(1, searchEdit->pos().y());
  }
}

void TreeWidget::endSearch()
{
  searchEdit->hide();
  searchLabel->hide();
  searchEdit->clear();
  searchLabel->clear();
  setFocus();
}

void TreeWidget::onShowContextMenu(const QPoint &pos)
{
  QMenu menu;
  menu.addAction(tr("Search"), this, SLOT(doSearch()));

  if (addrColumn != -1) {
    menu.addAction(tr("Find address"), this, SLOT(findAddress()));
  }

  ctxItem = itemAt(pos);
  if (ctxItem != nullptr) {
    ctxCol = indexAt(pos).column();

    menu.addSeparator();
    menu.addAction(tr("Copy field"), this, SLOT(copyField()));
    menu.addAction(tr("Copy row"), this, SLOT(copyRow()));

    if (machineCodeColumns.contains(ctxCol)) {
      menu.addSeparator();
      menu.addAction(tr("Disassemble"), this, SLOT(disassemble()));
    }

    menu.addSeparator();
    menu.addAction(tr("Conversion helper"), this, SLOT(showConversionHelper()));
  }

  // Use cursor because mapToGlobal(pos) is off by the height of the
  // tree widget header anyway.
  menu.exec(QCursor::pos());

  ctxItem = nullptr;
  ctxCol = -1;
}

void TreeWidget::doSearch()
{
  searchEdit->move(width() - searchEdit->width() - 1, height() - searchEdit->height() - 1);
  searchEdit->show();
  searchEdit->setFocus();
}

void TreeWidget::disassemble()
{
  if (ctxItem == nullptr) return;
  QString text = ctxItem->text(ctxCol);
  quint64 offset{0};
  if (addrColumn != -1) {
    bool ok = false;
    offset = ctxItem->text(addrColumn).toULongLong(&ok, 16);
    if (!ok) offset = 0;
  }
  DisassemblerDialog diag(this, cpuType, text, offset);
  diag.exec();
}

void TreeWidget::copyField()
{
  if (ctxItem == nullptr) return;
  QString text = ctxItem->text(ctxCol);
  QApplication::clipboard()->setText(text);
}

void TreeWidget::copyRow()
{
  if (ctxItem == nullptr) return;
  QString text;
  for (int i = 0; i < columnCount(); i++) {
    text += ctxItem->text(i);
    if (i < columnCount() - 1) {
      text += "\t";
    }
  }
  QApplication::clipboard()->setText(text);
}

void TreeWidget::findAddress()
{
  bool ok = false;
  QString text = QInputDialog::getText(this, tr("Find Address"), tr("Address (hex):"),
                                       QLineEdit::Normal, QString(), &ok);
  if (!ok || text.isEmpty()) {
    return;
  }

  quint64 num = text.toULongLong(&ok, 16);
  if (!ok) {
    QMessageBox::warning(this, "dispar", tr("Invalid address! Must be in hexadecimal."));
    findAddress();
    return;
  }

  int cnt = topLevelItemCount();
  for (int i = 0; i < cnt; i++) {
    auto *item = topLevelItem(i);
    quint64 n = item->text(addrColumn).toULongLong(&ok, 16);
    if (!ok) continue;
    if (n == num) {
      setCurrentItem(item);
      scrollToItem(item, QAbstractItemView::PositionAtCenter);
      return;
    }

    if (i < cnt - 1) {
      auto *item2 = topLevelItem(i + 1);
      quint64 n2 = item2->text(addrColumn).toULongLong(&ok, 16);
      if (!ok) continue;
      if (num >= n && num < n2) {
        setCurrentItem(item);
        scrollToItem(item, QAbstractItemView::PositionAtCenter);
        return;
      }
    }
  }

  QMessageBox::information(this, "dispar", tr("Did not find anything."));
}

void TreeWidget::showConversionHelper()
{
  auto *helper = new ConversionHelper(this);
  connect(helper, &QDialog::finished, helper, &QDialog::deleteLater);
  helper->show();
}

void TreeWidget::resetSearch()
{
  searchEdit->clear();
  searchLabel->clear();
  searchLabel->hide();
  searchResults.clear();
  lastQuery.clear();
  curCol = curItem = cur = total = 0;
}

void TreeWidget::onSearchLostFocus()
{
  if (searchEdit->isVisible() && searchEdit->text().isEmpty()) {
    endSearch();
  }
}

void TreeWidget::onSearchReturnPressed()
{
  QString query = searchEdit->text().trimmed();
  if (query.isEmpty()) {
    resetSearch();
    return;
  }

  if (query == lastQuery) {
    nextSearchResult();
    return;
  }

  int cols = columnCount();
  searchResults.clear();
  total = 0;
  for (int col = 0; col < cols; col++) {
    auto res = findItems(query, Qt::MatchContains, col);
    if (!res.isEmpty()) {
      searchResults[col] = res;
      total += res.size();
    }
  }

  if (searchResults.isEmpty()) {
    showSearchText(tr("No matches found"));
    return;
  }

  lastQuery = query;
  cur = 0;
  curCol = searchResults.keys().first();
  curItem = 0;
  selectSearchResult(curCol, curItem);
}

void TreeWidget::selectSearchResult(int col, int item)
{
  if (!searchResults.contains(col)) {
    return;
  }

  const auto &list = searchResults[col];
  if (item < 0 || item > list.size() - 1) {
    return;
  }

  const auto &res = list[item];

  showSearchText(tr("%1 of %2 matches").arg(cur + 1).arg(total));

  // Select entry and not entire row.
  scrollToItem(res, QAbstractItemView::PositionAtCenter);
  int row = indexOfTopLevelItem(res);
  auto index = model()->index(row, col);
  selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

void TreeWidget::nextSearchResult()
{
  const auto &list = searchResults[curCol];
  int pos = curItem;
  pos++;
  if (pos > list.size() - 1) {
    curItem = 0;
    const auto &keys = searchResults.keys();
    int pos2 = keys.indexOf(curCol);
    pos2++;
    if (pos2 > keys.size() - 1) {
      curCol = keys[0];
    }
    else {
      curCol = keys[pos2];
    }
  }
  else {
    curItem = pos;
  }

  cur++;
  if (cur > total - 1) {
    cur = 0;
  }

  selectSearchResult(curCol, curItem);
}

void TreeWidget::prevSearchResult()
{
  int pos = curItem;
  pos--;
  if (pos < 0) {
    const auto &keys = searchResults.keys();
    int pos2 = keys.indexOf(curCol);
    pos2--;
    if (pos2 < 0) {
      curCol = keys.last();
    }
    else {
      curCol = keys[pos2];
    }
    curItem = searchResults[curCol].size() - 1;
  }
  else {
    curItem = pos;
  }

  cur--;
  if (cur < 0) {
    cur = total - 1;
  }

  selectSearchResult(curCol, curItem);
}

void TreeWidget::onSearchEdited(const QString &text)
{
  // If search was performed or no results were found then hide search
  // label when editing the field.
  if (!lastQuery.isEmpty() || searchResults.isEmpty()) {
    searchLabel->clear();
    searchLabel->hide();
  }
}

void TreeWidget::showSearchText(const QString &text)
{
  searchLabel->setText(text + "    ");
  searchLabel->setFixedWidth(width() - searchEdit->width());
  searchLabel->move(1, searchEdit->pos().y());
  searchLabel->show();
}

} // namespace dispar

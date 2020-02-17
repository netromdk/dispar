#ifndef SRC_WIDGETS_TREEWIDGET_H
#define SRC_WIDGETS_TREEWIDGET_H

#include "CpuType.h"

#include <QList>
#include <QMap>
#include <QTreeWidget>

class QLabel;

namespace dispar {

class LineEdit;

class TreeWidget : public QTreeWidget {
  Q_OBJECT

public:
  TreeWidget(QWidget *parent = nullptr);

  void setCpuType(CpuType type)
  {
    cpuType = type;
  }

  void setMachineCodeColumns(const QList<int> &columns);
  void setAddressColumn(int column);

protected:
  void keyPressEvent(QKeyEvent *event);
  void resizeEvent(QResizeEvent *event);

private slots:
  void doSearch();
  void endSearch();
  void onSearchLostFocus();
  void onSearchReturnPressed();
  void nextSearchResult();
  void prevSearchResult();
  void onSearchEdited(const QString &text);
  void onShowContextMenu(const QPoint &pos);
  void disassemble();
  void copyField();
  void copyRow();
  void findAddress();
  void showConversionHelper();

private:
  void resetSearch();
  void selectSearchResult(int col, int item);
  void showSearchText(const QString &text);

  QList<int> machineCodeColumns;
  CpuType cpuType;
  QTreeWidgetItem *ctxItem;
  int ctxCol, addrColumn;

  QMap<int, QList<QTreeWidgetItem *>> searchResults;
  int curCol, curItem, cur, total;
  QString lastQuery;

  LineEdit *searchEdit;
  QLabel *searchLabel;
};

} // namespace dispar

#endif // SRC_WIDGETS_TREEWIDGET_H

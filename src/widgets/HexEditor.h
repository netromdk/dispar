#ifndef SRC_WIDGETS_HEXEDITOR_H
#define SRC_WIDGETS_HEXEDITOR_H

#include <QDateTime>
#include <QDialog>

#include "BinaryObject.h"
#include "Section.h"

class TreeWidget;

class QLabel;
class QTreeWidgetItem;

class HexEditor : public QDialog {
public:
  HexEditor(Section *section, BinaryObject *object, QWidget *parent = nullptr);

protected:
  void showEvent(QShowEvent *event) override;

private:
  void createLayout();
  void setup();
  void createEntries();
  void markModifiedRegions();

  Section *section;
  BinaryObject *object;

  bool shown;
  int rows;
  QLabel *label;
  TreeWidget *treeWidget;
};

#endif // SRC_WIDGETS_HEXEDITOR_H

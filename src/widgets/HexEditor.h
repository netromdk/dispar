#ifndef SRC_WIDGETS_HEXEDITOR_H
#define SRC_WIDGETS_HEXEDITOR_H

#include <QDateTime>
#include <QDialog>

#include "BinaryObject.h"
#include "Section.h"

class QLabel;
class QTreeWidgetItem;

namespace dispar {

class TreeWidget;

class HexEditor : public QDialog {
public:
  HexEditor(Section *section, BinaryObject *object, QWidget *parent = nullptr);
  virtual ~HexEditor();

  void updateModified();

protected:
  void showEvent(QShowEvent *event) override;

private:
  void createLayout();
  void setup();
  void createEntries();
  void markModifiedRegions();

  Section *section;
  BinaryObject *object;
  QDateTime sectionModified;

  bool shown;
  int rows;
  QLabel *label;
  TreeWidget *treeWidget;
};

} // namespace dispar

#endif // SRC_WIDGETS_HEXEDITOR_H

#ifndef SRC_WIDGETS_DISASSEMBLYEDITOR_H
#define SRC_WIDGETS_DISASSEMBLYEDITOR_H

#include <QDialog>

class Section;
class TreeWidget;
class BinaryObject;

class QLabel;
class QPushButton;

class DisassemblyEditor : public QDialog {
public:
  DisassemblyEditor(Section *section, BinaryObject *object, QWidget *parent = nullptr);

  void showUpdateButton();

protected:
  void showEvent(QShowEvent *event);

private slots:
  void updateDisassembly();

private:
  void createLayout();
  void setup();

  Section *section;
  BinaryObject *object;

  bool shown;
  QLabel *label;
  QPushButton *updateButton;
  TreeWidget *treeWidget;
};

#endif // SRC_WIDGETS_DISASSEMBLYEDITOR_H

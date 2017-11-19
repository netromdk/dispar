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

public slots:
  void done(int result) override;

protected:
  void showEvent(QShowEvent *event) override;

private slots:
  void updateDisassembly();

private:
  void createLayout();
  void setup();
  void createEntries();
  void markModifiedRegions();

  Section *section;
  BinaryObject *object;

  bool shown;
  QLabel *label;
  QPushButton *updateButton;
  TreeWidget *treeWidget;
};

#endif // SRC_WIDGETS_DISASSEMBLYEDITOR_H

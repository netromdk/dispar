#ifndef SRC_WIDGETS_DISASSEMBLYEDITOR_H
#define SRC_WIDGETS_DISASSEMBLYEDITOR_H

#include <QDateTime>
#include <QDialog>

class QLabel;
class QPushButton;

namespace dispar {

class Section;
class TreeWidget;
class BinaryObject;

class DisassemblyEditor : public QDialog {
public:
  DisassemblyEditor(Section *section, BinaryObject *object, QWidget *parent = nullptr);
  ~DisassemblyEditor() override;

  DisassemblyEditor(const DisassemblyEditor &other) = delete;
  DisassemblyEditor &operator=(const DisassemblyEditor &rhs) = delete;

  DisassemblyEditor(DisassemblyEditor &&other) = delete;
  DisassemblyEditor &operator=(DisassemblyEditor &&rhs) = delete;

  void showUpdateButton();
  void updateModified();

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
  QDateTime sectionModified, lastModified;

  bool shown = false;
  QLabel *label = nullptr;
  QPushButton *updateButton = nullptr;
  TreeWidget *treeWidget = nullptr;
};

} // namespace dispar

#endif // SRC_WIDGETS_DISASSEMBLYEDITOR_H

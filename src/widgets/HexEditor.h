#ifndef SRC_WIDGETS_HEXEDITOR_H
#define SRC_WIDGETS_HEXEDITOR_H

#include <QDateTime>
#include <QDialog>
#include <QPointer>

#include "BinaryObject.h"
#include "Section.h"

class QLabel;
class QProgressDialog;

namespace dispar {

class HexEdit;

class HexEditor : public QDialog {
public:
  HexEditor(Section *section, BinaryObject *object, QWidget *parent = nullptr);
  ~HexEditor() override;

  HexEditor(const HexEditor &other) = delete;
  HexEditor &operator=(const HexEditor &rhs) = delete;

  HexEditor(HexEditor &&other) = delete;
  HexEditor &operator=(HexEditor &&rhs) = delete;

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

  Section *section;
  BinaryObject *object;
  QDateTime sectionModified, lastModified;

  bool shown = false;
  QLabel *label = nullptr;
  HexEdit *textEdit = nullptr;

  QPointer<QProgressDialog> progDiag = nullptr;
};

} // namespace dispar

#endif // SRC_WIDGETS_HEXEDITOR_H

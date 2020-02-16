#ifndef SRC_WIDGETS_MACSDKVERSIONSEDITOR_H
#define SRC_WIDGETS_MACSDKVERSIONSEDITOR_H

#include <QDialog>

#include "BinaryObject.h"
#include "Section.h"

class QSpinBox;

namespace dispar {

class MacSdkVersionsEditor : public QDialog {
public:
  MacSdkVersionsEditor(Section *section, BinaryObject *object, QWidget *parent = nullptr);
  virtual ~MacSdkVersionsEditor();

  void accept() override;

protected:
  void showEvent(QShowEvent *event) override;

private:
  void setup();
  void readValues();
  void createLayout();

  Section *section;

  QSpinBox *targetMajorSpin, *targetMinorSpin, *sdkMajorSpin, *sdkMinorSpin;

  quint64 targetAddr, sdkAddr;
  int targetMajor, targetMinor;
  int sdkMajor, sdkMinor;

  bool shown;
  QDateTime sectionModified;
};

} // namespace dispar

#endif // SRC_WIDGETS_MACSDKVERSIONSEDITOR_H

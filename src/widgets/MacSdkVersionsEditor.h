#ifndef SRC_WIDGETS_MACSDKVERSIONSEDITOR_H
#define SRC_WIDGETS_MACSDKVERSIONSEDITOR_H

#include <QDialog>

#include "BinaryObject.h"
#include "MacSdkVersionPatcher.h"
#include "Section.h"

class QSpinBox;

namespace dispar {

class MacSdkVersionsEditor : public QDialog {
public:
  MacSdkVersionsEditor(Section *section, BinaryObject *object, QWidget *parent = nullptr);
  ~MacSdkVersionsEditor() override;

  void accept() override;

protected:
  void showEvent(QShowEvent *event) override;

private:
  void setup();
  void readValues();
  void createLayout();

  Section *section;
  MacSdkVersionPatcher patcher;

  QSpinBox *targetMajorSpin = nullptr, *targetMinorSpin = nullptr, *sdkMajorSpin = nullptr,
           *sdkMinorSpin = nullptr;

  bool shown = false;
  QDateTime sectionModified;
};

} // namespace dispar

#endif // SRC_WIDGETS_MACSDKVERSIONSEDITOR_H

#ifndef DISPAR_OPTIONS_DIALOG_H
#define DISPAR_OPTIONS_DIALOG_H

#include "Debugger.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;

namespace dispar {

class OptionsDialog : public QDialog {
  Q_OBJECT

public:
  OptionsDialog(QWidget *parent = nullptr);

private slots:
  void onTestDebugger();
  void onDetectInstalledDebuggers();
  void onAccept();

private:
  void createLayout();

  /// Returns instance of debugger from values in UI.
  Debugger currentDebugger() const;

  QCheckBox *showMachineCode;
  QComboBox *disAsmSyntax, *logLevelBox;
  QLineEdit *debuggerEdit, *launchPatternEdit, *versionArgumentEdit;
};

} // namespace dispar

#endif // DISPAR_OPTIONS_DIALOG_H

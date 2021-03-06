#ifndef DISPAR_OPTIONS_DIALOG_H
#define DISPAR_OPTIONS_DIALOG_H

#include "Debugger.h"

#include <QDialog>

class QLabel;
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
  void onSyntaxChanged(int index);

private:
  void createLayout();

  /// Returns instance of debugger from values in UI.
  [[nodiscard]] Debugger currentDebugger() const;

  QCheckBox *showMachineCode = nullptr;
  QComboBox *disAsmSyntax = nullptr, *logLevelBox = nullptr;
  QLineEdit *debuggerEdit = nullptr, *launchPatternEdit = nullptr, *versionArgumentEdit = nullptr;
  QLabel *disAsmExample = nullptr;
};

} // namespace dispar

#endif // DISPAR_OPTIONS_DIALOG_H

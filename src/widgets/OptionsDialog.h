#ifndef DISPAR_OPTIONS_DIALOG_H
#define DISPAR_OPTIONS_DIALOG_H

#include <QDialog>

class QCheckBox;
class QComboBox;

class OptionsDialog : public QDialog {
  Q_OBJECT

public:
  OptionsDialog(QWidget *parent = nullptr);

private slots:
  void onAccept();

private:
  void createLayout();

  QCheckBox *showMachineCode;
  QComboBox *disAsmSyntax;
};

#endif // DISPAR_OPTIONS_DIALOG_H

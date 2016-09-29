#ifndef DISPAR_OPTIONS_DIALOG_H
#define DISPAR_OPTIONS_DIALOG_H

#include <QDialog>

class QCheckBox;

class OptionsDialog : public QDialog {
  Q_OBJECT

public:
  OptionsDialog(QWidget *parent = nullptr);
  ~OptionsDialog();

private slots:
  void onAccept();

private:
  void createLayout();

  QCheckBox *showMachineCode;
};

#endif // DISPAR_OPTIONS_DIALOG_H

#ifndef DISPAR_ABOUT_DIALOG_H
#define DISPAR_ABOUT_DIALOG_H

#include <QDialog>

class AboutDialog : public QDialog {
public:
  AboutDialog(QWidget *parent = nullptr);

private:
  void createLayout();
};

#endif // DISPAR_ABOUT_DIALOG_H

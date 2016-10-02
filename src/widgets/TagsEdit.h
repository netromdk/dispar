#ifndef DISPAR_TAGS_EDIT_H
#define DISPAR_TAGS_EDIT_H

#include <QWidget>

class QLineEdit;
class QListWidget;

class TagsEdit : public QWidget {
  Q_OBJECT

public:
  TagsEdit();

  void setAddress(quint64 address);

private slots:
  void onReturnPressed();

private:
  void createLayout();

  quint64 address;
  QListWidget *listWidget;
  QLineEdit *lineEdit;
};

#endif // DISPAR_TAGS_EDIT_H

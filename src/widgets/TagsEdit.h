#ifndef DISPAR_TAGS_EDIT_H
#define DISPAR_TAGS_EDIT_H

#include <QWidget>

class QLineEdit;
class QListWidget;

class TagItemDelegate;

class TagsEdit : public QWidget {
  Q_OBJECT

public:
  TagsEdit();
  ~TagsEdit();

  void setAddress(quint64 address);

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private slots:
  void onReturnPressed();

  void updateTags();

private:
  void createLayout();
  void removeTag();

  quint64 address;
  QListWidget *listWidget;
  QLineEdit *lineEdit;
  TagItemDelegate *itemDelegate;
};

#endif // DISPAR_TAGS_EDIT_H

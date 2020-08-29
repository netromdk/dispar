#ifndef DISPAR_TAGS_EDIT_H
#define DISPAR_TAGS_EDIT_H

#include <QWidget>

class QLineEdit;
class QListWidget;

namespace dispar {

class TagItemDelegate;

class TagsEdit : public QWidget {
  Q_OBJECT

public:
  TagsEdit();
  ~TagsEdit() override;

  TagsEdit(const TagsEdit &other) = delete;
  TagsEdit &operator=(const TagsEdit &rhs) = delete;

  TagsEdit(TagsEdit &&other) = delete;
  TagsEdit &operator=(TagsEdit &&rhs) = delete;

  void setAddress(quint64 address);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void onReturnPressed();

  void updateTags();

private:
  void createLayout();
  void removeTag();

  quint64 address_ = 0;
  QListWidget *listWidget = nullptr;
  QLineEdit *lineEdit = nullptr;
  TagItemDelegate *itemDelegate = nullptr;
};

} // namespace dispar

#endif // DISPAR_TAGS_EDIT_H

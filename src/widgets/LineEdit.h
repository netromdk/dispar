#ifndef SRC_WIDGETS_LINEEDIT_H
#define SRC_WIDGETS_LINEEDIT_H

#include <QLineEdit>

namespace dispar {

class LineEdit : public QLineEdit {
  Q_OBJECT

public:
  LineEdit(QWidget *parent = nullptr);

signals:
  void focusLost();
  void keyUp();
  void keyDown();

protected:
  void focusOutEvent(QFocusEvent *event);
  void keyPressEvent(QKeyEvent *event);
};

} // namespace dispar

#endif // SRC_WIDGETS_LINEEDIT_H

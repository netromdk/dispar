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

  /// Ctrl+N, with N in {1, 2, 3, 4, 5, 6, 7, 8, 9}.
  /** On macOS, it is Command instead. */
  void keyCtrlNumber(int num);

protected:
  void focusOutEvent(QFocusEvent *event);
  void keyPressEvent(QKeyEvent *event);
};

} // namespace dispar

#endif // SRC_WIDGETS_LINEEDIT_H

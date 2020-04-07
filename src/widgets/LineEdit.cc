#include <QKeyEvent>

#include "LineEdit.h"

namespace dispar {

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent)
{
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
  QLineEdit::focusOutEvent(event);
  emit focusLost();
}

void LineEdit::keyPressEvent(QKeyEvent *event)
{
  static const QList<int> numberKeys{Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4, Qt::Key_5,
                                     Qt::Key_6, Qt::Key_7, Qt::Key_8, Qt::Key_9};

  if (event->key() == Qt::Key_Up) {
    emit keyUp();
  }
  else if (event->key() == Qt::Key_Down) {
    emit keyDown();
  }
  else if ((event->modifiers() & Qt::CTRL) > 0 && numberKeys.contains(event->key())) {
    const int num = (event->key() - Qt::Key_1 + 1);
    emit keyCtrlNumber(num);
  }
  else {
    QLineEdit::keyPressEvent(event);
  }
}

} // namespace dispar

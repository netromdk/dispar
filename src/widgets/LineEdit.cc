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
  if (event->key() == Qt::Key_Up) {
    emit keyUp();
  }
  else if (event->key() == Qt::Key_Down) {
    emit keyDown();
  }
  else {
    QLineEdit::keyPressEvent(event);
  }
}

} // namespace dispar

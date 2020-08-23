#include "widgets/TagItemDelegate.h"

#include <QPainter>
#include <QPainterPath>

namespace dispar {

void TagItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
  painter->save();

  painter->setRenderHint(QPainter::Antialiasing);

  // Use a rectangle that is 5% smaller to give some room.
  auto lessRect = option.rect;
  auto w = float(lessRect.width()) * 0.95f;
  auto diffW = lessRect.width() - w;
  auto h = float(lessRect.height()) * 0.95f;
  auto diffH = lessRect.height() - h;
  lessRect.setWidth(w);
  lessRect.setHeight(h);
  lessRect.setX(lessRect.x() + diffW);
  lessRect.setY(lessRect.y() + diffH);

  QPainterPath path;
  path.addRoundedRect(lessRect, 10, 10);

  QBrush bg("#cce0ff");
  if (option.state & QStyle::State_Selected && option.state & QStyle::State_HasFocus) {
    bg = QBrush("#80b3ff");
  }

  painter->fillPath(path, bg);
  // painter->drawPath(path);

  auto font = option.font;
  font.setPointSize(font.pointSize() - 1);
  painter->setFont(font);

  auto tag = index.data().toString();
  painter->drawText(lessRect, Qt::AlignCenter, tag);

  painter->restore();
}

QSize TagItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  auto tag = index.data().toString();
  QFontMetrics fm(option.font);
  return QSize(float(fm.horizontalAdvance(tag)) * 1.3f, float(fm.height()) * 1.5f);
}

} // namespace dispar

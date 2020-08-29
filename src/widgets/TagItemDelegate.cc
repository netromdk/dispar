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
  auto w = float(lessRect.width()) * 0.95F;
  auto diffW = float(lessRect.width()) - w;
  auto h = float(lessRect.height()) * 0.95F;
  auto diffH = float(lessRect.height()) - h;
  lessRect.setWidth(w);
  lessRect.setHeight(h);
  lessRect.setX(lessRect.x() + int(diffW));
  lessRect.setY(lessRect.y() + int(diffH));

  QPainterPath path;
  path.addRoundedRect(lessRect, 10, 10);

  QBrush bg("#cce0ff");
  if (((option.state & QStyle::State_Selected) != 0U) &&
      ((option.state & QStyle::State_HasFocus) != 0U)) {
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
  const auto tag = index.data().toString();
  const QFontMetrics fm(option.font);
  return {int(float(fm.horizontalAdvance(tag)) * 1.3F), int(float(fm.height()) * 1.5F)};
}

} // namespace dispar

#ifndef DISPAR_TAG_ITEM_DELEGATE_H
#define DISPAR_TAG_ITEM_DELEGATE_H

#include <QAbstractItemDelegate>

namespace dispar {

class TagItemDelegate : public QAbstractItemDelegate {
public:
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option,
                               const QModelIndex &index) const override;
};

} // namespace dispar

#endif // DISPAR_TAG_ITEM_DELEGATE_H

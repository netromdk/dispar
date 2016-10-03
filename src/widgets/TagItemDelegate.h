#ifndef DISPAR_TAG_ITEM_DELEGATE_H
#define DISPAR_TAG_ITEM_DELEGATE_H

#include <QAbstractItemDelegate>

class TagItemDelegate : public QAbstractItemDelegate {
public:
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // DISPAR_TAG_ITEM_DELEGATE_H

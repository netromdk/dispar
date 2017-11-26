#include "widgets/PersistentSplitter.h"
#include "Context.h"

#include <QDebug>
#include <QShowEvent>

namespace dispar {

PersistentSplitter::PersistentSplitter(const QString &settingsKey, Qt::Orientation orientation,
                                       QWidget *parent)
  : QSplitter(orientation, parent), settingsKey(settingsKey)
{
}

PersistentSplitter::~PersistentSplitter()
{
  QVariantList var;
  for (const auto &size : sizes()) {
    var << size;
  }
  Context::get().setValue(settingsKey, var);
}

void PersistentSplitter::showEvent(QShowEvent *event)
{
  QSplitter::showEvent(event);

  const auto list = Context::get().value(settingsKey, QVariantList()).toList();
  QList<int> sizes;
  bool ok;
  for (const auto &elm : list) {
    auto tmp = elm.toInt(&ok);
    if (ok) {
      sizes << tmp;
    }
  }
  if (!sizes.isEmpty()) {
    setSizes(sizes);
  }
}

} // namespace dispar

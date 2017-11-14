#include "widgets/PersistentSplitter.h"

#include <QDebug>
#include <QSettings>
#include <QShowEvent>

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
  QSettings().setValue(settingsKey, var);
}

void PersistentSplitter::showEvent(QShowEvent *event)
{
  QSplitter::showEvent(event);

  QSettings settings;
  if (settings.contains(settingsKey)) {
    auto var = settings.value(settingsKey);
    if (var.type() == QVariant::List) {
      QList<int> sizes;
      bool ok;
      for (const auto &elm : var.toList()) {
        auto tmp = elm.toInt(&ok);
        if (ok) {
          sizes << tmp;
        }
      }
      if (!sizes.isEmpty()) {
        setSizes(sizes);
      }
    }
  }
}

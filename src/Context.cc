#include "Context.h"

#include <QDebug>
#include <QSettings>

Context::Context()
{
  qDebug() << "Loading settings";
  QSettings settings;
  showMachineCode_ = settings.value("Context.showMachineCode", true).toBool();
}

Context::~Context()
{
  qDebug() << "Saving settings";
  QSettings settings;
  settings.setValue("Context.showMachineCode", showMachineCode());
}

Context &Context::get()
{
  static Context instance;
  return instance;
}

bool Context::showMachineCode() const
{
  return showMachineCode_;
}

void Context::setShowMachineCode(bool show)
{
  showMachineCode_ = show;
  emit showMachineCodeChanged(show);
}

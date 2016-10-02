#include "Context.h"
#include "Project.h"

#include <QDebug>
#include <QFile>
#include <QSettings>

Context::Context() : project_(nullptr)
{
  loadSettings();
}

Context::~Context()
{
  saveSettings();
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
  bool changed = (show != showMachineCode_);
  showMachineCode_ = show;
  if (changed) {
    emit showMachineCodeChanged(show);
  }
}

Disassembler::Syntax Context::disassemblerSyntax() const
{
  return disassemblerSyntax_;
}

void Context::setDisassemblerSyntax(Disassembler::Syntax syntax)
{
  disassemblerSyntax_ = syntax;
}

void Context::loadSettings()
{
  qDebug() << "Loading settings";
  QSettings settings;
  showMachineCode_ = settings.value("Context.showMachineCode", true).toBool();
  disassemblerSyntax_ = static_cast<Disassembler::Syntax>(
    settings.value("Context.disassemblerSyntax", static_cast<int>(Disassembler::Syntax::INTEL))
      .toInt());
}

void Context::saveSettings()
{
  qDebug() << "Saving settings";
  QSettings settings;
  settings.setValue("Context.showMachineCode", showMachineCode());
  settings.setValue("Context.disassemblerSyntax", static_cast<int>(disassemblerSyntax()));
}

std::shared_ptr<Project> Context::project() const
{
  return project_;
}

std::shared_ptr<Project> Context::resetProject()
{
  clearProject();
  project_ = std::make_shared<Project>();
  return project_;
}

void Context::clearProject()
{
  if (project_) {
    project_.reset();
  }
  project_ = nullptr;
}

std::shared_ptr<Project> Context::loadProject(const QString &file)
{
  project_ = Project::load(file);
  return project_;
}

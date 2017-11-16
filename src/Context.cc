#include "Context.h"
#include "Project.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

static QString settingsPath()
{
  return QDir::home().absoluteFilePath(".dispar");
}

} // anon

Context::Context()
  : showMachineCode_(true), disassemblerSyntax_(Disassembler::Syntax::INTEL), project_(nullptr)
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
  auto path = settingsPath();
  qDebug() << "Loading settings from" << path;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qCritical() << "Could not read settings from" << path;
    return;
  }

  auto doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isNull() || doc.isEmpty() || !doc.isObject()) {
    qCritical() << "Malformed or empty settings file!";
    return;
  }

  auto obj = doc.object();
  if (obj.contains("showMachineCode")) {
    showMachineCode_ = obj["showMachineCode"].toBool(true);
  }
  if (obj.contains("disassemblerSyntax")) {
    disassemblerSyntax_ = static_cast<Disassembler::Syntax>(
      obj["disassemblerSyntax"].toInt(static_cast<int>(Disassembler::Syntax::INTEL)));
  }
}

void Context::saveSettings()
{
  auto path = settingsPath();
  qDebug() << "Saving settings to" << path;

  QJsonObject obj;
  obj["showMachineCode"] = showMachineCode();
  obj["disassemblerSyntax"] = static_cast<int>(disassemblerSyntax());

  QJsonDocument doc;
  doc.setObject(obj);

  auto data = doc.toJson();
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    qCritical() << "Could not write settings to" << path;
    return;
  }
  file.write(data);
}

Project *Context::project() const
{
  return project_.get();
}

Project *Context::resetProject()
{
  clearProject();
  project_ = std::make_unique<Project>();
  return project();
}

void Context::clearProject()
{
  if (project_) {
    project_.reset(nullptr);
  }
}

Project *Context::loadProject(const QString &file)
{
  project_ = Project::load(file);
  return project();
}

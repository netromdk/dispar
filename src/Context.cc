#include "Context.h"
#include "Project.h"

#include "cxx.h"

#include <cassert>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

static QString settingsPath()
{
  return QDir::home().absoluteFilePath(".dispar");
}

} // namespace

namespace dispar {

namespace {

Context *instance = nullptr;
bool initialized = false;

} // namespace

Context::Context()
  : showMachineCode_(true), disassemblerSyntax_(Disassembler::Syntax::INTEL), backupEnabled_(true),
    backupAmount_(5), project_(nullptr)
{
  ASSERT_X(!instance, "Only one Context can be live at any one time");
  instance = this;
}

Context::~Context()
{
  assert(instance);
  instance = nullptr;

  ASSERT_X(initialized, "Context wasn't initialized before being destroyed");
  initialized = false;

  saveSettings();
}

Context &Context::get()
{
  ASSERT_X(instance, "Context must be constructed first");
  return *instance;
}

void Context::init()
{
  ASSERT_X(!initialized, "Context is already initialized");
  initialized = true;

  logHandler = std::make_unique<LogHandler>(*this);
  loadSettings();
}

void Context::setVerbose(bool verbose)
{
  verbose_ = verbose;
}

bool Context::verbose() const
{
  return verbose_;
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

bool Context::backupEnabled() const
{
  return backupEnabled_;
}

void Context::setBackupEnabled(bool enabled)
{
  backupEnabled_ = enabled;
}

int Context::backupAmount() const
{
  return backupAmount_;
}

void Context::setBackupAmount(int amount)
{
  backupAmount_ = amount;
}

const QStringList &Context::recentProjects()
{
  for (int i = recentProjects_.size() - 1; i >= 0; i--) {
    if (!QFile::exists(recentProjects_[i])) {
      recentProjects_.removeAt(i);
    }
  }
  return recentProjects_;
}

void Context::addRecentProject(const QString &project)
{
  if (!recentProjects_.contains(project)) {
    recentProjects_ << project;
  }
  if (recentProjects_.size() > 10) {
    recentProjects_.removeFirst();
  }
}

const QStringList &Context::recentBinaries()
{
  for (int i = recentBinaries_.size() - 1; i >= 0; i--) {
    if (!QFile::exists(recentBinaries_[i])) {
      recentBinaries_.removeAt(i);
    }
  }
  return recentBinaries_;
}

void Context::addRecentBinary(const QString &binary)
{
  if (!recentBinaries_.contains(binary)) {
    recentBinaries_ << binary;
  }
  if (recentBinaries_.size() > 10) {
    recentBinaries_.removeFirst();
  }
}

void Context::setValue(const QString &key, const QVariant &value)
{
  // Only allow types that are handled by JSON.
  assert(static_cast<QMetaType::Type>(value.type()) == QMetaType::Bool ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::Int ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::UInt ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::LongLong ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::ULongLong ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::Float ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::Double ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::QString ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::QStringList ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::QVariantList ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::QVariantMap ||
         static_cast<QMetaType::Type>(value.type()) == QMetaType::QVariantHash);
  values[key] = value;
}

QVariant Context::value(const QString &key, const QVariant &defaultValue) const
{
  return values.value(key, defaultValue);
}

void Context::setDebugger(const Debugger &debugger)
{
  debugger_ = debugger;
}

Debugger Context::debugger() const
{
  return debugger_;
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

  if (obj.contains("backup")) {
    const auto backupValue = obj["backup"];
    if (backupValue.isObject()) {
      const auto backupObj = backupValue.toObject();
      if (backupObj.contains("enabled")) {
        backupEnabled_ = backupObj["enabled"].toBool(true);
      }

      if (backupObj.contains("amount")) {
        backupAmount_ = backupObj["amount"].toInt(5);
      }
    }
  }

  if (obj.contains("recent")) {
    const auto recentValue = obj["recent"];
    if (recentValue.isObject()) {
      const auto recentObj = recentValue.toObject();

      if (recentObj.contains("projects")) {
        const auto projectsValue = recentObj["projects"];
        if (projectsValue.isArray()) {
          for (const auto &value : projectsValue.toArray().toVariantList()) {
            addRecentProject(value.toString());
          }
        }
      }

      if (recentObj.contains("binaries")) {
        const auto binariesValue = recentObj["binaries"];
        if (binariesValue.isArray()) {
          for (const auto &value : binariesValue.toArray().toVariantList()) {
            addRecentBinary(value.toString());
          }
        }
      }
    }
  }

  if (obj.contains("values")) {
    const auto valuesValue = obj["values"];
    if (valuesValue.isObject()) {
      values = valuesValue.toObject().toVariantHash();
    }
  }

  if (obj.contains("debugger")) {
    const auto debuggerValue = obj["debugger"];
    if (debuggerValue.isObject()) {
      const auto debuggerObj = debuggerValue.toObject();

      QString program, launchPattern, versionArgument;
      if (debuggerObj.contains("program")) {
        program = debuggerObj["program"].toString();
      }
      if (debuggerObj.contains("launchPattern")) {
        launchPattern = debuggerObj["launchPattern"].toString();
      }
      if (debuggerObj.contains("versionArgument")) {
        versionArgument = debuggerObj["versionArgument"].toString();
      }

      Debugger dbg(program, versionArgument, launchPattern);
      if (dbg.valid()) {
        setDebugger(dbg);
      }
    }
  }
}

void Context::saveSettings()
{
  auto path = settingsPath();
  qDebug() << "Saving settings to" << path;

  QJsonObject backupObj;
  backupObj["enabled"] = backupEnabled();
  backupObj["amount"] = backupAmount();

  QJsonObject recentObj;
  recentObj["projects"] = QJsonArray::fromStringList(recentProjects());
  recentObj["binaries"] = QJsonArray::fromStringList(recentBinaries());

  QJsonObject debuggerObj;
  if (debugger_.valid()) {
    debuggerObj["program"] = debugger_.program();
    debuggerObj["launchPattern"] = debugger_.launchPattern();
    debuggerObj["versionArgument"] = debugger_.versionArgument();
  }

  QJsonObject obj;
  obj["showMachineCode"] = showMachineCode();
  obj["disassemblerSyntax"] = static_cast<int>(disassemblerSyntax());
  obj["backup"] = backupObj;
  obj["recent"] = recentObj;
  obj["values"] = QJsonValue::fromVariant(values);
  obj["debugger"] = debuggerObj;

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

} // namespace dispar

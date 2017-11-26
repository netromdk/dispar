#include "Context.h"
#include "Project.h"

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

Context::Context()
  : showMachineCode_(true), disassemblerSyntax_(Disassembler::Syntax::INTEL), backupEnabled_(true),
    backupAmount_(5), project_(nullptr)
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

void Context::setGeometry(const QString &key, const QByteArray &geometry)
{
  geometries[key] = geometry;
}

QByteArray Context::geometry(const QString &key) const
{
  return geometries.value(key);
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
  values[key] = value;
}

QVariant Context::value(const QString &key, const QVariant &defaultValue) const
{
  return values.value(key, defaultValue);
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

  if (obj.contains("geometry")) {
    const auto geometryValue = obj["geometry"];
    if (geometryValue.isObject()) {
      const auto geometryObj = geometryValue.toObject();
      for (const auto &key : geometryObj.keys()) {
        const auto val = geometryObj[key];
        if (!val.isString()) continue;

        const auto geometry = QByteArray::fromHex(val.toString().toUtf8());
        setGeometry(key, geometry);
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
}

void Context::saveSettings()
{
  auto path = settingsPath();
  qDebug() << "Saving settings to" << path;

  QJsonObject backupObj;
  backupObj["enabled"] = backupEnabled();
  backupObj["amount"] = backupAmount();

  QJsonObject geometryObj;
  for (const auto &key : geometries.keys()) {
    geometryObj[key] = QString::fromUtf8(geometries[key].toHex());
  }

  QJsonObject recentObj;
  recentObj["projects"] = QJsonArray::fromStringList(recentProjects());
  recentObj["binaries"] = QJsonArray::fromStringList(recentBinaries());

  QJsonObject obj;
  obj["showMachineCode"] = showMachineCode();
  obj["disassemblerSyntax"] = static_cast<int>(disassemblerSyntax());
  obj["backup"] = backupObj;
  obj["geometry"] = geometryObj;
  obj["recent"] = recentObj;
  obj["values"] = QJsonValue::fromVariant(values);

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

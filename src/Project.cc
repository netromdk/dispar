#include "Project.h"
#include "cxx.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

static constexpr int PROJECT_VERSION = 1;

} // namespace

namespace dispar {

Project::Project()
{
  qDebug() << "Create project";
}

Project::~Project()
{
  qDebug() << "Destroy project" << file() << binary();
}

std::unique_ptr<Project> Project::load(const QString &file)
{
  qDebug() << "Loading from" << file;

  auto project = std::make_unique<Project>();

  QFile qfile(file);
  if (!qfile.open(QIODevice::ReadOnly)) {
    qCritical() << "Could not read project from" << file;
    return nullptr;
  }

  auto doc = QJsonDocument::fromBinaryData(qfile.readAll());
  if (doc.isNull() || doc.isEmpty() || !doc.isObject()) {
    qCritical() << "Malformed or empty project file!";
    return nullptr;
  }

  auto obj = doc.object();
  if (!obj.contains("version")) {
    qCritical() << "Project does not contain project version!";
    return nullptr;
  }

  // TODO: Use this when project version is bumped in the future to handle parsing older formats.
  int version = obj["version"].toInt();
  qDebug() << "Version:" << version;

  if (obj.contains("binary")) {
    project->setBinary(obj["binary"].toString());
  }

  if (obj.contains("addressTags")) {
    auto tmp = obj["addressTags"];
    if (!tmp.isObject()) return nullptr;

    auto tagsObj = tmp.toObject();
    for (const auto &key : tagsObj.keys()) {
      bool ok;
      quint64 addr = key.toLongLong(&ok);
      if (!ok) return nullptr;

      auto arr = tagsObj[key];
      if (!arr.isArray()) return nullptr;

      for (const auto &tag : arr.toArray()) {
        project->addAddressTag(tag.toString(), addr);
      }
    }
  }

  if (obj.contains("modifiedRegions")) {
    const auto tmp = obj["modifiedRegions"];
    if (!tmp.isObject()) return nullptr;

    const auto modsObj = tmp.toObject();
    for (const auto &key : modsObj.keys()) {
      bool ok;
      auto const addr = key.toLongLong(&ok);
      if (!ok) return nullptr;

      const auto val = modsObj[key];
      if (!val.isString()) return nullptr;

      const auto data = QByteArray::fromHex(val.toString().toUtf8());
      project->addModifiedRegion(addr, data);
    }
  }

  // Set where file was loaded from.
  project->file_ = file;
  return project;
}

bool Project::save(const QString &path)
{
  auto outFile = path;
  if (outFile.isEmpty()) {
    outFile = file();
  }

  qDebug() << "Saving to" << outFile;

  QJsonObject obj;
  obj["version"] = PROJECT_VERSION;
  obj["binary"] = binary();

  QJsonObject tagsObj;
  for (const auto addr : addressTags_.keys()) {
    tagsObj[QString::number(addr)] = QJsonArray::fromStringList(addressTags_[addr]);
  }
  obj["addressTags"] = tagsObj;

  QJsonObject modsObj;
  for (const auto addr : modifiedRegions_.keys()) {
    modsObj[QString::number(addr)] = QString::fromUtf8(modifiedRegions_[addr].toHex());
  }
  obj["modifiedRegions"] = modsObj;

  QJsonDocument doc;
  doc.setObject(obj);

  auto data = doc.toBinaryData();
  QFile qfile(outFile);
  if (!qfile.open(QIODevice::WriteOnly)) {
    qCritical() << "Could not write project to" << outFile;
    return false;
  }
  qfile.write(data);

  file_ = outFile;
  return true;
}

QString Project::binary() const
{
  return binary_;
}

void Project::setBinary(const QString &file)
{
  binary_ = file;
}

QString Project::file() const
{
  return file_;
}

QStringList Project::addressTags(quint64 address) const
{
  if (addressTags_.contains(address)) {
    return addressTags_[address];
  }
  return QStringList();
}

const QHash<quint64, QStringList> &Project::tags() const
{
  return addressTags_;
}

bool Project::addAddressTag(const QString &tag, quint64 address)
{
  if (cxx::any_of(addressTags_.keys(),
                  [&](const auto addr) { return addressTags_[addr].contains(tag); })) {
    return false;
  }

  if (addressTags_.contains(address)) {
    addressTags_[address].append(tag);
  }
  else {
    addressTags_[address] = QStringList{tag};
  }

  emit modified();
  emit tagsChanged();
  return true;
}

bool Project::removeAddressTag(const QString &tag)
{
  return removeAddressTags({tag});
}

bool Project::removeAddressTags(const QStringList &tags)
{
  if (tags.isEmpty()) {
    return false;
  }

  bool removed = false;
  for (const auto &tag : tags) {
    for (const auto addr : addressTags_.keys()) {
      auto &tags_ = addressTags_[addr];
      if (tags_.contains(tag)) {
        tags_.removeAll(tag);
        removed = true;
        break;
      }
    }
  }

  if (removed) {
    emit modified();
    emit tagsChanged();
  }

  return removed;
}

const QMap<quint64, QByteArray> &Project::modifiedRegions() const
{
  return modifiedRegions_;
}

void Project::addModifiedRegion(const quint64 address, const QByteArray &data)
{
  modifiedRegions_[address] = data;
}

void Project::clearModifiedRegions()
{
  modifiedRegions_.clear();
}

} // namespace dispar

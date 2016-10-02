#include "Project.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

Project::Project()
{
  qDebug() << "Create project";
}

Project::~Project()
{
  qDebug() << "Destroy project" << file() << binary();
}

std::shared_ptr<Project> Project::load(const QString &file)
{
  qDebug() << "Loading from" << file;

  auto project = std::make_shared<Project>();

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
  if (obj.contains("binary")) {
    project->setBinary(obj["binary"].toString());
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
  obj["binary"] = binary();

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

#include "Project.h"

#include <QDebug>
#include <QSettings>

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

  // TODO: Use another format!
  QSettings settings(file, QSettings::IniFormat);
  if (QSettings::NoError != settings.status()) {
    return nullptr;
  }

  if (!settings.contains("binary")) {
    return nullptr;
  }

  auto binary = settings.value("binary");
  if (binary.isNull() || binary.type() != QVariant::String) {
    return nullptr;
  }
  project->setBinary(binary.toString());

  return project;
}

bool Project::save(const QString &path)
{
  auto outFile = path;
  if (outFile.isEmpty()) {
    outFile = file();
  }

  qDebug() << "Saving to" << outFile;

  // TODO: Use another format!
  QSettings settings(outFile, QSettings::IniFormat);
  settings.setValue("binary", binary());
  settings.sync();

  if (QSettings::NoError != settings.status()) {
    return false;
  }

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

#include "Project.h"

#include <QDebug>

Project::Project()
{
  qDebug() << "Create project";
}

Project::~Project()
{
  qDebug() << "Destroy project" << project() << binary();
}

std::shared_ptr<Project> Project::load(const QString &file)
{
  return nullptr;
}

bool Project::save(const QString &file)
{
  return false;
}

QString Project::binary() const
{
  return binary_;
}

void Project::setBinary(const QString &file)
{
  binary_ = file;
}

QString Project::project() const
{
  return project_;
}

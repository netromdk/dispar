#include "testutils.h"

#include <QDir>
#include <QFile>
#include <QUuid>

std::unique_ptr<QFile, std::function<void(QFile *)>> tempFile(const QByteArray &data)
{
  auto deleter = [](QFile *file) {
    if (file) {
      file->remove();
      delete file;
    }
  };

  auto *file = new QFile(tempFilePath());

  if (!data.isEmpty()) {
    file->open(QIODevice::WriteOnly);
    file->write(data);
    file->close();
  }

  return std::unique_ptr<QFile, std::function<void(QFile *) >>(file, deleter);
}

QString tempFilePath()
{
  auto name = QUuid::createUuid().toString();
  auto path = QDir::temp().absoluteFilePath(name);
  QFile::remove(path);
  return path;
}

std::ostream &operator<<(std::ostream &os, const QString &str)
{
  return os << str.toStdString();
}

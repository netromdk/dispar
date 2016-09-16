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

  auto name = QUuid::createUuid().toString();
  auto *file = new QFile(QDir::temp().absoluteFilePath(name));
  file->remove();

  if (!data.isEmpty()) {
    file->open(QIODevice::WriteOnly);
    file->write(data);
    file->close();
  }

  return std::unique_ptr<QFile, std::function<void(QFile *) >>(file, deleter);
}

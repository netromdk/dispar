#include <QString>
#include <QDebug>

#include "Util.h"
#include "formats/Format.h"

int main(int argc, char **argv)
{
  // Temporary!
  if (argc != 2) return -1;
  auto file = QString::fromUtf8(argv[1]);
  qDebug() << "input:" << file;

  auto fmt = Format::detect(file);
  if (!fmt) return -1;
  qDebug() << "format:" << Util::formatTypeString(fmt->getType());

  qDebug() << "parsing..";
  bool ret = fmt->parse();
  if (!ret) return -1;

  const auto &objects = fmt->getObjects();
  qDebug() << "#objects:" << objects.size();

  for (const auto &object : objects) {
    qDebug() << "===";
    qDebug() << "cpu type:" << Util::cpuTypeString(object->getCpuType());
    qDebug() << "cpu sub type:" << Util::cpuTypeString(object->getCpuSubType());
    qDebug() << "little endian:" << object->isLittleEndian();
    qDebug() << "file type:" << Util::fileTypeString(object->getFileType());
  }

  return 0;
}

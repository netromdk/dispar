#include <QString>
#include <QDebug>

#include "Util.h"
#include "CpuType.h"
#include "formats/Format.h"

int main(int argc, char **argv)
{
  // Temporary!
  if (argc != 2) return -1;
  auto file = QString::fromUtf8(argv[1]);
  qDebug() << "input:" << file;

  auto fmt = Format::detect(file);
  if (!fmt) return -1;
  qDebug() << "format:" << Format::typeName(fmt->getType());

  qDebug() << "parsing..";
  bool ret = fmt->parse();
  if (!ret) return -1;

  const auto &objects = fmt->getObjects();
  qDebug() << "#objects:" << objects.size();

  for (const auto &object : objects) {
    qDebug() << "===========";
    qDebug() << "cpu type:" << cpuTypeName(object->getCpuType());
    qDebug() << "cpu sub type:" << cpuTypeName(object->getCpuSubType());
    qDebug() << "little endian:" << object->isLittleEndian();
    qDebug() << "file type:" << fileTypeName(object->getFileType());

    const auto &sections = object->getSections();
    for (const auto &section : sections) {
      qDebug() << "***";
      qDebug() << "section name:" << section->getName();
      qDebug() << "section type:" << Section::typeName(section->getType());
      qDebug() << "section size:" << Util::formatSize(section->getSize());
    }
  }

  return 0;
}

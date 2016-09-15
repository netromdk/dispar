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
  qDebug() << "format:" << Format::typeName(fmt->type());

  qDebug() << "parsing..";
  bool ret = fmt->parse();
  if (!ret) return -1;

  const auto &objects = fmt->objects();
  qDebug() << "#objects:" << objects.size();

  for (const auto &object : objects) {
    qDebug() << "===========";
    qDebug() << "cpu type:" << cpuTypeName(object->cpuType());
    qDebug() << "cpu sub type:" << cpuTypeName(object->cpuSubType());
    qDebug() << "little endian:" << object->isLittleEndian();
    qDebug() << "file type:" << fileTypeName(object->fileType());

    const auto &sections = object->sections();
    for (const auto &section : sections) {
      qDebug() << "***";
      qDebug() << "section name:" << section->name();
      qDebug() << "section type:" << Section::typeName(section->type());
      qDebug() << "section size:" << Util::formatSize(section->size());
    }
  }

  return 0;
}

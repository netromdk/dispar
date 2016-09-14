#include <QString>
#include <QDebug>

#include "Util.h"
#include "formats/Format.h"

int main(int argc, char **argv) {
  // Temporary!
  if (argc != 2) return -1;
  auto file = QString::fromUtf8(argv[1]);
  qDebug() << "input:" << file;

  auto fmt = Format::detect(file);
  if (!fmt) return -1;

  qDebug() << "format:" << Util::formatTypeString(fmt->getType());

  return 0;
}

#include "FormatLoader.h"
#include "../BinaryObject.h"
#include "Format.h"

#include <QDateTime>
#include <QDebug>

FormatLoader::FormatLoader(const QString &file) : file(file)
{
}

void FormatLoader::run()
{
  auto start = QDateTime::currentDateTime();

  auto fmt = Format::detect(file);
  if (fmt == nullptr) {
    emit failed(tr("Unknown file type - could not detect or open!"));
    return;
  }

  auto typeName = Format::typeName(fmt->type());
  emit status(tr("Detected %1 - Reading and parsing binary..").arg(typeName));
  emit progress(0.5);

  if (!fmt->parse()) {
    emit failed(tr("Could not parse file!"));
    return;
  }

  emit progress(1);
  emit status(tr("Success!"));

  auto end = QDateTime::currentDateTime();
  qDebug() << "Loaded in" << start.msecsTo(end) << "ms";

  emit success(fmt);
}

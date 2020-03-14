#include "formats/FormatLoader.h"
#include "BinaryObject.h"
#include "formats/Format.h"

#include <QDebug>
#include <QElapsedTimer>

namespace dispar {

FormatLoader::FormatLoader(const QString &file_) : file(file_)
{
}

void FormatLoader::run()
{
  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  auto fmt = Format::detect(file);
  if (fmt == nullptr) {
    emit failed(tr("Unknown file type - could not detect or open!"));
    return;
  }

  const auto typeName = Format::typeName(fmt->type());
  emit status(tr("Detected %1 - Reading and parsing binary..").arg(typeName));
  emit progress(0.5);

  if (!fmt->parse()) {
    emit failed(tr("Could not parse file!"));
    return;
  }

  emit progress(1);
  emit status(tr("Success!"));

  qDebug() << ">" << elapsedTimer.restart() << "ms";
  emit success(fmt);
}

} // namespace dispar

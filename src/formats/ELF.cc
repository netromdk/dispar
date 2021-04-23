#include <QDebug>
#include <QFile>

#include <cmath>

#include "BinaryObject.h"
#include "Reader.h"
#include "Util.h"
#include "formats/ELF.h"

namespace dispar {

ELF::ELF(const QString &file) : Format(Format::Type::ELF), file_{file}
{
}

QString ELF::file() const
{
  return file_;
}

bool ELF::detect()
{
  qDebug() << "Running ELF::detect()";
  QFile f{file_};
  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }

  Reader r(f);
  r.setLittleEndian(false);

  bool ok = false;
  quint32 magic = r.getUInt32(&ok);
  if (!ok) return false;

  // magic: [u8; 4] = [0x7F, E, L, F]
  return magic == 0x7F454C46;
}

bool ELF::parse()
{
  qWarning() << "Not implemented!";
  return true;
}

QList<BinaryObject *> ELF::objects() const
{
  qWarning() << "Not implemented!";
  QList<BinaryObject *> res;
  return res;
}

bool ELF::parseHeader(quint32 offset, quint32 size, Reader &r)
{
  qWarning() << "Not implemented!";
  return true;
}

} // namespace dispar

#ifndef BMOD_MACHO_FORMAT_H
#define BMOD_MACHO_FORMAT_H

#include "Format.h"

class Reader;

class MachO : public Format {
public:
  MachO(const QString &file);

  QString getFile() const override;

  bool detect() override;
  bool parse() override;

  QList<std::shared_ptr<BinaryObject>> getObjects() const override;

private:
  bool parseHeader(quint32 offset, quint32 size, Reader &reader);

  QString file;
  QList<std::shared_ptr<BinaryObject>> objects;
};

#endif // BMOD_MACHO_FORMAT_H

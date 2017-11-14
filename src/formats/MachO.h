#ifndef DISPAR_MACHO_FORMAT_H
#define DISPAR_MACHO_FORMAT_H

#include "formats/Format.h"

class Reader;

class MachO : public Format {
public:
  MachO(const QString &file);

  QString file() const override;

  bool detect() override;
  bool parse() override;

  QList<std::shared_ptr<BinaryObject>> objects() const override;

private:
  bool parseHeader(quint32 offset, quint32 size, Reader &reader);

  QString file_;
  QList<std::shared_ptr<BinaryObject>> objects_;
};

#endif // DISPAR_MACHO_FORMAT_H

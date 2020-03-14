#ifndef DISPAR_MACHO_FORMAT_H
#define DISPAR_MACHO_FORMAT_H

#include "formats/Format.h"

#include <vector>

namespace dispar {

class Reader;

class MachO : public Format {
public:
  MachO(const QString &file);
  virtual ~MachO() override = default;

  QString file() const override;

  bool detect() override;
  bool parse() override;

  QList<BinaryObject *> objects() const override;

private:
  bool parseHeader(quint32 offset, quint32 size, Reader &reader);

  QString file_;
  std::vector<std::unique_ptr<BinaryObject>> objects_;
};

} // namespace dispar

#endif // DISPAR_MACHO_FORMAT_H

#ifndef DISPAR_MACHO_FORMAT_H
#define DISPAR_MACHO_FORMAT_H

#include "formats/Format.h"

#include <vector>

namespace dispar {

class Reader;

class MachO : public Format {
public:
  MachO(const QString &file);
  ~MachO() override = default;

  MachO(const MachO &other) = default;
  MachO &operator=(const MachO &rhs) = default;

  MachO(MachO &&other) = default;
  MachO &operator=(MachO &&rhs) = default;

  [[nodiscard]] QString file() const override;

  bool detect() override;
  bool parse() override;

  [[nodiscard]] QList<BinaryObject *> objects() const override;

private:
  bool parseHeader(quint32 offset, quint32 size, Reader &reader);

  QString file_;
  std::vector<std::unique_ptr<BinaryObject>> objects_;
};

} // namespace dispar

#endif // DISPAR_MACHO_FORMAT_H

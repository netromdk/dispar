#ifndef DISPAR_ELF_FORMAT_H
#define DISPAR_ELF_FORMAT_H

#include "formats/Format.h"

#include <vector>

namespace dispar {

class Reader;

class ELF : public Format {
public:
  ELF(const QString &file);
  ~ELF() override = default;

  ELF(const ELF &other) = default;
  ELF &operator=(const ELF &rhs) = default;

  ELF(ELF &&other) = default;
  ELF &operator=(ELF &&rhs) = default;

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

#endif // DISPAR_ELF_FORMAT_H

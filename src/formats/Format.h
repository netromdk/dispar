#ifndef DISPAR_FORMAT_H
#define DISPAR_FORMAT_H

#include <QList>
#include <QMetaType>
#include <QString>

#include <memory>

#include "CpuType.h"
#include "FileType.h"
#include "Section.h"

class QIODevice;

namespace dispar {

class BinaryObject;

class Format {
public:
  enum class Type { MACH_O };

  Format(Type type);
  virtual ~Format() = default;

  [[nodiscard]] Type type() const;

  [[nodiscard]] virtual QString toString() const;

  [[nodiscard]] virtual QString file() const = 0;

  /// Detect whether the magic code of the file corresponds to the format.
  /** Only reads the first chunk of the file and not all of it! */
  virtual bool detect() = 0;

  /// Parses the file into the various sections and so on.
  virtual bool parse() = 0;

  /// Get the list of probed binary objects of the file.
  /** Format keeps ownership of objects. */
  [[nodiscard]] virtual QList<BinaryObject *> objects() const = 0;

  /// Write modified sections of objects to \p device.
  void write(QIODevice &device);

  /// Try each of the known formats and see if any of them are compatible with the file.
  static std::shared_ptr<Format> detect(const QString &file);

  /// Get string representation of type.
  static QString typeName(Type type);

  static void registerType();

private:
  Type type_;
};

} // namespace dispar

Q_DECLARE_METATYPE(std::shared_ptr<dispar::Format>)

#endif // DISPAR_FORMAT_H

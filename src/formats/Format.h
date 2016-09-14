#ifndef BMOD_FORMAT_H
#define BMOD_FORMAT_H

#include <QList>
#include <QString>

#include <memory>

#include "../Section.h"
#include "../CpuType.h"
#include "../FileType.h"
#include "../BinaryObject.h"

class Format {
public:
  enum class Type {
    MachO
  };

  Format(Type type);

  Type getType() const;

  virtual QString getFile() const =0;

  /// Detect whether the magic code of the file corresponds to the format.
  /** Only reads the first chunk of the file and not all of it! */
  virtual bool detect() =0;

  /// Parses the file into the various sections and so on.
  virtual bool parse() =0;

  /// Get the list of probed binary objects of the file.
  virtual QList<std::shared_ptr<BinaryObject>> getObjects() const =0;

  /// Try each of the known formats and see if any of them are compatible with the file.
  static std::shared_ptr<Format> detect(const QString &file);

private:
  Type type;
};

#endif // BMOD_FORMAT_H

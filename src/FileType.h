#ifndef DISPAR_FILE_TYPE_H
#define DISPAR_FILE_TYPE_H

#include <QString>

namespace dispar {

enum class FileType : int {
  OBJECT,   ///< Intermediate object file (.o).
  EXECUTE,  ///< Executable file.
  CORE,     ///< Crash report.
  PRELOAD,  ///< Special-purpose program, like programs burned into ROM chips.
  DYLIB,    ///< Dynamic shared library (.dylib).
  DYLINKER, ///< Dynamic linker shared library.
  BUNDLE    ///< Bundle/plugin (.bundle).
};

QString fileTypeName(FileType type);

} // namespace dispar

#endif // DISPAR_FILE_TYPE_H

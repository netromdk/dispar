#ifndef BMOD_FILE_TYPE_H
#define BMOD_FILE_TYPE_H

enum class FileType : int {
  Object,   ///< Intermediate object file (.o).
  Execute,  ///< Executable file.
  Core,     ///< Crash report.
  Preload,  ///< Special-purpose program, like programs burned into ROM chips.
  Dylib,    ///< Dynamic shared library (.dylib).
  Dylinker, ///< Dynamic linker shared library.
  Bundle    ///< Bundle/plugin (.bundle).
};

#endif // BMOD_FILE_TYPE_H

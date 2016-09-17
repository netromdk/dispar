#include "FileType.h"

QString fileTypeName(FileType type)
{
  switch (type) {
  case FileType::OBJECT:
    return "Object";

  default:
  case FileType::EXECUTE:
    return "Executable";

  case FileType::CORE:
    return "Core";

  case FileType::PRELOAD:
    return "Preloaded Program";

  case FileType::DYLIB:
    return "Dylib";

  case FileType::DYLINKER:
    return "Dylinker";

  case FileType::BUNDLE:
    return "Bundle";
  }
}

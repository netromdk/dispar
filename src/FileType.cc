#include "FileType.h"

namespace dispar {

QString fileTypeName(FileType type)
{
  switch (type) {
  case FileType::OBJECT:
    return "Object";

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

  // This won't be reached.
  return {};
}

} // namespace dispar

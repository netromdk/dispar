#include "FileType.h"

QString fileTypeName(FileType type)
{
  switch (type) {
  case FileType::Object:
    return "Object";

  default:
  case FileType::Execute:
    return "Executable";

  case FileType::Core:
    return "Core";

  case FileType::Preload:
    return "Preloaded Program";

  case FileType::Dylib:
    return "Dylib";

  case FileType::Dylinker:
    return "Dylinker";

  case FileType::Bundle:
    return "Bundle";
  }
}

#include "MachO.h"
#include "Format.h"

Format::Format(Type type) : type{type}
{
}

Format::Type Format::getType() const
{
  return type;
}

std::shared_ptr<Format> Format::detect(const QString &file)
{
  // Mach-O
  auto res = std::make_shared<MachO>(file);
  if (res->detect()) {
    return res;
  }
  return nullptr;
}

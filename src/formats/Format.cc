#include "MachO.h"
#include "Format.h"

Format::Format(Type type) : type_{type}
{
}

Format::Type Format::type() const
{
  return type_;
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

// *************************************************************************************************
QString Format::typeName(Type type)
// *************************************************************************************************
{
  switch (type) {
  default:
  case Type::MachO:
    return "Mach-O";
  }
}

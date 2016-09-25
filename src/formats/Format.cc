#include "Format.h"
#include "MachO.h"

Format::Format(Type type) : type_(type)
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

QString Format::typeName(Type type)
{
  switch (type) {
  case Type::MACH_O:
    return "Mach-O";
  }

  return "";
}

void Format::registerType()
{
  qRegisterMetaType<std::shared_ptr<Format>>("std::shared_ptr<Format>");
}

#include "formats/Format.h"
#include "BinaryObject.h"
#include "Util.h"
#include "formats/MachO.h"

#include <QIODevice>

namespace dispar {

Format::Format(Type type) : type_(type)
{
}

Format::Type Format::type() const
{
  return type_;
}

QString Format::toString() const
{
  const auto objs = objects();
  auto res = QString("Format: %1\n").arg(typeName(type()));
  if (objs.size() > 1) {
    res += QString("%1 objects:\n").arg(objs.size());
  }
  for (const auto *obj : objs) {
    res += QString("- %1\n").arg(obj->toString());
    for (const auto *sec : obj->sections()) {
      res += QString("  - %1\n      0x%2-0x%3 (%4)\n")
               .arg(sec->toString())
               .arg(sec->address(), 0, 16)
               .arg(sec->address() + sec->size(), 0, 16)
               .arg(Util::formatSize(sec->size()));
      if (sec->address() != sec->offset()) {
        res += QString("      0x%1 offset\n").arg(sec->offset(), 0, 16);
      }
    }
  }
  return res;
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

void Format::write(QIODevice &device)
{
  for (const auto *object : objects()) {
    for (const auto *section : object->sections()) {
      if (!section->isModified()) {
        continue;
      }

      const auto &data = section->data();
      for (const auto &region : section->modifiedRegions()) {
        device.seek(section->offset() + region.position);
        device.write(data.mid(region.position, region.size));
      }
    }
  }
}

void Format::registerType()
{
  qRegisterMetaType<std::shared_ptr<Format>>("std::shared_ptr<Format>");
}

} // namespace dispar

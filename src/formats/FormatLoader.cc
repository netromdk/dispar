#include "FormatLoader.h"
#include "../BinaryObject.h"
#include "../Disassembler.h"
#include "Format.h"

#include <QDebug>

FormatLoader::FormatLoader(const QString &file) : file(file)
{
}

void FormatLoader::run()
{
  auto fmt = Format::detect(file);
  if (fmt == nullptr) {
    emit failed(tr("Unknown file type - could not detect or open!"));
    return;
  }

  auto typeName = Format::typeName(fmt->type());
  emit status(tr("Detected %1 - Reading and parsing binary..").arg(typeName));

  if (!fmt->parse()) {
    emit failed(tr("Could not parse file!"));
    return;
  }

  emit status(tr("Disassembling code sections.."));

  for (auto &object : fmt->objects()) {
    Disassembler dis(object);
    if (dis.valid()) {
      for (auto &sec : object->sections()) {
        switch (sec->type()) {
        case Section::Type::TEXT:
        case Section::Type::SYMBOL_STUBS: {
          auto res = dis.disassemble(sec->data());
          if (res) {
            sec->setDisassembly(res);
          }
          break;
        }

        default:
          break;
        }
      }
    }
  }

  emit success(fmt);
}

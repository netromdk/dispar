#include "Disassembler.h"
#include "BinaryObject.h"
#include "Util.h"

#include <cassert>

#include <QByteArray>
#include <QDebug>

namespace dispar {

Disassembler::Result::Result(cs_insn *insn, size_t count) : insn(insn), count_(count)
{
}

Disassembler::Result::~Result()
{
  if (insn) {
    cs_free(insn, count_);
  }
}

size_t Disassembler::Result::count() const
{
  return count_;
}

cs_insn *Disassembler::Result::instructions(size_t pos) const
{
  assert(pos < count());
  return insn + pos;
}

Disassembler::Disassembler(const BinaryObject &object, Syntax syntax) : valid_(false)
{
  cs_arch arch;
  switch (object.cpuType()) {
  case CpuType::X86:
  case CpuType::X86_64:
    arch = cs_arch::CS_ARCH_X86;
    break;

  default:
    qCritical() << "Invalid arch:" << cpuTypeName(object.cpuType());
    return;
  }

  int mode = (object.systemBits() == 32 ? cs_mode::CS_MODE_32 : cs_mode::CS_MODE_64);
  mode += (object.isLittleEndian() ? cs_mode::CS_MODE_LITTLE_ENDIAN : cs_mode::CS_MODE_BIG_ENDIAN);

  cs_err err = cs_open(arch, static_cast<cs_mode>(mode), &handle);
  if (err) {
    qCritical() << "Failed to create cs disassembler!" << (int) err;
    return;
  }

  // Don't use CS_OPT_ON because I want to use the 'size' and 'bytes' variables on cs_insn!
  // valid_ = !cs_option(handle, cs_opt_type::CS_OPT_DETAIL, cs_opt_value::CS_OPT_ON);

  cs_opt_value csSyntax;
  switch (syntax) {
  case Syntax::ATT:
    csSyntax = cs_opt_value::CS_OPT_SYNTAX_ATT;
    break;

  default:
  case Syntax::INTEL:
    csSyntax = cs_opt_value::CS_OPT_SYNTAX_INTEL;
    break;

  case Syntax::INTEL_MASM:
    csSyntax = cs_opt_value::CS_OPT_SYNTAX_MASM;
    break;
  }

  valid_ = !cs_option(handle, cs_opt_type::CS_OPT_SYNTAX, csSyntax);
}

Disassembler::~Disassembler()
{
  if (valid()) {
    cs_close(&handle);
  }
}

std::unique_ptr<Disassembler::Result> Disassembler::disassemble(const QByteArray &data,
                                                                quint64 baseAddr)
{
  const void *code = data.constData();
  cs_insn *insn = nullptr;
  size_t count =
    cs_disasm(handle, static_cast<const unsigned char *>(code), data.size(), baseAddr, 0, &insn);
  if (count == 0) {
    return nullptr;
  }
  return std::make_unique<Result>(insn, count);
}

std::unique_ptr<Disassembler::Result> Disassembler::disassemble(const QString &text,
                                                                quint64 baseAddr)
{
  auto input = Util::hexToData(text.simplified().trimmed().replace(" ", ""));
  return disassemble(input, baseAddr);
}

bool Disassembler::valid() const
{
  return valid_;
}

} // namespace dispar

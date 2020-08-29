#include "CpuType.h"

namespace dispar {

QString cpuTypeName(CpuType type)
{
  switch (type) {
  case CpuType::X86:
    return "x86";

  case CpuType::X86_64:
    return "x86 64";

  case CpuType::HPPA:
    return "HPPA";

  case CpuType::ARM:
    return "ARM";

  case CpuType::ARM_64:
    return "ARM 64";

  case CpuType::SPARC:
    return "SPARC";

  case CpuType::I860:
    return "i860";

  case CpuType::POWER_PC:
    return "PowerPC";

  case CpuType::POWER_PC_64:
    return "PowerPC 64";

  case CpuType::I386:
    return "i386";

  case CpuType::I486:
    return "i486";

  case CpuType::I486_SX:
    return "i486 SX";

  case CpuType::PENTIUM:
    return "Pentium";

  case CpuType::PENTIUM_PRO:
    return "Pentium Pro";

  case CpuType::PENTIUM_II_M3:
    return "Pentium II M3";

  case CpuType::PENTIUM_II_M5:
    return "Pentium II M5";

  case CpuType::CELERON:
    return "Celeron";

  case CpuType::CELERON_MOBILE:
    return "Celeron Mobile";

  case CpuType::PENTIUM_3:
    return "Pentium 3";

  case CpuType::PENTIUM_3_M:
    return "Pentium 3 M";

  case CpuType::PENTIUM_3_Xeon:
    return "Pentium 3 Xeon";

  case CpuType::PENTIUM_M:
    return "Pentium M";

  case CpuType::PENTIUM_4:
    return "Pentium 4";

  case CpuType::PENTIUM_4_M:
    return "Pentium 4 M";

  case CpuType::ITANIUM:
    return "Itanium";

  case CpuType::ITANIUM_2:
    return "Itanium 2";

  case CpuType::XEON:
    return "Xeon";

  case CpuType::XEON_MP:
    return "Xeon MP";
  }

  // This won't be reached.
  return {};
}

} // namespace dispar

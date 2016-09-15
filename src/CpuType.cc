#include "CpuType.h"

QString cpuTypeName(CpuType type)
{
  switch (type) {
  default:
  case CpuType::X86:
    return "x86";

  case CpuType::X86_64:
    return "x86 64";

  case CpuType::HPPA:
    return "HPPA";

  case CpuType::ARM:
    return "ARM";

  case CpuType::SPARC:
    return "SPARC";

  case CpuType::I860:
    return "i860";

  case CpuType::PowerPC:
    return "PowerPC";

  case CpuType::PowerPC_64:
    return "PowerPC 64";

  case CpuType::I386:
    return "i386";

  case CpuType::I486:
    return "i486";

  case CpuType::I486_SX:
    return "i486 SX";

  case CpuType::Pentium:
    return "Pentium";

  case CpuType::PentiumPro:
    return "Pentium Pro";

  case CpuType::PentiumII_M3:
    return "Pentium II M3";

  case CpuType::PentiumII_M5:
    return "Pentium II M5";

  case CpuType::Celeron:
    return "Celeron";

  case CpuType::CeleronMobile:
    return "Celeron Mobile";

  case CpuType::Pentium_3:
    return "Pentium 3";

  case CpuType::Pentium_3_M:
    return "Pentium 3 M";

  case CpuType::Pentium_3_Xeon:
    return "Pentium 3 Xeon";

  case CpuType::Pentium_M:
    return "Pentium M";

  case CpuType::Pentium_4:
    return "Pentium 4";

  case CpuType::Pentium_4_M:
    return "Pentium 4 M";

  case CpuType::Itanium:
    return "Itanium";

  case CpuType::Itanium_2:
    return "Itanium 2";

  case CpuType::Xeon:
    return "Xeon";

  case CpuType::Xeon_MP:
    return "Xeon MP";
  }
}

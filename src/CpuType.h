#ifndef DISPAR_CPU_TYPE_H
#define DISPAR_CPU_TYPE_H

#include <QString>

enum class CpuType : int {
  X86, ///< Same as I386
  X86_64,
  HPPA,
  ARM,
  SPARC,
  I860,
  POWER_PC,
  POWER_PC_64,

  // Sub types
  I386,
  I486,
  I486_SX,
  PENTIUM,
  PENTIUM_PRO,
  PENTIUM_II_M3,
  PENTIUM_II_M5,
  CELERON,
  CELERON_MOBILE,
  PENTIUM_3,
  PENTIUM_3_M,
  PENTIUM_3_Xeon,
  PENTIUM_M,
  PENTIUM_4,
  PENTIUM_4_M,
  ITANIUM,
  ITANIUM_2,
  XEON,
  XEON_MP
};

QString cpuTypeName(CpuType type);

#endif // DISPAR_CPU_TYPE_H

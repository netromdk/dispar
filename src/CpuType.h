#ifndef BMOD_CPU_TYPE_H
#define BMOD_CPU_TYPE_H

enum class CpuType : int {
  X86, // Same as i386
  X86_64,
  HPPA,
  ARM,
  SPARC,
  I860,
  PowerPC,
  PowerPC_64,

  // Sub types
  I386,
  I486,
  I486_SX,
  Pentium,
  PentiumPro,
  PentiumII_M3,
  PentiumII_M5,
  Celeron,
  CeleronMobile,
  Pentium_3,
  Pentium_3_M,
  Pentium_3_Xeon,
  Pentium_M,
  Pentium_4,
  Pentium_4_M,
  Itanium,
  Itanium_2,
  Xeon,
  Xeon_MP
};

#endif // BMOD_CPU_TYPE_H

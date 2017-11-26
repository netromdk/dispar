#include "gtest/gtest.h"

#include "CpuType.h"
using namespace dispar;

TEST(CpuType, types)
{
  EXPECT_EQ((int) CpuType::X86, 0);
  EXPECT_EQ((int) CpuType::X86_64, 1);
  EXPECT_EQ((int) CpuType::HPPA, 2);
  EXPECT_EQ((int) CpuType::ARM, 3);
  EXPECT_EQ((int) CpuType::ARM_64, 4);
  EXPECT_EQ((int) CpuType::SPARC, 5);
  EXPECT_EQ((int) CpuType::I860, 6);
  EXPECT_EQ((int) CpuType::POWER_PC, 7);
  EXPECT_EQ((int) CpuType::POWER_PC_64, 8);
  EXPECT_EQ((int) CpuType::I386, 9);
  EXPECT_EQ((int) CpuType::I486, 10);
  EXPECT_EQ((int) CpuType::I486_SX, 11);
  EXPECT_EQ((int) CpuType::PENTIUM, 12);
  EXPECT_EQ((int) CpuType::PENTIUM_PRO, 13);
  EXPECT_EQ((int) CpuType::PENTIUM_II_M3, 14);
  EXPECT_EQ((int) CpuType::PENTIUM_II_M5, 15);
  EXPECT_EQ((int) CpuType::CELERON, 16);
  EXPECT_EQ((int) CpuType::CELERON_MOBILE, 17);
  EXPECT_EQ((int) CpuType::PENTIUM_3, 18);
  EXPECT_EQ((int) CpuType::PENTIUM_3_M, 19);
  EXPECT_EQ((int) CpuType::PENTIUM_3_Xeon, 20);
  EXPECT_EQ((int) CpuType::PENTIUM_M, 21);
  EXPECT_EQ((int) CpuType::PENTIUM_4, 22);
  EXPECT_EQ((int) CpuType::PENTIUM_4_M, 23);
  EXPECT_EQ((int) CpuType::ITANIUM, 24);
  EXPECT_EQ((int) CpuType::ITANIUM_2, 25);
  EXPECT_EQ((int) CpuType::XEON, 26);
  EXPECT_EQ((int) CpuType::XEON_MP, 27);
}

TEST(CpuType, typeNames)
{
  EXPECT_EQ(cpuTypeName(CpuType::X86), "x86");
  EXPECT_EQ(cpuTypeName(CpuType::X86_64), "x86 64");
  EXPECT_EQ(cpuTypeName(CpuType::HPPA), "HPPA");
  EXPECT_EQ(cpuTypeName(CpuType::ARM), "ARM");
  EXPECT_EQ(cpuTypeName(CpuType::SPARC), "SPARC");
  EXPECT_EQ(cpuTypeName(CpuType::I860), "i860");
  EXPECT_EQ(cpuTypeName(CpuType::POWER_PC), "PowerPC");
  EXPECT_EQ(cpuTypeName(CpuType::POWER_PC_64), "PowerPC 64");
  EXPECT_EQ(cpuTypeName(CpuType::I386), "i386");
  EXPECT_EQ(cpuTypeName(CpuType::I486), "i486");
  EXPECT_EQ(cpuTypeName(CpuType::I486_SX), "i486 SX");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM), "Pentium");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_PRO), "Pentium Pro");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_II_M3), "Pentium II M3");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_II_M5), "Pentium II M5");
  EXPECT_EQ(cpuTypeName(CpuType::CELERON), "Celeron");
  EXPECT_EQ(cpuTypeName(CpuType::CELERON_MOBILE), "Celeron Mobile");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_3), "Pentium 3");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_3_M), "Pentium 3 M");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_3_Xeon), "Pentium 3 Xeon");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_M), "Pentium M");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_4), "Pentium 4");
  EXPECT_EQ(cpuTypeName(CpuType::PENTIUM_4_M), "Pentium 4 M");
  EXPECT_EQ(cpuTypeName(CpuType::ITANIUM), "Itanium");
  EXPECT_EQ(cpuTypeName(CpuType::ITANIUM_2), "Itanium 2");
  EXPECT_EQ(cpuTypeName(CpuType::XEON), "Xeon");
  EXPECT_EQ(cpuTypeName(CpuType::XEON_MP), "Xeon MP");
}

#include "gtest/gtest.h"

#include "formats/Format.h"

TEST(Format, types)
{
  EXPECT_EQ((int) Format::Type::MachO, 0);
}

TEST(Format, typeNames)
{
  EXPECT_EQ(Format::typeName(Format::Type::MachO), "Mach-O");
}

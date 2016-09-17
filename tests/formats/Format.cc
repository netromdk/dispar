#include "gtest/gtest.h"

#include "testutils.h"

#include "formats/Format.h"

TEST(Format, types)
{
  EXPECT_EQ((int) Format::Type::MACH_O, 0);
}

TEST(Format, typeNames)
{
  EXPECT_EQ(Format::typeName(Format::Type::MACH_O), "Mach-O");
  EXPECT_EQ(Format::typeName(static_cast<Format::Type>(-1)), "");
}

TEST(Format, detect)
{
  auto fmt = Format::detect("");
  ASSERT_EQ(fmt, nullptr);

  fmt = Format::detect(":macho_main");
  ASSERT_NE(fmt, nullptr);
  EXPECT_EQ(fmt->type(), Format::Type::MACH_O);
}

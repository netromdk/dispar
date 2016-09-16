#include "gtest/gtest.h"

#include "SymbolEntry.h"

TEST(SymbolEntry, instantiate)
{
  SymbolEntry se(0, 0);
  SymbolEntry se2(0, 0, "");
}

TEST(SymbolEntry, index)
{
  SymbolEntry se(42, 0);
  EXPECT_EQ(se.index(), 42);
}

TEST(SymbolEntry, value)
{
  SymbolEntry se(0, 42);
  EXPECT_EQ(se.value(), 42);

  se.setValue(82);
  EXPECT_EQ(se.value(), 82);
}

TEST(SymbolEntry, string)
{
  SymbolEntry se(0, 0, "string");
  EXPECT_EQ(se.string(), "string");

  se.setString("ok");
  EXPECT_EQ(se.string(), "ok");
}

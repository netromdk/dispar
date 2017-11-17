#include "gtest/gtest.h"

#include "testutils.h"

#include "Util.h"

TEST(Util, padString)
{
  EXPECT_EQ(Util::padString("x", 3, true, ' '), "  x");
  EXPECT_EQ(Util::padString("x", 3, false, ' '), "x  ");
}

TEST(Util, demangle)
{
  auto res = Util::demangle("__ZSt9terminatev");
  EXPECT_EQ(res, "std::terminate()") << res;

  res = Util::demangle("__ZNK9QComboBox8findDataERK8QVarianti6QFlagsIN2Qt9MatchFlagEE");
  EXPECT_EQ(res, "QComboBox::findData(QVariant const&, int, QFlags<Qt::MatchFlag>) const") << res;

  res = Util::demangle("__ZdlPv");
  EXPECT_EQ(res, "operator delete(void*)") << res;

  res = Util::demangle("__Znam");
  EXPECT_EQ(res, "operator new[](unsigned long)") << res;
}

TEST(Util, convertAddress)
{
  // Ignore all white space.
  bool ok = false;
  auto addr = Util::convertAddress("     1\n2\t3\r  ", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, 123);

  // Hexadecimal.
  addr = Util::convertAddress("0xFACE", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, 0xFACE);

  // Detect hexadecimal even without "0x" in front.
  addr = Util::convertAddress("FACE", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, 0xFACE);

  // Octal.
  addr = Util::convertAddress("0777", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, 0777);

  // Decimal.
  addr = Util::convertAddress("9886", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, 9886);

  // Invalid text input.
  addr = Util::convertAddress("FACE of testing", &ok);
  ASSERT_FALSE(ok);
  EXPECT_EQ(addr, 0);

  addr = Util::convertAddress("Hello, World!", &ok);
  ASSERT_FALSE(ok);
  EXPECT_EQ(addr, 0);

  // Zero is zero!
  addr = Util::convertAddress("0", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, 0);
}

TEST(Util, moveTo)
{
  auto count = 10;
  auto value = 42;
  std::vector<decltype(value)> v1(count, value);
  const auto v1Copy = v1;
  decltype(v1) v2;
  Util::moveTo(v1, v2);
  EXPECT_TRUE(v1.empty());
  EXPECT_EQ(v2, v1Copy);
}

TEST(Util, copyTo)
{
  auto count = 10;
  auto value = 42;
  std::vector<decltype(value)> v1(count, value);
  decltype(v1) v2;
  Util::copyTo(v1, v2);
  EXPECT_EQ(v2, v1);
}

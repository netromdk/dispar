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

#include "gtest/gtest.h"

#include "Version.h"
using namespace dispar;

TEST(Version, versionString)
{
  EXPECT_EQ("0.2", versionString());
  EXPECT_EQ("0.2 [November 23, 2017]", versionString(true));
  EXPECT_EQ("9.3", versionString(9, 3));
  EXPECT_EQ("3.7 [November 23, 2017]", versionString(3, 7, true));
}

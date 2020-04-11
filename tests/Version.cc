#include "gtest/gtest.h"

#include "Version.h"
using namespace dispar;

TEST(Version, versionString)
{
  EXPECT_EQ("0.3", versionString());
  EXPECT_EQ("0.3 [April 11, 2020]", versionString(true));
  EXPECT_EQ("9.3", versionString(9, 3));
  EXPECT_EQ("3.7 [April 11, 2020]", versionString(3, 7, true));
}

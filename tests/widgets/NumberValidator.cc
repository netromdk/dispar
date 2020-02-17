#include "gtest/gtest.h"

#include "testutils.h"

#include "widgets/NumberValidator.h"
using namespace dispar;

TEST(NumberValidator, emptyIsIntermediate)
{
  NumberValidator v(10);
  QString input;
  int pos = 0;
  EXPECT_EQ(QValidator::Intermediate, v.validate(input, pos));
}

TEST(NumberValidator, cursorPosIsIrrelevant)
{
  NumberValidator v(10);
  QString input;
  for (int pos = 0; pos < 100; ++pos) {
    EXPECT_EQ(QValidator::Intermediate, v.validate(input, pos));
  }
}

TEST(NumberValidator, decBase)
{
  NumberValidator v(10);
  QString input("0123456789");
  int pos = 0;
  EXPECT_EQ(QValidator::Acceptable, v.validate(input, pos));

  input = "test";
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));

  input = "abcdef";
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));

  input = "()[]{}/\\&%€#$_*-+=";
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));
}

TEST(NumberValidator, octBase)
{
  NumberValidator v(8);
  QString input("01234567");
  int pos = 0;
  EXPECT_EQ(QValidator::Acceptable, v.validate(input, pos));

  input = "test";
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));

  input = "abcdef";
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));

  input = "()[]{}/\\&%€#$_*-+=";
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));
}

TEST(NumberValidator, hexBase)
{
  NumberValidator v(16);
  QString input("0123456789ABCDEF");
  int pos = 0;
  EXPECT_EQ(QValidator::Acceptable, v.validate(input, pos));

  input = "test";
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));

  input = "()[]{}/\\&%€#$_*-+=";
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));
}

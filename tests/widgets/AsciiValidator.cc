#include "gtest/gtest.h"

#include "testutils.h"

#include "widgets/AsciiValidator.h"
using namespace dispar;

TEST(AsciiValidator, emptyIsIntermediate)
{
  AsciiValidator v;
  QString input;
  int pos = 0;
  EXPECT_EQ(QValidator::Intermediate, v.validate(input, pos));
}

TEST(AsciiValidator, cursorPosIsIrrelevant)
{
  AsciiValidator v;
  QString input;
  for (int pos = 0; pos < 100; ++pos) {
    EXPECT_EQ(QValidator::Intermediate, v.validate(input, pos));
  }
}

TEST(AsciiValidator, correctRange)
{
  AsciiValidator v;
  int pos = 0;
  for (int ch = 32; ch < 127; ++ch) {
    QString input((QChar(ch)));
    EXPECT_EQ(QValidator::Acceptable, v.validate(input, pos)) << ch;
  }
}

TEST(AsciiValidator, whiteSpacesAreNotPrintable)
{
  AsciiValidator v;
  int pos = 0;

  // Space is printable, though, but has value 0x20.
  for (int ch = 9; ch < 14; ++ch) {
    QString input((QChar(ch)));
    EXPECT_EQ(QValidator::Invalid, v.validate(input, pos)) << ch;
  }
}

TEST(AsciiValidator, controlCodesAreNotPrintable)
{
  AsciiValidator v;
  int pos = 0;

  for (int ch = 0; ch < 9; ++ch) {
    QString input((QChar(ch)));
    EXPECT_EQ(QValidator::Invalid, v.validate(input, pos)) << ch;
  }
  for (int ch = 14; ch < 32; ++ch) {
    QString input((QChar(ch)));
    EXPECT_EQ(QValidator::Invalid, v.validate(input, pos)) << ch;
  }
}

TEST(AsciiValidator, backspaceIsNotPrintable)
{
  AsciiValidator v;
  QString input("\x7F"); // 127
  int pos = 0;
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));
}

TEST(AsciiValidator, multipleChars)
{
  AsciiValidator v;
  int pos = 0;
  QString input("abcd");
  EXPECT_EQ(QValidator::Acceptable, v.validate(input, pos));

  input = "ab";
  input.append(0x10);
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));

  input = "ab";
  input.append(0x10);
  input.append("cd");
  input.append(0x03);
  EXPECT_EQ(QValidator::Invalid, v.validate(input, pos));
}

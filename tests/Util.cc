#include "gtest/gtest.h"

#include "testutils.h"

#include <QTemporaryDir>

#include <vector>

#include "Util.h"
using namespace dispar;

TEST(Util, formatSize)
{
  auto str = Util::formatSize(0);
  EXPECT_EQ("0.0 B", str) << str;
  str = Util::formatSize(0, 0);
  EXPECT_EQ("0 B", str) << str;
  str = Util::formatSize(0, 3);
  EXPECT_EQ("0.000 B", str) << str;

  str = Util::formatSize(1234);
  EXPECT_EQ("1.2 KB", str) << str;
  str = Util::formatSize(1234, 8);
  EXPECT_EQ("1.20507813 KB", str) << str;

  str = Util::formatSize(1234567, 2);
  EXPECT_EQ("1.18 MB", str) << str;

  str = Util::formatSize(1234567890, 2);
  EXPECT_EQ("1.15 GB", str) << str;

  str = Util::formatSize(1234567890123, 2);
  EXPECT_EQ("1.12 TB", str) << str;

  str = Util::formatSize(1234567890123456, 2);
  EXPECT_EQ("1.10 PB", str) << str;

  str = Util::formatSize(1234567890123456789, 2);
  EXPECT_EQ("1.07 EB", str) << str;
}

TEST(Util, padString)
{
  EXPECT_EQ(Util::padString("x", 3, true, ' '), "  x");
  EXPECT_EQ(Util::padString("x", 3, false, ' '), "x  ");

  // Input is returned if size is smaller than or equal to input.
  EXPECT_EQ(Util::padString("abc", 1, false, ' '), "abc");
  EXPECT_EQ(Util::padString("abc", 3, false, ' '), "abc");
}

TEST(Util, dataToAscii)
{
  auto ascii = Util::dataToAscii("\x30\x31\x32\x61\x62\x63", 0, 6);
  EXPECT_EQ("012abc", ascii) << ascii;

  ascii = Util::dataToAscii("\x30\x31\x32\x61\x62\x63", 2, 3);
  EXPECT_EQ("2ab", ascii) << ascii;

  // Unprintable bytes are shown as ".".
  ascii = Util::dataToAscii("\x01\x02\x03", 0, 3);
  EXPECT_EQ("...", ascii) << ascii;
}

// Demangling doesn't work on Windows right now..
#ifndef WIN32
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

  // Empty string yields empty string.
  res = Util::demangle("");
  EXPECT_EQ(QString(), res) << res;

  // Skip leading: . $
  res = Util::demangle("$__Znam");
  EXPECT_EQ("operator new[](unsigned long)", res) << res;
  res = Util::demangle(".__Znam");
  EXPECT_EQ("operator new[](unsigned long)", res) << res;
}
#endif

TEST(Util, convertAddress)
{
  // Ignore all white space.
  bool ok = false;
  auto addr = Util::convertAddress("     1\n2\t3\r  ", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, static_cast<quint64>(123));

  // Hexadecimal.
  addr = Util::convertAddress("0xFACE", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, static_cast<quint64>(0xFACE));

  // Detect hexadecimal even without "0x" in front.
  addr = Util::convertAddress("FACE", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, static_cast<quint64>(0xFACE));

  // Octal.
  addr = Util::convertAddress("0777", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, static_cast<quint64>(0777));

  // Decimal.
  addr = Util::convertAddress("9886", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, static_cast<quint64>(9886));

  // Invalid text input.
  addr = Util::convertAddress("FACE of testing", &ok);
  ASSERT_FALSE(ok);
  EXPECT_EQ(addr, static_cast<quint64>(0));

  addr = Util::convertAddress("Hello, World!", &ok);
  ASSERT_FALSE(ok);
  EXPECT_EQ(addr, static_cast<quint64>(0));

  // Zero is zero!
  addr = Util::convertAddress("0", &ok);
  ASSERT_TRUE(ok);
  EXPECT_EQ(addr, static_cast<quint64>(0));

  // Invalid text and no 'ok' given triggers rejection of false positives when input wasn't "0".
  addr = Util::convertAddress("P");
  EXPECT_EQ(addr, static_cast<quint64>(0));
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

TEST(Util, serializeByteArray)
{
  const QByteArray array("hello\nworld\t!");
  const auto string = Util::byteArrayString(array);
  const auto array2 = Util::byteArray(string);
  EXPECT_EQ(array, array2);
}

TEST(Util, escapeWhitespace)
{
  EXPECT_EQ(Util::escapeWhitespace("hello\nthere\rhello\fworld!\v"),
            "hello\\nthere\\rhello\\fworld!\\v");
}

TEST(Util, hexToAscii)
{
  EXPECT_EQ(Util::hexToAscii("P", 0, 1), ""); // Invalid hex.

  // If not in range [32; 126] then yield ".".
  EXPECT_EQ(Util::hexToAscii("1F", 0, 1), "."); // 31
  EXPECT_EQ(Util::hexToAscii("20", 0, 1), " "); // 32
  EXPECT_EQ(Util::hexToAscii("7E", 0, 1), "~"); // 126
  EXPECT_EQ(Util::hexToAscii("7F", 0, 1), "."); // 127

  EXPECT_EQ(Util::hexToAscii("61 62 63", 0, 3), "abc");
  EXPECT_EQ(Util::hexToAscii("616263", 0, 3), "abc");

  EXPECT_EQ(Util::hexToAscii("31 32 33 34 35 36", 0, 6), "123456");
  EXPECT_EQ(Util::hexToAscii("313233343536", 0, 6), "123456");

  // Offset
  EXPECT_EQ(Util::hexToAscii("31 32", 3, 1), "2"); // 0x32 = '2'
  EXPECT_EQ(Util::hexToAscii("31 32", 4, 1), "."); // 0x2 = '.' (<32)
}

TEST(Util, hexToUnicode)
{
  EXPECT_EQ(Util::hexToUnicode("20AC"), "€");
  EXPECT_EQ(Util::hexToUnicode("24 20AC 25"), "$€%");
  EXPECT_EQ(Util::hexToUnicode("2420AC25"), "$ ¬%");
  EXPECT_EQ(Util::hexToUnicode("24AC 25"), "⒬%");
}

TEST(Util, decodeMacSdkVersion)
{
  EXPECT_EQ(Util::decodeMacSdkVersion(0xA0900), std::make_tuple(10, 9));
  EXPECT_EQ(Util::decodeMacSdkVersion(0xB0900), std::make_tuple(11, 9));
  EXPECT_EQ(Util::decodeMacSdkVersion(0xA0C00), std::make_tuple(10, 12));
}

TEST(Util, encodeMacSdkVersion)
{
  EXPECT_EQ(Util::encodeMacSdkVersion(std::make_tuple(10, 9)), quint32(0xA0900));
  EXPECT_EQ(Util::encodeMacSdkVersion(std::make_tuple(11, 9)), quint32(0xB0900));
  EXPECT_EQ(Util::encodeMacSdkVersion(std::make_tuple(10, 12)), quint32(0xA0C00));
}

TEST(Util, longToData)
{
  auto data = Util::longToData(0xA0C00);
  std::vector<char> v(data.begin(), data.end());
  EXPECT_EQ(v, std::vector<char>({0x00, 0x0C, 0x0A, 0x00}));

  data = Util::longToData(0xB0700);
  v = std::vector<char>(data.begin(), data.end());
  EXPECT_EQ(v, std::vector<char>({0x00, 0x07, 0x0B, 0x00}));
}

TEST(Util, addrDataString)
{
  auto res = Util::addrDataString(0, {});
  EXPECT_EQ(res, "0: ") << res;

  res = Util::addrDataString(0, QByteArray(3, 'x'));
  EXPECT_EQ(res, "0: 78 78 78 00 00 00 00 00 00 00 00 00 00 00 00 00   xxx.............") << res;

  res = Util::addrDataString(0xabcdef, QByteArray(3, 'x'));
  EXPECT_EQ(res, "ABCDEF: 78 78 78 00 00 00 00 00 00 00 00 00 00 00 00 00   xxx.............")
    << res;

  res = Util::addrDataString(0xabc, QByteArray(45, '9'));
  EXPECT_EQ(res, R"***(ABC: 39 39 39 39 39 39 39 39 39 39 39 39 39 39 39 39   9999999999999999
ACC: 39 39 39 39 39 39 39 39 39 39 39 39 39 39 39 39   9999999999999999
ADC: 39 39 39 39 39 39 39 39 39 39 39 39 39 00 00 00   9999999999999...)***")
    << res;

  QByteArray arr;
  for (int i = 32; i < 32 * 3 - 7; ++i) {
    arr.append(char(i));
  }
  res = Util::addrDataString(0x1000, arr);
  EXPECT_EQ(res, R"***(1000: 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F   .!"#$%&'()*+,-./
1010: 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F   0123456789:;<=>?
1020: 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F   @ABCDEFGHIJKLMNO
1030: 50 51 52 53 54 55 56 57 58 00 00 00 00 00 00 00   PQRSTUVWX.......)***")
    << res;
}

TEST(Util, resolveAppBinary)
{
  // Returns input on any kind of failure.

  // Exists but isn't an .app bundle.
  EXPECT_EQ(":macho_main", Util::resolveAppBinary(":macho_main"));

  // Doesn't exist.
  EXPECT_EQ("thisprogramdoesnotexist", Util::resolveAppBinary("thisprogramdoesnotexist"));

  QTemporaryDir tempDir(QDir::tempPath() + "/XXXXXX.app");
  ASSERT_TRUE(tempDir.isValid());

  // Ignore trailing slash.
  EXPECT_EQ(tempDir.path(), Util::resolveAppBinary(tempDir.path() + "/"));

  // Not complete bundle yet.
  EXPECT_EQ(tempDir.path(), Util::resolveAppBinary(tempDir.path()));

  QDir dir(tempDir.path());
  ASSERT_TRUE(dir.mkpath("Contents"));

  // Not complete bundle yet.
  EXPECT_EQ(tempDir.path(), Util::resolveAppBinary(tempDir.path()));

  ASSERT_TRUE(dir.mkpath("Contents/MacOS"));

  // Not complete bundle yet.
  EXPECT_EQ(tempDir.path(), Util::resolveAppBinary(tempDir.path()));

  ASSERT_TRUE(dir.cd("Contents"));

  QFile plist(dir.absoluteFilePath("Info.plist"));
  ASSERT_TRUE(plist.open(QIODevice::WriteOnly));
  plist.write(R"***(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleExecutable</key>
  <string>binaryname</string>
</dict>
</plist>
)***");
  plist.close();

  // Not complete bundle yet.
  EXPECT_EQ(tempDir.path(), Util::resolveAppBinary(tempDir.path()));

  ASSERT_TRUE(dir.cd("MacOS"));
  const auto binaryName = dir.absoluteFilePath("binaryname");
  QFile binary(binaryName);
  ASSERT_TRUE(binary.open(QIODevice::WriteOnly));

  // Now it works!
  EXPECT_EQ(binaryName, Util::resolveAppBinary(tempDir.path()));
}

TEST(Util, hexToData)
{
  auto data = QString::fromUtf8(Util::hexToData("0123456789abcdef"));
  EXPECT_EQ(QByteArray("\x01\x23\x45\x67\x89\xAB\xCD\xEF"), data) << data;

  // Not hexadecimal.
  data = QString::fromUtf8(Util::hexToData("PQZ"));
  EXPECT_EQ(QByteArray(), data) << data;
}

TEST(Util, hexToString)
{
  auto data = Util::hexToString("0123456789abcdef");
  EXPECT_EQ(QByteArray("\x01\x23\x45\x67\x89\xAB\xCD\xEF"), data) << data;

  // Not hexadecimal.
  data = Util::hexToString("PQZ");
  EXPECT_EQ(QByteArray(), data) << data;
}

TEST(Util, bytesToHex)
{
  const auto *input = (unsigned char*) "\x01\x23\x45";
  auto data = Util::bytesToHex(input, 1);
  EXPECT_EQ(QString("01"), data) << data;
  data = Util::bytesToHex(input, 2);
  EXPECT_EQ(QString("01 23"), data) << data;
  data = Util::bytesToHex(input, 3);
  EXPECT_EQ(QString("01 23 45"), data) << data;
}

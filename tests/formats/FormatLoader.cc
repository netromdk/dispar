#include <QSignalSpy>

#include "gtest/gtest.h"

#include "testutils.h"

#include "formats/FormatLoader.h"
using namespace dispar;

TEST(FormatLoader, failedNonexistent)
{
  FormatLoader loader("_/something/that/does/not/exist");

  QSignalSpy failedSpy(&loader, &FormatLoader::failed);
  QSignalSpy successSpy(&loader, &FormatLoader::success);

  loader.start();

  // Wait for thread to complete such that proper cleanup is done. AND
  // it also means TSAN doesn't show a lot of errors.
  loader.wait();

  EXPECT_EQ(1, failedSpy.count());
  EXPECT_EQ(0, successSpy.count());
}

TEST(FormatLoader, failedCouldNotDetect)
{
  auto file = tempFile();
  FormatLoader loader(file->fileName());

  QSignalSpy failedSpy(&loader, &FormatLoader::failed);
  QSignalSpy successSpy(&loader, &FormatLoader::success);

  loader.start();
  loader.wait();

  EXPECT_EQ(1, failedSpy.count());
  EXPECT_EQ(0, successSpy.count());
}

TEST(FormatLoader, failedCouldNotParse)
{
  // This means having a correct magic code but invalid binary otherwise.
  // Put MachO magic code: 0xFEEDFACE

  QByteArray data;
  data.append(static_cast<char>(0xCE));
  data.append(static_cast<char>(0xFA));
  data.append(static_cast<char>(0xED));
  data.append(static_cast<char>(0xFE));

  auto file = tempFile(data);
  FormatLoader loader(file->fileName());

  QSignalSpy failedSpy(&loader, &FormatLoader::failed);
  QSignalSpy successSpy(&loader, &FormatLoader::success);

  loader.start();
  loader.wait();

  EXPECT_EQ(1, failedSpy.count());
  EXPECT_EQ(0, successSpy.count());
}

TEST(FormatLoader, success)
{
  FormatLoader loader(":macho_main");

  QSignalSpy failedSpy(&loader, &FormatLoader::failed);
  QSignalSpy successSpy(&loader, &FormatLoader::success);
  QSignalSpy statusSpy(&loader, &FormatLoader::status);
  QSignalSpy progressSpy(&loader, &FormatLoader::progress);

  loader.start();
  loader.wait();

  EXPECT_EQ(1, successSpy.count());
  EXPECT_EQ(0, failedSpy.count());

  // "Detected..." + "Success"
  EXPECT_EQ(2, statusSpy.count());

  // 0.5 + 1
  ASSERT_EQ(2, progressSpy.count());
  EXPECT_FLOAT_EQ(0.5f, progressSpy[0][0].toFloat());
  EXPECT_FLOAT_EQ(1.0f, progressSpy[1][0].toFloat());
}

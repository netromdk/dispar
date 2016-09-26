#include "gtest/gtest.h"

#include "testutils.h"

#include "formats/FormatLoader.h"

TEST(FormatLoader, failedNonexistent)
{
  FormatLoader loader("_/something/that/does/not/exist");

  auto failedSpy = SIGNAL_SPY_ONE(const QString &, &loader, &FormatLoader::failed);

  auto successSpy = SIGNAL_SPY_ONE(std::shared_ptr<Format>, &loader, &FormatLoader::success);
  successSpy->setExpect(false);

  loader.start();
  failedSpy->wait();
}

TEST(FormatLoader, failedCouldNotDetect)
{
  auto file = tempFile();
  FormatLoader loader(file->fileName());

  auto failedSpy = SIGNAL_SPY_ONE(const QString &, &loader, &FormatLoader::failed);

  auto successSpy = SIGNAL_SPY_ONE(std::shared_ptr<Format>, &loader, &FormatLoader::success);
  successSpy->setExpect(false);

  loader.start();
  failedSpy->wait();
}

TEST(FormatLoader, failedCouldNotParse)
{
  // This means having a correct magic code but invalid binary otherwise.
  // Put MachO magic code: 0xFEEDFACE

  QByteArray data;
  data.append(0xCE);
  data.append(0xFA);
  data.append(0xED);
  data.append(0xFE);

  auto file = tempFile(data);
  FormatLoader loader(file->fileName());

  auto failedSpy = SIGNAL_SPY_ONE(const QString &, &loader, &FormatLoader::failed);

  auto successSpy = SIGNAL_SPY_ONE(std::shared_ptr<Format>, &loader, &FormatLoader::success);
  successSpy->setExpect(false);

  loader.start();
  failedSpy->wait();
}

TEST(FormatLoader, success)
{
  FormatLoader loader(":macho_main");

  auto statusSpy = SIGNAL_SPY_ONE(const QString &, &loader, &FormatLoader::status);

  auto progressSpy =
    SIGNAL_SPY_ONE_FUNC(float, &loader, &FormatLoader::progress, [](float progress) {
      static int cnt = 0;
      cnt++;
      if (cnt == 1) {
        EXPECT_FLOAT_EQ(progress, 0.33);
      }
      else if (cnt == 2) {
        //EXPECT_FLOAT_EQ(progress, 0.66);
        EXPECT_FLOAT_EQ(progress, 1);
      }
      else if (cnt == 3) {
        EXPECT_FLOAT_EQ(progress, 1);
      }
    });

  auto successSpy = SIGNAL_SPY_ONE(std::shared_ptr<Format>, &loader, &FormatLoader::success);

  loader.start();

  statusSpy->wait();
  progressSpy->wait();
  successSpy->wait();
}

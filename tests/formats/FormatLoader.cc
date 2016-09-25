#include "gtest/gtest.h"

#include "testutils.h"

#include "formats/FormatLoader.h"

// TODO: create general create function with variadic templates instead of one(), two()..

TEST(FormatLoader, failedNonexistent)
{
  FormatLoader loader("_/something/that/does/not/exist");

  auto failedSpy = SIGNAL_SPY_ONE(const QString &, &loader, &FormatLoader::failed);

  auto successSpy = SIGNAL_SPY_ONE(std::shared_ptr<Format>, &loader, &FormatLoader::success);
  successSpy->setExpect(false);

  loader.start();
  failedSpy->wait();
}

TEST(FormatLoader, failedCouldNotParse)
{
  auto file = tempFile();
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
        EXPECT_FLOAT_EQ(progress, 0.66);
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

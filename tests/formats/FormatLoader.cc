#include "gtest/gtest.h"

#include "testutils.h"

#include "formats/FormatLoader.h"

TEST(FormatLoader, load)
{
  FormatLoader loader(":macho_main");

  auto statusSpy =
    SignalSpy::one<const QString &>(__FILE__, __LINE__, &loader, &FormatLoader::status);

  auto progressSpy =
    SignalSpy::one<float>(__FILE__, __LINE__, &loader, &FormatLoader::progress, [](float progress) {
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

  auto successSpy =
    SignalSpy::one<std::shared_ptr<Format>>(__FILE__, __LINE__, &loader, &FormatLoader::success);

  loader.start();

  statusSpy->wait();
  progressSpy->wait();
  successSpy->wait();
}

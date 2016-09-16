#include "gtest/gtest.h"

#include <QTimer>
#include <QCoreApplication>

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  Q_INIT_RESOURCE(binaries);
  testing::InitGoogleTest(&argc, argv);
  QTimer::singleShot(1, [&app] { app.exit(RUN_ALL_TESTS()); });
  auto ret = app.exec();
  Q_CLEANUP_RESOURCE(binaries);
  return ret;
}

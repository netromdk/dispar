#include <QApplication>
#include <QStringList>
#include <QTimer>

#include "Version.h"
#include "formats/Format.h"
#include "widgets/MainWindow.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  app.setApplicationName("Dispar");
  app.setApplicationVersion(versionString());

  Format::registerType();

  QString file;
  if (argc == 2) {
    file = QString::fromUtf8(argv[1]);
  }

  // Start in event loop.
  MainWindow main(file);
  QTimer::singleShot(0, &main, SLOT(show()));

  return app.exec();
}

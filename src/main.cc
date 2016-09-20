#include <QApplication>
#include <QTimer>
#include <QStringList>

#include "Version.h"
#include "widgets/MainWindow.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  app.setApplicationName("Dispar");
  app.setApplicationVersion(versionString());

  QString file;
  if (argc == 2) {
    file = QString::fromUtf8(argv[1]);
  }

  // Start in event loop.
  MainWindow main(file);
  QTimer::singleShot(0, &main, SLOT(show()));

  return app.exec();
}

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

  QStringList files;
  for (int i = 1; i < argc; i++) {
    files << QString::fromUtf8(argv[i]);
  }

  // Start in event loop.
  MainWindow main(files);
  QTimer::singleShot(0, &main, SLOT(show()));

  return app.exec();
}

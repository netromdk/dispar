#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QTimer>

#include <signal.h>

#include "Context.h"
#include "Version.h"
#include "formats/Format.h"
#include "widgets/MainWindow.h"

void signalHandler(int sig)
{
  qDebug().nospace() << "Caught software signal " << sig << ". Cleaning up and closing down.";
  QApplication::exit(0);
}

int main(int argc, char **argv)
{
  // Reset library paths to look for Qt plugins in the binary folder and not any system paths. But
  // only when plugins are found inside the binary folder because otherwise it's before install is
  // run.
  QDir curDir(QDir::current());
  QFileInfo fi(argv[0]);
  auto path = curDir.absoluteFilePath(fi.dir().absolutePath());
  if (QFile::exists(QDir(path).absoluteFilePath("platforms"))) {
    QCoreApplication::setLibraryPaths({path});
  }

  QApplication app(argc, argv);
  app.setApplicationName("Dispar");
  app.setOrganizationName("dispar");
  app.setApplicationVersion(versionString());

  // Register software signals.
  signal(SIGINT, &signalHandler);
  signal(SIGABRT, &signalHandler);
  signal(SIGTERM, &signalHandler);

  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", "Project .dispar or binary file to load.", "(file)");

  parser.process(app);
  auto posArgs = parser.positionalArguments();

  QString file;
  if (posArgs.size() == 1) {
    file = posArgs.first();
  }

  // Register meta types.
  Format::registerType();

  // Initialize and load context.
  Context::get();

  // Start in event loop.
  MainWindow main(file);
  QTimer::singleShot(0, &main, SLOT(show()));

  return app.exec();
}

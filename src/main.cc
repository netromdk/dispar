#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
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
  QApplication app(argc, argv);
  app.setApplicationName("Dispar");
  app.setApplicationVersion(versionString());

  // Register software signals.
  signal(SIGINT, &signalHandler);
  signal(SIGABRT, &signalHandler);
  signal(SIGTERM, &signalHandler);

  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("binary", "Binary file to load.", "(binary)");

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

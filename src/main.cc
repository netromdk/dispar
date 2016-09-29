#include <QApplication>
#include <QCommandLineParser>
#include <QStringList>
#include <QTimer>

#include "Context.h"
#include "Version.h"
#include "formats/Format.h"
#include "widgets/MainWindow.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  app.setApplicationName("Dispar");
  app.setApplicationVersion(versionString());

  Format::registerType();

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

  // Initialize and load context.
  Context::get();

  // Start in event loop.
  MainWindow main(file);
  QTimer::singleShot(0, &main, SLOT(show()));

  return app.exec();
}

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>

#include <algorithm>
#include <csignal>
#include <utility>

#include "BinaryObject.h"
#include "Context.h"
#include "MacSdkVersionPatcher.h"
#include "Util.h"
#include "Version.h"
#include "formats/Format.h"
#include "formats/FormatLoader.h"
#include "widgets/MainWindow.h"
using namespace dispar;

void signalHandler(int sig)
{
  qDebug().nospace() << "Caught software signal " << sig << ". Cleaning up and closing down.";
  QApplication::exit(0);
}

std::shared_ptr<Format> loadFile(const QString &fileName)
{
  FormatLoader loader(fileName);

  bool failed = false;
  QObject::connect(&loader, &FormatLoader::failed, [&](const QString &msg) {
    qCritical() << msg;
    failed = true;
  });

  QObject::connect(&loader, &FormatLoader::status,
                   [](const QString &msg) { qDebug() << qPrintable(msg); });

  std::shared_ptr<Format> res = nullptr;
  QObject::connect(&loader, &FormatLoader::success,
                   [&](std::shared_ptr<Format> fmt) { res = std::move(fmt); });

  loader.start();
  loader.wait();

  if (failed) return nullptr;
  return res;
}

bool saveFile(const std::shared_ptr<Format> &format)
{
  QFile f(format->file());
  if (!f.open(QIODevice::ReadWrite)) {
    qCritical() << "Could not open binary file for writing:" << format->file();
    return false;
  }
  format->write(f);
  return true;
}

int handleParse(const std::shared_ptr<Format> &format)
{
  qInfo() << "Binary parsed successfully!";
  qInfo().noquote() << format->toString();
  return 0;
}

int handlePatchSdk(const std::shared_ptr<Format> &format, const Section::Type type,
                   const QString &version)
{
  const bool list = (version == "list");

  MacSdkVersionPatcher::Version newTarget;
  if (!list) {
    QRegularExpression re(R"((\d+)\.(\d+))");
    const auto m = re.match(version);
    if (!m.hasMatch()) {
      qCritical() << "Invalid version:" << version;
      return 1;
    }

    bool ok, ok2;
    const auto major = m.captured(1).toInt(&ok);
    const auto minor = m.captured(2).toInt(&ok2);
    if (!ok || !ok2 || major < 0 || minor < 0) {
      qCritical() << "Invalid version:" << version;
      return 1;
    }

    newTarget = {major, minor};
  }

  const auto printSdks = [](const MacSdkVersionPatcher &patcher, const BinaryObject *object) {
    qInfo() << "[" << qPrintable(object->toString()) << "]";
    const auto target = patcher.target();
    qInfo().nospace() << "- Target SDK: " << std::get<0>(target) << "." << std::get<1>(target);
    const auto sdk = patcher.sdk();
    qInfo().nospace() << "- Source SDK: " << std::get<0>(sdk) << "." << std::get<1>(sdk);
  };

  bool modified = false;
  for (const auto *object : format->objects()) {
    auto *section = object->section(type);
    if (!section) {
      qInfo() << "[" << qPrintable(object->toString()) << "]";
      qWarning() << "Does not have section" << Section::typeName(type);
      continue;
    }

    MacSdkVersionPatcher patcher(*section);
    if (!patcher.valid()) {
      qWarning() << "Could not read SDK versions!";
      continue;
    }

    if (list) {
      printSdks(patcher, object);
    }
    else {
      modified |= patcher.setTarget(newTarget);
      if (modified) {
        printSdks(patcher, object);
      }
    }
  }

  if (list) return 0;

  // Save modified data to file.
  if (modified) {
    if (!saveFile(format)) {
      return 1;
    }
    qInfo() << "Modifications saved to file.";
  }
  else {
    qInfo() << "No modifications to save to file.";
  }
  return 0;
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

  QCommandLineOption versionOption("version", QObject::tr("Displays version information."));
  parser.addOption(versionOption);

  QCommandLineOption verboseOption({"v", "verbose"}, QObject::tr("Verbose mode."));
  parser.addOption(verboseOption);

  QCommandLineOption parseOption(
    "parse", QObject::tr("Parse binary, display information, and exit (headless)."));
  parser.addOption(parseOption);

  parser.addPositionalArgument("file", "Project .dispar or binary file to load.", "(file)");

  QCommandLineOption patchMacSdkOption(
    "patch-mac-sdk-version",
    QObject::tr(
      "Patch macOS SDK target version and exit (headless). The version must be on the "
      "format 'X.Y', like '10.14', or use 'list' to list the target and source SDK versions."),
    "version");
  parser.addOption(patchMacSdkOption);

  QCommandLineOption patchiOSSdkOption(
    "patch-ios-sdk-version", QObject::tr("Patch iOS SDK target version and exit (headless)."),
    "version");
  parser.addOption(patchiOSSdkOption);

  QCommandLineOption patchWatchOSSdkOption(
    "patch-watchos-sdk-version",
    QObject::tr("Patch watchOS SDK target version and exit (headless)."), "version");
  parser.addOption(patchWatchOSSdkOption);

  QCommandLineOption patchTvOSSdkOption(
    "patch-tvos-sdk-version", QObject::tr("Patch tvOS SDK target version and exit (headless)."),
    "version");
  parser.addOption(patchTvOSSdkOption);

  parser.process(app);
  auto posArgs = parser.positionalArguments();

  const bool patchMac = parser.isSet(patchMacSdkOption);
  const bool patchiOS = parser.isSet(patchiOSSdkOption);
  const bool patchWatchOS = parser.isSet(patchWatchOSSdkOption);
  const bool patchTvOS = parser.isSet(patchTvOSSdkOption);
  const bool patchSdk = patchMac | patchiOS | patchWatchOS | patchTvOS;

  const bool version = parser.isSet(versionOption);
  const bool verbose = parser.isSet(verboseOption);
  const bool parse = parser.isSet(parseOption);

  if (version) {
    qInfo().noquote().nospace() << qApp->applicationName() << " " << qApp->applicationVersion()
                                << " (" << __DATE__ << ")";
    return 0;
  }

  // Initialize and load context.
  Context context;
  context.setVerbose(verbose);
  context.init();

  QString file;
  if (posArgs.size() == 1) {
    file = Util::resolveAppBinary(posArgs.first());
  }

  const bool requiresFile = patchSdk || parse;

  std::shared_ptr<Format> format = nullptr;
  if (requiresFile) {
    qInfo() << "Loading" << file;
    if (file.isEmpty() || !QFile::exists(file)) {
      qCritical() << "Input binary file must be specified and exist!";
      return 1;
    }

    format = loadFile(file);
    if (!format) {
      qCritical() << "Could not load file:" << file;
      return 1;
    }
  }

  if (parse) {
    return handleParse(format);
  }

  if (patchSdk) {
    if (patchMac) {
      return handlePatchSdk(format, Section::Type::LC_VERSION_MIN_MACOSX,
                            parser.value(patchMacSdkOption));
    }
    else if (patchiOS) {
      return handlePatchSdk(format, Section::Type::LC_VERSION_MIN_IPHONEOS,
                            parser.value(patchiOSSdkOption));
    }
    else if (patchWatchOS) {
      return handlePatchSdk(format, Section::Type::LC_VERSION_MIN_WATCHOS,
                            parser.value(patchWatchOSSdkOption));
    }
    else if (patchTvOS) {
      return handlePatchSdk(format, Section::Type::LC_VERSION_MIN_TVOS,
                            parser.value(patchTvOSSdkOption));
    }
  }


  // Start in event loop.
  MainWindow main(file);
  QTimer::singleShot(0, &main, SLOT(show()));

  return app.exec();
}

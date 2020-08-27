#include "Debugger.h"
#include "cxx.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QProcess>
#include <QTimer>

namespace dispar {

Debugger::Debugger() = default;

Debugger::Debugger(const QString &program, const QString &versionArgument,
                   const QString &launchPattern)
  : program_(program), versionArgument_(versionArgument), launchPattern_(launchPattern)
{
}

QList<Debugger> Debugger::predefined()
{
  return {{"lldb", "--version", "-- {{BINARY}} {{ARGS}}"},
          {"gdb", "--version", "--args {{BINARY}} {{ARGS}}"},
          {"ggdb", "--version", "--args {{BINARY}} {{ARGS}}"}};
}

QList<Debugger> Debugger::detect()
{
  QList<Debugger> res;
  cxx::copy_if(predefined(), std::back_inserter(res),
               [](const auto &dbg) { return dbg.valid() && dbg.runnable(); });
  return res;
}

QString Debugger::program() const
{
  return program_;
}

QString Debugger::versionArgument() const
{
  return versionArgument_;
}

QString Debugger::launchPattern() const
{
  return launchPattern_;
}

QString Debugger::toString() const
{
  return QString(R"(Debugger[program = "%1", version = "%2", launch = "%3"])")
    .arg(program())
    .arg(versionArgument())
    .arg(launchPattern());
}

QString Debugger::substituteLaunchPattern(const QString &binary, const QStringList &args) const
{
  return launchPattern().replace("{{BINARY}}", binary).replace("{{ARGS}}", args.join(" "));
}

bool Debugger::valid() const
{
  return !program().isEmpty() && !versionArgument().isEmpty() && !launchPattern().isEmpty();
}

bool Debugger::runnable(int timeout) const
{
  QProcess proc;

  QEventLoop loop;
  QObject::connect(&proc, cxx::Use<int, QProcess::ExitStatus>::overloadOf(&QProcess::finished),
                   &loop, &QEventLoop::quit);

  bool timedOut = false;
  QTimer::singleShot(timeout, &loop, [&] {
    timedOut = true;
    loop.quit();
  });

  proc.start(program(), {versionArgument()});
  loop.exec();

  return !timedOut && proc.exitCode() == 0 && proc.exitStatus() == QProcess::NormalExit;
}

bool Debugger::detachStart(const QString &binary, const QStringList &args) const
{
  if (!QFile::exists(binary)) {
    qWarning() << "Binary doesn't exist:" << binary;
    return false;
  }

  // Create script file to execute binary in debugger.
  QFile scriptFile(QDir::temp().absoluteFilePath("dbgexec.sh"));
  scriptFile.remove();
  if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning() << "Could not open file for writing:" << scriptFile.fileName();
    return false;
  }

  const auto script = QString("%1 %2").arg(program()).arg(substituteLaunchPattern(binary, args));
  scriptFile.write(script.toUtf8());
  scriptFile.close();

  scriptFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::ExeOwner | QFileDevice::ReadUser |
                            QFileDevice::ExeUser);

  // TODO: Choose terminal launcher based on OS
  return QProcess::startDetached("open", {"-a", "Terminal.app", scriptFile.fileName()});
}

bool Debugger::operator==(const Debugger &rhs) const
{
  return program() == rhs.program() && versionArgument() == rhs.versionArgument() &&
         launchPattern() == rhs.launchPattern();
}

bool Debugger::operator!=(const Debugger &rhs) const
{
  return !(*this == rhs);
}

} // namespace dispar

QDebug operator<<(QDebug dbg, const dispar::Debugger &debugger)
{
  dbg << debugger.toString();
  return dbg;
}

#ifndef SRC_DEBUGGER_H
#define SRC_DEBUGGER_H

#include "Constants.h"

#include <QDebug>
#include <QList>
#include <QStringList>

namespace dispar {

/// Encapsulates detecting and running a binary inside a debugger.
class Debugger {
public:
  /// Create null-value instance.
  Debugger();

  /// Create debugger instance.
  Debugger(const QString &program, const QString &versionArgument, const QString &launchPattern);

  /// List of predefined debuggers.
  /** Call \p runnable() to find out which ones are runnable on the system. */
  static QList<Debugger> predefined();

  /// Detect known debuggers on the system.
  static QList<Debugger> detect();

  [[nodiscard]] QString program() const;
  [[nodiscard]] QString versionArgument() const;
  [[nodiscard]] QString launchPattern() const;

  [[nodiscard]] QString toString() const;

  /// Substitutes launch pattern with \p binary and optional \p args.
  [[nodiscard]] QString substituteLaunchPattern(const QString &binary,
                                                const QStringList &args = {}) const;

  /// Checks if debugger values are valid.
  [[nodiscard]] bool valid() const;

  /// Checks if the debugger is runnable on the system.
  /** It will \p timeout in milliseconds if program doesn't exit before that. */
  [[nodiscard]] bool runnable(int timeout = Constants::Debugger::runnableTimeout) const;

  /// Start debugger detached to execute \p binary with optional \p args.
  /** Returns true if the debugger was started successfully. */
  [[nodiscard]] bool detachStart(const QString &binary, const QStringList &args = {}) const;

  bool operator==(const Debugger &rhs) const;
  bool operator!=(const Debugger &rhs) const;

private:
  QString program_, versionArgument_, launchPattern_;
};

} // namespace dispar

QDebug operator<<(QDebug dbg, const dispar::Debugger &debugger);

#endif // SRC_DEBUGGER_H

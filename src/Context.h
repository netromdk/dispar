#ifndef DISPAR_CONTEXT_H
#define DISPAR_CONTEXT_H

#include <QHash>
#include <QObject>
#include <QVariantHash>

#include <memory>

#include "Constants.h"
#include "Debugger.h"
#include "Disassembler.h"
#include "LogHandler.h"

namespace dispar {

class Project;

/// It is required to create an instance once in the beginning of the program.
/** Since only one instance can be live at any one time, it is asserted that more aren't tried
    created. */
class Context : public QObject {
  Q_OBJECT

public:
  Context();
  ~Context() override;

  /// Singleton instance.
  static Context &get();

  /// Initializes context.
  void init();

  void setVerbose(bool verbose);
  [[nodiscard]] bool verbose() const;

  [[nodiscard]] bool showMachineCode() const;
  void setShowMachineCode(bool show);

  [[nodiscard]] Disassembler::Syntax disassemblerSyntax() const;
  void setDisassemblerSyntax(Disassembler::Syntax syntax);

  [[nodiscard]] bool backupEnabled() const;
  void setBackupEnabled(bool enabled);

  [[nodiscard]] int backupAmount() const;
  void setBackupAmount(int amount);

  const QStringList &recentProjects();
  void addRecentProject(const QString &project);

  const QStringList &recentBinaries();
  void addRecentBinary(const QString &binary);

  void setValue(const QString &key, const QVariant &value);
  [[nodiscard]] QVariant value(const QString &key, const QVariant &defaultValue = {}) const;

  void setDebugger(const Debugger &debugger);
  [[nodiscard]] Debugger debugger() const;

  void loadSettings();
  void saveSettings();

  /// Keeps ownership.
  [[nodiscard]] Project *project() const;

  /// Reset to empty project state and returns newly created instance.
  /** Keeps ownership. */
  Project *resetProject();

  void clearProject();

  /// Tries to load a project from \p file.
  /** Keeps ownership. */
  Project *loadProject(const QString &file);

  [[nodiscard]] LogHandler *logHandler() const;
  [[nodiscard]] int logLevel() const;
  void setLogLevel(int level);
  [[nodiscard]] bool acceptMsgType(QtMsgType type) const;

  [[nodiscard]] int omniSearchLimit() const;
  void setOmniSearchLimit(int limit);

signals:
  void showMachineCodeChanged(bool show);
  void logLevelChanged(int newLevel);

private:
  bool verbose_ = false;

  bool showMachineCode_;
  Disassembler::Syntax disassemblerSyntax_;

  bool backupEnabled_;
  int backupAmount_;

  QStringList recentProjects_, recentBinaries_;
  QVariantHash values;

  Debugger debugger_;

  int logLevel_ = Constants::Log::DEFAULT_LEVEL;
  int omniSearchLimit_ = Constants::Omni::DEFAULT_LIMIT;

  std::unique_ptr<Project> project_;
  std::unique_ptr<LogHandler> logHandler_;
};

} // namespace dispar

#endif // DISPAR_CONTEXT_H

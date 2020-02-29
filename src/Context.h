#ifndef DISPAR_CONTEXT_H
#define DISPAR_CONTEXT_H

#include <QHash>
#include <QObject>
#include <QVariantHash>

#include <memory>

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
  ~Context();

  /// Singleton instance.
  static Context &get();

  void setVerbose(bool verbose);
  bool verbose() const;

  bool showMachineCode() const;
  void setShowMachineCode(bool show);

  Disassembler::Syntax disassemblerSyntax() const;
  void setDisassemblerSyntax(Disassembler::Syntax syntax);

  bool backupEnabled() const;
  void setBackupEnabled(bool enabled);

  int backupAmount() const;
  void setBackupAmount(int amount);

  const QStringList &recentProjects();
  void addRecentProject(const QString &project);

  const QStringList &recentBinaries();
  void addRecentBinary(const QString &binary);

  void setValue(const QString &key, const QVariant &value);
  QVariant value(const QString &key, const QVariant &defaultValue = {}) const;

  void setDebugger(const Debugger &debugger);
  Debugger debugger() const;

  void loadSettings();
  void saveSettings();

  /// Keeps ownership.
  Project *project() const;

  /// Reset to empty project state and returns newly created instance.
  /** Keeps ownership. */
  Project *resetProject();

  void clearProject();

  /// Tries to load a project from \p file.
  /** Keeps ownership. */
  Project *loadProject(const QString &file);

signals:
  void showMachineCodeChanged(bool show);

private:
  bool verbose_ = false;

  bool showMachineCode_;
  Disassembler::Syntax disassemblerSyntax_;

  bool backupEnabled_;
  int backupAmount_;

  QStringList recentProjects_, recentBinaries_;
  QVariantHash values;

  Debugger debugger_;

  std::unique_ptr<Project> project_;
  std::unique_ptr<LogHandler> logHandler;
};

} // namespace dispar

#endif // DISPAR_CONTEXT_H

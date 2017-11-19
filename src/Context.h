#ifndef DISPAR_CONTEXT_H
#define DISPAR_CONTEXT_H

#include <QObject>

#include <memory>

#include "Disassembler.h"

class Project;

class Context : public QObject {
  Q_OBJECT

public:
  ~Context();

  /// Singleton instance.
  static Context &get();

  bool showMachineCode() const;
  void setShowMachineCode(bool show);

  Disassembler::Syntax disassemblerSyntax() const;
  void setDisassemblerSyntax(Disassembler::Syntax syntax);

  bool backupEnabled() const;
  void setBackupEnabled(bool enabled);

  int backupAmount() const;
  void setBackupAmount(int amount);

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
  Context();

  bool showMachineCode_;
  Disassembler::Syntax disassemblerSyntax_;

  bool backupEnabled_;
  int backupAmount_;

  std::unique_ptr<Project> project_;
};

#endif // DISPAR_CONTEXT_H

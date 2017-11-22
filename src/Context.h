#ifndef DISPAR_CONTEXT_H
#define DISPAR_CONTEXT_H

#include <QHash>
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

  void setGeometry(const QString &key, const QByteArray &geometry);
  QByteArray geometry(const QString &key) const;

  const QStringList &recentProjects();
  void addRecentProject(const QString &project);

  const QStringList &recentBinaries();
  void addRecentBinary(const QString &binary);

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

  QHash<QString, QByteArray> geometries;
  QStringList recentProjects_, recentBinaries_;

  std::unique_ptr<Project> project_;
};

#endif // DISPAR_CONTEXT_H

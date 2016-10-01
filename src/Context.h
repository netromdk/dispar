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

  std::shared_ptr<Project> project() const;

  /// Reset to empty project state and returns newly created instance.
  std::shared_ptr<Project> resetProject();

signals:
  void showMachineCodeChanged(bool show);

private:
  Context();

  bool showMachineCode_;
  Disassembler::Syntax disassemblerSyntax_;

  std::shared_ptr<Project> project_;
};

#endif // DISPAR_CONTEXT_H

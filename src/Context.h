#ifndef DISPAR_CONTEXT_H
#define DISPAR_CONTEXT_H

#include <QObject>

#include "Disassembler.h"

class Context : public QObject {
  Q_OBJECT

public:
  ~Context();

  // Singleton instance.
  static Context &get();

  bool showMachineCode() const;
  void setShowMachineCode(bool show);

  Disassembler::Syntax disassemblerSyntax() const;
  void setDisassemblerSyntax(Disassembler::Syntax syntax);

signals:
  void showMachineCodeChanged(bool show);

private:
  Context();

  bool showMachineCode_;
  Disassembler::Syntax disassemblerSyntax_;
};

#endif // DISPAR_CONTEXT_H

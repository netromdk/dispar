#ifndef DISPAR_DISASSEMBLER_DIALOG_H
#define DISPAR_DISASSEMBLER_DIALOG_H

#include <QDialog>

#include "CpuType.h"
#include "Disassembler.h"

class QTextEdit;
class QSplitter;
class QLineEdit;
class QComboBox;
class QPushButton;

namespace dispar {

class DisassemblerDialog : public QDialog {
  Q_OBJECT

public:
  DisassemblerDialog(QWidget *parent = nullptr, CpuType cpuType = CpuType::X86,
                     const QString &data = QString(), quint64 offset = 0,
                     Disassembler::Syntax syntax = Disassembler::Syntax::INTEL);

private slots:
  void onConvert();
  void onCpuTypeIndexChanged(int index);
  void onSyntaxIndexChanged(int index);

private:
  void createLayout();
  void setAsmVisible(bool visible = true);

  CpuType cpuType;
  quint64 offset;
  Disassembler::Syntax syntax;

  QTextEdit *machineText = nullptr, *asmText = nullptr;
  QSplitter *splitter = nullptr;
  QLineEdit *offsetEdit = nullptr;
  QComboBox *cpuTypeBox = nullptr, *syntaxBox = nullptr;
  QPushButton *convertBtn = nullptr;
};

} // namespace dispar

#endif // DISPAR_DISASSEMBLER_DIALOG_H

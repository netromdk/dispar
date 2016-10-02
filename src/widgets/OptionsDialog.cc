#include "OptionsDialog.h"
#include "../Context.h"
#include "../Disassembler.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent)
{
  setWindowTitle(tr("Options"));
  createLayout();
}

void OptionsDialog::onAccept()
{
  setWindowTitle(tr("Applying options!"));

  auto &ctx = Context::get();
  ctx.setShowMachineCode(showMachineCode->checkState() == Qt::Checked);

  auto syntax = static_cast<Disassembler::Syntax>(disAsmSyntax->currentData().toInt());
  static bool disWarnOnce = false;
  if (!disWarnOnce && syntax != ctx.disassemblerSyntax()) {
    disWarnOnce = true;
    QMessageBox::warning(
      this, "dispar", tr("Disassembler syntax won't change until the program has been restarted!"));
  }
  ctx.setDisassemblerSyntax(syntax);

  accept();
}

void OptionsDialog::createLayout()
{
  auto &ctx = Context::get();

  showMachineCode = new QCheckBox(tr("Show Machine Code"));
  showMachineCode->setChecked(ctx.showMachineCode());

  auto *disAsmLabel = new QLabel(tr("Disassembly Syntax:"));

  disAsmSyntax = new QComboBox;
  disAsmSyntax->addItem(tr("AT&T"), (int) Disassembler::Syntax::ATT);
  disAsmSyntax->addItem(tr("Intel"), (int) Disassembler::Syntax::INTEL);

  int idx = disAsmSyntax->findData((int) ctx.disassemblerSyntax());
  if (idx != -1) {
    disAsmSyntax->setCurrentIndex(idx);
  }

  auto *disAsmSyntaxLayout = new QHBoxLayout;
  disAsmSyntaxLayout->addWidget(disAsmLabel);
  disAsmSyntaxLayout->addWidget(disAsmSyntax);
  disAsmSyntaxLayout->addStretch();

  auto *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(showMachineCode);
  mainLayout->addLayout(disAsmSyntaxLayout);

  auto *mainGroup = new QGroupBox(tr("Main View"));
  mainGroup->setLayout(mainLayout);

  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::onAccept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto *layout = new QVBoxLayout;
  layout->addWidget(mainGroup);
  layout->addStretch();
  layout->addWidget(buttonBox);

  setLayout(layout);
}

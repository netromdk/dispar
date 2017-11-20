#include "widgets/OptionsDialog.h"
#include "Context.h"
#include "Disassembler.h"
#include "cxx.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
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

  ///// Main View

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

  ///// Binary Backups

  auto *backupLabel = new QLabel(tr("Backups are saved in the same folder as the originating "
                                    "binary file but with a post-fix of the form \".bakN\", where "
                                    "\"N\" is the backup number."));
  backupLabel->setWordWrap(true);

  auto *backupAmountLbl = new QLabel(tr("Number of copies to keep:"));

  auto *backupAmountInfo = new QLabel(tr("(Unlimited)"));
  backupAmountInfo->setVisible(ctx.backupAmount() == 0);

  auto *backupAmountSpin = new QSpinBox;
  backupAmountSpin->setRange(0, 1024);
  backupAmountSpin->setValue(ctx.backupAmount());
  connect(backupAmountSpin, cxx::Use<int>::overloadOf(&QSpinBox::valueChanged), this,
          [&ctx, backupAmountInfo](int amount) {
            ctx.setBackupAmount(amount);
            backupAmountInfo->setVisible(amount == 0);
          });

  auto *backupAmountLayout = new QHBoxLayout;
  backupAmountLayout->addWidget(backupAmountLbl);
  backupAmountLayout->addWidget(backupAmountSpin);
  backupAmountLayout->addWidget(backupAmountInfo);
  backupAmountLayout->addStretch();

  auto *backupLayout = new QVBoxLayout;
  backupLayout->addWidget(backupLabel);
  backupLayout->addLayout(backupAmountLayout);
  backupLayout->addStretch();

  auto *backupGroup = new QGroupBox(tr("Binary Backups"));
  backupGroup->setCheckable(true);
  backupGroup->setChecked(ctx.backupEnabled());
  backupGroup->setLayout(backupLayout);
  connect(backupGroup, &QGroupBox::toggled, this, [&ctx](bool on) { ctx.setBackupEnabled(on); });

  ///// Buttons and overall layout.

  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::onAccept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto *layout = new QVBoxLayout;
  layout->addWidget(mainGroup);
  layout->addWidget(backupGroup);
  layout->addStretch();
  layout->addWidget(buttonBox);

  setLayout(layout);
}

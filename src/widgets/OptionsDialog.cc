#include "widgets/OptionsDialog.h"
#include "Constants.h"
#include "Context.h"
#include "Disassembler.h"
#include "cxx.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace dispar {

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent)
{
  setWindowTitle(tr("Options"));
  createLayout();
}

void OptionsDialog::onTestDebugger()
{
  const auto dbg = currentDebugger();
  if (!dbg.valid()) {
    QMessageBox::warning(this, "", tr("Debugger is not valid! Specify all values."));
    return;
  }
  if (dbg.runnable()) {
    QMessageBox::information(this, "", tr("Debugger \"%1\" is working!").arg(dbg.program()));
  }
  else {
    QMessageBox::warning(this, "", tr("Could not run \"%1\"!").arg(dbg.program()));
  }
}

void OptionsDialog::onDetectInstalledDebuggers()
{
  const auto dbgs = Debugger::detect();
  if (dbgs.isEmpty()) {
    QMessageBox::warning(this, "",
                         tr("No installed debuggers were found! Make sure they are in PATH."));
    return;
  }

  QStringList items;
  for (const auto &dbg : dbgs) {
    items << dbg.program();
  }

  bool ok;
  const auto chosenDbg =
    QInputDialog::getItem(this, tr("Select debugger"), tr("Debugger:"), items, 0, false, &ok);
  if (!ok || chosenDbg.isEmpty()) {
    return;
  }

  if (const auto it =
        cxx::find_if(dbgs, [&](const auto &dbg) { return chosenDbg == dbg.program(); });
      it != dbgs.cend()) {
    debuggerEdit->setText(it->program());
    launchPatternEdit->setText(it->launchPattern());
    versionArgumentEdit->setText(it->versionArgument());
  }
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

  const auto newLevel = logLevelBox->currentData().toInt();
  ctx.setLogLevel(newLevel);

  const auto dbg = currentDebugger();
  if (dbg.valid()) {
    ctx.setDebugger(dbg);
  }

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
  disAsmSyntax->addItem(tr("Intel Masm"), (int) Disassembler::Syntax::INTEL_MASM);

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

  ///// Log Context

  logLevelBox = new QComboBox;
  logLevelBox->addItem(tr("Debug"), Constants::Log::DEBUG_LEVEL);
  logLevelBox->addItem(tr("Information"), Constants::Log::INFO_LEVEL);
  logLevelBox->addItem(tr("Warning"), Constants::Log::WARNING_LEVEL);
  logLevelBox->addItem(tr("Critical"), Constants::Log::CRITICAL_LEVEL);
  logLevelBox->addItem(tr("Fatal"), Constants::Log::FATAL_LEVEL);

  idx = logLevelBox->findData(ctx.logLevel());
  if (idx != -1) {
    logLevelBox->setCurrentIndex(idx);
  }

  auto *logLevelLayout = new QHBoxLayout;
  logLevelLayout->addWidget(new QLabel(tr("Level:")));
  logLevelLayout->addWidget(logLevelBox);
  logLevelLayout->addStretch();

  auto *logLayout = new QVBoxLayout;
  logLayout->addLayout(logLevelLayout);

  if (ctx.verbose()) {
    logLayout->addWidget(new QLabel(
      tr("Running in verbose mode!") + "\n" +
      tr("Log level will be saved if changed but debug is the effective session log level.")));
  }

  auto *logGroup = new QGroupBox(tr("Log Context"));
  logGroup->setLayout(logLayout);

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

  ///// Debugger

  debuggerEdit = new QLineEdit;
  debuggerEdit->setPlaceholderText("lldb");
  debuggerEdit->setMinimumWidth(200);
  debuggerEdit->setToolTip(tr("Debugger program name or full path."));

  launchPatternEdit = new QLineEdit;
  launchPatternEdit->setPlaceholderText("-- {{BINARY}} {{ARGS}}");
  launchPatternEdit->setMinimumWidth(200);
  launchPatternEdit->setToolTip(tr(
    "Launch pattern to invoke the debugger with in order to pass "
    "the binary to run and possible extra arguments.\nIt must at least include \"{{BINARY}}\" to "
    "signify the binary, and optionally also \"{{ARGS}}\" to specify how arguments are passed."));

  versionArgumentEdit = new QLineEdit;
  versionArgumentEdit->setPlaceholderText("--version");
  versionArgumentEdit->setMinimumWidth(200);
  versionArgumentEdit->setToolTip(
    tr("Version argument passed to debugger that will make the program exit successfully.\nIt can "
       "by anything that makes it exit with code 0, like \"--help\", for instance."));

  const auto dbg = ctx.debugger();
  if (dbg.valid()) {
    debuggerEdit->setText(dbg.program());
    launchPatternEdit->setText(dbg.launchPattern());
    versionArgumentEdit->setText(dbg.versionArgument());
  }

  auto *testDebuggerButton = new QPushButton(tr("Test Debugger"));
  connect(testDebuggerButton, &QPushButton::clicked, this, &OptionsDialog::onTestDebugger);

  auto *detectDebuggersButton = new QPushButton(tr("Detect Installed Debuggers"));
  connect(detectDebuggersButton, &QPushButton::clicked, this,
          &OptionsDialog::onDetectInstalledDebuggers);

  auto *debuggerButtonsLayout = new QHBoxLayout;
  debuggerButtonsLayout->addStretch();
  debuggerButtonsLayout->addWidget(testDebuggerButton);
  debuggerButtonsLayout->addWidget(detectDebuggersButton);
  debuggerButtonsLayout->addStretch();

  auto *debuggerLayout = new QFormLayout;
  debuggerLayout->addRow(tr("Debugger:"), debuggerEdit);
  debuggerLayout->addRow(tr("Launch Pattern:"), launchPatternEdit);
  debuggerLayout->addRow(tr("Version Argument:"), versionArgumentEdit);
  debuggerLayout->addRow(debuggerButtonsLayout);

  auto *debuggerGroup = new QGroupBox(tr("Debugger"));
  debuggerGroup->setLayout(debuggerLayout);

  ///// Buttons and overall layout.

  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::onAccept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto *layout = new QVBoxLayout;
  layout->addWidget(mainGroup);
  layout->addWidget(logGroup);
  layout->addWidget(backupGroup);
  layout->addWidget(debuggerGroup);
  layout->addStretch();
  layout->addWidget(buttonBox);

  setLayout(layout);
}

Debugger OptionsDialog::currentDebugger() const
{
  return {debuggerEdit->text(), versionArgumentEdit->text(), launchPatternEdit->text()};
}

} // namespace dispar

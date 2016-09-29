#include "OptionsDialog.h"
#include "../Context.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent)
{
  setWindowTitle(tr("Options"));
  createLayout();

  if (!restoreGeometry(QSettings().value("OptionsDialog.geometry").toByteArray())) {
    resize(600, 500);
  }
}

OptionsDialog::~OptionsDialog()
{
  QSettings().setValue("OptionsDialog.geometry", saveGeometry());
}

void OptionsDialog::onAccept()
{
  auto &ctx = Context::get();
  ctx.setShowMachineCode(showMachineCode->checkState() == Qt::Checked);

  accept();
}

void OptionsDialog::createLayout()
{
  auto &ctx = Context::get();

  showMachineCode = new QCheckBox(tr("Show Machine Code"));
  showMachineCode->setChecked(ctx.showMachineCode());

  auto *disAsmLabel = new QLabel(tr("Disassembly Syntax:"));

  auto *disAsmSyntax = new QComboBox;
  disAsmSyntax->addItem(tr("Intel"));
  disAsmSyntax->addItem(tr("AT/T"));

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

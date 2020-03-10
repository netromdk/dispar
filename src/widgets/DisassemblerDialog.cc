#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

#include "BinaryObject.h"
#include "Disassembler.h"
#include "Util.h"
#include "cxx.h"
#include "widgets/DisassemblerDialog.h"

namespace dispar {

DisassemblerDialog::DisassemblerDialog(QWidget *parent, CpuType cpuType, const QString &data,
                                       quint64 offset, Disassembler::Syntax syntax)
  : QDialog(parent), cpuType(cpuType), offset(offset), syntax(syntax)
{
  setWindowTitle(tr("Disassembler"));
  createLayout();
  resize(500, 400);
  Util::centerWidget(this);

  if (!data.isEmpty()) {
    machineText->setText(data);
    convertBtn->click();
  }
}

void DisassemblerDialog::onConvert()
{
  QString text = machineText->toPlainText();
  if (text.isEmpty()) {
    setAsmVisible(false);
    machineText->setFocus();
    QMessageBox::warning(this, "dispar", tr("Write some machine code!"));
    return;
  }

  const auto textOffset = offsetEdit->text().toULongLong(nullptr, 16);

  auto obj = std::make_unique<BinaryObject>(cpuType);
  Disassembler dis(*obj.get(), syntax);

  auto result = dis.disassemble(text, textOffset);
  if (result) {
    asmText->setText(result->toString());
    setAsmVisible();
  }
  else {
    setAsmVisible(false);
    machineText->setFocus();
    QMessageBox::warning(this, "dispar", tr("Could not disassemble machine code!"));
  }
}

void DisassemblerDialog::onCpuTypeIndexChanged(int index)
{
  cpuType = (CpuType) cpuTypeBox->itemData(index).toInt();
}

void DisassemblerDialog::onSyntaxIndexChanged(int index)
{
  syntax = (Disassembler::Syntax) syntaxBox->itemData(index).toInt();
}

void DisassemblerDialog::createLayout()
{
  machineText = new QTextEdit;
  machineText->setTabChangesFocus(true);

  offsetEdit = new QLineEdit;
  offsetEdit->setText(QString::number(offset, 16));
  offsetEdit->setAlignment(Qt::AlignRight);
  offsetEdit->setFixedWidth(140);
  offsetEdit->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f0-9]{1,16}"), this));

  auto *offsetLayout = new QHBoxLayout;
  offsetLayout->setContentsMargins(0, 0, 0, 0);
  offsetLayout->addWidget(new QLabel(tr("Hex offset:")));
  offsetLayout->addWidget(offsetEdit);
  offsetLayout->addStretch();

  auto *machineLayout = new QVBoxLayout;
  machineLayout->setContentsMargins(0, 0, 0, 0);
  machineLayout->addWidget(new QLabel(tr("Machine code:")));
  machineLayout->addWidget(machineText);
  machineLayout->addLayout(offsetLayout);

  auto *machineWidget = new QWidget;
  machineWidget->setLayout(machineLayout);

  asmText = new QTextEdit;
  asmText->setReadOnly(true);
  asmText->setTabChangesFocus(true);

  auto *asmLayout = new QVBoxLayout;
  asmLayout->setContentsMargins(0, 0, 0, 0);
  asmLayout->addWidget(new QLabel(tr("Disassembly:")));
  asmLayout->addWidget(asmText);

  auto *asmWidget = new QWidget;
  asmWidget->setLayout(asmLayout);

  splitter = new QSplitter(Qt::Vertical);
  splitter->addWidget(machineWidget);
  splitter->addWidget(asmWidget);

  splitter->setCollapsible(0, false);
  splitter->setCollapsible(1, true);
  setAsmVisible(false);

  cpuTypeBox = new QComboBox;
  cpuTypeBox->addItem(tr("X86"), (int) CpuType::X86);
  cpuTypeBox->addItem(tr("X86_64"), (int) CpuType::X86_64);

  int idx = cpuTypeBox->findData((int) cpuType);
  if (idx != -1) {
    cpuTypeBox->setCurrentIndex(idx);
  }

  connect(cpuTypeBox, cxx::Use<int>::overloadOf(&QComboBox::currentIndexChanged), this,
          &DisassemblerDialog::onCpuTypeIndexChanged);

  syntaxBox = new QComboBox;
  syntaxBox->addItem(tr("AT&T"), (int) Disassembler::Syntax::ATT);
  syntaxBox->addItem(tr("Intel"), (int) Disassembler::Syntax::INTEL);
  syntaxBox->addItem(tr("Intel Masm"), (int) Disassembler::Syntax::INTEL_MASM);

  idx = syntaxBox->findData((int) syntax);
  if (idx != -1) {
    syntaxBox->setCurrentIndex(idx);
  }

  connect(syntaxBox, cxx::Use<int>::overloadOf(&QComboBox::currentIndexChanged), this,
          &DisassemblerDialog::onSyntaxIndexChanged);

  convertBtn = new QPushButton(tr("Disassemble"));
  connect(convertBtn, &QPushButton::clicked, this, &DisassemblerDialog::onConvert);

  auto *bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(new QLabel(tr("CPU:")));
  bottomLayout->addWidget(cpuTypeBox);
  bottomLayout->addWidget(new QLabel(tr("Syntax:")));
  bottomLayout->addWidget(syntaxBox);
  bottomLayout->addStretch();
  bottomLayout->addWidget(convertBtn);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(splitter);
  layout->addLayout(bottomLayout);

  setLayout(layout);
}

void DisassemblerDialog::setAsmVisible(bool visible)
{
  splitter->setSizes(QList<int>{1, visible ? 1 : 0});
  if (!visible) asmText->clear();
}

} // namespace dispar

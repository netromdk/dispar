#include <QFile>
#include <QDebug>
#include <QListWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QStackedLayout>
#include <QProgressDialog>

#include "../Util.h"
#include "BinaryWidget.h"

/*
#include "../panes/Pane.h"
#include "../panes/ArchPane.h"
#include "../panes/ProgramPane.h"
#include "../panes/SymbolsPane.h"
#include "../panes/StringsPane.h"
#include "../panes/GenericPane.h"
#include "../panes/DisassemblyPane.h"
*/

BinaryWidget::BinaryWidget(std::shared_ptr<Format> fmt) : fmt(fmt)
{
  /*
  createLayout();
  setup();
  */
}

/*
void BinaryWidget::createLayout()
{
  listWidget = new QListWidget;
  listWidget->setFixedWidth(175);
  connect(listWidget, &QListWidget::currentRowChanged, this, &BinaryWidget::onModeChanged);

  stackLayout = new QStackedLayout;
  stackLayout->setContentsMargins(0, 0, 0, 0);

  auto *layout = new QHBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(listWidget);
  layout->addLayout(stackLayout);

  setLayout(layout);
}

void BinaryWidget::onModeChanged(int row)
{
  stackLayout->setCurrentIndex(row);
}

void BinaryWidget::setup()
{
  foreach (const auto obj, fmt->getObjects()) {
    auto *archPane = new ArchPane(fmt->getType(), obj);
    QString cpuStr = Util::cpuTypeString(obj->getCpuType()),
            cpuSubStr = Util::cpuTypeString(obj->getCpuSubType());
    addPane(tr("%1 (%2)").arg(cpuStr).arg(cpuSubStr), archPane);

    SectionPtr sec = obj->getSection(SectionType::Text);
    if (sec) {
      addPane(tr("Executable Code"), new ProgramPane(obj, sec), 1);
      addPane(tr("Disassembly"), new DisassemblyPane(obj, sec), 2);
    }

    sec = obj->getSection(SectionType::SymbolStubs);
    if (sec) {
      addPane(sec->getName(), new GenericPane(obj, sec), 1);
    }

    sec = obj->getSection(SectionType::Symbols);
    if (sec) {
      addPane(sec->getName(), new SymbolsPane(obj, sec, SymbolsPane::Type::Symbols), 1);
      addPane(tr("Raw View"), new GenericPane(obj, sec), 2);
    }

    sec = obj->getSection(SectionType::DynSymbols);
    if (sec) {
      addPane(sec->getName(), new SymbolsPane(obj, sec, SymbolsPane::Type::DynSymbols), 1);
      addPane(tr("Raw View"), new GenericPane(obj, sec), 2);
    }

    sec = obj->getSection(SectionType::String);
    if (sec) {
      addPane(sec->getName(), new StringsPane(obj, sec), 1);
      addPane(tr("Raw View"), new GenericPane(obj, sec), 2);
    }

    foreach (auto sec, obj->getSectionsByType(SectionType::CString)) {
      addPane(sec->getName(), new StringsPane(obj, sec), 1);
      addPane(tr("Raw View"), new GenericPane(obj, sec), 2);
    }

    sec = obj->getSection(SectionType::FuncStarts);
    if (sec) {
      addPane(sec->getName(), new GenericPane(obj, sec), 1);
    }

    sec = obj->getSection(SectionType::CodeSig);
    if (sec) {
      addPane(sec->getName(), new GenericPane(obj, sec), 1);
    }
  }

  if (listWidget->count() > 0) {
    listWidget->setCurrentRow(0);
  }
}

void BinaryWidget::addPane(const QString &title, Pane *pane, int level)
{
  listWidget->addItem(QString(level * 4, ' ') + title);
  stackLayout->addWidget(pane);
  connect(pane, SIGNAL(modified()), this, SIGNAL(modified()));
}
*/

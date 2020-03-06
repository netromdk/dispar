#include "widgets/LogDialog.h"
#include "Constants.h"
#include "Context.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace dispar {

LogDialog::LogDialog(QWidget *parent) : QDialog(parent)
{
  setWindowTitle(tr("Log"));
  setMinimumSize(600, 400);

  createLayout();

  loadEntries();

  auto &ctx = Context::get();
  connect(ctx.logHandler(), &LogHandler::newEntry, this, &LogDialog::addEntry);
  connect(&ctx, &Context::logLevelChanged, this, &LogDialog::loadEntries);
}

void LogDialog::createLayout()
{
  treeWidget = new QTreeWidget;
  treeWidget->setHeaderLabels({tr("When"), tr("Type"), tr("Message")});

  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

  auto *layout = new QVBoxLayout;
  layout->addWidget(treeWidget);
  layout->addWidget(buttonBox);

  setLayout(layout);
}

void LogDialog::loadEntries()
{
  treeWidget->clear();
  const auto &entries = Context::get().logHandler()->entries();
  for (int i = entries.firstIndex(), last = entries.lastIndex(); i <= last; ++i) {
    addEntry(entries.at(i));
  }
}

void LogDialog::addEntry(const LogHandler::Entry &entry)
{
  if (!Context::get().acceptMsgType(entry.type)) {
    return;
  }

  auto *item = new QTreeWidgetItem;
  item->setText(0, entry.time.toString("hh:mm:ss"));
  item->setText(1, entry.typeString());
  item->setText(2, entry.msg);
  treeWidget->addTopLevelItem(item);
  treeWidget->scrollToItem(item);

  // Remove initial entries when exceeding set amount.
  for (int i = 0, tooMany = treeWidget->topLevelItemCount() - Constants::Log::MEMORY_ENTRIES;
       i < tooMany; ++i) {
    delete treeWidget->takeTopLevelItem(i);
  }
}

} // namespace dispar

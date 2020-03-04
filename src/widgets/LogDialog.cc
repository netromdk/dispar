#include "widgets/LogDialog.h"
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
  connect(Context::get().logHandler(), &LogHandler::newEntry, this, &LogDialog::addEntry);

  // TODO: set log level in LogDialog or OptionsDialog?? when changed, call loadEntries()
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
  for (const auto &entry : Context::get().logHandler()->entries()) {
    addEntry(entry);
  }
}

void LogDialog::addEntry(const LogHandler::Entry &entry)
{
  auto *item = new QTreeWidgetItem;
  item->setText(0, entry.time.toString("hh:mm:ss"));
  item->setText(1, entry.typeString());
  item->setText(2, entry.msg);
  treeWidget->addTopLevelItem(item);
  treeWidget->scrollToItem(item);
}

} // namespace dispar

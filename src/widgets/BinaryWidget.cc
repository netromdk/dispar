#include <QFile>
#include <QTextEdit>
#include <QDebug>
#include <QListWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>

#include "../Util.h"
#include "BinaryWidget.h"

BinaryWidget::BinaryWidget(std::shared_ptr<Format> fmt) : fmt(fmt)
{
  createLayout();
  setup();
}

QString BinaryWidget::file() const
{
  return fmt->file();
}

void BinaryWidget::onSymbolChosen(int row)
{
  auto *item = symbolList->item(row);
  auto offset = item->data(Qt::UserRole).toLongLong();
  qDebug() << "Chosen offset:" << QString::number(offset, 16);
}

void BinaryWidget::createLayout()
{
  symbolList = new QListWidget;
  symbolList->setFixedWidth(175);
  connect(symbolList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  mainView = new QTextEdit;

  auto *layout = new QHBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(symbolList);
  layout->addWidget(mainView);

  setLayout(layout);
}

void BinaryWidget::setup()
{
  // For now we just support one object!

  auto obj = fmt->objects()[0];

  // Fill side bar with function names of the symbol table.
  for (auto symbol : obj->symbolTable().symbols()) {
    auto *item = new QListWidgetItem;
    item->setText(symbol.string());
    item->setData(Qt::UserRole, symbol.value()); // Offset to symbol.
    item->setToolTip(QString("0x%1").arg(symbol.value(), 0, 16));
    symbolList->addItem(item);
  }

  // The "_main" symbol is normally the second symbol so choose that. If not enough symbols then
  // choose the first.
  auto count = symbolList->count();
  if (count > 1) {
    symbolList->setCurrentRow(1);
  }
  else if (count > 0) {
    symbolList->setCurrentRow(0);
  }

  // TODO: Fill out the text edit!
}

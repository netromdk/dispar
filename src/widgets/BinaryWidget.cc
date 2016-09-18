#include <QTimer>
#include <QFile>
#include <QTextEdit>
#include <QDebug>
#include <QListWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>
#include <QTextTable>

#include "../Util.h"
#include "BinaryWidget.h"

namespace {

// Temporary solution!
class TextBlockUserData : public QTextBlockUserData {
public:
  QString toString()
  {
    if (addr) {
      return "address";
    }
    else if (inst) {
      return "instruction";
    }
    else {
      return "operands";
    }
  }

  bool addr = false, inst = false, ops = false;
};

} // anon

BinaryWidget::BinaryWidget(std::shared_ptr<Format> fmt) : fmt(fmt), doc(nullptr)
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
  // If offset is found then select the text block.
  auto *item = symbolList->item(row);
  auto offset = item->data(Qt::UserRole).toLongLong();
  if (offsetBlock.contains(offset)) {
    auto blockNum = offsetBlock[offset];
    auto block = doc->findBlockByNumber(blockNum);
    auto cursor = mainView->textCursor();
    cursor.setPosition(block.position());
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    mainView->setTextCursor(cursor);
    mainView->ensureCursorVisible();
    mainView->setFocus();
  }
}

void BinaryWidget::onCursorPositionChanged()
{
  auto cursor = mainView->textCursor();
  auto block = cursor.block();
  qDebug() << "column:" << cursor.columnNumber() << "block:" << block.blockNumber();

  auto *userData = reinterpret_cast<TextBlockUserData *>(block.userData());
  if (userData) {
    qDebug() << userData->toString();
  }
}

void BinaryWidget::createLayout()
{
  symbolList = new QListWidget;
  symbolList->setFixedWidth(175);
  connect(symbolList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  mainView = new QTextEdit;
  mainView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  connect(mainView, &QTextEdit::cursorPositionChanged, this,
          &BinaryWidget::onCursorPositionChanged);

  doc = mainView->document();

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

  // Create text edit of all binary contents.
  // TODO: FAKE IT FOW NOW!
  QTextCursor cursor(doc);

  auto createTable = [this, &cursor](const QStringList &values) {
    cursor.movePosition(QTextCursor::End);
    auto *table = cursor.insertTable(1, 3);
    Q_ASSERT(values.size() <= 3);

    auto format = table->format();
    QVector<QTextLength> colWidths;
    colWidths << QTextLength(QTextLength::FixedLength, 200)
              << QTextLength(QTextLength::FixedLength, 40)
              << QTextLength(QTextLength::VariableLength, 1);
    format.setColumnWidthConstraints(colWidths);
    format.setBorder(0);
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    table->setFormat(format);

    int col = 0;
    for (const auto &value : values) {
      int cellCol = col++;
      auto cellCursor = table->cellAt(0, cellCol).firstCursorPosition();
      cellCursor.insertText(value);

      auto *userData = new TextBlockUserData;
      userData->addr = (cellCol == 0);
      userData->inst = (cellCol == 1);
      userData->ops = (cellCol == 2);

      auto block = cellCursor.block();
      block.setUserData(userData);

      if (cellCol == 0) {
        offsetBlock[value.toLongLong(nullptr, 16)] = block.blockNumber();
      }
    }
  };

  cursor.beginEditBlock();

  cursor.movePosition(QTextCursor::End);

  // There is a default block at the beginning so reuse that.
  if (cursor.block() != doc->firstBlock()) {
    cursor.insertBlock();
  }
  cursor.insertText("_main:");

  createTable(QStringList{"0x0000000100000fa0", "push", "rbp"});
  createTable(QStringList{"0x0000000100000fa1", "mov", "rbp, rsp"});
  createTable(QStringList{"0x0000000100000fa4", "xor", "eax, eax"});
  createTable(QStringList{"0x0000000100000fa6", "mov", "dword [ss:rbp+4], 0x0"});
  createTable(QStringList{"0x0000000100000fad", "pop", "rbp"});
  createTable(QStringList{"0x0000000100000fae", "ret"});

  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();
  cursor.insertText("; endp");

  cursor.endEditBlock();

  Util::scrollToTop(mainView);

  // The "_main" symbol is normally the second symbol so choose that. If not enough symbols then
  // choose the first.
  QTimer::singleShot(1, this, [this] {
    auto count = symbolList->count();
    if (count > 1) {
      symbolList->setCurrentRow(1);
    }
    else if (count > 0) {
      symbolList->setCurrentRow(0);
    }
  });
}

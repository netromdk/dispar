#include <QTimer>
#include <QFile>
#include <QPlainTextEdit>
#include <QDebug>
#include <QListWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>
#include <QTextBlockUserData>

#include "../Util.h"
#include "../BinaryObject.h"
#include "BinaryWidget.h"

namespace {

// Temporary solution!
class TextBlockUserData : public QTextBlockUserData {
public:
  quint64 address;
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
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
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
    qDebug() << "address:" << QString::number(userData->address, 16);
  }

  // auto text = block.text();
}

void BinaryWidget::createLayout()
{
  symbolList = new QListWidget;
  symbolList->setFixedWidth(175);
  connect(symbolList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  mainView = new QPlainTextEdit;
  mainView->setReadOnly(true);
  mainView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  connect(mainView, &QPlainTextEdit::cursorPositionChanged, this,
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

  // Fill side bar with function names of the symbol tables.
  auto symbols = obj->symbolTable().symbols();
  symbols.append(obj->dynSymbolTable().symbols());

  for (auto symbol : symbols) {
    if (symbol.value() == 0) continue;
    auto *item = new QListWidgetItem;

    auto func = Util::demangle(symbol.string());
    if (func.isEmpty()) {
      func = QString("unnamed_%1").arg(symbol.value(), 0, 16);
    }
    item->setText(func);

    item->setData(Qt::UserRole, symbol.value()); // Offset to symbol.
    item->setToolTip(QString("0x%1").arg(symbol.value(), 0, 16));
    symbolList->addItem(item);
  }

  // Create text edit of all binary contents.
  QTextCursor cursor(doc);

  auto appendInstruction = [this, &cursor](const QStringList &values) {
    Q_ASSERT(values.size() <= 3);

    cursor.insertBlock();
    cursor.insertText(QString("%1%2%3").arg(values[0], -20).arg(values[1], -8).arg(values[2]));

    auto *userData = new TextBlockUserData;
    userData->address = values[0].toLongLong(nullptr, 16);

    auto block = cursor.block();
    block.setUserData(userData);

    offsetBlock[userData->address] = block.blockNumber();
  };

  cursor.beginEditBlock();

  for (auto &sec : obj->sections()) {
    auto disasm = sec->disassembly();
    if (!disasm) continue;

    cursor.movePosition(QTextCursor::End);

    // There is a default block at the beginning so reuse that.
    if (cursor.block() != doc->firstBlock()) {
      cursor.insertBlock();
    }

    auto secName = Section::typeName(sec->type());
    cursor.insertText("===== " + secName + " =====");

    for (size_t i = 0; i < disasm->count(); i++) {
      auto *instr = disasm->instructions(i);
      auto addr = instr->address + sec->address();

      // Check if address is the start of a procedure.
      for (const auto &symbol : symbols) {
        if (symbol.value() == addr && !symbol.string().isEmpty()) {
          cursor.movePosition(QTextCursor::End);
          cursor.insertBlock();
          cursor.insertText("\nPROC: " + Util::demangle(symbol.string()) + "\n");
          break;
        }
      }

      appendInstruction(
        QStringList{QString("0x%1").arg(addr, 0, 16), instr->mnemonic, instr->op_str});
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  cursor.endEditBlock();

  Util::scrollToTop(mainView);
}

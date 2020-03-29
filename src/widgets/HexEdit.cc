#include "widgets/HexEdit.h"
#include "AddrHexAsciiEncoder.h"
#include "BinaryObject.h"
#include "Constants.h"
#include "Section.h"
#include "Util.h"
#include "widgets/ConversionHelper.h"
#include "widgets/DisassemblerDialog.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QTextBlock>

#include <cassert>

namespace dispar {

QString HexEdit::Block::addrStr() const
{
  return QString::number(addr, 16).toUpper();
}

QString HexEdit::Block::addrEndStr() const
{
  // Return the last address of the block.
  return QString::number(addr + 15, 16).toUpper();
}

QString HexEdit::Block::hex() const
{
  return hexLow + hexHigh;
}

HexEdit::HexEdit(QWidget *parent) : QPlainTextEdit(parent)
{
  setReadOnly(true);
  setCenterOnScroll(true);
  setLineWrapMode(QPlainTextEdit::NoWrap);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setFont(Constants::FIXED_FONT);

  connect(this, &QPlainTextEdit::cursorPositionChanged, this, &HexEdit::cursorPositionChanged);
  connect(this, &QPlainTextEdit::customContextMenuRequested, this,
          &HexEdit::customContextMenuRequested);
}

void HexEdit::decode(Section *section_, BinaryObject *object_)
{
  section = section_;
  assert(section_);
  object = object_;
  assert(object_);

  AddrHexAsciiEncoder encoder(section);
  const bool blocking(true);
  encoder.start(blocking);
  setPlainText(encoder.result());
  markModifiedRegions();
}

void HexEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
  QPlainTextEdit::mouseDoubleClickEvent(event);
  editAtCursor();
}

void HexEdit::cursorPositionChanged()
{
  // Mark the whole line to highlight it.
  auto highlight = palette().highlight();
  highlight.setColor(highlight.color().lighter(120));

  QTextEdit::ExtraSelection selection;
  selection.format.setBackground(highlight);
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = textCursor();

  curLineSelection = {selection};

  setExtraSelections(modSelections + curLineSelection);
}

void HexEdit::customContextMenuRequested(const QPoint &pos)
{
  QMenu menu;
  menu.addAction(tr("Edit"), this, &HexEdit::editAtCursor);
  menu.addAction(tr("Find address"), this, &HexEdit::findAddress);
  menu.addSeparator();

  auto *copyMenu = menu.addMenu(tr("Copy"));
  copyMenu->addAction(tr("Address"), this, [this] { copyContent(Copy::ADDRESS); });
  copyMenu->addAction(tr("Hex low"), this, [this] { copyContent(Copy::HEX_LOW); });
  copyMenu->addAction(tr("Hex high"), this, [this] { copyContent(Copy::HEX_HIGH); });
  copyMenu->addAction(tr("Hex low + high"), this, [this] { copyContent(Copy::HEX_BOTH); });
  copyMenu->addAction(tr("ASCII"), this, [this] { copyContent(Copy::ASCII); });

  menu.addSeparator();
  menu.addAction(tr("Disassemble"), this, &HexEdit::disassemble);

  menu.addSeparator();
  menu.addAction(tr("Conversion helper"), this, &HexEdit::showConversionHelper);

  menu.exec(mapToGlobal(pos));
}

void HexEdit::copyContent(const Copy type)
{
  const auto block = currentBlock();
  const auto text = [block, type]() -> QString {
    switch (type) {
    case Copy::ADDRESS:
      return block.addrStr();
    case Copy::HEX_LOW:
      return block.hexLow;
    case Copy::HEX_HIGH:
      return block.hexHigh;
    case Copy::HEX_BOTH:
      return block.hex();
    case Copy::ASCII:
      return block.ascii;
    }
    return {};
  }();
  QApplication::clipboard()->setText(text);
}

void HexEdit::editAtCursor()
{
  const auto block = currentBlock();

  QDialog diag(this);
  diag.setWindowFlags(Qt::CustomizeWindowHint | Qt::Dialog | Qt::WindowTitleHint |
                      Qt::WindowCloseButtonHint);
  diag.setWindowTitle(tr("Hex block edit"));

  auto *addrLabel = new QLabel(block.addrStr() + " - " + block.addrEndStr());

  auto *asciiLowLabel = new QLabel(block.ascii.mid(0, 8));
  asciiLowLabel->setFont(Constants::FIXED_FONT);

  auto *asciiHighLabel = new QLabel(block.ascii.mid(8));
  asciiHighLabel->setFont(Constants::FIXED_FONT);

  const auto createEdit = [](const QString &data, QLabel *label) {
    auto *edit = new QLineEdit;
    edit->setMinimumWidth(200);
    edit->setFont(Constants::FIXED_FONT);

    if (data.isEmpty()) {
      edit->setDisabled(true);
      return edit;
    }

    const int blocks = data.size() / 2;
    QString mask, newData;
    for (int i = 0; i < blocks; i++) {
      mask += "HH ";
      newData += data.mid(i * 2, 2) + " ";
    }
    mask.chop(1);
    newData.chop(1);

    edit->setInputMask(mask);
    edit->setText(newData);

    // Update ASCII on editing hex.
    connect(edit, &QLineEdit::textEdited, label,
            [label](const QString newHex) { label->setText(Util::hexToAscii(newHex, 0, 8)); });

    return edit;
  };

  auto *editLow = createEdit(block.hexLow, asciiLowLabel),
       *editHigh = createEdit(block.hexHigh, asciiHighLabel);

  auto *lowLayout = new QHBoxLayout;
  lowLayout->addWidget(editLow);
  lowLayout->addWidget(asciiLowLabel);

  auto *highLayout = new QHBoxLayout;
  highLayout->addWidget(editHigh);
  highLayout->addWidget(asciiHighLabel);

  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, &diag, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &diag, &QDialog::reject);

  auto *layout = new QFormLayout;
  layout->addRow(tr("Address:"), addrLabel);
  layout->addRow(tr("Low:"), lowLayout);
  layout->addRow(tr("High:"), highLayout);
  layout->addWidget(buttonBox);

  diag.setLayout(layout);

  if (QDialog::Rejected == diag.exec()) {
    return;
  }

  auto const low = editLow->text().toUpper();
  auto const high = editHigh->text().toUpper();
  auto newStr = low + "  " + high;
  const auto newAscii = Util::hexToAscii(newStr, 0, 16);
  const auto newLine = QString::number(block.addr, 16).toUpper() + ": " + newStr + "   " + newAscii;

  // Ignore if hex wasn't changed.
  if (newLine == block.line) {
    return;
  }

  // Change region.
  quint64 pos = block.addr - section->address();
  const auto data = Util::hexToData(newStr.replace(" ", ""));
  section->setSubData(data, pos);

  // Change line text: "[ADDR]: [LOW]  [HIGH]   [ASCII]"
  auto cursor = textCursor();
  cursor.clearSelection();
  cursor.movePosition(QTextCursor::StartOfLine);
  cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
  cursor.insertText(newLine);

  markModifiedRegions();
  emit edited();
}

void HexEdit::findAddress()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("Find Address"), tr("Address (hex):"),
                                       QLineEdit::Normal, QString(), &ok);
  if (!ok || text.isEmpty()) {
    return;
  }

  quint64 num = text.toULongLong(&ok, 16);
  if (!ok) {
    QMessageBox::warning(this, "dispar", tr("Invalid address! Must be in hexadecimal."));
    findAddress();
    return;
  }

  const auto *doc = document();
  const auto firstAddr = cursorBlock(doc->firstBlock()).addr;
  const auto lastAddr = cursorBlock(doc->lastBlock()).addr + 16;

  if (num >= firstAddr && num < lastAddr) {
    num -= firstAddr; // Remove offset.
    num /= 16;        // Find nearest 16 byte multiple.
    const auto block = doc->findBlockByNumber(num);

    auto cursor = QTextCursor(block);
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::StartOfLine);
    setTextCursor(cursor);
    ensureCursorVisible();
    return;
  }

  QMessageBox::information(this, "dispar", tr("Did not find anything."));
}

void HexEdit::disassemble()
{
  const auto block = currentBlock();
  DisassemblerDialog diag(this, object->cpuType(), block.hex(), block.addr);
  diag.exec();
}

void HexEdit::showConversionHelper()
{
  auto *helper = new ConversionHelper(this);
  connect(helper, &QDialog::finished, helper, &QDialog::deleteLater);
  helper->show();
}

HexEdit::Block HexEdit::cursorBlock(const QTextBlock &block) const
{
  const auto oldText = block.text().trimmed();

  const auto addr = oldText.mid(0, oldText.indexOf(':')).toULongLong(nullptr, 16);
  const auto ascii = oldText.right(16);

  // Remove the address part "...:" and the last ASCII part.
  auto text = oldText.mid(oldText.indexOf(':') + 1);
  text.chop(16); // Remove ASCII.
  text = text.trimmed().replace(" ", "");

  // Each part is up to 2x8 chars.
  const auto hexLow = text.mid(0, 16), hexHigh = text.mid(16, 16);

  return {addr, hexLow, hexHigh, ascii, oldText};
}

HexEdit::Block HexEdit::currentBlock() const
{
  return cursorBlock(textCursor().block());
}

void HexEdit::markModifiedRegions()
{
  const auto &modRegs = section->modifiedRegions();
  if (modRegs.isEmpty()) return;

  const auto *doc = document();
  modSelections.clear();

  for (const auto &reg : modRegs) {
    const auto offset = reg.position;
    const auto block = doc->findBlockByNumber(offset / 16);

    QTextEdit::ExtraSelection selection;
    selection.format.setForeground(Qt::red);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = QTextCursor(block);

    modSelections << selection;
  }

  setExtraSelections(modSelections + curLineSelection);
}

} // namespace dispar

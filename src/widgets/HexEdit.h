#ifndef SRC_WIDGETS_HEXEDIT_H
#define SRC_WIDGETS_HEXEDIT_H

#include "CpuType.h"

#include <QPlainTextEdit>

namespace dispar {

class Section;
class BinaryObject;

class HexEdit : public QPlainTextEdit {
  Q_OBJECT

  enum class Copy { ADDRESS, HEX_LOW, HEX_HIGH, HEX_BOTH, ASCII };

public:
  HexEdit(QWidget *parent = nullptr);

  void decode(Section *section, BinaryObject *object);

signals:
  void edited();

protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
  void cursorPositionChanged();
  void customContextMenuRequested(const QPoint &pos);
  void editAtCursor();
  void findAddress();
  void copyContent(const Copy type);
  void disassemble();
  void showConversionHelper();

private:
  struct Block {
    quint64 addr = 0;
    QString hexLow, hexHigh, ascii, line;

    [[nodiscard]] QString addrStr() const;
    [[nodiscard]] QString addrEndStr() const;
    [[nodiscard]] QString hex() const;
    [[nodiscard]] bool hasHexHigh() const;
  };

  [[nodiscard]] Block cursorBlock(const QTextBlock &block) const;
  [[nodiscard]] Block currentBlock() const;
  void markModifiedRegions();

  Section *section = nullptr;
  BinaryObject *object = nullptr;
  QList<QTextEdit::ExtraSelection> modSelections, curLineSelection;
};

} // namespace dispar

#endif // SRC_WIDGETS_HEXEDIT_H

#ifndef SRC_WIDGETS_HEXEDIT_H
#define SRC_WIDGETS_HEXEDIT_H

#include "CpuType.h"

#include <QPlainTextEdit>

namespace dispar {

class Section;

class HexEdit : public QPlainTextEdit {
  Q_OBJECT

  enum class Copy { ADDRESS, HEX_LOW, HEX_HIGH, HEX_BOTH, ASCII };

public:
  HexEdit(CpuType cpuType, QWidget *parent = nullptr);

  void decode(Section *section);

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

    QString addrStr() const;
    QString addrEndStr() const;
    QString hex() const;
  };

  Block cursorBlock(const QTextBlock &block) const;
  Block currentBlock() const;
  void markModifiedRegions();

  CpuType cpuType;
  Section *section = nullptr;
  QList<QTextEdit::ExtraSelection> modSelections, curLineSelection;
};

} // namespace dispar

#endif // SRC_WIDGETS_HEXEDIT_H

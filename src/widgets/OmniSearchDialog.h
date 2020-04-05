#ifndef SRC_WIDGETS_OMNISEARCHDIALOG_H
#define SRC_WIDGETS_OMNISEARCHDIALOG_H

#include <QDialog>

class QTreeWidget;

namespace dispar {

class LineEdit;
class BinaryWidget;

class OmniSearchDialog : public QDialog {
  Q_OBJECT

  /// Types of candidates.
  enum class EntryType { SECTION, SYMBOL, STRING, TAG };

  /// Navigation of candidates.
  enum class Navigation { UP, DOWN };

public:
  OmniSearchDialog(QWidget *parent = nullptr);
  virtual ~OmniSearchDialog() override;

  void setBinaryWidget(BinaryWidget *widget);

protected:
  void showEvent(QShowEvent *event) override;

private slots:
  void inputEdited(const QString &text);
  void inputKeyDown();
  void inputKeyUp();
  void inputReturnPressed();

private:
  void setupLayout();
  void search(const QString &text);
  float flexMatch(const QString &haystack, const QString &needle);
  void addCandidate(const QString &text, const EntryType type, const float similarity,
                    const QVariant data);
  void navigateCandidates(const Navigation nav);

  BinaryWidget *binaryWidget = nullptr;

  LineEdit *inputEdit;
  QTreeWidget *candidatesWidget;

  // TODO: table should be hidden when no candidates are available?
};

} // namespace dispar

#endif // SRC_WIDGETS_OMNISEARCHDIALOG_H

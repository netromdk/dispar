#ifndef SRC_WIDGETS_OMNISEARCHDIALOG_H
#define SRC_WIDGETS_OMNISEARCHDIALOG_H

#include <QDialog>
#include <QRegularExpression>
#include <QTimer>
#include <QTreeWidgetItem>

class QLabel;
class QTreeWidget;
class QListWidget;
class QCheckBox;

namespace dispar {

class Section;
class LineEdit;
class BinaryWidget;

class OmniSearchItem : public QTreeWidgetItem {
public:
  OmniSearchItem(const QStringList &values);

  bool operator<(const QTreeWidgetItem &rhs) const override;
};

class OmniSearchDialog : public QDialog {
  Q_OBJECT

  /// Types of candidates.
  enum class EntryType {
    SECTION, ///< Section names and type names.
    SYMBOL,  ///< Symbols and function names.
    STRING,  ///< String entries (c-string, obj-c strings etc.).
    TAG,     ///< Tag names.
    TEXT,    ///< Text in binary widget.
  };

  /// Navigation of candidates.
  enum class Navigation { UP, DOWN };

public:
  OmniSearchDialog(QWidget *parent = nullptr);
  ~OmniSearchDialog() override;

  void setBinaryWidget(BinaryWidget *widget);

  void done(int result) override;

protected:
  void showEvent(QShowEvent *event) override;

private slots:
  void inputEdited(const QString &text);
  void inputKeyDown();
  void inputKeyUp();
  void inputKeyCtrlNumber(int num);
  void search();
  void activateCurrentItem();
  void candidateContextMenu(const QPoint &pos);
  void copyCurrentText();

private:
  void setupLayout();
  [[nodiscard]] float flexMatch(const QString &haystack) const;
  [[nodiscard]] QList<QTreeWidgetItem *> flexMatchSections(const QList<Section *> &sections) const;
  QList<QTreeWidgetItem *> flexMatchListRows(const QListWidget *list, int startRow, int amount,
                                             EntryType type) const;
  [[nodiscard]] QList<QTreeWidgetItem *> flexMatchTextOffset(const QString &text, int offset) const;
  [[nodiscard]] static QTreeWidgetItem *createCandidate(const QString &text, EntryType type,
                                                        float similarity, const QVariant &data,
                                                        const QString &fullText = {});
  void navigateCandidates(Navigation nav);
  void activateItem(const QTreeWidgetItem *item);

  static QString entryTypeString(EntryType type);

  BinaryWidget *binaryWidget = nullptr;

  LineEdit *inputEdit = nullptr;
  QTreeWidget *candidatesWidget = nullptr;
  QLabel *statusLabel = nullptr;
  QCheckBox *searchTextChk = nullptr;

  QTimer searchTimer;
  QString input;
  QRegularExpression regex;
};

} // namespace dispar

#endif // SRC_WIDGETS_OMNISEARCHDIALOG_H

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
  virtual ~OmniSearchDialog() override;

  void setBinaryWidget(BinaryWidget *widget);

  void done(int result) override;

protected:
  void showEvent(QShowEvent *event) override;

private slots:
  void inputEdited(const QString &text);
  void inputKeyDown();
  void inputKeyUp();
  void search();
  void activateCurrentItem();
  void candidateContextMenu(const QPoint &pos);
  void copyCurrentText();

private:
  void setupLayout();
  float flexMatch(const QString &haystack) const;
  QList<QTreeWidgetItem *> flexMatchSections(const QList<Section *> &sections) const;
  QList<QTreeWidgetItem *> flexMatchList(const QListWidget *list, const EntryType type) const;
  QList<QTreeWidgetItem *> flexMatchText() const;
  QTreeWidgetItem *createCandidate(const QString &text, const EntryType type,
                                   const float similarity, const QVariant data) const;
  void navigateCandidates(const Navigation nav);
  void activateItem(const QTreeWidgetItem *item);

  BinaryWidget *binaryWidget = nullptr;

  LineEdit *inputEdit;
  QTreeWidget *candidatesWidget;
  QLabel *statusLabel;
  QCheckBox *searchTextChk;

  QTimer searchTimer;
  QString input;
  QRegularExpression regex;
};

} // namespace dispar

#endif // SRC_WIDGETS_OMNISEARCHDIALOG_H

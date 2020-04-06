#ifndef SRC_WIDGETS_OMNISEARCHDIALOG_H
#define SRC_WIDGETS_OMNISEARCHDIALOG_H

#include <QDialog>
#include <QTimer>

class QTreeWidget;
class QTreeWidgetItem;
class QListWidget;

namespace dispar {

class Section;
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

  void done(int result) override;

protected:
  void showEvent(QShowEvent *event) override;

private slots:
  void inputEdited(const QString &text);
  void inputKeyDown();
  void inputKeyUp();
  void inputReturnPressed();
  void search();

private:
  void setupLayout();
  float flexMatch(const QString &haystack, const QString &needle) const;
  QList<QTreeWidgetItem *> flexMatchSections(const QList<Section *> &sections) const;
  QList<QTreeWidgetItem *> flexMatchList(const QListWidget *list, const EntryType type) const;
  QTreeWidgetItem *createCandidate(const QString &text, const EntryType type,
                                   const float similarity, const QVariant data) const;
  void navigateCandidates(const Navigation nav);

  BinaryWidget *binaryWidget = nullptr;

  LineEdit *inputEdit;
  QTreeWidget *candidatesWidget;

  QTimer searchTimer;
  QString input;
};

} // namespace dispar

#endif // SRC_WIDGETS_OMNISEARCHDIALOG_H

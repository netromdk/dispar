#ifndef SRC_WIDGETS_LOGDIALOG_H
#define SRC_WIDGETS_LOGDIALOG_H

#include "LogHandler.h"

#include <QDialog>

class QTreeWidget;

namespace dispar {

class LogDialog : public QDialog {
public:
  LogDialog(QWidget *parent = nullptr);

private slots:
  void addEntry(const LogHandler::Entry &entry);
  void loadEntries();

private:
  void createLayout();

  QTreeWidget *treeWidget = nullptr;
};

} // namespace dispar

#endif // SRC_WIDGETS_LOGDIALOG_H
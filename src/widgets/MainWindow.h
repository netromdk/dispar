#ifndef DISPAR_MAIN_WINDOW_H
#define DISPAR_MAIN_WINDOW_H

#include <QList>
#include <QMainWindow>

class QTabWidget;
class QStringList;
class BinaryWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(const QStringList &files = QStringList());
  virtual ~MainWindow();

protected:
  void showEvent(QShowEvent *event);

private slots:
  void openBinary();
  //void saveBinary();
  //void closeBinary();

private:
  /*
  void createLayout();
  void createMenu();
  */

  void loadBinary(QString file);

  bool shown, modified;
  QStringList startupFiles;

  QTabWidget *tabWidget;
  QList<BinaryWidget*> binaryWidgets;
};

#endif // DISPAR_MAIN_WINDOW_H

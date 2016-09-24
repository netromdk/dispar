#ifndef DISPAR_MAIN_WINDOW_H
#define DISPAR_MAIN_WINDOW_H

#include <QList>
#include <QMainWindow>

class QTabWidget;
class BinaryWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(const QString &file = QString());
  virtual ~MainWindow();

protected:
  void showEvent(QShowEvent *event);

private slots:
  void openBinary();
  // void saveBinary();
  // void closeBinary();

  void onRecentFile();
  void onAbout();

private:
  void readSettings();
  void createLayout();
  void createMenu();

  void loadBinary(QString file);

  bool shown, modified;
  QString startupFile;
  QStringList recentFiles;
  QByteArray geometry;

  BinaryWidget *binaryWidget;
};

#endif // DISPAR_MAIN_WINDOW_H

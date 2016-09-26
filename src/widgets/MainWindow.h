#ifndef DISPAR_MAIN_WINDOW_H
#define DISPAR_MAIN_WINDOW_H

#include <QList>
#include <QMainWindow>

#include <memory>

#include "../CpuType.h"

class Format;
class FormatLoader;
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
  void onConversionHelper();
  void onAbout();

  void onLoadSuccess(std::shared_ptr<Format> fmt);

private:
  void setTitle(const QString &file = QString(), CpuType type = CpuType::X86);
  void readSettings();
  void createLayout();
  void createMenu();
  void loadBinary(QString file);

  bool modified;
  QString startupFile;
  QStringList recentFiles;
  QByteArray geometry;

  std::unique_ptr<FormatLoader> loader;
};

#endif // DISPAR_MAIN_WINDOW_H

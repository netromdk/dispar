#ifndef DISPAR_MAIN_WINDOW_H
#define DISPAR_MAIN_WINDOW_H

#include <QList>
#include <QMainWindow>

#include <memory>

#include "CpuType.h"

class QAction;

namespace dispar {

class Format;
class FormatLoader;
class BinaryWidget;
class BinaryObject;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(const QString &file = QString());
  virtual ~MainWindow();

protected:
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

private slots:
  void newProject();
  void openProject(const QString &projectFile = QString());

  /// Saves/saves as project to file.
  /** Returns true if it was saved to file and not canceled or failed to. */
  bool saveProject();

  void closeProject();
  void openBinary();

  /// Commits modified regions to the binary file.
  /** Returns true if it was saved to file and not canceled or failed to. */
  bool saveBinary();

  void reloadBinary();

  void onRecentProject();
  void onRecentBinary();
  void onConversionHelper();
  void onDisassembler();
  void onAbout();
  void onOptions();
  void onLog();
  void onLoadSuccess(std::shared_ptr<Format> fmt);
  void onProjectModified();
  void onBinaryModified();

private:
  void setTitle(const QString &file = QString());
  void createLayout();
  void createMenu();
  void loadBinary(QString file);
  void saveBackup(const QString &file);

  /// If project is modified ask to save.
  /** Returns false if canceled. */
  bool checkSave();

  /// If binary is modified ask to save.
  /** Returns false if canceled. */
  bool checkSaveBinary();

  /// Apply any saved modified regions from project to \p object.
  void applyModifiedRegions(BinaryObject *object);

  bool modified, binaryModified;
  QString startupFile;

  QAction *newProjectAction, *saveProjectAction, *saveAsProjectAction, *closeProjectAction,
    *saveBinaryAction, *reloadBinaryAction;

  std::unique_ptr<FormatLoader> loader;
  std::shared_ptr<Format> format;
};

} // namespace dispar

#endif // DISPAR_MAIN_WINDOW_H

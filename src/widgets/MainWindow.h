#ifndef DISPAR_MAIN_WINDOW_H
#define DISPAR_MAIN_WINDOW_H

#include <QList>
#include <QMainWindow>
#include <QPointer>

#include <memory>

#include "CpuType.h"

class QAction;

namespace dispar {

class Format;
class FormatLoader;
class BinaryWidget;
class BinaryObject;
class OmniSearchDialog;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(const QString &file = QString());
  ~MainWindow() override;

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
  void reloadBinaryUi();

  void loadFile(const QString &file);

  void omniSearch();

  void onRecentProject();
  void onRecentBinary();
  void onConversionHelper();
  void onDisassembler();
  static void onAbout();
  static void onOptions();
  void onLog();
  void onLoadSuccess(const std::shared_ptr<Format> &fmt);
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

  bool modified = false, binaryModified = false;
  QString startupFile;

  QAction *newProjectAction = nullptr, *saveProjectAction = nullptr, *saveAsProjectAction = nullptr,
          *closeProjectAction = nullptr, *saveBinaryAction = nullptr, *reloadBinaryAction = nullptr,
          *reloadBinaryUiAction = nullptr, *omniSearchAction = nullptr;

  std::unique_ptr<FormatLoader> loader;
  std::shared_ptr<Format> format;

  QPointer<BinaryWidget> binaryWidget;
  QPointer<OmniSearchDialog> omniSearchDialog;
};

} // namespace dispar

#endif // DISPAR_MAIN_WINDOW_H

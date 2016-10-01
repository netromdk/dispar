#ifndef DISPAR_PROJECT_H
#define DISPAR_PROJECT_H

#include <QString>

#include <memory>

/// This class represents a project (saved with extension .dispar).
/** It contains information about the binary, machine code modifications, custom tags, comments
    etc. */
class Project {
public:
  Project();
  ~Project();

  /// Load project from \p file.
  /** Returns \p nullptr if nonexistent or failed. */
  static std::shared_ptr<Project> load(const QString &file);

  /// Save project to \p file, if specified, otherwise save to \p projectFile().
  /** If no path is resolved then it asks with a dialog. */
  bool save(const QString &file = QString());

  QString binary() const;
  void setBinary(const QString &file);

  QString project() const;

private:
  QString binary_, project_;
};

#endif // DISPAR_PROJECT_H

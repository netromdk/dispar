#ifndef DISPAR_PROJECT_H
#define DISPAR_PROJECT_H

#include <QByteArray>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

namespace dispar {

/// This class represents a project (saved with extension .dispar).
/** It contains information about the binary, machine code modifications, custom tags, comments
    etc. */
class Project : public QObject {
  Q_OBJECT

public:
  Project();
  ~Project() override;

  Project(const Project &other) = delete;
  Project &operator=(const Project &rhs) = delete;

  Project(Project &&other) = delete;
  Project &operator=(Project &&rhs) = delete;

  /// Load project from \p file.
  /** Returns \p nullptr if nonexistent or failed. */
  static std::unique_ptr<Project> load(const QString &file);

  /// Save project to \p file, if specified, otherwise save to \p file().
  bool save(const QString &path = QString());

  [[nodiscard]] QString binary() const;
  void setBinary(const QString &file);

  [[nodiscard]] QString file() const;

  /// Tags for a specified address.
  [[nodiscard]] QStringList addressTags(quint64 address) const;

  /// All tags.
  [[nodiscard]] const QHash<quint64, QStringList> &tags() const;

  /// Associates \p tag with \p address.
  /** Checks if tag is not taken already, too. */
  bool addAddressTag(const QString &tag, quint64 address);

  /// Removes \p tag.
  /** Returns true if removed. */
  bool removeAddressTag(const QString &tag);

  /// Removes \p tags.
  /** Returns true if anything was removed. */
  bool removeAddressTags(const QStringList &tags);

  /// All modified regions.
  [[nodiscard]] const QMap<quint64, QByteArray> &modifiedRegions() const;

  /// Add modified region \p data at absolute \p address.
  /** This is only used to be able to save to project file. */
  void addModifiedRegion(quint64 address, const QByteArray &data);

  /// Remove all modified regions from project.
  void clearModifiedRegions();

signals:
  /// Whenever something is modified in the project this signal is emitted.
  void modified();

  /// When tags are added or removed this signal is emitted.
  void tagsChanged();

private:
  QString binary_, file_;
  QHash<quint64, QStringList> addressTags_;

  // Absolute binary address and the data to write from there. The use of QMap is intentional for
  // sorted of keys to write data linearly.
  QMap<quint64, QByteArray> modifiedRegions_;
};

} // namespace dispar

#endif // DISPAR_PROJECT_H

#ifndef DISPAR_CSTRING_READER_H
#define DISPAR_CSTRING_READER_H

#include <QString>
#include <QStringList>

class QByteArray;

namespace dispar {

class CStringReader {
public:
  CStringReader(const QByteArray &data);

  /// Tries to read a string.
  bool next();

  /// If a string was read it is retrieved here.
  [[nodiscard]] QString string() const;

  /// Offset of string beginning in data block.
  [[nodiscard]] quint64 offset() const;

  /// Read all strings of data.
  QStringList readAll();

private:
  const QByteArray &data;
  int pos;
  QString string_;
  quint64 offset_;
};

} // namespace dispar

#endif // DISPAR_CSTRING_READER_H

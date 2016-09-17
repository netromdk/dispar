#ifndef DISPAR_CSTRING_READER_H
#define DISPAR_CSTRING_READER_H

#include <QString>
#include <QStringList>
#include <QByteArray>

class CStringReader {
public:
  CStringReader(const QByteArray &data);

  /// Tries to read a string.
  bool next();

  /// If a string was read it is retrieved here.
  QString string() const;

  /// Read all strings of data.
  QStringList readAll();

private:
  const QByteArray &data;
  int pos;
  QString string_;
};

#endif // DISPAR_CSTRING_READER_H

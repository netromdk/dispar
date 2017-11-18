#include "CStringReader.h"

#include <QByteArray>

CStringReader::CStringReader(const QByteArray &data) : data(data), pos(0), offset_(0)
{
}

bool CStringReader::next()
{
  auto size = data.size();
  if (pos == size - 1) return false;

  // Clear when trying to read next string, and jump over the \0.
  if (!string_.isEmpty()) {
    string_.clear();
    pos++;
    offset_ = pos;
  }

  // Count the length of the string until \0.
  int len = 0;
  for (; pos < size && data[pos] != 0; pos++, len++)
    ;

  string_ = QString::fromUtf8(data.mid(offset_, len));
  return !string_.isEmpty();
}

QString CStringReader::string() const
{
  return string_;
}

quint64 CStringReader::offset() const
{
  return offset_;
}

QStringList CStringReader::readAll()
{
  QStringList res;
  while (next()) {
    res << string();
  }
  return res;
}

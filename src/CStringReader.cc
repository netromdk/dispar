#include "CStringReader.h"

#include <QByteArray>

namespace dispar {

CStringReader::CStringReader(const QByteArray &data_) : data(data_), pos(0), offset_(0)
{
}

bool CStringReader::next()
{
  // Clear when trying to read next string, and jump over the \0.
  if (!string_.isEmpty()) {
    string_.clear();
    pos++;
    offset_ = pos;
  }

  auto it = data.cbegin() + offset_;
  if (it >= data.cend()) return false;

  // Read until null byte but skip for empty strings until at least one character is read or the end
  // is reached.
  for (; it != data.cend(); it++, pos++) {
    const auto ch = *it;
    if (ch == 0) {
      if (!string_.isEmpty()) {
        break;
      }
    }
    else {
      string_ += ch;
    }
  }

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

} // namespace dispar

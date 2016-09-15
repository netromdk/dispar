#include "Section.h"

Section::Section(Section::Type type, const QString &name, quint64 addr, quint64 size,
                 quint32 offset)
  : type_{type}, name_{name}, addr{addr}, size_{size}, offset_{offset}
{
}

QString Section::typeName(Type type)
{
  switch (type) {
  default:
  case Section::Type::Text:
    return "Text";

  case Section::Type::CString:
    return "CString";
  }
}

Section::Type Section::type() const
{
  return type_;
}

QString Section::name() const
{
  return name_;
}

quint64 Section::address() const
{
  return addr;
}

quint64 Section::size() const
{
  return size_;
}

quint32 Section::offset() const
{
  return offset_;
}

const QByteArray &Section::data() const
{
  return data_;
}

void Section::setData(const QByteArray &data)
{
  data_ = data;
}

void Section::setSubData(const QByteArray &subData, int pos)
{
  if (pos < 0 || pos > data_.size() - 1) {
    return;
  }

  data_.replace(pos, subData.size(), subData);
  modified = QDateTime::currentDateTime();

  QPair<int, int> region(pos, subData.size());
  if (!modifiedRegions_.contains(region)) {
    modifiedRegions_ << region;
  }
}

bool Section::isModified() const
{
  return !modifiedRegions_.isEmpty();
}

QDateTime Section::modifiedWhen() const
{
  return modified;
}

const QList<QPair<int, int>> &Section::modifiedRegions() const
{
  return modifiedRegions_;
}

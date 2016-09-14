#include "Section.h"

Section::Section(Section::Type type, const QString &name, quint64 addr,
                 quint64 size, quint32 offset)
  : type{type}, name{name}, addr{addr}, size{size}, offset{offset}
{ }

Section::Type Section::getType() const {
  return type;
}

QString Section::getName() const {
  return name;
}

quint64 Section::getAddress() const {
  return addr;
}

quint64 Section::getSize() const {
  return size;
}

quint32 Section::getOffset() const {
  return offset;
}

const QByteArray &Section::getData() const {
  return data;
}

void Section::setData(const QByteArray &data) {
  this->data = data;
}

void Section::setSubData(const QByteArray &subData, int pos) {
  if (pos < 0 || pos > data.size() - 1) {
    return;
  }
  data.replace(pos, subData.size(), subData);
  modified = QDateTime::currentDateTime();

  QPair<int, int> region(pos, subData.size());
  if (!modifiedRegions.contains(region)) {
    modifiedRegions << region;
  }
}

bool Section::isModified() const {
  return !modifiedRegions.isEmpty();
}

QDateTime Section::modifiedWhen() const {
  return modified;
}

const QList<QPair<int, int>> &Section::getModifiedRegions() const {
  return modifiedRegions;
}

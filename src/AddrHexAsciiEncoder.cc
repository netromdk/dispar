#include "AddrHexAsciiEncoder.h"
#include "Section.h"
#include "Util.h"

#include <QEventLoop>
#include <QMutexLocker>

#include <algorithm>

namespace dispar {

AddrHexAsciiEncoderTask::AddrHexAsciiEncoderTask(const QByteArray &data, const quint64 address, const quint64 index,
                               const quint64 size)
  : data(data), address(address), index(index), size(size)
{
}

void AddrHexAsciiEncoderTask::run()
{
  const auto subData = data.mid(index, size);
  emit result(index, Util::addrDataString(address + index, subData));
}

AddrHexAsciiEncoder::AddrHexAsciiEncoder(const Section *section) : section(section)
{
  pool.setMaxThreadCount(QThread::idealThreadCount());
}

void AddrHexAsciiEncoder::start(const bool blocking)
{
  tasks = 0;

  quint64 size = 4096 * 32;
  for (quint64 i = 0, n = section->data().size(); i < n; i += size) {
    if (i + size > n) {
      size = n - i;
    }
    tasks++;

    auto *task = new AddrHexAsciiEncoderTask(section->data(), section->address(), i, size);
    connect(task, &AddrHexAsciiEncoderTask::result, this, &AddrHexAsciiEncoder::addResult);

    // Takes ownership and starts task.
    pool.start(task);
  }

  if (blocking) {
    QEventLoop loop;
    connect(this, &AddrHexAsciiEncoder::finished, &loop, &QEventLoop::quit);
    loop.exec();
  }
}

QString AddrHexAsciiEncoder::result() const
{
  return result_;
}

void AddrHexAsciiEncoder::addResult(const quint64 index, const QString &result)
{
  QMutexLocker locker(&mutex);
  results.append({index, result});

  if (results.size() == tasks) {
    std::sort(results.begin(), results.end(),
              [](const auto &a, const auto &b) { return a.index < b.index; });
    QStringList totals;
    for (const auto &result : results) {
      totals.append(result.data);
    }
    result_ = totals.join("\n");

    emit finished();
  }
}

} // namespace dispar

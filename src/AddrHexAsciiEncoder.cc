#include "AddrHexAsciiEncoder.h"
#include "Section.h"
#include "Util.h"

#include <QEventLoop>
#include <QMutexLocker>
#include <QThread>
#include <QTimer>

#include <algorithm>

namespace dispar {

AddrHexAsciiEncoderTask::AddrHexAsciiEncoderTask(const QByteArray &data_, const quint64 address_,
                                                 const quint64 index_, const quint64 size_)
  : data(data_), address(address_), index(index_), size(size_)
{
}

void AddrHexAsciiEncoderTask::run()
{
  const auto subData = data.mid(index, size);
  emit result(index, Util::addrDataString(address + index, subData));
}

AddrHexAsciiEncoder::AddrHexAsciiEncoder(const Section *section_) : section(section_)
{
  pool.setMaxThreadCount(QThread::idealThreadCount());
}

void AddrHexAsciiEncoder::start(const bool blocking)
{
  tasks = 0;

  const int threads = QThread::idealThreadCount();
  quint64 size = threads * 1024;
  for (quint64 i = 0, n = section->data().size(); i < n; i += size) {
    if (i + size > n) {
      size = n - i;
    }
    tasks++;

    // The thread pool handles the destruction of the task, because AddrHexAsciiEncoder has
    // QRunnable::autoDelete() enabled by default, so it isn't necessary to use std::unique_ptr or
    // similar. NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto *task = new AddrHexAsciiEncoderTask(section->data(), section->address(), i, size);

    // It is very important that it is a blocking queued connection such that the task isn't
    // destroyed before the signal is processed.
    connect(task, &AddrHexAsciiEncoderTask::result, this, &AddrHexAsciiEncoder::addResult,
            Qt::BlockingQueuedConnection);

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

  bool done = false;
  if (results.size() == tasks) {
    std::sort(results.begin(), results.end(),
              [](const auto &a, const auto &b) { return a.index < b.index; });

    QStringList totals;
    for (const auto &res : results) {
      totals.append(res.data);
    }
    result_ = totals.join("\n");
    done = true;
  }

  locker.unlock();
  if (done) {
    // Emit finished afte returning from this function because the task will only get destroyed
    // afterwards due to Qt::BlockingQueuedConnection.
    QTimer::singleShot(1, [this] {
      pool.waitForDone(); // Wait for threads to get destroyed.
      emit finished();
    });
  }
}

} // namespace dispar

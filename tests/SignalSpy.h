#ifndef DISPAR_SIGNAL_SPY_H
#define DISPAR_SIGNAL_SPY_H

#include "gtest/gtest.h"

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QThread>

#include <cmath>
#include <memory>

class SignalSpy {
public:
  SignalSpy(const char *file, int line) : file(file), line(line), count_(0), expect(true)
  {
  }

  ~SignalSpy()
  {
    if (expect != wasTriggered()) {
      ADD_FAILURE_AT(file, line) << "SignalSpy was triggered: " << wasTriggered()
                                 << " Expect trigger: " << expect;
    }
  }

  void setExpect(bool expect)
  {
    this->expect = expect;
  }

  bool wasTriggered()
  {
    return count() > 0;
  }

  void increment()
  {
    count_++;
  }

  int count()
  {
    return count_;
  }

  void wait(int maxWait = 1000)
  {
    int waited = 0, step = 2, cnt = 1;
    while (!wasTriggered() && waited < maxWait) {
      QCoreApplication::processEvents();
      QThread::msleep(step);
      waited += step;
      step = pow(2, cnt++);
    }
  }

  template <typename Arg, typename Inst, typename MemFunc>
  static std::shared_ptr<SignalSpy> one(const char *file, int line, Inst *instance,
                                        MemFunc Inst::*mf,
                                        std::function<void(Arg)> slot = [](Arg arg) {})
  {
    auto spy = std::make_shared<SignalSpy>(file, line);
    auto weakSpy = std::weak_ptr<SignalSpy>(spy);
    QObject::connect(instance, mf, [weakSpy, slot](Arg arg) {
      if (auto ptr = weakSpy.lock()) {
        ptr->increment();
        slot(arg);
      }
    });
    return spy;
  }

private:
  const char *file;
  int line, count_;
  bool expect, triggered;
};

#endif // DISPAR_SIGNAL_SPY_H

#ifndef DISPAR_CONTEXT_H
#define DISPAR_CONTEXT_H

#include <QObject>

class Context : public QObject {
  Q_OBJECT

public:
  ~Context();

  // Singleton instance.
  static Context &get();

  bool showMachineCode() const;
  void setShowMachineCode(bool show);

signals:
  void showMachineCodeChanged(bool show);

private:
  Context();

  bool showMachineCode_;
};

#endif // DISPAR_CONTEXT_H

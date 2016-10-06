#ifndef DISPAR_TOGGLE_BOX_H
#define DISPAR_TOGGLE_BOX_H

#include <QWidget>

class QScrollArea;
class QLayout;
class QToolButton;

class ToggleBox : public QWidget {
  Q_OBJECT

public:
  ToggleBox(const QString &title, const QString &settingsKey, QWidget *parent = nullptr);
  ~ToggleBox();

  void setContentLayout(QLayout *layout);

  void setCollapsed(bool collapsed = true);
  void setExpanded(bool expanded = true);

  bool isCollapsed() const;
  bool isExpanded() const;

signals:
  void expanded();
  void collapsed();
  void stateChanged(bool collapsed);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();

  QString title, settingsKey;
  QToolButton *toggleButton;
  QScrollArea *contentWidget;
};

#endif // DISPAR_TOGGLE_BOX_H

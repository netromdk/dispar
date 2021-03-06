#ifndef SRC_WIDGETS_CENTER_LABEL_H
#define SRC_WIDGETS_CENTER_LABEL_H

#include <QLabel>

namespace dispar {

class CenterLabel : public QLabel {
  Q_OBJECT

public:
  CenterLabel(const QString &text, QWidget *parent = nullptr);
  ~CenterLabel() override;

  CenterLabel(const CenterLabel &other) = delete;
  CenterLabel &operator=(const CenterLabel &rhs) = delete;

  CenterLabel(CenterLabel &&other) = delete;
  CenterLabel &operator=(CenterLabel &&rhs) = delete;

signals:
  void droppedFileName(const QString &fileName);

protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
};

} // namespace dispar

#endif // SRC_WIDGETS_CENTER_LABEL_H

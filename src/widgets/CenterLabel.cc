#include "widgets/CenterLabel.h"

#include <QDragEnterEvent>
#include <QMimeData>

namespace dispar {

CenterLabel::CenterLabel(const QString &text, QWidget *parent) : QLabel(text, parent)
{
  setAcceptDrops(true);
  setAlignment(Qt::AlignCenter);

  auto f = font();
  f.setPointSize(24);
  f.setBold(true);
  setFont(f);
}

CenterLabel::~CenterLabel()
{
}

void CenterLabel::dragEnterEvent(QDragEnterEvent *event)
{
  const auto *mime = event->mimeData();
  if (!mime || !mime->hasUrls()) {
    event->ignore();
    return;
  }

  const auto urls = mime->urls();
  if (urls.size() != 1) {
    event->ignore();
    return;
  }

  if (!urls.first().isLocalFile()) {
    event->ignore();
    return;
  }

  event->acceptProposedAction();
}

void CenterLabel::dropEvent(QDropEvent *event)
{
  const auto *mime = event->mimeData();
  const auto url = mime->urls().first();
  emit droppedFileName(url.toLocalFile());
}

} // namespace dispar

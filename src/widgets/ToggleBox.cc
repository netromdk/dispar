#include "ToggleBox.h"

#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

ToggleBox::ToggleBox(const QString &title, QWidget *parent) : QWidget(parent), title(title)
{
  createLayout();
  setCollapsed();
}

void ToggleBox::setContentLayout(QLayout *layout)
{
  Q_ASSERT(layout);
  if (contentWidget->layout()) {
    delete contentWidget->layout();
  }

  contentWidget->setLayout(layout);

  // Set new maximum height to match tool button height and content layout height.
  auto collapsedHeight = sizeHint().height() - contentWidget->maximumHeight();
  auto contentHeight = layout->sizeHint().height();
  setMaximumHeight(collapsedHeight + contentHeight);
}

void ToggleBox::setCollapsed(bool collapsed)
{
  toggleButton->setArrowType(collapsed ? Qt::ArrowType::RightArrow : Qt::ArrowType::DownArrow);
  toggleButton->setChecked(!collapsed);

  if (collapsed) {
    contentWidget->setMinimumHeight(0);
    contentWidget->setMaximumHeight(0);
  }
  else {
    contentWidget->setMinimumHeight(minimumHeight());
    contentWidget->setMaximumHeight(maximumHeight());
  }
}

void ToggleBox::setExpanded(bool expanded)
{
  setCollapsed(!expanded);
}

void ToggleBox::createLayout()
{
  toggleButton = new QToolButton;
  toggleButton->setStyleSheet("QToolButton { border: none; }");
  toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  toggleButton->setText(title);
  toggleButton->setCheckable(true);
  toggleButton->setChecked(false);

  connect(toggleButton, &QToolButton::clicked, this,
          [this](bool checked) { setCollapsed(!checked); });

  contentWidget = new QScrollArea;
  contentWidget->setFrameShadow(QFrame::Plain);
  contentWidget->setFrameShape(QFrame::NoFrame);
  contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto pal = contentWidget->palette();
  pal.setBrush(QPalette::Window, pal.window().color().darker(105));
  contentWidget->setPalette(pal);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(toggleButton);
  layout->addWidget(contentWidget);

  setLayout(layout);
}

#include "gui/FluidLayout.hpp"

FluidLayout::FluidLayout(QWidget* parent, int margin,
                         int hSpacing, int vSpacing)
  : FlowLayout{parent, margin, hSpacing, vSpacing}, lineCount{1}
{}

FluidLayout::FluidLayout(int margin, int hSpacing, int vSpacing)
  : FlowLayout{margin, hSpacing, vSpacing}, lineCount{1}
{
  setContentsMargins(margin, margin, margin, margin);
}

int FluidLayout::heightForWidth(int width) const
{
  const auto height = checkLayout(QRect{0, 0, width, 0});

  return height;
}

void FluidLayout::setGeometry(const QRect& rect)
{
  QLayout::setGeometry(rect);
  doLayout(rect);
}

QSize FluidLayout::minimumSize() const
{
  QSize size{};

  for (auto item : itemList)
  {
    size = size.expandedTo(item->minimumSize());
  }

  // Account for number of lines
  size.setHeight(size.height() * lineCount);
  size += QSize{2 * lineCount * margin(), 2 * lineCount * margin()};

  return size;
}

void FluidLayout::doLayout(const QRect& rect)
{
  int left, top, right, bottom;
  getContentsMargins(&left, &top, &right, &bottom);
  auto effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
  auto x = effectiveRect.x();
  auto y = effectiveRect.y();
  int lineHeight = 0;

  auto testLineCount = 1;

  for (auto item : itemList)
  {
    auto wid = item->widget();

    auto spaceX = horizontalSpacing();
    if (spaceX == -1)
    {
      spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                           QSizePolicy::PushButton,
                                           Qt::Horizontal);
    }

    auto spaceY = verticalSpacing();
    if (spaceY == -1)
    {
      spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                           QSizePolicy::PushButton,
                                           Qt::Vertical);
    }

    auto nextX = x + item->sizeHint().width() + spaceX;
    if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
    {
      x = effectiveRect.x();
      y = y + lineHeight + spaceY;
      nextX = x + item->sizeHint().width() + spaceX;
      lineHeight = 0;

      ++testLineCount;
    }

    item->setGeometry(QRect{QPoint{x, y}, item->sizeHint()});

    x = nextX;
    lineHeight = qMax(lineHeight, item->sizeHint().height());
  }

  if (testLineCount != lineCount)
  {
    lineCount = testLineCount;
    invalidate();
  }
}

int FluidLayout::checkLayout(const QRect& rect) const
{
  int left, top, right, bottom;
  getContentsMargins(&left, &top, &right, &bottom);
  auto effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
  auto x = effectiveRect.x();
  auto y = effectiveRect.y();
  int lineHeight = 0;

  for (auto item : itemList)
  {
    auto wid = item->widget();

    auto spaceX = horizontalSpacing();
    if (spaceX == -1)
    {
      spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                           QSizePolicy::PushButton,
                                           Qt::Horizontal);
    }

    auto spaceY = verticalSpacing();
    if (spaceY == -1)
    {
      spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                           QSizePolicy::PushButton,
                                           Qt::Vertical);
    }

    auto nextX = x + item->sizeHint().width() + spaceX;
    if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
    {
      x = effectiveRect.x();
      y = y + lineHeight + spaceY;
      nextX = x + item->sizeHint().width() + spaceX;
      lineHeight = 0;
    }

    x = nextX;
    lineHeight = qMax(lineHeight, item->sizeHint().height());
  }

  return y + lineHeight - rect.y() + bottom;
}
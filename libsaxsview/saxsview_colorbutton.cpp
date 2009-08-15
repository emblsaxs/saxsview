#include "saxsview_colorbutton.h"

#include <QBrush>
#include <QColor>
#include <QColorDialog>
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QResizeEvent>

namespace Saxsview {

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent) {
  connect(this, SIGNAL(clicked()), SLOT(getColor()));
  setColor(Qt::white);
}

ColorButton::~ColorButton() {
}

QColor ColorButton::color() const {
  return mColor;
}

void ColorButton::getColor() {
  QColor color = QColorDialog::getColor(mColor, this);
  if (color.isValid())
    setColor(color);
}

void ColorButton::setColor(const QColor& color) {
  if (mColor != color) {
    mColor = color;
    updateIcon();
    emit colorChanged(mColor);
  }
}

void ColorButton::resizeEvent(QResizeEvent *) {
  updateIcon();
}

void ColorButton::updateIcon() {
  QPixmap pixmap(width() * 0.40, 16);

  QPainter painter;
  painter.begin(&pixmap);
  painter.setBrush(QBrush(mColor));
  painter.setPen(QPen(Qt::black));
  painter.drawRect(pixmap.rect());
  painter.end();

  QIcon icon(pixmap);
  setIconSize(pixmap.size());
  setIcon(icon);
}

} // end of namespace Saxsview

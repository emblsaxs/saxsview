/*
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of saxsview.
 *
 * saxsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * saxsview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with saxsview. If not, see <http://www.gnu.org/licenses/>.
 */

#include "saxsview_colorbutton.h"

#include <QtGui>

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

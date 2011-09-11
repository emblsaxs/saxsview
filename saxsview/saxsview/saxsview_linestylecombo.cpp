/*
 * Copyright (C) 2010 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "saxsview_linestylecombo.h"

static QIcon penStyleIcon(Qt::PenStyle style) {
  QPixmap pixmap(16, 16);

  QPen pen;
  pen.setColor(Qt::black);
  pen.setStyle(style);
  pen.setWidth(1);

  // FIXME: Use palette or style or whatever to get the colors right
  QPainter painter;
  painter.begin(&pixmap);
  painter.setPen(QPen(Qt::NoPen));
  painter.fillRect(pixmap.rect(), QBrush(Qt::white));
  painter.setPen(pen);
  painter.drawLine(0, 16, 16, 0);
  painter.end();

  return QIcon(pixmap);
}


LineStyleCombo::LineStyleCombo(QWidget *parent)
 : QComboBox(parent) {

#define ADD_ITEM(style, text) \
  addItem(penStyleIcon(style), text, style)

  ADD_ITEM(Qt::NoPen, "none");
  ADD_ITEM(Qt::SolidLine, "solid");
  ADD_ITEM(Qt::DashLine, "dashed");
  ADD_ITEM(Qt::DotLine, "dotted");
  ADD_ITEM(Qt::DashDotLine, "dash-dot");
  ADD_ITEM(Qt::DashDotDotLine, "dash-dot-dot");

#undef ADD_ITEM
}

Qt::PenStyle LineStyleCombo::currentStyle() const {
  return (Qt::PenStyle) itemData(currentIndex()).toInt();
}

void LineStyleCombo::setCurrentStyle(int style) {
  setCurrentIndex(findData(style));
}

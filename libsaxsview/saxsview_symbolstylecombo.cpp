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

#include "saxsview_symbolstylecombo.h"
using Saxsview::PlotSymbol;


static QIcon symbolStyleIcon(PlotSymbol::Style style) {
  QPixmap pixmap(16, 16);

  // FIXME: Use palette or style or whatever to get the colors right
  QPainter painter;
  painter.begin(&pixmap);
  painter.setPen(QPen(Qt::NoPen));
  painter.fillRect(pixmap.rect(), QBrush(Qt::white));

  PlotSymbol symbol;
  symbol.setSize(10);
  symbol.setColor(Qt::black);
  symbol.setStyle(style);
  symbol.qwtSymbol()->drawSymbol(&painter, QPointF(7, 7));

  painter.end();

  return QIcon(pixmap);
}


SymbolStyleCombo::SymbolStyleCombo(QWidget *parent)
 : QComboBox(parent) {

#define ADD_ITEM(style, text) \
  addItem(symbolStyleIcon(style), text, style)

  ADD_ITEM(PlotSymbol::NoSymbol, "none");
  insertSeparator(1);
  ADD_ITEM(PlotSymbol::Ellipse, "circle");
  ADD_ITEM(PlotSymbol::Rect, "rectangle");
  ADD_ITEM(PlotSymbol::Diamond, "diamond");
  ADD_ITEM(PlotSymbol::DTriangle, "triangle (down)");
  ADD_ITEM(PlotSymbol::UTriangle, "triangle (up)");
  ADD_ITEM(PlotSymbol::LTriangle, "triangle (left)");
  ADD_ITEM(PlotSymbol::RTriangle, "triangle (right)");
  ADD_ITEM(PlotSymbol::Star2, "star (outline)");
  ADD_ITEM(PlotSymbol::Hexagon, "hexagon");
  insertSeparator(12);
  ADD_ITEM(PlotSymbol::Cross, "cross");
  ADD_ITEM(PlotSymbol::XCross, "cross (diagonal)");
  ADD_ITEM(PlotSymbol::HLine, "line (horizontal)");
  ADD_ITEM(PlotSymbol::VLine, "line (vertical)");
  ADD_ITEM(PlotSymbol::Star1, "star");
  insertSeparator(18);
  ADD_ITEM(PlotSymbol::FilledEllipse, "circle");
  ADD_ITEM(PlotSymbol::FilledRect, "rectangle");
  ADD_ITEM(PlotSymbol::FilledDiamond, "diamond");
  ADD_ITEM(PlotSymbol::FilledDTriangle, "triangle (down)");
  ADD_ITEM(PlotSymbol::FilledUTriangle, "triangle (up)");
  ADD_ITEM(PlotSymbol::FilledLTriangle, "triangle (left)");
  ADD_ITEM(PlotSymbol::FilledRTriangle, "triangle (right)");
  ADD_ITEM(PlotSymbol::FilledStar2, "star");
  ADD_ITEM(PlotSymbol::FilledHexagon, "hexagon");

#undef ADD_ITEM
}

PlotSymbol::Style SymbolStyleCombo::currentStyle() const {
  return (PlotSymbol::Style) itemData(currentIndex()).toInt();
}

void SymbolStyleCombo::setCurrentStyle(int style) {
  setCurrentIndex(findData(style));
}

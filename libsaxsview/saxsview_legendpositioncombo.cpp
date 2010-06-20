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

#include "saxsview_legendpositioncombo.h"

LegendPositionCombo::LegendPositionCombo(QWidget *parent)
 : QComboBox(parent) {

#define ADD_ITEM(position, text) \
  addItem(text, position)

  ADD_ITEM(QwtPlot::ExternalLegend, "Inside the plot area");
  ADD_ITEM(QwtPlot::RightLegend, "Right of the plot");
  ADD_ITEM(QwtPlot::LeftLegend, "Left of the plot");
  ADD_ITEM(QwtPlot::BottomLegend, "Below the plot");
  ADD_ITEM(QwtPlot::TopLegend, "Above the plot");

#undef ADD_ITEM
}

QwtPlot::LegendPosition LegendPositionCombo::currentLegendPosition() const {
  return (QwtPlot::LegendPosition) itemData(currentIndex()).toInt();
}

void LegendPositionCombo::setCurrentLegendPosition(QwtPlot::LegendPosition style) {
  setCurrentIndex(findData(style));
}

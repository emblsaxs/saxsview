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

#ifndef SAXSVIEW_LEGENDPOSITIONCOMBO_H
#define SAXSVIEW_LEGENDPOSITIONCOMBO_H

#include <QtGui>
#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_layout.h"


class LegendPositionCombo : public QComboBox {
  Q_OBJECT

public:
  LegendPositionCombo(QWidget *parent = 0L);

  QwtPlot::LegendPosition currentLegendPosition() const;
  void setCurrentLegendPosition(QwtPlot::LegendPosition position);
};

#endif // !SAXSVIEW_LEGENDPOSITIONCOMBO_H

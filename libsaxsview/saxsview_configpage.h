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

#ifndef SAXSVIEW_CONFIGPAGE_H
#define SAXSVIEW_CONFIGPAGE_H

#include "ui_saxsview_configpage_plot.h"
#include "ui_saxsview_configpage_legend.h"

#include <QWidget>

namespace Saxsview {

class Plot;

class AbstractConfigPage : public QWidget {
public:
  AbstractConfigPage(QWidget *parent = 0L);
  virtual ~AbstractConfigPage() = 0;
  virtual void apply() = 0;
  virtual void reset() = 0;
};

//-------------------------------------------------------------------------
class PlotConfigPage : public AbstractConfigPage,
                       private Ui::PlotConfigPage {
public:
  PlotConfigPage(Plot *plot, QWidget *parent = 0L);
  void apply();
  void reset();

private:
  Plot *plot;
};

//-------------------------------------------------------------------------
class LegendConfigPage : public AbstractConfigPage ,
                         private Ui::LegendConfigPage {
public:
  LegendConfigPage(Plot *plot, QWidget *parent = 0L);
  void apply();
  void reset();

private:
  Plot *plot;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_CONFIGPAGE_H

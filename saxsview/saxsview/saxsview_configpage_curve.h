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

#ifndef SAXSVIEW_CONFIGPAGE_CURVE_H
#define SAXSVIEW_CONFIGPAGE_CURVE_H

#include "saxsview_configpage.h"
#include "ui_saxsview_configpage_curve.h"

namespace Saxsview {

class CurveConfigPage : public AbstractConfigPage,
                        private Ui::CurveConfigPage {
Q_OBJECT

public:
  CurveConfigPage(Plot *plot, QWidget *parent = 0L);
  ~CurveConfigPage();
  void apply();
  void reset();

private slots:
  void applyTemplate();
  void setCurrentIndex(int);

private:
  class CurveConfigPagePrivate;
  CurveConfigPagePrivate *p;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_CONFIGPAGE_CURVE_H

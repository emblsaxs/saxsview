/*
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SAXSVIEW_PLOTCURVE_H
#define SAXSVIEW_PLOTCURVE_H

#include <QObject>
#include <QRect>

#include <qwt_series_data.h>

namespace Saxsview {

class Plot;

typedef QwtArray<QwtDoublePoint> PlotPointData;
typedef QwtArray<QwtIntervalSample> PlotIntervalData;

/**
 *
 * @note In Qwt-terms, PlotCurve is not one but two curves.
 *       One for the data points (@ref QwtPlotCurve) and one
 *       for error bars (@ref QwtPlotIntervalCurve).
 */
class PlotCurve : public QObject {
  Q_OBJECT

public:
  PlotCurve(QObject *parent = 0L);
  ~PlotCurve();

  void attach(Plot *plot);
  void detach();

  void setData(const PlotPointData& points = PlotPointData(),
               const PlotIntervalData& intervals = PlotIntervalData());

  QString fileName() const;
  QString title() const;
  bool isVisible() const;
  bool errorBarsEnabled() const;

  QRectF boundingRect() const;

  double scalingFactorX() const;
  double scalingFactorY() const;

public slots:
  void setFileName(const QString&);
  void setTitle(const QString& title);
  void setVisible(bool on);
  void setErrorBarsEnabled(bool on);

  void setScalingFactorX(double);
  void setScalingFactorY(double);

private:
  class PlotCurvePrivate;
  PlotCurvePrivate *p;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_PLOTCURVE_H

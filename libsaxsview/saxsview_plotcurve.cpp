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

#include "saxsview_plotcurve.h"
#include "saxsview_plot.h"

#include <qwt_interval_symbol.h>
#include <qwt_legend_item.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_symbol.h>

namespace Saxsview {

class PlotCurve::PlotCurvePrivate {
public:
  PlotCurvePrivate();

  QwtPlotCurve *curve;
  QwtPlotIntervalCurve *errorCurve;

  bool errorBarsEnabled;
};

PlotCurve::PlotCurvePrivate::PlotCurvePrivate()
 : curve(0L), errorCurve(0L), errorBarsEnabled(true) {

  // data points
  QwtSymbol symbol;
//  symbol.setStyle(QwtSymbol::Cross);
//  symbol.setPen(QPen(color));
  symbol.setSize(2);

  curve = new QwtPlotCurve;
  curve->setStyle(QwtPlotCurve::NoCurve);
  curve->setSymbol(symbol);

  // error bars
  QwtIntervalSymbol errorBar(QwtIntervalSymbol::Bar);
  errorBar.setWidth(1);
//   errorBar.setPen(Qt::black);

  errorCurve = new QwtPlotIntervalCurve;
  errorCurve->setCurveStyle(QwtPlotIntervalCurve::NoCurve);
//  errorCurve->setPen();
  errorCurve->setSymbol(errorBar);
}


PlotCurve::PlotCurve(QObject *parent)
 : QObject(parent), p(new PlotCurvePrivate) {
}

PlotCurve::~PlotCurve() {
  delete p;
}

void PlotCurve::attach(Plot *plot) {
  p->curve->attach(plot);
  p->errorCurve->attach(plot);

  if (QwtLegendItem* legendItem = qobject_cast<QwtLegendItem*>(plot->legend()->find(p->curve)))
    legendItem->setIdentifierWidth(20);
}

void PlotCurve::detach() {
  p->curve->detach();
  p->errorCurve->detach();
}

void PlotCurve::setData(const PlotPointData& points,
                        const PlotIntervalData& intervals) {
  p->curve->setData(points);
  p->errorCurve->setData(intervals);
}

bool PlotCurve::errorBarsEnabled() const {
  return p->errorBarsEnabled;
}

void PlotCurve::setErrorBarsEnabled(bool on) {
  p->errorBarsEnabled = on;
  p->errorCurve->setVisible(on);
}

bool PlotCurve::isVisible() const {
  return p->curve->isVisible();
}

void PlotCurve::setVisible(bool on) {
  p->curve->setVisible(on);
  p->errorCurve->setVisible(on && p->errorBarsEnabled);
}

QRect PlotCurve::boundingRect() const {
  return p->errorBarsEnabled
               ? p->errorCurve->boundingRect().toRect()
               : p->curve->boundingRect().toRect();
}

void PlotCurve::setTitle(const QString& title) {
  p->curve->setTitle(title);
}

} // end of namespace Saxsview

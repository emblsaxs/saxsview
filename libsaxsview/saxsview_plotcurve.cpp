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

PlotSymbol::PlotSymbol() {
  setStyle(NoSymbol);
  setSize(0);
  setColor(Qt::black);
}

PlotSymbol::PlotSymbol(Style style, int size, const QColor& color) {
  setStyle(style);
  setSize(size);
  setColor(color);
}

QColor PlotSymbol::color() const {
  return mSymbol.pen().color();
}

void PlotSymbol::setColor(const QColor& color) {
  mSymbol.setPen(QPen(color));
  if (mSymbol.brush() != Qt::NoBrush)
    mSymbol.setBrush(QBrush(color));
}

int PlotSymbol::size() const {
  return mSymbol.size().width();
}

void PlotSymbol::setSize(int size) {
  mSymbol.setSize(size);
}

PlotSymbol::Style PlotSymbol::style() const {
  return (Style)(mSymbol.style() + (mSymbol.brush() == Qt::NoBrush ? 0 : 100));
}

void PlotSymbol::setStyle(Style s) {
  if (s >= 100) {
    mSymbol.setStyle((QwtSymbol::Style)(s - 100));
    mSymbol.setBrush(QBrush(color()));
  } else {
    mSymbol.setStyle((QwtSymbol::Style)(s));
    mSymbol.setBrush(QBrush(Qt::NoBrush));
  }
}

const QwtSymbol& PlotSymbol::qwtSymbol() const {
  return mSymbol;
}




class PlotCurve::PlotCurvePrivate {
public:
  PlotCurvePrivate();
  ~PlotCurvePrivate();

  void scale();

  QwtPlotCurve *curve;
  QwtPlotIntervalCurve *errorCurve;
  PlotSymbol curveSymbol;

  PlotPointData *pointData;
  PlotIntervalData *intervalData;
  double scaleX, scaleY;

  bool errorBarsEnabled;
  QString fileName;
};

PlotCurve::PlotCurvePrivate::PlotCurvePrivate()
 : curve(0L), errorCurve(0L), pointData(0L), intervalData(0L),
   scaleX(1.0), scaleY(1.0), errorBarsEnabled(true) {

  // TODO: Read default values

  // data points
  QwtSymbol symbol;
//  symbol.setStyle(QwtSymbol::Cross);
//   symbol.setPen(QPen(Qt::black));
//   symbol.setSize(2);

  curve = new QwtPlotCurve;
  curve->setStyle(QwtPlotCurve::Lines);
  curve->setSymbol(symbol);
  curve->setPen(QPen(Qt::black));

  // error bars
  QwtIntervalSymbol errorBar(QwtIntervalSymbol::Bar);
  errorBar.setWidth(1);
  errorBar.setPen(QPen(Qt::lightGray));

  errorCurve = new QwtPlotIntervalCurve;
  errorCurve->setCurveStyle(QwtPlotIntervalCurve::NoCurve);
  errorCurve->setSymbol(errorBar);
}

PlotCurve::PlotCurvePrivate::~PlotCurvePrivate() {
  delete curve;
  delete errorCurve;

  delete pointData;
  delete intervalData;
}

void PlotCurve::PlotCurvePrivate::scale() {
  PlotPointData scaledPoints;
  foreach (QwtDoublePoint point, *pointData)
    scaledPoints.push_back(QwtDoublePoint(point.x() * scaleX,
                                          point.y() * scaleY));
  curve->setData(scaledPoints);

  PlotIntervalData scaledIntervals;
  foreach (QwtIntervalSample is, *intervalData)
    scaledIntervals.push_back(QwtIntervalSample(is.value * scaleX,
                                                QwtDoubleInterval(is.interval.minValue() * scaleY,
                                                                  is.interval.maxValue() * scaleY)));
  errorCurve->setData(scaledIntervals);
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
  delete p->pointData;
  p->pointData = new PlotPointData(points);

  delete p->intervalData;
  p->intervalData = new PlotIntervalData(intervals);

  p->scaleX = 1.0;
  p->scaleY = 1.0;

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

 if (Plot *plot = qobject_cast<Plot*>(p->curve->plot())) {
    qobject_cast<QwtLegendItem*>(plot->legend()->find(p->curve))->setVisible(on);

    //
    // UpdateLayout() is required to hide/show the
    // legend on the last/first curve.
    //
    plot->updateLayout();

    //
    // Update the bounding-rect and show the actual change.
    //
    plot->setZoomBase();
  }
}

QRectF PlotCurve::boundingRect() const {
  return p->errorBarsEnabled
               ? p->errorCurve->boundingRect()
               : p->curve->boundingRect();
}


QString PlotCurve::fileName() const {
  return p->fileName;
}

void PlotCurve::setFileName(const QString& fileName) {
  p->fileName = fileName;
}

QString PlotCurve::title() const {
  return p->curve->title().text();
}

void PlotCurve::setTitle(const QString& title) {
  //
  // Remove the legend entry if the title is empty.
  //
  if (Plot *plot = qobject_cast<Plot*>(p->curve->plot()))
    qobject_cast<QwtLegendItem*>(plot->legend()->find(p->curve))->setVisible(!title.isEmpty());

  p->curve->setTitle(title);
}


double PlotCurve::scalingFactorX() const {
  return p->scaleX;
}

void PlotCurve::setScalingFactorX(double scale) {
  p->scaleX = scale;
  p->scale();
}

double PlotCurve::scalingFactorY() const {
  return p->scaleY;
}

void PlotCurve::setScalingFactorY(double scale) {
  p->scaleY = scale;
  p->scale();
}

QPen PlotCurve::pen() const {
  return p->curve->pen();
}

void PlotCurve::setPen(const QPen& pen) {
  p->curve->setPen(pen);
}

QPen PlotCurve::errorBarPen() const {
  return p->errorCurve->symbol().pen();
}

void PlotCurve::setErrorBarPen(const QPen& pen) {
  QwtIntervalSymbol symbol(QwtIntervalSymbol::Bar);
  symbol.setWidth(1);  // cap width
  symbol.setPen(pen);
  p->errorCurve->setSymbol(symbol);
}

PlotSymbol PlotCurve::symbol() const {
  return p->curveSymbol;
}

void PlotCurve::setSymbol(const PlotSymbol& symbol) {
  p->curveSymbol = symbol;
  p->curve->setSymbol(symbol.qwtSymbol());
}

} // end of namespace Saxsview

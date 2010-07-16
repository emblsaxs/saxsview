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

#include "saxsview_plotcurve.h"
#include "saxsview_plot.h"
#include "saxsview_config.h"

#include <qwt_interval_symbol.h>
#include <qwt_legend_item.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_symbol.h>

namespace Saxsview {

PlotSymbol::PlotSymbol() : mSymbol(new QwtSymbol) {
  setStyle(NoSymbol);
  setSize(0);
  setColor(Qt::black);
}

PlotSymbol::PlotSymbol(Style style, int size, const QColor& color)
  : mSymbol(new QwtSymbol) {
  setStyle(style);
  setSize(size);
  setColor(color);
}

PlotSymbol::PlotSymbol(const PlotSymbol& other)
 : mSymbol(new QwtSymbol) {
  mSymbol->setStyle(other.mSymbol->style());
  mSymbol->setBrush(other.mSymbol->brush());
  mSymbol->setPen(other.mSymbol->pen());
  mSymbol->setSize(other.mSymbol->size());
}

PlotSymbol::~PlotSymbol() {
//   delete mSymbol;
}

PlotSymbol& PlotSymbol::operator=(const PlotSymbol& other) {
  PlotSymbol tmp(other);
  qSwap(mSymbol, tmp.mSymbol);
  return *this;
}


QColor PlotSymbol::color() const {
  return mSymbol->pen().color();
}

void PlotSymbol::setColor(const QColor& color) {
  mSymbol->setPen(QPen(color));
  if (mSymbol->brush() != Qt::NoBrush)
    mSymbol->setBrush(QBrush(color));
}

int PlotSymbol::size() const {
  return mSymbol->size().width();
}

void PlotSymbol::setSize(int size) {
  mSymbol->setSize(size);
}

PlotSymbol::Style PlotSymbol::style() const {
  return (Style)(mSymbol->style() + (mSymbol->brush() == Qt::NoBrush ? 0 : 100));
}

void PlotSymbol::setStyle(Style s) {
  if (s >= 100) {
    mSymbol->setStyle((QwtSymbol::Style)(s - 100));
    mSymbol->setBrush(QBrush(color()));
  } else {
    mSymbol->setStyle((QwtSymbol::Style)(s));
    mSymbol->setBrush(QBrush(Qt::NoBrush));
  }
}

QwtSymbol* PlotSymbol::qwtSymbol() const {
  return mSymbol;
}




class PlotCurve::PlotCurvePrivate {
public:
  PlotCurvePrivate(int type);
  ~PlotCurvePrivate();

  void scale();

  int type;
  QwtPlotCurve *curve;
  QwtPlotIntervalCurve *errorCurve;
  PlotSymbol curveSymbol;

  PlotPointData *pointData;
  PlotIntervalData *intervalData;
  double scaleX, scaleY;
  int every;

  bool errorBarsEnabled;
  QString fileName;
};

PlotCurve::PlotCurvePrivate::PlotCurvePrivate(int t)
 : type(t), curve(0L), errorCurve(0L), pointData(0L), intervalData(0L),
   scaleX(1.0), scaleY(1.0), every(1), errorBarsEnabled(true) {

  // Template is applied by plot when attaching this curve.

  // data points
  curve = new QwtPlotCurve;
  curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
  curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
  curve->setLegendAttribute(QwtPlotCurve::LegendShowBrush);
  curve->setSymbol(curveSymbol.qwtSymbol());

  // error bars
  QwtIntervalSymbol *errorBar = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
  errorBar->setWidth(1);        // cap width

  errorCurve = new QwtPlotIntervalCurve;
  errorCurve->setItemAttribute(QwtPlotItem::Legend, false);
  errorCurve->setCurveStyle(QwtPlotIntervalCurve::NoCurve);
}

PlotCurve::PlotCurvePrivate::~PlotCurvePrivate() {
  delete curve;
  delete errorCurve;

  delete pointData;
  delete intervalData;
}

void PlotCurve::PlotCurvePrivate::scale() {
  PlotPointData scaledPoints;
  for (int i = 0 ; i < pointData->size(); i += every)
    scaledPoints.push_back(QPointF(pointData->at(i).x() * scaleX,
                                   pointData->at(i).y() * scaleY));
  curve->setSamples(scaledPoints);

  PlotIntervalData scaledIntervals;
  for (int i = 0 ; i < intervalData->size(); i += every) {
    QwtIntervalSample is = intervalData->at(i);
    scaledIntervals.push_back(QwtIntervalSample(is.value * scaleX,
                                                is.interval.minValue() * scaleY,
                                                is.interval.maxValue() * scaleY));
  }
  errorCurve->setSamples(scaledIntervals);
}

PlotCurve::PlotCurve(int type, QObject *parent)
 : QObject(parent), p(new PlotCurvePrivate(type)) {
}

PlotCurve::~PlotCurve() {
  delete p;
}

int PlotCurve::type() const {
  return p->type;
}

void PlotCurve::attach(Plot *plot) {
  p->curve->attach(plot);
  p->errorCurve->attach(plot);

  if (QwtLegendItem* legendItem = qobject_cast<QwtLegendItem*>(plot->legend()->find(p->curve)))
    legendItem->setIdentifierSize(QSize(15, 10));

  p->curve->updateLegend(plot->legend());
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
  p->every  = 1;

  p->curve->setSamples(points);
  p->errorCurve->setSamples(intervals);
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
    //
    // The equivalent
    //    p->curve->setItemAttribute(QwtPlotItem::Legend, on);
    //
    // deletes the legend item and thus all settings may be lost.
    //
    QwtLegendItem* legendItem = qobject_cast<QwtLegendItem*>(plot->legend()->find(p->curve));
    if (legendItem)
      legendItem->setVisible(on);

    //
    // UpdateLayout() is required to hide/show the
    // legend on the last/first curve.
    //
    plot->updateLayout();

    //
    // Update the bounding-rect and show the actual change.
    //
    plot->replot();
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
  // Test for the attribute as it is also used by setVisible().
  //
  if (p->curve->testItemAttribute(QwtPlotItem::Legend))
    p->curve->setItemAttribute(QwtPlotItem::Legend, !title.isEmpty());
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

int PlotCurve::every() const {
  return p->every;
}

void PlotCurve::setEvery(int every) {
  p->every = every;
  p->scale();
}

QPen PlotCurve::pen() const {
  return p->curve->pen();
}

void PlotCurve::setPen(const QPen& pen) {
  p->curve->setPen(pen);
}

QPen PlotCurve::errorBarPen() const {
  return p->errorCurve->symbol()->pen();
}

void PlotCurve::setErrorBarPen(const QPen& pen) {
  QwtIntervalSymbol *symbol = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
  symbol->setWidth(1);  // cap width
  symbol->setPen(pen);
  p->errorCurve->setSymbol(symbol);
}

PlotSymbol PlotCurve::symbol() const {
  return p->curveSymbol;
}

void PlotCurve::setSymbol(const PlotSymbol& symbol) {
  p->curveSymbol = symbol;
  p->curve->setSymbol(p->curveSymbol.qwtSymbol());

//   QwtSymbol *sym = new QwtSymbol(QwtSymbol::NoSymbol);

//   p->curve->setSymbol(sym);
}

} // end of namespace Saxsview

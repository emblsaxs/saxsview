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


class SaxsviewPlotCurve::Private {
public:
  Private(int type);
  ~Private();

  void scale();
  void replot();

  int type;
  QwtPlotCurve *curve;
  QwtPlotIntervalCurve *errorCurve;
  QwtSymbol *symbol;
  QwtIntervalSymbol *intervalSymbol;

  SaxsviewPlotPointData *pointData;
  SaxsviewPlotIntervalData *intervalData;
  double scaleX, scaleY;
  int merge;

  bool errorBarsEnabled;
  QString fileName;
};

SaxsviewPlotCurve::Private::Private(int t)
 : type(t), curve(0L), errorCurve(0L),
   symbol(new QwtSymbol()), intervalSymbol(0L),
   pointData(0L), intervalData(0L),
   scaleX(1.0), scaleY(1.0), merge(1), errorBarsEnabled(true) {

  // Template is applied by plot when attaching this curve.

  // data points
  curve = new QwtPlotCurve;
  curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
  curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
  curve->setLegendAttribute(QwtPlotCurve::LegendShowBrush);
  curve->setSymbol(symbol);

  // error bars
  intervalSymbol = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
  intervalSymbol->setWidth(1);        // cap width

  errorCurve = new QwtPlotIntervalCurve;
  errorCurve->setItemAttribute(QwtPlotItem::Legend, false);
  errorCurve->setCurveStyle(QwtPlotIntervalCurve::NoCurve);
  errorCurve->setSymbol(intervalSymbol);
}

SaxsviewPlotCurve::Private::~Private() {
  delete curve;
  delete errorCurve;

  delete pointData;
  delete intervalData;
}

void SaxsviewPlotCurve::Private::scale() {
  SaxsviewPlotPointData scaledPoints;
  SaxsviewPlotIntervalData scaledIntervals;

  //
  // When merging N points, average them.
  // Error propagation:
  //
  //   \sigma* = \sqrt{\sum_{i=1}^{n} \sigma_i^2} / n
  //
  for (int i = 0 ; i < pointData->size(); i += merge) {
    double x = 0.0;
    double y = 0.0, y_lower_err = 0.0, y_upper_err = 0.0;
    double n = 0;

    for (int j = i; j < i + merge && j < pointData->size(); ++j) {
      const double& curx = pointData->at(j).x();
      const double& cury = pointData->at(j).y();
      const QwtIntervalSample& is = intervalData->at(i);

      x           += curx;
      y           += cury;
      y_upper_err += pow(is.interval.maxValue() - cury, 2);
      y_lower_err += pow(cury - is.interval.minValue(), 2);
      n           += 1.0;
    }
    x /= n;
    y /= n;

    scaledPoints.push_back(QPointF(x * scaleX, y * scaleY));
    scaledIntervals.push_back(QwtIntervalSample(x * scaleX,
                                                (y - sqrt(y_lower_err)/n) * scaleY,
                                                (y + sqrt(y_upper_err)/n) * scaleY));
  }

  curve->setSamples(scaledPoints);
  errorCurve->setSamples(scaledIntervals);

  replot();
}

void SaxsviewPlotCurve::Private::replot() {
  if (curve->plot())
    curve->plot()->replot();
}


SaxsviewPlotCurve::SaxsviewPlotCurve(int type, QObject *parent)
 : QObject(parent), p(new Private(type)) {
}

SaxsviewPlotCurve::~SaxsviewPlotCurve() {
  delete p;
}

int SaxsviewPlotCurve::type() const {
  return p->type;
}

void SaxsviewPlotCurve::attach(SaxsviewPlot *plot) {
  p->curve->attach(plot);
  p->errorCurve->attach(plot);

  if (QwtLegendItem* legendItem = qobject_cast<QwtLegendItem*>(plot->legend()->find(p->curve)))
    legendItem->setIdentifierSize(QSize(15, 10));

  p->curve->updateLegend(plot->legend());
}

void SaxsviewPlotCurve::detach() {
  p->curve->detach();
  p->errorCurve->detach();
}

void SaxsviewPlotCurve::setData(const SaxsviewPlotPointData& points,
                                const SaxsviewPlotIntervalData& intervals) {
  delete p->pointData;
  p->pointData = new SaxsviewPlotPointData(points);

  delete p->intervalData;
  p->intervalData = new SaxsviewPlotIntervalData(intervals);

  p->scaleX = 1.0;
  p->scaleY = 1.0;
  p->merge  = 1;

  p->curve->setSamples(points);
  p->errorCurve->setSamples(intervals);
}

bool SaxsviewPlotCurve::errorBarsEnabled() const {
  return p->errorBarsEnabled;
}

void SaxsviewPlotCurve::setErrorBarsEnabled(bool on) {
  p->errorBarsEnabled = on;
  p->errorCurve->setVisible(on);
}

bool SaxsviewPlotCurve::isVisible() const {
  return p->curve->isVisible();
}

void SaxsviewPlotCurve::setVisible(bool on) {
  p->curve->setVisible(on);
  p->errorCurve->setVisible(on && p->errorBarsEnabled);

  if (SaxsviewPlot *plot = qobject_cast<SaxsviewPlot*>(p->curve->plot())) {
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

QRectF SaxsviewPlotCurve::boundingRect() const {
  return p->errorBarsEnabled
               ? p->errorCurve->boundingRect()
               : p->curve->boundingRect();
}


QString SaxsviewPlotCurve::fileName() const {
  return p->fileName;
}

void SaxsviewPlotCurve::setFileName(const QString& fileName) {
  p->fileName = fileName;
}

QString SaxsviewPlotCurve::title() const {
  return p->curve->title().text();
}

void SaxsviewPlotCurve::setTitle(const QString& title) {
  //
  // Hide the legend entry if the title is empty.
  //
  if (SaxsviewPlot *plot = qobject_cast<SaxsviewPlot*>(p->curve->plot())) {
    QwtLegendItem* legendItem = qobject_cast<QwtLegendItem*>(plot->legend()->find(p->curve));
    if (legendItem)
      legendItem->setVisible(p->curve->isVisible() && !title.isEmpty());
  }

  p->curve->setTitle(title);

  if (SaxsviewPlot *plot = qobject_cast<SaxsviewPlot*>(p->curve->plot()))
    plot->updateLayout();
}

double SaxsviewPlotCurve::scalingFactorX() const {
  return p->scaleX;
}

void SaxsviewPlotCurve::setScalingFactorX(double scale) {
  p->scaleX = scale;
  p->scale();
}

double SaxsviewPlotCurve::scalingFactorY() const {
  return p->scaleY;
}

void SaxsviewPlotCurve::setScalingFactorY(double scale) {
  p->scaleY = scale;
  p->scale();
}

int SaxsviewPlotCurve::merge() const {
  return p->merge;
}

void SaxsviewPlotCurve::setMerge(int merge) {
  p->merge = merge;
  p->scale();
}

int SaxsviewPlotCurve::closestPoint(const QPoint &pos, double *dist) const {
  return p->curve->closestPoint(pos, dist);
}

void SaxsviewPlotCurve::setLineStyle(Saxsview::LineStyle style) {
  QPen lp = p->curve->pen();
  lp.setStyle((Qt::PenStyle)style);
  p->curve->setPen(lp);

  p->replot();
}

Saxsview::LineStyle SaxsviewPlotCurve::lineStyle() const {
  return (Saxsview::LineStyle)p->curve->pen().style();
}

void SaxsviewPlotCurve::setLineWidth(int n) {
  QPen lp = p->curve->pen();
  lp.setWidth(n);
  p->curve->setPen(lp);

  p->replot();
}

int SaxsviewPlotCurve::lineWidth() const {
  return p->curve->pen().width();
}

void SaxsviewPlotCurve::setLineColor(const QColor& c) {
  QPen lp = p->curve->pen();
  lp.setColor(c);
  p->curve->setPen(lp);

  p->replot();
}

QColor SaxsviewPlotCurve::lineColor() const {
  return p->curve->pen().color();
}

void SaxsviewPlotCurve::setSymbolStyle(Saxsview::SymbolStyle s) {
  QwtSymbol::Style style;
  switch (s) {
    default:
    case Saxsview::NoSymbol:              style = QwtSymbol::NoSymbol; break;
    case Saxsview::Ellipse:               style = QwtSymbol::Ellipse; break;
    case Saxsview::Rect:                  style = QwtSymbol::Rect; break;
    case Saxsview::Diamond:               style = QwtSymbol::Diamond; break;
    case Saxsview::TrianglePointingDown:  style = QwtSymbol::DTriangle; break;
    case Saxsview::TrianglePointingUp:    style = QwtSymbol::UTriangle; break;
    case Saxsview::TrianglePointingLeft:  style = QwtSymbol::LTriangle; break;
    case Saxsview::TrianglePointingRight: style = QwtSymbol::RTriangle; break;
    case Saxsview::Cross:                 style = QwtSymbol::Cross; break;
    case Saxsview::XCross:                style = QwtSymbol::XCross; break;
    case Saxsview::HorizontalLine:        style = QwtSymbol::HLine; break;
    case Saxsview::VerticalLine:          style = QwtSymbol::VLine; break;
    case Saxsview::StarLine:              style = QwtSymbol::Star1; break;
    case Saxsview::StarOutline:           style = QwtSymbol::Star2; break;
    case Saxsview::Hexagon:               style = QwtSymbol::Hexagon; break;
  }

  p->symbol->setStyle(style);
  p->replot();
}

Saxsview::SymbolStyle SaxsviewPlotCurve::symbolStyle() const {
  switch (p->symbol->style()) {
    default:
    case QwtSymbol::NoSymbol:  return Saxsview::NoSymbol;
    case QwtSymbol::Ellipse:   return Saxsview::Ellipse;
    case QwtSymbol::Rect:      return Saxsview::Rect;
    case QwtSymbol::Diamond:   return Saxsview::Diamond;
    case QwtSymbol::DTriangle: return Saxsview::TrianglePointingDown;
    case QwtSymbol::UTriangle: return Saxsview::TrianglePointingUp;
    case QwtSymbol::LTriangle: return Saxsview::TrianglePointingLeft;
    case QwtSymbol::RTriangle: return Saxsview::TrianglePointingRight;
    case QwtSymbol::Cross:     return Saxsview::Cross;
    case QwtSymbol::XCross:    return Saxsview::XCross;
    case QwtSymbol::HLine:     return Saxsview::HorizontalLine;
    case QwtSymbol::VLine:     return Saxsview::VerticalLine;
    case QwtSymbol::Star1:     return Saxsview::StarLine;
    case QwtSymbol::Star2:     return Saxsview::StarOutline;
    case QwtSymbol::Hexagon:   return Saxsview::Hexagon;
  }
}

void SaxsviewPlotCurve::setSymbolSize(int size) {
  p->symbol->setSize(size);
  p->replot();
}

int SaxsviewPlotCurve::symbolSize() const {
  return p->symbol->size().width();
}

void SaxsviewPlotCurve::setSymbolFilled(bool filled) {
  p->symbol->setBrush(filled ? QBrush(symbolColor()) : Qt::NoBrush);
  p->replot();
}

bool SaxsviewPlotCurve::isSymbolFilled() const {
  return p->symbol->brush() != Qt::NoBrush;
}

void SaxsviewPlotCurve::setSymbolColor(const QColor& c) {
  p->symbol->setPen(QPen(c));
  if (isSymbolFilled())
    p->symbol->setBrush(QBrush(c));

  p->replot();
}

QColor SaxsviewPlotCurve::symbolColor() const {
  return p->symbol->pen().color();
}

void SaxsviewPlotCurve::setErrorLineStyle(Saxsview::LineStyle style) {
  QPen ep = p->intervalSymbol->pen();
  ep.setStyle((Qt::PenStyle)style);
  p->intervalSymbol->setPen(ep);

  p->replot();
}

Saxsview::LineStyle SaxsviewPlotCurve::errorLineStyle() const {
  return (Saxsview::LineStyle)p->intervalSymbol->pen().style();
}

void SaxsviewPlotCurve::setErrorLineWidth(int n) {
  QPen ep = p->intervalSymbol->pen();
  ep.setWidth(n);
  p->intervalSymbol->setPen(ep);

  p->replot();
}

int SaxsviewPlotCurve::errorLineWidth() const {
  return p->intervalSymbol->pen().width();
}

void SaxsviewPlotCurve::setErrorLineColor(const QColor& c) {
  QPen ep = p->intervalSymbol->pen();
  ep.setColor(c);
  p->intervalSymbol->setPen(ep);

  p->replot();
}

QColor SaxsviewPlotCurve::errorLineColor() const {
  return p->intervalSymbol->pen().color();
}

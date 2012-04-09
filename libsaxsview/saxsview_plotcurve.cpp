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
#include "saxsview_transformation.h"

#include <qwt_interval_symbol.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_symbol.h>


class SaxsviewPlotCurve::Private {
public:
  Private(int type);
  ~Private();

  void transform();
  void replot();

  int type;
  QwtPlotCurve *curve;
  QwtPlotIntervalCurve *errorCurve;
  QwtSymbol *symbol;
  QwtIntervalSymbol *intervalSymbol;

  SaxsviewPlotPointData *pointData;
  SaxsviewPlotIntervalData *intervalData;
  SaxsviewTransformation *transformation;

  bool errorBarsEnabled;
  QString fileName;
};

SaxsviewPlotCurve::Private::Private(int t)
 : type(t), curve(0L), errorCurve(0L),
   symbol(new QwtSymbol()), intervalSymbol(0L),
   pointData(0L), intervalData(0L), transformation(0L),
   errorBarsEnabled(true) {

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
  errorCurve->setStyle(QwtPlotIntervalCurve::NoCurve);
  errorCurve->setSymbol(intervalSymbol);
}

SaxsviewPlotCurve::Private::~Private() {
  delete curve;
  delete errorCurve;

  delete pointData;
  delete intervalData;
}

void SaxsviewPlotCurve::Private::transform() {
  if (transformation) {
    curve->setSamples(transformation->transform(*pointData));
    errorCurve->setSamples(transformation->transform(*intervalData));

  } else {
    curve->setSamples(*pointData);
    errorCurve->setSamples(*intervalData);
  }

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

  if (p->transformation) {
    p->transformation->setMerge(1);
    p->transformation->setScaleX(1.0);
    p->transformation->setScaleY(1.0);
  }

  p->transform();
}

void SaxsviewPlotCurve::setTransformation(SaxsviewTransformation *t) {
  delete p->transformation;
  p->transformation = t;
  p->transform();
}

bool SaxsviewPlotCurve::errorBarsEnabled() const {
  return p->errorBarsEnabled;
}

void SaxsviewPlotCurve::setErrorBarsEnabled(bool on) {
  p->errorBarsEnabled = on;
  p->errorCurve->setVisible(on);
  p->replot();
}

bool SaxsviewPlotCurve::isVisible() const {
  return p->curve->isVisible();
}

void SaxsviewPlotCurve::setVisible(bool on) {
  // Visibility change doesn't change the legend - do it manually.
  p->curve->setVisible(on);
  p->curve->legendChanged();
  p->curve->setItemAttribute(QwtPlotItem::Legend,
                             on && !p->curve->title().isEmpty());

  p->errorCurve->setVisible(on && p->errorBarsEnabled);
  p->replot();
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
  p->curve->setTitle(title);
  p->curve->setItemAttribute(QwtPlotItem::Legend,
                             p->curve->isVisible() &&  !title.isEmpty());
  p->replot();
}

double SaxsviewPlotCurve::scalingFactorX() const {
  return p->transformation->scaleX();
}

void SaxsviewPlotCurve::setScalingFactorX(double factor) {
  p->transformation->setScaleX(factor);
  p->transform();
}

double SaxsviewPlotCurve::scalingFactorY() const {
  return p->transformation->scaleY();
}

void SaxsviewPlotCurve::setScalingFactorY(double factor) {
  p->transformation->setScaleY(factor);
  p->transform();
}

int SaxsviewPlotCurve::merge() const {
  return p->transformation->merge();
}

void SaxsviewPlotCurve::setMerge(int n) {
  p->transformation->setMerge(n);
  p->transform();
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
  // This invalidates the icon cache for the legend item and makes
  // sure the icon is updated properly.
  p->curve->legendChanged();
  p->curve->itemChanged();

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

  p->curve->legendChanged();
  p->curve->itemChanged();

  p->replot();
}

int SaxsviewPlotCurve::symbolSize() const {
  return p->symbol->size().width();
}

void SaxsviewPlotCurve::setSymbolFilled(bool filled) {
  p->symbol->setBrush(filled ? QBrush(symbolColor()) : Qt::NoBrush);

  p->curve->legendChanged();
  p->curve->itemChanged();

  p->replot();
}

bool SaxsviewPlotCurve::isSymbolFilled() const {
  return p->symbol->brush() != Qt::NoBrush;
}

void SaxsviewPlotCurve::setSymbolColor(const QColor& c) {
  p->symbol->setPen(QPen(c));
  if (isSymbolFilled())
    p->symbol->setBrush(QBrush(c));

  p->curve->legendChanged();
  p->curve->itemChanged();

  p->replot();
}

QColor SaxsviewPlotCurve::symbolColor() const {
  return p->symbol->pen().color();
}

void SaxsviewPlotCurve::setErrorLineStyle(Saxsview::LineStyle style) {
  p->errorBarsEnabled = (style != Saxsview::NoLine);

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

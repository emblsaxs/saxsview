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

#include <QtGui>

#include <qwt_series_data.h>
#include <qwt_symbol.h>

#include "saxsview.h"
class SaxsviewPlot;

typedef QVector<QPointF> SaxsviewPlotPointData;
typedef QVector<QwtIntervalSample> SaxsviewPlotIntervalData;


/**
 *
 * @note In Qwt-terms, PlotCurve is not one but two curves.
 *       One for the data points (@ref QwtPlotCurve) and one
 *       for error bars (@ref QwtPlotIntervalCurve).
 */
class SaxsviewPlotCurve : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool curveEnabled READ isVisible WRITE setVisible)
  Q_PROPERTY(QString curveTitle READ title WRITE setTitle)

  Q_PROPERTY(Saxsview::LineStyle lineStyle READ lineStyle WRITE setLineStyle)
  Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth)
  Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor)

  Q_PROPERTY(Saxsview::SymbolStyle symbolStyle READ symbolStyle WRITE setSymbolStyle)
  Q_PROPERTY(int symbolSize READ symbolSize WRITE setSymbolSize)
  Q_PROPERTY(bool isSymbolFilled READ isSymbolFilled WRITE setSymbolFilled)
  Q_PROPERTY(QColor symbolColor READ symbolColor WRITE setSymbolColor)

  Q_PROPERTY(double scalingFactorX READ scalingFactorX WRITE setScalingFactorX)
  Q_PROPERTY(double scalingFactorY READ scalingFactorY WRITE setScalingFactorY)
  Q_PROPERTY(int merge READ merge WRITE setMerge)

  Q_PROPERTY(Saxsview::LineStyle errorLineStyle READ errorLineStyle WRITE setErrorLineStyle)
  Q_PROPERTY(int errorLineWidth READ errorLineWidth WRITE setErrorLineWidth)
  Q_PROPERTY(QColor errorLineColor READ errorLineColor WRITE setErrorLineColor)

public:
  SaxsviewPlotCurve(int type, QObject *parent = 0L);
  ~SaxsviewPlotCurve();

  int type() const;

  void attach(SaxsviewPlot *plot);
  void detach();

  void setData(const SaxsviewPlotPointData& points,
               const SaxsviewPlotIntervalData& intervals);

  QString fileName() const;
  QString title() const;
  bool isVisible() const;
  bool errorBarsEnabled() const;

  QRectF boundingRect() const;

  double scalingFactorX() const;
  double scalingFactorY() const;
  int merge() const;

  Saxsview::LineStyle lineStyle() const;
  int lineWidth() const;
  QColor lineColor() const;

  Saxsview::SymbolStyle symbolStyle() const;
  int symbolSize() const;
  bool isSymbolFilled() const;
  QColor symbolColor() const;

  Saxsview::LineStyle errorLineStyle() const;
  int errorLineWidth() const;
  QColor errorLineColor() const;

  int closestPoint(const QPoint &pos, double *dist = NULL) const;

public slots:
  void setFileName(const QString&);
  void setTitle(const QString& title);
  void setVisible(bool on);
  void setErrorBarsEnabled(bool on);

  void setScalingFactorX(double);
  void setScalingFactorY(double);
  void setMerge(int);

  void setLineStyle(Saxsview::LineStyle);
  void setLineWidth(int);
  void setLineColor(const QColor&);

  void setSymbolStyle(Saxsview::SymbolStyle);
  void setSymbolSize(int);
  void setSymbolFilled(bool);
  void setSymbolColor(const QColor&);

  void setErrorLineStyle(Saxsview::LineStyle);
  void setErrorLineWidth(int);
  void setErrorLineColor(const QColor&);

private:
  class Private;
  Private *p;
};

#endif // !SAXSVIEW_PLOTCURVE_H

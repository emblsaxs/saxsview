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
class QPen;
class QRectF;

#include <qwt_series_data.h>
#include <qwt_symbol.h>

namespace Saxsview {

class Plot;

typedef QVector<QPointF> PlotPointData;
typedef QVector<QwtIntervalSample> PlotIntervalData;


/**
 * Generally, symbols have an outline and an inner area.
 * The pen color of the outline may differ from the color
 * of the brush to fill the interior. Here this is
 * simplified to a single color. Filled symbols have a
 * brush the same color as the pen for the outline,
 * all others use Qt::NoBrush.
 */
class PlotSymbol {
public:
  enum Style {
    NoSymbol = QwtSymbol::NoSymbol,
    Ellipse = QwtSymbol::Ellipse,
    Rect = QwtSymbol::Rect,
    Diamond = QwtSymbol::Diamond,
    Triangle = QwtSymbol::Triangle,
    DTriangle = QwtSymbol::DTriangle,
    UTriangle = QwtSymbol::UTriangle,
    LTriangle = QwtSymbol::LTriangle,
    RTriangle = QwtSymbol::RTriangle,
    Cross = QwtSymbol::Cross,
    XCross = QwtSymbol::XCross,
    HLine = QwtSymbol::HLine,
    VLine = QwtSymbol::VLine,
    Star1 = QwtSymbol::Star1,
    Star2 = QwtSymbol::Star2,
    Hexagon = QwtSymbol::Hexagon,

    FilledEllipse = Ellipse + 100,
    FilledRect = Rect + 100,
    FilledDiamond = Diamond + 100,
    FilledTriangle = Triangle + 100,
    FilledDTriangle = DTriangle + 100,
    FilledUTriangle = UTriangle + 100,
    FilledLTriangle = LTriangle + 100,
    FilledRTriangle = RTriangle + 100,
    FilledStar2 = Star2 + 100,
    FilledHexagon = Hexagon + 100
  };

  PlotSymbol();
  PlotSymbol(Style style, int size, const QColor& color);
  PlotSymbol(const PlotSymbol&);
  ~PlotSymbol();

  PlotSymbol& operator=(const PlotSymbol&);

  QColor color() const;
  void setColor(const QColor& color);

  int size() const;
  void setSize(int size);

  Style style() const;
  void setStyle(Style s);

  QwtSymbol* qwtSymbol() const;

private:
  QwtSymbol *mSymbol;
};

/**
 *
 * @note In Qwt-terms, PlotCurve is not one but two curves.
 *       One for the data points (@ref QwtPlotCurve) and one
 *       for error bars (@ref QwtPlotIntervalCurve).
 */
class PlotCurve : public QObject {
  Q_OBJECT

public:
  PlotCurve(int type, QObject *parent = 0L);
  ~PlotCurve();

  int type() const;

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
  int every() const;

  QPen pen() const;
  void setPen(const QPen&); 

  QPen errorBarPen() const;
  void setErrorBarPen(const QPen&); 

  PlotSymbol symbol() const;
  void setSymbol(const PlotSymbol&); 

public slots:
  void setFileName(const QString&);
  void setTitle(const QString& title);
  void setVisible(bool on);
  void setErrorBarsEnabled(bool on);

  void setScalingFactorX(double);
  void setScalingFactorY(double);
  void setEvery(int);

private:
  class PlotCurvePrivate;
  PlotCurvePrivate *p;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_PLOTCURVE_H

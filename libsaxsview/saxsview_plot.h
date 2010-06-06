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

#ifndef SAXSVIEW_PLOT_H
#define SAXSVIEW_PLOT_H

#include <QAction>
class QEvent;
class QPainter;
class QRect;

#include <qwt_plot.h>
#include <qwt_plot_item.h>

namespace Saxsview {

class PlotCurve;

class Plot : public QwtPlot {
  Q_OBJECT

public:
  enum PlotScale {
    AbsoluteScale = 1,
    Log10Scale
  };

  Plot(QWidget *parent = 0L);
  ~Plot();

  void addCurve(PlotCurve *);
  void removeCurve(PlotCurve *);
  QList<PlotCurve*> curves() const;

  QRectF zoomBase() const;
  bool zoomEnabled() const;
  bool moveEnabled() const;
  PlotScale scale() const;

  bool replotBlocked() const;

  void zoom(const QRectF&);

public slots:
  void replot();
  void blockReplot(bool);

  void clear();
  void exportAs();
  void exportAs(const QString&);
  void print();
  void configure();
  void setZoomBase(const QRectF& rect = QRectF());
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);
  void setScale(PlotScale);

// protected:
//   void printLegend(QPainter *, const QRect &) const;

protected:
  bool eventFilter(QObject*, QEvent*);

private:
  class PlotPrivate;
  PlotPrivate *p;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_PLOT_H

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

class QEvent;
class QPainter;
class QRect;

#include <qwt_plot.h>
#include <qwt_plot_item.h>

#include "saxsview.h"
class SaxsviewPlotCurve;
class SaxsviewPlotSymbol;

class SaxsviewPlot : public QwtPlot {
  Q_OBJECT

  Q_PROPERTY(Saxsview::Scale scale READ scale WRITE setScale)

  Q_PROPERTY(QString plotTitle READ plotTitle WRITE setPlotTitle)
  Q_PROPERTY(QFont plotTitleFont READ plotTitleFont WRITE setPlotTitleFont)
  Q_PROPERTY(QString axisTitleX READ axisTitleX WRITE setAxisTitleX)
  Q_PROPERTY(QString axisTitleY READ axisTitleY WRITE setAxisTitleY)
  Q_PROPERTY(QFont axisTitleFont READ axisTitleFont WRITE setAxisTitleFont)
  Q_PROPERTY(bool ticksEnabledX READ ticksEnabledX WRITE setTicksEnabledX)
  Q_PROPERTY(bool ticksEnabledY READ ticksEnabledY WRITE setTicksEnabledY)
  Q_PROPERTY(QFont ticksFont READ ticksFont WRITE setTicksFont)
  Q_PROPERTY(bool legendVisible READ legendVisible WRITE setLegendVisible)
  Q_PROPERTY(Qt::Corner legendPosition READ legendPosition WRITE setLegendPosition)
  Q_PROPERTY(int legendColumnsCount READ legendColumnCount WRITE setLegendColumnCount)
  Q_PROPERTY(int legendSpacing READ legendSpacing WRITE setLegendSpacing)
  Q_PROPERTY(int legendMargin READ legendMargin WRITE setLegendMargin)
  Q_PROPERTY(QFont legendFont READ legendFont WRITE setLegendFont)

public:
  SaxsviewPlot(QWidget *parent = 0L);
  ~SaxsviewPlot();

  void addCurve(SaxsviewPlotCurve *);
  void removeCurve(SaxsviewPlotCurve *);
  QList<SaxsviewPlotCurve*> curves() const;

  QRectF zoomBase() const;
  bool isZoomEnabled() const;
  bool isMoveEnabled() const;

  bool replotBlocked() const;

  void zoom(const QRectF&);

  Saxsview::Scale scale() const;

  QString plotTitle() const;
  QFont plotTitleFont() const;
  QString axisTitleX() const;
  QString axisTitleY() const;
  QFont axisTitleFont() const;
  bool ticksEnabledX() const;
  bool ticksEnabledY() const;
  QFont ticksFont() const;
  bool legendVisible() const;
  Qt::Corner legendPosition() const;
  int legendColumnCount() const;
  int legendSpacing() const;
  int legendMargin() const;
  QFont legendFont() const;

public slots:
  void replot();
  void blockReplot(bool);

  void clear();
  void exportAs();
  void exportAs(const QString& fileName, const QString& format = QString());
  void print();
  void configure();
  void setZoomBase(const QRectF& rect = QRectF());
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);

  void setScale(Saxsview::Scale);

  void setPlotTitle(const QString&);
  void setPlotTitleFont(const QFont&);
  void setAxisTitleX(const QString&);
  void setAxisTitleY(const QString&);
  void setAxisTitleFont(const QFont&);
  void setTicksEnabledX(bool);
  void setTicksEnabledY(bool);
  void setTicksFont(const QFont&);
  void setLegendVisible(bool);
  void setLegendPosition(Qt::Corner);
  void setLegendColumnCount(int);
  void setLegendSpacing(int);
  void setLegendMargin(int);
  void setLegendFont(const QFont&);

private:
  class Private;
  Private *p;
};

#endif // !SAXSVIEW_PLOT_H

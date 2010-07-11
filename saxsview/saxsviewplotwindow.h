/*
 * Implementation of 1D-plot subwindows.
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of saxsview.
 *
 * saxsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * saxsview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with saxsview. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SAXSVIEW_PLOTWINDOW_H
#define SAXSVIEW_PLOTWINDOW_H

#include "saxsviewsubwindow.h"
class QEvent;

class SaxsviewPlotWindow : public SaxsviewSubWindow {
  Q_OBJECT

public:
  SaxsviewPlotWindow(QWidget *parent = 0L);
  ~SaxsviewPlotWindow();

  static bool canShow(const QString& fileName);

  int scale() const;
  bool zoomEnabled() const;
  bool moveEnabled() const;

public slots:
  void load(const QString& fileName);
  void exportAs(const QString& fileName);
  void print();
  void zoomFit();
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);
  void setScale(int);
  void configure();
  void explode();

protected:
  bool eventFilter(QObject*, QEvent*);

private:
  class SaxsviewPlotWindowPrivate;
  SaxsviewPlotWindowPrivate *p;
};

#endif // !SAXSVIEW_PLOTWINDOW_H

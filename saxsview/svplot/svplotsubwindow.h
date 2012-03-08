/*
 * Copyright (C) 2012 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SVPLOTSUBWINDOW_H
#define SVPLOTSUBWINDOW_H

class SaxsviewPlot;
class SVPlotProject;

#include <QMdiSubWindow>
class QEvent;

class SVPlotSubWindow : public QMdiSubWindow {
  Q_OBJECT

public:
  SVPlotSubWindow(QWidget *parent = 0L);
  ~SVPlotSubWindow();

  SVPlotProject* project();

  SaxsviewPlot* plot() const;

  bool zoomEnabled() const;
  bool moveEnabled() const;

public slots:
  bool load(const QString& fileName);
  void reload();
  void exportAs(const QString& fileName, const QString& format);
  void print();
  void zoomFit();
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);

private:
  class Private;
  Private *p;
};

#endif // !SVPLOTSUBWINDOW_H

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
#ifndef SVPLOTFILEDOCKWIDGET_H
#define SVPLOTFILEDOCKWIDGET_H

#include <QDockWidget>

class QMdiSubWindow;
class QtProperty;

class SVPlotFileDockWidget : public QDockWidget {
  Q_OBJECT

public:
  SVPlotFileDockWidget(QWidget *parent = 0L);
  ~SVPlotFileDockWidget();

public slots:
  void subWindowActivated(QMdiSubWindow*);

private:
  class Private;
  Private *p;
};

#endif // !SVPLOTFILEDOCKWIDGET_H

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
#ifndef SVPLOTFILEBROWSERDOCKWIDGET_H
#define SVPLOTFILEBROWSERDOCKWIDGET_H

#include <QDockWidget>
class QModelIndex;

class SVPlotFileBrowserDockWidget : public QDockWidget {
  Q_OBJECT

public:
  SVPlotFileBrowserDockWidget(QWidget *parent = 0L);
  ~SVPlotFileBrowserDockWidget();

signals:
  void selected(const QString&);

public slots:
  void setDirectory(const QString&);

private slots:
  void setNameFilter(const QString&);
  void parentDirectory();
  void homeDirectory();
  void indexSelected(const QModelIndex&);

private:
  class Private;
  Private *p;
};

#endif // !SVPLOTFILEBROWSERDOCKWIDGET_H

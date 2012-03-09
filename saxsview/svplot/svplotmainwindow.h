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

#ifndef SVPLOTMAINWINDOW_H
#define SVPLOTMAINWINDOW_H

#include <QMainWindow>
class QMdiSubWindow;
class QString;

class SVPlotSubWindow;

class SVPlotMainWindow : public QMainWindow {
  Q_OBJECT

public:
  SVPlotMainWindow(QWidget *parent = 0L);
  ~SVPlotMainWindow();

  void addSubWindow(SVPlotSubWindow*);
  SVPlotSubWindow* currentSubWindow() const;

public slots:
  void newSubWindow();
  void load();
  void load(const QString& fileName);
  void reload();
  void exportAs(const QString& format);
  void print();
  void zoomFit();
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);

  void about();

private slots:
  void prepareWindowMenu();
  void prepareRecentFilesMenu();
  void setActiveSubWindow(QWidget*);
  void subWindowActivated(QMdiSubWindow*);
  void subWindowDestroyed(QObject*);

protected:
  void closeEvent(QCloseEvent*);

private:
  class SVPlotMainWindowPrivate;
  SVPlotMainWindowPrivate *p;
};

#endif // !SVPLOTMAINWINDOW_H

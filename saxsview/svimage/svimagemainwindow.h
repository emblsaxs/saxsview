/*
 * Copyright (C) 2011, 2012, 2013 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SVIMAGEMAINWINDOW_H
#define SVIMAGEMAINWINDOW_H

#include <QMainWindow>
class QMdiSubWindow;
class QString;

class SVImageSubWindow;

class SVImageMainWindow : public QMainWindow {
  Q_OBJECT

public:
  SVImageMainWindow(QWidget *parent = 0L);
  ~SVImageMainWindow();

  void addSubWindow(SVImageSubWindow*);
  SVImageSubWindow* currentSubWindow() const;

public slots:
  void load();
  void load(const QString& fileName);
  void reload();
  void exportAs(const QString& format);
  void print();
  void zoomFit();
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);

  void goFirst();
  void goPrevious();
  void goNext();
  void goLast();
  void setWatchLatest(bool);

  void about();

private slots:
  void prepareWindowMenu();
  void prepareRecentFilesMenu();
  void setActiveSubWindow(QWidget*);
  void subWindowActivated(QMdiSubWindow*);
  void subWindowDestroyed(QObject*);

protected:
  bool eventFilter(QObject*, QEvent*);

private:
  class SVImageMainWindowPrivate;
  SVImageMainWindowPrivate *p;
};

#endif // !SVIMAGEMAINWINDOW_H

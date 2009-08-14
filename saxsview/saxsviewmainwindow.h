/*
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SAXSVIEW_MAINWINDOW_H
#define SAXSVIEW_MAINWINDOW_H

#include <QMainWindow>
class QMdiSubWindow;
class QString;

class SaxsviewSubWindow;

class SaxsviewMainWindow : public QMainWindow {
  Q_OBJECT

public:
  SaxsviewMainWindow(QWidget *parent = 0L);
  ~SaxsviewMainWindow();

  SaxsviewSubWindow* currentSubWindow() const;

public slots:
  void createSubWindow();
  void load();
  void load(const QString& fileName);

  void exportAs(const QString& format);
  void print();
  void zoomIn();
  void zoomOut();
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);
  void setScale(int);
  void configure();

  void about();

private slots:
  void prepareWindowMenu();
  void prepareRecentFilesMenu();
  void setActiveSubWindow(QWidget*);
  void subWindowActivated(QMdiSubWindow*);

private:
  class SaxsviewMainWindowPrivate;
  SaxsviewMainWindowPrivate *p;
};

#endif // !SAXSVIEW_MAINWINDOW_H

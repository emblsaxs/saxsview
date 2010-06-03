/*
 * Interface for view subwindows.
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

#ifndef SAXSVIEW_SUBWINDOW_H
#define SAXSVIEW_SUBWINDOW_H

#include <QMdiSubWindow>

class SaxsviewSubWindow : public QMdiSubWindow {
  Q_OBJECT

public:
  SaxsviewSubWindow(QWidget *parent = 0L);
  virtual ~SaxsviewSubWindow();

  virtual int scale() const = 0;
  virtual bool zoomEnabled() const = 0;
  virtual bool moveEnabled() const = 0;

public slots:
  virtual void load(const QString& fileName) = 0;
  virtual void exportAs(const QString& fileName) = 0;
  virtual void print() = 0;
  virtual void zoomIn() = 0;
  virtual void zoomOut() = 0;
  virtual void setZoomEnabled(bool) = 0;
  virtual void setMoveEnabled(bool) = 0;
  virtual void setScale(int) = 0;
  virtual void configure();
};

#endif // !SAXSVIEW_SUBWINDOW_H

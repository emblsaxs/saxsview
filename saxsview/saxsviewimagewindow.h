/*
 * Implementation of 2D-images subwindows.
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

#ifndef SAXSVIEW_IMAGEWINDOW_H
#define SAXSVIEW_IMAGEWINDOW_H

#include "saxsviewsubwindow.h"
class QEvent;

class SaxsviewImageWindow : public SaxsviewSubWindow {
  Q_OBJECT

public:
  SaxsviewImageWindow(SaxsviewMainWindow *parent = 0L);
  ~SaxsviewImageWindow();

  static bool canShow(const QString& fileName);

  int scale() const;
  bool zoomEnabled() const;
  bool moveEnabled() const;

  QToolBar* createToolBar();

public slots:
  void load(const QString& fileName);
  void exportAs(const QString& fileName);
  void print();
  void zoomFit();
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);
  void setScale(int);

private slots:
  void setRange();
  void resetRange();

private:
  class SaxsviewImageWindowPrivate;
  SaxsviewImageWindowPrivate *p;
};

#endif // !SAXSVIEW_IMAGEWINDOW_H

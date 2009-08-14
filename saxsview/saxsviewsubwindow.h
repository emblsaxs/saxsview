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

#ifndef SAXSVIEW_SUBWINDOW_H
#define SAXSVIEW_SUBWINDOW_H

#include <QMdiSubWindow>
class QEvent;

class SaxsviewSubWindow : public QMdiSubWindow {
  Q_OBJECT

public:
  SaxsviewSubWindow(QWidget *parent = 0L);
  ~SaxsviewSubWindow();

  int scale() const;
  bool zoomEnabled() const;
  bool moveEnabled() const;

public slots:
  void load(const QString& fileName);
  void exportAs(const QString& fileName);
  void print();
  void zoomIn();
  void zoomOut();
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);
  void setScale(int);
  void configure();

protected:
  bool eventFilter(QObject*, QEvent*);

private:
  class SaxsviewSubWindowPrivate;
  SaxsviewSubWindowPrivate *p;
};

#endif // !SAXSVIEW_SUBWINDOW_H

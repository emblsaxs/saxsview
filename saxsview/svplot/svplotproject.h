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

#ifndef SAXSVIEWPROJECT_H
#define SAXSVIEWPROJECT_H

#include <QtGui>

class SaxsviewPlot;
class SaxsviewPlotCurve;


class SaxsviewPlotItem : public QStandardItem {
public:
  SaxsviewPlotItem(SaxsviewPlot *plot, const QString& title);
  int type() const;

  SaxsviewPlot *plot() const;
};

class SaxsviewPlotCurveItem : public QStandardItem {
public:
  SaxsviewPlotCurveItem(SaxsviewPlotCurve *curve);
  int type() const;

  SaxsviewPlotCurve *curve() const;
};



class SVPlotProject : public QObject {
  Q_OBJECT

public:
  SVPlotProject(QObject *parent = 0L);
  ~SVPlotProject();

  void addPlot(SaxsviewPlot *plot, const QString& title);
  void addPlotCurve(SaxsviewPlotCurve *curve);

  QStandardItemModel* model();
  QItemSelectionModel* selectionModel();

signals:
  void itemChanged(QStandardItem*);
  void currentIndexChanged(const QModelIndex&);

private:
  class Private;
  Private *p;
};

#endif // !SAXSVIEWPROJECT_H

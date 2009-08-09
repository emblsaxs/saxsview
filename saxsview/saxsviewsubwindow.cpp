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

#include "saxsviewsubwindow.h"
#include "saxsview_plot.h"
#include "saxsview_plotcurve.h"

#include "saxsdocument.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

class SaxsviewSubWindow::SaxsviewSubWindowPrivate {
public:
  SaxsviewSubWindowPrivate(SaxsviewSubWindow *w) : sw(w) {}

  void setupUi();

  SaxsviewSubWindow *sw;
  Saxsview::Plot *plot;
};

void SaxsviewSubWindow::SaxsviewSubWindowPrivate::setupUi() {
  sw->setAttribute(Qt::WA_DeleteOnClose);

  plot = new Saxsview::Plot(sw);
  sw->setWidget(plot);
}

SaxsviewSubWindow::SaxsviewSubWindow(QWidget *parent)
 : QMdiSubWindow(parent), p(new SaxsviewSubWindowPrivate(this)) {

  p->setupUi();

  static int id = 1;
  setWindowTitle(QString("Plot %1").arg(id++));
}

SaxsviewSubWindow::~SaxsviewSubWindow() {
  delete p;
}

int SaxsviewSubWindow::scale() const {
  return p->plot->scale();
}

void SaxsviewSubWindow::load(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  if (!fileInfo.exists())
    return;

  saxs_document *doc = saxs_document_create();
  saxs_document_read(doc, fileName.toAscii(), 0L);

  saxs_curve *curve = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  while (curve) {
    Saxsview::PlotPointData points;
    Saxsview::PlotIntervalData intervals;

    saxs_data *data = saxs_curve_data(curve);
    while (data) {
      const double x     = saxs_data_x(data);
      const double y     = saxs_data_y(data);
      const double y_err = saxs_data_y_err(data);

      points.push_back(QwtDoublePoint(x, y));
      intervals.push_back(QwtIntervalSample(x, QwtDoubleInterval(y-y_err, y+y_err)));

      data = saxs_data_next(data);
    }

    Saxsview::PlotCurve *plotCurve = new Saxsview::PlotCurve;
    plotCurve->setData(points, intervals);
    plotCurve->setTitle(fileInfo.fileName());

    p->plot->addCurve(plotCurve);

    curve = saxs_curve_find_next(curve, SAXS_CURVE_SCATTERING_DATA);
  }

  saxs_document_free(doc);
}

void SaxsviewSubWindow::saveAs() {
  p->plot->saveAs();
}

void SaxsviewSubWindow::print() {
  p->plot->print();
}

void SaxsviewSubWindow::zoomIn() {
//   p->plot->zoomIn();
}

void SaxsviewSubWindow::zoomOut() {
//   p->plot->zoomOut();
}

void SaxsviewSubWindow::setZoomEnabled(bool on) {
  p->plot->setZoomEnabled(on);
}

void SaxsviewSubWindow::setMoveEnabled(bool on) {
  p->plot->setMoveEnabled(on);
}

void SaxsviewSubWindow::setScale(int scale) {
  p->plot->setScale((Saxsview::Plot::PlotScale)scale);
}

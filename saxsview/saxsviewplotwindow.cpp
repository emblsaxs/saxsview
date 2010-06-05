/*
 * Implementation of 1D-plot subwindows.
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

#include "saxsviewplotwindow.h"
#include "saxsview_plot.h"
#include "saxsview_plotcurve.h"
#include "saxsview_plotconfigdialog.h"

#include "saxsdocument.h"
#include "saxsdocument_format.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <QUrl>

class SaxsviewPlotWindow::SaxsviewPlotWindowPrivate {
public:
  SaxsviewPlotWindowPrivate(SaxsviewPlotWindow *w) : sw(w) {}

  void setupUi();

  SaxsviewPlotWindow *sw;
  Saxsview::Plot *plot;
};

void SaxsviewPlotWindow::SaxsviewPlotWindowPrivate::setupUi() {
  sw->setAttribute(Qt::WA_DeleteOnClose);

  plot = new Saxsview::Plot(sw);
  sw->setWidget(plot);

  plot->setAcceptDrops(true);
  plot->installEventFilter(sw);
}

SaxsviewPlotWindow::SaxsviewPlotWindow(QWidget *parent)
 : SaxsviewSubWindow(parent), p(new SaxsviewPlotWindowPrivate(this)) {

  p->setupUi();
  setScale(Saxsview::Plot::Log10Scale);

  static int id = 1;
  setWindowTitle(QString("Plot %1").arg(id++));
}

SaxsviewPlotWindow::~SaxsviewPlotWindow() {
  delete p;
}

bool SaxsviewPlotWindow::canShow(const QString& fileName) {
  saxs_document_format *format = saxs_document_format_find(fileName.toAscii(), 0L);
  return format && format->read;
}

int SaxsviewPlotWindow::scale() const {
  return p->plot->scale();
}

bool SaxsviewPlotWindow::zoomEnabled() const {
  return p->plot->zoomEnabled();
}

bool SaxsviewPlotWindow::moveEnabled() const {
  return p->plot->moveEnabled();
}

void SaxsviewPlotWindow::load(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  if (!fileInfo.exists())
    return;

  if (!canShow(fileName)) {
    QMessageBox::critical(this,
                          "Filetype not recognized",
                          QString("Could not load file as plot:\n"
                                  "'%1'.").arg(fileName));
    return;
  }

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

      data = saxs_data_next(data);
      if (y - y_err < 1e-6)
        continue;

      points.push_back(QPointF(x, y));
      intervals.push_back(QwtIntervalSample(x, y - y_err, y + y_err));
    }

    Saxsview::PlotCurve *plotCurve = new Saxsview::PlotCurve;
    plotCurve->setData(points, intervals);
    if (plotCurve->boundingRect().isValid()) {
      QString curveTitle = fileInfo.fileName();
      if (saxs_curve_title(curve))
        curveTitle += QString(" (%1)").arg(saxs_curve_title(curve));

      plotCurve->setTitle(curveTitle);
      plotCurve->setFileName(fileInfo.absoluteFilePath());

      p->plot->addCurve(plotCurve);

    } else
      delete plotCurve;

    curve = saxs_curve_find_next(curve, SAXS_CURVE_SCATTERING_DATA);
  }

  saxs_document_free(doc);
}

void SaxsviewPlotWindow::exportAs(const QString& fileName) {
  p->plot->exportAs(fileName);
}

void SaxsviewPlotWindow::print() {
  p->plot->print();
}

void SaxsviewPlotWindow::zoomIn() {
//   p->plot->zoomIn();
}

void SaxsviewPlotWindow::zoomOut() {
//   p->plot->zoomOut();
}

void SaxsviewPlotWindow::setZoomEnabled(bool on) {
  p->plot->setZoomEnabled(on);
}

void SaxsviewPlotWindow::setMoveEnabled(bool on) {
  p->plot->setMoveEnabled(on);
}

void SaxsviewPlotWindow::setScale(int scale) {
  p->plot->setScale((Saxsview::Plot::PlotScale)scale);
}

void SaxsviewPlotWindow::configure() {
  Saxsview::PlotConfigDialog dlg(p->plot, this);
  dlg.exec();
}

bool SaxsviewPlotWindow::eventFilter(QObject *watchedObj, QEvent *e) {
  if (watchedObj != p->plot)
    return QMdiSubWindow::eventFilter(watchedObj, e);

  switch (e->type()) {
    case QEvent::Drop:
      if (QDropEvent *dropEvent = dynamic_cast<QDropEvent*>(e)) {
        if (dropEvent->mimeData()->hasUrls())
          foreach (QUrl url, dropEvent->mimeData()->urls())
            load(url.toLocalFile());

        dropEvent->acceptProposedAction();
      }
      // fall through

    case QEvent::DragEnter:
    case QEvent::DragMove:
      e->accept();
      return true;

    default:
      return false;
  }
}

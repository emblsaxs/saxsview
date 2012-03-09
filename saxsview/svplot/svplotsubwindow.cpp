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

// saxsview/svplot
#include "svplotsubwindow.h"
#include "svplotmainwindow.h"
#include "svplotproject.h"

// libsaxsview
#include "saxsview.h"
#include "saxsview_plot.h"
#include "saxsview_plotcurve.h"

// libsaxsdocument
#include "saxsdocument.h"

// external/qwt
#include "qwt_picker_machine.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_picker.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

// global
#include <QtGui>


class SVPlotSubWindow::Private {
public:
  Private() : plot(0L) {
    project = new SVPlotProject();
  }

  void setupUi(SVPlotSubWindow *w);

  SVPlotProject *project;
  SaxsviewPlot *plot;
};

void SVPlotSubWindow::Private::setupUi(SVPlotSubWindow *w) {
  plot = new SaxsviewPlot(w);
  plot->setScale(Saxsview::Log10Scale);
  plot->setAcceptDrops(true);
  plot->installEventFilter(w);

  w->setAttribute(Qt::WA_DeleteOnClose);
  w->setWidget(plot);
}


SVPlotSubWindow::SVPlotSubWindow(QWidget *parent)
 : QMdiSubWindow(parent), p(new Private) {

  p->setupUi(this);

  static int id = 1;

  setWindowTitle(QString("Plot %1").arg(id++));
  p->project->addPlot(p->plot, windowTitle());
}

SVPlotSubWindow::~SVPlotSubWindow() {
  delete p;
}

SVPlotProject* SVPlotSubWindow::project() {
  return p->project;
}

SaxsviewPlot* SVPlotSubWindow::plot() const {
  return p->plot;
}

bool SVPlotSubWindow::zoomEnabled() const {
  return p->plot->isZoomEnabled();
}

bool SVPlotSubWindow::moveEnabled() const {
  return p->plot->isMoveEnabled();
}


bool SVPlotSubWindow::load(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  if (!fileInfo.exists())
    return false;

  setCursor(Qt::WaitCursor);

  saxs_document *doc = saxs_document_create();
  if (saxs_document_read(doc, fileName.toAscii(), 0L) != 0) {
    qDebug() << "load failed";
    return false;
  }

  saxs_curve *curve = saxs_document_curve(doc);
  while (curve) {
    if (!(saxs_curve_type(curve) & SAXS_CURVE_SCATTERING_DATA)) {
    qDebug() << "skipped load" << saxs_curve_type(curve) << SAXS_CURVE_SCATTERING_DATA;
      curve = saxs_curve_next(curve);
      continue;
    }

    SaxsviewPlotPointData points;
    SaxsviewPlotIntervalData intervals;

    saxs_data *data = saxs_curve_data(curve);
    while (data) {
      const double x     = saxs_data_x(data);
      const double y     = saxs_data_y(data);
      const double y_err = saxs_data_y_err(data);

      points.push_back(QPointF(x, y));
      intervals.push_back(QwtIntervalSample(x, y - y_err, y + y_err));

      data = saxs_data_next(data);
    }

    SaxsviewPlotCurve *plotCurve = new SaxsviewPlotCurve(saxs_curve_type(curve));
    plotCurve->setData(points, intervals);

    if (plotCurve->boundingRect().isValid()) {
      QString curveTitle = fileInfo.fileName();
      if (saxs_curve_title(curve))
        curveTitle += QString(" (%1)").arg(saxs_curve_title(curve));

      plotCurve->setTitle(curveTitle);
      plotCurve->setFileName(fileInfo.absoluteFilePath());

      p->plot->addCurve(plotCurve);
      p->project->addPlotCurve(plotCurve);

    } else
      qDebug() << "boundingrect invalid";

    curve = saxs_curve_next(curve);
  }

  saxs_document_free(doc);

  unsetCursor();
  return true;
}

void SVPlotSubWindow::reload() {
  // TODO
}

void SVPlotSubWindow::exportAs(const QString& fileName,
                                const QString& format) {
  p->plot->exportAs(fileName, format);
}

void SVPlotSubWindow::print() {
  p->plot->print();
}

void SVPlotSubWindow::zoomFit() {
//   p->plot->setZoomBase(p->plot->boundingRect());
}

void SVPlotSubWindow::setZoomEnabled(bool on) {
  p->plot->setZoomEnabled(on);
}

void SVPlotSubWindow::setMoveEnabled(bool on) {
  p->plot->setMoveEnabled(on);
}

bool SVPlotSubWindow::eventFilter(QObject *watched, QEvent *e) {
  if (watched != p->plot)
    return QMdiSubWindow::eventFilter(watched, e);

  switch (e->type()) {
    case QEvent::Drop:
    {
      QDropEvent *dropEvent = dynamic_cast<QDropEvent*>(e);
      if (dropEvent->mimeData()->hasUrls())
        foreach (QUrl url, dropEvent->mimeData()->urls())
          load(url.toLocalFile());

      dropEvent->acceptProposedAction();
      // fall through
    }

    case QEvent::DragEnter:
    case QEvent::DragMove:
      e->accept();
      return true;

    default:
      return QMdiSubWindow::eventFilter(watched, e);
  }
}

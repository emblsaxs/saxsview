/*
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of saxsview.
 *
 * saxsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * saxsview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with saxsview. If not, see <http://www.gnu.org/licenses/>.
 */

#include "saxsview_plot.h"
#include "saxsview_plotcurve.h"

#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QSvgGenerator>

#include <qwt_dyngrid_layout.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_engine.h>


namespace Saxsview {


class Plot::PlotPrivate {
public:
  PlotPrivate(Plot *p)
   : plot(p), scale(Log10Scale), blockReplot(false),
     legend(0L), marker(0L), panner(0L), zoomer(0L) {
  }

  void setupCanvas();
  void setupLegend();
  void setupMarker();
  void setupPanner();
  void setupZoomer();

  Plot *plot;
  PlotScale scale;

  bool blockReplot;

  QwtLegend *legend;
  QwtPlotMarker *marker;
  QwtPlotPanner *panner;
  QwtPlotZoomer *zoomer;
  QList<PlotCurve*> curves;
};

void Plot::PlotPrivate::setupCanvas() {
  // initial background color
  plot->setAutoFillBackground(true);
  plot->setPalette(Qt::white);
  plot->canvas()->setFrameStyle(QFrame::NoFrame);

  // to intercept right-click events
  plot->canvas()->installEventFilter(plot);
}

void Plot::PlotPrivate::setupLegend() {
  legend = new QwtLegend(plot->canvas());
  legend->show();

  // to intercept right-click events
  legend->installEventFilter(plot);

  plot->insertLegend(legend, QwtPlot::RightLegend);
}

void Plot::PlotPrivate::setupMarker() {
  // lines at x=0, y=0
  marker = new QwtPlotMarker();
  marker->setLineStyle(QwtPlotMarker::Cross);
  marker->setValue(0.0, 0.0);
  marker->attach(plot);
}

void Plot::PlotPrivate::setupPanner() {
  //
  // QwtPanner:
  //   "QwtPanner grabs the content of the widget into a pixmap and moves
  //    the pixmap around, without initiating any repaint events for the
  //    widget. Areas, that are not part of content are not painted while
  //    panning in in process. This makes panning fast enough for widgets,
  //    where repaints are too slow for mouse movements."
  //
  // QwtPlotPanner:
  //   "Note: The axes are not updated, while dragging the canvas"
  //
  panner = new QwtPlotPanner(plot->canvas());
  panner->setCursor(Qt::SizeAllCursor);
  panner->setEnabled(false);
}

void Plot::PlotPrivate::setupZoomer() {
  zoomer = new QwtPlotZoomer(plot->canvas());
  zoomer->setEnabled(true);

  // RightButton: zoom out by 1
  zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                          Qt::RightButton);

  // Ctrl+RightButton: zoom out to full size
  zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                          Qt::RightButton, Qt::ControlModifier);
}


Plot::Plot(QWidget *parent)
 : QwtPlot(parent), p(new PlotPrivate(this)) {

//   Q_INIT_RESOURCE(libsaxsview);

  // margin around the plot
  plotLayout()->setMargin(12);

  p->setupCanvas();
  p->setupLegend();
  p->setupMarker();
  p->setupPanner();
  p->setupZoomer();

  setScale(Log10Scale);
}

Plot::~Plot() {
  clear();
  delete p->marker;
  delete p->legend;
  delete p;
}

void Plot::replot() {
  if (!replotBlocked())
    QwtPlot::replot();
}

void Plot::blockReplot(bool blocked) {
  //
  // When lifiting the blockage, do a replot
  // to show all changes.
  //
  const bool replotNow = p->blockReplot && !blocked;

  p->blockReplot = blocked;

  if (replotNow)
    replot();
}

bool Plot::replotBlocked() const {
  return p->blockReplot;
}

void Plot::clear() {
  foreach (PlotCurve *curve, p->curves) {
    curve->detach();
    delete curve;
  }

  p->curves.clear();
  replot();
}

void Plot::exportAs() {
  QString fileName = QFileDialog::getSaveFileName(this, "Export As",
                                                  QDir::currentPath(),
                                                  "All files (*.*)");
  exportAs(fileName);
}

void Plot::exportAs(const QString& fileName) {
  if (fileName.isEmpty())
    return;

  QString ext = QFileInfo(fileName).completeSuffix();

  bool bitmap = (ext == "bmp");
#ifdef QT_IMAGEFORMAT_PNG
  bitmap = bitmap || (ext == "png");
#endif
#ifdef QT_IMAGEFORMAT_JPEG
  bitmap = bitmap || (ext == "jpg") || ext == "jpeg";
#endif

  if (bitmap) {
    QPixmap::grabWidget(this).save(fileName);

#ifdef QT_SVG_LIB
  } else if (ext == "svg") {
    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(QSize(800, 600));
    QwtPlot::print(generator);
#endif
  } else if (ext == "ps" || ext == "pdf") {
    QPrinter printer(QPrinter::HighResolution);
    printer.setOrientation(QPrinter::Landscape);
    printer.setOutputFileName(fileName);
    QwtPlot::print(printer);

  } else
    QMessageBox::warning(this, "Not supported",
                         QString("File format \".%1\" is not supported").arg(ext));
}

void Plot::print() {
  //
  // FIXME: The indicator of legend-items is not properly
  //        scaled
  //
  QPrinter printer(QPrinter::HighResolution);
  printer.setOrientation(QPrinter::Landscape);

  QPrintDialog dlg(&printer, this);
  if (dlg.exec() == QDialog::Accepted)
    QwtPlot::print(printer);
}

void Plot::configure() {
//   PlotProperties config(this);
//   config.exec();
}

void Plot::addCurve(PlotCurve *curve) {
  p->curves.push_back(curve);
  curve->attach(this);

  //
  // Compute the overall bounding rect, push
  // that topmost on the stack.
  //
  QRectF boundingRect;
  foreach (PlotCurve *curve, p->curves)
    boundingRect = boundingRect.united(curve->boundingRect());

  //
  // This seems to be wierd, but gives the best possible results.
  // E.g. if zoomBase() is not set before the initial zoom, an all-negative
  // curve will result in an initial zoom to an (0,0,0x0) rect.
  //
  p->zoomer->setZoomBase(boundingRect);
  p->zoomer->zoom(boundingRect);
  p->zoomer->setZoomBase(boundingRect);

  replot();
}

void Plot::removeCurve(PlotCurve *curve) {
  int i = p->curves.indexOf(curve);

  if (i >= 0) {
    p->curves[i]->detach();
    p->curves.removeAt(i);
//     updateLayout();
  }
}

QList<PlotCurve*> Plot::curves() const {
  return p->curves;
}

bool Plot::eventFilter(QObject *watchedObj, QEvent *e) {
  //
  // Open a context menu on the legend to allow selection of curves.
  //
  // If no curves are visible, Qwt sets the legend's width to '0'.
  // Thus, if all curves have been disabled, show the context menu
  // on the canvas instead.
  //
  if (watchedObj == p->legend
      || (watchedObj == canvas()
          && !p->curves.isEmpty()
          && p->legend && p->legend->width() == 0)) {

    if (QMouseEvent *me = dynamic_cast<QMouseEvent*>(e)) {
      if (me->button() == Qt::RightButton) {
        QMenu contextMenu(this);
        foreach (PlotCurve *curve, p->curves) {
          QAction *action = contextMenu.addAction(curve->title());
          action->setCheckable(true);
          action->setChecked(curve->isVisible());
          connect(action, SIGNAL(toggled(bool)),
                  curve, SLOT(setVisible(bool)));
        }
        contextMenu.exec(me->globalPos());
        return true;
      }
    }
  }

  return QwtPlot::eventFilter(watchedObj, e);
}

void Plot::setZoomEnabled(bool on) {
  p->zoomer->setEnabled(on);
}

bool Plot::zoomEnabled() const {
  return p->zoomer->isEnabled();
}

void Plot::setMoveEnabled(bool on) {
  p->panner->setEnabled(on);
}

bool Plot::moveEnabled() const {
  return p->panner->isEnabled();
}

void Plot::setScale(PlotScale scale) {
  switch(scale) {
    case AbsoluteScale:
      setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
      break;

    case Log10Scale:
      setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
  }

  p->scale = scale;
  replot();
}

Plot::PlotScale Plot::scale() const {
  return p->scale;
}



// void Plot::printLegend(QPainter *painter, const QRect &) const {
//   //
//   // Positioning of legend on print.
//   //
//   // Computations are done in device coordinates, converted to layout
//   // coordinates (to be converted to device coordinates).
//   //
//   //  -> makes sure that the legend is positioned properly on the
//   //     right hand side.
//   //
//   const QwtMetricsMap &metricsMap = QwtPainter::metricsMap();
// 
//   QSize mappedSizeHint = metricsMap.layoutToDevice(p->legend->sizeHint());
//   const int mappedMargin = metricsMap.layoutToDeviceY(plotLayout()->margin());
//   const int mappedTitleHeight = metricsMap.layoutToDeviceY(plotLayout()->titleRect().height());
// 
//   QRect rect(painter->device()->width() - mappedMargin - mappedSizeHint.width(),
//              mappedMargin + mappedTitleHeight,
//              mappedSizeHint.width(), mappedSizeHint.height());
//   rect = metricsMap.deviceToLayout(rect);
// 
//   QwtPlot::printLegend(painter, rect);
// }

} // end of namespace Saxsview

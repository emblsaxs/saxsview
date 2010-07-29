/*
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
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
#include "saxsview_config.h"

#include <QtGui>

#include "qwt_dyngrid_layout.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include "qwt_painter.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_scaleitem.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"


namespace Saxsview {

/* An external legend, i.e. a legend shown on top of the
   plot canvas, needs to be positioned properly. */
class PlotRenderer : public QwtPlotRenderer {
protected:
  void renderLegend(QPainter *painter, const QRectF &r) const {
    QRectF rect = r;

    if (!rect.isValid()) {
      QRectF canvasRect = plot()->plotLayout()->canvasRect();
      QSize legendSizeHint = plot()->legend()->sizeHint();

      rect.setTop(30);
      rect.setLeft(canvasRect.right() - legendSizeHint.width() - 30);
      rect.setWidth(legendSizeHint.width());
      rect.setHeight(legendSizeHint.height());
    }

    QwtPlotRenderer::renderLegend(painter, rect);
  }
};



class Plot::PlotPrivate {
public:
  PlotPrivate(Plot *p)
   : plot(p), scale(Log10Scale), blockReplot(false),
     legend(0L), marker(0L), panner(0L), zoomer(0L) {
  }

  void setupCanvas();
  void setupLegend();
  void setupPanner();
  void setupZoomer();
  void setupScales();

  void setupDefaultColors();
  void setStyle(PlotCurve *curve);

  Plot *plot;
  PlotScale scale;

  bool blockReplot;

  QwtLegend *legend;
  QwtPlotMarker *marker;
  QwtPlotPanner *panner;
  QwtPlotZoomer *zoomer;
  QList<PlotCurve*> curves;
  QList<QColor> defaultLineColor, defaultErrorBarColor;
  int currentLineColor, currentErrorBarColor;
};

void Plot::PlotPrivate::setupCanvas() {
  // initial background color
  plot->setAutoFillBackground(true);
  plot->setPalette(Qt::white);
  plot->canvas()->setFrameStyle(QFrame::NoFrame);
  plot->canvas()->setLineWidth(1);

  // to intercept right-click events
  plot->canvas()->installEventFilter(plot);
}

void Plot::PlotPrivate::setupLegend() {
  legend = new QwtLegend(plot->canvas());

  QLayout *layout = legend->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  ll->setMaxCols(1);
  ll->setMargin(6);
  ll->setSpacing(0);

  // to intercept right-click events
  legend->installEventFilter(plot);
  legend->show();

  plot->insertLegend(legend, QwtPlot::RightLegend);
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

void Plot::PlotPrivate::setupScales() {
  QwtPlotScaleItem *yRight = new QwtPlotScaleItem(QwtScaleDraw::LeftScale);
  yRight->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  yRight->attach(plot);
  yRight->setBorderDistance(1);

  QwtPlotScaleItem *yLeft = new QwtPlotScaleItem(QwtScaleDraw::RightScale);
  yLeft->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  yLeft->attach(plot);
  yLeft->setBorderDistance(0);

  QwtPlotScaleItem *xTop = new QwtPlotScaleItem(QwtScaleDraw::BottomScale);
  xTop->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  xTop->attach(plot);
  xTop->setBorderDistance(0);

  QwtPlotScaleItem *xBottom = new QwtPlotScaleItem(QwtScaleDraw::TopScale);
  xBottom->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  xBottom->attach(plot);
  xBottom->setBorderDistance(1);

  QwtScaleDraw *scaleDraw = plot->axisScaleDraw(QwtPlot::yLeft);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, false);

  scaleDraw = plot->axisScaleDraw(QwtPlot::xBottom);
  scaleDraw->enableComponent(QwtAbstractScaleDraw:: Backbone, false);
  scaleDraw->enableComponent(QwtAbstractScaleDraw:: Ticks, false);

  plot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, false);
}

void Plot::PlotPrivate::setupDefaultColors() {
  config().defaultColors(defaultLineColor, defaultErrorBarColor);
  currentLineColor = currentErrorBarColor = 0; 
}

void Plot::PlotPrivate::setStyle(PlotCurve *curve) {
  config().applyTemplate(curve);

  QColor lineColor = defaultLineColor[currentLineColor++ % defaultLineColor.size()];
  QPen linePen = curve->pen();
  linePen.setColor(lineColor);
  curve->setPen(linePen);

  PlotSymbol symbol = curve->symbol();
  symbol.setColor(lineColor);
  curve->setSymbol(symbol);

  QColor errorBarColor = defaultErrorBarColor[currentErrorBarColor++ % defaultErrorBarColor.size()];
  QPen errorPen = curve->errorBarPen();
  errorPen.setColor(errorBarColor);
  curve->setErrorBarPen(errorPen);
}




Plot::Plot(QWidget *parent)
 : QwtPlot(parent), p(new PlotPrivate(this)) {

//   Q_INIT_RESOURCE(libsaxsview);

  // margin around the plot
  plotLayout()->setMargin(12);
  plotLayout()->setAlignCanvasToScales(true);

  p->setupScales();
  p->setupCanvas();
  p->setupLegend();
  p->setupPanner();
  p->setupZoomer();
  p->setupDefaultColors();
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

  if (replotNow) {
    updateLayout();
    replot();
  }
}

bool Plot::replotBlocked() const {
  return p->blockReplot;
}

void Plot::clear() {
  // FIXME: is this equivalent to QwtPlotDict::detachItems() ?

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

  if (ext == "ps" || ext == "pdf" || ext == "svg") {

    //
    // FIXME: there's no "do you want to overwrite" dialog if a
    //        file already exists?!
    //
    PlotRenderer renderer;
    renderer.setLayoutFlag(QwtPlotRenderer::KeepMargins);
    renderer.renderDocument(this, fileName, size()*25.4/85, 600);

  } else
    QPixmap::grabWidget(this).save(fileName);
}

void Plot::print() {
  QString printerName = config().recentPrinter();

  QPrinter printer(QPrinter::HighResolution);
  printer.setOrientation(QPrinter::Landscape);
  if (!printerName.isEmpty())
    printer.setPrinterName(printerName);

  QPrintDialog dlg(&printer, this);
  if (dlg.exec() == QDialog::Accepted) {
    PlotRenderer renderer;
    renderer.renderTo(this, printer);
  }

  config().setRecentPrinter(printer.printerName());
}

void Plot::configure() {
//   PlotProperties config(this);
//   config.exec();
}

void Plot::addCurve(PlotCurve *curve) {
  p->curves.push_back(curve);
  p->setStyle(curve);

  curve->attach(this);

  updateLayout();
  setZoomBase();
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

void Plot::updateLayout() {
  QwtPlot::updateLayout();

  if (plotLayout()->legendPosition() == QwtPlot::ExternalLegend) {
    QSize legendSizeHint = legend()->sizeHint();
    legend()->setGeometry(width() - legendSizeHint.width() - 30, 30,
                          legendSizeHint.width(), legendSizeHint.height());
  }
}

void Plot::setZoomBase(const QRectF& rect) {
  QRectF r = rect;

  //
  // If no rect is specified, compute the overall bounding
  // rect of all visible curves.
  //
  if (!r.isValid()) {
    foreach (PlotCurve *curve, p->curves)
      if (curve->isVisible())
        r = r.united(curve->boundingRect());

    // make it two signigificant digits
    r.setLeft(qFloor(r.left() * 100.0) / 100.0);
    r.setRight(qCeil(r.right() * 100.0) / 100.0);
  }

  if (r.isValid()) {
    //
    // This seems to be wierd, but gives the best possible results.
    // E.g. if zoomBase() is not set before the initial zoom, an all-negative
    // curve will result in an initial zoom to an (0,0,0x0) rect.
    //
    p->zoomer->setZoomBase(r);
    p->zoomer->zoom(r);
    p->zoomer->setZoomBase(r);
  }

  replot();
}

QRectF Plot::zoomBase() const {
  return p->zoomer->zoomBase();
}

void Plot::zoom(const QRectF& rect) {
  p->zoomer->zoom(rect);
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

} // end of namespace Saxsview

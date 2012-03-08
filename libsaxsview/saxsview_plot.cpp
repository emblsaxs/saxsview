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

#include "saxsview.h"
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
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"


/* An external legend, i.e. a legend shown on top of the
   plot canvas, needs to be positioned properly. */
class SaxsviewPlotRenderer : public QwtPlotRenderer {
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



class SaxsviewPlot::Private {
public:
  Private(SaxsviewPlot *p)
   : plot(p), scale(Saxsview::Log10Scale), blockReplot(false),
     legend(0L), marker(0L), panner(0L), zoomer(0L) {
  }

  void setupCanvas();
  void setupLegend();
  void setupPanner();
  void setupZoomer();
  void setupScales();

  void setupDefaultColors();
  void setStyle(SaxsviewPlotCurve *curve);

  SaxsviewPlot *plot;
  Saxsview::Scale scale;

  bool blockReplot;

  QwtLegend *legend;
  QwtPlotMarker *marker;
  QwtPlotPanner *panner;
  QwtPlotZoomer *zoomer;
  QList<SaxsviewPlotCurve*> curves;
  QList<QColor> defaultLineColor, defaultErrorBarColor;
  int currentLineColor, currentErrorBarColor;
};

void SaxsviewPlot::Private::setupCanvas() {
  // initial background color
  plot->setAutoFillBackground(true);
  plot->setPalette(Qt::white);
  plot->canvas()->setFrameStyle(QFrame::NoFrame);
  plot->canvas()->setLineWidth(1);

  // to intercept right-click events
  plot->canvas()->installEventFilter(plot);
}

void SaxsviewPlot::Private::setupLegend() {
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

void SaxsviewPlot::Private::setupPanner() {
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

void SaxsviewPlot::Private::setupZoomer() {
  zoomer = new QwtPlotZoomer(plot->canvas());
  zoomer->setEnabled(true);

  // RightButton: zoom out by 1
  zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                          Qt::RightButton);

  // Ctrl+RightButton: zoom out to full size
  zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                          Qt::RightButton, Qt::ControlModifier);
}

void SaxsviewPlot::Private::setupScales() {
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

void SaxsviewPlot::Private::setupDefaultColors() {
  config().defaultColors(defaultLineColor, defaultErrorBarColor);
  currentLineColor = currentErrorBarColor = 0; 
}

void SaxsviewPlot::Private::setStyle(SaxsviewPlotCurve *curve) {
  config().applyTemplate(curve);

  QColor lineColor = defaultLineColor[currentLineColor++ % defaultLineColor.size()];
  curve->setLineColor(lineColor);
  curve->setSymbolColor(lineColor);

  QColor errorBarColor = defaultErrorBarColor[currentErrorBarColor++ % defaultErrorBarColor.size()];
  curve->setErrorLineColor(errorBarColor);
}




SaxsviewPlot::SaxsviewPlot(QWidget *parent)
 : QwtPlot(parent), p(new Private(this)) {

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

SaxsviewPlot::~SaxsviewPlot() {
  clear();
  delete p->marker;
  delete p->legend;
  delete p;
}

void SaxsviewPlot::replot() {
  if (!replotBlocked())
    QwtPlot::replot();
}

void SaxsviewPlot::blockReplot(bool blocked) {
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

bool SaxsviewPlot::replotBlocked() const {
  return p->blockReplot;
}

void SaxsviewPlot::clear() {
  // FIXME: is this equivalent to QwtPlotDict::detachItems() ?

  foreach (SaxsviewPlotCurve *curve, p->curves) {
    curve->detach();
    delete curve;
  }

  p->curves.clear();
  replot();
}

void SaxsviewPlot::exportAs() {
  QString fileName = QFileDialog::getSaveFileName(this, "Export As",
                                                  config().recentDirectory(),
                                                  "All files (*.*)");
  exportAs(fileName);
}

void SaxsviewPlot::exportAs(const QString& fileName, const QString& format) {
  if (fileName.isEmpty())
    return;

  QString ext = format.isEmpty() ? QFileInfo(fileName).completeSuffix() : format;

  if (ext == "ps" || ext == "pdf" || ext == "svg") {
    SaxsviewPlotRenderer renderer;
    renderer.setLayoutFlag(QwtPlotRenderer::KeepMargins);
    renderer.renderDocument(this, fileName, ext, size()*25.4/85, 600);

  } else
    QPixmap::grabWidget(this).save(fileName, qPrintable(ext));
}

void SaxsviewPlot::print() {
  QString printerName = config().recentPrinter();

  QPrinter printer(QPrinter::HighResolution);
  printer.setOrientation(QPrinter::Landscape);
  if (!printerName.isEmpty())
    printer.setPrinterName(printerName);

  QPrintDialog dlg(&printer, this);
  if (dlg.exec() == QDialog::Accepted) {
    SaxsviewPlotRenderer renderer;
    renderer.renderTo(this, printer);
  }

  config().setRecentPrinter(printer.printerName());
}

void SaxsviewPlot::configure() {
//   PlotProperties config(this);
//   config.exec();
}

void SaxsviewPlot::addCurve(SaxsviewPlotCurve *curve) {
  p->curves.push_back(curve);
  p->setStyle(curve);

  curve->attach(this);

  updateLayout();
  setZoomBase();
}

void SaxsviewPlot::removeCurve(SaxsviewPlotCurve *curve) {
  int i = p->curves.indexOf(curve);

  if (i >= 0) {
    p->curves[i]->detach();
    p->curves.removeAt(i);
//     updateLayout();
  }
}

QList<SaxsviewPlotCurve*> SaxsviewPlot::curves() const {
  return p->curves;
}

bool SaxsviewPlot::eventFilter(QObject *watchedObj, QEvent *e) {
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
        foreach (SaxsviewPlotCurve *curve, p->curves) {
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

void SaxsviewPlot::updateLayout() {
  QwtPlot::updateLayout();

  if (plotLayout()->legendPosition() == QwtPlot::ExternalLegend) {
    QSize legendSizeHint = legend()->sizeHint();
    legend()->setGeometry(width() - legendSizeHint.width() - 30, 30,
                          legendSizeHint.width(), legendSizeHint.height());
  }
}

void SaxsviewPlot::setZoomBase(const QRectF& rect) {
  QRectF r = rect;

  //
  // If no rect is specified, compute the overall bounding
  // rect of all visible curves.
  //
  if (!r.isValid()) {
    foreach (SaxsviewPlotCurve *curve, p->curves)
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

QRectF SaxsviewPlot::zoomBase() const {
  return p->zoomer->zoomBase();
}

void SaxsviewPlot::zoom(const QRectF& rect) {
  p->zoomer->zoom(rect);
}

void SaxsviewPlot::setZoomEnabled(bool on) {
  p->zoomer->setEnabled(on);
}

bool SaxsviewPlot::isZoomEnabled() const {
  return p->zoomer->isEnabled();
}

void SaxsviewPlot::setMoveEnabled(bool on) {
  p->panner->setEnabled(on);
}

bool SaxsviewPlot::isMoveEnabled() const {
  return p->panner->isEnabled();
}

void SaxsviewPlot::setScale(Saxsview::Scale scale) {
  switch(scale) {
    case Saxsview::AbsoluteScale:
      setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
      break;

    case Saxsview::Log10Scale:
      setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
  }

  p->scale = scale;
  replot();
}

Saxsview::Scale SaxsviewPlot::scale() const {
  return p->scale;
}

void SaxsviewPlot::setPlotTitle(const QString& text) {
  QwtText t = title();
  t.setText(text);
  setTitle(t);
}

QString SaxsviewPlot::plotTitle() const {
  return title().text();
}

void SaxsviewPlot::setPlotTitleFont(const QFont& font) {
  QwtText t = title();
  t.setFont(font);
  setTitle(t);
}

QFont SaxsviewPlot::plotTitleFont() const {
  return title().font();
}

void SaxsviewPlot::setAxisTitleX(const QString& text) {
  QwtText t = axisTitle(QwtPlot::xBottom);
  t.setText(text);
  setAxisTitle(QwtPlot::xBottom, t);
}

QString SaxsviewPlot::axisTitleX() const {
  return axisTitle(QwtPlot::xBottom).text();
}

void SaxsviewPlot::setAxisTitleY(const QString& text) {
  QwtText t = axisTitle(QwtPlot::yLeft);
  t.setText(text);
  setAxisTitle(QwtPlot::yLeft, t);
}

QString SaxsviewPlot::axisTitleY() const {
  return axisTitle(QwtPlot::yLeft).text();
}

void SaxsviewPlot::setAxisTitleFont(const QFont& font) {
  QwtText t = axisTitle(QwtPlot::xBottom);
  t.setFont(font);
  setAxisTitle(QwtPlot::xBottom, t);
}

QFont SaxsviewPlot::axisTitleFont() const {
  return axisTitle(QwtPlot::xBottom).font();
}

void SaxsviewPlot::setTicksEnabledX(bool on) {
  QwtScaleDraw *scale = axisScaleDraw(QwtPlot::xBottom);
  scale->enableComponent(QwtAbstractScaleDraw::Labels, on);
  updateLayout();
}

bool SaxsviewPlot::ticksEnabledX() const {
  const QwtScaleDraw *scale = axisScaleDraw(QwtPlot::xBottom);
  return scale->hasComponent(QwtAbstractScaleDraw::Labels);
}

void SaxsviewPlot::setTicksEnabledY(bool on) {
  QwtScaleDraw *scale = axisScaleDraw(QwtPlot::yLeft);
  scale->enableComponent(QwtAbstractScaleDraw::Labels, on);
  updateLayout();
}

bool SaxsviewPlot::ticksEnabledY() const {
  const QwtScaleDraw *scale = axisScaleDraw(QwtPlot::yLeft);
  return scale->hasComponent(QwtAbstractScaleDraw::Labels);
}

void SaxsviewPlot::setTicksFont(const QFont& font) {
  setAxisFont(QwtPlot::xBottom, font);
  setAxisFont(QwtPlot::yLeft, font);
}

QFont SaxsviewPlot::ticksFont() const {
  return axisFont(QwtPlot::xBottom);
}

void SaxsviewPlot::setLegendEnabled(bool) {
  
}

bool SaxsviewPlot::legendEnabled() const {
  
}

void SaxsviewPlot::setLegendPosition(SaxsviewPlot::LegendPosition pos) {
  plotLayout()->setLegendPosition((QwtPlot::LegendPosition)pos);
  updateLayout();
}

SaxsviewPlot::LegendPosition SaxsviewPlot::legendPosition() const {
  return (SaxsviewPlot::LegendPosition)plotLayout()->legendPosition();
}

void SaxsviewPlot::setLegendColumnCount(int n) {
  QLayout *layout = legend()->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  ll->setMaxCols(n);
}

int SaxsviewPlot::legendColumnCount() const {
  QLayout *layout = legend()->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  return ll->maxCols();
}

void SaxsviewPlot::setLegendSpacing(int n) {
  QLayout *layout = legend()->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  ll->setMaxCols(n);
}

int SaxsviewPlot::legendSpacing() const {
  QLayout *layout = legend()->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  return ll->spacing();
}

void SaxsviewPlot::setLegendMargin(int n) {
  QLayout *layout = legend()->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  ll->setMaxCols(n);
}

int SaxsviewPlot::legendMargin() const {
  QLayout *layout = legend()->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  return ll->margin();
}

void SaxsviewPlot::setLegendFont(const QFont& font) {
}

QFont SaxsviewPlot::legendFont() const {
  return QFont();
}

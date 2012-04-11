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
#include "saxsview_scaledraw.h"
#include "saxsview_transformation.h"

#include <QtGui>

#include "qwt_dyngrid_layout.h"
#include "qwt_legend.h"
#include "qwt_painter.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_legenditem.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_scaleitem.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"
#include "qwt_text_label.h"



class SaxsviewPlot::Private {
public:
  Private(SaxsviewPlot *p)
   : plot(p), transformation(0), antiAliasing(false), blockReplot(false),
     legend(0L), marker(0L), panner(0L), zoomer(0L) {
  }

  void setupCanvas();
  void setupLegend();
  void setupPanner();
  void setupZoomer();
  void setupScales();

  void setupDefaultColors();
  void setStyle(SaxsviewPlotCurve *curve);
  void setTransformation(SaxsviewPlotCurve *curve);

  SaxsviewPlot *plot;
  int transformation;

  bool antiAliasing;
  bool blockReplot;

  QwtPlotLegendItem *legend;
  QwtPlotScaleItem *scales[QwtPlot::axisCnt];
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
}

void SaxsviewPlot::Private::setupLegend() {
  legend = new QwtPlotLegendItem;
  legend->setRenderHint(QwtPlotItem::RenderAntialiased);
  legend->setBorderPen(Qt::NoPen);
  legend->setBackgroundBrush(Qt::NoBrush);
  legend->setMaxColumns(1);
  legend->setMargin(6);
  legend->setSpacing(0);
  legend->setAlignment(Qt::AlignRight | Qt::AlignTop);
  legend->attach(plot);
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
  scales[QwtPlot::yRight] = yRight;

  QwtPlotScaleItem *yLeft = new QwtPlotScaleItem(QwtScaleDraw::RightScale);
  yLeft->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  yLeft->attach(plot);
  yLeft->setBorderDistance(0);
  scales[QwtPlot::yLeft] = yLeft;

  QwtPlotScaleItem *xTop = new QwtPlotScaleItem(QwtScaleDraw::BottomScale);
  xTop->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  xTop->attach(plot);
  xTop->setBorderDistance(0);
  scales[QwtPlot::xTop] = xTop;

  QwtPlotScaleItem *xBottom = new QwtPlotScaleItem(QwtScaleDraw::TopScale);
  xBottom->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  xBottom->attach(plot);
  xBottom->setBorderDistance(1);
  scales[QwtPlot::xBottom] = xBottom;

  SaxsviewScaleDraw *scaleDraw;
  scaleDraw = new SaxsviewScaleDraw;
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, false);
  plot->setAxisScaleDraw(QwtPlot::yLeft, scaleDraw);

  scaleDraw = new SaxsviewScaleDraw;
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, false);
  plot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);

  plot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, false);
}

void SaxsviewPlot::Private::setupDefaultColors() {
  config().colors(defaultLineColor, defaultErrorBarColor);
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

void SaxsviewPlot::Private::setTransformation(SaxsviewPlotCurve *curve) {
  QStandardItemModel model;
  config().plotScaleTransformations(&model);

  if (model.rowCount() >= transformation)
    transformation = 0;

  SaxsviewTransformation *t = new SaxsviewTransformation;
  t->setTransformationX(model.item(transformation, 2)->text());
  t->setTransformationY(model.item(transformation, 4)->text());

  curve->setTransformation(t);
}


SaxsviewPlot::SaxsviewPlot(QWidget *parent)
 : QwtPlot(parent), p(new Private(this)) {

  // margin around the plot
  setContentsMargins(12, 12, 12, 12);
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
    QwtPlotRenderer renderer;
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
    QwtPlotRenderer renderer;
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
  p->setTransformation(curve);

  curve->attach(this);

  //
  // Not nice, but the quickest way for now to set the
  // anti-aliasing render hint for the added curve to be
  // the same as the already existing curves. Otherwise
  // we have some curve with and some without anti-aliasing.
  // Ouch.
  //
  setAntiAliasing(p->antiAliasing);

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

void SaxsviewPlot::setTransformation(int i) {
  QStandardItemModel model;
  config().plotScaleTransformations(&model);

  if (model.rowCount() < i)
    i = 0;

  p->transformation = i;
  setTitle(model.item(i, 1)->text());
  setAxisTitleX(model.item(i, 3)->text());
  setAxisTitleY(model.item(i, 5)->text());

  foreach (SaxsviewPlotCurve *curve, curves()) {
    SaxsviewTransformation *t = new SaxsviewTransformation;
    t->setTransformationX(model.item(i, 2)->text());
    t->setTransformationY(model.item(i, 4)->text());

    curve->setTransformation(t);
  }

  replot();
  setZoomBase();
}

int SaxsviewPlot::transformation() const {
  return p->transformation;
}

void SaxsviewPlot::setBackgroundColor(const QColor& c) {
  QPalette pal = palette();
  pal.setColor(QPalette::Window, c);

  // The plot.
  setPalette(pal);

  // The scales.
  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i)
    p->scales[i]->setPalette(pal);

  replot();
}

QColor SaxsviewPlot::backgroundColor() const {
  return palette().color(QPalette::Window);
}

void SaxsviewPlot::setForegroundColor(const QColor& c) {
  QPalette pal = palette();
  pal.setColor(QPalette::WindowText, c);

  // The plot.
  setPalette(pal);

  // The scales.
  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i)
    p->scales[i]->setPalette(pal);

  replot();
}

QColor SaxsviewPlot::foregroundColor() const {
  return palette().color(QPalette::WindowText);
}

void SaxsviewPlot::setAntiAliasing(bool on) {
  p->antiAliasing = on;
  foreach (QwtPlotItem *item, itemList())
    item->setRenderHint(QwtPlotItem::RenderAntialiased, on);

  replot();
}

bool SaxsviewPlot::antiAliasing() const {
  return p->antiAliasing;
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

void SaxsviewPlot::setPlotTitleFontColor(const QColor& c) {
  QPalette pal = titleLabel()->palette();
  pal.setColor(QPalette::Text, c);
  titleLabel()->setPalette(pal);

  replot();
}

QColor SaxsviewPlot::plotTitleFontColor() const {
  return titleLabel()->palette().color(QPalette::Text);
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
  QwtText t;
  t = axisTitle(QwtPlot::xBottom);
  t.setFont(font);
  setAxisTitle(QwtPlot::xBottom, t);

  t = axisTitle(QwtPlot::yLeft);
  t.setFont(font);
  setAxisTitle(QwtPlot::yLeft, t);
}

QFont SaxsviewPlot::axisTitleFont() const {
  return axisTitle(QwtPlot::xBottom).font();
}

void SaxsviewPlot::setAxisTitleFontColor(const QColor& c) {
  QPalette pal = axisWidget(QwtPlot::xBottom)->palette();
  pal.setColor(QPalette::Text, c);

  axisWidget(QwtPlot::xBottom)->setPalette(pal);
  axisWidget(QwtPlot::yLeft)->setPalette(pal);

  replot();
}

QColor SaxsviewPlot::axisTitleFontColor() const {
  return axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text);
}

void SaxsviewPlot::setXTickLabelsVisible(bool on) {
  QwtScaleDraw *scale = axisScaleDraw(QwtPlot::xBottom);
  scale->enableComponent(QwtAbstractScaleDraw::Labels, on);
  updateLayout();
}

bool SaxsviewPlot::xTickLabelsVisible() const {
  const QwtScaleDraw *scale = axisScaleDraw(QwtPlot::xBottom);
  return scale->hasComponent(QwtAbstractScaleDraw::Labels);
}

void SaxsviewPlot::setYTickLabelsVisible(bool on) {
  QwtScaleDraw *scale = axisScaleDraw(QwtPlot::yLeft);
  scale->enableComponent(QwtAbstractScaleDraw::Labels, on);
  updateLayout();
}

bool SaxsviewPlot::yTickLabelsVisible() const {
  const QwtScaleDraw *scale = axisScaleDraw(QwtPlot::yLeft);
  return scale->hasComponent(QwtAbstractScaleDraw::Labels);
}

void SaxsviewPlot::setMinorTicksVisible(bool on) {
  //
  // There is a ScaleDraw component "Ticks", like the "Labels" used
  // for the visibility of the labels, but "Ticks" shows/hides
  // all ticks, not selected ones. Here we "disable" tick marks by
  // setting their size to 0 if disabled, and their default value
  // when enabled.
  //
  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i) {
    QwtScaleDraw *draw = p->scales[i]->scaleDraw();
    draw->setTickLength(QwtScaleDiv::MinorTick, on ? 4 : 0);
    draw->setTickLength(QwtScaleDiv::MediumTick, on ? 6 : 0);
  }

  replot();
}

bool SaxsviewPlot::minorTicksVisible() const {
  //
  // All axis are in sync, just pick one.
  //
  QwtScaleDraw *draw = p->scales[QwtPlot::xBottom]->scaleDraw();
  return draw->tickLength(QwtScaleDiv::MinorTick) > 0;
}

void SaxsviewPlot::setMajorTicksVisible(bool on) {
  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i) {
    QwtScaleDraw *draw = p->scales[i]->scaleDraw();
    draw->setTickLength(QwtScaleDiv::MajorTick, on ? 8 : 0);
  }

  replot();
}

bool SaxsviewPlot::majorTicksVisible() const {
  QwtScaleDraw *draw = p->scales[QwtPlot::xBottom]->scaleDraw();
  return draw->tickLength(QwtScaleDiv::MajorTick) > 0;
}

void SaxsviewPlot::setTickLabelFont(const QFont& font) {
  setAxisFont(QwtPlot::xBottom, font);
  setAxisFont(QwtPlot::yLeft, font);
}

QFont SaxsviewPlot::tickLabelFont() const {
  return axisFont(QwtPlot::xBottom);
}

void SaxsviewPlot::setTickLabelFontColor(const QColor& c) {
  SaxsviewScaleDraw *draw;
  draw = dynamic_cast<SaxsviewScaleDraw*>(axisScaleDraw(QwtPlot::xBottom));
  if (draw)
    draw->setLabelColor(c);

  draw = dynamic_cast<SaxsviewScaleDraw*>(axisScaleDraw(QwtPlot::yLeft));
  if (draw)
    draw->setLabelColor(c);

  replot();
}

QColor SaxsviewPlot::tickLabelFontColor() const {
  const SaxsviewScaleDraw *draw;
  draw = dynamic_cast<const SaxsviewScaleDraw*>(axisScaleDraw(QwtPlot::xBottom));
  return draw ? draw->labelColor() : QColor();
}

void SaxsviewPlot::setLegendVisible(bool on) {
  p->legend->setVisible(on);
  replot();
}

bool SaxsviewPlot::legendVisible() const {
  return p->legend->isVisible();
}

void SaxsviewPlot::setLegendPosition(Qt::Corner pos) {
  switch (pos) {
    case Qt::TopLeftCorner:
      p->legend->setAlignment(Qt::AlignTop | Qt::AlignLeft);
      break;

    case Qt::TopRightCorner:
      p->legend->setAlignment(Qt::AlignTop | Qt::AlignRight);
      break;

    case Qt::BottomLeftCorner:
      p->legend->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
      break;

    case Qt::BottomRightCorner:
      p->legend->setAlignment(Qt::AlignBottom | Qt::AlignRight);
      break;
  }

  replot();
}

Qt::Corner SaxsviewPlot::legendPosition() const {
  Qt::Alignment pos = p->legend->alignment();

  if (pos == (Qt::AlignTop | Qt::AlignLeft))
    return Qt::TopLeftCorner;

  else if (pos == (Qt::AlignTop | Qt::AlignRight))
    return Qt::TopRightCorner;

  else if (pos == (Qt::AlignBottom | Qt::AlignLeft))
    return Qt::BottomLeftCorner;

  else if (pos == (Qt::AlignBottom | Qt::AlignRight))
    return Qt::BottomRightCorner;

  else
    return Qt::TopRightCorner;
}

void SaxsviewPlot::setLegendColumnCount(int n) {
  p->legend->setMaxColumns(n);
  replot();
}

int SaxsviewPlot::legendColumnCount() const {
  return p->legend->maxColumns();
}

void SaxsviewPlot::setLegendSpacing(int n) {
  p->legend->setSpacing(n);
  replot();
}

int SaxsviewPlot::legendSpacing() const {
  return p->legend->spacing();
}

void SaxsviewPlot::setLegendMargin(int n) {
  p->legend->setMargin(n);
  replot();
}

int SaxsviewPlot::legendMargin() const {
  return p->legend->margin();
}

void SaxsviewPlot::setLegendFont(const QFont& font) {
  p->legend->setFont(font);
  replot();
}

QFont SaxsviewPlot::legendFont() const {
  return p->legend->font();
}

void SaxsviewPlot::setLegendFontColor(const QColor& c) {
  QPen pen = p->legend->textPen();
  pen.setColor(c);
  p->legend->setTextPen(pen);

  replot();
}

QColor SaxsviewPlot::legendFontColor() const {
  return p->legend->textPen().color();  
}

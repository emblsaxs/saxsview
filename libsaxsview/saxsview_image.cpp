/*
 * Copyright (C) 2011, 2012, 2013
 * Daniel Franke <dfranke@users.sourceforge.net>
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

// libsaxsview
#include "saxsview_image.h"
#include "saxsview_config.h"
#include "saxsview_colormap.h"
#include "saxsview_scaledraw.h"

// libsaxsimage
#include "saxsimage.h"
#include "saxsimage_format.h"

// external/qwt
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_rescaler.h"
#include "qwt_plot_scaleitem.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"
#include "qwt_text_label.h"

#include <QtGui>


class Log10ScaleEngine : public QwtLog10ScaleEngine {
public:
  //
  // Try to avoid that the color scale maps out the range
  // of 0 to negative infinity by specifying that the lower
  // bound shall have a minimum value of 1.0.
  //
  // NOTE: The code of QwtLog10ScaleEngine has 1e-100 as minimum.
  //
  QwtScaleDiv divideScale(double x1, double x2,
                          int numMajorSteps, int numMinorSteps,
                          double stepSize = 0.0) const {
    return QwtLog10ScaleEngine::divideScale(qMax(1.0, x1), x2,
                                            numMajorSteps, numMinorSteps,
                                            stepSize);
  }
};





class SaxsviewImage::Private {
public:
  Private();

  void setupCanvas(SaxsviewImage *image);
  void setupScales(SaxsviewImage *image);
  void setupPanner(SaxsviewImage *image);
  void setupZoomer(SaxsviewImage *image);

  void updateScaleAndColor(SaxsviewImage*,
                           Saxsview::Scale, Saxsview::ColorMap);

  SaxsviewFrame *frame;
  SaxsviewMask *mask;
  Saxsview::Scale scale;
  Saxsview::ColorMap colorMap;

  QwtPlotScaleItem *scales[QwtPlot::axisCnt];
  QwtPlotPanner *panner;
  QwtPlotZoomer *zoomer;
  QwtPlotRescaler *rescaler;
};

SaxsviewImage::Private::Private()
 : frame(0L), mask(0L),
   scale(Saxsview::AbsoluteScale), colorMap(Saxsview::GrayColorMap) {
}

void SaxsviewImage::Private::setupCanvas(SaxsviewImage *image) {
  // initial background color
  image->setAutoFillBackground(true);
  image->setPalette(Qt::white);
  image->canvas()->setFrameStyle(QFrame::NoFrame);
  image->canvas()->setLineWidth(1);

  // margin around the plot
  image->setContentsMargins(12, 12, 12, 12);
}

void SaxsviewImage::Private::setupScales(SaxsviewImage *image) {
  QwtPlotScaleItem *yRight = new QwtPlotScaleItem(QwtScaleDraw::LeftScale);
  yRight->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  yRight->attach(image);
  yRight->setBorderDistance(1);
  scales[QwtPlot::yRight] = yRight;

  QwtPlotScaleItem *yLeft = new QwtPlotScaleItem(QwtScaleDraw::RightScale);
  yLeft->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  yLeft->attach(image);
  yLeft->setBorderDistance(0);
  scales[QwtPlot::yLeft] = yLeft;

  QwtPlotScaleItem *xTop = new QwtPlotScaleItem(QwtScaleDraw::BottomScale);
  xTop->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  xTop->attach(image);
  xTop->setBorderDistance(0);
  scales[QwtPlot::xTop] = xTop;

  QwtPlotScaleItem *xBottom = new QwtPlotScaleItem(QwtScaleDraw::TopScale);
  xBottom->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  xBottom->attach(image);
  xBottom->setBorderDistance(1);
  scales[QwtPlot::xBottom] = xBottom;

  SaxsviewScaleDraw *scaleDraw;
  scaleDraw = new SaxsviewScaleDraw;
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, false);
  image->setAxisScaleDraw(QwtPlot::yLeft, scaleDraw);

  scaleDraw = new SaxsviewScaleDraw;
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, false);
  image->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);

  image->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, false);

  //
  // A color bar on the right axis.
  //
  QwtScaleWidget *rightAxis = image->axisWidget(QwtPlot::yRight);
  rightAxis->setTitle("Counts");
  rightAxis->setColorBarEnabled(true);

  image->enableAxis(QwtPlot::yRight);
  image->plotLayout()->setAlignCanvasToScales(true);

  //
  // Allow rescaling to a fixed aspect ratio - but exempt
  // the color bar on the right axis.
  //
  rescaler = new QwtPlotRescaler(image->canvas(),
                                 QwtPlot::xBottom,
                                 QwtPlotRescaler::Fixed);
  rescaler->setAspectRatio(QwtPlot::yRight, 0.0);
  rescaler->setEnabled(false);
}

void SaxsviewImage::Private::setupPanner(SaxsviewImage *image) {
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
  panner = new QwtPlotPanner(image->canvas());
  panner->setCursor(Qt::SizeAllCursor);
  panner->setEnabled(false);
}

void SaxsviewImage::Private::setupZoomer(SaxsviewImage *image) {
  zoomer = new QwtPlotZoomer(image->canvas());
  zoomer->setEnabled(true);

  // RightButton: zoom out by 1
  zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                          Qt::RightButton);

  // Ctrl+RightButton: zoom out to full size
  zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                          Qt::RightButton, Qt::ControlModifier);
}

void SaxsviewImage::Private::updateScaleAndColor(SaxsviewImage *image,
                                                 Saxsview::Scale s,
                                                 Saxsview::ColorMap m) {
  scale = s;
  colorMap = m;

  if (!frame)
    return;

  QwtInterval interval;
  if (frame->data())
    interval = frame->data()->interval(Qt::ZAxis);


  if (s == Saxsview::AbsoluteScale && m == Saxsview::HSVColorMap) {
      frame->setColorMap(new HSVColorMap());
      image->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
      image->axisWidget(QwtPlot::yRight)->setColorMap(interval,
                                                      new HSVColorMap());

  } else if (s == Saxsview::AbsoluteScale && m == Saxsview::GrayColorMap) {
      frame->setColorMap(new GrayColorMap());
      image->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
      image->axisWidget(QwtPlot::yRight)->setColorMap(interval,
                                                      new GrayColorMap());

  } else if (s == Saxsview::Log10Scale && m == Saxsview::HSVColorMap) {
      frame->setColorMap(new Log10HSVColorMap());
      image->setAxisScaleEngine(QwtPlot::yRight, new Log10ScaleEngine);
      image->axisWidget(QwtPlot::yRight)->setColorMap(interval,
                                                      new Log10HSVColorMap());

  } else if (s == Saxsview::Log10Scale && m == Saxsview::GrayColorMap) {
      frame->setColorMap(new Log10GrayColorMap());
      image->setAxisScaleEngine(QwtPlot::yRight, new Log10ScaleEngine);
      image->axisWidget(QwtPlot::yRight)->setColorMap(interval,
                                                      new Log10GrayColorMap());

  } else
    qFatal("internal error: unhandled combination of scale and color map");
}



SaxsviewImage::SaxsviewImage(QWidget *parent)
 : QwtPlot(parent), p(new Private) {

  p->setupPanner(this);
  p->setupZoomer(this);
  p->setupCanvas(this);
  p->setupScales(this);
}

SaxsviewImage::~SaxsviewImage() {
  delete p;
}

void SaxsviewImage::setFrame(SaxsviewFrame *frame) {
  if (p->frame)
    p->frame->detach();

  p->frame = frame;
  frame->attach(this);

  if (frame->data()) {
    const QwtInterval range = p->frame->data()->interval(Qt::ZAxis);
    axisWidget(QwtPlot::yRight)->setColorBarInterval(range);
    setAxisScale(QwtPlot::yRight, range.minValue(), range.maxValue());
  }

  setZoomBase(p->frame->boundingRect());

  replot();
}

SaxsviewFrame* SaxsviewImage::frame() const {
  return p->frame;
}

void SaxsviewImage::setMask(SaxsviewMask *mask) {
  if (p->mask)
    p->mask->detach();

  p->mask = mask;
  mask->attach(this);

  replot();
}

SaxsviewMask* SaxsviewImage::mask() const {
  return p->mask;
}

QRectF SaxsviewImage::zoomBase() const {
  return p->zoomer->zoomBase();
}

void SaxsviewImage::zoom(const QRectF& rect) {
  p->zoomer->zoom(rect);
}

bool SaxsviewImage::isZoomEnabled() const {
  return p->zoomer->isEnabled();
}

bool SaxsviewImage::isMoveEnabled() const {
  return p->panner->isEnabled();
}

void SaxsviewImage::exportAs() {
  QString fileName = QFileDialog::getSaveFileName(this, "Export As",
                                                  config().recentDirectory(),
                                                  "All files (*.*)");
  exportAs(fileName);
}

void SaxsviewImage::exportAs(const QString& fileName, const QString& format) {
  if (fileName.isEmpty())
    return;

  QString ext = format.isEmpty() ? QFileInfo(fileName).completeSuffix() : format;

  if (ext == "ps" || ext == "pdf" || ext == "svg") {
    QwtPlotRenderer renderer;
    renderer.renderDocument(this, fileName, ext, size()*25.4/85, 600);

  } else
    QPixmap::grabWidget(this).save(fileName, qPrintable(ext));
}

void SaxsviewImage::print() {
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


void SaxsviewImage::setZoomBase(const QRectF& rect) {
  QRectF r = rect;

  //
  // If no rect is specified, take the bounding rect of the frame.
  //
  if (!r.isValid())
    r = p->frame->boundingRect();

  if (r.isValid()) {
    //
    // This seems to be weird, but gives the best possible results.
    // E.g. if zoomBase() is not set before the initial zoom, an all-negative
    // curve will result in an initial zoom to an (0,0,0x0) rect.
    //
    p->zoomer->setZoomBase(r);
    p->zoomer->zoom(r);
    p->zoomer->setZoomBase(r);
  }

  replot();
}

void SaxsviewImage::setZoomEnabled(bool on) {
  p->zoomer->setEnabled(on);
}

void SaxsviewImage::setMoveEnabled(bool on) {
  p->panner->setEnabled(on);
}

Saxsview::Scale SaxsviewImage::scale() const {
  return p->scale;
}

void SaxsviewImage::setScale(Saxsview::Scale s) {
  p->updateScaleAndColor(this, s, p->colorMap);
  replot();
}

bool SaxsviewImage::isAspectRatioFixed() const {
  return p->rescaler->isEnabled();
}

void SaxsviewImage::setAspectRatioFixed(bool yes) {
  p->rescaler->setEnabled(yes);
  if (yes)
    p->rescaler->rescale();
  else
    setZoomBase(p->frame->boundingRect());
}

void SaxsviewImage::setBackgroundColor(const QColor& c) {
  QPalette pal = palette();
  pal.setColor(QPalette::Window, c);

  // The plot.
  setPalette(pal);

  // The scales.
  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i)
    p->scales[i]->setPalette(pal);

  replot();
}

QColor SaxsviewImage::backgroundColor() const {
  return palette().color(QPalette::Window);
}

void SaxsviewImage::setForegroundColor(const QColor& c) {
  QPalette pal = palette();
  pal.setColor(QPalette::WindowText, c);

  // The plot.
  setPalette(pal);

  // The scales.
  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i)
    p->scales[i]->setPalette(pal);

  replot();
}


QColor SaxsviewImage::foregroundColor() const {
  return palette().color(QPalette::WindowText);
}

void SaxsviewImage::setImageTitle(const QString& text) {
  QwtText t = title();
  t.setText(text);
  setTitle(t);
}

QString SaxsviewImage::imageTitle() const {
  return title().text();
}

void SaxsviewImage::setImageTitleFont(const QFont& font) {
  QwtText t = title();
  t.setFont(font);
  setTitle(t);
}

QFont SaxsviewImage::imageTitleFont() const {
  return title().font();
}

void SaxsviewImage::setImageTitleFontColor(const QColor& c) {
  QPalette pal = titleLabel()->palette();
  pal.setColor(QPalette::Text, c);
  titleLabel()->setPalette(pal);

  replot();
}

QColor SaxsviewImage::imageTitleFontColor() const {
  return titleLabel()->palette().color(QPalette::Text);
}

void SaxsviewImage::setAxisTitleX(const QString& text) {
  QwtText t = axisTitle(QwtPlot::xBottom);
  t.setText(text);
  setAxisTitle(QwtPlot::xBottom, t);
}

QString SaxsviewImage::axisTitleX() const {
  return axisTitle(QwtPlot::xBottom).text();
}

void SaxsviewImage::setAxisTitleY(const QString& text) {
  QwtText t = axisTitle(QwtPlot::yLeft);
  t.setText(text);
  setAxisTitle(QwtPlot::yLeft, t);
}

QString SaxsviewImage::axisTitleY() const {
  return axisTitle(QwtPlot::yLeft).text();
}

void SaxsviewImage::setAxisTitleZ(const QString& text) {
  QwtText t = axisTitle(QwtPlot::yRight);
  t.setText(text);
  setAxisTitle(QwtPlot::yRight, t);
}

QString SaxsviewImage::axisTitleZ() const {
  return axisTitle(QwtPlot::yRight).text();
}

void SaxsviewImage::setAxisTitleFont(const QFont& font) {
  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i) {
    QwtText t = axisTitle(i);
    t.setFont(font);
    setAxisTitle(i, t);
  }
}

QFont SaxsviewImage::axisTitleFont() const {
  return axisTitle(QwtPlot::xBottom).font();
}

void SaxsviewImage::setAxisTitleFontColor(const QColor& c) {
  QPalette pal = axisWidget(QwtPlot::xBottom)->palette();
  pal.setColor(QPalette::Text, c);

  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i)
    axisWidget(i)->setPalette(pal);

  replot();
}

QColor SaxsviewImage::axisTitleFontColor() const {
  return axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text);
}

void SaxsviewImage::setXTickLabelsVisible(bool on) {
  QwtScaleDraw *scale = axisScaleDraw(QwtPlot::xBottom);
  scale->enableComponent(QwtAbstractScaleDraw::Labels, on);
  updateLayout();
}

bool SaxsviewImage::xTickLabelsVisible() const {
  const QwtScaleDraw *scale = axisScaleDraw(QwtPlot::xBottom);
  return scale->hasComponent(QwtAbstractScaleDraw::Labels);
}

void SaxsviewImage::setYTickLabelsVisible(bool on) {
  QwtScaleDraw *scale = axisScaleDraw(QwtPlot::yLeft);
  scale->enableComponent(QwtAbstractScaleDraw::Labels, on);
  updateLayout();
}

bool SaxsviewImage::yTickLabelsVisible() const {
  const QwtScaleDraw *scale = axisScaleDraw(QwtPlot::yLeft);
  return scale->hasComponent(QwtAbstractScaleDraw::Labels);
}

void SaxsviewImage::setMinorTicksVisible(bool on) {
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

bool SaxsviewImage::minorTicksVisible() const {
  //
  // All axis are in sync, just pick one.
  //
  QwtScaleDraw *draw = p->scales[QwtPlot::xBottom]->scaleDraw();
  return draw->tickLength(QwtScaleDiv::MinorTick) > 0;
}

void SaxsviewImage::setMajorTicksVisible(bool on) {
  for (int i = QwtPlot::yLeft; i < QwtPlot::axisCnt; ++i) {
    QwtScaleDraw *draw = p->scales[i]->scaleDraw();
    draw->setTickLength(QwtScaleDiv::MajorTick, on ? 8 : 0);
  }

  replot();
}

bool SaxsviewImage::majorTicksVisible() const {
  QwtScaleDraw *draw = p->scales[QwtPlot::xBottom]->scaleDraw();
  return draw->tickLength(QwtScaleDiv::MajorTick) > 0;
}

void SaxsviewImage::setTickLabelFont(const QFont& font) {
  setAxisFont(QwtPlot::xBottom, font);
  setAxisFont(QwtPlot::yLeft, font);
}

QFont SaxsviewImage::tickLabelFont() const {
  return axisFont(QwtPlot::xBottom);
}

void SaxsviewImage::setTickLabelFontColor(const QColor& c) {
  SaxsviewScaleDraw *draw;
  draw = dynamic_cast<SaxsviewScaleDraw*>(axisScaleDraw(QwtPlot::xBottom));
  if (draw)
    draw->setLabelColor(c);

  draw = dynamic_cast<SaxsviewScaleDraw*>(axisScaleDraw(QwtPlot::yLeft));
  if (draw)
    draw->setLabelColor(c);

  replot();
}

QColor SaxsviewImage::tickLabelFontColor() const {
  const SaxsviewScaleDraw *draw;
  draw = dynamic_cast<const SaxsviewScaleDraw*>(axisScaleDraw(QwtPlot::xBottom));
  return draw ? draw->labelColor() : QColor();
}

void SaxsviewImage::setColorBarVisible(bool on) {
  enableAxis(QwtPlot::yRight, on);
}

bool SaxsviewImage::colorBarVisible() const {
  return axisEnabled(QwtPlot::yRight);
}

void SaxsviewImage::setColorMap(Saxsview::ColorMap colorMap) {
  p->updateScaleAndColor(this, p->scale, colorMap);
  replot();
}

Saxsview::ColorMap SaxsviewImage::colorMap() const {
  return p->colorMap;
}






class SaxsviewFrame::Private {
public:
};

SaxsviewFrame::SaxsviewFrame(QObject *parent)
 : QObject(parent), QwtPlotSpectrogram(), p(new Private) {

  setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
}

SaxsviewFrame::~SaxsviewFrame() {
  delete p;
}

QSize SaxsviewFrame::size() const {
  if (data())
    return QSize(data()->interval(Qt::XAxis).width(),
                 data()->interval(Qt::YAxis).width());
  else
    return QSize();
}

void SaxsviewFrame::setMinValue(double x) {
  if (SaxsviewFrameData *d = dynamic_cast<SaxsviewFrameData*>(data())) {
    d->setMinValue(x);

    QwtScaleWidget *scale = plot()->axisWidget(QwtPlot::yRight);
    scale->setColorBarInterval(data()->interval(Qt::ZAxis));

    plot()->replot();
  }
}

double SaxsviewFrame::minValue() const {
  return data() ? data()->interval(Qt::ZAxis).minValue() : 0.0;
}

void SaxsviewFrame::setMaxValue(double x) {
  if (SaxsviewFrameData *d = dynamic_cast<SaxsviewFrameData*>(data())) {
    d->setMaxValue(x);

    QwtScaleWidget *scale = plot()->axisWidget(QwtPlot::yRight);
    scale->setColorBarInterval(data()->interval(Qt::ZAxis));

    plot()->replot();
  }
}

double SaxsviewFrame::maxValue() const {
  return data() ? data()->interval(Qt::ZAxis).maxValue() : 0.0;
}


class SaxsviewMask::Private {
public:
  Private();

  void setValue(SaxsviewMask *mask, const QPointF&, double value);
  void setValue(SaxsviewMask *mask, const QPolygonF&, double value);

  QColor color;
  bool modified;
};

SaxsviewMask::Private::Private()
 : modified(false) {
}

void SaxsviewMask::Private::setValue(SaxsviewMask *mask, const QPointF& p, double value) {
  if (SaxsviewFrameData *d = (SaxsviewFrameData*)(mask->data())) {
    // QPointF::toPoint() rounds to nearest integer, that's not what we want here.
    const QPoint point((int)p.x(), (int)p.y());

    d->setValue(point.x(), point.y(), value);

    modified = true;
    mask->plot()->replot();
  }
}

void SaxsviewMask::Private::setValue(SaxsviewMask *mask, const QPolygonF& p, double value) {
  if (SaxsviewFrameData *d = (SaxsviewFrameData*)(mask->data())) {
    // QPolygonF::toPolygon() internally uses toPoint() rounding, see above.
    QPolygon polygon;
    foreach (QPointF pt, p)
      polygon << QPoint((int)pt.x(), (int)pt.y());

    const QRect r = polygon.boundingRect();

    for (int x = r.x(); x <= r.x() + r.width(); ++x)
      for (int y = r.y(); y <= r.y() + r.height(); ++y)
        if (polygon.containsPoint(QPoint(x, y), Qt::OddEvenFill))
          d->setValue(x, y, value);

    modified = true;
    mask->plot()->replot();
  }
}



SaxsviewMask::SaxsviewMask(QObject *parent)
 : QObject(parent), QwtPlotSpectrogram(), p(new Private) {

  // The mask shall be "above" the image.
  setZ(10);

  // TODO: make initial mask color configurable (save the last mask color?)
  setColor(QColor(255, 0, 255, 128));

  setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
}

SaxsviewMask::~SaxsviewMask() {
}

bool SaxsviewMask::save(const QString& fileName) const {
  if (SaxsviewFrameData *d = (SaxsviewFrameData*)data())
    return d->save(fileName);
  else
    return false;
}

void SaxsviewMask::add(const QPointF& pt) {
  p->setValue(this, pt, 1.0);
}

void SaxsviewMask::remove(const QPointF& pt) {
  p->setValue(this, pt, 0.0);
}

void SaxsviewMask::add(const QPolygonF& pt) {
  p->setValue(this, pt, 1.0);
}

void SaxsviewMask::remove(const QPolygonF& pt) {
  p->setValue(this, pt, 0.0);
}

bool SaxsviewMask::isModified() const {
  return p->modified;
}

QSize SaxsviewMask::size() const {
  if (data())
    return QSize(data()->interval(Qt::XAxis).width(),
                 data()->interval(Qt::YAxis).width());
  else
    return QSize();
}

void SaxsviewMask::setColor(const QColor& c) {
  if (c != p->color) {
    p->color = c;

    // Color map: fully transparent (mask == 0) to a more opaque (mask == 1).
    setColorMap(new MaskColorMap(c));

    if (plot())
      plot()->replot();
  }
}

QColor SaxsviewMask::color() const {
  return p->color;
}

void SaxsviewMask::setVisible(bool visible) {
  // Is there a better way to achieve this?
  QwtPlotSpectrogram::setVisible(visible);
  if (plot())
    plot()->replot();
}



class SaxsviewFrameData::Private {
public:
  saxs_image *data;
  QwtInterval range, selectedRange;
};

SaxsviewFrameData::SaxsviewFrameData(const QString& fileName)
 : QwtRasterData(), p(new Private) {

  p->data = saxs_image_create();
  if (saxs_image_read(p->data, qPrintable(fileName), 0L) == 0) {

    setInterval(Qt::XAxis, QwtInterval(0.0, saxs_image_width(p->data) - 1.0));
    setInterval(Qt::YAxis, QwtInterval(0.0, saxs_image_height(p->data) - 1.0));
    setInterval(Qt::ZAxis, QwtInterval(saxs_image_value_min(p->data),
                                       saxs_image_value_max(p->data)));

  } else {
    saxs_image_free(p->data);
    p->data = 0L;
  }
}

SaxsviewFrameData::SaxsviewFrameData(const SaxsviewFrameData& other)
  : QwtRasterData(), p(new Private) {
  p->data = other.p->data;
}

SaxsviewFrameData::SaxsviewFrameData(const QSize& size)
  : QwtRasterData(), p(new Private) {

  p->data = saxs_image_create();
  saxs_image_set_size(p->data, size.width(), size.height());

  setInterval(Qt::XAxis, QwtInterval(0.0, size.width()));
  setInterval(Qt::YAxis, QwtInterval(0.0, size.height()));
  setInterval(Qt::ZAxis, QwtInterval(0.0, 1.0));
}

SaxsviewFrameData::~SaxsviewFrameData() {
  saxs_image_free(p->data);
  delete p;
}

SaxsviewFrameData* SaxsviewFrameData::copy() const {
  qFatal("SaxsviewFrameData::copy() not implemented as it was assumed to be unused.\n"
         "Please let me know about your use case so I can fix this. Thanks.");
  return 0L;
}

void SaxsviewFrameData::setMinValue(double x) {
  QwtInterval zrange = interval(Qt::ZAxis);
  setInterval(Qt::ZAxis, QwtInterval(qMax(x, saxs_image_value_min(p->data)),
                                     zrange.maxValue()));
}

void SaxsviewFrameData::setMaxValue(double x) {
  QwtInterval zrange = interval(Qt::ZAxis);
  setInterval(Qt::ZAxis, QwtInterval(zrange.minValue(),
                                     qMin(x, saxs_image_value_max(p->data))));
}

double SaxsviewFrameData::value(double x, double y) const {
  return saxs_image_value(p->data, (int)x, (int)y);
}

void SaxsviewFrameData::setValue(double x, double y, double value) {
  saxs_image_set_value(p->data, (int)x, (int)y, value);
}

bool SaxsviewFrameData::save(const QString& fileName) const {
  if (p->data)
    return saxs_image_write(p->data, qPrintable(fileName), 0L) == 0;
  else
    return false;
}

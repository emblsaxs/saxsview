/*
 * Copyright (C) 2011 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "saxsview_image.h"

// libsaxsimage
#include "saxsimage.h"
#include "saxsimage_format.h"

// external/qwt
#include "qwt_color_map.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_rescaler.h"
#include "qwt_plot_scaleitem.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

#include <QtGui>

class Log10ColorMap : public QwtLinearColorMap {
public:
  Log10ColorMap(const QColor &from, const QColor &to,
        QwtColorMap::Format format = QwtColorMap::RGB)
   : QwtLinearColorMap(from, to, format) {
  }

  Log10ColorMap(const Log10ColorMap& other) : QwtLinearColorMap(other) {
  }

  Log10ColorMap* copy() const {
    return new Log10ColorMap(*this);
  }

  QRgb rgb(const QwtDoubleInterval& interval, double x) const {
    //
    // Due to selectable thresholds it may happen that 'x' is outside the
    // range. If this is the case, we automatically get color1() if 'x'
    // is below minValue() and color2() if 'x' is above maxValue().
    //
    // I.e. if a lower threshold is defined, all pixels below that value
    // will be from-color, all those above an upper threshold will be to-color.
    //
    double min = interval.minValue() > 1.0 ? log10(interval.minValue()) : 0.0;
    double max = interval.maxValue() > 1.0 ? log10(interval.maxValue()) : 0.0;
    double val = x > 1.0 ? log10(x) : 0.0;

    return QwtLinearColorMap::rgb(QwtDoubleInterval(min, max), val);
  }

  unsigned char colorIndex(const QwtDoubleInterval& interval, double x) const {
    double min = interval.minValue() > 1.0 ? log10(interval.minValue()) : 0.0;
    double max = interval.maxValue() > 1.0 ? log10(interval.maxValue()) : 0.0;
    double val = x > 1.0 ? log10(x) : 0.0;
    return QwtLinearColorMap::colorIndex(QwtDoubleInterval(min, max), val);
  }
};


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
  Private() : frame(0L) {}

  void setupCanvas(SaxsviewImage *image);
  void setupScales(SaxsviewImage *image);
  void setupPanner(SaxsviewImage *image);
  void setupZoomer(SaxsviewImage *image);

  SaxsviewFrame *frame;
  Saxsview::Scale scale;

  QwtPlotPanner *panner;
  QwtPlotZoomer *zoomer;
  QwtPlotRescaler *rescaler;
};

void SaxsviewImage::Private::setupCanvas(SaxsviewImage *image) {
  // initial background color
  image->setAutoFillBackground(true);
  image->setPalette(Qt::white);
  image->canvas()->setFrameStyle(QFrame::NoFrame);
  image->canvas()->setLineWidth(1);

  // margin around the plot
  image->plotLayout()->setMargin(12);
  image->plotLayout()->setAlignCanvasToScales(true);
}

void SaxsviewImage::Private::setupScales(SaxsviewImage *image) {
  QwtPlotScaleItem *yRight = new QwtPlotScaleItem(QwtScaleDraw::LeftScale);
  yRight->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  yRight->attach(image);
  yRight->setBorderDistance(1);

  QwtPlotScaleItem *yLeft = new QwtPlotScaleItem(QwtScaleDraw::RightScale);
  yLeft->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  yLeft->attach(image);
  yLeft->setBorderDistance(0);

  QwtPlotScaleItem *xTop = new QwtPlotScaleItem(QwtScaleDraw::BottomScale);
  xTop->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  xTop->attach(image);
  xTop->setBorderDistance(0);

  QwtPlotScaleItem *xBottom = new QwtPlotScaleItem(QwtScaleDraw::TopScale);
  xBottom->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
  xBottom->attach(image);
  xBottom->setBorderDistance(1);

  QwtScaleDraw *scaleDraw = image->axisScaleDraw(QwtPlot::yLeft);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, false);

  scaleDraw = image->axisScaleDraw(QwtPlot::xBottom);
  scaleDraw->enableComponent(QwtAbstractScaleDraw:: Backbone, false);
  scaleDraw->enableComponent(QwtAbstractScaleDraw:: Ticks, false);

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

  //
  // Invert the left-hand axis to bring origin of images
  // to the upper-left corner.
  //
  // NOTE: This should be in setupScales() instead, but detach/attach
  //       of a frame makes QwtPlot forget that it should keep the
  //       left axis inverted. Thus we restate the fact here.
  //
  axisScaleDiv(QwtPlot::yLeft)->invert();

  const QwtDoubleInterval range = p->frame->data()->range();
  axisWidget(QwtPlot::yRight)->setColorMap(range, p->frame->colorMap());

  setAxisScale(QwtPlot::yRight, range.minValue(), range.maxValue());

  setZoomBase(p->frame->boundingRect());

  replot();
}

SaxsviewFrame* SaxsviewImage::frame() const {
  return p->frame;
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

bool SaxsviewImage::isAspectRatioFixed() const {
  return p->rescaler->isEnabled();
}

Saxsview::Scale SaxsviewImage::scale() const {
  return p->scale;
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

void SaxsviewImage::setAspectRatioFixed(bool yes) {
  p->rescaler->setEnabled(yes);
  if (yes)
    p->rescaler->rescale();
  else
    setZoomBase(p->frame->boundingRect());
}

void SaxsviewImage::setScale(Saxsview::Scale s) {
  p->scale = s;

  if (!p->frame)
    return;

  switch (s) {
    case Saxsview::AbsoluteScale:
      p->frame->setColorMap(QwtLinearColorMap(Qt::white, Qt::black));
      setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
      break;

    case Saxsview::Log10Scale:
      p->frame->setColorMap(Log10ColorMap(Qt::white, Qt::black));
      setAxisScaleEngine(QwtPlot::yRight, new Log10ScaleEngine);
      break;
  }

  if (p->frame->boundingRect().isValid())
    axisWidget(QwtPlot::yRight)->setColorMap(p->frame->data()->range(),
                                             p->frame->colorMap());

  replot();
}




class SaxsviewFrame::Private {
public:
};

SaxsviewFrame::SaxsviewFrame(QObject *parent)
 : QwtPlotSpectrogram(), p(new Private) {

  setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
}

SaxsviewFrame::~SaxsviewFrame() {
  delete p;
}

QSize SaxsviewFrame::size() const {
  const QRectF rect = data()->boundingRect();
  return QSize((int)rect.width(), (int)rect.height());
}

double SaxsviewFrame::minValue() const {
  return data()->range().minValue();
}

double SaxsviewFrame::maxValue() const {
  return data()->range().maxValue();
}

void SaxsviewFrame::setMinValue(double x) {
  if (SaxsviewFrameData *d = dynamic_cast<SaxsviewFrameData*>(data())) {
    d->setMinValue(x);

    QwtScaleWidget *scale = plot()->axisWidget(QwtPlot::yRight);
    scale->setColorMap(data()->range(), colorMap());

    plot()->replot();
  }
}

void SaxsviewFrame::setMaxValue(double x) {
  if (SaxsviewFrameData *d = dynamic_cast<SaxsviewFrameData*>(data())) {
    d->setMaxValue(x);

    QwtScaleWidget *scale = plot()->axisWidget(QwtPlot::yRight);
    scale->setColorMap(data()->range(), colorMap());

    plot()->replot();
  }
}



class SaxsviewFrameData::Private {
public:
  saxs_image* data;
  QwtDoubleInterval range, selectedRange;
};

SaxsviewFrameData::SaxsviewFrameData(const QString& fileName)
 : QwtRasterData(), p(new Private) {

  p->data = saxs_image_create();
  if (saxs_image_read(p->data, qPrintable(fileName), 0L) == 0) {

    setBoundingRect(QRectF(0.0, 0.0,
                           saxs_image_width(p->data) - 1.0,
                           saxs_image_height(p->data) - 1.0));

    p->range = QwtDoubleInterval(saxs_image_value_min(p->data),
                                 saxs_image_value_max(p->data));
    p->selectedRange = p->range;

  } else {
    saxs_image_free(p->data);
    p->data = 0L;
  }
}

SaxsviewFrameData::SaxsviewFrameData(const SaxsviewFrameData& other)
  : p(new Private) {
  p->data = other.p->data;
  p->range = other.p->range;
  p->selectedRange = other.p->selectedRange;
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

QwtDoubleInterval SaxsviewFrameData::range() const {
  return p->selectedRange;
}

void SaxsviewFrameData::setMinValue(double x) {
  p->selectedRange.setMinValue(qMax(x, p->range.minValue()));
}

void SaxsviewFrameData::setMaxValue(double x) {
  p->selectedRange.setMaxValue(qMin(x, p->range.maxValue()));
}

double SaxsviewFrameData::value(double x, double y) const {
  const double z = saxs_image_value(p->data, (int)x, (int)y);
  return z >= 0.0 ? z : 0.0;
}

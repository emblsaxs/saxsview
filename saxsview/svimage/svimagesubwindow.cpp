/*
 * Copyright (C) 2011 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "svimagesubwindow.h"
#include "svimagemainwindow.h"
#include "saxsview_plot.h"
#include "saxsview_image.h"

#include "saxsimage.h"
#include "saxsimage_format.h"

#include <QtGui>

#include "qwt_color_map.h"
#include "qwt_picker_machine.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_rescaler.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_raster_data.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

class ImageData : public QwtRasterData {
public:
  //
  // Share the image pointer between instances.
  // Only free if the last instance goes out of scope.
  //
  // Qt-4.5 also provides QSharedDataPointer which would
  // simplify things a bit, but
  //  (a) it's not available in Qt-4.4
  //  (b) in Qt-4.5 it's not possible to store an opaque
  //      pointer as a QSharedPointer<T>. As a workaround,
  //      the opaque pointer may be held in a separate class,
  //      which then could be used with a QSharedPointer<T>.
  //      This was fixed for Qt-4.6. See also:
  //      http://lists.trolltech.com/pipermail/qt-interest/2009-August/011754.html
  //
  // @sa copy
  //
  class ImagePointerHolder : public QSharedData {
  public:
    ImagePointerHolder(saxs_image *img)
     : image(img) {
    }

    ImagePointerHolder(const ImagePointerHolder& other)
     : QSharedData(other), image(other.image) {
    }

    ~ImagePointerHolder() { saxs_image_free(image);}

    saxs_image *image;
  };

  ImageData(saxs_image *image)
   : QwtRasterData(QRectF(0.0, 0.0,
                          saxs_image_width(image) - 1,
                          saxs_image_height(image) - 1)),
     p(new ImagePointerHolder(image)) {
     mRange = QwtDoubleInterval(saxs_image_value_min(image),
                                saxs_image_value_max(image));
     mSelectedRange = mRange;
  }

  ImageData(const ImageData& other)
   : p(other.p), mRange(other.mRange), mSelectedRange(other.mSelectedRange) {
  }

  ~ImageData() {
  }

  QwtRasterData *copy() const {
    return new ImageData(*this);
  }

  QwtDoubleInterval range() const {
    return mSelectedRange;
  }

  void setMin(double x) {
    mSelectedRange.setMinValue(qMax(x, mRange.minValue()));
  }

  void setMax(double x) {
    mSelectedRange.setMaxValue(qMin(x, mRange.maxValue()));
  }

  double value(double x, double y) const {
    return saxs_image_value(p.data()->image, (int)x, (int)y);
  }

private:
  QSharedDataPointer<ImagePointerHolder> p;
  QwtDoubleInterval mRange, mSelectedRange;
};


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
    // will be white, all those above an upper threshold will be black.
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


class ImagePicker : public QwtPlotPicker {
public:
  ImagePicker(Saxsview::Image *img, QwtPlotCanvas *canvas)
   : QwtPlotPicker(canvas), image(img) {
  }

  QwtText trackerTextF(const QPointF &pos) const {
    if (QWidget *w = qApp->activeWindow()) {
      static const QString format = "x=%1, y=%2, count=%3";
      const float x = pos.x(), y = pos.y();

      QStatusTipEvent event(format.arg((int)x, 4).arg((int)y, 4).arg(image->data()->value(x, y)));
      qApp->sendEvent(w, &event);
    }

    return QwtText();
  }

private:
  Saxsview::Image *image;
};




class SVImageSubWindow::SVImageSubWindowPrivate {
public:
  SVImageSubWindowPrivate(SVImageSubWindow *w) 
   : sw(w), plot(0L), scale(Saxsview::Plot::AbsoluteScale),
     image(0L), tracker(0L), rescaler(0L) {
  }

  void setupUi();
  void setupActions();
  void setupSignalMappers();
  void setupToolBar();
  void setupTracker();

  void setupImage();
  void setScale(Saxsview::Plot::PlotScale);

  SVImageSubWindow *sw;
  Saxsview::Plot *plot;
  Saxsview::Plot::PlotScale scale;
  Saxsview::Image *image;

  QString fileName;

  QwtPicker *tracker;
  QwtPlotRescaler *rescaler;
};

void SVImageSubWindow::SVImageSubWindowPrivate::setupUi() {
  plot = new Saxsview::Plot(sw);
  sw->setWidget(plot);
}

void SVImageSubWindow::SVImageSubWindowPrivate::setupActions() {
}

void SVImageSubWindow::SVImageSubWindowPrivate::setupToolBar() {
}

void SVImageSubWindow::SVImageSubWindowPrivate::setupTracker() {
  tracker = new ImagePicker(image, plot->canvas());
  tracker->setStateMachine(new QwtPickerTrackerMachine);
  tracker->setTrackerMode(QwtPicker::AlwaysOn);
}

void SVImageSubWindow::SVImageSubWindowPrivate::setupSignalMappers() {
}

void SVImageSubWindow::SVImageSubWindowPrivate::setupImage() {
  image = new Saxsview::Image(sw);
  image->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);

  //
  // Invert the left-hand axis to bring origin of images
  // to the upper-left corner.
  //
  plot->axisScaleDiv(QwtPlot::yLeft)->invert();

  //
  // A color bar on the right axis.
  //
  QwtScaleWidget *rightAxis = plot->axisWidget(QwtPlot::yRight);
  rightAxis->setTitle("Counts");
  rightAxis->setColorBarEnabled(true);

  plot->enableAxis(QwtPlot::yRight);
  plot->plotLayout()->setAlignCanvasToScales(true);

  //
  // Allow rescaling to a fixed aspect ratio - but exempt
  // the color bar on the right axis.
  //
  rescaler = new QwtPlotRescaler(plot->canvas(),
                                 QwtPlot::xBottom,
                                 QwtPlotRescaler::Fixed);
  rescaler->setAspectRatio(QwtPlot::yRight, 0.0);
  rescaler->setEnabled(false);
}

void SVImageSubWindow::SVImageSubWindowPrivate::setScale(Saxsview::Plot::PlotScale s) {
  scale = s;
  switch (s) {
    case Saxsview::Plot::AbsoluteScale:
      image->setColorMap(QwtLinearColorMap(Qt::white, Qt::black));
      plot->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
      break;

    case Saxsview::Plot::Log10Scale:
      image->setColorMap(Log10ColorMap(Qt::white, Qt::black));
      plot->setAxisScaleEngine(QwtPlot::yRight, new QwtLog10ScaleEngine);
      break;
  }

  if (image->data()->boundingRect().isValid())
    plot->axisWidget(QwtPlot::yRight)->setColorMap(image->data()->range(),
                                                   image->colorMap());
  plot->replot();
}

SVImageSubWindow::SVImageSubWindow(QWidget *parent)
 : QMdiSubWindow(parent), p(new SVImageSubWindowPrivate(this)) {

  setAttribute(Qt::WA_DeleteOnClose);

  p->setupUi();
  p->setupActions();
  p->setupSignalMappers();
  p->setupImage();
  p->setupTracker();
  p->setupToolBar();

  p->setScale(Saxsview::Plot::AbsoluteScale);
}

SVImageSubWindow::~SVImageSubWindow() {
  delete p;
}

QString& SVImageSubWindow::fileName() const {
  return p->fileName;
}

int SVImageSubWindow::scale() const {
  return p->scale;
}

bool SVImageSubWindow::isAspectRatioFixed() const {
  return p->rescaler->isEnabled();
}

bool SVImageSubWindow::zoomEnabled() const {
  return p->plot->zoomEnabled();
}

bool SVImageSubWindow::moveEnabled() const {
  return p->plot->moveEnabled();
}

double SVImageSubWindow::lowerThreshold() const {
  ImageData *imageData = dynamic_cast<ImageData*>(p->image->data());
  return imageData->range().minValue();
}

double SVImageSubWindow::upperThreshold() const {
  ImageData *imageData = dynamic_cast<ImageData*>(p->image->data());
  return imageData->range().maxValue();
}


bool SVImageSubWindow::load(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  if (!fileInfo.exists())
    return false;

  setCursor(Qt::WaitCursor);

  saxs_image *image = saxs_image_create();
  if (saxs_image_read(image, qPrintable(fileName), 0L) != 0) {
    QMessageBox::critical(this,
                          "Filetype not recognized",
                          QString("Could not load file as image:\n"
                                  "'%1'.").arg(fileName));
    saxs_image_free(image);

    unsetCursor();
    return false;
  }

  setWindowTitle(fileName);
  p->fileName = fileName;

  p->image->detach();
  p->image->setData(new ImageData(image));
  p->image->attach(p->plot);

  const QwtDoubleInterval range = p->image->data()->range();
  p->plot->axisWidget(QwtPlot::yRight)->setColorMap(range,
                                                    p->image->colorMap());

  p->plot->setAxisScale(QwtPlot::yRight,
                        range.minValue(),
                        range.maxValue());

  p->plot->setZoomBase(p->image->boundingRect());
  p->plot->replot();

  unsetCursor();
  return true;
}

void SVImageSubWindow::reload() {
  load(p->fileName);
}

void SVImageSubWindow::exportAs(const QString& fileName,
                                   const QString& format) {
  p->plot->exportAs(fileName, format);
}

void SVImageSubWindow::print() {
  p->plot->print();
}

void SVImageSubWindow::zoomFit() {
  p->plot->setZoomBase(p->image->boundingRect());
}

void SVImageSubWindow::setZoomEnabled(bool on) {
  p->plot->setZoomEnabled(on);
}

void SVImageSubWindow::setMoveEnabled(bool on) {
  p->plot->setMoveEnabled(on);
}

void SVImageSubWindow::setAspectRatioFixed(bool yes) {
  p->rescaler->setEnabled(yes);
  if (yes)
    p->rescaler->rescale();
  else
    zoomFit();
}

void SVImageSubWindow::setScale(int scale) {
  p->setScale((Saxsview::Plot::PlotScale)scale);
}

void SVImageSubWindow::setLowerThreshold(double threshold) {
  ImageData *imageData = dynamic_cast<ImageData*>(p->image->data());
  imageData->setMin(threshold);

  QwtScaleWidget *scale = p->plot->axisWidget(QwtPlot::yRight);
  scale->setColorMap(imageData->range(), p->image->colorMap());

  p->plot->replot();
}

void SVImageSubWindow::setUpperThreshold(double threshold) {
  ImageData *imageData = dynamic_cast<ImageData*>(p->image->data());
  imageData->setMax(threshold);

  QwtScaleWidget *scale = p->plot->axisWidget(QwtPlot::yRight);
  scale->setColorMap(imageData->range(), p->image->colorMap());

  p->plot->replot();
}

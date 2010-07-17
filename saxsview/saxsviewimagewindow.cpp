/*
 * Implementation of 2D-images subwindows.
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "saxsviewimagewindow.h"
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
#include "qwt_plot_spectrogram.h"
#include "qwt_raster_data.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

//
// In Qt-4.5.x, maybe earlier ones as well, it's not possible to store
// an opaque pointer as a QSharedPointer<T>. As a workaround, the opaque
// pointer is held in a separate class, which then can be used with a
// QSharedPointer<T>. See also:
//
// http://lists.trolltech.com/pipermail/qt-interest/2009-August/011754.html
//
class ImageData : public QwtRasterData {
public:
  class ImagePointerHolder {
  public:
    ImagePointerHolder(saxs_image *img) : image(img) { }
    ~ImagePointerHolder() { saxs_image_free(image);}
    saxs_image *image;
  };

  ImageData(saxs_image *image)
   : QwtRasterData(QRectF(0.0, 0.0,
                          saxs_image_width(image) - 1,
                          saxs_image_height(image) - 1)),
     p(QSharedPointer<ImagePointerHolder>(new ImagePointerHolder(image))),
     mMin(qMax((int)saxs_image_value_min(image), 1)),
     mMax(saxs_image_value_max(image)) {
  }

  ImageData(const ImageData& other)
   : p(other.p), mMin(other.mMin), mMax(other.mMax) {
  }

  ~ImageData() {
  }

  QwtRasterData *copy() const {
    return new ImageData(*this);
  }

  QwtDoubleInterval range() const {
    return QwtDoubleInterval(mMin, mMax);
  }

  void setMin(int n) {
    mMin = qMax(n, 1);
  }

  double value(double x, double y) const {
    return saxs_image_value(p.data()->image, (int)floor(x), (int)floor(y));
  }

private:
  QSharedPointer<ImagePointerHolder> p;
  size_t mMin, mMax;
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
    return QwtLinearColorMap::rgb(QwtDoubleInterval(log10(interval.minValue()),
                                                    log10(interval.maxValue())),
                                  log10(x));
  }

  unsigned char colorIndex(const QwtDoubleInterval& interval, double x) const {
    return QwtLinearColorMap::colorIndex(QwtDoubleInterval(log10(interval.minValue()),
                                                           log10(interval.maxValue())),
                                         log10(x));
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




class SaxsviewImageWindow::SaxsviewImageWindowPrivate {
public:
  SaxsviewImageWindowPrivate(SaxsviewImageWindow *w) : sw(w) {}

  void setupUi();
  void setupActions();
  void updateActions(const QString& fileName);
  void setupSignalMappers();
  void setupToolBar();
  void setupTracker();

  void setupImage();
  void setScale(Saxsview::Plot::PlotScale);

  SaxsviewImageWindow *sw;
  Saxsview::Plot *plot;
  Saxsview::Plot::PlotScale scale;
  Saxsview::Image *image;

  QAction *actionPrevious, *actionNext, *actionThreshold;
  QSpinBox *spinThreshold;
  QToolBar *toolBar;
  QSignalMapper *fileNameMapper;

  QwtPicker *tracker;
};

void SaxsviewImageWindow::SaxsviewImageWindowPrivate::setupUi() {
  sw->setAttribute(Qt::WA_DeleteOnClose);

  plot = new Saxsview::Plot(sw);
  sw->setWidget(plot);
}

void SaxsviewImageWindow::SaxsviewImageWindowPrivate::setupActions() {
  QStyle *style = qApp->style();
  QAction *action;

  // FIXME: the menus leak memory
  actionPrevious = new QAction("&Previous", sw);
  actionPrevious->setIcon(style->standardIcon(QStyle::SP_ArrowBack));
  actionPrevious->setMenu(new QMenu);
  actionPrevious->setEnabled(false);

  actionNext = new QAction("&Next", sw);
  actionNext->setIcon(style->standardIcon(QStyle::SP_ArrowForward));
  actionNext->setMenu(new QMenu);
  actionNext->setEnabled(false);
}

void SaxsviewImageWindow::SaxsviewImageWindowPrivate::setupToolBar() {
  spinThreshold = new QSpinBox(sw);
  spinThreshold->setRange(1, 1e12);
  spinThreshold->setAccelerated(true);
  spinThreshold->setSingleStep(1);
  connect(spinThreshold, SIGNAL(valueChanged(int)),
          sw, SLOT(setThreshold(int)));

  toolBar = new QToolBar(sw);

  toolBar->addAction(actionPrevious);
  toolBar->addAction(actionNext);
  actionThreshold = toolBar->addWidget(spinThreshold);
}


void SaxsviewImageWindow::SaxsviewImageWindowPrivate::setupTracker() {
  tracker = new ImagePicker(image, plot->canvas());
  tracker->setStateMachine(new QwtPickerTrackerMachine);
  tracker->setTrackerMode(QwtPicker::AlwaysOn);
}

void SaxsviewImageWindow::SaxsviewImageWindowPrivate::setupSignalMappers() {
  //
  // Map previous/next actions to load-slot.
  //
  fileNameMapper = new QSignalMapper(sw);

  connect(fileNameMapper, SIGNAL(mapped(const QString&)),
          sw, SLOT(load(const QString&)));

  connect(actionPrevious, SIGNAL(triggered()),
          fileNameMapper, SLOT(map()));
  fileNameMapper->setMapping(actionPrevious, "");

  connect(actionNext, SIGNAL(triggered()),
          fileNameMapper, SLOT(map()));
  fileNameMapper->setMapping(actionNext, "");
}

void SaxsviewImageWindow::SaxsviewImageWindowPrivate::updateActions(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  QDir fileDir = fileInfo.dir();

  const QStringList entries = fileDir.entryList(QDir::NoFilter, QDir::Name);
  const int currentIndex = entries.indexOf(fileInfo.fileName());

  //
  // Walk the directory entries. For each file that may be shown,
  // generate a menu entry for the "Previous" action; but 10 at most.
  // The first entry in the menu gets the shortcut assigned (to make
  // it show up in the list, otherwise the shortcut could also be
  // assigned to the actual action).
  //
  actionPrevious->menu()->clear();
  actionPrevious->setToolTip("No previous file.");
  for (int i = currentIndex - 1, j = 0; i >= 0 && j < 10; --i) {
    fileInfo.setFile(fileDir, entries[i]);
    if (canShow(fileInfo.filePath())) {
      QAction *action = actionPrevious->menu()->addAction(fileInfo.fileName());
      connect(action, SIGNAL(triggered()),
              fileNameMapper, SLOT(map()));
      fileNameMapper->setMapping(action, fileInfo.filePath());

      if (j == 0) {
        action->setShortcut(QKeySequence::Back);
        actionPrevious->setToolTip(QString("Previous: %1").arg(fileInfo.fileName()));
        fileNameMapper->setMapping(actionPrevious, fileInfo.filePath());
      }

      ++j;
    }
  }
  actionPrevious->setEnabled(!actionPrevious->menu()->isEmpty());

  //
  // Likewise for "Next".
  //
  actionNext->menu()->clear();
  actionNext->setToolTip("No next file.");
  for (int i = currentIndex + 1, j = 0; i < entries.size() && j < 10; ++i) {
    fileInfo.setFile(fileDir, entries[i]);
    if (canShow(fileInfo.filePath())) {
      QAction *action = actionNext->menu()->addAction(fileInfo.fileName());
      connect(action, SIGNAL(triggered()),
              fileNameMapper, SLOT(map()));
      fileNameMapper->setMapping(action, fileInfo.filePath());

      if (j == 0) {
        action->setShortcut(QKeySequence::Forward);
        actionNext->setToolTip(QString("Next: %1").arg(fileInfo.fileName()));
        fileNameMapper->setMapping(actionNext, fileInfo.filePath());
      }

      ++j;
    }
  }
  actionNext->setEnabled(!actionNext->menu()->isEmpty());
}

void SaxsviewImageWindow::SaxsviewImageWindowPrivate::setupImage() {
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
}

void SaxsviewImageWindow::SaxsviewImageWindowPrivate::setScale(Saxsview::Plot::PlotScale s) {
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

SaxsviewImageWindow::SaxsviewImageWindow(QWidget *parent)
 : SaxsviewSubWindow(parent), p(new SaxsviewImageWindowPrivate(this)) {
  p->setupUi();
  p->setupActions();
  p->setupToolBar();
  p->setupSignalMappers();
  p->setupImage();
  p->setupTracker();

  setScale(Saxsview::Plot::Log10Scale);
}

SaxsviewImageWindow::~SaxsviewImageWindow() {
  delete p;
}

bool SaxsviewImageWindow::canShow(const QString& fileName) {
  saxs_image_format *format = saxs_image_format_find(fileName.toAscii(), 0L);
  return format && format->read;
}

int SaxsviewImageWindow::scale() const {
  return p->scale;
}

bool SaxsviewImageWindow::zoomEnabled() const {
  return p->plot->zoomEnabled();
}

bool SaxsviewImageWindow::moveEnabled() const {
  return p->plot->moveEnabled();
}

QToolBar* SaxsviewImageWindow::createToolBar() {
  return p->toolBar;
}

void SaxsviewImageWindow::load(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  if (!fileInfo.exists())
    return;

  setCursor(Qt::WaitCursor);

  saxs_image *image = saxs_image_create();
  if (saxs_image_read(image, fileName.toAscii(), 0L) != 0) {
    QMessageBox::critical(this,
                          "Filetype not recognized",
                          QString("Could not load file as image:\n"
                                  "'%1'.").arg(fileName));
    saxs_image_free(image);

    unsetCursor();
    return;
  }

  setWindowTitle(fileName);
  p->updateActions(fileName);

  p->image->detach();
  p->image->setData(new ImageData(image));
  p->image->attach(p->plot);

  const QwtDoubleInterval range = p->image->data()->range();
  p->plot->axisWidget(QwtPlot::yRight)->setColorMap(range,
                                                    p->image->colorMap());

  p->spinThreshold->setValue(1);

  p->plot->setAxisScale(QwtPlot::yRight,
                        range.minValue(),
                        range.maxValue());

  p->plot->setZoomBase(p->image->boundingRect());
  p->plot->replot();

  unsetCursor();
}

void SaxsviewImageWindow::exportAs(const QString& fileName) {
  p->plot->exportAs(fileName);
}

void SaxsviewImageWindow::print() {
  p->plot->print();
}

void SaxsviewImageWindow::zoomFit() {
  p->plot->setZoomBase(p->image->boundingRect());
}

void SaxsviewImageWindow::setZoomEnabled(bool on) {
  p->plot->setZoomEnabled(on);
}

void SaxsviewImageWindow::setMoveEnabled(bool on) {
  p->plot->setMoveEnabled(on);
}

void SaxsviewImageWindow::setScale(int scale) {
  p->setScale((Saxsview::Plot::PlotScale)scale);
}

void SaxsviewImageWindow::setThreshold(int n) {
  QwtScaleWidget *scale = p->plot->axisWidget(QwtPlot::yRight);

  QwtDoubleInterval range = scale->colorBarInterval();
  range.setMinValue(n);

  scale->setColorMap(range, p->image->colorMap());

  dynamic_cast<ImageData*>(p->image->data())->setMin(n);

  p->plot->replot();
}

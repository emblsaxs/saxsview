/*
 * Copyright (C) 2011, 2013 Daniel Franke <dfranke@users.sourceforge.net>
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

// saxsview/svimage
#include "svimagesubwindow.h"
#include "svimagemainwindow.h"
#include "svimagemaskthresholdsdialog.h"

// libsaxsview
#include "saxsview.h"
#include "saxsview_config.h"
#include "saxsview_image.h"

// external/qwt
#include "qwt_picker_machine.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_picker.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

// global
#include <QtGui>


class ImagePicker : public QwtPlotPicker {
public:
  ImagePicker(SaxsviewFrame *f, QwtPlotCanvas *canvas)
   : QwtPlotPicker(canvas), frame(f) {
  }

  QwtText trackerTextF(const QPointF &pos) const {
    if (QWidget *w = qApp->activeWindow()) {
      static const QString format = "x=%1, y=%2, count=%3";
      const float x = pos.x(), y = pos.y();

      QStatusTipEvent event(format.arg((int)x, 4).arg((int)y, 4).arg(frame->data()->value(x, y)));
      qApp->sendEvent(w, &event);
    }

    return QwtText();
  }

private:
  SaxsviewFrame *frame;
};


class SVImageSubWindow::Private {
public:
  Private();
  ~Private();

  void setupUi(SVImageSubWindow *w);
  void setupPicker(SVImageSubWindow *w);
  void setupFilesystemModel(SVImageSubWindow *w);

  void setFilePath(const QString&);

  QString filePath;

  SaxsviewImage *image;
  SaxsviewFrame *frame;
  SaxsviewMask *mask;
  QwtPicker *tracker;

  QwtPlotPicker *addPointPicker, *addPolygonPicker;
  QwtPlotPicker *removePointPicker, *removePolygonPicker;

  bool watchLatest;
  QFileSystemModel *model;
  QModelIndex rootIndex;
};

SVImageSubWindow::Private::Private()
 : image(0L), frame(0L), tracker(0L),
   addPointPicker(0L), addPolygonPicker(0L),
   removePointPicker(0L), removePolygonPicker(0L),
   watchLatest(false) {
}

SVImageSubWindow::Private::~Private() {
  delete model;
}

void SVImageSubWindow::Private::setupUi(SVImageSubWindow *w) {
  frame = new SaxsviewFrame(w);
  mask  = new SaxsviewMask(w);
  mask->setVisible(true);

  image = new SaxsviewImage(w);
  image->setFrame(frame);
  image->setMask(mask);
  image->setColorMap(Saxsview::GrayColorMap);
  image->setScale(Saxsview::Log10Scale);

  w->setWidget(image);
}

void SVImageSubWindow::Private::setupPicker(SVImageSubWindow *w) {
  tracker = new ImagePicker(frame, image->canvas());
  tracker->setStateMachine(new QwtPickerTrackerMachine);
  tracker->setTrackerMode(QwtPicker::AlwaysOn);

  addPointPicker = new QwtPlotPicker(image->canvas());
  addPointPicker->setStateMachine(new QwtPickerClickPointMachine);
  addPointPicker->setTrackerMode(QwtPicker::ActiveOnly);
  addPointPicker->setRubberBand(QwtPicker::CrossRubberBand);
  addPointPicker->setEnabled(false);
  connect(addPointPicker, SIGNAL(selected(const QPointF&)),
          w, SLOT(addSelectionToMask(const QPointF&)));

  addPolygonPicker = new QwtPlotPicker(image->canvas());
  addPolygonPicker->setStateMachine(new QwtPickerPolygonMachine);
  addPolygonPicker->setTrackerMode(QwtPicker::ActiveOnly);
  addPolygonPicker->setRubberBand(QwtPicker::PolygonRubberBand);
  addPolygonPicker->setEnabled(false);
  connect(addPolygonPicker, SIGNAL(selected(const QVector<QPointF>&)),
          w, SLOT(addSelectionToMask(const QVector<QPointF>&)));

  removePointPicker = new QwtPlotPicker(image->canvas());
  removePointPicker->setStateMachine(new QwtPickerClickPointMachine);
  removePointPicker->setTrackerMode(QwtPicker::ActiveOnly);
  removePointPicker->setRubberBand(QwtPicker::CrossRubberBand);
  removePointPicker->setEnabled(false);
  connect(removePointPicker, SIGNAL(selected(const QPointF&)),
          w, SLOT(removeSelectionFromMask(const QPointF&)));

  removePolygonPicker = new QwtPlotPicker(image->canvas());
  removePolygonPicker->setStateMachine(new QwtPickerPolygonMachine);
  removePolygonPicker->setTrackerMode(QwtPicker::ActiveOnly);
  removePolygonPicker->setRubberBand(QwtPicker::PolygonRubberBand);
  removePolygonPicker->setEnabled(false);
  connect(removePolygonPicker, SIGNAL(selected(const QVector<QPointF>&)),
          w, SLOT(removeSelectionFromMask(const QVector<QPointF>&)));
}

void SVImageSubWindow::Private::setupFilesystemModel(SVImageSubWindow *w) {

  // FIXME: make name filters configurable
  model = new QFileSystemModel;
  model->setFilter(QDir::Files);
  model->setNameFilters(QStringList() << "*.tiff" << "*.cbf" << "*.edf" << "*.msk");
  model->setNameFilterDisables(false);
  model->sort(3);                       // by modification date

  connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          w, SLOT(rowsInserted(const QModelIndex&, int, int)));
}

void SVImageSubWindow::Private::setFilePath(const QString& filePath) {
  this->filePath = filePath;
  rootIndex = model->setRootPath(QFileInfo(filePath).absolutePath());
}


SVImageSubWindow::SVImageSubWindow(QWidget *parent)
 : QMdiSubWindow(parent), p(new Private) {

  setAttribute(Qt::WA_DeleteOnClose);

  p->setupUi(this);
  p->setupPicker(this);
  p->setupFilesystemModel(this);
}

SVImageSubWindow::~SVImageSubWindow() {
  delete p;
}

SaxsviewImage *SVImageSubWindow::image() const {
  return p->image;
}

QString& SVImageSubWindow::fileName() const {
  return p->filePath;
}

bool SVImageSubWindow::zoomEnabled() const {
  return p->image->isZoomEnabled();
}

bool SVImageSubWindow::moveEnabled() const {
  return p->image->isMoveEnabled();
}

double SVImageSubWindow::lowerThreshold() const {
  return p->frame->minValue();
}

double SVImageSubWindow::upperThreshold() const {
  return p->frame->maxValue();
}

bool SVImageSubWindow::watchLatest() const {
  return p->watchLatest;
}

bool SVImageSubWindow::maskAddPointsEnabled() const {
  return p->addPointPicker->isEnabled();
}

bool SVImageSubWindow::maskAddPolygonEnabled() const {
  return p->addPolygonPicker->isEnabled();
}

bool SVImageSubWindow::maskRemovePointsEnabled() const {
  return false;
}

bool SVImageSubWindow::maskRemovePolygonEnabled() const {
  return false;
}

bool SVImageSubWindow::load(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  if (!fileInfo.exists())
    return false;

  setCursor(Qt::WaitCursor);

  p->frame->setData(new SaxsviewFrameData(fileName));
  p->image->setFrame(p->frame);

  setWindowTitle(QString("%1 - %2").arg(fileName)
                                   .arg(qApp->applicationName()));

  p->setFilePath(fileInfo.filePath());

  unsetCursor();
  return true;
}

void SVImageSubWindow::reload() {
  load(p->filePath);
}

void SVImageSubWindow::exportAs(const QString& fileName,
                                const QString& format) {
  p->image->exportAs(fileName, format);
}

void SVImageSubWindow::print() {
  p->image->print();
}

void SVImageSubWindow::zoomFit() {
  p->image->setZoomBase(p->frame->boundingRect());
}

void SVImageSubWindow::setZoomEnabled(bool on) {
  p->image->setZoomEnabled(on);
}

void SVImageSubWindow::setMoveEnabled(bool on) {
  p->image->setMoveEnabled(on);
}

void SVImageSubWindow::setLowerThreshold(double threshold) {
  p->frame->setMinValue(threshold);
}

void SVImageSubWindow::setUpperThreshold(double threshold) {
  p->frame->setMaxValue(threshold);
}

void SVImageSubWindow::goFirst() {
  if (p->rootIndex.isValid()) {
    int firstRow = 0;
    QModelIndex newIndex = p->model->index(firstRow, 0, p->rootIndex);

    load(p->model->fileInfo(newIndex).filePath());
  }
}

void SVImageSubWindow::goPrevious() {
  if (p->rootIndex.isValid()) {
    QModelIndex currentIndex = p->model->index(p->filePath);
    int previousRow = currentIndex.row() - 1;

    if (previousRow >= 0) {
      QModelIndex newIndex = p->model->index(previousRow, 0, p->rootIndex);
      load(p->model->fileInfo(newIndex).filePath());
    }
  }
}

void SVImageSubWindow::goNext() {
  if (p->rootIndex.isValid()) {
    QModelIndex currentIndex = p->model->index(p->filePath);
    int nextRow = currentIndex.row() + 1;

    if (nextRow < p->model->rowCount(p->rootIndex)) {
      QModelIndex newIndex = p->model->index(nextRow, 0, p->rootIndex);
      load(p->model->fileInfo(newIndex).filePath());
    }
  }
}

void SVImageSubWindow::goLast() {
  if (p->rootIndex.isValid()) {
    int lastRow = p->model->rowCount(p->rootIndex) - 1;
    QModelIndex newIndex = p->model->index(lastRow, 0, p->rootIndex);

    load(p->model->fileInfo(newIndex).filePath());
  }
}

void SVImageSubWindow::setWatchLatest(bool on) {
  if (p->watchLatest != on) {
    p->watchLatest = on;
    if (on)
      goLast();
  }
}

void SVImageSubWindow::newMask() {
  //
  // The current mask, if any, is owned by the plot.
  // No need to delete it.
  //
  p->mask->setData(new SaxsviewFrameData(p->frame->size()));
  p->image->replot();
}

bool SVImageSubWindow::loadMask(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  if (!fileInfo.exists())
    return false;

  p->mask->setData(new SaxsviewFrameData(fileName));
  p->image->replot();

  return true;
}

bool SVImageSubWindow::saveMaskAs(const QString& fileName) {
  return p->mask->save(fileName);
}

void SVImageSubWindow::setMaskByThreshold() {
  double min = p->frame->minValue();
  double max = p->frame->maxValue();

  SVImageMaskThresholdsDialog dlg(this);
  dlg.setRange(min, max);

  if (dlg.exec() == QDialog::Accepted) {
    dlg.selectedThreshold(&min, &max);

    SaxsviewFrameData *frameData = (SaxsviewFrameData*)p->frame->data();
    SaxsviewFrameData *maskData = (SaxsviewFrameData*)p->mask->data();

    setCursor(Qt::WaitCursor);
    for (int x = 0; x < p->frame->size().width(); ++x)
      for (int y = 0; y < p->frame->size().height(); ++y)
        if (frameData->value(x, y) < min || frameData->value(x, y) > max)
          maskData->setValue(x, y, 1.0);
        else
          maskData->setValue(x, y, 0.0);
    unsetCursor();

    p->image->replot();
  }
}

void SVImageSubWindow::setMaskAddPointsEnabled(bool on) {
  p->addPointPicker->setEnabled(on);
}

void SVImageSubWindow::setMaskAddPolygonEnabled(bool on) {
  p->addPolygonPicker->setEnabled(on);
}

void SVImageSubWindow::setMaskRemovePointsEnabled(bool on) {
  p->removePointPicker->setEnabled(on);
}

void SVImageSubWindow::setMaskRemovePolygonEnabled(bool on) {
  p->removePolygonPicker->setEnabled(on);
}


void SVImageSubWindow::rowsInserted(const QModelIndex&, int, int) {
  if (watchLatest())
    goLast();
}

void SVImageSubWindow::addSelectionToMask(const QPointF& point) {
  p->mask->add(point);
}

void SVImageSubWindow::addSelectionToMask(const QVector<QPointF>& points) {
  p->mask->add(points);
}

void SVImageSubWindow::removeSelectionFromMask(const QPointF& point) {
  p->mask->remove(point);
}

void SVImageSubWindow::removeSelectionFromMask(const QVector<QPointF>& points) {
  p->mask->remove(points);
}

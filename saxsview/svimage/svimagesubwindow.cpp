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

// libsaxsview
#include "saxsview.h"
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
  void setupTracker(SVImageSubWindow *w);
  void setupFilesystemModel(SVImageSubWindow *w);

  void setFilePath(const QString&);

  QString filePath;

  SaxsviewImage *image;
  SaxsviewFrame *frame;
  QwtPicker *tracker;

  bool watchLatest;
  QFileSystemModel *model;
  QModelIndex rootIndex;
};

SVImageSubWindow::Private::Private()
 : image(0L), frame(0L), tracker(0L),
   watchLatest(false) {
}

SVImageSubWindow::Private::~Private() {
  delete model;
}

void SVImageSubWindow::Private::setupUi(SVImageSubWindow *w) {
  frame = new SaxsviewFrame(w);

  image = new SaxsviewImage(w);
  image->setFrame(frame);
  image->setScale(Saxsview::AbsoluteScale);

  w->setWidget(image);
}

void SVImageSubWindow::Private::setupTracker(SVImageSubWindow*) {
  tracker = new ImagePicker(frame, image->canvas());
  tracker->setStateMachine(new QwtPickerTrackerMachine);
  tracker->setTrackerMode(QwtPicker::AlwaysOn);
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
  p->setupTracker(this);
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

void SVImageSubWindow::rowsInserted(const QModelIndex&, int, int) {
  if (watchLatest())
    goLast();
}

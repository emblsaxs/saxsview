/*
 * Implementation of 1D-plot subwindows.
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

#include "saxsviewplotwindow.h"
#include "saxsviewmainwindow.h"
#include "saxsview_plot.h"
#include "saxsview_plotcurve.h"
#include "saxsview_configdialog.h"
#include "selectplotwindowdialog.h"

#include "saxsdocument.h"
#include "saxsdocument_format.h"

#include "qwt_picker_machine.h"
#include "qwt_plot_picker.h"

#include <QtGui>


class PlotPicker : public QwtPlotPicker {
public:
  PlotPicker(Saxsview::Plot *p)
   : QwtPlotPicker(p->canvas()), plot(p) {
  }

  QwtText trackerText(const QPoint &pos) const {
    if (QWidget *w = qApp->activeWindow()) {
      Saxsview::PlotCurve *closestCurve = 0L;
      int closestPointIndex = -1;
      double dist = 10000.0;                    // some large number in pixels

      foreach (Saxsview::PlotCurve *curve, plot->curves()) {
        double d;
        int index = curve->closestPoint(pos, &d);
        if (index > 0 && d < dist && d < 10.0) {
          closestCurve = curve;
          closestPointIndex = index;
          dist = d;
        }
      }

      QString tip;
      if (closestCurve) {
        tip += closestCurve->fileName();
        if (!closestCurve->title().isEmpty())
          tip += " (" + closestCurve->title() + ")";
      }
      QStatusTipEvent event(tip);
      qApp->sendEvent(w, &event);
    }

    return QwtText();
  }

private:
  Saxsview::Plot *plot;
};


class SaxsviewPlotWindow::SaxsviewPlotWindowPrivate {
public:
  SaxsviewPlotWindowPrivate(SaxsviewPlotWindow *w) : sw(w) {}

  void setupUi();
  void setupActions();
  void setupToolBar();
  void setupTracker();

  SaxsviewPlotWindow *sw;
  Saxsview::Plot *plot;

  QAction *actionExplode;

  QToolBar *toolBar;

  PlotPicker *tracker;
};

void SaxsviewPlotWindow::SaxsviewPlotWindowPrivate::setupUi() {
  plot = new Saxsview::Plot(sw);
  plot->setAcceptDrops(true);
  plot->installEventFilter(sw);

  sw->setWidget(plot);
}

void SaxsviewPlotWindow::SaxsviewPlotWindowPrivate::setupActions() {
  actionExplode = new QAction("E&xplode", sw);
  actionExplode->setIcon(QIcon(":/icons/office-chart-line-stacked.png"));
  connect(actionExplode, SIGNAL(triggered()),
          sw, SLOT(explode()));
}

void SaxsviewPlotWindow::SaxsviewPlotWindowPrivate::setupToolBar() {
  toolBar = new QToolBar(sw);

  toolBar->addAction(actionExplode);
}

void SaxsviewPlotWindow::SaxsviewPlotWindowPrivate::setupTracker() {
  tracker = new PlotPicker(plot);
  tracker->setStateMachine(new QwtPickerTrackerMachine);
  tracker->setTrackerMode(QwtPicker::AlwaysOn);
}


SaxsviewPlotWindow::SaxsviewPlotWindow(SaxsviewMainWindow *parent)
 : SaxsviewSubWindow(parent), p(new SaxsviewPlotWindowPrivate(this)) {

  p->setupUi();
  p->setupActions();
  p->setupToolBar();
  p->setupTracker();

  setScale(Saxsview::Plot::Log10Scale);

  static int id = 1;
  setWindowTitle(QString("Plot %1").arg(id++));
}

SaxsviewPlotWindow::~SaxsviewPlotWindow() {
  delete p;
}

bool SaxsviewPlotWindow::canShow(const QString& fileName) {
  saxs_document_format *format = saxs_document_format_find(fileName.toAscii(), 0L);
  return format && format->read;
}

int SaxsviewPlotWindow::scale() const {
  return p->plot->scale();
}

bool SaxsviewPlotWindow::zoomEnabled() const {
  return p->plot->zoomEnabled();
}

bool SaxsviewPlotWindow::moveEnabled() const {
  return p->plot->moveEnabled();
}

QToolBar* SaxsviewPlotWindow::createToolBar() {
  return p->toolBar;
}


void SaxsviewPlotWindow::load(const QString& fileName, saxs_curve *curve) {
  QFileInfo fileInfo(fileName);

  Saxsview::PlotPointData points;
  Saxsview::PlotIntervalData intervals;

  saxs_data *data = saxs_curve_data(curve);
  while (data) {
    const double x     = saxs_data_x(data);
    const double y     = saxs_data_y(data);
    const double y_err = saxs_data_y_err(data);

    data = saxs_data_next(data);
    if (y - y_err < 1e-6)
      continue;

    points.push_back(QPointF(x, y));
    intervals.push_back(QwtIntervalSample(x, y - y_err, y + y_err));
  }

  Saxsview::PlotCurve *plotCurve = new Saxsview::PlotCurve(saxs_curve_type(curve));
  plotCurve->setData(points, intervals);
  if (plotCurve->boundingRect().isValid()) {
    QString curveTitle = fileInfo.fileName();
    if (saxs_curve_title(curve))
      curveTitle += QString(" (%1)").arg(saxs_curve_title(curve));

    plotCurve->setTitle(curveTitle);
    plotCurve->setFileName(fileInfo.absoluteFilePath());

    p->plot->addCurve(plotCurve);

    if (saxs_curve_type(curve) & SAXS_CURVE_PROBABILITY_DATA)
      setScale(Saxsview::Plot::AbsoluteScale);

  } else
    delete plotCurve;
}

void SaxsviewPlotWindow::load(const QString& fileName) {
  QFileInfo fileInfo(fileName);
  if (!fileInfo.exists())
    return;

  if (!canShow(fileName)) {
    QMessageBox::critical(this,
                          "Filetype not recognized",
                          QString("Could not load file as plot:\n"
                                  "'%1'.").arg(fileName));
    return;
  }

  //
  // Get a mask of curve types we already have in this plot
  //
  //  -> if the mask is empty, load the first curve) and add
  //     the respective types to the mask
  //
  //  -> for each curve after the first, verify that the
  //     current type of curve doesn't add any bits, if
  //     it does, ask whether it really shall be opened
  //     in the this plot or in another window.
  //

  int curve_type_mask = 0;
  foreach (Saxsview::PlotCurve *curve, p->plot->curves())
    if (curve->type() & SAXS_CURVE_SCATTERING_DATA)
      curve_type_mask |= SAXS_CURVE_SCATTERING_DATA;
    else
      curve_type_mask |= curve->type();

  saxs_document *doc = saxs_document_create();
  saxs_document_read(doc, fileName.toAscii(), 0L);

  saxs_curve *curve = saxs_document_curve(doc);
  while (curve) {
    SaxsviewPlotWindow *plotWindow = 0L;

    if (!curve_type_mask || (curve_type_mask & saxs_curve_type(curve))) {
      plotWindow = this;

    } else {
      QString msg = "The current plot (%1) only contains %2. "
                    "The next curve to open is a %3. Please select "
                    "the plot to display the curve or choose "
                    "'New Window' to open another plot window.";

      msg = msg.arg(windowTitle());
      if (curve_type_mask & SAXS_CURVE_SCATTERING_DATA)
        msg = msg.arg("scattering curves");
      else if (curve_type_mask & SAXS_CURVE_PROBABILITY_DATA)
        msg = msg.arg("probability curves");
      else
        msg = msg.arg("unknown data");

      if (saxs_curve_type(curve) & SAXS_CURVE_SCATTERING_DATA)
        msg = msg.arg("scattering curve");
      else if (saxs_curve_type(curve) & SAXS_CURVE_PROBABILITY_DATA)
        msg = msg.arg("probability curve");
      else
        msg = msg.arg("unknown data");

      SelectPlotWindowDialog dlg(msg);

      dlg.addPlotWindow("New Window", 0L);
      foreach (QMdiSubWindow *subWindow, mdiArea()->subWindowList())
        if (SaxsviewPlotWindow *w = dynamic_cast<SaxsviewPlotWindow*>(subWindow))
          dlg.addPlotWindow(w->windowTitle(), w);

      if (dlg.exec() == QDialog::Accepted) {
        plotWindow = dlg.selectedPlotWindow();
        if (!plotWindow) {
          plotWindow = new SaxsviewPlotWindow(mainWindow());
          mainWindow()->addSubWindow(plotWindow);
        }
      }

      if (dlg.tileSubWindows())
        mdiArea()->tileSubWindows();
    }

    if (plotWindow) {
      plotWindow->load(fileName, curve);
      if (saxs_curve_type(curve) & SAXS_CURVE_SCATTERING_DATA)
        curve_type_mask |= SAXS_CURVE_SCATTERING_DATA;
      else
        curve_type_mask |= saxs_curve_type(curve);
    }

    curve = saxs_curve_next(curve);
  }

  saxs_document_free(doc);
}

void SaxsviewPlotWindow::exportAs(const QString& fileName,
                                  const QString& format) {
  p->plot->exportAs(fileName, format);
}

void SaxsviewPlotWindow::print() {
  p->plot->print();
}

void SaxsviewPlotWindow::zoomFit() {
  p->plot->setZoomBase();
}

void SaxsviewPlotWindow::setZoomEnabled(bool on) {
  p->plot->setZoomEnabled(on);
}

void SaxsviewPlotWindow::setMoveEnabled(bool on) {
  p->plot->setMoveEnabled(on);
}

void SaxsviewPlotWindow::setScale(int scale) {
  p->plot->setScale((Saxsview::Plot::PlotScale)scale);
}

void SaxsviewPlotWindow::configure() {
  Saxsview::PlotConfigDialog dlg(p->plot, this);
  dlg.exec();

  p->plot->replot();
}

void SaxsviewPlotWindow::explode() {
  QStringList fileNames;
  foreach (Saxsview::PlotCurve *curve, p->plot->curves())
    if (!fileNames.contains(curve->fileName()))
      fileNames << curve->fileName();

  //
  // NOTE: scaling by a factor of 10 is usually sufficient to separate
  // curves, but it is not necessarily the case. Maybe one could/should
  // check the overlap of adjacent curves and increase the scaling factor
  // if too close?
  //
  double factor = pow(10.0, floor(fileNames.size() / 2.0));
  foreach (Saxsview::PlotCurve *curve, p->plot->curves())
    curve->setScalingFactorY(factor / pow(10.0, fileNames.indexOf(curve->fileName())));

  //
  // When scaling like this, it is not unlikely that curves
  // have been moved out of the current zoom range. Adjust
  // accordingly.
  //
  p->plot->setZoomBase();
  p->plot->replot();
}

bool SaxsviewPlotWindow::eventFilter(QObject *watchedObj, QEvent *e) {
  if (watchedObj != p->plot)
    return QMdiSubWindow::eventFilter(watchedObj, e);

  switch (e->type()) {
    case QEvent::Drop:
      if (QDropEvent *dropEvent = dynamic_cast<QDropEvent*>(e)) {
        if (dropEvent->mimeData()->hasUrls())
          foreach (QUrl url, dropEvent->mimeData()->urls())
            load(url.toLocalFile());

        dropEvent->acceptProposedAction();
      }
      // fall through

    case QEvent::DragEnter:
    case QEvent::DragMove:
      e->accept();
      return true;

    default:
      return false;
  }
}

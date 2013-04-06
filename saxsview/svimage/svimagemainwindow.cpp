/*
 * Copyright (C) 2011, 2012, 2013 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "svimagemainwindow.h"
#include "svimagesubwindow.h"
#include "svimagepropertydockwidget.h"
#include "saxsview_plot.h"
#include "saxsview_config.h"
#include "config.h"

#include <QtGui>


class SVImageMainWindow::SVImageMainWindowPrivate {
public:
  SVImageMainWindowPrivate(SVImageMainWindow *w);

  void setupActions();
  void setupUi();
  void setupMenus();
  void setupToolbars();
  void setupSignalMappers();

  SVImageMainWindow *mw;

  // "File"-menu
  QAction *actionLoad, *actionReload, *actionQuit, *actionPrint;

  // "Plot"-menu
  QAction *actionZoomFit, *actionZoom, *actionMove;

  // "Go"-menu
  QAction *actionGoFirst, *actionGoPrevious, *actionGoNext, *actionGoLast;
  QAction *actionWatchLatest;

  // "Tools"-menu
  QAction *actionMaskNew, *actionMaskLoad, *actionMaskSaveAs;
  QAction *actionMaskByThreshold;
  QAction *actionMaskAddPoint, *actionMaskAddPolygon;
  QAction *actionMaskRemovePoint, *actionMaskRemovePolygon;

  // "Window"-menu
  QAction *actionPreviousPlot, *actionNextPlot, *actionCascadePlots;
  QAction *actionTilePlots, *actionClosePlot, *actionCloseAllPlots;

  // "Help"-menu
  QAction *actionAbout;

  // A group for all picker-related tasks (zooming, panning, mask editing, ...).
  QActionGroup *actionGroupPlotPicker;

  QMenu *menuFile, *menuCreateSubWindow, *menuRecentFiles, *menuExportAs;
  QMenu *menuPlot, *menuGo, *menuTools, *menuWindow, *menuView, *menuHelp;

  // toolbars
  QToolBar *mainToolBar, *maskToolBar;

  // dock widgets
  SVImagePropertyDockWidget *propertyDock;

  QMdiArea *mdiArea;
  QSignalMapper *windowMapper;
  QSignalMapper *scaleMapper;
  QSignalMapper *recentFileNameMapper;
  QSignalMapper *exportAsFormatMapper;

  typedef QMap<QString, QString> supportedFormatsMap;
  supportedFormatsMap exportAsFormat;
};

SVImageMainWindow::SVImageMainWindowPrivate::SVImageMainWindowPrivate(SVImageMainWindow *w)
 : mw(w) {

  exportAsFormat["pdf"]  = "Portable Document Format";
  exportAsFormat["ps"]   = "Postscript";
#ifdef QT_SVG_LIB
  exportAsFormat["svg"]  = "Scalable Vector Graphics";
#endif
  exportAsFormat["png"]  = "Portable Network Graphics";
  exportAsFormat["jpg"]  = "JPEG";
  exportAsFormat["tiff"] = "TIFF";
  exportAsFormat["bmp"]  = "Windows Bitmap";
}

void SVImageMainWindow::SVImageMainWindowPrivate::setupActions() {
  //
  // "File"-menu
  //
  actionLoad = new QAction("&Open", mw);
  actionLoad->setIcon(QIcon(":icons/document-open.png"));
  actionLoad->setShortcut(QKeySequence::Open);
  connect(actionLoad, SIGNAL(triggered()),
          mw, SLOT(load()));

  actionReload = new QAction("&Reload", mw);
  actionReload->setIcon(QIcon(":icons/view-refresh.png"));
  actionReload->setShortcut(QKeySequence::Refresh);
  actionReload->setEnabled(false);
  connect(actionReload, SIGNAL(triggered()),
          mw, SLOT(reload()));

  actionPrint = new QAction("&Print", mw);
  actionPrint->setIcon(QIcon(":icons/document-print.png"));
  actionPrint->setShortcut(QKeySequence::Print);
  actionPrint->setEnabled(false);
  connect(actionPrint, SIGNAL(triggered()),
          mw, SLOT(print()));

  actionQuit = new QAction("&Quit", mw);
#if QT_VERSION >= 0x040600
  actionQuit->setShortcut(QKeySequence::Quit);
#else
  actionQuit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
#endif
  connect(actionQuit, SIGNAL(triggered()),
          mw, SLOT(close()));

  //
  // "Plot"-menu
  //
  actionZoomFit = new QAction("Fit to Window", mw);
  actionZoomFit->setIcon(QIcon(":icons/zoom-fit-best.png"));
  actionZoomFit->setEnabled(false);
  connect(actionZoomFit, SIGNAL(triggered()),
          mw, SLOT(zoomFit()));

  actionZoom = new QAction("&Zoom", mw);
  actionZoom->setIcon(QIcon(":icons/page-zoom.png"));
  actionZoom->setCheckable(true);
  actionZoom->setEnabled(false);
  connect(actionZoom, SIGNAL(toggled(bool)),
          mw, SLOT(setZoomEnabled(bool)));

  actionMove = new QAction("&Move", mw);
  actionMove->setIcon(QIcon(":icons/input-mouse.png"));
  actionMove->setCheckable(true);
  actionMove->setChecked(false);
  actionMove->setEnabled(false);
  connect(actionMove, SIGNAL(toggled(bool)),
          mw, SLOT(setMoveEnabled(bool)));
  actionZoom->setChecked(true);

  //
  // "Go"-menu
  //
  actionGoFirst = new QAction("&First", mw);
  actionGoFirst->setShortcut(QKeySequence::MoveToStartOfLine);
  actionGoFirst->setEnabled(false);
  connect(actionGoFirst, SIGNAL(triggered()),
          mw, SLOT(goFirst()));

  actionGoPrevious = new QAction("&Previous", mw);
  actionGoPrevious->setShortcut(QKeySequence::MoveToPreviousPage);
  actionGoPrevious->setIcon(mw->style()->standardIcon(QStyle::SP_MediaSkipBackward));
  actionGoPrevious->setEnabled(false);
  connect(actionGoPrevious, SIGNAL(triggered()),
          mw, SLOT(goPrevious()));

  actionGoNext = new QAction("&Next", mw);
  actionGoNext->setShortcut(QKeySequence::MoveToNextPage);
  actionGoNext->setIcon(mw->style()->standardIcon(QStyle::SP_MediaSkipForward));
  actionGoNext->setEnabled(false);
  connect(actionGoNext, SIGNAL(triggered()),
          mw, SLOT(goNext()));

  actionGoLast = new QAction("&Last", mw);
  actionGoLast->setShortcut(QKeySequence::MoveToEndOfLine);
  actionGoLast->setEnabled(false);
  connect(actionGoLast, SIGNAL(triggered()),
          mw, SLOT(goLast()));

  actionWatchLatest = new QAction("&Watch Latest", mw);
  actionWatchLatest->setCheckable(true);
  actionWatchLatest->setChecked(false);
  actionWatchLatest->setEnabled(false);
  connect(actionWatchLatest, SIGNAL(toggled(bool)),
          mw, SLOT(setWatchLatest(bool)));

  //
  // "Tools"-menu
  //
  actionMaskNew = new QAction("&New", mw);
  actionMaskNew->setEnabled(false);
  connect(actionMaskNew, SIGNAL(triggered()),
          mw, SLOT(newMask()));

  actionMaskLoad = new QAction("&Open", mw);
  actionMaskLoad->setEnabled(false);
  connect(actionMaskLoad, SIGNAL(triggered()),
          mw, SLOT(loadMask()));

  actionMaskSaveAs = new QAction("&Save As ...", mw);
  actionMaskSaveAs->setEnabled(false);
  connect(actionMaskSaveAs, SIGNAL(triggered()),
          mw, SLOT(saveMaskAs()));

  actionMaskByThreshold = new QAction("By Threshold ...", mw);
  actionMaskByThreshold->setEnabled(false);
  connect(actionMaskByThreshold, SIGNAL(triggered()),
          mw, SLOT(setMaskByThreshold()));

  actionMaskAddPoint = new QAction("Add pixel", mw);
  actionMaskAddPoint->setCheckable(true);
  actionMaskAddPoint->setChecked(false);
  actionMaskAddPoint->setEnabled(false);
  connect(actionMaskAddPoint, SIGNAL(toggled(bool)),
          mw, SLOT(setMaskAddPointsEnabled(bool)));

  actionMaskAddPolygon = new QAction("Add polygon", mw);
  actionMaskAddPolygon->setCheckable(true);
  actionMaskAddPolygon->setChecked(false);
  actionMaskAddPolygon->setEnabled(false);
  connect(actionMaskAddPolygon, SIGNAL(toggled(bool)),
          mw, SLOT(setMaskAddPolygonEnabled(bool)));

  actionMaskRemovePoint = new QAction("Remove pixel", mw);
  actionMaskRemovePoint->setCheckable(true);
  actionMaskRemovePoint->setChecked(false);
  actionMaskRemovePoint->setEnabled(false);
  connect(actionMaskRemovePoint, SIGNAL(toggled(bool)),
          mw, SLOT(setMaskRemovePointsEnabled(bool)));

  actionMaskRemovePolygon = new QAction("Remove polygon", mw);
  actionMaskRemovePolygon->setCheckable(true);
  actionMaskRemovePolygon->setChecked(false);
  actionMaskRemovePolygon->setEnabled(false);
  connect(actionMaskRemovePolygon, SIGNAL(toggled(bool)),
          mw, SLOT(setMaskRemovePolygonEnabled(bool)));

  //
  // "Window"-menu
  //
  actionPreviousPlot = new QAction("&Previous Image", mw);
  actionPreviousPlot->setShortcut(QKeySequence::PreviousChild);
  connect(actionPreviousPlot, SIGNAL(triggered()),
          mdiArea, SLOT(activatePreviousSubWindow()));

  actionNextPlot = new QAction("&Next Image", mw);
  actionNextPlot->setShortcut(QKeySequence::NextChild);
  connect(actionNextPlot, SIGNAL(triggered()),
          mdiArea, SLOT(activateNextSubWindow()));

  actionCascadePlots = new QAction("C&ascade Images", mw);
  connect(actionCascadePlots, SIGNAL(triggered()),
          mdiArea, SLOT(cascadeSubWindows()));

  actionTilePlots =  new QAction("&Tile Images", mw);
  connect(actionTilePlots, SIGNAL(triggered()),
          mdiArea, SLOT(tileSubWindows()));

  actionClosePlot = new QAction("&Close Current Image", mw);
  actionClosePlot->setShortcut(QKeySequence::Close);
  connect(actionClosePlot, SIGNAL(triggered()),
          mdiArea, SLOT(closeActiveSubWindow()));

  actionCloseAllPlots = new QAction("Close &All Images", mw);
  connect(actionCloseAllPlots, SIGNAL(triggered()),
          mdiArea, SLOT(closeAllSubWindows()));

  //
  // "Help"-menu
  //
  actionAbout = new QAction("&About", mw);
  connect(actionAbout, SIGNAL(triggered()),
          mw, SLOT(about()));


  actionGroupPlotPicker = new QActionGroup(mw);
  actionGroupPlotPicker->addAction(actionZoom);
  actionGroupPlotPicker->addAction(actionMove);

  actionGroupPlotPicker->addAction(actionMaskAddPoint);
  actionGroupPlotPicker->addAction(actionMaskAddPolygon);
  actionGroupPlotPicker->addAction(actionMaskRemovePoint);
  actionGroupPlotPicker->addAction(actionMaskRemovePolygon);
}

void SVImageMainWindow::SVImageMainWindowPrivate::setupUi() {
  mdiArea = new QMdiArea(mw);
  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          mw, SLOT(subWindowActivated(QMdiSubWindow*)));

  propertyDock = new SVImagePropertyDockWidget(mw);
  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          propertyDock, SLOT(subWindowActivated(QMdiSubWindow*)));

  mw->addDockWidget(Qt::RightDockWidgetArea, propertyDock);
  mw->setCentralWidget(mdiArea);
}

void SVImageMainWindow::SVImageMainWindowPrivate::setupMenus() {
  menuRecentFiles = new QMenu("Open &Recent", mw);
  connect(menuRecentFiles, SIGNAL(aboutToShow()),
          mw, SLOT(prepareRecentFilesMenu()));

  menuExportAs = new QMenu("E&xport As", mw);
  menuExportAs->setEnabled(false);

  supportedFormatsMap::const_iterator i = exportAsFormat.constBegin();
  for ( ; i != exportAsFormat.constEnd(); ++i) {
    QAction *action = menuExportAs->addAction(QString("%1 (%2)").arg(i.value()).arg(i.key()));
    connect(action, SIGNAL(triggered()),
            exportAsFormatMapper, SLOT(map()));
    exportAsFormatMapper->setMapping(action, i.key());
  }

  QMenuBar *menuBar = mw->menuBar();

  menuFile = new QMenu("&File", mw);
  menuFile->addAction(actionLoad);
  menuFile->addMenu(menuRecentFiles);
  menuFile->addAction(actionReload);
  menuFile->addMenu(menuExportAs);
  menuFile->addAction(actionPrint);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
  menuBar->addMenu(menuFile);

  menuPlot = new QMenu("&Plot", mw);
  menuPlot->addAction(actionZoomFit);
  menuPlot->addSeparator();
  menuPlot->addAction(actionZoom);
  menuPlot->addAction(actionMove);
  menuBar->addMenu(menuPlot);

  menuGo = new QMenu("&Go", mw);
  menuGo->addAction(actionGoFirst);
  menuGo->addAction(actionGoPrevious);
  menuGo->addAction(actionGoNext);
  menuGo->addAction(actionGoLast);
  menuGo->addSeparator();
  menuGo->addAction(actionWatchLatest);
  menuBar->addMenu(menuGo);

  menuTools = new QMenu("&Tools", mw);
  QMenu *menuMaskTools = menuTools->addMenu("Mask");
  menuMaskTools->addSeparator();
  menuMaskTools->addAction(actionMaskNew);
  menuMaskTools->addAction(actionMaskLoad);
  menuMaskTools->addAction(actionMaskSaveAs);
  menuMaskTools->addSeparator();
  menuMaskTools->addAction(actionMaskByThreshold);
  menuMaskTools->addAction(actionMaskAddPoint);
  menuMaskTools->addAction(actionMaskAddPolygon);
  menuMaskTools->addAction(actionMaskRemovePoint);
  menuMaskTools->addAction(actionMaskRemovePolygon);
  menuBar->addMenu(menuTools);

  menuView = new QMenu("&Views", mw);
  menuView->addAction(propertyDock->toggleViewAction());
  menuView->addSeparator();
  menuView->addAction(mainToolBar->toggleViewAction());
  menuView->addAction(maskToolBar->toggleViewAction());
  menuBar->addMenu(menuView);

  menuWindow = new QMenu("&Window", mw);
  connect(menuWindow, SIGNAL(aboutToShow()),
          mw, SLOT(prepareWindowMenu()));
  menuBar->addMenu(menuWindow);

  menuHelp = new QMenu("&Help", mw);
  menuHelp->addAction(actionAbout);
  menuBar->addMenu(menuHelp);
}

void SVImageMainWindow::SVImageMainWindowPrivate::setupToolbars() {
  mw->setIconSize(QSize(24, 24));
  mw->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  mainToolBar = mw->addToolBar("Main Toolbar");
  mainToolBar->addAction(actionLoad);
  mainToolBar->addAction(actionReload);
  mainToolBar->addAction(actionPrint);
  mainToolBar->addSeparator();
  mainToolBar->addAction(actionZoomFit);
  mainToolBar->addAction(actionZoom);
  mainToolBar->addAction(actionMove);

  maskToolBar = mw->addToolBar("Mask Toolbar");
  maskToolBar->addAction(actionMaskNew);
  maskToolBar->addAction(actionMaskSaveAs);
  maskToolBar->addAction(actionMaskAddPoint);
  maskToolBar->addAction(actionMaskAddPolygon);
  maskToolBar->addAction(actionMaskRemovePoint);
  maskToolBar->addAction(actionMaskRemovePolygon);
}

void SVImageMainWindow::SVImageMainWindowPrivate::setupSignalMappers() {
  //
  // Maps the selected Window in the menu activate the
  // respective subwindow
  //
  windowMapper = new QSignalMapper(mw);
  connect(windowMapper, SIGNAL(mapped(QWidget*)),
          mw, SLOT(setActiveSubWindow(QWidget*)));

  //
  // Open a selected, recently opened file.
  //
  recentFileNameMapper = new QSignalMapper(mw);
  connect(recentFileNameMapper, SIGNAL(mapped(const QString&)),
          mw, SLOT(load(const QString&)));

  //
  // Export a plot to a file of the selected format.
  //
  exportAsFormatMapper = new QSignalMapper(mw);
  connect(exportAsFormatMapper, SIGNAL(mapped(const QString&)),
          mw, SLOT(exportAs(const QString&)));
}

SVImageMainWindow::SVImageMainWindow(QWidget *parent)
 : QMainWindow(parent), p(new SVImageMainWindowPrivate(this)) {

  p->setupSignalMappers();
  p->setupUi();
  p->setupActions();
  p->setupToolbars();
  p->setupMenus();

  statusBar();

  // We want to be able to handle FileOpen events ...
  qApp->installEventFilter(this);
}

SVImageMainWindow::~SVImageMainWindow() {
  delete p;
}

SVImageSubWindow* SVImageMainWindow::currentSubWindow() const {
  QMdiSubWindow *subWindow = p->mdiArea->currentSubWindow();
  return subWindow ? qobject_cast<SVImageSubWindow*>(subWindow) : 0L;
}

void SVImageMainWindow::load() {
  QStringList fileNames = QFileDialog::getOpenFileNames(this, "Open file ...",
                                                        config().recentDirectory());

  setCursor(Qt::WaitCursor);
  foreach (const QString& fileName, fileNames)
    load(fileName);
  unsetCursor();
}

void SVImageMainWindow::load(const QString& fileName) {
  //
  // Check first if the file is already open ...
  //
  foreach (QMdiSubWindow *sw, p->mdiArea->subWindowList())
    if (SVImageSubWindow *w = qobject_cast<SVImageSubWindow*>(sw))
      if (fileName == w->fileName()) {
        p->mdiArea->setActiveSubWindow(w);
        if (w->isMinimized())
          w->showMaximized();
        return;
      }

  //
  // ... it is not. Create a new subwindow and load the file.
  //
  SVImageSubWindow *w = new SVImageSubWindow(this);
  if (!w->load(fileName)) {
    delete w;
    return;
  }

  connect(w, SIGNAL(destroyed(QObject*)),
          this, SLOT(subWindowDestroyed(QObject*)));

  p->mdiArea->addSubWindow(w);

  if (p->mdiArea->subWindowList().size() == 1)
    w->showMaximized();
  else
    w->show();

  config().addRecentFile(fileName);
  config().setRecentDirectory(fileName);
}

void SVImageMainWindow::reload() {
  if (currentSubWindow())
    currentSubWindow()->reload();
}

void SVImageMainWindow::exportAs(const QString& format) {
  if (!currentSubWindow())
    return;

  QString filterFormat = "%1 (*.%2)";

  QString filter = "All files (*.*)";
  SVImageMainWindowPrivate::supportedFormatsMap::const_iterator i = p->exportAsFormat.constBegin();
  for ( ; i != p->exportAsFormat.constEnd(); ++i)
    filter += ";; " + filterFormat.arg(i.value()).arg(i.key());

  QString selectedFilter = filterFormat.arg(p->exportAsFormat.value(format))
                                       .arg(format);

  QString fileName = QFileDialog::getSaveFileName(this, "Export As",
                                                  config().recentDirectory(),
                                                  filter,
                                                  &selectedFilter);

  if (!fileName.isEmpty()) {
    currentSubWindow()->exportAs(fileName, format);
    config().setRecentDirectory(fileName);
  }
}

void SVImageMainWindow::print() {
  if (currentSubWindow())
    currentSubWindow()->print();
}

void SVImageMainWindow::zoomFit() {
  if (currentSubWindow())
    currentSubWindow()->zoomFit();
}

void SVImageMainWindow::setZoomEnabled(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setZoomEnabled(on);
}

void SVImageMainWindow::setMoveEnabled(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setMoveEnabled(on);
}

void SVImageMainWindow::goFirst() {
  if (currentSubWindow())
    currentSubWindow()->goFirst();
}

void SVImageMainWindow::goPrevious() {
  if (currentSubWindow())
    currentSubWindow()->goPrevious();
}

void SVImageMainWindow::goNext() {
  if (currentSubWindow())
    currentSubWindow()->goNext();
}

void SVImageMainWindow::goLast() {
  if (currentSubWindow())
    currentSubWindow()->goLast();
}

void SVImageMainWindow::setWatchLatest(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setWatchLatest(on);
}

void SVImageMainWindow::newMask() {
  if (currentSubWindow())
    currentSubWindow()->newMask();
}

void SVImageMainWindow::loadMask() {
  if (currentSubWindow()) {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Mask ...",
                                                    config().recentDirectory(),
                                                    "Mask files (*.msk)");

    if (!fileName.isEmpty())
      if (!currentSubWindow()->loadMask(fileName))
        QMessageBox::warning(this, "Loading the mask failed",
                             QString("Failed to load mask file: %1").arg(fileName));


      config().setRecentDirectory(fileName);
  }
}

void SVImageMainWindow::saveMaskAs() {
  if (currentSubWindow()) {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Mask As...",
                                                    config().recentDirectory(),
                                                    "Mask files (*.msk)");

    if (!fileName.isEmpty())
      if (!currentSubWindow()->saveMaskAs(fileName))
        QMessageBox::warning(this, "Saving the mask failed",
                             QString("Failed to save the current mask to: %1").arg(fileName));

      config().setRecentDirectory(fileName);
  }
}

void SVImageMainWindow::setMaskByThreshold() {
  if (currentSubWindow())
    currentSubWindow()->setMaskByThreshold();
}

void SVImageMainWindow::setMaskAddPointsEnabled(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setMaskAddPointsEnabled(on);
}

void SVImageMainWindow::setMaskAddPolygonEnabled(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setMaskAddPolygonEnabled(on);
}

void SVImageMainWindow::setMaskRemovePointsEnabled(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setMaskRemovePointsEnabled(on);
}

void SVImageMainWindow::setMaskRemovePolygonEnabled(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setMaskRemovePolygonEnabled(on);
}



void SVImageMainWindow::about() {
  QString title = QString("About %1").arg(PROJECT_NAME);
  QString about = QString("%1 %3\n"
                          "Written by Daniel Franke <%2>\n"
                          "\n"
                          "This is free software: you are free to "
                          "change and redistribute it. There is NO, "
                          "WARRANTY to the extent permitted by law.\n"
                          "\n"
                          // And complying to COPYING of qwt:
                          "%1 is based in part on the work of the "
                          "Qwt project (http://qwt.sourceforge.net).");

  QMessageBox::about(this, title,
                     about.arg(PROJECT_NAME)
                          .arg(PROJECT_BUGREPORT)
                          .arg(PROJECT_VERSION));
}

void SVImageMainWindow::prepareWindowMenu() {
  QList<QMdiSubWindow*> windowList = p->mdiArea->subWindowList();

  const bool on = !windowList.isEmpty();
  p->actionPreviousPlot->setEnabled(on);
  p->actionNextPlot->setEnabled(on);
  p->actionCascadePlots->setEnabled(on);
  p->actionTilePlots->setEnabled(on);
  p->actionClosePlot->setEnabled(on);
  p->actionCloseAllPlots->setEnabled(on);

  p->menuWindow->clear();
  p->menuWindow->addAction(p->actionClosePlot);
  p->menuWindow->addAction(p->actionCloseAllPlots);
  p->menuWindow->addSeparator();
  p->menuWindow->addAction(p->actionCascadePlots);
  p->menuWindow->addAction(p->actionTilePlots);
  p->menuWindow->addSeparator();
  p->menuWindow->addAction(p->actionPreviousPlot);
  p->menuWindow->addAction(p->actionNextPlot);
  if (!windowList.isEmpty())
    p->menuWindow->addSeparator();

  QActionGroup *windowGroup = new QActionGroup(this);
  foreach (QMdiSubWindow *window, windowList) {
    QAction *action = windowGroup->addAction(window->windowTitle());
    action->setCheckable(true);
    action->setChecked(window == p->mdiArea->currentSubWindow());
    connect(action, SIGNAL(triggered()),
            p->windowMapper, SLOT(map()));
    p->windowMapper->setMapping(action, window);
  }
  p->menuWindow->addActions(windowGroup->actions());
}

void SVImageMainWindow::prepareRecentFilesMenu() {
  p->menuRecentFiles->clear();
  foreach (QString fileName, config().recentFiles()) {
    QAction *action = p->menuRecentFiles->addAction(fileName);
    connect(action, SIGNAL(triggered()),
            p->recentFileNameMapper, SLOT(map()));
    p->recentFileNameMapper->setMapping(action, fileName);
  }
}

void SVImageMainWindow::setActiveSubWindow(QWidget *w) {
  if (w)
    p->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(w));
}

void SVImageMainWindow::subWindowActivated(QMdiSubWindow *w) {
  if (SVImageSubWindow *subWindow = qobject_cast<SVImageSubWindow*>(w)) {
    //
    // Synchronize actions beween subwindow and local actions.
    //
    p->actionZoom->setChecked(subWindow->zoomEnabled());
    p->actionMove->setChecked(subWindow->moveEnabled());

    p->actionWatchLatest->setChecked(subWindow->watchLatest());

    p->actionMaskAddPoint->setChecked(subWindow->maskAddPointsEnabled());
    p->actionMaskAddPolygon->setChecked(subWindow->maskAddPolygonEnabled());
    p->actionMaskRemovePoint->setChecked(subWindow->maskRemovePointsEnabled());
    p->actionMaskRemovePolygon->setChecked(subWindow->maskRemovePolygonEnabled());
  }

  //
  // 0L if and only if the last subwindow was closed.
  //
  const bool on = (currentSubWindow() != 0L);
  p->actionReload->setEnabled(on);
  p->actionPrint->setEnabled(on);
  p->actionZoomFit->setEnabled(on);
  p->actionZoom->setEnabled(on);
  p->actionMove->setEnabled(on);
  p->menuExportAs->setEnabled(on);

  p->actionGoFirst->setEnabled(on);
  p->actionGoPrevious->setEnabled(on);
  p->actionGoNext->setEnabled(on);
  p->actionGoLast->setEnabled(on);
  p->actionWatchLatest->setEnabled(on);

  p->actionMaskNew->setEnabled(on);
  p->actionMaskLoad->setEnabled(on);
  p->actionMaskSaveAs->setEnabled(on);
  p->actionMaskByThreshold->setEnabled(on);
  p->actionMaskAddPoint->setEnabled(on);
  p->actionMaskAddPolygon->setEnabled(on);
  p->actionMaskRemovePoint->setEnabled(on);
  p->actionMaskRemovePolygon->setEnabled(on);
}

void SVImageMainWindow::subWindowDestroyed(QObject*) {
}

bool SVImageMainWindow::eventFilter(QObject *o, QEvent *e) {
  //
  // The Mac Finder does not pass the filename as an argument on double-click
  // but opens the application without any argument and sends a FileOpen event
  // instead.
  //
  // This explains it in more detail and gives a code example for Qt3: 
  //    http://doc.qt.nokia.com/qq/qq12-mac-events.html
  //
  // Luckily the event is available in Qt4 already ...
  //
  if (QFileOpenEvent *open = dynamic_cast<QFileOpenEvent*>(e)) {
    load(open->file());
    return true;

  } else
    return QMainWindow::eventFilter(o, e);
}
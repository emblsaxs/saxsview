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
  QActionGroup *actionGroupZoomMove;

  // "Window"-menu
  QAction *actionPreviousPlot, *actionNextPlot, *actionCascadePlots;
  QAction *actionTilePlots, *actionClosePlot, *actionCloseAllPlots;

  // "Help"-menu
  QAction *actionAbout;

  QMenu *menuFile, *menuCreateSubWindow, *menuRecentFiles, *menuExportAs;
  QMenu *menuPlot, *menuWindow, *menuView, *menuHelp;

  // toolbars
  QToolBar *svimageToolBar;

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

  actionGroupZoomMove = new QActionGroup(mw);
  actionGroupZoomMove->addAction(actionZoom);
  actionGroupZoomMove->addAction(actionMove);

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
}

void SVImageMainWindow::SVImageMainWindowPrivate::setupUi() {
  mdiArea = new QMdiArea(mw);
  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          mw, SLOT(subWindowActivated(QMdiSubWindow*)));

  propertyDock = new SVImagePropertyDockWidget(mw);
  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          propertyDock, SLOT(subWindowActivated(QMdiSubWindow*)));

//   menu->addAction(dock->toggleViewAction());
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
  menuPlot->addActions(actionGroupZoomMove->actions());
  menuBar->addMenu(menuPlot);

  menuView = new QMenu("&View", mw);
  menuView->addAction(svimageToolBar->toggleViewAction());
  menuView->addAction(propertyDock->toggleViewAction());
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

  svimageToolBar = mw->addToolBar("svimage Toolbar");
  svimageToolBar->addAction(actionLoad);
  svimageToolBar->addAction(actionReload);
  svimageToolBar->addAction(actionPrint);
  svimageToolBar->addSeparator();
  svimageToolBar->addAction(actionZoomFit);
  svimageToolBar->addActions(actionGroupZoomMove->actions());
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
    // Synchronize zoom and move actions beween subwindow
    // and local actions.
    //
    p->actionZoom->setChecked(subWindow->zoomEnabled());
    p->actionMove->setChecked(subWindow->moveEnabled());
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
}

void SVImageMainWindow::subWindowDestroyed(QObject*) {
}

/*
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

#include "saxsviewmainwindow.h"
#include "saxsviewplotwindow.h"
#include "saxsviewimagewindow.h"
#include "saxsview_plot.h"
#include "saxsview_configdialog.h"
#include "config.h"

#include <QtGui>

class SaxsviewMainWindow::SaxsviewMainWindowPrivate {
public:
  SaxsviewMainWindowPrivate(SaxsviewMainWindow *w);

  void setupActions();
  void setupUi();
  void setupMenus();
  void setupToolbars();
  void setupSignalMappers();

  void addSubWindow(SaxsviewSubWindow *w);

  SaxsviewMainWindow *mw;

  // "File"-menu
  QAction *actionCreatePlotWindow, *actionCreateImageWindow;
  QAction *actionLoad, *actionQuit, *actionPrint;

  // "Plot"-menu
  QAction *actionAbsScale, *actionLogScale;
  QActionGroup *actionGroupScale;

  QAction *actionZoomFit, *actionZoom, *actionMove;
  QActionGroup *actionGroupZoomMove;

  // "Settings"-menu
  QAction *actionConfigurePlot, *actionConfigureSaxsview;

  // "Window"-menu
  QAction *actionPreviousPlot, *actionNextPlot, *actionCascadePlots;
  QAction *actionTilePlots, *actionClosePlot, *actionCloseAllPlots;

  // "Help"-menu
  QAction *actionAbout;

  QMenu *menuFile, *menuCreateSubWindow, *menuRecentFiles, *menuExportAs;
  QMenu *menuPlot, *menuWindow, *menuSettings, *menuHelp;

  // toolbars
  QToolBar *saxsviewToolBar, *plotToolBar, *subwindowToolBar;

  QMdiArea *mdiArea;
  QSignalMapper *windowMapper;
  QSignalMapper *scaleMapper;
  QSignalMapper *recentFileNameMapper;
  QSignalMapper *exportAsFormatMapper;

  typedef QMap<QString, QString> supportedFormatsMap;
  supportedFormatsMap exportAsFormat;
};

SaxsviewMainWindow::SaxsviewMainWindowPrivate::SaxsviewMainWindowPrivate(SaxsviewMainWindow *w)
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

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::setupActions() {
  //
  // "File"-menu
  //
  actionCreatePlotWindow = new QAction("&New Plot", mw);
  actionCreatePlotWindow->setIcon(QIcon(":icons/document-new.png"));
  connect(actionCreatePlotWindow, SIGNAL(triggered()),
          mw, SLOT(createPlotWindow()));

  actionCreateImageWindow = new QAction("&New Image", mw);
  actionCreateImageWindow->setIcon(QIcon(":icons/document-new.png"));
  connect(actionCreateImageWindow, SIGNAL(triggered()),
          mw, SLOT(createImageWindow()));

  actionLoad = new QAction("&Open", mw);
  actionLoad->setIcon(QIcon(":icons/document-open.png"));
  actionLoad->setShortcut(QKeySequence::Open);
  connect(actionLoad, SIGNAL(triggered()),
          mw, SLOT(load()));

  actionPrint = new QAction("&Print", mw);
  actionPrint->setIcon(QIcon(":icons/document-print.png"));
  actionPrint->setShortcut(QKeySequence::Print);
  actionPrint->setEnabled(false);
  connect(actionPrint, SIGNAL(triggered()),
          mw, SLOT(print()));

  actionQuit = new QAction("&Quit", mw);
  actionQuit->setShortcut(QKeySequence("Ctrl+Q"));
  connect(actionQuit, SIGNAL(triggered()),
          mw, SLOT(close()));

  //
  // "Plot"-menu
  //
  actionAbsScale = new QAction("Absolute Scale", mw);
  actionAbsScale->setCheckable(true);
  actionAbsScale->setChecked(false);
  connect(actionAbsScale, SIGNAL(toggled(bool)),
          scaleMapper, SLOT(map()));
  scaleMapper->setMapping(actionAbsScale, Saxsview::Plot::AbsoluteScale);

  actionLogScale = new QAction("Logarithmic Scale", mw);
  actionLogScale->setCheckable(true);
  connect(actionLogScale, SIGNAL(toggled(bool)),
          scaleMapper, SLOT(map()));
  scaleMapper->setMapping(actionLogScale, Saxsview::Plot::Log10Scale);
  actionLogScale->setChecked(true);

  actionGroupScale = new QActionGroup(mw);
  actionGroupScale->addAction(actionAbsScale);
  actionGroupScale->addAction(actionLogScale);
  actionGroupScale->setEnabled(false);

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

  // "Settings"-menu
  actionConfigurePlot = new QAction("&Configure Plot", mw);
  actionConfigurePlot->setIcon(QIcon(":icons/document-edit.png"));
  actionConfigurePlot->setEnabled(false);
  connect(actionConfigurePlot, SIGNAL(triggered()),
          mw, SLOT(configurePlot()));

  actionConfigureSaxsview = new QAction("&Configure Saxsview", mw);
  actionConfigureSaxsview->setIcon(QIcon(":icons/configure.png"));
  connect(actionConfigureSaxsview, SIGNAL(triggered()),
          mw, SLOT(configureSaxsview()));

  //
  // "Window"-menu
  //
  actionPreviousPlot = new QAction("&Previous Plot", mw);
  actionPreviousPlot->setShortcut(QKeySequence::PreviousChild);
  connect(actionPreviousPlot, SIGNAL(triggered()),
          mdiArea, SLOT(activatePreviousSubWindow()));

  actionNextPlot = new QAction("&Next Plot", mw);
  actionNextPlot->setShortcut(QKeySequence::NextChild);
  connect(actionNextPlot, SIGNAL(triggered()),
          mdiArea, SLOT(activateNextSubWindow()));

  actionCascadePlots = new QAction("C&ascade Plots", mw);
  connect(actionCascadePlots, SIGNAL(triggered()),
          mdiArea, SLOT(cascadeSubWindows()));

  actionTilePlots =  new QAction("&Tile Plots", mw);
  connect(actionTilePlots, SIGNAL(triggered()),
          mdiArea, SLOT(tileSubWindows()));

  actionClosePlot = new QAction("&Close Current Plot", mw);
  actionClosePlot->setShortcut(QKeySequence::Close);
  connect(actionClosePlot, SIGNAL(triggered()),
          mdiArea, SLOT(closeActiveSubWindow()));

  actionCloseAllPlots = new QAction("Close &All Plots", mw);
  connect(actionCloseAllPlots, SIGNAL(triggered()),
          mdiArea, SLOT(closeAllSubWindows()));

  //
  // "Help"-menu
  //
  actionAbout = new QAction("&About", mw);
  connect(actionAbout, SIGNAL(triggered()),
          mw, SLOT(about()));
}

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::setupUi() {
  mdiArea = new QMdiArea(mw);
  mw->setCentralWidget(mdiArea);

  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          mw, SLOT(subWindowActivated(QMdiSubWindow*)));
}

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::setupMenus() {

  menuCreateSubWindow = new QMenu("New", mw);
  menuCreateSubWindow->addAction(actionCreatePlotWindow);
  menuCreateSubWindow->addAction(actionCreateImageWindow);

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
  menuFile->addMenu(menuCreateSubWindow);
  menuFile->addAction(actionLoad);
  menuFile->addMenu(menuRecentFiles);
  menuFile->addMenu(menuExportAs);
  menuFile->addAction(actionPrint);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
  menuBar->addMenu(menuFile);

  menuPlot = new QMenu("&Plot", mw);
  menuPlot->addActions(actionGroupScale->actions());
  menuPlot->addSeparator();
  menuPlot->addAction(actionZoomFit);
  menuPlot->addSeparator();
  menuPlot->addActions(actionGroupZoomMove->actions());
  menuBar->addMenu(menuPlot);

  menuSettings = new QMenu("&Settings", mw);
  menuSettings->addAction(saxsviewToolBar->toggleViewAction());
  menuSettings->addAction(plotToolBar->toggleViewAction());
  menuSettings->addSeparator();
  menuSettings->addAction(actionConfigurePlot);
  menuSettings->addAction(actionConfigureSaxsview);
  menuBar->addMenu(menuSettings);

  menuWindow = new QMenu("&Window", mw);
  connect(menuWindow, SIGNAL(aboutToShow()),
          mw, SLOT(prepareWindowMenu()));
  menuBar->addMenu(menuWindow);

  menuHelp = new QMenu("&Help", mw);
  menuHelp->addAction(actionAbout);
  menuBar->addMenu(menuHelp);
}

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::setupToolbars() {
  mw->setIconSize(QSize(24, 24));
  mw->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  saxsviewToolBar = mw->addToolBar("saxsview Toolbar");
  QAction *action = saxsviewToolBar->addAction(QIcon(":icons/document-new.png"), "New Plot");
  connect(action, SIGNAL(triggered()),
          mw, SLOT(createPlotWindow()));
//   action->setMenu(menuCreateSubWindow);

  plotToolBar = mw->addToolBar("plot Toolbar");
  plotToolBar->addAction(actionLoad);
  plotToolBar->addAction(actionPrint);
  plotToolBar->addSeparator();
  plotToolBar->addAction(actionZoomFit);
  plotToolBar->addActions(actionGroupZoomMove->actions());
  plotToolBar->addSeparator();
  plotToolBar->addAction(actionConfigurePlot);

  subwindowToolBar = mw->addToolBar("subwindow Toolbar");
  mw->removeToolBar(subwindowToolBar);
}

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::setupSignalMappers() {
  //
  // Maps the selected Window in the menu activate the
  // respective subwindow
  //
  windowMapper = new QSignalMapper(mw);
  connect(windowMapper, SIGNAL(mapped(QWidget*)),
          mw, SLOT(setActiveSubWindow(QWidget*)));

  //
  // Maps scaling-actions (abs, log10) to the scaling slot of
  // the respective plots
  //
  scaleMapper = new QSignalMapper(mw);
  connect(scaleMapper, SIGNAL(mapped(int)),
          mw, SLOT(setScale(int)));

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

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::addSubWindow(SaxsviewSubWindow *w) {
  mdiArea->addSubWindow(w);

  if (mdiArea->subWindowList().size() == 1)
    w->showMaximized();
  else
    w->show();
}


SaxsviewMainWindow::SaxsviewMainWindow(QWidget *parent)
 : QMainWindow(parent), p(new SaxsviewMainWindowPrivate(this)) {

  p->setupSignalMappers();
  p->setupUi();
  p->setupActions();
  p->setupToolbars();
  p->setupMenus();
}

SaxsviewMainWindow::~SaxsviewMainWindow() {
  delete p;
}

SaxsviewSubWindow* SaxsviewMainWindow::currentSubWindow() const {
  QMdiSubWindow *subWindow = p->mdiArea->currentSubWindow();
  return subWindow ? qobject_cast<SaxsviewSubWindow*>(subWindow) : 0L;
}

void SaxsviewMainWindow::createPlotWindow() {
  p->addSubWindow(new SaxsviewPlotWindow(this));
}

void SaxsviewMainWindow::createImageWindow() {
  p->addSubWindow(new SaxsviewImageWindow(this));
}

void SaxsviewMainWindow::load() {
  QStringList fileNames = QFileDialog::getOpenFileNames(this, "Open file ...");

  foreach (QString fileName, fileNames)
    load(fileName);
}

void SaxsviewMainWindow::load(const QString& fileName) {
  //
  // 1. If there is no subwindow at all, create an
  //    appropriate one and load the file. If the
  //    file type is unknown, reject it and inform user.
  //
  // 2. If a subwindow exists and can load the file, load it
  //
  // 3. If a subwindow exists and it can not load the file,
  //    reject it and inform user.
  //    (is done by loading it anyway, the subwindows reject
  //     anything they don't like)
  //

  if (!currentSubWindow()) {
    if (SaxsviewPlotWindow::canShow(fileName))
      createPlotWindow();
    else if (SaxsviewImageWindow::canShow(fileName))
      createImageWindow();
    else {
      QMessageBox::critical(this,
                            "Filetype not recognized",
                            QString("Could not load '%1'.").arg(fileName));
      return;
    }
  }

  if (currentSubWindow()) {
    currentSubWindow()->load(fileName);

    //
    // Add to the list of recently opened files;
    // remove duplicates (if any), prepend current
    // filename and remove old ones (if any).
    //
    QSettings settings;
    QStringList recentFiles = settings.value("saxsview/recentfiles").toStringList();

    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    while (recentFiles.size() > 10)
      recentFiles.removeLast();

    settings.setValue("saxsview/recentfiles", recentFiles);
  }
}

void SaxsviewMainWindow::exportAs(const QString& format) {
  if (!currentSubWindow())
    return;

  QString filterFormat = "%1 (*.%2)";

  QString filter = "All files (*.*)";
  SaxsviewMainWindowPrivate::supportedFormatsMap::const_iterator i = p->exportAsFormat.constBegin();
  for ( ; i != p->exportAsFormat.constEnd(); ++i)
    filter += ";; " + filterFormat.arg(i.value()).arg(i.key());

  QString selectedFilter = filterFormat.arg(p->exportAsFormat.value(format))
                                       .arg(format);

  QString fileName = QFileDialog::getSaveFileName(this, "Export As",
                                                  QDir::currentPath(),
                                                  filter,
                                                  &selectedFilter);

  if (fileName.isEmpty())
    return;

  // append selected extension if the user didn't enter one
  QString ext = QFileInfo(fileName).completeSuffix();
  if (ext.isEmpty()) {
    fileName += "." + format;
    ext = format;
  }

  currentSubWindow()->exportAs(fileName);
}

void SaxsviewMainWindow::print() {
  if (currentSubWindow())
    currentSubWindow()->print();
}

void SaxsviewMainWindow::zoomFit() {
  if (currentSubWindow())
    currentSubWindow()->zoomFit();
}

void SaxsviewMainWindow::setZoomEnabled(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setZoomEnabled(on);
}

void SaxsviewMainWindow::setMoveEnabled(bool on) {
  if (currentSubWindow())
    currentSubWindow()->setMoveEnabled(on);
}

void SaxsviewMainWindow::setScale(int scale) {
  if (currentSubWindow())
    currentSubWindow()->setScale(scale);
}

void SaxsviewMainWindow::configurePlot() {
  if (currentSubWindow())
    currentSubWindow()->configure();
}

void SaxsviewMainWindow::configureSaxsview() {
  Saxsview::SaxsviewConfigDialog config(this);
  config.exec();
}

void SaxsviewMainWindow::about() {
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

void SaxsviewMainWindow::prepareWindowMenu() {
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

void SaxsviewMainWindow::prepareRecentFilesMenu() {
  QSettings settings;
  QStringList recentFiles = settings.value("saxsview/recentfiles").toStringList();

  p->menuRecentFiles->clear();
  foreach (QString fileName, recentFiles) {
    QAction *action = p->menuRecentFiles->addAction(fileName);
    connect(action, SIGNAL(triggered()),
            p->recentFileNameMapper, SLOT(map()));
    p->recentFileNameMapper->setMapping(action, fileName);
  }
}

void SaxsviewMainWindow::setActiveSubWindow(QWidget *w) {
  if (w)
    p->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(w));
}

void SaxsviewMainWindow::subWindowActivated(QMdiSubWindow *w) {
  if (p->subwindowToolBar)
    removeToolBar(p->subwindowToolBar);

  if (SaxsviewSubWindow *subWindow = qobject_cast<SaxsviewSubWindow*>(w)) {
    //
    // Synchronize the scale of the subwindow with the local
    // actions (i.e. the scaling shown in the menu or toolbar).
    //
    QObject *sender = p->scaleMapper->mapping(subWindow->scale());
    if (QAction *action = qobject_cast<QAction*>(sender))
      action->setChecked(true);

    //
    // Synchronize zoom and move actions beween subwindow
    // and local actions.
    //
    p->actionZoom->setChecked(subWindow->zoomEnabled());
    p->actionMove->setChecked(subWindow->moveEnabled());

    //
    // Add subwindows specifc toolbar (if any).
    //
    p->subwindowToolBar = subWindow->createToolBar();
    if (p->subwindowToolBar) {
      p->subwindowToolBar->show();
      addToolBar(p->subwindowToolBar);
    }
  }

  //
  // 0L if and only if the last subwindow was closed.
  //
  const bool on = (w != 0L);
  p->actionPrint->setEnabled(on);
  p->actionGroupScale->setEnabled(on);
  p->actionZoomFit->setEnabled(on);
  p->actionZoom->setEnabled(on);
  p->actionMove->setEnabled(on);
  p->actionConfigurePlot->setEnabled(on);
  p->menuExportAs->setEnabled(on);
}

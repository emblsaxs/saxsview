/*
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
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

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QList>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMenuBar>
#include <QSettings>
#include <QSignalMapper>
#include <QStyle>
#include <QToolBar>

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

  QAction *actionZoomIn, *actionZoomOut, *actionZoom, *actionMove;
  QActionGroup *actionGroupZoomMove;

  QAction *actionConfigure;

  // "Window"-menu
  QAction *actionPreviousPlot, *actionNextPlot, *actionCascadePlots;
  QAction *actionTilePlots, *actionClosePlot, *actionCloseAllPlots;

  // "Help"-menu
  QAction *actionAbout;

  QMenu *menuFile, *menuCreateSubWindow, *menuRecentFiles, *menuExportAs;
  QMenu *menuPlot, *menuWindow, *menuHelp;

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

  exportAsFormat["pdf"] = "Portable Document Format";
  exportAsFormat["ps"]  = "Postscript";
#ifdef QT_SVG_LIB
  exportAsFormat["svg"] = "Scalable Vector Graphics";
#endif
#ifdef QT_IMAGEFORMAT_PNG
  exportAsFormat["png"] = "Portable Network Graphics";
#endif
#ifdef QT_IMAGEFORMAT_JPEG
  exportAsFormat["jpg"] = "JPEG";
#endif
  exportAsFormat["bmp"] = "Windows Bitmap";
}

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::setupActions() {
  QStyle *style = qApp->style();

  //
  // "File"-menu
  //
  actionCreatePlotWindow = new QAction("&New Plot", mw);
  actionCreatePlotWindow->setIcon(style->standardIcon(QStyle::SP_FileIcon));
  connect(actionCreatePlotWindow, SIGNAL(triggered()),
          mw, SLOT(createPlotWindow()));

  actionCreateImageWindow = new QAction("&New Image", mw);
  actionCreateImageWindow->setIcon(style->standardIcon(QStyle::SP_FileIcon));
  connect(actionCreateImageWindow, SIGNAL(triggered()),
          mw, SLOT(createImageWindow()));

  actionLoad = new QAction("&Open", mw);
  actionLoad->setIcon(style->standardIcon(QStyle::SP_DirIcon));
  actionLoad->setShortcut(QKeySequence::Open);
  connect(actionLoad, SIGNAL(triggered()),
          mw, SLOT(load()));

  actionPrint = new QAction("&Print", mw);
  actionPrint->setShortcut(QKeySequence::Print);
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

  actionZoomIn = new QAction("Zoom &in", mw);
  connect(actionZoomIn, SIGNAL(triggered()),
          mw, SLOT(zoomIn()));

  actionZoomOut = new QAction("Zoom &out", mw);
  connect(actionZoomOut, SIGNAL(triggered()),
          mw, SLOT(zoomOut()));

  actionZoom = new QAction("&Zoom", mw);
  actionZoom->setCheckable(true);
  connect(actionZoom, SIGNAL(toggled(bool)),
          mw, SLOT(setZoomEnabled(bool)));

  actionMove = new QAction("&Move", mw);
  actionMove->setCheckable(true);
  actionMove->setChecked(false);
  connect(actionMove, SIGNAL(toggled(bool)),
          mw, SLOT(setMoveEnabled(bool)));
  actionZoom->setChecked(true);

  actionGroupZoomMove = new QActionGroup(mw);
  actionGroupZoomMove->addAction(actionZoom);
  actionGroupZoomMove->addAction(actionMove);

  actionConfigure = new QAction("&Configure", mw);
  connect(actionConfigure, SIGNAL(triggered()),
          mw, SLOT(configure()));

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
  menuPlot->addAction(actionZoomIn);
  menuPlot->addAction(actionZoomOut);
  menuPlot->addActions(actionGroupZoomMove->actions());
  menuPlot->addSeparator();
  menuPlot->addAction(actionConfigure);
  menuBar->addMenu(menuPlot);

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

  QToolBar *toolBar;

  toolBar = mw->addToolBar("saxsview Toolbar");
  QAction *action = toolBar->addAction(qApp->style()->standardIcon(QStyle::SP_FileIcon), "New");
  connect(action, SIGNAL(triggered()),
          mw, SLOT(createPlotWindow()));
  action->setMenu(menuCreateSubWindow);

  toolBar = mw->addToolBar("plot Toolbar");
  toolBar->addAction(actionLoad);
  toolBar->addAction(actionPrint);
  toolBar->addAction(actionZoomIn);
  toolBar->addAction(actionZoomOut);
  toolBar->addActions(actionGroupZoomMove->actions());
  toolBar->addSeparator();
  toolBar->addAction(actionConfigure);
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
  p->setupMenus();
  p->setupToolbars();
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

void SaxsviewMainWindow::zoomIn() {
  if (currentSubWindow())
    currentSubWindow()->zoomIn();
}

void SaxsviewMainWindow::zoomOut() {
  if (currentSubWindow())
    currentSubWindow()->zoomOut();
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

void SaxsviewMainWindow::configure() {
  if (currentSubWindow())
    currentSubWindow()->configure();
}

void SaxsviewMainWindow::about() {
  QMessageBox::about(this, "About saxsview",
                     "Saxsview 0.1\n"
                     "Written by Daniel Franke <dfranke@users.sourceforge.net>\n"
                     "\n"
                     "This is free software: you are free to "
                     "change and redistribute it. There is NO, "
                     "WARRANTY to the extent permitted by law.\n"
                     "\n"
                     // And complying to COPYING of qwt:
                     "saxsview is based in part on the work of"
                     "the Qwt project (http://qwt.sourceforge.net).");
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
  }
}

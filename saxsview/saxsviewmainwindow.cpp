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
#include "saxsviewsubwindow.h"
#include "saxsview_plot.h"

#include <QAction>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QKeySequence>
#include <QList>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMenuBar>
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

  SaxsviewMainWindow *mw;

  // "File"-menu
  QAction *actionCreateSubWindow, *actionLoad, *actionQuit;
  QAction *actionSaveAs, *actionPrint;

  // "Plot"-menu
  QAction *actionAbsScale, *actionLogScale;
  QActionGroup *actionGroupScale;

  QAction *actionZoomIn, *actionZoomOut, *actionZoom, *actionMove;
  QActionGroup *actionGroupZoom;

  // "Window"-menu
  QAction *actionPreviousPlot, *actionNextPlot, *actionCascadePlots;
  QAction *actionTilePlots, *actionClosePlot, *actionCloseAllPlots;

  // "Help"-menu
  QAction *actionAbout;

  QMenu *menuFile, *menuPlot, *menuWindow, *menuHelp;

  QMdiArea *mdiArea;
  QSignalMapper *windowMapper;
  QSignalMapper *scaleMapper;
};

SaxsviewMainWindow::SaxsviewMainWindowPrivate::SaxsviewMainWindowPrivate(SaxsviewMainWindow *w)
 : mw(w),
   windowMapper(new QSignalMapper(mw)),
   scaleMapper(new QSignalMapper(mw)) {
}

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::setupActions() {
  QStyle *style = qApp->style();

  //
  // "File"-menu
  //
  actionCreateSubWindow = new QAction("&New Plot", mw);
  actionCreateSubWindow->setIcon(style->standardIcon(QStyle::SP_FileIcon));
  actionCreateSubWindow->setShortcut(QKeySequence::New);
  connect(actionCreateSubWindow, SIGNAL(triggered()),
          mw, SLOT(createSubWindow()));

  actionLoad = new QAction("&Open", mw);
  actionLoad->setIcon(style->standardIcon(QStyle::SP_DirIcon));
  actionLoad->setShortcut(QKeySequence::Open);
  connect(actionLoad, SIGNAL(triggered()),
          mw, SLOT(load()));

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

  actionSaveAs = new QAction("&Export", mw);
  connect(actionSaveAs, SIGNAL(triggered()),
          mw, SLOT(saveAs()));

  actionPrint = new QAction("&Print", mw);
  connect(actionPrint, SIGNAL(triggered()),
          mw, SLOT(print()));

  actionZoomIn = new QAction("Zoom &in", mw);
  connect(actionZoomIn, SIGNAL(triggered()),
          mw, SLOT(zoomIn()));

  actionZoomOut = new QAction("Zoom &out", mw);
  connect(actionZoomOut, SIGNAL(triggered()),
          mw, SLOT(zoomOut()));

  actionZoom = new QAction("&Zoom", mw);
  actionZoom->setCheckable(true);
  actionZoom->setChecked(false);
  connect(actionZoom, SIGNAL(toggled(bool)),
          mw, SLOT(setZoomEnabled(bool)));

  actionMove = new QAction("&Move", mw);
  actionMove->setCheckable(true);
  connect(actionMove, SIGNAL(toggled(bool)),
          mw, SLOT(setMoveEnabled(bool)));
  actionMove->setChecked(true);

  actionGroupZoom = new QActionGroup(mw);
  actionGroupZoom->addAction(actionZoom);
  actionGroupZoom->addAction(actionMove);

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
  QMenuBar *menuBar = mw->menuBar();

  menuFile = new QMenu("&File", mw);
  menuFile->addAction(actionCreateSubWindow);
  menuFile->addAction(actionLoad);
  menuFile->addAction(actionSaveAs);
  menuFile->addAction(actionPrint);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
  menuBar->addMenu(menuFile);

  menuPlot = new QMenu("&Plot", mw);
  menuPlot->addActions(actionGroupScale->actions());
  menuPlot->addSeparator();
  menuPlot->addAction(actionZoomIn);
  menuPlot->addAction(actionZoomOut);
  menuPlot->addActions(actionGroupZoom->actions());
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
  toolBar->addAction(actionCreateSubWindow);

  toolBar = mw->addToolBar("plot Toolbar");
  toolBar->addAction(actionLoad);
  toolBar->addAction(actionSaveAs);
  toolBar->addAction(actionPrint);
  toolBar->addAction(actionZoomIn);
  toolBar->addAction(actionZoomOut);
  toolBar->addActions(actionGroupZoom->actions());
}

void SaxsviewMainWindow::SaxsviewMainWindowPrivate::setupSignalMappers() {
  //
  // Maps the selected Window in th menu activate the
  // respective subwindow
  //
  connect(windowMapper, SIGNAL(mapped(QWidget*)),
          mw, SLOT(setActiveSubWindow(QWidget*)));

  //
  // Maps scaling-actions (abs, log10) to the scaling slot of
  // the respective plots
  //
  connect(scaleMapper, SIGNAL(mapped(int)),
          mw, SLOT(setScale(int)));
}

SaxsviewMainWindow::SaxsviewMainWindow(QWidget *parent)
 : QMainWindow(parent), p(new SaxsviewMainWindowPrivate(this)) {

  p->setupUi();
  p->setupActions();
  p->setupMenus();
  p->setupToolbars();
  p->setupSignalMappers();
}

SaxsviewMainWindow::~SaxsviewMainWindow() {
  delete p;
}

SaxsviewSubWindow* SaxsviewMainWindow::currentSubWindow() const {
  QMdiSubWindow *subWindow = p->mdiArea->currentSubWindow();
  return subWindow ? qobject_cast<SaxsviewSubWindow*>(subWindow) : 0L;
}

void SaxsviewMainWindow::createSubWindow() {
  SaxsviewSubWindow *w = new SaxsviewSubWindow(this);
  p->mdiArea->addSubWindow(w);
  w->show();
}

void SaxsviewMainWindow::load() {
  QStringList fileNames = QFileDialog::getOpenFileNames(this, "Open file ...");

  foreach (QString fileName, fileNames)
    load(fileName);
}

void SaxsviewMainWindow::load(const QString& fileName) {
//   if (!QFile::exists(fileName)) {
//     QMessageBox::critical(this, "No such file",
//                           QString("File not found: %1").arg(fileName));
//     return;
//   }

  if (!currentSubWindow())
    createSubWindow();

  if (currentSubWindow())
    currentSubWindow()->load(fileName);
}

void SaxsviewMainWindow::saveAs() {
  if (currentSubWindow())
    currentSubWindow()->saveAs();
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

void SaxsviewMainWindow::setActiveSubWindow(QWidget *w) {
  if (w)
    p->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(w));
}

void SaxsviewMainWindow::subWindowActivated(QMdiSubWindow *w) {
  //
  // Select the active scaling in p->actionGroupScale ...
  //
  if (SaxsviewSubWindow *subWindow = qobject_cast<SaxsviewSubWindow*>(w)) {
    QObject *sender = p->scaleMapper->mapping(subWindow->scale());
    if (QAction *action = qobject_cast<QAction*>(sender))
      action->setChecked(true);
  }
}

/*
 * Copyright (C) 2012 Daniel Franke <dfranke@users.sourceforge.net>
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

// svplot includes
#include "svplotfilebrowserdockwidget.h"

// libsaxsview includes
#include "saxsview_config.h"

// global includes
#include <QtGui>


class SVPlotFileBrowserDockWidget::Private {
public:
  void setupUi(SVPlotFileBrowserDockWidget *dock);

  QLineEdit *editFilter;
  QToolButton *btnParentDir, *btnHomeDir;
  QLabel *lblCurrentDirectory;

  QFileSystemModel *model;
  QListView *view;
};

void SVPlotFileBrowserDockWidget::Private::setupUi(SVPlotFileBrowserDockWidget *dock) {
  QStyle *style = qApp->style();

  editFilter = new QLineEdit(dock);
  connect(editFilter, SIGNAL(textChanged(const QString&)),
          dock, SLOT(setNameFilter(const QString&)));

  btnParentDir = new QToolButton(dock);
  btnParentDir->setIcon(style->standardIcon(QStyle::SP_FileDialogToParent));
  btnParentDir->setToolTip("Parent Directory");
  connect(btnParentDir, SIGNAL(clicked()),
          dock, SLOT(parentDir()));

  btnHomeDir = new QToolButton(dock);
  btnHomeDir->setIcon(style->standardIcon(QStyle::SP_DirHomeIcon));
  btnHomeDir->setToolTip("Home Directory");
  connect(btnHomeDir, SIGNAL(clicked()),
          dock, SLOT(homeDir()));

  model = new QFileSystemModel(dock);
  model->setReadOnly(true);
  model->setNameFilterDisables(false);   // hide items not matched by the filter

  lblCurrentDirectory = new QLabel(dock);

  view = new QListView(dock);
  view->setModel(model);
  view->setDragEnabled(true);
  connect(view, SIGNAL(doubleClicked(const QModelIndex&)),
          dock, SLOT(indexSelected(const QModelIndex&)));

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(new QLabel("Filter:"), 0, 0);
  layout->addWidget(editFilter, 0, 1);
  layout->addWidget(btnParentDir, 0, 2);
  layout->addWidget(btnHomeDir, 0, 3);
  layout->addWidget(lblCurrentDirectory, 1, 0, 1, 4);
  layout->addWidget(view, 2, 0, 1, 4);
  layout->setContentsMargins(4, 0, 4, 0);

  QWidget *w = new QWidget(dock);
  w->setLayout(layout);

  dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
  dock->setObjectName("FileBrowserDock");
  dock->indexSelected(model->index(config().recentDirectory()));
  dock->setWidget(w);
}

SVPlotFileBrowserDockWidget::SVPlotFileBrowserDockWidget(QWidget *parent)
 : QDockWidget("File System Browser", parent), p(new Private) {

  p->setupUi(this);
}

SVPlotFileBrowserDockWidget::~SVPlotFileBrowserDockWidget() {
  delete p;
}

void SVPlotFileBrowserDockWidget::setNameFilter(const QString& filter) {
  p->model->setNameFilters(QStringList() << (filter.isEmpty() ? "*" : filter));
}

void SVPlotFileBrowserDockWidget::parentDir() {
  indexSelected(p->view->rootIndex().parent());
}

void SVPlotFileBrowserDockWidget::homeDir() {
  indexSelected(p->model->index(QDir::homePath()));
}

void SVPlotFileBrowserDockWidget::indexSelected(const QModelIndex& index) {	
  if (p->model->fileInfo(index).isDir()) {
    QFontMetrics fontMetric(p->lblCurrentDirectory->font());
    QString elidedPath = fontMetric.elidedText(p->model->filePath(index),
                                               Qt::ElideLeft, p->view->width());

    p->lblCurrentDirectory->setText(elidedPath);
    p->lblCurrentDirectory->setToolTip(p->model->filePath(index));
    p->model->setRootPath(p->model->filePath(index));
    p->view->setRootIndex(index);

  } else
    emit selected(p->model->filePath(index));
}

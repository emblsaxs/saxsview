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

#include "svplotfiledockwidget.h"
#include "svplotsubwindow.h"
#include "svplotproject.h"

#include "saxsview.h"
#include "saxsview_plot.h"

#include <QtGui>


class SVPlotFileDockWidget::Private {
public:
  void setupUi(SVPlotFileDockWidget *dock);

  QTreeView *view;
};

void SVPlotFileDockWidget::Private::setupUi(SVPlotFileDockWidget *dock) {
  view = new QTreeView(dock);
  view->setHeaderHidden(true);
  view->setRootIsDecorated(false);
  view->setSelectionBehavior(QAbstractItemView::SelectItems);
  view->setSelectionMode(QAbstractItemView::SingleSelection);

  dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
  dock->setObjectName("FileDock");
  dock->setWidget(view);
}

SVPlotFileDockWidget::SVPlotFileDockWidget(QWidget *parent)
 : QDockWidget("File List", parent), p(new Private) {

  p->setupUi(this);
}

SVPlotFileDockWidget::~SVPlotFileDockWidget() {
  delete p;
}

void SVPlotFileDockWidget::subWindowActivated(QMdiSubWindow *w) {
  if (SVPlotSubWindow *sv = qobject_cast<SVPlotSubWindow*>(w)) {
    QStandardItemModel *model     = sv->project()->model();
    QItemSelectionModel *selModel = sv->project()->selectionModel();

    p->view->setModel(model);
    p->view->setSelectionModel(selModel);
    p->view->expandAll();

  } else
    p->view->setModel(0L);
}

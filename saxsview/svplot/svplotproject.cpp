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

#include "svplotproject.h"

#include "saxsview_plot.h"
#include "saxsview_plotcurve.h"


SaxsviewPlotItem::SaxsviewPlotItem(SaxsviewPlot *plot, const QString& title)
 : QStandardItem(title) {
  setEditable(false);
  setData(qVariantFromValue((void*)plot));
}

int SaxsviewPlotItem::type() const {
  return QStandardItem::UserType + 42;
}

SaxsviewPlot* SaxsviewPlotItem::plot() const {
  return (SaxsviewPlot*)qVariantValue<void*>(data());
}


SaxsviewPlotCurveItem::SaxsviewPlotCurveItem(SaxsviewPlotCurve *curve)
 : QStandardItem(curve->title()) {
  setToolTip(curve->fileName());
  setEditable(false);
  // TODO: I'd like to have the "enabled" checkbox in the curve list ...
//   setCheckable(true);
//   setCheckState(Qt::Checked);
  setData(qVariantFromValue((void*)curve));
}

int SaxsviewPlotCurveItem::type() const {
  return QStandardItem::UserType + 43;
}

SaxsviewPlotCurve* SaxsviewPlotCurveItem::curve() const {
  return (SaxsviewPlotCurve*)qVariantValue<void*>(data());
}


class SVPlotProject::Private {
public:
  Private(SVPlotProject *parent);

  QStandardItemModel *model;
  QItemSelectionModel *selectionModel;
};

SVPlotProject::Private::Private(SVPlotProject *parent) {
  model = new QStandardItemModel(parent);
  selectionModel = new QItemSelectionModel(model, parent);

  connect(selectionModel,
          SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
          parent,
          SIGNAL(currentIndexChanged(const QModelIndex&)));
}


SVPlotProject::SVPlotProject(QObject *parent)
 : QObject(parent), p(new Private(this)) {
}

SVPlotProject::~SVPlotProject() {
  delete p;
}

void SVPlotProject::addPlot(SaxsviewPlot *plot, const QString& title) {
  p->model->insertRow(p->model->rowCount(),
                      new SaxsviewPlotItem(plot, title));

  // Initially select the root node.
  p->selectionModel->setCurrentIndex(p->model->index(0, 0),
                                     QItemSelectionModel::SelectCurrent);
}

void SVPlotProject::addPlotCurve(SaxsviewPlotCurve *curve) {
  if (QStandardItem *root = p->model->item(0, 0))
    root->insertRow(root->rowCount(),
                    new SaxsviewPlotCurveItem(curve));
}

QStandardItemModel* SVPlotProject::model() {
  return p->model;
}

QItemSelectionModel* SVPlotProject::selectionModel() {
  return p->selectionModel;
}

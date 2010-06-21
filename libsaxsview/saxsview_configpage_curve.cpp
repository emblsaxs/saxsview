/*
 * Copyright (C) 2010 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of saxsview.
 *
 * saxsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * saxsview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with saxsview. If not, see <http://www.gnu.org/licenses/>.
 */

#include "saxsview_configpage_curve.h"
#include "saxsview_config.h"
#include "saxsview_plotcurve.h"
#include "saxsview_plot.h"

#include <QtGui>

namespace Saxsview {

class CurveConfigPage::CurveConfigPagePrivate {
public:
  CurveConfigPagePrivate(Plot *p)
   : plot(p),
     model(new QStandardItemModel),
     templates(new QStandardItemModel),
     mapper(new QDataWidgetMapper) {
  }

  ~CurveConfigPagePrivate() {
    delete mapper;
    delete templates;
    delete model;
  }

  Plot *plot;
  QStandardItemModel *model, *templates;
  QDataWidgetMapper *mapper;
};


CurveConfigPage::CurveConfigPage(Plot *plot, QWidget *parent)
 : AbstractConfigPage(parent), p(new CurveConfigPagePrivate(plot)) {

  setupUi(this);
  reset();

  curveList->setModel(p->model);
  curveList->setModelColumn(1);
  connect(curveList->selectionModel(),
          SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
          p->mapper, SLOT(setCurrentModelIndex(const QModelIndex&)));
  connect(p->mapper, SIGNAL(currentIndexChanged (int)),
          this, SLOT(setCurrentIndex(int)));

  p->mapper->setModel(p->model);
  p->mapper->addMapping(lblFileNameDisplay, 0, "text");
  p->mapper->addMapping(editLegendLabel, 1);
  p->mapper->addMapping(comboLineStyle, 2, "currentStyle");
  p->mapper->addMapping(spinLineWidth, 3);
  p->mapper->addMapping(btnLineColor, 4, "color");
  p->mapper->addMapping(comboSymbolStyle, 5, "currentStyle");
  p->mapper->addMapping(spinSymbolSize, 6);
  p->mapper->addMapping(btnSymbolStyleColor, 7, "color");
  p->mapper->addMapping(comboErrorBarStyle, 8, "currentStyle");
  p->mapper->addMapping(spinErrorBarWidth, 9);
  p->mapper->addMapping(btnErrorBarStyleColor, 10, "color");
  p->mapper->addMapping(spinScaleX, 11);
  p->mapper->addMapping(spinScaleY, 12);
  p->mapper->addMapping(spinEvery, 13);
  p->mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
  p->mapper->toFirst();

  config().curveTemplates(p->templates);
  comboTemplates->setModel(p->templates);

  connect(btnApplyTemplate, SIGNAL(clicked()),
          this, SLOT(applyTemplate()));
}

CurveConfigPage::~CurveConfigPage() {
  delete p;
}

void CurveConfigPage::apply() {
  QList<PlotCurve*> curve = p->plot->curves();

  for (int i = 0; i < p->model->rowCount(); ++i) {
    // ignore file name (index 0), it does not change
    curve[i]->setTitle(p->model->item(i, 1)->text());
    curve[i]->setVisible(p->model->item(i, 1)->checkState() == Qt::Checked);

    QPen line;
    line.setStyle((Qt::PenStyle)p->model->item(i, 2)->text().toInt());
    line.setWidth(p->model->item(i, 3)->text().toInt());
    line.setColor(QColor(p->model->item(i, 4)->text()));
    curve[i]->setPen(line);

    PlotSymbol symbol;
    symbol.setStyle((PlotSymbol::Style)p->model->item(i, 5)->text().toInt());
    symbol.setSize(p->model->item(i, 6)->text().toInt());
    symbol.setColor(QColor(p->model->item(i, 7)->text()));
    curve[i]->setSymbol(symbol);

    QPen errors;
    errors.setStyle((Qt::PenStyle)p->model->item(i, 8)->text().toInt());
    errors.setWidth(p->model->item(i, 9)->text().toInt());
    errors.setColor(QColor(p->model->item(i, 10)->text()));
    curve[i]->setErrorBarPen(errors);

    curve[i]->setScalingFactorX(p->model->item(i, 11)->text().toDouble());
    curve[i]->setScalingFactorY(p->model->item(i, 12)->text().toDouble());
    curve[i]->setEvery(p->model->item(i, 13)->text().toInt());
  }
}

void CurveConfigPage::reset() {
  foreach (PlotCurve *curve, p->plot->curves()) {
    QList<QStandardItem*> row;
    row.push_back(new QStandardItem(QFileInfo(curve->fileName()).fileName()));
    row.push_back(new QStandardItem(curve->title()));
    row[1]->setCheckable(true);
    row[1]->setCheckState(curve->isVisible() ? Qt::Checked : Qt::Unchecked);

    QPen line = curve->pen();
    row.push_back(new QStandardItem(QString::number(line.style())));
    row.push_back(new QStandardItem(QString::number(line.width())));
    row.push_back(new QStandardItem(line.color().name()));

    PlotSymbol symbol = curve->symbol();
    row.push_back(new QStandardItem(QString::number(symbol.style())));
    row.push_back(new QStandardItem(QString::number(symbol.size())));
    row.push_back(new QStandardItem(symbol.color().name()));

    QPen errors = curve->errorBarPen();
    row.push_back(new QStandardItem(QString::number(errors.style())));
    row.push_back(new QStandardItem(QString::number(errors.width())));
    row.push_back(new QStandardItem(errors.color().name()));

    row.push_back(new QStandardItem(QString::number(curve->scalingFactorX())));
    row.push_back(new QStandardItem(QString::number(curve->scalingFactorY())));
    row.push_back(new QStandardItem(QString::number(curve->every())));

    p->model->appendRow(row);
  }
}

void CurveConfigPage::applyTemplate() {
  const int srcRow = comboTemplates->currentIndex();
  const int destRow = p->mapper->currentIndex();

  p->model->item(destRow, 2)->setText(p->templates->item(srcRow, 1)->text());
  p->model->item(destRow, 3)->setText(p->templates->item(srcRow, 2)->text());
  p->model->item(destRow, 5)->setText(p->templates->item(srcRow, 3)->text());
  p->model->item(destRow, 6)->setText(p->templates->item(srcRow, 4)->text());
  p->model->item(destRow, 8)->setText(p->templates->item(srcRow, 5)->text());
  p->model->item(destRow, 9)->setText(p->templates->item(srcRow, 6)->text());
}

void CurveConfigPage::setCurrentIndex(int i) {
  QItemSelectionModel *selModel = curveList->selectionModel();
  selModel->setCurrentIndex(p->model->index(i, curveList->modelColumn()),
                            QItemSelectionModel::ClearAndSelect);
}

} // end of namespace Saxsview

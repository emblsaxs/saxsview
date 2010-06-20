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

#include "saxsview_config.h"
#include "saxsview_configpage.h"
#include "saxsview_configpage_curvetemplate.h"

#include "saxsdocument.h"

#include <QtGui>

namespace Saxsview {

enum {
  ColumnName = 0,
  ColumnLineStyle,      // stores currentStyle() of the combobox
  ColumnLineWidth,
  ColumnSymbolStyle,    // likewise
  ColumnSymbolSize,
  ColumnErrorBarStyle,  // likewise
  ColumnErrorBarWidth,
};


class CurveTemplateConfigPage::CurveTemplateConfigPagePrivate {
public:
  CurveTemplateConfigPagePrivate()
   : model(new QStandardItemModel),
     mapper(new QDataWidgetMapper) {
  }

  ~CurveTemplateConfigPagePrivate() {
    delete mapper;
    delete model;
  }

  QStandardItemModel *model;
  QDataWidgetMapper *mapper;
};


CurveTemplateConfigPage::CurveTemplateConfigPage(QWidget *parent)
 : AbstractConfigPage(parent), p(new CurveTemplateConfigPagePrivate) {

  setupUi(this);

  templateList->setModel(p->model);
  connect(templateList->selectionModel(),
          SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
          p->mapper, SLOT(setCurrentModelIndex(const QModelIndex&)));
  connect(p->mapper, SIGNAL(currentIndexChanged (int)),
          this, SLOT(setCurrentIndex(int)));

  comboExperimentalCurveTemplate->setModel(p->model);
  comboTheoreticalCurveTemplate->setModel(p->model);
  comboProbabilityCurveTemplate->setModel(p->model);

  reset();

  p->mapper->setModel(p->model);
  p->mapper->addMapping(editName, ColumnName);
  p->mapper->addMapping(comboLineStyle, ColumnLineStyle, "currentStyle");
  p->mapper->addMapping(spinLineWidth, ColumnLineWidth);
  p->mapper->addMapping(comboSymbolStyle, ColumnSymbolStyle, "currentStyle");
  p->mapper->addMapping(spinSymbolSize, ColumnSymbolSize);
  p->mapper->addMapping(comboErrorBarStyle, ColumnErrorBarStyle, "currentStyle");
  p->mapper->addMapping(spinErrorBarWidth, ColumnErrorBarWidth);
  p->mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
  p->mapper->toFirst();

  connect(btnAdd, SIGNAL(clicked()),
          this, SLOT(addTemplate()));
  connect(btnRemove, SIGNAL(clicked()),
          this, SLOT(removeTemplate()));
}

CurveTemplateConfigPage::~CurveTemplateConfigPage() {
  delete p;
}

void CurveTemplateConfigPage::apply() {
  config().setCurveTemplates(p->model);

  config().setCurrentCurveTemplate(SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA,
                                   comboExperimentalCurveTemplate->currentIndex());
  config().setCurrentCurveTemplate(SAXS_CURVE_THEORETICAL_SCATTERING_DATA,
                                   comboTheoreticalCurveTemplate->currentIndex());
  config().setCurrentCurveTemplate(SAXS_CURVE_PROBABILITY_DATA,
                                   comboProbabilityCurveTemplate->currentIndex());
}

void CurveTemplateConfigPage::reset() {
  config().curveTemplates(p->model);

  if (p->model->rowCount() > 0) {
    int index;
    index = config().currentCurveTemplate(SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
    comboExperimentalCurveTemplate->setCurrentIndex(index);

    index = config().currentCurveTemplate(SAXS_CURVE_THEORETICAL_SCATTERING_DATA);
    comboTheoreticalCurveTemplate->setCurrentIndex(index);

    index = config().currentCurveTemplate(SAXS_CURVE_PROBABILITY_DATA);
    comboProbabilityCurveTemplate->setCurrentIndex(index);

  } else
    defaults();

  setEditorEnabled(p->model->rowCount() > 0);
}

void CurveTemplateConfigPage::defaults() {
  QList<QStandardItem*> t1, t2;

  t1.push_back(new QStandardItem("open circles w/ errors"));
  t1.push_back(new QStandardItem(QString::number(Qt::NoPen)));            // line style
  t1.push_back(new QStandardItem("1"));                                   // line width
  t1.push_back(new QStandardItem(QString::number(PlotSymbol::Ellipse)));  // symbol style
  t1.push_back(new QStandardItem("4"));                                   // symbol size
  t1.push_back(new QStandardItem(QString::number(Qt::SolidLine)));        // error bar style
  t1.push_back(new QStandardItem("1"));                                   // error bar width
  p->model->appendRow(t1);

  t2.push_back(new QStandardItem("solid line w/o errors"));
  t2.push_back(new QStandardItem(QString::number(Qt::SolidLine)));
  t2.push_back(new QStandardItem("2"));
  t2.push_back(new QStandardItem(QString::number(PlotSymbol::NoSymbol)));
  t2.push_back(new QStandardItem("1"));
  t2.push_back(new QStandardItem(QString::number(Qt::NoPen)));
  t2.push_back(new QStandardItem("1"));
  p->model->appendRow(t2);

  comboExperimentalCurveTemplate->setCurrentIndex(0);
  comboTheoreticalCurveTemplate->setCurrentIndex(1);
  comboProbabilityCurveTemplate->setCurrentIndex(1);
}

void CurveTemplateConfigPage::addTemplate() {
  QList<QStandardItem*> row;

  row.push_back(new QStandardItem("new template"));
  row.push_back(new QStandardItem(QString::number(Qt::NoPen)));
  row.push_back(new QStandardItem("1"));
  row.push_back(new QStandardItem(QString::number(PlotSymbol::NoSymbol)));
  row.push_back(new QStandardItem("1"));
  row.push_back(new QStandardItem(QString::number(Qt::NoPen)));
  row.push_back(new QStandardItem("1"));

  p->model->appendRow(row);
  p->mapper->toLast();

  setEditorEnabled(p->model->rowCount() > 0);
}

void CurveTemplateConfigPage::removeTemplate() {
  QString msg = "Shall template '%1' really be removed?";

  if (QMessageBox::question(this, "Please confirm",
                            msg.arg(editName->text()),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
    p->model->removeRow(p->mapper->currentIndex());

  setEditorEnabled(p->model->rowCount() > 0);
}

void CurveTemplateConfigPage::setEditorEnabled(bool on) {
  btnRemove->setEnabled(on);

  editName->setEnabled(on);
  comboLineStyle->setEnabled(on);
  spinLineWidth->setEnabled(on);
  comboSymbolStyle->setEnabled(on);
  spinSymbolSize->setEnabled(on);
  comboErrorBarStyle->setEnabled(on);
  spinErrorBarWidth->setEnabled(on);

  if (!on) {
    editName->setText("");
    comboLineStyle->setCurrentStyle(Qt::NoPen);
    spinLineWidth->setValue(1);
    comboSymbolStyle->setCurrentStyle(PlotSymbol::NoSymbol);
    spinSymbolSize->setValue(1);
    comboErrorBarStyle->setCurrentStyle(Qt::NoPen);
    spinErrorBarWidth->setValue(1);
  }
}

void CurveTemplateConfigPage::setCurrentIndex(int i) {
  QItemSelectionModel *selModel = templateList->selectionModel();
  selModel->setCurrentIndex(p->model->index(i, templateList->modelColumn()),
                            QItemSelectionModel::ClearAndSelect);
}

} // end of namespace Saxsview

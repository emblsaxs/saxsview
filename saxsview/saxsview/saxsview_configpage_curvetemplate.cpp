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
   : templateModel(new QStandardItemModel),
     templateMapper(new QDataWidgetMapper),
     fileTypeModel(new QStandardItemModel),
     fileTypeMapper(new QDataWidgetMapper) {
  }

  ~CurveTemplateConfigPagePrivate() {
    delete templateMapper;
    delete templateModel;
    delete fileTypeModel;
    delete fileTypeMapper;
  }

  QStandardItemModel *templateModel;
  QDataWidgetMapper *templateMapper;

  QStandardItemModel *fileTypeModel;
  QDataWidgetMapper *fileTypeMapper;
};


CurveTemplateConfigPage::CurveTemplateConfigPage(QWidget *parent)
 : AbstractConfigPage(parent), p(new CurveTemplateConfigPagePrivate) {

  setupUi(this);

  templateList->setModel(p->templateModel);
  connect(templateList->selectionModel(),
          SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
          p->templateMapper, SLOT(setCurrentModelIndex(const QModelIndex&)));
  connect(p->templateMapper, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setCurrentTemplateIndex(int)));

  connect(btnAdd, SIGNAL(clicked()),
          this, SLOT(addTemplate()));
  connect(btnRemove, SIGNAL(clicked()),
          this, SLOT(removeTemplate()));

  fileTypeList->setModel(p->fileTypeModel);
  connect(fileTypeList->selectionModel(),
          SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
          p->fileTypeMapper, SLOT(setCurrentModelIndex(const QModelIndex&)));
  connect(p->fileTypeMapper, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setCurrentFileTypeIndex(int)));

  reset();

  p->templateMapper->setModel(p->templateModel);
  p->templateMapper->addMapping(editName, ColumnName);
  p->templateMapper->addMapping(comboLineStyle, ColumnLineStyle, "currentStyle");
  p->templateMapper->addMapping(spinLineWidth, ColumnLineWidth);
  p->templateMapper->addMapping(comboSymbolStyle, ColumnSymbolStyle, "currentStyle");
  p->templateMapper->addMapping(spinSymbolSize, ColumnSymbolSize);
  p->templateMapper->addMapping(comboErrorBarStyle, ColumnErrorBarStyle, "currentStyle");
  p->templateMapper->addMapping(spinErrorBarWidth, ColumnErrorBarWidth);
  p->templateMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
  p->templateMapper->toFirst();

  comboExperimentalCurveTemplate->setModel(p->templateModel);
  comboTheoreticalCurveTemplate->setModel(p->templateModel);
  comboProbabilityCurveTemplate->setModel(p->templateModel);

  p->fileTypeMapper->setModel(p->fileTypeModel);
  p->fileTypeMapper->addMapping(comboExperimentalCurveTemplate, 1, "currentIndex");
  p->fileTypeMapper->addMapping(comboTheoreticalCurveTemplate, 2, "currentIndex");
  p->fileTypeMapper->addMapping(comboProbabilityCurveTemplate, 3, "currentIndex");
  p->fileTypeMapper->toFirst();
}

CurveTemplateConfigPage::~CurveTemplateConfigPage() {
  delete p;
}

void CurveTemplateConfigPage::apply() {
  config().setCurveTemplates(p->templateModel);
  config().setFileTypeTemplates(p->fileTypeModel);
}

void CurveTemplateConfigPage::reset() {
  config().curveTemplates(p->templateModel);
  config().fileTypeTemplates(p->fileTypeModel);

  setEditorEnabled(p->templateModel->rowCount() > 0);
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

  p->templateModel->appendRow(row);
  p->templateMapper->toLast();

  setEditorEnabled(p->templateModel->rowCount() > 0);
}

void CurveTemplateConfigPage::removeTemplate() {
  QString msg = "Shall template '%1' really be removed?";

  if (QMessageBox::question(this, "Please confirm",
                            msg.arg(editName->text()),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
    p->templateModel->removeRow(p->templateMapper->currentIndex());

  setEditorEnabled(p->templateModel->rowCount() > 0);
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
  groupTemplatesByFileType->setEnabled(on);

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

void CurveTemplateConfigPage::setCurrentTemplateIndex(int i) {
  QItemSelectionModel *selModel = templateList->selectionModel();
  selModel->setCurrentIndex(p->templateModel->index(i, templateList->modelColumn()),
                            QItemSelectionModel::ClearAndSelect);
}

void CurveTemplateConfigPage::setCurrentFileTypeIndex(int i) {
  QItemSelectionModel *selModel = fileTypeList->selectionModel();
  selModel->setCurrentIndex(p->fileTypeModel->index(i, fileTypeList->modelColumn()),
                            QItemSelectionModel::ClearAndSelect);
}

} // end of namespace Saxsview

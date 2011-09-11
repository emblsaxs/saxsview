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

#include "saxsview_configdialog.h"
#include "saxsview_configpage.h"
#include "saxsview_configpage_curve.h"
#include "saxsview_configpage_curvetemplate.h"
#include "saxsview_configpage_defaultcolors.h"

#include <QtGui>

namespace Saxsview {

//-------------------------------------------------------------------------
class AbstractConfigDialog::ConfigDialogPrivate {
public:
  ConfigDialogPrivate();

  void setupUi(AbstractConfigDialog *dlg);

  void apply();
  void reset();

  QTabWidget *tab;
  QDialogButtonBox *buttonBox;

  QList<AbstractConfigPage*> configPages;
};

AbstractConfigDialog::ConfigDialogPrivate::ConfigDialogPrivate()
 : tab(new QTabWidget),
   buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok
                                   | QDialogButtonBox::Apply
                                   | QDialogButtonBox::Reset
                                   | QDialogButtonBox::Cancel,
                                  Qt::Horizontal)) {
}

void AbstractConfigDialog::ConfigDialogPrivate::setupUi(AbstractConfigDialog *dlg) {
  QVBoxLayout *verticalLayout = new QVBoxLayout;
  verticalLayout->addWidget(tab);
  verticalLayout->addWidget(buttonBox);

  dlg->setLayout(verticalLayout);
  dlg->setWindowTitle("");

  connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
          dlg, SLOT(buttonClicked(QAbstractButton*)));
}

void AbstractConfigDialog::ConfigDialogPrivate::apply() {
  foreach (AbstractConfigPage *page, configPages)
    page->apply();
}

void AbstractConfigDialog::ConfigDialogPrivate::reset() {
  foreach (AbstractConfigPage *page, configPages)
    page->reset();
}


//-------------------------------------------------------------------------
AbstractConfigDialog::AbstractConfigDialog(QWidget *parent)
 : QDialog(parent), p(new ConfigDialogPrivate) {
  p->setupUi(this);
}

AbstractConfigDialog::~AbstractConfigDialog() {
  delete p;
}

void AbstractConfigDialog::addConfigPage(AbstractConfigPage *page,
                                         const QString& title) {
  p->configPages.append(page);
  p->tab->addTab(page, title);
}

void AbstractConfigDialog::buttonClicked(QAbstractButton *button) {
  setCursor(Qt::WaitCursor);

  switch (p->buttonBox->standardButton(button)) {
    case QDialogButtonBox::Ok:
      p->apply();
      accept();
      break;

    case QDialogButtonBox::Apply:
      p->apply();
      break;

    case QDialogButtonBox::Reset:
      p->reset();
      break;

    case QDialogButtonBox::Cancel:
    default:
      reject();
      break;
  }

  unsetCursor();
}

//-------------------------------------------------------------------------
SaxsviewConfigDialog::SaxsviewConfigDialog(QWidget *parent) 
 : AbstractConfigDialog(parent) {

  addConfigPage(new CurveTemplateConfigPage(this), "Curve Templates");
  addConfigPage(new DefaultColorsConfigPage(this), "Default Colors");
}

//-------------------------------------------------------------------------
PlotConfigDialog::PlotConfigDialog(Plot *plot, QWidget *parent) 
 : AbstractConfigDialog(parent) {

  addConfigPage(new PlotConfigPage(plot, this), "Plot");
  addConfigPage(new CurveConfigPage(plot, this), "Curve");
  addConfigPage(new LegendConfigPage(plot, this), "Legend");
}

} // end of namespace Saxsview

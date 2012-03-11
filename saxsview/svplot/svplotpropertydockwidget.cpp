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

#include "svplotpropertydockwidget.h"
#include "svplotsubwindow.h"
#include "svplotproject.h"

#include "saxsview.h"
#include "saxsview_plot.h"
#include "saxsview_plotcurve.h"
#include "saxsview_property.h"

#include <QtTreePropertyBrowser>

#include <QtGui>


class SVPlotPropertyDockWidget::Private {
public:
  ~Private();

  void setupUi(SVPlotPropertyDockWidget *dock);
  void setupPlotProperties(QtTreePropertyBrowser*);
  void setupCurveProperties(QtTreePropertyBrowser*);

  void hideProperties();
  void showPlotProperties();
  void showCurveProperties();

  QStandardItemModel *model;

  QStackedWidget *browserStack;

  QList<SaxsviewProperty*> groups;
  QList<SaxsviewProperty*> plotProperties, curveProperties;
};

SVPlotPropertyDockWidget::Private::~Private() {
  qDeleteAll(plotProperties.begin(), plotProperties.end());
  plotProperties.clear();

  qDeleteAll(curveProperties.begin(), curveProperties.end());
  curveProperties.clear();

  qDeleteAll(groups.begin(), groups.end());
  groups.clear();
}

void SVPlotPropertyDockWidget::Private::setupUi(SVPlotPropertyDockWidget *dock) {
  QtTreePropertyBrowser *plotBrowser = new QtTreePropertyBrowser(dock);
  plotBrowser->setRootIsDecorated(false);
  setupPlotProperties(plotBrowser);

  QtTreePropertyBrowser *curveBrowser = new QtTreePropertyBrowser(dock);
  curveBrowser->setRootIsDecorated(false);
  setupCurveProperties(curveBrowser);

  browserStack = new QStackedWidget(dock);
  browserStack->addWidget(plotBrowser);
  browserStack->addWidget(curveBrowser);

  dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
  dock->setObjectName("PropertyDock");
  dock->setWidget(browserStack);

  hideProperties();
}

void SVPlotPropertyDockWidget::Private::setupPlotProperties(QtTreePropertyBrowser *browser) {
  SaxsviewProperty *plotGroup = new SaxsviewProperty("Plot", browser);
  plotProperties.append(new SaxsviewProperty("Scale", "scale",
                                             browser, plotGroup));
  groups.append(plotGroup);

  SaxsviewProperty *titleGroup = new SaxsviewProperty("Title", browser);
  plotProperties.append(new SaxsviewProperty("Text", "plotTitle",
                                             browser, titleGroup));
  plotProperties.append(new SaxsviewProperty("Font", "plotTitleFont",
                                             browser, titleGroup));
  groups.append(titleGroup);

  SaxsviewProperty *axisGroup = new SaxsviewProperty("Axis", browser);
  plotProperties.append(new SaxsviewProperty("X Text", "axisTitleX",
                                             browser, axisGroup));
  plotProperties.append(new SaxsviewProperty("Y Text", "axisTitleY",
                                             browser, axisGroup));
  plotProperties.append(new SaxsviewProperty("Font", "axisTitleFont",
                                             browser, axisGroup));
  groups.append(axisGroup);

  SaxsviewProperty *ticksGroup = new SaxsviewProperty("Ticks", browser);
  plotProperties.append(new SaxsviewProperty("X Ticks Enabled", "ticksEnabledX",
                                             browser, ticksGroup));
  plotProperties.append(new SaxsviewProperty("Y Ticks Enabled", "ticksEnabledY",
                                             browser, ticksGroup));
  plotProperties.append(new SaxsviewProperty("Font", "ticksFont",
                                             browser, ticksGroup));
  groups.append(ticksGroup);

  SaxsviewProperty *legendGroup = new SaxsviewProperty("Legend", browser);
  plotProperties.append(new SaxsviewProperty("Visible", "legendVisible",
                                             browser, legendGroup));
  plotProperties.append(new SaxsviewProperty("Position", "legendPosition",
                                             browser, legendGroup));
  plotProperties.append(new SaxsviewProperty("Columns", "legendColumnsCount",
                                             browser, legendGroup));
  plotProperties.append(new SaxsviewProperty("Spacing", "legendSpacing",
                                             browser, legendGroup));
  plotProperties.append(new SaxsviewProperty("Margin", "legendMargin",
                                             browser, legendGroup));
  plotProperties.append(new SaxsviewProperty("Font", "legendFont",
                                             browser, legendGroup));
  groups.append(legendGroup);

  // FIXME: not ideal, but breaking up above's layout is messy ...
  plotProperties[11]->setMinimum(1);   // legend columns
  plotProperties[12]->setMinimum(0);   // legend spacing
  plotProperties[13]->setMinimum(0);   // legend margin
}

void SVPlotPropertyDockWidget::Private::setupCurveProperties(QtTreePropertyBrowser *browser) {
  SaxsviewProperty *curveGroup = new SaxsviewProperty("Curve", browser);
  curveProperties.append(new SaxsviewProperty("Visible", "curveVisible",
                                              browser, curveGroup));
  curveProperties.append(new SaxsviewProperty("Title", "curveTitle",
                                              browser, curveGroup));
  groups.append(curveGroup);

  SaxsviewProperty *lineGroup = new SaxsviewProperty("Line", browser);
  curveProperties.append(new SaxsviewProperty("Style", "lineStyle",
                                              browser, lineGroup));
  curveProperties.append(new SaxsviewProperty("Width", "lineWidth",
                                              browser, lineGroup));
  curveProperties.append(new SaxsviewProperty("Color", "lineColor",
                                              browser, lineGroup));
  groups.append(lineGroup);

  SaxsviewProperty *symbolGroup = new SaxsviewProperty("Symbol", browser);
  curveProperties.append(new SaxsviewProperty("Style", "symbolStyle",
                                              browser, symbolGroup));
  curveProperties.append(new SaxsviewProperty("Size", "symbolSize",
                                              browser, symbolGroup));
  curveProperties.append(new SaxsviewProperty("Filled", "isSymbolFilled",
                                              browser, symbolGroup));
  curveProperties.append(new SaxsviewProperty("Color", "symbolColor",
                                              browser, symbolGroup));
  groups.append(symbolGroup);

  SaxsviewProperty *errorGroup = new SaxsviewProperty("Error", browser);
  curveProperties.append(new SaxsviewProperty("Style", "errorLineStyle",
                                              browser, errorGroup));
  curveProperties.append(new SaxsviewProperty("Width", "errorLineWidth",
                                              browser, errorGroup));
  curveProperties.append(new SaxsviewProperty("Color", "errorLineColor",
                                              browser, errorGroup));
  groups.append(errorGroup);

  SaxsviewProperty *transformGroup = new SaxsviewProperty("Transformation",
                                                          browser);
  curveProperties.append(new SaxsviewProperty("Scaling X", "scalingFactorX",
                                              browser, transformGroup));
  curveProperties.append(new SaxsviewProperty("Scaling Y", "scalingFactorY",
                                              browser, transformGroup));
  curveProperties.append(new SaxsviewProperty("Merge", "merge",
                                              browser, transformGroup));
  groups.append(transformGroup);

  curveProperties[ 3]->setMinimum(1);    // line width
  curveProperties[ 6]->setMinimum(1);    // symbol size
  curveProperties[10]->setMinimum(1);    // error bar width
  curveProperties[12]->setMinimum(0.01); // scaling factor x
  curveProperties[13]->setMinimum(0.01); // scaling factor y
  curveProperties[14]->setMinimum(1);    // merge
}

void SVPlotPropertyDockWidget::Private::hideProperties() {
  browserStack->setEnabled(false);
}

void SVPlotPropertyDockWidget::Private::showPlotProperties() {
  browserStack->setEnabled(true);
  browserStack->setCurrentIndex(0);
}

void SVPlotPropertyDockWidget::Private::showCurveProperties() {
  browserStack->setEnabled(true);
  browserStack->setCurrentIndex(1);
}




SVPlotPropertyDockWidget::SVPlotPropertyDockWidget(QWidget *parent)
 : QDockWidget("Property Editor", parent), p(new Private) {

  p->setupUi(this);
}

SVPlotPropertyDockWidget::~SVPlotPropertyDockWidget() {
  delete p;
}

void SVPlotPropertyDockWidget::currentIndexChanged(const QModelIndex& index) {
  QStandardItem *item = p->model->itemFromIndex(index);

  if (SaxsviewPlotItem *plotItem = dynamic_cast<SaxsviewPlotItem*>(item)) {
    //
    // First set up the values, then show the updated page to reduce flicker
    // when a page is set up the first time.
    //
    foreach (SaxsviewProperty *property, p->plotProperties)
      property->setValue(plotItem->plot());
    p->showPlotProperties();

  } else if (SaxsviewPlotCurveItem *curveItem = dynamic_cast<SaxsviewPlotCurveItem*>(item)) {
    foreach (SaxsviewProperty *property, p->curveProperties)
      property->setValue(curveItem->curve());
    p->showCurveProperties();

  } else
    p->hideProperties();
}

void SVPlotPropertyDockWidget::subWindowActivated(QMdiSubWindow *w) {
  if (SVPlotSubWindow *sv = qobject_cast<SVPlotSubWindow*>(w)) {
    p->model = sv->project()->model();

    currentIndexChanged(sv->project()->selectionModel()->currentIndex());
  }
}

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
  void setupUi(SVPlotPropertyDockWidget *dock);
  void setupPlotProperties();
  void setupCurveProperties();
  void clear();

  QStandardItemModel *model;

  QtTreePropertyBrowser *browser;
  QList<SaxsviewProperty*> groups;
  QList<SaxsviewProperty*> properties;
};

void SVPlotPropertyDockWidget::Private::setupUi(SVPlotPropertyDockWidget *dock) {
  browser = new QtTreePropertyBrowser(dock);
  browser->setRootIsDecorated(false);

  dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
  dock->setWidget(browser);
}

void SVPlotPropertyDockWidget::Private::setupPlotProperties() {
  clear();

  SaxsviewProperty *plotGroup = new SaxsviewProperty("Plot", browser);
  properties.append(new SaxsviewProperty("Scale", "scale",
                                         browser, plotGroup));
  groups.append(plotGroup);

  SaxsviewProperty *titleGroup = new SaxsviewProperty("Title", browser);
  properties.append(new SaxsviewProperty("Text", "plotTitle",
                                         browser, titleGroup));
  properties.append(new SaxsviewProperty("Font", "plotTitleFont",
                                         browser, titleGroup));
  groups.append(titleGroup);

  SaxsviewProperty *axisGroup = new SaxsviewProperty("Axis", browser);
  properties.append(new SaxsviewProperty("X Text", "axisTitleX",
                                         browser, axisGroup));
  properties.append(new SaxsviewProperty("Y Text", "axisTitleY",
                                         browser, axisGroup));
  properties.append(new SaxsviewProperty("Font", "axisTitleFont",
                                         browser, axisGroup));
  groups.append(axisGroup);

  SaxsviewProperty *ticksGroup = new SaxsviewProperty("Ticks", browser);
  properties.append(new SaxsviewProperty("X Ticks Enabled", "ticksEnabledX",
                                         browser, ticksGroup));
  properties.append(new SaxsviewProperty("Y Ticks Enabled", "ticksEnabledY",
                                         browser, ticksGroup));
  properties.append(new SaxsviewProperty("Font", "ticksFont",
                                         browser, ticksGroup));
  groups.append(ticksGroup);

  SaxsviewProperty *legendGroup = new SaxsviewProperty("Legend", browser);
  properties.append(new SaxsviewProperty("Enabled", "legendEnabled",
                                         browser, legendGroup));
  properties.append(new SaxsviewProperty("Position", "legendPosition",
                                         browser, legendGroup));
  properties.append(new SaxsviewProperty("Columns", "legendColumnsCount",
                                         browser, legendGroup));
  properties.append(new SaxsviewProperty("Spacing", "legendSpacing",
                                         browser, legendGroup));
  properties.append(new SaxsviewProperty("Margin", "legendMargin",
                                         browser, legendGroup));
  properties.append(new SaxsviewProperty("Font", "legendFont",
                                         browser, legendGroup));
  groups.append(legendGroup);
}

void SVPlotPropertyDockWidget::Private::setupCurveProperties() {
  clear();

  SaxsviewProperty *curveGroup = new SaxsviewProperty("Curve", browser);
  properties.append(new SaxsviewProperty("Enabled", "curveEnabled",
                                              browser, curveGroup));
  properties.append(new SaxsviewProperty("Title", "curveTitle",
                                              browser, curveGroup));
  groups.append(curveGroup);

  SaxsviewProperty *lineGroup = new SaxsviewProperty("Line", browser);
  properties.append(new SaxsviewProperty("Style", "lineStyle",
                                              browser, lineGroup));
  properties.append(new SaxsviewProperty("Width", "lineWidth",
                                              browser, lineGroup));
  properties.append(new SaxsviewProperty("Color", "lineColor",
                                              browser, lineGroup));
  groups.append(lineGroup);

  SaxsviewProperty *symbolGroup = new SaxsviewProperty("Symbol", browser);
  properties.append(new SaxsviewProperty("Style", "symbolStyle",
                                              browser, symbolGroup));
  properties.append(new SaxsviewProperty("Width", "symbolSize",
                                              browser, symbolGroup));
  properties.append(new SaxsviewProperty("Filled", "isSymbolFilled",
                                              browser, symbolGroup));
  properties.append(new SaxsviewProperty("Color", "symbolColor",
                                              browser, symbolGroup));
  groups.append(symbolGroup);

  SaxsviewProperty *errorGroup = new SaxsviewProperty("Error", browser);
  properties.append(new SaxsviewProperty("Style", "errorLineStyle",
                                              browser, errorGroup));
  properties.append(new SaxsviewProperty("Width", "errorLineWidth",
                                              browser, errorGroup));
  properties.append(new SaxsviewProperty("Color", "errorLineColor",
                                              browser, errorGroup));
  groups.append(errorGroup);

  SaxsviewProperty *transformGroup = new SaxsviewProperty("Transformation",
                                                          browser);
  properties.append(new SaxsviewProperty("Scaling X", "scalingFactorX",
                                              browser, transformGroup));
  properties.append(new SaxsviewProperty("Scaling Y", "scalingFactorY",
                                              browser, transformGroup));
  properties.append(new SaxsviewProperty("Merge", "merge",
                                              browser, transformGroup));
  groups.append(transformGroup);
}


void SVPlotPropertyDockWidget::Private::clear() {
  browser->clear();

  qDeleteAll(properties.begin(), properties.end());
  properties.clear();

  qDeleteAll(groups.begin(), groups.end());
  groups.clear();
}


SVPlotPropertyDockWidget::SVPlotPropertyDockWidget(QWidget *parent)
 : QDockWidget("Property Editor", parent), p(new Private) {

  p->setupUi(this);
}

SVPlotPropertyDockWidget::~SVPlotPropertyDockWidget() {
  p->clear();
  delete p;
}

void SVPlotPropertyDockWidget::currentIndexChanged(const QModelIndex& index) {
  QStandardItem *item = p->model->itemFromIndex(index);

  if (SaxsviewPlotItem *plotItem = dynamic_cast<SaxsviewPlotItem*>(item)) {
    p->setupPlotProperties();
    foreach (SaxsviewProperty *property, p->properties)
      property->setValue(plotItem->plot());

  } else if (SaxsviewPlotCurveItem *curveItem = dynamic_cast<SaxsviewPlotCurveItem*>(item)) {
    p->setupCurveProperties();
    foreach (SaxsviewProperty *property, p->properties)
      property->setValue(curveItem->curve());
  }
}

void SVPlotPropertyDockWidget::subWindowActivated(QMdiSubWindow *w) {
  p->clear();

  if (SVPlotSubWindow *sv = qobject_cast<SVPlotSubWindow*>(w)) {
    p->model = sv->project()->model();

    currentIndexChanged(sv->project()->selectionModel()->currentIndex());
  }
}

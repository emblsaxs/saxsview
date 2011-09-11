/*
 * Copyright (C) 2011 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "svimagepropertydockwidget.h"
#include "svimageproperty.h"
#include "svimagesubwindow.h"

#include <QtTreePropertyBrowser>

#include <QtGui>

class SVImagePropertyDockWidget::SVImagePropertyDockWidgetPrivate {
public:
  SVImagePropertyDockWidgetPrivate(SVImagePropertyDockWidget *dock) {
    browser = new QtTreePropertyBrowser(dock);
    browser->setRootIsDecorated(false);
    browser->setEnabled(false);

    fileName          = new ReadOnlyProperty(browser, "Filename", QString());
    width             = new ReadOnlyProperty(browser, "Width", 0);
    height            = new ReadOnlyProperty(browser, "Height", 0);
    aspectRatioFixed  = new ReadWriteProperty(browser, "Fix Aspect Ratio", false);
    scale             = new ReadWriteProperty(browser, "Scale",
                                              QStringList() << "Absolute"
                                                            << "Logarithmic");
    lowerThreshold    = new ReadWriteProperty(browser, "Lower Threshold",
                                              0, 0, 1e9, 1);
    upperThreshold    = new ReadWriteProperty(browser, "Upper Threshold",
                                              0, 0, 1e9, 1);

    group1 = new PropertyGroup(browser, "Image");
    group1->addSubProperty(fileName);
    group1->addSubProperty(width);
    group1->addSubProperty(height);
    group1->addSubProperty(aspectRatioFixed);

    group2 = new PropertyGroup(browser, "Z Scaling");
    group2->addSubProperty(scale);
    group2->addSubProperty(lowerThreshold);
    group2->addSubProperty(upperThreshold);

    connect(aspectRatioFixed, SIGNAL(valueChanged(const QVariant&)),
            dock, SLOT(aspectRatioChanged(const QVariant&)));
    connect(scale, SIGNAL(valueChanged(const QVariant&)),
            dock, SLOT(scaleChanged(const QVariant&)));
    connect(lowerThreshold, SIGNAL(valueChanged(const QVariant&)),
            dock, SLOT(lowerThresholdChanged(const QVariant&)));
    connect(upperThreshold, SIGNAL(valueChanged(const QVariant&)),
            dock, SLOT(upperThresholdChanged(const QVariant&)));
  }

  ~SVImagePropertyDockWidgetPrivate() {
    delete group1;
    delete group2;
    delete fileName;
    delete width;
    delete height;
    delete aspectRatioFixed;
    delete scale;
    delete lowerThreshold;
    delete upperThreshold;
  }

  SVImageSubWindow *window;

  QtTreePropertyBrowser *browser;
  PropertyGroup *group1, *group2;
  ReadOnlyProperty *fileName, *width, *height;
  ReadWriteProperty *aspectRatioFixed, *scale,
                    *lowerThreshold, *upperThreshold;
};


SVImagePropertyDockWidget::SVImagePropertyDockWidget(QWidget *parent)
 : QDockWidget("Property Editor", parent),
   p(new SVImagePropertyDockWidgetPrivate(this)) {

  setFeatures(QDockWidget::AllDockWidgetFeatures);
  setWidget(p->browser);
}

SVImagePropertyDockWidget::~SVImagePropertyDockWidget() {
  delete p;
}

void SVImagePropertyDockWidget::aspectRatioChanged(const QVariant& value) {
  if (p->window)
    p->window->setAspectRatioFixed(value.toBool());
}

void SVImagePropertyDockWidget::scaleChanged(const QVariant& value) {
  if (p->window)
    p->window->setScale(value.toInt());
}

void SVImagePropertyDockWidget::lowerThresholdChanged(const QVariant& value) {
  if (p->window)
    p->window->setLowerThreshold(value.toDouble());
}

void SVImagePropertyDockWidget::upperThresholdChanged(const QVariant& value) {
  if (p->window)
    p->window->setUpperThreshold(value.toDouble());
}

void SVImagePropertyDockWidget::subWindowActivated(QMdiSubWindow *w) {
  p->browser->setEnabled(w != 0L);

  if (SVImageSubWindow *sv = qobject_cast<SVImageSubWindow*>(w)) {
    p->window = sv;

    p->fileName->setValue(sv->fileName());
    p->aspectRatioFixed->setValue(sv->isAspectRatioFixed());
    p->lowerThreshold->setValue(sv->lowerThreshold());
    p->upperThreshold->setValue(sv->upperThreshold());

    p->scale->setValue(sv->scale());
  }

//   Saxsview::Image *image = w->image();

//   p->fileNameProperty->setValue(image->fileName());
}

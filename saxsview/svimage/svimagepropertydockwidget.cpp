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
#include "svimagesubwindow.h"

#include "saxsview.h"
#include "saxsview_image.h"
#include "saxsview_property.h"

#include <QtTreePropertyBrowser>

#include <QtGui>


class SVImagePropertyDockWidget::Private {
public:
  Private(SVImagePropertyDockWidget *dock) {
    browser = new QtTreePropertyBrowser(dock);
    browser->setRootIsDecorated(false);
    browser->setEnabled(false);

    SaxsviewProperty *imageGroup = new SaxsviewProperty("Image", browser);
    imageProperties.append(new SaxsviewProperty("Z Scale", "scale",
                                                browser, imageGroup));
    imageProperties.append(new SaxsviewProperty("Fix Aspect Ratio",
                                                "aspectRatioFixed",
                                                browser, imageGroup));
//     imageProperties.append(new SaxsviewProperty("Title", "imageTitle",
//                                                 browser, imageGroup));
//     imageProperties.append(new SaxsviewProperty("Font", "imageTitleFont",
//                                                 browser, imageGroup));

    SaxsviewProperty *frameGroup = new SaxsviewProperty("Frame", browser);
    frameProperties.append(new SaxsviewProperty("Size", "size",
                                                browser, frameGroup));
    frameProperties.append(new SaxsviewProperty("Lower Threshold", "minValue",
                                                browser, frameGroup));
    frameProperties.append(new SaxsviewProperty("Upper Threshold", "maxValue",
                                                browser, frameGroup));
  }

  QtTreePropertyBrowser *browser;
  QList<SaxsviewProperty*> imageProperties, frameProperties;
};


SVImagePropertyDockWidget::SVImagePropertyDockWidget(QWidget *parent)
 : QDockWidget("Property Editor", parent),
   p(new Private(this)) {

  setFeatures(QDockWidget::AllDockWidgetFeatures);
  setWidget(p->browser);
}

SVImagePropertyDockWidget::~SVImagePropertyDockWidget() {
  delete p;
}

void SVImagePropertyDockWidget::subWindowActivated(QMdiSubWindow *w) {
  p->browser->setEnabled(w != 0L);

  if (SVImageSubWindow *sv = qobject_cast<SVImageSubWindow*>(w)) {
    foreach (SaxsviewProperty *property, p->imageProperties)
      property->setValue(sv->image());

    foreach (SaxsviewProperty *property, p->frameProperties)
      property->setValue(sv->image()->frame());
  }
}

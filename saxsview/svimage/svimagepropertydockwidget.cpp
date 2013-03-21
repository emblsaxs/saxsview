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
  void setupUi(SVImagePropertyDockWidget *dock);
  void setupImageProperties(QtTreePropertyBrowser*);

  QtTreePropertyBrowser *browser;
  QList<SaxsviewProperty*> imageProperties, frameProperties;
};

void SVImagePropertyDockWidget::Private::setupUi(SVImagePropertyDockWidget *dock) {
  browser = new QtTreePropertyBrowser(dock);
  browser->setRootIsDecorated(false);
  browser->setEnabled(false);

  setupImageProperties(browser);
  dock->setWidget(browser);
}

void SVImagePropertyDockWidget::Private::setupImageProperties(QtTreePropertyBrowser *browser) {

  SaxsviewProperty *imageGroup = new SaxsviewProperty("Image", browser);
  imageProperties.append(new SaxsviewProperty("Z Scale", "scale",
                                              browser, imageGroup));
  imageProperties.append(new SaxsviewProperty("Fix Aspect Ratio",
                                              "aspectRatioFixed",
                                              browser, imageGroup));
  imageProperties.append(new SaxsviewProperty("Background", "backgroundColor",
                                              browser, imageGroup));
  imageProperties.append(new SaxsviewProperty("Foreground", "foregroundColor",
                                              browser, imageGroup));

  SaxsviewProperty *titleGroup = new SaxsviewProperty("Title", browser);
  imageProperties.append(new SaxsviewProperty("Text", "imageTitle",
                                              browser, titleGroup));
  imageProperties.append(new SaxsviewProperty("Font", "imageTitleFont",
                                              browser, titleGroup));
  imageProperties.append(new SaxsviewProperty("Color", "imageTitleFontColor",
                                              browser, titleGroup));

  SaxsviewProperty *axisGroup = new SaxsviewProperty("Axis", browser);
  imageProperties.append(new SaxsviewProperty("X Text", "axisTitleX",
                                              browser, axisGroup));
  imageProperties.append(new SaxsviewProperty("Y Text", "axisTitleY",
                                              browser, axisGroup));
  imageProperties.append(new SaxsviewProperty("Z Text", "axisTitleZ",
                                              browser, axisGroup));
  imageProperties.append(new SaxsviewProperty("Font", "axisTitleFont",
                                              browser, axisGroup));
  imageProperties.append(new SaxsviewProperty("Color", "axisTitleFontColor",
                                              browser, axisGroup));

  SaxsviewProperty *colorBarGroup = new SaxsviewProperty("Color Bar", browser);
  imageProperties.append(new SaxsviewProperty("Visible", "colorBarVisible",
                                              browser, colorBarGroup));
  imageProperties.append(new SaxsviewProperty("From Color", "colorBarFromColor",
                                              browser, colorBarGroup));
  imageProperties.append(new SaxsviewProperty("To Color", "colorBarToColor",
                                              browser, colorBarGroup));

  SaxsviewProperty *ticksGroup = new SaxsviewProperty("Ticks", browser);
  imageProperties.append(new SaxsviewProperty("Minor Tick Marks", "minorTicksVisible",
                                              browser, ticksGroup));
  imageProperties.append(new SaxsviewProperty("Major Tick Marks", "majorTicksVisible",
                                              browser, ticksGroup));
  imageProperties.append(new SaxsviewProperty("X Tick Labels", "xTickLabelsVisible",
                                              browser, ticksGroup));
  imageProperties.append(new SaxsviewProperty("Y Tick Labels", "yTickLabelsVisible",
                                              browser, ticksGroup));
  imageProperties.append(new SaxsviewProperty("Tick Label Font", "tickLabelFont",
                                              browser, ticksGroup));
  imageProperties.append(new SaxsviewProperty("Color", "tickLabelFontColor",
                                              browser, ticksGroup));

  SaxsviewProperty *frameGroup = new SaxsviewProperty("Frame", browser);
  frameProperties.append(new SaxsviewProperty("Size", "size",
                                              browser, frameGroup));
  frameProperties.append(new SaxsviewProperty("Lower Threshold", "minValue",
                                              browser, frameGroup));
  frameProperties.append(new SaxsviewProperty("Upper Threshold", "maxValue",
                                              browser, frameGroup));

  // TODO: The mask file name should not be a line edit, but a button with file open dialog.
  SaxsviewProperty *maskGroup = new SaxsviewProperty("Mask", browser);
  frameProperties.append(new SaxsviewProperty("Mask File", "maskFileName",
                                              browser, maskGroup));
  frameProperties.append(new SaxsviewProperty("Apply Mask", "isMaskApplied",
                                              browser, maskGroup));
}





SVImagePropertyDockWidget::SVImagePropertyDockWidget(QWidget *parent)
 : QDockWidget("Property Editor", parent), p(new Private) {

  setFeatures(QDockWidget::AllDockWidgetFeatures);
  p->setupUi(this);
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

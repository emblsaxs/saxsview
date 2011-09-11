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
#include "saxsview_configpage_defaultcolors.h"

#include <QtGui>

namespace Saxsview {

class DefaultColorsConfigPage::DefaultColorsConfigPagePrivate {
public:
  QList<ColorButton*> lineColorButtons, errorBarColorButtons;
};


DefaultColorsConfigPage::DefaultColorsConfigPage(QWidget *parent)
 : AbstractConfigPage(parent), p(new DefaultColorsConfigPagePrivate) {

  setupUi(this);

  p->lineColorButtons.push_back(lineColor01);
  p->lineColorButtons.push_back(lineColor02);
  p->lineColorButtons.push_back(lineColor03);
  p->lineColorButtons.push_back(lineColor04);
  p->lineColorButtons.push_back(lineColor05);
  p->lineColorButtons.push_back(lineColor06);
  p->lineColorButtons.push_back(lineColor07);
  p->lineColorButtons.push_back(lineColor08);

  p->errorBarColorButtons.push_back(errorBarColor01);
  p->errorBarColorButtons.push_back(errorBarColor02);
  p->errorBarColorButtons.push_back(errorBarColor03);
  p->errorBarColorButtons.push_back(errorBarColor04);
  p->errorBarColorButtons.push_back(errorBarColor05);
  p->errorBarColorButtons.push_back(errorBarColor06);
  p->errorBarColorButtons.push_back(errorBarColor07);
  p->errorBarColorButtons.push_back(errorBarColor08);

  reset();
}

DefaultColorsConfigPage::~DefaultColorsConfigPage() {
  delete p;
}

void DefaultColorsConfigPage::apply() {
  QList<QColor> defaultLineColor, defaultErrorBarColor;

  for (size_t i = 0, j = 0; i < p->lineColorButtons.size(); ++i)
    if (p->lineColorButtons[i]->color().isValid())
      defaultLineColor.push_back(p->lineColorButtons[i]->color());

  for (size_t i = 0, j = 0; i < p->errorBarColorButtons.size(); ++i)
    if (p->errorBarColorButtons[i]->color().isValid())
      defaultErrorBarColor.push_back(p->errorBarColorButtons[i]->color());

  config().setDefaultColors(defaultLineColor, defaultErrorBarColor);
}

void DefaultColorsConfigPage::reset() {
  QList<QColor> defaultLineColor, defaultErrorBarColor;
  config().defaultColors(defaultLineColor, defaultErrorBarColor);

  for (size_t i = 0, j = 0; i < defaultLineColor.size(); ++i)
    if (defaultLineColor[i].isValid())
      p->lineColorButtons[j++]->setColor(defaultLineColor[i]);

  for (size_t i = 0, j = 0; i < defaultErrorBarColor.size(); ++i)
    if (defaultErrorBarColor[i].isValid())
      p->errorBarColorButtons[j++]->setColor(defaultErrorBarColor[i]);
}

} // end of namespace Saxsview

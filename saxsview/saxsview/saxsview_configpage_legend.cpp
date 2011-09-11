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

#include "saxsview_configpage.h"
#include "saxsview_plot.h"

#include "qwt_dyngrid_layout.h"
#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_layout.h"

#include <QtGui>

namespace Saxsview {

LegendConfigPage::LegendConfigPage(Plot *p, QWidget *parent)
 : AbstractConfigPage(parent), plot(p) {

  setupUi(this);
  reset();
}

void LegendConfigPage::apply() {
  // FIXME: apply legend font

  plot->plotLayout()->setLegendPosition(comboPosition->currentLegendPosition());

  QwtLegend *legend = plot->legend();
  QLayout *layout = legend->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  ll->setMaxCols(spinColumns->value());
  ll->setMargin(spinMargin->value());
  ll->setSpacing(spinSpacing->value());

  if (checkFramed->isChecked())
    legend->setFrameStyle(QFrame::Box);
  else
    legend->setFrameStyle(QFrame::NoFrame);

  plot->updateLayout();
}

void LegendConfigPage::reset() {
  // FIXME: read legend font

  comboPosition->setCurrentLegendPosition(plot->plotLayout()->legendPosition());

  QwtLegend *legend = plot->legend();
  QLayout *layout = legend->contentsWidget()->layout();
  QwtDynGridLayout *ll = qobject_cast<QwtDynGridLayout*>(layout);
  spinColumns->setValue(ll->maxCols());
  spinMargin->setValue(ll->margin());
  spinSpacing->setValue(ll->spacing());

  checkFramed->setChecked(legend->frameStyle() == QFrame::Box);
}

} // end of namespace Saxsview

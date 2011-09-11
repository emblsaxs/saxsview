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

#include "qwt_plot.h"
#include "qwt_scale_draw.h"
#include "qwt_text.h"

#include <QtGui>

namespace Saxsview {

PlotConfigPage::PlotConfigPage(Plot *p, QWidget *parent)
 : AbstractConfigPage(parent), plot(p) {
   setupUi(this);
   reset();
}

void PlotConfigPage::apply() {
  // title
  QFont titleFont = comboTitleFontFamily->currentFont();
  titleFont.setPointSize(spinTitleFontSize->value());
  titleFont.setBold(checkTitleFontBold->isChecked());
  titleFont.setItalic(checkTitleFontItalic->isChecked());

  QwtText title;
  title.setText(editTitle->text());
  title.setFont(titleFont);

  plot->setTitle(title);

  // axis
  QFont axisLabelFont = comboAxisFontFamily->currentFont();
  axisLabelFont.setPointSize(spinAxisFontSize->value());
  axisLabelFont.setBold(checkAxisFontBold->isChecked());
  axisLabelFont.setItalic(checkAxisFontItalic->isChecked());

  QwtText xLabel;
  xLabel.setText(editXLabel->text());
  xLabel.setFont(axisLabelFont);
  plot->setAxisTitle(QwtPlot::xBottom, xLabel);

  QwtText yLabel;
  yLabel.setText(editYLabel->text());
  yLabel.setFont(axisLabelFont);
  plot->setAxisTitle(QwtPlot::yLeft, yLabel);

  // ticks
  QFont ticksFont = comboTicksFontFamily->currentFont();
  ticksFont.setPointSize(spinTicksFontSize->value());
  ticksFont.setBold(checkTicksFontBold->isChecked());
  ticksFont.setItalic(checkTicksFontItalic->isChecked());

  QwtScaleDraw *scale;

  // x axis tick labels
  scale = plot->axisScaleDraw(QwtPlot::xBottom);
  scale->enableComponent(QwtAbstractScaleDraw::Labels,
                         checkXTickLabels->isChecked());
  plot->setAxisFont(QwtPlot::xBottom, ticksFont);

  // y axis tick labels
  scale = plot->axisScaleDraw(QwtPlot::yLeft);
  scale->enableComponent(QwtAbstractScaleDraw::Labels,
                         checkYTickLabels->isChecked());
  plot->setAxisFont(QwtPlot::yLeft, ticksFont);

  //
  // Update the zoomBase, then zoom to it.
  // Sanity check: If the data has really small values, the spin box
  //               can't show it and rounds to 0.0 - which is a problem
  //               in log-plots. If the lower bound (spinRangeYFrom) is
  //               less than the minimum step of the spin box, re-use
  //               the current lower bound.
  //
  QRectF r(QPointF(spinXRangeFrom->value(), spinYRangeFrom->value()),
           QPointF(spinXRangeTo->value(), spinYRangeTo->value()));

  if (plot->scale() == Plot::Log10Scale
      && spinYRangeFrom->value() < 1.0/pow(10.0, spinYRangeFrom->decimals()))
    r.setTop(plot->zoomBase().top());

  plot->setZoomBase(r);
  plot->zoom(r);
  plot->setZoomBase(r);

  // other
  const bool antiAliased = checkAntiAliased->isChecked();
  foreach (QwtPlotItem* item, plot->itemList())
    item->setRenderHint(QwtPlotItem::RenderAntialiased, antiAliased);
}

void PlotConfigPage::reset() {
  // title
  QwtText title = plot->title();
  QFont titleFont = title.font();

  editTitle->setText(title.text());
  comboTitleFontFamily->setCurrentFont(titleFont);
  spinTitleFontSize->setValue(titleFont.pointSize());
  checkTitleFontBold->setChecked(titleFont.bold());
  checkTitleFontItalic->setChecked(titleFont.italic());

  // axis
  QwtText xLabel = plot->axisTitle(QwtPlot::xBottom);
  QwtText yLabel = plot->axisTitle(QwtPlot::yLeft);
  QFont axisLabelFont = xLabel.font();

  editXLabel->setText(xLabel.text());
  editYLabel->setText(yLabel.text());
  comboAxisFontFamily->setCurrentFont(axisLabelFont);
  spinAxisFontSize->setValue(axisLabelFont.pointSize());
  checkAxisFontBold->setChecked(axisLabelFont.bold());
  checkAxisFontItalic->setChecked(axisLabelFont.italic());

  // ticks
  QFont ticksFont = plot->axisFont(QwtPlot::xBottom);
  comboTicksFontFamily->setCurrentFont(ticksFont);
  spinTicksFontSize->setValue(ticksFont.pointSize());
  checkTicksFontBold->setChecked(ticksFont.bold());
  checkTicksFontItalic->setChecked(ticksFont.italic());

  QwtScaleDraw *scale;

  // x axis tick labels
  scale = plot->axisScaleDraw(QwtPlot::xBottom);
  checkXTickLabels->setChecked(scale->hasComponent(QwtAbstractScaleDraw::Labels));

  // y axis tick labels
  scale = plot->axisScaleDraw(QwtPlot::yLeft);
  checkYTickLabels->setChecked(scale->hasComponent(QwtAbstractScaleDraw::Labels));

  QRectF zoomBase = plot->zoomBase();
  spinXRangeFrom->setValue(zoomBase.left());
  spinXRangeTo->setValue(zoomBase.right());
  spinYRangeFrom->setValue(zoomBase.top());
  spinYRangeTo->setValue(zoomBase.bottom());

  // other
  if (!plot->itemList().isEmpty()) {
    QwtPlotItem* item = plot->itemList().first();
    checkAntiAliased->setChecked(item->testRenderHint(QwtPlotItem::RenderAntialiased));
  } else
    checkAntiAliased->setChecked(false);
}

} // end of namespace Saxsview

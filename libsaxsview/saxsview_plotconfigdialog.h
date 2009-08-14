/*
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SAXSVIEW_PLOTCONFIGDIALOG_H
#define SAXSVIEW_PLOTCONFIGDIALOG_H

#include <QDialog>
class QAbstractButton;
class QWidget;
class QListWidgetItem;

namespace Saxsview {

class Plot;

class PlotConfigDialog : public QDialog {
  Q_OBJECT

public:
  PlotConfigDialog(Plot *plot, QWidget *parent = 0L);
  ~PlotConfigDialog();

  // Apply dialog's settings to plot
  void apply(Plot *plot);

  // Reset the dialog with values from the given plot.
  void reset(Plot *plot);

private slots:
  void changePage(QListWidgetItem *, QListWidgetItem*);
  void buttonClicked(QAbstractButton*);

private:
  class PlotConfigDialogPrivate;
  PlotConfigDialogPrivate *p;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_PLOTCONFIGDIALOG_H

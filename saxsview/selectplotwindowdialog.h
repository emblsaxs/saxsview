/*
 * [FIXME]
 * Copyright (C) 2010 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SELECT_PLOT_WINDOW_DIALOG_H
#define SELECT_PLOT_WINDOW_DIALOG_H

#include "ui_selectplotwindowdialog.h"
class SaxsviewPlotWindow;

#include <QDialog>
class QAbstractButton;


class SelectPlotWindowDialog : public QDialog,
                               private Ui::SelectPlotWindowDialog {
  Q_OBJECT

public:
  SelectPlotWindowDialog(const QString& caption, QWidget *parent = 0L);
  ~SelectPlotWindowDialog();

  void addPlotWindow(const QString& label, SaxsviewPlotWindow *w);
  SaxsviewPlotWindow* selectedPlotWindow() const;

  bool tileSubWindows() const;
};

#endif // !SELECT_PLOT_WINDOW_DIALOG_H

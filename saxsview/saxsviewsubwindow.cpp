/*
 * Interface for view subwindows.
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "saxsviewsubwindow.h"
#include "saxsview_plot.h"

#include <QtGui>

SaxsviewSubWindow::SaxsviewSubWindow(QWidget *parent)
 : QMdiSubWindow(parent) {
}

SaxsviewSubWindow::~SaxsviewSubWindow() {
}

QList<QAction*> SaxsviewSubWindow::saxsviewActions() const {
  return QList<QAction*>();
}

void SaxsviewSubWindow::configure() {
  QMessageBox::information(this, "Sorry", "Nothing to configure");
}

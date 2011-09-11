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

#include "selectplotwindowdialog.h"

#include <QtGui>


SelectPlotWindowDialog::SelectPlotWindowDialog(const QString& caption,
                                               QWidget *parent)
 : QDialog(parent) {

  setupUi(this);
  label->setText(caption);

  //
  // Double-clicking an entry is the same as selecting one and
  // pressing the ok-button.
  //
  connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
          this, SLOT(accept()));
}

SelectPlotWindowDialog::~SelectPlotWindowDialog() {
}

//
// Required to make SaxsviewSubWindow* known to QVariant.
//
Q_DECLARE_METATYPE(SaxsviewPlotWindow*);

void SelectPlotWindowDialog::addPlotWindow(const QString& label,
                                          SaxsviewPlotWindow *w) {
  QListWidgetItem *item = new QListWidgetItem(label);
  item->setData(Qt::UserRole, qVariantFromValue(w));
  listWidget->addItem(item);
}

SaxsviewPlotWindow* SelectPlotWindowDialog::selectedPlotWindow() const {
  QListWidgetItem *item = listWidget->currentItem();
  return qVariantValue<SaxsviewPlotWindow*>(item->data(Qt::UserRole));
}

bool SelectPlotWindowDialog::tileSubWindows() const {
  return checkTileSubWindows->isChecked();
}

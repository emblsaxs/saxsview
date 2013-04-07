/*
 * Copyright (C) 2013 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SVIMAGEMASKTHRESHOLDSDIALOG_H
#define SVIMAGEMASKTHRESHOLDSDIALOG_H

#include <QDialog>

class SVImageMaskThresholdsDialog : public QDialog {
  Q_OBJECT

public:
  SVImageMaskThresholdsDialog(QWidget *parent = 0L);

  void setRange(double min, double max);

  void selectedThreshold(double *min, double *max) const;

signals:
  /** Emitted whenever a threshold changes. */
  void currentThresholdChanged(double lower, double upper);

  /** Emitted once the dialog was accepted. */
  void thresholdSelected(double lower, double upper);

private slots:
  void thresholdChanged();
  void accept();

private:
  class Private;
  Private *p;
};

#endif // !SVIMAGEMASKTHRESHOLDSDIALOG_H

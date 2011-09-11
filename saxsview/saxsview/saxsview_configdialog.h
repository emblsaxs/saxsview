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

#ifndef SAXSVIEW_CONFIGDIALOG_H
#define SAXSVIEW_CONFIGDIALOG_H

#include <QDialog>
class QAbstractButton;

namespace Saxsview {

class Plot;
class AbstractConfigPage;

class AbstractConfigDialog : public QDialog {
  Q_OBJECT

public:
  AbstractConfigDialog(QWidget *parent = 0L);
  virtual ~AbstractConfigDialog() = 0;

protected:
  void addConfigPage(AbstractConfigPage *page, const QString& title);

private slots:
  void buttonClicked(QAbstractButton*);

private:
  class ConfigDialogPrivate;
  ConfigDialogPrivate *p;
};


class SaxsviewConfigDialog : public AbstractConfigDialog {
public:
  SaxsviewConfigDialog(QWidget *parent);
};

class PlotConfigDialog : public AbstractConfigDialog {
public:
  PlotConfigDialog(Plot *plot, QWidget *parent);
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_CONFIGDIALOG_H

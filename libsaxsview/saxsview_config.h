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

#ifndef SAXSVIEW_CONFIG_H
#define SAXSVIEW_CONFIG_H

#include <QList>
class QColor;
class QPen;
class QStandardItemModel;

namespace Saxsview {

class PlotCurve;


class SaxsviewConfig {
public:
  void curveTemplates(QStandardItemModel*) const;
  void setCurveTemplates(QStandardItemModel*);

  void fileTypeTemplates(QStandardItemModel*) const;
  void setFileTypeTemplates(QStandardItemModel*);

  void applyTemplate(PlotCurve* curve) const;

  void defaultColors(QList<QColor>&, QList<QColor>&) const;
  void setDefaultColors(const QList<QColor>&, const QList<QColor>&);

private:
  SaxsviewConfig();
  ~SaxsviewConfig();

  friend SaxsviewConfig& config();
};

SaxsviewConfig& config();

} // end of namespace Saxsview

#endif // !SAXSVIEW_CONFIG_H

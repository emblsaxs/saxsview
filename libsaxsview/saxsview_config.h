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

class QPen;
class QStandardItemModel;

namespace Saxsview {

class PlotSymbol;


class SaxsviewConfig {
public:
  void curveTemplates(QStandardItemModel*) const;
  void setCurveTemplates(QStandardItemModel*);

  int currentCurveTemplate(int type);
  void setCurrentCurveTemplate(int type, int index);

  void templateForCurveType(int type, QPen&, PlotSymbol&, QPen&);

private:
  SaxsviewConfig();
  ~SaxsviewConfig();

  friend SaxsviewConfig& config();
};

SaxsviewConfig& config();

} // end of namespace Saxsview

#endif // !SAXSVIEW_CONFIG_H

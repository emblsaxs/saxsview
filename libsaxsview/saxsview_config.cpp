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

#include "saxsview_config.h"
#include "saxsview_plotcurve.h"

#include <QtGui>

namespace Saxsview {

SaxsviewConfig& config() {
  static SaxsviewConfig config;
  return config;
}

QSettings& settings() {
  // uses organization, domain and appname from QCoreApplication
  static QSettings settings;
  return settings;
}

SaxsviewConfig::SaxsviewConfig() {
}

SaxsviewConfig::~SaxsviewConfig() {
}

void SaxsviewConfig::curveTemplates(QStandardItemModel *model) const {
  QStringList column;
  column << "name"
         << "line-style" << "line-width"
         << "symbol-style" << "symbol-size"
         << "error-bar-style" << "error-bar-width";

  settings().beginGroup("Templates");
  int count = settings().beginReadArray("template");

  for (int i = 0; i < count; ++i) {
    settings().setArrayIndex(i);
    for (int j = 0; j < column.size(); ++j) {
      QString value = settings().value(column[j]).toString();
      model->setItem(i, j, new QStandardItem(value));
    }
  }

  settings().endArray();
  settings().endGroup();
}

void SaxsviewConfig::setCurveTemplates(QStandardItemModel *model) {
  QStringList column;
  column << "name"
         << "line-style" << "line-width"
         << "symbol-style" << "symbol-size"
         << "error-bar-style" << "error-bar-width";

  settings().beginGroup("Templates");
  settings().beginWriteArray("template");

  for (int i = 0; i < model->rowCount(); ++i) {
    settings().setArrayIndex(i);
    for (int j = 0; j < column.size(); ++j)
      settings().setValue(column[j], model->item(i, j)->text());
  }

  settings().endArray();
  settings().endGroup();
}

int SaxsviewConfig::currentCurveTemplate(int type) {
  const QString key = "current-curve-template-%1";

  settings().beginGroup("Templates");
  int index = settings().value(key.arg(type)).toInt();
  settings().endGroup();

  return index;
}

void SaxsviewConfig::setCurrentCurveTemplate(int type, int index) {
  const QString key = "current-curve-template-%1";

  settings().beginGroup("Templates");
  settings().setValue(key.arg(type), index);
  settings().endGroup();
}

void SaxsviewConfig::templateForCurveType(int type, QPen& line,
                                          PlotSymbol& symbol,
                                          QPen& errors) {

  int current = currentCurveTemplate(type);

  settings().beginGroup("Templates");
  settings().beginReadArray("template");

  settings().setArrayIndex(current);

  line.setStyle((Qt::PenStyle) settings().value("line-style", 0).toInt());
  line.setWidth(settings().value("line-width", 1).toInt());

  symbol.setStyle((Saxsview::PlotSymbol::Style) settings().value("symbol-style", 0).toInt());
  symbol.setSize(settings().value("symbol-size", 1).toInt());

  errors.setStyle((Qt::PenStyle) settings().value("error-bar-style", 0).toInt());
  errors.setWidth(settings().value("error-bar-width", 1).toInt());
  errors.setColor(Qt::lightGray);

  settings().endArray();
  settings().endGroup();
}

} // end of namespace Saxsview

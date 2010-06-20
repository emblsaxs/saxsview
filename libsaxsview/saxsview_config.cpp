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

  //
  // Defaults
  //
  if (model->rowCount() == 0) {
    QList<QStandardItem*> t1, t2;

    t1.push_back(new QStandardItem("open circles w/ errors"));
    t1.push_back(new QStandardItem(QString::number(Qt::NoPen)));            // line style
    t1.push_back(new QStandardItem("1"));                                   // line width
    t1.push_back(new QStandardItem(QString::number(PlotSymbol::Ellipse)));  // symbol style
    t1.push_back(new QStandardItem("4"));                                   // symbol size
    t1.push_back(new QStandardItem(QString::number(Qt::SolidLine)));        // error bar style
    t1.push_back(new QStandardItem("1"));                                   // error bar width
    model->appendRow(t1);

    t2.push_back(new QStandardItem("solid line w/o errors"));
    t2.push_back(new QStandardItem(QString::number(Qt::SolidLine)));
    t2.push_back(new QStandardItem("2"));
    t2.push_back(new QStandardItem(QString::number(PlotSymbol::NoSymbol)));
    t2.push_back(new QStandardItem("1"));
    t2.push_back(new QStandardItem(QString::number(Qt::NoPen)));
    t2.push_back(new QStandardItem("1"));
    model->appendRow(t2);
  }
}

void SaxsviewConfig::setCurveTemplates(QStandardItemModel *model) {
  QStringList column;
  column << "name"
         << "line-style" << "line-width"
         << "symbol-style" << "symbol-size"
         << "error-bar-style" << "error-bar-width";

  settings().beginGroup("Templates");
  settings().remove("template");
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

void SaxsviewConfig::defaultColors(QList<QColor>& lineColor,
                                   QList<QColor>& errorBarColor) const {
  int count;

  settings().beginGroup("Default Colors");
  count = settings().beginReadArray("lineColors");
  for (int i = 0; i < count; ++i) {
    settings().setArrayIndex(i);
    lineColor.push_back(qVariantValue<QColor>(settings().value("color", QColor())));
  }
  settings().endArray();

  count = settings().beginReadArray("errorBarColors");
  for (int i = 0; i < count; ++i) {
    settings().setArrayIndex(i);
    errorBarColor.push_back(qVariantValue<QColor>(settings().value("color", QColor())));
  }
  settings().endArray();
  settings().endGroup();

  //
  // Defaults
  //
  if (lineColor.isEmpty()) {
    lineColor.push_back(QColor("red"));
    lineColor.push_back(QColor("blue"));
    lineColor.push_back(QColor("lightgreen"));
    lineColor.push_back(QColor("khaki"));
    lineColor.push_back(QColor("aqua"));
    lineColor.push_back(QColor("greenyellow"));
  }

  if (errorBarColor.isEmpty()) {
    errorBarColor.push_back(QColor("lightgray"));
  }
}

void SaxsviewConfig::setDefaultColors(const QList<QColor>& lineColor,
                                      const QList<QColor>& errorBarColor) {
  settings().beginGroup("Default Colors");

  settings().remove("lineColors");
  settings().beginWriteArray("lineColors");
  for (int i = 0; i < lineColor.size(); ++i) {
    settings().setArrayIndex(i);
    settings().setValue("color", lineColor[i]);
  }
  settings().endArray();

  settings().remove("errorBarColors");
  settings().beginWriteArray("errorBarColors");
  for (int i = 0; i < errorBarColor.size(); ++i) {
    settings().setArrayIndex(i);
    settings().setValue("color", errorBarColor[i]);
  }
  settings().endArray();
  settings().endGroup();
}

} // end of namespace Saxsview

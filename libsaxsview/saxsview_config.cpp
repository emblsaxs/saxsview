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

#include "saxsdocument.h"
#include "saxsdocument_format.h"

#include <QtGui>

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
  //
  // Check the config, create default values and update existing ones,
  // if necessary.
  //
  // TODO: set a version flag and check it?
  //
  if (!settings().contains("Templates/template"))
    setDefaultCurveTemplates();

  if (!settings().contains("Templates/file-type"))
    setDefaultFileTypeTemplates();

  if (!settings().contains("Default Colors/lineColors")
      && !settings().contains("Default Colors/errorBarColors"))
    setDefaultColors();
}

SaxsviewConfig::~SaxsviewConfig() {
}

QString SaxsviewConfig::recentDirectory() const {
  return settings().value("recentDirectory", QDir::currentPath()).toString();
}

void SaxsviewConfig::setRecentDirectory(const QString& path) {
  settings().setValue("recentDirectory", QFileInfo(path).absolutePath());
}

QStringList SaxsviewConfig::recentFiles() const {
  return settings().value("recentFiles").toStringList();
}

void SaxsviewConfig::addRecentFile(const QString& fileName) {
  //
  // Add to the list of recently opened files;
  // remove duplicates (if any), prepend current
  // filename and remove old ones (if any).
  //
  QStringList recent = recentFiles();

  recent.removeAll(fileName);
  recent.prepend(fileName);
  while (recent.size() > 10)
    recent.removeLast();

  settings().setValue("recentFiles", recent);
}

QString SaxsviewConfig::recentPrinter() const {
  return settings().value("recentPrinter").toString();
}

void SaxsviewConfig::setRecentPrinter(const QString& printer) {
  settings().setValue("recentPrinter", printer);
}

void SaxsviewConfig::curveTemplates(QStandardItemModel *model) const {
  QStringList column;
  column << "name"
         << "line-style" << "line-width"
         << "symbol-style" << "symbol-size" << "symbol-filled"
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

void SaxsviewConfig::setDefaultCurveTemplates() {
  QStandardItemModel model;
  QList<QStandardItem*> t1, t2;

  t1.push_back(new QStandardItem("filled circles w/ errors"));
  t1.push_back(new QStandardItem(QString::number(Qt::NoPen)));          // line style
  t1.push_back(new QStandardItem("1"));                                 // line width
  t1.push_back(new QStandardItem(QString::number(Saxsview::Ellipse)));  // symbol style
  t1.push_back(new QStandardItem("4"));                                 // symbol size
  t1.push_back(new QStandardItem("1"));                                 // symbol filled
  t1.push_back(new QStandardItem(QString::number(Qt::SolidLine)));      // error bar style
  t1.push_back(new QStandardItem("1"));                                 // error bar width
  model.appendRow(t1);

  t2.push_back(new QStandardItem("solid line w/o errors"));
  t2.push_back(new QStandardItem(QString::number(Qt::SolidLine)));
  t2.push_back(new QStandardItem("2"));
  t2.push_back(new QStandardItem(QString::number(Saxsview::NoSymbol)));
  t2.push_back(new QStandardItem("1"));
  t2.push_back(new QStandardItem("0"));
  t2.push_back(new QStandardItem(QString::number(Qt::NoPen)));
  t2.push_back(new QStandardItem("1"));
  model.appendRow(t2);

  setCurveTemplates(&model);
}

void SaxsviewConfig::setCurveTemplates(QStandardItemModel *model) {
  QStringList column;
  column << "name"
         << "line-style" << "line-width"
         << "symbol-style" << "symbol-size" << "symbol-filled"
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

void SaxsviewConfig::fileTypeTemplates(QStandardItemModel *model) const {
  QStringList column;
  column << "format"
         << QString("template-%1").arg(SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA)
         << QString("template-%1").arg(SAXS_CURVE_THEORETICAL_SCATTERING_DATA)
         << QString("template-%1").arg(SAXS_CURVE_PROBABILITY_DATA);

  settings().beginGroup("Templates");
  int n = settings().beginReadArray("file-type");

  for (int i = 0; i < n; ++i) {
    settings().setArrayIndex(i);
    for (int j = 0; j < column.size(); ++j) {
      QString value = settings().value(column[j]).toString();
      model->setItem(i, j, new QStandardItem(value));
    }
  }

  settings().endArray();
  settings().endGroup();
}

void SaxsviewConfig::setDefaultFileTypeTemplates() {
  QStandardItemModel model;

  saxs_document_format *fmt = saxs_document_format_first();
  while (fmt) {
    QList<QStandardItem*> row;
    row.push_back(new QStandardItem(fmt->name));
    row.push_back(new QStandardItem("0"));  // default for experimental data
    row.push_back(new QStandardItem("0"));  // default for theoretical data
    row.push_back(new QStandardItem("0"));  // default for probability data
    model.appendRow(row);

    fmt = saxs_document_format_next(fmt);
  }

  setFileTypeTemplates(&model);
}

void SaxsviewConfig::setFileTypeTemplates(QStandardItemModel *model) {
  QStringList column;
  column << "format"
         << QString("template-%1").arg(SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA)
         << QString("template-%1").arg(SAXS_CURVE_THEORETICAL_SCATTERING_DATA)
         << QString("template-%1").arg(SAXS_CURVE_PROBABILITY_DATA);

  settings().beginGroup("Templates");
  settings().remove("file-type");
  settings().beginWriteArray("file-type");

  for (int i = 0; i < model->rowCount(); ++i) {
    settings().setArrayIndex(i);
    for (int j = 0; j < column.size(); ++j)
      settings().setValue(column[j], model->item(i, j)->text());
  }

  settings().endArray();
  settings().endGroup();
}

/**
 * Derive the format from curve->fileName() and use curve->type()
 * to find the template to apply.
 */
void SaxsviewConfig::applyTemplate(SaxsviewPlotCurve *curve) const {
  int n, template_id;
  saxs_document_format *fmt;

  settings().beginGroup("Templates");

  fmt = saxs_document_format_find_first(qPrintable(curve->fileName()), 0L);
  if (!fmt)
    return;

  n = settings().beginReadArray("file-type");
  for (int i = 0; i < n; ++i) {
    settings().setArrayIndex(i);
    QString format = settings().value("format").toString();
    if (format.compare(fmt->name, Qt::CaseInsensitive) == 0) {
      template_id = settings().value(QString("template-%1").arg(curve->type())).toInt();
      break;
    }
  }
  settings().endArray();

  settings().beginReadArray("template");
  settings().setArrayIndex(template_id);

  curve->setLineStyle((Saxsview::LineStyle) settings().value("line-style", 0).toInt());
  curve->setLineWidth(settings().value("line-width", 1).toInt());

  curve->setSymbolStyle((Saxsview::SymbolStyle) settings().value("symbol-style", 0).toInt());
  curve->setSymbolSize(settings().value("symbol-size", 1).toInt());
  curve->setSymbolFilled(settings().value("symbol-filled", 1).toBool());

  curve->setErrorLineStyle((Saxsview::LineStyle) settings().value("error-bar-style", 0).toInt());
  curve->setErrorLineWidth(settings().value("error-bar-width", 1).toInt());

  settings().endArray();
  settings().endGroup();
}

void SaxsviewConfig::colors(QList<QColor>& lineColor,
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
}

void SaxsviewConfig::setDefaultColors() {
  QList<QColor> lineColor, errorBarColor;

  lineColor.push_back(QColor( 55, 104, 184));
  lineColor.push_back(QColor(127, 207, 215));
  lineColor.push_back(QColor(228,  26,  28));
  lineColor.push_back(QColor(238, 131, 181));
  lineColor.push_back(QColor(166,  86,  40));
  lineColor.push_back(QColor( 52,  47, 145));
  lineColor.push_back(QColor( 62, 175,  59));
  lineColor.push_back(QColor(255, 236,   0));

  errorBarColor.push_back(QColor(196, 219, 255));
  errorBarColor.push_back(QColor(222, 245, 245));
  errorBarColor.push_back(QColor(255, 180, 181));
  errorBarColor.push_back(QColor(255, 217, 236));
  errorBarColor.push_back(QColor(255, 212, 190));
  errorBarColor.push_back(QColor(200, 197, 255));
  errorBarColor.push_back(QColor(201, 255, 199));
  errorBarColor.push_back(QColor(255, 248, 170));

  setColors(lineColor, errorBarColor);
}

void SaxsviewConfig::setColors(const QList<QColor>& lineColor,
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

QByteArray SaxsviewConfig::geometry() const {
  return settings().value("Window/Geometry").toByteArray();
}

void SaxsviewConfig::setGeometry(const QByteArray& value) {
  settings().setValue("Window/Geometry", value);
}

QByteArray SaxsviewConfig::windowState() const {
  return settings().value("Window/State").toByteArray();
}

void SaxsviewConfig::setWindowState(const QByteArray& value) {
  settings().setValue("Window/State", value);
}

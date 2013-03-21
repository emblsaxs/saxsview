/*
 * Copyright (C) 2011, 2012, 2013
 * Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SAXSVIEWIMAGE_H
#define SAXSVIEWIMAGE_H

#include <QObject>
class QString;

#include "qwt_plot.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_raster_data.h"

#include "saxsview.h"
class SaxsviewFrame;
class SaxsviewFrameData;


class SaxsviewImage : public QwtPlot {
  Q_OBJECT
  Q_PROPERTY(Saxsview::Scale scale READ scale WRITE setScale)
  Q_PROPERTY(bool aspectRatioFixed READ isAspectRatioFixed WRITE setAspectRatioFixed)

  Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
  Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
  Q_PROPERTY(QString imageTitle READ imageTitle WRITE setImageTitle)
  Q_PROPERTY(QFont imageTitleFont READ imageTitleFont WRITE setImageTitleFont)
  Q_PROPERTY(QColor imageTitleFontColor READ imageTitleFontColor WRITE setImageTitleFontColor)

  Q_PROPERTY(QString axisTitleX READ axisTitleX WRITE setAxisTitleX)
  Q_PROPERTY(QString axisTitleY READ axisTitleY WRITE setAxisTitleY)
  Q_PROPERTY(QString axisTitleZ READ axisTitleZ WRITE setAxisTitleZ)
  Q_PROPERTY(QFont axisTitleFont READ axisTitleFont WRITE setAxisTitleFont)
  Q_PROPERTY(QColor axisTitleFontColor READ axisTitleFontColor WRITE setAxisTitleFontColor)
  Q_PROPERTY(QColor colorBarFromColor READ colorBarFromColor WRITE setColorBarFromColor)
  Q_PROPERTY(QColor colorBarToColor READ colorBarToColor WRITE setColorBarToColor)

  Q_PROPERTY(bool minorTicksVisible READ minorTicksVisible WRITE setMinorTicksVisible)
  Q_PROPERTY(bool majorTicksVisible READ majorTicksVisible WRITE setMajorTicksVisible)
  Q_PROPERTY(bool xTickLabelsVisible READ xTickLabelsVisible WRITE setXTickLabelsVisible)
  Q_PROPERTY(bool yTickLabelsVisible READ yTickLabelsVisible WRITE setYTickLabelsVisible)
  Q_PROPERTY(bool colorBarVisible READ colorBarVisible WRITE setColorBarVisible)
  Q_PROPERTY(QFont tickLabelFont READ tickLabelFont WRITE setTickLabelFont)
  Q_PROPERTY(QColor tickLabelFontColor READ tickLabelFontColor WRITE setTickLabelFontColor)

public:
  SaxsviewImage(QWidget *parent = 0L);
  ~SaxsviewImage();

  SaxsviewFrame* frame() const;

  QRectF zoomBase() const;
  void zoom(const QRectF&);

  bool isZoomEnabled() const;
  bool isMoveEnabled() const;

  Saxsview::Scale scale() const;
  bool isAspectRatioFixed() const;

  QColor backgroundColor() const;
  QColor foregroundColor() const;
  QString imageTitle() const;
  QFont imageTitleFont() const;
  QColor imageTitleFontColor() const;
  QString axisTitleX() const;
  QString axisTitleY() const;
  QString axisTitleZ() const;
  QFont axisTitleFont() const;
  QColor axisTitleFontColor() const;
  bool xTickLabelsVisible() const;
  bool yTickLabelsVisible() const;
  bool majorTicksVisible() const;
  bool minorTicksVisible() const;
  QFont tickLabelFont() const;
  QColor tickLabelFontColor() const;
  bool colorBarVisible() const;
  QColor colorBarFromColor() const;
  QColor colorBarToColor() const;

public slots:
  void exportAs();
  void exportAs(const QString& fileName, const QString& format = QString());
  void print();

  void setFrame(SaxsviewFrame *frame);
  void setZoomBase(const QRectF& rect = QRectF());
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);

  void setScale(Saxsview::Scale);
  void setAspectRatioFixed(bool);

  void setBackgroundColor(const QColor&);
  void setForegroundColor(const QColor&);
  void setImageTitle(const QString&);
  void setImageTitleFont(const QFont&);
  void setImageTitleFontColor(const QColor&);
  void setAxisTitleX(const QString&);
  void setAxisTitleY(const QString&);
  void setAxisTitleZ(const QString&);
  void setAxisTitleFont(const QFont&);
  void setAxisTitleFontColor(const QColor&);
  void setXTickLabelsVisible(bool);
  void setYTickLabelsVisible(bool);
  void setMinorTicksVisible(bool);
  void setMajorTicksVisible(bool);
  void setTickLabelFont(const QFont&);
  void setTickLabelFontColor(const QColor&);
  void setColorBarVisible(bool);
  void setColorBarFromColor(const QColor&);
  void setColorBarToColor(const QColor&);

private:
  class Private;
  Private *p;
};


class SaxsviewFrame : public QObject,
                      public QwtPlotSpectrogram {
  Q_OBJECT
  Q_PROPERTY(QSize  size     READ size)
  Q_PROPERTY(double minValue READ minValue WRITE setMinValue)
  Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue)

  Q_PROPERTY(QString maskFileName READ maskFileName WRITE setMaskFileName)
  Q_PROPERTY(bool isMaskApplied READ isMaskApplied WRITE setMaskApplied)

public:
  SaxsviewFrame(QObject *parent = 0L);
  ~SaxsviewFrame();

  QSize size() const;

  double minValue() const;
  double maxValue() const;

  QString maskFileName() const;
  bool isMaskApplied() const;

public slots:
  void setMinValue(double);
  void setMaxValue(double);
  void setMaskFileName(const QString&);
  void setMaskApplied(bool);

private:
  class Private;
  Private *p;
};


class SaxsviewFrameData : public QwtRasterData {
public:
  explicit SaxsviewFrameData(const QString& fileName);
  SaxsviewFrameData(const SaxsviewFrameData& other);
  ~SaxsviewFrameData();

  SaxsviewFrameData *copy() const;

  void setMinValue(double x);
  void setMaxValue(double x);
  bool setMaskFileName(const QString&);
  void setMaskApplied(bool);

  double value(double x, double y) const;

private:
  class Private;
  Private *p;
};

#endif // !SAXSVIEWIMAGE_H

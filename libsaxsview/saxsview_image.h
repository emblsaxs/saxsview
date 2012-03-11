/*
 * Copyright (C) 2011 Daniel Franke <dfranke@users.sourceforge.net>
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
  Q_PROPERTY(bool aspectRatioFixed READ isAspectRatioFixed WRITE setAspectRatioFixed)
  Q_PROPERTY(Saxsview::Scale scale READ scale WRITE setScale)

  // Defined in QwtPlot
  // TODO: QwtTextPropertyManager + extension of QtVariantPropertyManager
//   Q_PROPERTY(QwtText title READ title WRITE setTitle)

public:
  SaxsviewImage(QWidget *parent = 0L);
  ~SaxsviewImage();

  SaxsviewFrame* frame() const;

  QRectF zoomBase() const;
  void zoom(const QRectF&);

  bool isZoomEnabled() const;
  bool isMoveEnabled() const;
  bool isAspectRatioFixed() const;

  Saxsview::Scale scale() const;

public slots:
  void setFrame(SaxsviewFrame *frame);
  void setZoomBase(const QRectF& rect = QRectF());
  void setZoomEnabled(bool);
  void setMoveEnabled(bool);
  void setAspectRatioFixed(bool);
  void setScale(Saxsview::Scale);

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

public:
  SaxsviewFrame(QObject *parent = 0L);
  ~SaxsviewFrame();

  QSize size() const;

  double minValue() const;
  double maxValue() const;

public slots:
  void setMinValue(double);
  void setMaxValue(double);

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

  double value(double x, double y) const;

private:
  class Private;
  Private *p;
};

#endif // !SAXSVIEWIMAGE_H

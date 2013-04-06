/*
 * Copyright (C) 2013 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "saxsview_colormap.h"

#include <cmath>

MaskColorMap::MaskColorMap(const QColor& c) {
  masked   = c;
  unmasked = c;
  unmasked.setAlpha(0);  // fully transparent
}

QRgb MaskColorMap::rgb(const QwtInterval&, double value) const {
  return value < 0.5 ? unmasked.rgba() : masked.rgba();
}

unsigned char MaskColorMap::colorIndex(const QwtInterval&, double) const {
  return 0;
}



QRgb HSVColorMap::rgb(const QwtInterval& interval, double value) const {
  const double min = interval.minValue();
  const double max = interval.maxValue();

  const int h = 260 * (max - value) / (max - min);
  const int s = 255;
  const int v = 255;

  return QColor::fromHsv(h, s, v).rgb();
}

unsigned char HSVColorMap::colorIndex(const QwtInterval&, double) const {
  return 0;
}



QRgb Log10HSVColorMap::rgb(const QwtInterval& interval, double x) const {
  //
  // Due to selectable thresholds it may happen that 'x' is outside the
  // range. If this is the case, we automatically get color1() if 'x'
  // is below minValue() and color2() if 'x' is above maxValue().
  //
  // I.e. if a lower threshold is defined, all pixels below that value
  // will be from-color, all those above an upper threshold will be to-color.
  //
  double min = interval.minValue() > 1.0 ? log10(interval.minValue()) : 0.0;
  double max = interval.maxValue() > 1.0 ? log10(interval.maxValue()) : 0.0;
  double val = x > 1.0 ? log10(x) : 0.0;

  return HSVColorMap::rgb(QwtInterval(min, max), val);
}

unsigned char Log10HSVColorMap::colorIndex(const QwtInterval&, double) const {
  return 0;
}



GrayColorMap::GrayColorMap()
 : QwtLinearColorMap(Qt::black, Qt::white, QwtColorMap::RGB) {
}


QRgb Log10GrayColorMap::rgb(const QwtInterval& interval, double x) const {
  double min = interval.minValue() > 1.0 ? log10(interval.minValue()) : 0.0;
  double max = interval.maxValue() > 1.0 ? log10(interval.maxValue()) : 0.0;
  double val = x > 1.0 ? log10(x) : 0.0;

  return QwtLinearColorMap::rgb(QwtInterval(min, max), val);
}

unsigned char Log10GrayColorMap::colorIndex(const QwtInterval&, double) const {
  return 0;
}

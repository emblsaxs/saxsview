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

#ifndef SAXSVIEW_COLORMAP_H
#define SAXSVIEW_COLORMAP_H

#include "qwt_color_map.h"

/**
 * A MaskColorMap is either fully transparent (mask == 0),
 * or partially transparant with a specified color (mask == 1).
 */
class MaskColorMap : public QwtColorMap {
public:
  MaskColorMap(const QColor& c);

  QRgb rgb(const QwtInterval&, double value) const;

  /** Reimplemented as it is pure virtual, but not used. */
  unsigned char colorIndex(const QwtInterval&, double) const;

private:
  QColor unmasked, masked;
};


/**
 * Circles through HSV color space; Hue is modified, Saturation
 * and Value are fixed at 255.
 */
class HSVColorMap : public QwtColorMap {
public:
  QRgb rgb(const QwtInterval& interval, double value) const;

  /** Reimplemented as it is pure virtual, but not used. */
  unsigned char colorIndex(const QwtInterval&, double) const;
};


/**
 * Same as HSVColorMap, but on logarithmic scale.
 */
class Log10HSVColorMap : public HSVColorMap {
public:
  QRgb rgb(const QwtInterval& interval, double x) const;

  /** Reimplemented as it is pure virtual, but not used. */
  unsigned char colorIndex(const QwtInterval& interval, double x) const;
};


/**
 * 
 */
class GrayColorMap : public QwtLinearColorMap {
public:
  GrayColorMap();
};


/**
 * Same as GrayColorMap, but on logarithmic scale.
 */
class Log10GrayColorMap : public GrayColorMap {
public:
  QRgb rgb(const QwtInterval& interval, double x) const;

  /** Reimplemented as it is pure virtual, but not used. */
  unsigned char colorIndex(const QwtInterval& interval, double x) const;
};

#endif // !SAXSVIEW_COLORMAP_H

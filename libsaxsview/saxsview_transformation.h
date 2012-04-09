/*
 * Copyright (C) 2012 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SAXSVIEW_TRANSFORMATION_H
#define SAXSVIEW_TRANSFORMATION_H

#include <QString>

#include "saxsview.h"

class SaxsviewTransformation {
public:
  SaxsviewTransformation();
  ~SaxsviewTransformation();

  int merge() const;
  void setMerge(int);

  double scaleX() const;
  void setScaleX(double);

  QString transformationX() const;
  void setTransformationX(const QString&);

  double scaleY() const;
  void setScaleY(double);

  QString transformationY() const;
  void setTransformationY(const QString&);

  SaxsviewPlotPointData transform(const SaxsviewPlotPointData&) const;
  SaxsviewPlotIntervalData transform(const SaxsviewPlotIntervalData&) const;

  static bool isTransformationValid(const QString&);

private:
  class Private;
  Private *p;
};

#endif // !SAXSVIEW_TRANSFORMATION_H

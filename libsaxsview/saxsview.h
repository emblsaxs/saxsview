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

#ifndef SAXSVIEW_H
#define SAXSVIEW_H

#include <QObject>
#include <QMetaType>

class Saxsview : public QObject {
  Q_OBJECT
  Q_ENUMS(Scale)
  Q_ENUMS(LineStyle SymbolStyle)

public:
  enum Scale {
    AbsoluteScale,
    Log10Scale
  };

  enum LineStyle {
    None          = Qt::NoPen,
    Solid         = Qt::SolidLine,
    Dashed        = Qt::DashLine,
    Dotted        = Qt::DotLine,
    DashDotted    = Qt::DashDotLine,
    DashDotDotted = Qt::DashDotDotLine
  };

  enum SymbolStyle {
    NoSymbol,
    Ellipse,
    Rect,
    Diamond,
    Hexagon,
    TrianglePointingDown,
    TrianglePointingUp,
    TrianglePointingLeft,
    TrianglePointingRight,
    Cross,
    XCross,
    HorizontalLine,
    VerticalLine,
    StarLine,
    StarOutline
  };
};

#endif // !SAXSVIEW_H

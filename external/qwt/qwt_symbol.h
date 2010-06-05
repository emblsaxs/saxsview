/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SYMBOL_H
#define QWT_SYMBOL_H

#include "qwt_global.h"
#include <QPolygonF>

class QPainter;
class QRect;
class QSize;
class QBrush;
class QPen;
class QColor;
class QPointF;

//! A class for drawing symbols
class QWT_EXPORT QwtSymbol
{
public:
    /*!
        Symbol Style

         - NoSymbol\n
           No Style. The symbol cannot be drawn.

         - Ellipse
           Ellipse or circle

         - Rect
           Rectangle

         - Diamond
           Diamond

         - Triangle
           Triangle pointing upwards

         - DTriangle
           Triangle pointing downwards

         - UTriangle
           Triangle pointing upwards

         - LTriangle
           Triangle pointing left

         - RTriangle
           Triangle pointing right

         - Cross
           Cross (+)

         - XCross
           Diagonal cross (X)

         - HLine
           Horizontal line

         - VLine
           Vertical line

         - Star1
           X combined with +

         - Star2
           Six-pointed star

         - Hexagon
           Hexagon

        \sa setStyle(), style()
     */
    enum Style 
    { 
        NoSymbol = -1, 

        Ellipse, 
        Rect, 
        Diamond, 
        Triangle, 
        DTriangle,
        UTriangle, 
        LTriangle, 
        RTriangle, 
        Cross, 
        XCross, 
        HLine, 
        VLine, 
        Star1, 
        Star2, 
        Hexagon, 

        UserSymbol = 1000 
    };
   
public:
    QwtSymbol(Style = NoSymbol);
    QwtSymbol(Style, const QBrush &, const QPen &, const QSize &);
    virtual ~QwtSymbol();
    
    bool operator!=(const QwtSymbol &) const;
    virtual bool operator==(const QwtSymbol &) const;

    void setSize(const QSize &);
    void setSize(int width, int height = -1);
    const QSize& size() const;

    virtual void setColor(const QColor &);

    void setBrush(const QBrush& b);
    const QBrush& brush() const;

    void setPen(const QPen &);
    const QPen& pen() const;

    void setStyle(Style);
    Style style() const;

    void drawSymbol(QPainter *, const QPointF &) const; 
    void drawSymbols(QPainter *, const QPolygonF &) const;

    virtual QSize boundingSize() const;

protected:
    virtual void drawSymbols(QPainter *, 
        const QPointF *, int numPoints ) const;

private:
    // Disabled copy constructor and operator=
    QwtSymbol( const QwtSymbol & );
    QwtSymbol &operator=( const QwtSymbol & );

    class PrivateData;
    PrivateData *d_data;
};

/*!
  \brief Draw the symbol at a specified position

  \param painter Painter
  \param pos Position of the symbol in screen coordinates
*/
inline void QwtSymbol::drawSymbol(
    QPainter *painter, const QPointF &pos) const
{
    drawSymbols(painter, &pos, 1);
}

/*!
  \brief Draw symbols at the specified points

  \param painter Painter
  \param points Positions of the symbols in screen coordinates
*/

inline void QwtSymbol::drawSymbols(
    QPainter *painter, const QPolygonF &points) const
{
    drawSymbols(painter, points.data(), points.size());
}

#endif

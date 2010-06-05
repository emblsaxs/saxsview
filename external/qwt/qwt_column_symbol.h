/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_COLUMN_SYMBOL_H
#define QWT_COLUMN_SYMBOL_H

#include "qwt_global.h"
#include "qwt_double_interval.h"
#include <qpen.h>
#include <qsize.h>
#include <qrect.h>

class QPainter;
class QPalette;
class QRect;
class QwtText;

/*!
    Directed rectangle representing bounding rectangle und orientation
    of a column.
*/
class QWT_EXPORT QwtColumnRect
{
public:
    enum Direction
    {
        LeftToRight,
        RightToLeft,
        BottomToTop,
        TopToBottom
    };

    QwtColumnRect():
        direction(BottomToTop)
    {
    }

    QRectF toRect() const
    {
        return QRectF(hInterval.minValue(), vInterval.minValue(),
            hInterval.maxValue() - hInterval.minValue(),
            vInterval.maxValue() - vInterval.minValue() ).normalized();
    }

    Qt::Orientation orientation() const
    {
        if ( direction == LeftToRight || direction == RightToLeft )
            return Qt::Horizontal;

        return Qt::Vertical;
    }

    
    QwtDoubleInterval hInterval;
    QwtDoubleInterval vInterval;
    Direction direction;
};

//! A drawing primitive for columns
class QWT_EXPORT QwtColumnSymbol
{
public:

    /*!
        Style
        \sa setStyle(), style()
     */
    enum Style 
    { 
        NoSymbol = -1, 

        Box, 

        StyleCnt 
    };
   
    enum FrameStyle
    {
        NoFrame,

        Plain,
        Raised
    };

public:
    QwtColumnSymbol(Style = NoSymbol);
    virtual ~QwtColumnSymbol();
    
    void setFrameStyle(FrameStyle style);
    FrameStyle frameStyle() const;

    void setLineWidth(int width);
    int lineWidth() const;
    
    bool operator!=(const QwtColumnSymbol &) const;
    virtual bool operator==(const QwtColumnSymbol &) const;

    void setPalette(const QPalette &);
    const QPalette &palette() const;

    void setStyle(Style);
    Style style() const;
    
    void setLabel(const QwtText&);
    const QwtText &label() const;

    virtual void draw(QPainter *, const QwtColumnRect &) const;

protected:
    void drawBox(QPainter *, const QwtColumnRect &) const;

private:
    class PrivateData;
    PrivateData* d_data;
};

#endif

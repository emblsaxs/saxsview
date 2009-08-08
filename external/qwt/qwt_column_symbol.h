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

#include <qpen.h>
#include <qsize.h>
#include "qwt_global.h"

class QPainter;
class QPalette;
class QRect;
class QwtText;

//! A drawing primitive for columns
class QWT_EXPORT QwtColumnSymbol
{
public:
    enum Direction
    {
        LeftToRight,
        RightToLeft,
        BottomToTop,
        TopToBottom
    };

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
   
public:
    QwtColumnSymbol(Style = NoSymbol);
    virtual ~QwtColumnSymbol();
    
    void setFrameStyle(int style);
    int frameStyle() const;

    void setLineWidth(int width);
    int lineWidth() const;
    
    bool operator!=(const QwtColumnSymbol &) const;
    virtual bool operator==(const QwtColumnSymbol &) const;

    virtual QwtColumnSymbol *clone() const;

    void setPalette(const QPalette &);
    const QPalette &palette() const;

    void setStyle(Style);
    Style style() const;
    
    void setLabel(const QwtText&);
    const QwtText &label() const;

    virtual void draw(QPainter *, Direction, const QRect&) const;

protected:
    void drawBox(QPainter *, Direction, const QRect&) const;

private:
    class PrivateData;
    PrivateData* d_data;
};

#endif

/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_INTERVAL_SYMBOL_H
#define QWT_INTERVAL_SYMBOL_H

#include <qpen.h>
#include <qsize.h>
#include "qwt_global.h"

class QPainter;
class QRect;

//! A drawing primitive for bars
class QWT_EXPORT QwtIntervalSymbol
{
public:
    /*!
        Style
        \sa setStyle(), style()
     */
    enum Style 
    { 
        NoSymbol = -1, 

        Bar, 
        Box, 

        StyleCnt 
    };
   
public:
    QwtIntervalSymbol(Style = NoSymbol);
    virtual ~QwtIntervalSymbol();
    
    bool operator!=(const QwtIntervalSymbol &) const;
    virtual bool operator==(const QwtIntervalSymbol &) const;

    virtual QwtIntervalSymbol *clone() const;

    void setWidth(int);
    int width() const;

    void setBrush(const QBrush& b);
    const QBrush& brush() const;

    void setPen(const QPen &);
    const QPen& pen() const; 

    void setStyle(Style);
    Style style() const;
    
    virtual void draw(QPainter *, 
        const QPoint& from, const QPoint& to) const;

private:
    class PrivateData;
    PrivateData* d_data;
};

#endif

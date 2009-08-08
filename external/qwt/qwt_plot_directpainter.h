/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_DIRECT_PAINTER_H
#define QWT_PLOT_DIRECT_PAINTER_H

#include "qwt_global.h"
#include <qobject.h>

class QwtPlotAbstractSeriesItem;

class QWT_EXPORT QwtPlotDirectPainter: public QObject
{
public:
    enum Attribute
    {
        AtomicPainter = 1,
        FullRepaint = 2
    };

    QwtPlotDirectPainter(QObject *parent = NULL);
    virtual ~QwtPlotDirectPainter();

    void setAttribute(Attribute, bool on);
    bool testAttribute(Attribute) const;

    void drawSeries(QwtPlotAbstractSeriesItem *, int from, int to);
    void reset();

    virtual bool eventFilter(QObject *, QEvent *);

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif

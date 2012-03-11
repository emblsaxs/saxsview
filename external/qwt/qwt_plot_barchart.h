/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_BAR_CHART_H
#define QWT_PLOT_BAR_CHART_H

#include "qwt_global.h"
#include "qwt_plot_baritem.h"
#include "qwt_series_data.h"

class QwtColumnRect;
class QwtColumnSymbol;

class QWT_EXPORT QwtPlotBarChart:
    public QwtPlotBarItem, public QwtSeriesStore<QPointF>
{
public:
    explicit QwtPlotBarChart( const QString &title = QString::null );
    explicit QwtPlotBarChart( const QwtText &title );

    virtual ~QwtPlotBarChart();

    virtual int rtti() const;

    void setSamples( const QVector<QPointF> & );
    void setSamples( const QVector<double> & );

    void setSymbol( QwtColumnSymbol * );
    const QwtColumnSymbol *symbol() const;

    virtual void drawSeries( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

    virtual QRectF boundingRect() const;

protected:
    virtual void drawSample( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, const QwtInterval &boundingInterval,
        int index, const QPointF& sample ) const;

    virtual void drawBar( QPainter *,
        int sampleIndex, const QwtColumnRect & ) const;

    virtual void drawLabel( QPainter *, int sampleIndex,
        const QwtColumnRect &, const QwtText & ) const;

    virtual QwtText label( int sampleIndex, const QPointF & ) const;

private:
    void init();

    class PrivateData;
    PrivateData *d_data;
};

#endif

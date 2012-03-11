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
#include <qlist.h>

class QwtColumnRect;
class QwtColumnSymbol;

class QWT_EXPORT QwtPlotMultiBarChart: 
    public QwtPlotBarItem, public QwtSeriesStore<QwtSetSample>
{
public:
    enum ChartStyle
    {
        Stacked,
        Grouped
    };

    explicit QwtPlotMultiBarChart( const QString &title = QString::null );
    explicit QwtPlotMultiBarChart( const QwtText &title );

    virtual ~QwtPlotMultiBarChart();

    virtual int rtti() const;

    void setTitles( const QList<QwtText> & );
    QList<QwtText> titles() const;

    void setSamples( const QVector<QwtSetSample> & );
    void setSamples( const QVector< QVector<double> > & );

    void setStyle( ChartStyle style );
    ChartStyle style() const;

    void setColorTable( const QList<QBrush> &colorTable );
    QList<QBrush> colorTable() const;

    void setSymbol( int barIndex, QwtColumnSymbol *symbol );
    void clearSymbols();

    const QwtColumnSymbol *symbol( int barIndex ) const;

    virtual void drawSeries( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

    virtual QRectF boundingRect() const;

    virtual QList<QwtLegendData> legendData() const;

    virtual QwtGraphic legendIcon( int index, const QSizeF & ) const;

protected:
    QwtColumnSymbol *symbol( int barIndex );

    virtual void drawSample( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, const QwtInterval &boundingInterval,
        int index, const QwtSetSample& sample ) const;

    virtual void drawBar( QPainter *, int sampleIndex,
        int barIndex, const QwtColumnRect & ) const;

    virtual void drawLabel( QPainter *, int sampleIndex,
        int barIndex, const QwtColumnRect &, const QwtText & ) const;

    virtual QwtText label( int sampleIndex, int barIndex,
        const QwtSetSample& ) const;

    void drawStackedBars( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int index,
        double sampleWidth, const QwtSetSample& sample ) const;

    void drawGroupedBars( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int index,
        double sampleWidth, const QwtSetSample& sample ) const;

private:
    void init();

    class PrivateData;
    PrivateData *d_data;
};

#endif

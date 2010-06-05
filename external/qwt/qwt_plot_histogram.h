/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_HISTOGRAM_H
#define QWT_PLOT_HISTOGRAM_H

#include "qwt_global.h" 
#include "qwt_plot_seriesitem.h" 
#include "qwt_column_symbol.h" 
#include <qcolor.h>
#include <qvector.h>

class QwtIntervalData;
class QString;
class QPolygonF;

class QWT_EXPORT QwtPlotHistogram: public QwtPlotSeriesItem<QwtIntervalSample>
{
public:
    enum CurveStyle
    {
        NoCurve,

        Outline,

        Columns,
        Lines,

        UserCurve = 100
    };

    explicit QwtPlotHistogram(const QString &title = QString::null);
    explicit QwtPlotHistogram(const QwtText &title);
    virtual ~QwtPlotHistogram();

    virtual int rtti() const;

    void setPen(const QPen &);
    const QPen &pen() const;

    void setBrush(const QBrush &);
    const QBrush &brush() const;

    void setSamples(const QVector<QwtIntervalSample> &);

    void setBaseline(double reference);
    double baseline() const;

    void setStyle(CurveStyle style);
    CurveStyle style() const;

    void setSymbol(const QwtColumnSymbol *);
    const QwtColumnSymbol *symbol() const;

    virtual void drawSeries(QPainter *p,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to) const;

    virtual QRectF boundingRect() const;

    virtual void drawLegendIdentifier(QPainter *, const QRectF &) const;

protected:
    virtual QwtColumnRect columnRect(const QwtIntervalSample &,
        const QwtScaleMap &, const QwtScaleMap &) const;

    virtual void drawColumn(QPainter *, const QwtColumnRect &,
        const QwtIntervalSample & ) const;

    void drawColumns(QPainter *,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

    void drawOutline(QPainter *,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

    void drawLines(QPainter *,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

private:
    void init();
    void flushPolygon(QPainter *, double baseLine, QPolygonF &) const;

    class PrivateData;
    PrivateData *d_data;
};

#endif

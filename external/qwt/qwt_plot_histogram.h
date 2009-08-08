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

#include <qglobal.h>
#include <qcolor.h>

#include "qwt_polygon.h" 
#include "qwt_plot_seriesitem.h" 
#include "qwt_column_symbol.h" 

class QwtIntervalData;
class QString;

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

    void setData(const QwtArray<QwtIntervalSample> &data);
    void setData(const QwtSeriesData<QwtIntervalSample> &data);

    void setBaseline(double reference);
    double baseline() const;

    void setStyle(CurveStyle style);
    CurveStyle style() const;

    void setSymbol(const QwtColumnSymbol &);
    const QwtColumnSymbol &symbol() const;

    virtual void drawSeries(QPainter *p,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &canvasRect, int from, int to) const;

    virtual QwtDoubleRect boundingRect() const;
    virtual void updateLegend(QwtLegend *) const;

protected:
    virtual QRect columnRect(const QwtIntervalSample &,
        const QwtScaleMap &, const QwtScaleMap &,
        QwtColumnSymbol::Direction &) const;

    virtual void drawColumn(QPainter *, const QRect &, 
        QwtColumnSymbol::Direction, const QwtIntervalSample &) const;

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
#if QT_VERSION < 0x040000
    void flushPolygon(QPainter *, int baseLine, QValueList<QPoint> &) const;
    void drawPolygon(QPainter *, const QValueList<QPoint>&) const;
#else
    void flushPolygon(QPainter *, int baseLine, QwtPolygon &) const;
#endif

    class PrivateData;
    PrivateData *d_data;
};

#endif

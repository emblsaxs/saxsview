/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_histogram.h"
#include "qwt_plot.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include "qwt_painter.h"
#include "qwt_column_symbol.h"
#include "qwt_scale_map.h"
#include <qstring.h>
#include <qpainter.h>

class QwtPlotHistogram::PrivateData
{
public:
    PrivateData():
        reference(0.0),
        curveStyle(NoCurve),
        symbol(NULL)
    {
    }

    ~PrivateData()
    {
        delete symbol;
    }

    double reference;

    QPen pen;
    QBrush brush;
    QwtPlotHistogram::CurveStyle curveStyle;
    const QwtColumnSymbol *symbol;
};

QwtPlotHistogram::QwtPlotHistogram(const QwtText &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

QwtPlotHistogram::QwtPlotHistogram(const QString &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

QwtPlotHistogram::~QwtPlotHistogram()
{
    delete d_data;
}

void QwtPlotHistogram::init()
{
    d_data = new PrivateData();
    d_series = new QwtIntervalSeriesData();

    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, true);

    setZ(20.0);
}

void QwtPlotHistogram::setStyle(CurveStyle style)
{
    if ( style != d_data->curveStyle )
    {
        d_data->curveStyle = style;
        itemChanged();
    }
}

QwtPlotHistogram::CurveStyle QwtPlotHistogram::style() const
{
    return d_data->curveStyle;
}

void QwtPlotHistogram::setPen(const QPen &pen)
{
    if ( pen != d_data->pen )
    {
        d_data->pen = pen;
        itemChanged();
    }
}

const QPen &QwtPlotHistogram::pen() const
{
    return d_data->pen;
}

void QwtPlotHistogram::setBrush(const QBrush &brush)
{
    if ( brush != d_data->brush )
    { 
        d_data->brush = brush;
        itemChanged();
    }
}

const QBrush &QwtPlotHistogram::brush() const
{
    return d_data->brush; 
}

void QwtPlotHistogram::setSymbol(const QwtColumnSymbol *symbol)
{
    if ( symbol != d_data->symbol )
    {
        delete d_data->symbol;
        d_data->symbol = symbol;
        itemChanged();
    }
}

const QwtColumnSymbol *QwtPlotHistogram::symbol() const
{
    return d_data->symbol;
}

void QwtPlotHistogram::setBaseline(double reference)
{
    if ( d_data->reference != reference )
    {
        d_data->reference = reference;
        itemChanged();
    }
}

double QwtPlotHistogram::baseline() const
{
    return d_data->reference;
}

QRectF QwtPlotHistogram::boundingRect() const
{
    QRectF rect = d_series->boundingRect();
    if ( !rect.isValid() ) 
        return rect;

    if ( orientation() == Qt::Horizontal )
    {
        rect = QRectF( rect.y(), rect.x(), 
            rect.height(), rect.width() );

        if ( rect.left() > d_data->reference ) 
            rect.setLeft( d_data->reference );
        else if ( rect.right() < d_data->reference ) 
            rect.setRight( d_data->reference );
    } 
    else 
    {
        if ( rect.bottom() < d_data->reference ) 
            rect.setBottom( d_data->reference );
        else if ( rect.top() > d_data->reference ) 
            rect.setTop( d_data->reference );
    }

    return rect;
}


int QwtPlotHistogram::rtti() const
{
    return QwtPlotItem::Rtti_PlotHistogram;
}

void QwtPlotHistogram::setSamples(
    const QVector<QwtIntervalSample> &data)
{
    delete d_series;
    d_series = new QwtIntervalSeriesData(data);
    itemChanged();
}

void QwtPlotHistogram::drawSeries(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &, int from, int to) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = dataSize() - 1;

    switch (d_data->curveStyle)
    {
        case Outline:
            drawOutline(painter, xMap, yMap, from, to);
            break;
        case Lines:
            drawLines(painter, xMap, yMap, from, to);
            break;
        case Columns:
            drawColumns(painter, xMap, yMap, from, to);
            break;
        case NoCurve:
        default:
            break;
    }
}

void QwtPlotHistogram::drawOutline(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
    const double v0 = (orientation() == Qt::Horizontal) ?
        xMap.transform(baseline()) : yMap.transform(baseline());

    QwtIntervalSample previous;

    QPolygonF polygon;
    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = d_series->sample(i);

        if ( !sample.interval.isValid() )
        {
            flushPolygon(painter, v0, polygon);
            previous = sample;
            continue;
        }

        if ( previous.interval.isValid() && 
            previous.interval.maxValue() != sample.interval.minValue() )
        {
            flushPolygon(painter, v0, polygon);
        }

        if ( orientation() == Qt::Vertical )
        {
            const double x1 = xMap.transform( sample.interval.minValue());
            const double x2 = xMap.transform( sample.interval.maxValue());
            const double y = yMap.transform(sample.value);

            if ( polygon.size() == 0 )
                polygon += QPointF(x1, v0);

            polygon += QPointF(x1, y);
            polygon += QPointF(x2, y);
        }
        else
        {
            const double y1 = yMap.transform( sample.interval.minValue());
            const double y2 = yMap.transform( sample.interval.maxValue());
            const double x = xMap.transform(sample.value);

            if ( polygon.size() == 0 )
                polygon += QPointF(v0, y1);

            polygon += QPointF(x, y1);
            polygon += QPointF(x, y2);
        }
        previous = sample;
    }

    flushPolygon(painter, v0, polygon);
}

void QwtPlotHistogram::drawColumns(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
    painter->setPen(d_data->pen);
    painter->setBrush(d_data->brush);

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = d_series->sample(i);
        if ( !sample.interval.isNull() )
        { 
            const QwtColumnRect rect = columnRect(sample, xMap, yMap);
            drawColumn(painter, rect, sample);
        }
    }
}

void QwtPlotHistogram::drawLines(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
    painter->setPen(d_data->pen);
    painter->setBrush(Qt::NoBrush);

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = d_series->sample(i);
        if ( !sample.interval.isNull() )
        { 
            const QwtColumnRect rect = columnRect(sample, xMap, yMap);
            const QRectF r = rect.toRect();

            switch(rect.direction)
            {
                case QwtColumnRect::LeftToRight:
                {
                    QwtPainter::drawLine(painter, 
                        r.topRight(), r.bottomRight());
                    break;
                }
                case QwtColumnRect::RightToLeft:
                {
                    QwtPainter::drawLine(painter, 
                        r.topLeft(), r.bottomLeft());
                    break;
                }
                case QwtColumnRect::TopToBottom:
                {
                    QwtPainter::drawLine(painter, 
                        r.bottomRight(), r.bottomLeft());
                    break;
                }
                case QwtColumnRect::BottomToTop:
                {
                    QwtPainter::drawLine(painter, 
                        r.topRight(), r.topLeft());
                    break;
                }
            }
        }
    }
}

void QwtPlotHistogram::flushPolygon(QPainter *painter, 
    double baseLine, QPolygonF &polygon ) const
{
    if ( polygon.size() == 0 )
        return;

    if ( orientation() == Qt::Horizontal )
        polygon += QPointF(baseLine, polygon.last().y());
    else
        polygon += QPointF(polygon.last().x(), baseLine);

    if ( d_data->brush.style() != Qt::NoBrush )
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(d_data->brush);

        if ( orientation() == Qt::Horizontal )
        {
            polygon += QPointF(polygon.last().x(), baseLine);
            polygon += QPointF(polygon.first().x(), baseLine);
        }
        else
        {
            polygon += QPointF(baseLine, polygon.last().y());
            polygon += QPointF(baseLine, polygon.first().y());
        }
        QwtPainter::drawPolygon(painter, polygon);
        polygon.resize(polygon.size() - 2);
    }
    if ( d_data->pen.style() != Qt::NoPen )
    {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(d_data->pen);
        QwtPainter::drawPolyline(painter, polygon);
    }
    polygon.clear();
}

QwtColumnRect QwtPlotHistogram::columnRect(const QwtIntervalSample &sample,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap) const
{
    QwtColumnRect rect;

    const QwtDoubleInterval &iv = sample.interval;
    if ( !iv.isValid() )
        return rect;

    if ( orientation() == Qt::Horizontal )
    {
        const double x0 = xMap.transform(baseline());
        const double x  = xMap.transform(sample.value);
        const double y1 = yMap.transform( iv.minValue());
        const double y2 = yMap.transform( iv.maxValue());

        rect.hInterval.setInterval(x0, x);
        rect.vInterval.setInterval(y1, y2, iv.borderFlags());
        rect.direction = (x < x0) ? QwtColumnRect::RightToLeft :
            QwtColumnRect::LeftToRight;
    }
    else
    {
        const double x1 = xMap.transform( iv.minValue());
        const double x2 = xMap.transform( iv.maxValue());
        const double y0 = yMap.transform(baseline());
        const double y = yMap.transform(sample.value);

        rect.hInterval.setInterval(x1, x2, iv.borderFlags());
        rect.vInterval.setInterval(y0, y);
        rect.direction = (y < y0) ? QwtColumnRect::BottomToTop :
            QwtColumnRect::TopToBottom;
    }

    return rect;
}

void QwtPlotHistogram::drawColumn(QPainter *painter, 
    const QwtColumnRect &rect, const QwtIntervalSample &) const
{
    if ( d_data->symbol &&
        ( d_data->symbol->style() != QwtColumnSymbol::NoSymbol ) )
    {
        d_data->symbol->draw(painter, rect);
    }
    else
    {
        const QRectF r = rect.toRect();
        QwtPainter::drawRect(painter, r.adjusted(0, 0, -1, -1));
    }
}

void QwtPlotHistogram::drawLegendIdentifier(
    QPainter *painter, const QRectF &rect) const
{
    const double dim = qMin(rect.width(), rect.height());

    QSizeF size(dim, dim);

    QRectF r(0, 0, size.width(), size.height());
    r.moveCenter(rect.center());

    painter->fillRect(r, d_data->brush);
}

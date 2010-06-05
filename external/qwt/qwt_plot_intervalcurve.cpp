/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_intervalcurve.h"
#include "qwt_interval_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"

#include <qpainter.h>

class QwtPlotIntervalCurve::PrivateData
{
public:
    PrivateData():
        curveStyle(Tube),
        symbol(NULL),
        pen(Qt::black),
        brush(Qt::white)
    {
        pen.setCapStyle(Qt::FlatCap);
    }

    ~PrivateData()
    {
        delete symbol;
    }

    CurveStyle curveStyle;
    const QwtIntervalSymbol *symbol;

    QPen pen;
    QBrush brush;
};

/*!
  \brief Ctor
  \param title title of the curve   
*/
QwtPlotIntervalCurve::QwtPlotIntervalCurve(const QwtText &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

/*!
  \brief Ctor
  \param title title of the curve   
*/
QwtPlotIntervalCurve::QwtPlotIntervalCurve(const QString &title):
    QwtPlotSeriesItem<QwtIntervalSample>(QwtText(title))
{
    init();
}

//! Dtor
QwtPlotIntervalCurve::~QwtPlotIntervalCurve()
{
    delete d_data;
}

/*!
  \brief Initialize data members
*/
void QwtPlotIntervalCurve::init()
{
    setItemAttribute(QwtPlotItem::Legend, true);
    setItemAttribute(QwtPlotItem::AutoScale, true);

    d_data = new PrivateData;
    d_series = new QwtIntervalSeriesData();

    setZ(19.0);
}

int QwtPlotIntervalCurve::rtti() const
{
    return QwtPlotIntervalCurve::Rtti_PlotIntervalCurve;
}

void QwtPlotIntervalCurve::setSamples(
    const QVector<QwtIntervalSample> &data)
{
    delete d_series;
    d_series = new QwtIntervalSeriesData(data);
    itemChanged();
}

void QwtPlotIntervalCurve::setCurveStyle(CurveStyle style)
{
    if ( style != d_data->curveStyle )
    {
        d_data->curveStyle = style;
        itemChanged();
    }
}

/*!
    \brief Return the current style
    \sa setStyle()
*/
QwtPlotIntervalCurve::CurveStyle QwtPlotIntervalCurve::curveStyle() const 
{ 
    return d_data->curveStyle; 
}

void QwtPlotIntervalCurve::setSymbol(const QwtIntervalSymbol *symbol)
{
    if ( symbol != d_data->symbol )
    {
        delete d_data->symbol;
        d_data->symbol = symbol;
        itemChanged();
    }
}

const QwtIntervalSymbol *QwtPlotIntervalCurve::symbol() const 
{ 
    return d_data->symbol; 
}

/*!
  \brief Assign a pen
  \param p New pen
  \sa pen(), brush()
*/
void QwtPlotIntervalCurve::setPen(const QPen &p)
{
    if ( p != d_data->pen )
    {
        d_data->pen = p;
        itemChanged();
    }
}

/*!
    \brief Return the pen used to draw the lines
    \sa setPen(), brush()
*/
const QPen& QwtPlotIntervalCurve::pen() const 
{ 
    return d_data->pen; 
}

void QwtPlotIntervalCurve::setBrush(const QBrush &brush)
{
    if ( brush != d_data->brush )
    {
        d_data->brush = brush;
        itemChanged();
    }
}

const QBrush& QwtPlotIntervalCurve::brush() const 
{
    return d_data->brush;
}

QRectF QwtPlotIntervalCurve::boundingRect() const
{
    QRectF br = QwtPlotSeriesItem<QwtIntervalSample>::boundingRect();
    if ( br.isValid() )
    {
        if ( orientation() == Qt::Vertical )
            br.setRect(br.y(), br.x(), br.height(), br.width());
        else
            br.setRect(br.x(), br.y(), br.width(), br.height());
    }

    return br;
}

void QwtPlotIntervalCurve::drawSeries(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    const QRectF &, int from, int to) const
{
    if (to < 0)
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from >= to )
        return;
        
    switch(d_data->curveStyle)
    {
        case Tube:
            drawTube(painter, xMap, yMap, from, to);
            break;
        case NoCurve:
        default:
            break;
    }

    if ( d_data->symbol &&
        ( d_data->symbol->style() != QwtIntervalSymbol::NoSymbol ) )
    {
        drawSymbols(painter, *d_data->symbol, xMap, yMap, from, to);
    }
}

void QwtPlotIntervalCurve::drawTube(QPainter *painter, 
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    painter->save();

    const size_t size = to - from + 1;
    QPolygonF polygon(2 * size);
    QPointF *points = polygon.data();

    for ( uint i = 0; i < size; i++ )
    {
        QPointF &minValue = points[i];
        QPointF &maxValue = points[2 * size - 1 - i];

        const QwtIntervalSample intervalSample = sample(from + i);
        if ( orientation() == Qt::Vertical )
        {
            const double x = xMap.transform(intervalSample.value);
            const double y1 = yMap.transform(intervalSample.interval.minValue());
            const double y2 = yMap.transform(intervalSample.interval.maxValue());

            minValue.rx() = x;
            minValue.ry() = y1;
            maxValue.rx() = x;
            maxValue.ry() = y2;
        }
        else
        {
            const double y = yMap.transform(intervalSample.value);
            const double x1 = xMap.transform(intervalSample.interval.minValue());
            const double x2 = xMap.transform(intervalSample.interval.maxValue());

            minValue.rx() = x1;
            minValue.ry() = y;
            maxValue.rx() = x2;
            maxValue.ry() = y;
        }
    }

    if ( d_data->brush.style() != Qt::NoBrush )
    {
        painter->setPen(QPen(Qt::NoPen));
        painter->setBrush(d_data->brush);
        QwtPainter::drawPolygon(painter, polygon);
    }

    if ( d_data->pen.style() != Qt::NoPen )
    {
        painter->setPen(d_data->pen);
        painter->setBrush(Qt::NoBrush);

        QwtPainter::drawPolyline(painter, points, size);
        QwtPainter::drawPolyline(painter, points + size, size);
    }

    painter->restore();
}

void QwtPlotIntervalCurve::drawSymbols(
    QPainter *painter, const QwtIntervalSymbol &symbol,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    painter->save();

    QPen pen = symbol.pen();
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(symbol.brush());

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample intervalSample = sample(i);

        if ( orientation() == Qt::Vertical )
        {
            const double x = xMap.transform(intervalSample.value);
            const double y1 = yMap.transform(intervalSample.interval.minValue());
            const double y2 = yMap.transform(intervalSample.interval.maxValue());

            symbol.draw(painter, QPointF(x, y1), QPointF(x, y2));
        }
        else
        {
            const double y = yMap.transform(intervalSample.value);
            const double x1 = xMap.transform(intervalSample.interval.minValue());
            const double x2 = xMap.transform(intervalSample.interval.maxValue());

            symbol.draw(painter, QPointF(x1, y), QPointF(x2, y));
        }
    }

    painter->restore();
}

void QwtPlotIntervalCurve::drawLegendIdentifier(
    QPainter *painter, const QRectF &rect) const
{
    const double dim = qMin(rect.width(), rect.height());

    QSizeF size(dim, dim);

    QRectF r(0, 0, size.width(), size.height());
    r.moveCenter(rect.center());

    if (d_data->curveStyle == Tube)
    {
        painter->fillRect(r, d_data->brush);
    }

    if ( d_data->symbol &&
        ( d_data->symbol->style() != QwtIntervalSymbol::NoSymbol ) )
    {
        QPen pen = d_data->symbol->pen();
        pen.setWidthF(pen.widthF());
        pen.setCapStyle(Qt::FlatCap);

        painter->setPen(pen);
        painter->setBrush(d_data->symbol->brush());

        if ( orientation() == Qt::Vertical )
        {
            d_data->symbol->draw(painter,
                QPointF(r.center().x(), r.top()), 
                QPointF(r.center().x(), r.bottom()) );
        }
        else
        {
            d_data->symbol->draw(painter,
                QPointF(r.left(), r.center().y()), 
                QPointF(r.right(), r.center().y()) );
        }
    }
}

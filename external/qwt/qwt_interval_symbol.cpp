/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_interval_symbol.h"
#include "qwt_painter.h"
#include "qwt_math.h"
#include <qpainter.h>

#if QT_VERSION < 0x040601
#define qAtan2(y, x) ::atan2(y, x) 
#endif  

class QwtIntervalSymbol::PrivateData
{
public:
    PrivateData():
        style(QwtIntervalSymbol::NoSymbol),
        width(6)
    {
    }

    QwtIntervalSymbol::Style style;
    int width;

    QPen pen;
    QBrush brush;
};

QwtIntervalSymbol::QwtIntervalSymbol(Style style) 
{
    d_data = new PrivateData();
    d_data->style = style;
}

QwtIntervalSymbol::~QwtIntervalSymbol()
{
    delete d_data;
}

//! == operator
bool QwtIntervalSymbol::operator==(const QwtIntervalSymbol &other) const
{
    return d_data->style == other.d_data->style &&
        d_data->width == other.d_data->width &&
        d_data->pen == other.d_data->pen &&
        d_data->brush == other.d_data->brush;
}

//! != operator
bool QwtIntervalSymbol::operator!=(const QwtIntervalSymbol &other) const
{
    return !(*this == other);
}

void QwtIntervalSymbol::setStyle(Style style)
{
    d_data->style = style;
}

QwtIntervalSymbol::Style QwtIntervalSymbol::style() const
{
    return d_data->style;
}

void QwtIntervalSymbol::setWidth(int width)
{
    d_data->width = width;
}

int QwtIntervalSymbol::width() const
{
    return d_data->width;
}

void QwtIntervalSymbol::setBrush(const QBrush &brush)
{
    d_data->brush = brush;
}

const QBrush& QwtIntervalSymbol::brush() const
{
    return d_data->brush;
}

void QwtIntervalSymbol::setPen(const QPen &pen)
{
    d_data->pen = pen;
}

const QPen& QwtIntervalSymbol::pen() const
{
    return d_data->pen;
}

void QwtIntervalSymbol::draw(QPainter *painter, 
        const QPointF &from, const QPointF &to) const
{
    const double pw = qMax(painter->pen().widthF(), 1.0);

    switch(d_data->style)
    {
        case QwtIntervalSymbol::Bar:
        {
            QwtPainter::drawLine(painter, from, to);
            if ( d_data->width > pw )
            {
                if ( from.y() == to.y() )
                {
                    const double sw = d_data->width;

                    const double y = from.y() - sw / 2;
                    QwtPainter::drawLine(painter,
                        from.x(), y, from.x(), y + sw);
                    QwtPainter::drawLine(painter,
                        to.x(), y, to.x(), y + sw);
                }
                else if ( from.x() == to.x() )
                {
                    const double sw = d_data->width;

                    const double x = from.x() - sw / 2;
                    QwtPainter::drawLine(painter,
                        x, from.y(), x + sw, from.y());
                    QwtPainter::drawLine(painter,
                        x, to.y(), x + sw, to.y());
                }
                else    
                {
                    const double sw = d_data->width;

                    const double dx = to.x() - from.x();
                    const double dy = to.y() - from.y();
                    const double angle = qAtan2(dy, dx) + M_PI_2;
                    double dw2 = sw / 2.0;

                    const double cx = qCos(angle) * dw2;
                    const double sy = qSin(angle) * dw2;

                    QwtPainter::drawLine(painter, 
                        from.x() - cx, from.y() - sy,
                        from.x() + cx, from.y() + sy);
                    QwtPainter::drawLine(painter, 
                        to.x() - cx, to.y() - sy,
                        to.x() + cx, to.y() + sy);
                }
            }
            break;
        }
        case QwtIntervalSymbol::Box:
        {
            if ( d_data->width <= pw )
            {
                QwtPainter::drawLine(painter, from, to);
            }
            else
            {
                if ( from.y() == to.y() )
                {
                    const double sw = d_data->width;

                    const double y = from.y() - d_data->width / 2;
                    QwtPainter::drawRect(painter,
                        from.x(), y, to.x() - from.x(),  sw);
                }
                else if ( from.x() == to.x() )
                {
                    const double sw = d_data->width;

                    const double x = from.x() - d_data->width / 2;
                    QwtPainter::drawRect(painter,
                        x, from.y(), sw, to.y() - from.y() );
                }
                else
                {
                    const double sw = d_data->width;

                    const double dx = to.x() - from.x();
                    const double dy = to.y() - from.y();
                    const double angle = qAtan2(dy, dx) + M_PI_2;
                    double dw2 = sw / 2.0;

                    const int cx = qCos(angle) * dw2;
                    const int sy = qSin(angle) * dw2;

                    QPolygonF polygon;
                    polygon += QPointF(from.x() - cx, from.y() - sy);
                    polygon += QPointF(from.x() + cx, from.y() + sy);
                    polygon += QPointF(to.x() + cx, to.y() + sy);
                    polygon += QPointF(to.x() - cx, to.y() - sy);

                    QwtPainter::drawPolygon(painter, polygon);
                }
            }
            break;
        }
        default:;
    }
}

/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include "qwt_math.h"
#include "qwt_interval_symbol.h"
#include "qwt_painter.h"

class QwtIntervalSymbol::PrivateData
{
public:
    PrivateData():
        style(QwtIntervalSymbol::NoSymbol),
        width(5)
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

QwtIntervalSymbol *QwtIntervalSymbol::clone() const
{
    QwtIntervalSymbol *other = new QwtIntervalSymbol;
    *other->d_data = *d_data;

    return other;
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
        const QPoint& from, const QPoint& to) const
{
    switch(d_data->style)
    {
        case QwtIntervalSymbol::Bar:
        {
            const int pw = qwtMax(painter->pen().width(), 1);

            QwtPainter::drawLine(painter, from, to);
            if ( d_data->width > pw )
            {
                if ( from.y() == to.y() )
                {
                    const int y = from.y() - d_data->width / 2;
                    QwtPainter::drawLine(painter,
                        from.x(), y, from.x(), y + d_data->width);
                    QwtPainter::drawLine(painter,
                        to.x(), y, to.x(), y + d_data->width);
                }
                else if ( from.x() == to.x() )
                {
                    const int x = from.x() - d_data->width / 2;
                    QwtPainter::drawLine(painter,
                        x, from.y(), x + d_data->width, from.y());
                    QwtPainter::drawLine(painter,
                        x, to.y(), x + d_data->width, to.y());
                }
                else    
                {
#ifdef __GNUC__
#warning TODO
#endif
                }
            }
            break;
        }
        case QwtIntervalSymbol::Box:
        {
            if ( from.y() == to.y() )
            {
                const int y = from.y() - d_data->width / 2;
                QwtPainter::drawRect(painter,
                    from.x(), y, to.x() - from.x(),  d_data->width);
            }
            else if ( from.x() == to.x() )
            {
                const int x = from.x() - d_data->width / 2;
                QwtPainter::drawRect(painter,
                    x, from.y(), d_data->width, to.y() - from.y() );
            }
            else
            {
#ifdef __GNUC__
#warning TODO
#endif
            }
            break;
        }
        default:;
    }
}

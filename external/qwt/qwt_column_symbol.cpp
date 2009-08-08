/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include <qpalette.h>
#include <qframe.h>
#include "qwt_math.h"
#include "qwt_text.h"
#include "qwt_column_symbol.h"
#include "qwt_painter.h"
#if QT_VERSION < 0x040000
#include <qdrawutil.h>
#endif

class QwtColumnSymbol::PrivateData
{
public:
    PrivateData():
        style(QwtColumnSymbol::Box),
        lineWidth(2),
        frameStyle(QFrame::Box | QFrame::Raised)
    {
        palette = QPalette(Qt::gray);
    }

    QwtColumnSymbol::Style style;

    QPalette palette;
    QwtText label;

    int lineWidth;
    int frameStyle;
};

QwtColumnSymbol::QwtColumnSymbol(Style style) 
{
    d_data = new PrivateData();
    d_data->style = style;
}

QwtColumnSymbol::~QwtColumnSymbol()
{
    delete d_data;
}

QwtColumnSymbol *QwtColumnSymbol::clone() const
{
    QwtColumnSymbol *other = new QwtColumnSymbol;
    *other->d_data = *d_data;

    return other;
}

//! == operator
bool QwtColumnSymbol::operator==(const QwtColumnSymbol &other) const
{
    return d_data->style == other.d_data->style &&
        d_data->palette == other.d_data->palette &&
        d_data->label == other.d_data->label &&
        d_data->lineWidth == other.d_data->lineWidth &&
        d_data->frameStyle == other.d_data->frameStyle;
}

//! != operator
bool QwtColumnSymbol::operator!=(const QwtColumnSymbol &other) const
{
    return !(*this == other);
}

void QwtColumnSymbol::setStyle(Style style)
{
    d_data->style = style;
}

QwtColumnSymbol::Style QwtColumnSymbol::style() const
{
    return d_data->style;
}

void QwtColumnSymbol::setPalette(const QPalette &palette)
{
    d_data->palette = palette;
}

const QPalette& QwtColumnSymbol::palette() const
{
    return d_data->palette;
}

void QwtColumnSymbol::setFrameStyle(int style)
{
    d_data->frameStyle = style;
}

int QwtColumnSymbol::frameStyle() const
{
    return d_data->frameStyle;
}

void QwtColumnSymbol::setLineWidth(int width)
{
    d_data->lineWidth = width;
}

int QwtColumnSymbol::lineWidth() const
{
    return d_data->lineWidth;
}


void QwtColumnSymbol::setLabel(const QwtText &label)
{
    d_data->label = label;
}

const QwtText& QwtColumnSymbol::label() const
{
    return d_data->label;
}

void QwtColumnSymbol::draw(QPainter *painter, 
    Direction direction, const QRect &rect) const
{
#if QT_VERSION >= 0x040000
    const QRect r = rect.normalized();
#else
    const QRect r = rect.normalize();
#endif
    painter->save();

    switch(d_data->style)
    {
        case QwtColumnSymbol::Box:
        {
            drawBox(painter, direction, r);
            break;
        }
        default:;
    }

    painter->restore();
}

void QwtColumnSymbol::drawBox(QPainter *painter, 
    Direction, const QRect &rect) const
{
#if QT_VERSION < 0x040000
    QRect r = rect.normalize();
#else
    QRect r = rect.normalized();
#endif
    r = QwtPainter::metricsMap().layoutToDevice(r, painter);

    r.setTop(r.top() + 1);
    r.setRight(r.right() + 1);

#if QT_VERSION < 0x040000
    const int shadowMask = 0x00f0;
    const int shapeMask = 0x000f;
#else
    const int shadowMask = QFrame::Shadow_Mask;
    const int shapeMask = QFrame::Shape_Mask;
#endif
    int shadow = d_data->frameStyle & shadowMask;
    if ( shadow == 0 )
        shadow = QFrame::Plain;

    int shape = d_data->frameStyle & shapeMask;
    if ( shadow == QFrame::Plain )
        shape = QFrame::Box;

    const QBrush brush =
#if QT_VERSION < 0x040000
        d_data->palette.brush(QPalette::Active, QColorGroup::Background);
#else
        d_data->palette.brush(QPalette::Window);
#endif

    switch(d_data->frameStyle & shapeMask )
    {
        case QFrame::Panel:
        case QFrame::StyledPanel:
        case QFrame::WinPanel:
        {
            qDrawShadePanel(painter, r, 
#if QT_VERSION < 0x040000
                d_data->palette.active(),
#else
                d_data->palette, 
#endif
                shadow == QFrame::Sunken, d_data->lineWidth, &brush
            );
            break;
        }
        case QFrame::Box:
        default:
        {
            if ( shadow == QFrame::Plain )
            {
                qDrawPlainRect(painter, r, 
#if QT_VERSION < 0x040000
                    d_data->palette.color(
                        QPalette::Active, QColorGroup::Foreground),
#else
                    d_data->palette.color(QPalette::Foreground),
#endif
                    d_data->lineWidth, &brush);
            }
            else
            {
                const int midLineWidth = 0;

                qDrawShadeRect( painter, r, 
#if QT_VERSION < 0x040000
                    d_data->palette.active(),
#else
                    d_data->palette, 
#endif
                    shadow == QFrame::Sunken, 
                    d_data->lineWidth, midLineWidth, &brush 
                );
            }
        }
    }
}

/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include <qevent.h>
#include <qapplication.h>
#include "qwt_global.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_plot_directpainter.h"

static void renderItem(QPainter *painter,
    QwtPlotAbstractSeriesItem *seriesItem, int from, int to)
{
    QwtPlot *plot = seriesItem->plot();

    const QwtScaleMap xMap = plot->canvasMap(seriesItem->xAxis());
    const QwtScaleMap yMap = plot->canvasMap(seriesItem->yAxis());

#if QT_VERSION >= 0x040000
    painter->setRenderHint(QPainter::Antialiasing,
        seriesItem->testRenderHint(QwtPlotItem::RenderAntialiased) );
#endif
    seriesItem->drawSeries(painter, xMap, yMap,
        plot->canvas()->contentsRect(), from, to);
}

class QwtPlotDirectPainter::PrivateData
{
public:
    PrivateData():
        attributes(0),
        seriesItem(NULL)
    {
    }

    int attributes;

    QPainter painter;

    QwtPlotAbstractSeriesItem *seriesItem;
    int from;
    int to;
};

QwtPlotDirectPainter::QwtPlotDirectPainter(QObject *parent):
    QObject(parent)
{
    d_data = new PrivateData;
}

QwtPlotDirectPainter::~QwtPlotDirectPainter()
{
    delete d_data;
}

void QwtPlotDirectPainter::setAttribute(Attribute attribute, bool on)
{
    if ( bool(d_data->attributes & attribute) != on )
    {
        if ( on )
            d_data->attributes |= attribute;
        else
            d_data->attributes &= ~attribute;

        if ( attribute == AtomicPainter && on )
            reset();
    }
}

bool QwtPlotDirectPainter::testAttribute(Attribute attribute) const
{
    return d_data->attributes & attribute;
}

/*!
  \brief Draw a set of points of a seriesItem.

  When observing an measurement while it is running, new points have to be
  added to an existing seriesItem. drawSeries can be used to display them avoiding
  a complete redraw of the canvas.

  Setting plot()->canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
  will result in faster painting, if the paint engine of the canvas widget
  supports this feature. 

  \param from Index of the first point to be painted
  \param to Index of the last point to be painted. If to < 0 the
         series will be painted to its last point.
*/
void QwtPlotDirectPainter::drawSeries(
    QwtPlotAbstractSeriesItem *seriesItem, int from, int to) 
{
    if ( seriesItem == NULL || seriesItem->plot() == NULL )
        return;

    QwtPlotCanvas *canvas = seriesItem->plot()->canvas();

    const QwtScaleMap xMap = seriesItem->plot()->canvasMap(seriesItem->xAxis());
    const QwtScaleMap yMap = seriesItem->plot()->canvasMap(seriesItem->yAxis());

    if ( canvas->testPaintAttribute(QwtPlotCanvas::PaintCached) &&
        canvas->paintCache() && !canvas->paintCache()->isNull() )
    {
        QPainter painter((QPixmap *)canvas->paintCache());
        painter.translate(-canvas->contentsRect().x(),
            -canvas->contentsRect().y());

        renderItem(&painter, seriesItem, from, to);

        if ( d_data->attributes & FullRepaint )
        {
            canvas->repaint();
            return;
        }
    }

    bool immediatePaint = true;
#if QT_VERSION >= 0x040000
    if ( !canvas->testAttribute(Qt::WA_WState_InPaintEvent) &&
        !canvas->testAttribute(Qt::WA_PaintOutsidePaintEvent) )
    {
        immediatePaint = false;
    }
#endif

    if ( immediatePaint )
    {
        QwtPlotCanvas *canvas = seriesItem->plot()->canvas();
        if ( !(d_data->painter.isActive() && 
            d_data->painter.device() == canvas) )
        {
            reset();

            d_data->painter.begin(canvas);
            d_data->painter.setClipping(true);
            d_data->painter.setClipRect(canvas->contentsRect());

            canvas->installEventFilter(this);
        }

        renderItem(&d_data->painter, seriesItem, from, to);

        if ( d_data->attributes & AtomicPainter )
            reset();
    }
    else
    {
        reset();

        d_data->seriesItem = seriesItem;
        d_data->from = from;
        d_data->to = to;

        canvas->installEventFilter(this);
        canvas->repaint();
        canvas->removeEventFilter(this);

        d_data->seriesItem = NULL;
    }
}

void QwtPlotDirectPainter::reset()
{
    if ( d_data->painter.isActive() )
    {
        QWidget *w = (QWidget *)d_data->painter.device();
        if ( w )
            w->removeEventFilter(this);

        d_data->painter.end();
    }
}

bool QwtPlotDirectPainter::eventFilter(QObject *, QEvent *event)
{
    if ( event->type() == QEvent::Paint )
    {
        reset();

        if ( d_data->seriesItem )
        {
            QwtPlotCanvas *canvas = d_data->seriesItem->plot()->canvas();

            QPainter painter(canvas);
            painter.setClipping(true);
            painter.setClipRect(canvas->contentsRect());

            renderItem(&painter, d_data->seriesItem,
                d_data->from, d_data->to);

            return true;
        }
    }

    return false;
}

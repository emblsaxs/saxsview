/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_barchart.h"
#include "qwt_scale_map.h"
#include "qwt_column_symbol.h"
#include "qwt_painter.h"
#include <qpainter.h>
#include <qpalette.h>

class QwtPlotBarChart::PrivateData
{
public:
    PrivateData():
        symbol( NULL )
    {
    }

    QwtColumnSymbol *symbol;
};

QwtPlotBarChart::QwtPlotBarChart( const QwtText &title ):
    QwtPlotBarItem( title )
{
    init();
}

QwtPlotBarChart::QwtPlotBarChart( const QString &title ):
    QwtPlotBarItem( QwtText( title ) )
{
    init();
}

QwtPlotBarChart::~QwtPlotBarChart()
{
    delete d_data;
}

void QwtPlotBarChart::init()
{
    d_data = new PrivateData;
    setData( new QwtPointSeriesData() );
}

//! \return QwtPlotItem::Rtti_PlotBarChart
int QwtPlotBarChart::rtti() const
{
    return QwtPlotItem::Rtti_PlotBarChart;
}

void QwtPlotBarChart::setSamples(
    const QVector<QPointF> &samples )
{
    setData( new QwtPointSeriesData( samples ) );
}

void QwtPlotBarChart::setSamples(
    const QVector<double> &samples )
{
    QVector<QPointF> points;
    for ( int i = 0; i < samples.size(); i++ )
        points += QPointF( i, samples[ i ] );

    setData( new QwtPointSeriesData( points ) );
}

void QwtPlotBarChart::setSymbol( QwtColumnSymbol *symbol )
{
    if ( symbol != d_data->symbol )
    {
        delete d_data->symbol;
        d_data->symbol = symbol;

        legendChanged();
        itemChanged();
    }
}

/*!
  \return Current symbol or NULL, when no symbol has been assigned
  \sa setSymbol()
*/
const QwtColumnSymbol *QwtPlotBarChart::symbol() const
{
    return d_data->symbol;
}

QRectF QwtPlotBarChart::boundingRect() const
{
    const size_t numSamples = dataSize();
    if ( numSamples == 0 )
        return QwtPlotSeriesItem::boundingRect();

    const double baseLine = baseline();

    QRectF rect = QwtPlotSeriesItem::boundingRect();
    if ( rect.bottom() < baseLine )
        rect.setBottom( baseLine );
    if ( rect.top() > baseLine )
        rect.setTop( baseLine );

    if ( rect.isValid() && ( orientation() == Qt::Horizontal ) )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

/*!
  Draw an interval of the bar chart

  \param painter Painter
  \param xMap Maps x-values into pixel coordinates.
  \param yMap Maps y-values into pixel coordinates.
  \param canvasRect Contents rect of the canvas
  \param from Index of the first point to be painted
  \param to Index of the last point to be painted. If to < 0 the
         curve will be painted to its last point.

  \sa drawSymbols()
*/
void QwtPlotBarChart::drawSeries( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to ) const
{
    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;


    const QRectF br = data()->boundingRect();
    const QwtInterval interval( br.left(), br.right() );

    painter->save();

    for ( int i = from; i <= to; i++ )
    {
        drawSample( painter, xMap, yMap,
                    canvasRect, interval, i, sample( i ) );
    }

    painter->restore();
}

/*!
  Draw a sample

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param canvasRect Contents rect of the canvas
  \param boundingInterval Bounding interval of sample values
  \param from Index of the first point to be painted
  \param to Index of the last point to be painted

  \sa drawSeries()
*/
void QwtPlotBarChart::drawSample( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, const QwtInterval &boundingInterval,
    int index, const QPointF &point ) const
{
    double sampleW;

    if ( orientation() == Qt::Horizontal )
    {
        sampleW = sampleWidth( yMap, canvasRect.height(),
            boundingInterval.width(), point.y() );
    }
    else
    {
        sampleW = sampleWidth( xMap, canvasRect.width(),
            boundingInterval.width(), point.y() );
    }

    // ....
    Q_UNUSED( painter );
    Q_UNUSED( sampleW );
    Q_UNUSED( index );
}

void QwtPlotBarChart::drawBar( QPainter *painter,
    int sampleIndex, const QwtColumnRect &rect ) const
{
    static Qt::GlobalColor colors[] =
        { Qt::blue, Qt::red, Qt::green, Qt::magenta, Qt::yellow };

    const int colorIndex = sampleIndex % ( sizeof( colors ) / sizeof( colors[0] ) );

    if ( d_data->symbol &&
        ( d_data->symbol->style() != QwtColumnSymbol::NoStyle ) )
    {
        d_data->symbol->setPalette( QPalette( colors[ colorIndex ] ) );
        d_data->symbol->draw( painter, rect );
    }
    else
    {
        QRectF r = rect.toRect();
        if ( QwtPainter::roundingAlignment( painter ) )
        {
            r.setLeft( qRound( r.left() ) );
            r.setRight( qRound( r.right() ) );
            r.setTop( qRound( r.top() ) );
            r.setBottom( qRound( r.bottom() ) );
        }

        painter->setPen( QPen( Qt::black, 1 ) );
        painter->setBrush( colors[ colorIndex ] );
        QwtPainter::drawRect( painter, r );
    }
}

void QwtPlotBarChart::drawLabel( QPainter *painter, int sampleIndex,
    const QwtColumnRect &rect, const QwtText &text ) const
{
    Q_UNUSED( painter );
    Q_UNUSED( sampleIndex );
    Q_UNUSED( rect );
    Q_UNUSED( text );
}

QwtText QwtPlotBarChart::label(
    int sampleIndex, const QPointF &sample ) const
{
    Q_UNUSED( sampleIndex );

    QString labelText;
    labelText.setNum( sample.y() );

    return QwtText( labelText );
}

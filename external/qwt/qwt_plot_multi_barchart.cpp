/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_multi_barchart.h"
#include "qwt_scale_map.h"
#include "qwt_column_symbol.h"
#include "qwt_painter.h"
#include <qpainter.h>
#include <qpalette.h>
#include <qmap.h>

inline static bool qwtIsIncreasing(
    const QwtScaleMap &map, const QVector<double> &values )
{
    bool isInverting = map.isInverting();

    for ( int i = 0; i < values.size(); i++ )
    {
        const double y = values[ i ];
        if ( y != 0.0 )
            return ( map.isInverting() != ( y > 0.0 ) );
    }

    return !isInverting;
}

class QwtPlotMultiBarChart::PrivateData
{
public:
    PrivateData():
        style( QwtPlotMultiBarChart::Grouped )
    {
        colorTable << Qt::red << Qt::blue << Qt::darkGreen << Qt::yellow
            << Qt::darkCyan << Qt::darkMagenta << Qt::darkYellow
            << Qt::darkBlue << Qt::green << Qt::magenta;
    }

    QwtPlotMultiBarChart::ChartStyle style;
    QList<QBrush> colorTable;
    QList<QwtText> barTitles;
    QMap<int, QwtColumnSymbol *> symbolMap;
};

QwtPlotMultiBarChart::QwtPlotMultiBarChart( const QwtText &title ):
    QwtPlotBarItem( title )
{
    init();
}

QwtPlotMultiBarChart::QwtPlotMultiBarChart( const QString &title ):
    QwtPlotBarItem( QwtText( title ) )
{
    init();
}

QwtPlotMultiBarChart::~QwtPlotMultiBarChart()
{
    clearSymbols();
    delete d_data;
}

void QwtPlotMultiBarChart::init()
{
    d_data = new PrivateData;
    setData( new QwtSetSeriesData() );
}

//! \return QwtPlotItem::Rtti_PlotBarChart
int QwtPlotMultiBarChart::rtti() const
{
    return QwtPlotItem::Rtti_PlotMultiBarChart;
}

void QwtPlotMultiBarChart::setSamples(
    const QVector<QwtSetSample> &samples )
{
    setData( new QwtSetSeriesData( samples ) );
}

void QwtPlotMultiBarChart::setSamples(
    const QVector< QVector<double> > &samples )
{
    QVector<QwtSetSample> s;
    for ( int i = 0; i < samples.size(); i++ )
        s += QwtSetSample( i, samples[ i ] );

    setData( new QwtSetSeriesData( s ) );
}

void QwtPlotMultiBarChart::setTitles( const QList<QwtText> &titles )
{
    d_data->barTitles = titles;
    itemChanged();
}

QList<QwtText> QwtPlotMultiBarChart::titles() const
{
    return d_data->barTitles;
}

void QwtPlotMultiBarChart::setColorTable( const QList<QBrush> &colorTable )
{
    d_data->colorTable = colorTable;

    legendChanged();
    itemChanged();
}

QList<QBrush> QwtPlotMultiBarChart::colorTable() const
{
    return d_data->colorTable;
}

void QwtPlotMultiBarChart::setSymbol( int barIndex, QwtColumnSymbol *symbol )
{
    if ( barIndex < 0 )
        return;

    QMap<int, QwtColumnSymbol *>::iterator it = 
        d_data->symbolMap.find(barIndex);
    if ( it == d_data->symbolMap.end() )
    {
        if ( symbol != NULL )
        {
            d_data->symbolMap.insert( barIndex, symbol );

            legendChanged();
            itemChanged();
        }
    }
    else
    {
        if ( symbol != it.value() )
        {
            delete it.value();

            if ( symbol == NULL )
            {
                d_data->symbolMap.remove( barIndex );
            }
            else
            {
                it.value() = symbol;
            }

            legendChanged();
            itemChanged();
        }
    }
}

const QwtColumnSymbol *QwtPlotMultiBarChart::symbol( int barIndex ) const
{
    QMap<int, QwtColumnSymbol *>::const_iterator it =
        d_data->symbolMap.find(barIndex);

    return ( it == d_data->symbolMap.end() ) ? NULL : it.value();
}

QwtColumnSymbol *QwtPlotMultiBarChart::symbol( int barIndex ) 
{
    QMap<int, QwtColumnSymbol *>::iterator it =
        d_data->symbolMap.find(barIndex);

    return ( it == d_data->symbolMap.end() ) ? NULL : it.value();
}

void QwtPlotMultiBarChart::clearSymbols()
{
    for ( QMap<int, QwtColumnSymbol *>::iterator it 
        = d_data->symbolMap.begin(); it != d_data->symbolMap.end(); ++it )
    {
        delete it.value();
    }

    d_data->symbolMap.clear();
}

void QwtPlotMultiBarChart::setStyle( ChartStyle style )
{
    if ( style != d_data->style )
    {
        d_data->style = style;

        legendChanged();
        itemChanged();
    }
}

QwtPlotMultiBarChart::ChartStyle QwtPlotMultiBarChart::style() const
{
    return d_data->style;
}

QRectF QwtPlotMultiBarChart::boundingRect() const
{
    const size_t numSamples = dataSize();

    if ( numSamples == 0 )
        return QwtPlotSeriesItem::boundingRect();

    const double baseLine = baseline();

    QRectF rect;

    if ( d_data->style != QwtPlotMultiBarChart::Stacked )
    {
        rect = QwtPlotSeriesItem::boundingRect();
        if ( rect.bottom() < baseLine )
            rect.setBottom( baseLine );
        if ( rect.top() > baseLine )
            rect.setTop( baseLine );
    }
    else
    {
        double xMin, xMax, yMin, yMax;

        xMin = xMax = 0.0;
        yMin = yMax = baseLine;

        const QwtSeriesData<QwtSetSample> *series = data();

        for ( size_t i = 0; i < numSamples; i++ )
        {
            const QwtSetSample sample = series->sample( i );
            if ( i == 0 )
            {
                xMin = xMax = sample.value;
            }
            else
            {
                xMin = qMin( xMin, sample.value );
                xMax = qMax( xMax, sample.value );
            }

            const double y = baseLine + sample.added();

            yMin = qMin( yMin, y );
            yMax = qMax( yMax, y );
        }
        rect.setRect( xMin, yMin, xMax - xMin, yMax - yMin );
    }

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
void QwtPlotMultiBarChart::drawSeries( QPainter *painter,
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
void QwtPlotMultiBarChart::drawSample( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, const QwtInterval &boundingInterval,
    int index, const QwtSetSample& sample ) const
{
    if ( sample.set.size() <= 0 )
        return;

    double sampleW;

    if ( orientation() == Qt::Horizontal )
    {
        sampleW = sampleWidth( yMap, canvasRect.height(),
            boundingInterval.width(), sample.value );
    }
    else
    {
        sampleW = sampleWidth( xMap, canvasRect.width(),
            boundingInterval.width(), sample.value );
    }

    if ( d_data->style == Stacked )
    {
        drawStackedBars( painter, xMap, yMap,
            canvasRect, index, sampleW, sample );
    }
    else
    {
        drawGroupedBars( painter, xMap, yMap,
            canvasRect, index, sampleW, sample );
    }
}

void QwtPlotMultiBarChart::drawGroupedBars( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int index, double sampleWidth,
    const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect );

    const int numBars = sample.set.size();
    if ( numBars == 0 )
        return;

    if ( orientation() == Qt::Vertical )
    {
        const double barWidth = sampleWidth / numBars;

        const double y1 = yMap.transform( baseline() );
        const double x0 = xMap.transform( sample.value ) - 0.5 * sampleWidth;

        for ( int i = 0; i < numBars; i++ )
        {
            const double x1 = x0 + i * barWidth;
            const double x2 = x1 + barWidth;

            const double y2 = yMap.transform( sample.set[i] );

            QwtColumnRect bar;
            bar.direction = ( y1 < y2 ) ?
                QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

            bar.hInterval = QwtInterval( x1, x2 ).normalized();
            if ( i != 0 )
                bar.hInterval.setBorderFlags( QwtInterval::ExcludeMinimum );

            bar.vInterval = QwtInterval( y1, y2 ).normalized();

            drawBar( painter, index, i, bar );

            if ( testChartAttribute( QwtPlotBarItem::ShowLabels ) )
            {
                const QwtText text = label( index, i, sample );
                drawLabel( painter, index, i, bar, text );
            }
        }
    }
    else
    {
        const double barHeight = sampleWidth / numBars;

        const double x1 = xMap.transform( baseline() );
        const double y0 = yMap.transform( sample.value ) - 0.5 * sampleWidth;

        for ( int i = 0; i < numBars; i++ )
        {
            double y1 = y0 + i * barHeight;
            double y2 = y1 + barHeight;

            double x2 = xMap.transform( sample.set[i] );

            QwtColumnRect bar;
            bar.direction = x1 < x2 ?
                QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;

            bar.hInterval = QwtInterval( x1, x2 ).normalized();

            bar.vInterval = QwtInterval( y1, y2 );
            if ( i != 0 )
                bar.vInterval.setBorderFlags( QwtInterval::ExcludeMinimum );

            drawBar( painter, index, i, bar );

            if ( testChartAttribute( QwtPlotBarItem::ShowLabels ) )
            {
                const QwtText text = label( index, i, sample );
                drawLabel( painter, index, i, bar, text );
            }
        }
    }
}

void QwtPlotMultiBarChart::drawStackedBars( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int index, 
    double sampleWidth, const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect ); // clipping the bars ?

    const int numBars = sample.set.size();
    if ( numBars == 0 )
        return;

    QwtInterval::BorderFlag borderFlags = QwtInterval::IncludeBorders;

    if ( orientation() == Qt::Vertical )
    {
        const double x1 = xMap.transform( sample.value ) - 0.5 * sampleWidth;
        const double x2 = x1 + sampleWidth;

        const bool increasing = qwtIsIncreasing( yMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
            QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

        bar.hInterval = QwtInterval( x1, x2 ).normalized();

        double sum = baseline();

        const int numBars = sample.set.size();
        for ( int i = 0; i < numBars; i++ )
        {
            const double si = sample.set[ i ];
            if ( si == 0.0 )
                continue;

            const double y1 = yMap.transform( sum );
            const double y2 = yMap.transform( sum + si );

            if ( ( y2 > y1 ) != increasing )
            {
                // stacked bars need to be in the same direction
                continue;
            }

            bar.vInterval = QwtInterval( y1, y2 ).normalized();
            bar.vInterval.setBorderFlags( borderFlags );

            drawBar( painter, index, i, bar );

            sum += si;

            if ( increasing )
                borderFlags = QwtInterval::ExcludeMinimum;
            else
                borderFlags = QwtInterval::ExcludeMaximum;
        }
    }
    else
    {
        const double y1 = yMap.transform( sample.value ) - 0.5 * sampleWidth;
        const double y2 = y1 + sampleWidth;

        const bool increasing = qwtIsIncreasing( xMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
            QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;
        bar.vInterval = QwtInterval( y1, y2 ).normalized();

        double sum = baseline();

        for ( int i = 0; i < sample.set.size(); i++ )
        {
            const double si = sample.set[ i ];
            if ( si == 0.0 )
                continue;

            const double x1 = xMap.transform( sum );
            const double x2 = xMap.transform( sum + si );

            if ( ( x2 > x1 ) != increasing )
            {
                // stacked bars need to be in the same direction
                continue;
            }

            bar.hInterval = QwtInterval( x1, x2 ).normalized();
            bar.hInterval.setBorderFlags( borderFlags );

            drawBar( painter, index, i, bar );

            sum += si;

            if ( increasing )
                borderFlags = QwtInterval::ExcludeMinimum;
            else
                borderFlags = QwtInterval::ExcludeMaximum;
        }
    }
}

void QwtPlotMultiBarChart::drawBar( QPainter *painter,
    int sampleIndex, int barIndex, const QwtColumnRect &rect ) const
{
    Q_UNUSED( sampleIndex );

    const QwtColumnSymbol *sym = symbol( barIndex );
    if ( sym )
    {
        sym->draw( painter, rect );
    }
    else
    {
        QBrush brush( Qt::white );

        if ( d_data->colorTable.size() > 0 )
        {
            const int colorIndex = barIndex % d_data->colorTable.size();
            brush = d_data->colorTable[ colorIndex ];
        }

        QPalette palette;
        palette.setBrush( QPalette::Window, brush );
        palette.setColor( QPalette::Dark, Qt::black );

        QwtColumnSymbol sym( QwtColumnSymbol::Box );
        sym.setPalette( palette );
        sym.setLineWidth( 0 );
        sym.setFrameStyle( QwtColumnSymbol::Plain );
        sym.draw( painter, rect );
    }
}

void QwtPlotMultiBarChart::drawLabel( QPainter *painter, int sampleIndex,
    int barIndex, const QwtColumnRect &rect, const QwtText &text ) const
{
    Q_UNUSED( painter );
    Q_UNUSED( sampleIndex );
    Q_UNUSED( barIndex );
    Q_UNUSED( rect );
    Q_UNUSED( text );
}


QwtText QwtPlotMultiBarChart::label(
    int sampleIndex, int barIndex, const QwtSetSample& sample ) const
{
    Q_UNUSED( sampleIndex );

    QString labelText;
    if ( barIndex >= 0 && barIndex <= sample.set.size() )
        labelText.setNum( sample.set[ barIndex ] );

    return QwtText( labelText );
}

QList<QwtLegendData> QwtPlotMultiBarChart::legendData() const
{
    QList<QwtLegendData> list;

    for ( int i = 0; i < d_data->barTitles.size(); i++ )
    {
        QwtLegendData data;

        QVariant titleValue;
        qVariantSetValue( titleValue, d_data->barTitles[i] );
        data.setValue( QwtLegendData::TitleRole, titleValue );

        if ( !legendIconSize().isEmpty() )
        {
            QVariant iconValue;
            qVariantSetValue( iconValue, 
                legendIcon( i, legendIconSize() ) );

            data.setValue( QwtLegendData::IconRole, iconValue );
        }

        list += data;
    }

    return list;
}

QwtGraphic QwtPlotMultiBarChart::legendIcon( int index,
    const QSizeF &size ) const
{
    QwtColumnRect column;
    column.hInterval = QwtInterval( 0.0, size.width() - 1.0 );
    column.vInterval = QwtInterval( 0.0, size.height() - 1.0 );

    QwtGraphic icon;
    icon.setDefaultSize( size );
    icon.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    drawBar( &painter, -1, index, column );

    return icon;
}


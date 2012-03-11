/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_legenditem.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include <qlayoutitem.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qmath.h>

class QwtLegendLayoutItem: public QLayoutItem
{
public:
    QwtLegendLayoutItem( const QwtPlotLegendItem *, const QwtPlotItem * );
    virtual ~QwtLegendLayoutItem();

    const QwtPlotItem *plotItem() const;

    void setData( const QwtLegendData & );
    const QwtLegendData &data() const;

    virtual Qt::Orientations expandingDirections() const;
    virtual QRect geometry() const;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth( int w ) const;
    virtual bool isEmpty() const;
    virtual QSize maximumSize() const;
    virtual int minimumHeightForWidth( int w ) const;
    virtual QSize minimumSize() const;
    virtual void setGeometry( const QRect & r );
    virtual QSize sizeHint() const;

private:

    const QwtPlotLegendItem *d_legendItem;
    const QwtPlotItem *d_plotItem;
    QwtLegendData d_data;

    QRect d_rect;
};

QwtLegendLayoutItem::QwtLegendLayoutItem( 
        const QwtPlotLegendItem *legendItem, const QwtPlotItem *plotItem ):
    d_legendItem( legendItem ),
    d_plotItem( plotItem)
{
}

QwtLegendLayoutItem::~QwtLegendLayoutItem()
{
}

const QwtPlotItem *QwtLegendLayoutItem::plotItem() const
{
    return d_plotItem;
}

void QwtLegendLayoutItem::setData( const QwtLegendData &data )
{
    d_data = data;
}

const QwtLegendData &QwtLegendLayoutItem::data() const
{
    return d_data;
}

Qt::Orientations QwtLegendLayoutItem::expandingDirections() const
{
    return Qt::Horizontal;
}

bool QwtLegendLayoutItem::hasHeightForWidth() const
{
    return !d_data.title().isEmpty();
}

int QwtLegendLayoutItem::minimumHeightForWidth( int w ) const
{
    return d_legendItem->heightForWidth( d_data, w );
}

int QwtLegendLayoutItem::heightForWidth( int w ) const
{
    return d_legendItem->heightForWidth( d_data, w );
}

bool QwtLegendLayoutItem::isEmpty() const
{
    return false;
}

QSize QwtLegendLayoutItem::maximumSize() const
{
    return QSize( QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX );
}

QSize QwtLegendLayoutItem::minimumSize() const
{
    return d_legendItem->minimumSize( d_data );
}

QSize QwtLegendLayoutItem::sizeHint() const
{
    return minimumSize();
}

void QwtLegendLayoutItem::setGeometry( const QRect &rect )
{
    d_rect = rect;
}

QRect QwtLegendLayoutItem::geometry() const
{
    return d_rect;
}

class QwtPlotLegendItem::PrivateData
{
public:
    PrivateData():
        itemMargin( 4 ),
        itemSpacing( 4 ),
        borderRadius( 0.0 ),
        borderPen( Qt::NoPen ),
        backgroundBrush( Qt::NoBrush ),
        backgroundMode( QwtPlotLegendItem::LegendBackground ),
        borderDistance( 10 ),
        alignment( Qt::AlignRight | Qt::AlignBottom )
    {
        layout = new QwtDynGridLayout();
        layout->setMaxCols( 2 );

        layout->setSpacing( 0 );
        layout->setContentsMargins( 0, 0, 0, 0 );
    }

    ~PrivateData()
    {
        delete layout;
    }

    QFont font;
    QPen textPen;
    int itemMargin;
    int itemSpacing;

    double borderRadius;
    QPen borderPen;
    QBrush backgroundBrush;
    QwtPlotLegendItem::BackgroundMode backgroundMode;

    int borderDistance;
    Qt::Alignment alignment;

    QMap< const QwtPlotItem *, QList<QwtLegendLayoutItem *> > map;
    QwtDynGridLayout *layout;
};

QwtPlotLegendItem::QwtPlotLegendItem():
    QwtPlotItem( QwtText( "Legend" ) )
{
    d_data = new PrivateData;

    setItemInterest( QwtPlotItem::LegendInterest, true );
    setZ( 100.0 );
}

//! Destructor
QwtPlotLegendItem::~QwtPlotLegendItem()
{
    clearLegend();
    delete d_data;
}

//! \return QwtPlotItem::Rtti_PlotLegend
int QwtPlotLegendItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotLegend;
}

void QwtPlotLegendItem::setAlignment( Qt::Alignment alignment )
{
    if ( d_data->alignment != alignment )
    {
        d_data->alignment = alignment;
        itemChanged();
    }
}

Qt::Alignment QwtPlotLegendItem::alignment() const
{
    return d_data->alignment;
}

void QwtPlotLegendItem::setMaxColumns( uint maxColumns )
{
    if ( maxColumns != d_data->layout->maxCols() )
    {
        d_data->layout->setMaxCols( maxColumns );
        itemChanged();
    }
}

uint QwtPlotLegendItem::maxColumns() const
{
    return d_data->layout->maxCols();
}

void QwtPlotLegendItem::setMargin( int margin )
{
    margin = qMax( margin, 0 );
    if ( margin != this->margin() )
    {
        d_data->layout->setContentsMargins( 
            margin, margin, margin, margin );
        itemChanged();
    }
}

int QwtPlotLegendItem::margin() const
{
    int left;
    d_data->layout->getContentsMargins( &left, NULL, NULL, NULL );

    return left;
}

void QwtPlotLegendItem::setSpacing( int spacing )
{
    spacing = qMax( spacing, 0 );
    if ( spacing != d_data->layout->spacing() )
    {
        d_data->layout->setSpacing( spacing );
        itemChanged();
    }
}

int QwtPlotLegendItem::spacing() const
{
    return d_data->layout->spacing();
}

void QwtPlotLegendItem::setItemMargin( int margin )
{
    margin = qMax( margin, 0 );
    if ( margin != d_data->itemMargin )
    {
        d_data->itemMargin = margin;
        d_data->layout->invalidate();
    }
}

int QwtPlotLegendItem::itemMargin() const
{
    return d_data->itemMargin;
}

void QwtPlotLegendItem::setItemSpacing( int spacing )
{
    spacing = qMax( spacing, 0 );
    if ( spacing != d_data->itemSpacing )
    {
        d_data->itemSpacing = spacing;
        d_data->layout->invalidate();
    }

}

int QwtPlotLegendItem::itemSpacing() const
{
    return d_data->itemSpacing;
}

/*!
   Change the tick label font
   \sa font()
*/
void QwtPlotLegendItem::setFont( const QFont &font )
{
    if ( font != d_data->font )
    {
        d_data->font = font;
        itemChanged();
    }
}

/*!
   \return tick label font
   \sa setFont()
*/
QFont QwtPlotLegendItem::font() const
{
    return d_data->font;
}

void QwtPlotLegendItem::setBorderDistance( int distance )
{
    if ( distance < 0 )
        distance = -1;

    if ( distance != d_data->borderDistance )
    {
        d_data->borderDistance = distance;
        itemChanged();
    }
}

int QwtPlotLegendItem::borderDistance() const
{
    return d_data->borderDistance;
}

void QwtPlotLegendItem::setBorderRadius( double radius )
{
    radius = qMax( 0.0, radius );

    if ( radius != d_data->borderRadius )
    {
        d_data->borderRadius = radius;
        itemChanged();
    }
}

double QwtPlotLegendItem::borderRadius() const
{
    return d_data->borderRadius;
}

void QwtPlotLegendItem::setBorderPen( const QPen &pen )
{
    if ( d_data->borderPen != pen )
    {
        d_data->borderPen = pen;
        itemChanged();
    }
}

QPen QwtPlotLegendItem::borderPen() const
{
    return d_data->borderPen;
}


void QwtPlotLegendItem::setBackgroundBrush( const QBrush &brush )
{
    if ( d_data->backgroundBrush != brush )
    {
        d_data->backgroundBrush = brush;
        itemChanged();
    }
}

QBrush QwtPlotLegendItem::backgroundBrush() const
{
    return d_data->backgroundBrush;
}

void QwtPlotLegendItem::setBackgroundMode( BackgroundMode mode )
{
    if ( mode != d_data->backgroundMode )
    {
        d_data->backgroundMode = mode;
        itemChanged();
    }
}

QwtPlotLegendItem::BackgroundMode QwtPlotLegendItem::backgroundMode() const
{
    return d_data->backgroundMode;
}

void QwtPlotLegendItem::setTextPen( const QPen &pen )
{
    if ( d_data->textPen != pen )
    {
        d_data->textPen = pen;
        itemChanged();
    }
}

QPen QwtPlotLegendItem::textPen() const
{
    return d_data->textPen;
}

void QwtPlotLegendItem::draw( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect ) const
{
    Q_UNUSED( xMap );
    Q_UNUSED( yMap );

    d_data->layout->setGeometry( geometry( canvasRect ) );

    if ( d_data->backgroundMode == QwtPlotLegendItem::LegendBackground )
        drawBackground( painter, d_data->layout->geometry() );
    
    for ( int i = 0; i <  d_data->layout->count(); i++ )
    {
        const QwtLegendLayoutItem *layoutItem = 
            static_cast<QwtLegendLayoutItem *>( d_data->layout->itemAt( i ) );

        if ( d_data->backgroundMode == QwtPlotLegendItem::ItemBackground )
            drawBackground( painter, layoutItem->geometry() );

        painter->save();

        drawLegendData( painter, layoutItem->plotItem(),
            layoutItem->data(), layoutItem->geometry() );

        painter->restore();
    }
}

void QwtPlotLegendItem::drawBackground( 
    QPainter *painter, const QRectF &rect ) const
{
    painter->save();

    painter->setPen( d_data->borderPen );
    painter->setBrush( d_data->backgroundBrush );
    
    const double radius = d_data->borderRadius;
    painter->drawRoundedRect( rect, radius, radius );
    
    painter->restore();
}

QRect QwtPlotLegendItem::geometry( const QRectF &canvasRect ) const
{
    QRect rect;
    rect.setSize( d_data->layout->sizeHint() );

    int margin = d_data->borderDistance;
    if ( d_data->alignment & Qt::AlignHCenter )
    {
        int x = qRound( canvasRect.center().x() );
        rect.moveCenter( QPoint( x, rect.center().y() ) ); 
    }
    else if ( d_data->alignment & Qt::AlignRight )
    {
        rect.moveRight( qFloor( canvasRect.right() - margin ) );
    }
    else 
    {
        rect.moveLeft( qCeil( canvasRect.left() + margin ) );
    }

    if ( d_data->alignment & Qt::AlignVCenter )
    {
        int y = qRound( canvasRect.center().y() );
        rect.moveCenter( QPoint( rect.center().x(), y ) );
    }
    else if ( d_data->alignment & Qt::AlignBottom )
    {
        rect.moveBottom( qFloor( canvasRect.bottom() - margin ) );
    }
    else 
    {
        rect.moveTop( qCeil( canvasRect.top() + margin ) ); 
    }

    return rect;
}

void QwtPlotLegendItem::updateLegend( const QwtPlotItem *plotItem,
        const QList<QwtLegendData> &data )
{
    if ( plotItem == NULL )
        return;

    QList<QwtLegendLayoutItem *> layoutItems;

    QMap<const QwtPlotItem *, QList<QwtLegendLayoutItem *> >::iterator it = 
        d_data->map.find( plotItem );
    if ( it != d_data->map.end() )
        layoutItems = it.value();

    bool changed = false;

    if ( data.size() != layoutItems.size() )
    {
        changed = true;

        for ( int i = 0; i < layoutItems.size(); i++ )
        {
            d_data->layout->removeItem( layoutItems[i] );
            delete layoutItems[i];
        }
        if ( it != d_data->map.end() )
            d_data->map.remove( plotItem );

        if ( !data.isEmpty() )
        {
            for ( int i = 0; i < data.size(); i++ )
            {
                QwtLegendLayoutItem *layoutItem = 
                    new QwtLegendLayoutItem( this, plotItem );
                d_data->layout->addItem( layoutItem );
                layoutItems += layoutItem;
            }

            d_data->map.insert( plotItem, layoutItems );
        }
    }

    for ( int i = 0; i < data.size(); i++ )
    {
        if ( layoutItems[i]->data().values() != data[i].values() )
        {
            layoutItems[i]->setData( data[i] );
            changed = true;
        }
    }

    if ( changed )
        itemChanged();
}

void QwtPlotLegendItem::clearLegend()
{
    if ( !d_data->map.isEmpty() )
    {
        d_data->map.clear();

        for ( int i = d_data->layout->count() - 1; i >= 0; i-- )
            delete d_data->layout->takeAt( i );

        itemChanged();
    }
}

void QwtPlotLegendItem::drawLegendData( QPainter *painter,
    const QwtPlotItem *plotItem, const QwtLegendData &data, 
    const QRectF &rect ) const
{
    Q_UNUSED( plotItem );

    const int m = d_data->itemMargin;
    const QRectF r = rect.toRect().adjusted( m, m, -m, -m );

    painter->setClipRect( r, Qt::IntersectClip );

    int titleOff = 0;

    const QwtGraphic graphic = data.icon();
    if ( !graphic.isEmpty() )
    {
        QRectF iconRect( r.topLeft(), graphic.defaultSize() );

        iconRect.moveCenter( 
            QPoint( iconRect.center().x(), rect.center().y() ) );

        graphic.render( painter, iconRect, Qt::KeepAspectRatio );

        titleOff += iconRect.width() + d_data->itemSpacing;
    }

    const QwtText text = data.title();
    if ( !text.isEmpty() )
    {
        painter->setPen( textPen() );
        painter->setFont( font() );

        const QRectF textRect = r.adjusted( titleOff, 0, 0, 0 );
        text.draw( painter, textRect );
    }
}

QSize QwtPlotLegendItem::minimumSize( const QwtLegendData &data ) const
{
    QSize size( 2 * d_data->itemMargin, 2 * d_data->itemMargin );

    if ( !data.isValid() )
        return size;

    const QwtGraphic graphic = data.icon();
    const QwtText text = data.title();

    int w = 0;
    int h = 0;

    if ( !graphic.isNull() )
    {
        w = graphic.width();
        h = graphic.height();
    }

    if ( !text.isEmpty() )
    {
        const QSizeF sz = text.textSize( font() );

        w += qCeil( sz.width() );
        h = qMax( h, qCeil( sz.height() ) );
    }

    if ( graphic.width() > 0 && !text.isEmpty() )
        w += d_data->itemSpacing;

    size += QSize( w, h );
    return size;
}

int QwtPlotLegendItem::heightForWidth( 
    const QwtLegendData &data, int w ) const
{
    w -= 2 * d_data->itemMargin;

    const QwtGraphic graphic = data.icon();
    const QwtText text = data.title();

    if ( text.isEmpty() )
        return graphic.height();

    if ( graphic.width() > 0 )
        w -= graphic.width() + d_data->itemSpacing;

    int h = text.heightForWidth( w, font() );
    h += 2 * d_data->itemMargin;

    return qMax( graphic.height(), h );
}

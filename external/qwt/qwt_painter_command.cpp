/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_painter_command.h"

QwtPainterCommand::QwtPainterCommand():
    d_type( Invalid )
{
}

QwtPainterCommand::QwtPainterCommand( const QPainterPath &path ):
    d_type( Path )
{
    d_path = new QPainterPath( path );
}

QwtPainterCommand::QwtPainterCommand( const QRectF &rect,
        const QPixmap &pixmap, const QRectF& subRect ):
    d_type( Pixmap )
{
    d_pixmapData = new PixmapData();
    d_pixmapData->rect = rect;
    d_pixmapData->pixmap = pixmap;
    d_pixmapData->subRect = subRect;
}

QwtPainterCommand::QwtPainterCommand( const QRectF &rect,
        const QImage &image, const QRectF& subRect,
        Qt::ImageConversionFlags flags ):
    d_type( Image )
{
    d_imageData = new ImageData();
    d_imageData->rect = rect;
    d_imageData->image = image;
    d_imageData->subRect = subRect;
    d_imageData->flags = flags;
}

QwtPainterCommand::QwtPainterCommand( const QPaintEngineState &state ):
    d_type( State )
{
    d_stateData = new StateData();

    d_stateData->flags = state.state();

    if ( d_stateData->flags & QPaintEngine::DirtyPen ) 
        d_stateData->pen = state.pen();

    if ( d_stateData->flags & QPaintEngine::DirtyBrush ) 
        d_stateData->brush = state.brush();

    if ( d_stateData->flags & QPaintEngine::DirtyBrushOrigin ) 
        d_stateData->brushOrigin = state.brushOrigin();

    if ( d_stateData->flags & QPaintEngine::DirtyFont ) 
        d_stateData->font = state.font();

    if ( d_stateData->flags & QPaintEngine::DirtyBackground ) 
    {
        d_stateData->backgroundMode = state.backgroundMode();
        d_stateData->backgroundBrush = state.backgroundBrush();
    }

    if ( d_stateData->flags & QPaintEngine::DirtyTransform ) 
        d_stateData->transform = state.transform();

    if ( d_stateData->flags & QPaintEngine::DirtyClipEnabled ) 
        d_stateData->isClipEnabled = state.isClipEnabled();

    if ( d_stateData->flags & QPaintEngine::DirtyClipRegion ) 
    {
        d_stateData->clipRegion = state.clipRegion();
        d_stateData->clipOperation = state.clipOperation();
    }

    if ( d_stateData->flags & QPaintEngine::DirtyClipPath ) 
    {
        d_stateData->clipPath = state.clipPath();
        d_stateData->clipOperation = state.clipOperation();
    }

    if ( d_stateData->flags & QPaintEngine::DirtyHints ) 
        d_stateData->renderHints = state.renderHints();

    if ( d_stateData->flags & QPaintEngine::DirtyCompositionMode ) 
        d_stateData->compositionMode = state.compositionMode();

    if ( d_stateData->flags & QPaintEngine::DirtyOpacity ) 
        d_stateData->opacity = state.opacity();
}

QwtPainterCommand::QwtPainterCommand(const QwtPainterCommand &other)
{
    copy( other );
}

QwtPainterCommand::~QwtPainterCommand()
{
    reset();
}

QwtPainterCommand &QwtPainterCommand::operator=(const QwtPainterCommand &other)
{
    reset();
    copy( other );

    return *this;
}

void QwtPainterCommand::copy( const QwtPainterCommand &other )
{
    d_type = other.d_type;

    switch( other.d_type )
    {
        case Path:
        {
            d_path = new QPainterPath( *other.d_path );
            break;
        }
        case Pixmap:
        {
            d_pixmapData = new PixmapData( *other.d_pixmapData );
            break;
        }
        case Image:
        {
            d_imageData = new ImageData( *other.d_imageData );
            break;
        }
        case State:
        {
            d_stateData = new StateData( *other.d_stateData );
            break;
        }
        default:
            break;
    }
}

void QwtPainterCommand::reset()
{
    switch( d_type )
    {
        case Path:
        {
            delete d_path;
            break;
        }
        case Pixmap:
        {
            delete d_pixmapData;
            break;
        }
        case Image:
        {
            delete d_imageData;
            break;
        }
        case State:
        {
            delete d_stateData;
            break;
        }
        default:
            break;
    }

    d_type = Invalid;
}

QPainterPath *QwtPainterCommand::path() 
{
    return d_path;
}

QwtPainterCommand::PixmapData* QwtPainterCommand::pixmapData() 
{
    return d_pixmapData;
}

QwtPainterCommand::ImageData* QwtPainterCommand::imageData() 
{
    return d_imageData;
}

QwtPainterCommand::StateData* QwtPainterCommand::stateData() 
{
    return d_stateData;
}

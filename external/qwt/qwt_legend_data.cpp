/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_legend_data.h"

QwtLegendData::QwtLegendData()
{
}

QwtLegendData::~QwtLegendData()
{
}

void QwtLegendData::setValues( const QMap<int, QVariant> &map )
{
    d_map = map;
}

const QMap<int, QVariant> &QwtLegendData::values() const
{
    return d_map;
}

bool QwtLegendData::hasRole( int role ) const
{
    return d_map.contains( role );
}

void QwtLegendData::setValue( int role, const QVariant &data )
{
    d_map[role] = data;
}

QVariant QwtLegendData::value( int role ) const
{
    if ( !d_map.contains( role ) )
        return QVariant();

    return d_map[role];
}

bool QwtLegendData::isValid() const
{
    return !d_map.isEmpty();
}

QwtText QwtLegendData::title() const
{
    QwtText text;

    const QVariant titleValue = value( QwtLegendData::TitleRole );
    if ( titleValue.canConvert<QwtText>() )
    {
        text = qvariant_cast<QwtText>( titleValue );
    }
    else if ( titleValue.canConvert<QString>() )
    {
        text.setText( qvariant_cast<QString>( titleValue ) );
    }

    return text;
}

QwtGraphic QwtLegendData::icon() const
{
    const QVariant iconValue = value( QwtLegendData::IconRole );

    QwtGraphic graphic;
    if ( iconValue.canConvert<QwtGraphic>() )
    {
        graphic = qvariant_cast<QwtGraphic>( iconValue );
    }

    return graphic;
}

QwtLegendData::Mode QwtLegendData::mode() const
{
    const QVariant modeValue = value( QwtLegendData::ModeRole );
    if ( modeValue.canConvert<int>() )
    {
        const int mode = qvariant_cast<int>( modeValue );
        return static_cast<QwtLegendData::Mode>( mode );
    }
    
    return QwtLegendData::ReadOnly;
}


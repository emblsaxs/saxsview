/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_LEGEND_DATA_H
#define QWT_LEGEND_DATA_H

#include "qwt_global.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include <qvariant.h>
#include <qpixmap.h>
#include <qmap.h>

/*!
    \brief Attributes of an entry on a legend
 */
class QWT_EXPORT QwtLegendData
{
public:
    enum Mode
    {
        //! The legend item is not interactive, like a label
        ReadOnly,

        //! The legend item is clickable, like a push button
        Clickable,

        //! The legend item is checkable, like a checkable button
        Checkable
    };

    enum Role
    {
        ModeRole, 

        TitleRole, 
        IconRole, 

        UserRole  = 32
    };

    QwtLegendData();
    ~QwtLegendData();

    void setValues( const QMap<int, QVariant> & );
    const QMap<int, QVariant> &values() const;

    void setValue( int role, const QVariant & );
    QVariant value( int role ) const;

    bool hasRole( int role ) const;
    bool isValid() const;

    QwtGraphic icon() const;
    QwtText title() const;
    Mode mode() const;

private:
    QMap<int, QVariant> d_map;
};

#endif

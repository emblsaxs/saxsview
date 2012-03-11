/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2003   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PIXEL_MATRIX_H
#define QWT_PIXEL_MATRIX_H

#include "qwt_global.h"
#include <qbitarray.h>
#include <qrect.h>

/*!
  \brief A bitfield corresponding to the pixels of a rectangle

  QwtPixelMatrix is intended to filter out duplicates in an
  unsorted array of points.
*/
class QWT_EXPORT QwtPixelMatrix: public QBitArray
{
public:
    QwtPixelMatrix( const QRect& rect );
    ~QwtPixelMatrix();

    void setRect( const QRect& rect );
    QRect rect() const;

    bool testPixel( const QPoint& pos ) const;
    bool testAndSetPixel( const QPoint& pos, bool on );

    int index( const QPoint& pos ) const;

private:
    QRect d_rect;
};

/*!
  \brief Test if a pixel has been set

  \param pos Position
  \return true, when pos is outside of rect(), or when the pixel
          has already been set.
 */
inline bool QwtPixelMatrix::testPixel( const QPoint& pos ) const
{
    const int idx = index( pos );
    return ( idx >= 0 ) ? testBit( idx ) : true;
}

/*!
  \brief Set a pixel and test if a pixel has been set before

  \param pos Position
  \param on Set/Clear the pixel

  \return true, when pos is outside of rect(), or when the pixel
          was set before.
 */
inline bool QwtPixelMatrix::testAndSetPixel( const QPoint& pos, bool on )
{
    const int idx = index( pos );
    if ( idx < 0 )
        return true;

    const bool onBefore = testBit( idx );
    setBit( idx, on );

    return onBefore;
}

/*!
  \brief Calculate the index in the bitfield correxponding to a position

  \param pos Position
  \return Index, when rect() contains pos - otherwise -1.
 */
inline int QwtPixelMatrix::index( const QPoint& pos ) const
{
    if ( !d_rect.contains( pos ) )
        return -1;

    return d_rect.width() * ( pos.y() - d_rect.y() )
           + ( pos.x() - d_rect.x() );
}

#endif

/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SERIES_STORE_H
#define QWT_SERIES_STORE_H

#include "qwt_global.h"
#include "qwt_series_data.h"

/*!
  \brief Bridge between QwtSeriesStore and QwtPlotSeriesItem

  QwtAbstractSeriesStore is an abstract interface only
  to make it possible to isolate the template based methods ( QwtSeriesStore )
  from the regular methods ( QwtPlotSeriesItem ) to make it possible
  to derive from QwtPlotSeriesItem without any hassle with templates.
*/
class QwtAbstractSeriesStore
{
protected:
    virtual ~QwtAbstractSeriesStore() {}

    //! dataChanged() indicates, that the series has been changed.
    virtual void dataChanged() = 0;

    /*!
      Set a the "rect of interest" for the stored series
      \sa QwtSeriesData<T>::setRectOfInterest()
     */
    virtual void setRectOfInterest( const QRectF & ) = 0;

    //! \return Bounding rectangle of the stored series
    virtual QRectF dataRect() const = 0;

    //! \return Number of samples
    virtual size_t dataSize() const = 0;
};

/*!
  \brief Class storing a QwtSeriesData object

  QwtSeriesStore and QwtPlotSeriesItem are intended as base classes for all
  plot items iterating over a series of samples. Both classes share
  a virtual base class ( QwtAbstractSeriesStore ) to bridge between them.

  QwtSeriesStore offers the template based part for the plot item API, so
  that QwtPlotSeriesItem can be derived without any hassle with templates.
 */
template <typename T>
class QwtSeriesStore: public virtual QwtAbstractSeriesStore
{
public:
    explicit QwtSeriesStore<T>();
    ~QwtSeriesStore<T>();

    void setData( QwtSeriesData<T> * );

    QwtSeriesData<T> *data();
    const QwtSeriesData<T> *data() const;

    T sample( int index ) const;

    virtual size_t dataSize() const;
    virtual QRectF dataRect() const;

    virtual void setRectOfInterest( const QRectF & );

private:
    QwtSeriesData<T> *d_series;
};

/*!
  \brief Constructor
  The store contains no series
*/
template <typename T>
QwtSeriesStore<T>::QwtSeriesStore():
    d_series( NULL )
{
}

//! Destructor
template <typename T>
QwtSeriesStore<T>::~QwtSeriesStore()
{
    delete d_series;
}

//! \return the the series data
template <typename T>
inline QwtSeriesData<T> *QwtSeriesStore<T>::data()
{
    return d_series;
}

//! \return the the series data
template <typename T>
inline const QwtSeriesData<T> *QwtSeriesStore<T>::data() const
{
    return d_series;
}

/*!
    \param index Index
    \return Sample at position index
*/
template <typename T>
inline T QwtSeriesStore<T>::sample( int index ) const
{
    return d_series ? d_series->sample( index ) : T();
}

/*!
  Assign a series of samples

  \param data Data
  \warning The item takes ownership of the data object, deleting
           it when its not used anymore.
*/
template <typename T>
void QwtSeriesStore<T>::setData( QwtSeriesData<T> *data )
{
    if ( d_series != data )
    {
        delete d_series;
        d_series = data;
        dataChanged();
    }
}

/*!
  Return the number of samples of the series

  \sa setData(), QwtSeriesData<T>::size()
*/
template <typename T>
size_t QwtSeriesStore<T>::dataSize() const
{
    if ( d_series == NULL )
        return 0;

    return d_series->size();
}

/*!
  Return the bounding rectangle of the series
  or an invalid rectangle, when no series is stored

  \sa QwtSeriesData<T>::boundingRect()
*/
template <typename T>
QRectF QwtSeriesStore<T>::dataRect() const
{
    if ( d_series == NULL )
        return QRectF( 1.0, 1.0, -2.0, -2.0 ); // invalid

    return d_series->boundingRect();
}

/*!
   Set a the "rect of interest" for the series
   \sa QwtSeriesData<T>::setRectOfInterest()
*/
template <typename T>
void QwtSeriesStore<T>::setRectOfInterest( const QRectF &rect )
{
    if ( d_series )
        d_series->setRectOfInterest( rect );
}

#endif

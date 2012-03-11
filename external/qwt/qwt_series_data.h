/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SERIES_DATA_H
#define QWT_SERIES_DATA_H 1

#include "qwt_global.h"
#include "qwt_samples.h"
#include "qwt_point_3d.h"
#include "qwt_point_polar.h"
#include <qvector.h>
#include <qrect.h>

/*!
   \brief Abstract interface for iterating over samples

   Qwt offers several implementations of the QwtSeriesData API,
   but in situations, where data of an application specific format
   needs to be displayed, without having to copy it, it is recommended
   to implement an individual data access.

   A subclass of QwtSeriesData<QPointF> must implement: 

   - size()\n 
     Should return number of data points.

   - sample()\n
     Should return values x and y values of the sample at specific position
     as QPointF object.

   - boundingRect()\n 
     Should return the bounding rectangle of the data series.
     It is used for autoscaling and might help certain algorithms for displaying
     the data. You can use qwtBoundingRect() for an implementation
     but often it is possible to implement a more efficient alogrithm 
     depending on the characteristics of the series.
     The member d_boundingRect is intended for caching the calculated rectangle.
    
*/
template <typename T>
class QwtSeriesData
{
public:
    QwtSeriesData();
    virtual ~QwtSeriesData();

    //! \return Number of samples
    virtual size_t size() const = 0;

    /*!
      Return a sample
      \param i Index
      \return Sample at position i
     */
    virtual T sample( size_t i ) const = 0;

    /*!
       Calculate the bounding rect of all samples

       The bounding rect is necessary for autoscaling and can be used
       for a couple of painting optimizations.

       qwtBoundingRect(...) offers slow implementations iterating
       over the samples. For large sets it is recommended to implement
       something faster f.e. by caching the bounding rect.
     */
    virtual QRectF boundingRect() const = 0;

    virtual void setRectOfInterest( const QRectF & );

protected:
    //! Can be used to cache a calculated bounding rectangle
    mutable QRectF d_boundingRect;

private:
    QwtSeriesData<T> &operator=( const QwtSeriesData<T> & );
};

//! Constructor
template <typename T>
QwtSeriesData<T>::QwtSeriesData():
    d_boundingRect( 0.0, 0.0, -1.0, -1.0 )
{
}

//! Destructor
template <typename T>
QwtSeriesData<T>::~QwtSeriesData()
{
}

/*!
   Set a the "rect of interest"

   QwtPlotSeriesItem defines the current area of the plot canvas
   as "rect of interest" ( QwtPlotSeriesItem::updateScaleDiv() ).
   It can be used to implement different levels of details.

   The default implementation does nothing.
*/
template <typename T>
void QwtSeriesData<T>::setRectOfInterest( const QRectF & )
{
}

/*!
  \brief Template class for data, that is organized as QVector

  QVector uses implicit data sharing and can be
  passed around as argument efficiently.
*/
template <typename T>
class QwtArraySeriesData: public QwtSeriesData<T>
{
public:
    QwtArraySeriesData();
    QwtArraySeriesData( const QVector<T> & );

    void setSamples( const QVector<T> & );
    const QVector<T> samples() const;

    virtual size_t size() const;
    virtual T sample( size_t ) const;

protected:
    //! Vector of samples
    QVector<T> d_samples;
};

//! Constructor
template <typename T>
QwtArraySeriesData<T>::QwtArraySeriesData()
{
}

/*!
   Constructor
   \param samples Array of samples
*/
template <typename T>
QwtArraySeriesData<T>::QwtArraySeriesData( const QVector<T> &samples ):
    d_samples( samples )
{
}

/*!
  Assign an array of samples
  \param samples Array of samples
*/
template <typename T>
void QwtArraySeriesData<T>::setSamples( const QVector<T> &samples )
{
    QwtSeriesData<T>::d_boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    d_samples = samples;
}

//! \return Array of samples
template <typename T>
const QVector<T> QwtArraySeriesData<T>::samples() const
{
    return d_samples;
}

//! \return Number of samples
template <typename T>
size_t QwtArraySeriesData<T>::size() const
{
    return d_samples.size();
}

/*!
  Return a sample
  \param i Index
  \return Sample at position i
*/
template <typename T>
T QwtArraySeriesData<T>::sample( size_t i ) const
{
    return d_samples[i];
}

//! Interface for iterating over an array of points
class QWT_EXPORT QwtPointSeriesData: public QwtArraySeriesData<QPointF>
{
public:
    QwtPointSeriesData(
        const QVector<QPointF> & = QVector<QPointF>() );

    virtual QRectF boundingRect() const;
};

//! Interface for iterating over an array of 3D points
class QWT_EXPORT QwtPoint3DSeriesData: public QwtArraySeriesData<QwtPoint3D>
{
public:
    QwtPoint3DSeriesData(
        const QVector<QwtPoint3D> & = QVector<QwtPoint3D>() );
    virtual QRectF boundingRect() const;
};

//! Interface for iterating over an array of intervals
class QWT_EXPORT QwtIntervalSeriesData: public QwtArraySeriesData<QwtIntervalSample>
{
public:
    QwtIntervalSeriesData(
        const QVector<QwtIntervalSample> & = QVector<QwtIntervalSample>() );

    virtual QRectF boundingRect() const;
};

//! Interface for iterating over an array of samples
class QWT_EXPORT QwtSetSeriesData: public QwtArraySeriesData<QwtSetSample>
{
public:
    QwtSetSeriesData(
        const QVector<QwtSetSample> & = QVector<QwtSetSample>() );

    virtual QRectF boundingRect() const;
};

/*!
    Interface for iterating over an array of OHLC samples
*/
class QWT_EXPORT QwtTradingChartData: public QwtArraySeriesData<QwtOHLCSample>
{
public:
    QwtTradingChartData(
        const QVector<QwtOHLCSample> & = QVector<QwtOHLCSample>() );

    virtual QRectF boundingRect() const;
};

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData<QPointF> &, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData<QwtPoint3D> &, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData<QwtPointPolar> &, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData<QwtIntervalSample> &, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData<QwtSetSample> &, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData<QwtOHLCSample> &, int from = 0, int to = -1 );

#endif

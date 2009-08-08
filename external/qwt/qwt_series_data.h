/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_SERIES_DATA_H
#define QWT_SERIES_DATA_H 1

#include "qwt_global.h"
#include "qwt_array.h"
#include "qwt_double_rect.h"
#include "qwt_double_interval.h"

//! A sample of the types (x1-x2, y) or (x, y1-y2)
class QWT_EXPORT QwtIntervalSample
{
public:
    inline QwtIntervalSample():
        value(0.0)
    {
    }

    inline QwtIntervalSample(double v, const QwtDoubleInterval &intv):
        value(v),
        interval(intv)
    {
    }

    bool operator==(const QwtIntervalSample &other) const
    {
        return value == other.value && interval == other.interval;
    }

    double value;
    QwtDoubleInterval interval;
};

//! A sample of the types (x1...xn, y) or (x, y1..yn)
class QWT_EXPORT QwtSetSample
{
public:
    QwtSetSample():
        value(0.0)
    {
    }

    bool operator==(const QwtSetSample &other) const
    {   
        return value == other.value && set == other.set;
    }

    double value;
    QwtArray<double> set;
};

/*!
   Abstract interface for iterating over samples

   Qwt offers several implementations of the QwtSeriesData API,
   but in situations, where data of an application specific format
   needs to be displayed, without having to copy it, it is recommended
   to implement an individual data access.
*/
template <typename T> 
class QwtSeriesData
{
public:
    virtual ~QwtSeriesData() {} 

    /*! 
       Virtual copy constructor

       When accessing a large amount of samples it is recommended
       to copy only the interface (shallow copy) to them.

       \return Pointer to a copy 
    */
    virtual QwtSeriesData *copy() const = 0;

    //! \return Number of samples
    virtual size_t size() const = 0;

    /*!
      Return a sample
      \param i Index
      \return Sample at position i
     */
    virtual T sample(size_t i) const = 0;

    /*!
       Calculate the bounding rect of all samples

       The bounding rect is necessary for autoscaling and can be used
       for a couple of painting optimizations.

       qwtBoundingRect(...) offers slow implementations iterating
       over the samples. For large sets it is recommended to implement
       something faster f.e. by caching the bounding rect.
     */
    virtual QwtDoubleRect boundingRect() const = 0;

    /*!
       Set a the "rect of interest"

       QwtPlotSeriesItem defines the current area of the plot canvas
       as "rect of interest" ( QwtPlotSeriesItem::updateScaleDiv() ).
       It can be used to implement different levels of details.

       The default implementation does nothing.
     */
    virtual void setRectOfInterest(const QwtDoubleRect &) {};

private:
    /*!
      Assignment operator (virtualized)
     */
    QwtSeriesData<T> &operator=(const QwtSeriesData<T> &);
};

/*!
  \brief Template class for data, that is organized as QwtArray

  QwtArray uses implicit data sharing and can be
  passed around as argument efficiently.
*/
template <typename T>
class QwtArraySeriesData: public QwtSeriesData<T>
{
public:
    QwtArraySeriesData();
    QwtArraySeriesData(const QwtArray<T> &);

    void setData(const QwtArray<T> &);
    const QwtArray<T> data() const;

    virtual size_t size() const;
    virtual T sample(size_t) const;

protected:
    QwtArray<T> d_samples;
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
QwtArraySeriesData<T>::QwtArraySeriesData(const QwtArray<T> &samples):
    d_samples(samples)
{
}
    
/*!
  Assign an array of samples
  \param samples Array of samples
*/
template <typename T>
void QwtArraySeriesData<T>::setData(const QwtArray<T> &samples)
{
    d_samples = samples;
}

//! \return Array of samples
template <typename T>
const QwtArray<T> QwtArraySeriesData<T>::data() const
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
T QwtArraySeriesData<T>::sample(size_t i) const
{
    return d_samples[i];
}

//! Interface for iterating over an array of points
class QWT_EXPORT QwtPointSeriesData: public QwtArraySeriesData<QwtDoublePoint>
{
public:
    QwtPointSeriesData(
        const QwtArray<QwtDoublePoint> & = QwtArray<QwtDoublePoint>());

    virtual QwtSeriesData<QwtDoublePoint> *copy() const;
    virtual QwtDoubleRect boundingRect() const;
};

//! Interface for iterating over an array of intervals
class QWT_EXPORT QwtIntervalSeriesData: public QwtArraySeriesData<QwtIntervalSample>
{
public:
    QwtIntervalSeriesData(
        const QwtArray<QwtIntervalSample> & = QwtArray<QwtIntervalSample>());

    virtual QwtSeriesData<QwtIntervalSample> *copy() const;
    virtual QwtDoubleRect boundingRect() const;
};

//! Interface for iterating over an array of samples
class QWT_EXPORT QwtSetSeriesData: public QwtArraySeriesData<QwtSetSample>
{
public:
    QwtSetSeriesData(
        const QwtArray<QwtSetSample> & = QwtArray<QwtSetSample>());

    virtual QwtSeriesData<QwtSetSample> *copy() const;
    virtual QwtDoubleRect boundingRect() const;
};

/*! 
  Interface for iterating over two QwtArray<double> objects.
*/
class QWT_EXPORT QwtPointArrayData: public QwtSeriesData<QwtDoublePoint>
{
public:
    QwtPointArrayData(const QwtArray<double> &x, const QwtArray<double> &y);
    QwtPointArrayData(const double *x, const double *y, size_t size);
    QwtPointArrayData &operator=(const QwtPointArrayData &);
    virtual QwtSeriesData<QwtDoublePoint> *copy() const;

    virtual QwtDoubleRect boundingRect() const;
    virtual size_t size() const;
    virtual QwtDoublePoint sample(size_t i) const;

    const QwtArray<double> &xData() const;
    const QwtArray<double> &yData() const;

private:
    QwtArray<double> d_x;
    QwtArray<double> d_y;
};

/*!
  \brief Data class containing two pointers to memory blocks of doubles.
 */
class QWT_EXPORT QwtCPointerData: public QwtSeriesData<QwtDoublePoint>
{
public:
    QwtCPointerData(const double *x, const double *y, size_t size);
    QwtCPointerData &operator=(const QwtCPointerData &);
    virtual QwtSeriesData<QwtDoublePoint> *copy() const;

    virtual QwtDoubleRect boundingRect() const;
    virtual size_t size() const;
    virtual QwtDoublePoint sample(size_t i) const;

    const double *xData() const;
    const double *yData() const;

private:
    const double *d_x;
    const double *d_y;
    size_t d_size;
};

/*!
  \brief Synthetic point data

  QwtSyntheticPointData provides a fixed number of points in an interval.
  The points are calculated in equidistant steps in x-direction.

  If the interval is invalid, the points are calculated for
  the "rect of interest", what normally is the displayed area on the
  plot canvas. In this mode you get different level of details, when
  zooming in/out.

  \par Example

  The following example shows how to implement a sinus curve.

  \verbatim
#include <cmath>
#include <qwt_series_data.h>
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <qapplication.h>

class SinusData: public QwtSyntheticPointData
{
public:
    SinusData():
        QwtSyntheticPointData(100)
    {
    }
    virtual QwtSeriesData<QwtDoublePoint> *copy() const
    {
        return new SinusData();
    }
    virtual double y(double x) const
    {
        return std::sin(x);
    }
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QwtPlot plot;
    plot.setAxisScale(QwtPlot::xBottom, 0.0, 10.0);
    plot.setAxisScale(QwtPlot::yLeft, -1.0, 1.0);

    QwtPlotCurve *curve = new QwtPlotCurve("y = sin(x)");
    curve->setData(SinusData());
    curve->attach(&plot);

    plot.show();
    return a.exec();
}
   \endverbatim
*/
class QWT_EXPORT QwtSyntheticPointData: public QwtSeriesData<QwtDoublePoint>
{
public:
    QwtSyntheticPointData(size_t size, 
        const QwtDoubleInterval & = QwtDoubleInterval());

    void setSize(size_t size);
    size_t size() const;

    void setInterval(const QwtDoubleInterval& );
    QwtDoubleInterval interval() const;

    virtual QwtDoubleRect boundingRect() const;
    virtual QwtDoublePoint sample(size_t i) const;

    /*! 
       Calculate a y value for a x value

       \param x x value
       \return Corresponding y value
     */
    virtual double y(double x) const = 0;
    virtual double x(uint index) const;

    virtual void setRectOfInterest(const QwtDoubleRect &);
    QwtDoubleRect rectOfInterest() const;

private:
    size_t d_size;
    QwtDoubleInterval d_interval;
    QwtDoubleRect d_rectOfInterest;
    QwtDoubleInterval d_intervalOfInterest;
};


QWT_EXPORT QwtDoubleRect qwtBoundingRect(
    const QwtSeriesData<QwtDoublePoint> &);
QWT_EXPORT QwtDoubleRect qwtBoundingRect(
    const QwtSeriesData<QwtIntervalSample> &);
QWT_EXPORT QwtDoubleRect qwtBoundingRect(
    const QwtSeriesData<QwtSetSample> &);

#if defined(QWT_TEMPLATEDLL)
// MOC_SKIP_BEGIN
#if QT_VERSION < 0x040000 // there is already a QVector<QPointF> in qvector.h
template class QWT_EXPORT QwtArray<QwtDoublePoint>;
#endif
template class QWT_EXPORT QwtArray<QwtIntervalSample>;
template class QWT_EXPORT QwtArray<QwtSetSample>;
// MOC_SKIP_END
#endif

#endif // !QWT_SERIES_DATA_H

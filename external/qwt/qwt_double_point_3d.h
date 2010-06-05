/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

/*! \file */
#ifndef QWT_DOUBLE_POINT_3D_H
#define QWT_DOUBLE_POINT_3D_H 1

#include "qwt_global.h"
#include <qpoint.h>

/*!
  \brief QwtDoublePoint3D class defines a 3D point in double coordinates
*/

class QWT_EXPORT QwtDoublePoint3D
{
public:
    QwtDoublePoint3D();
    QwtDoublePoint3D(double x, double y, double z);
    QwtDoublePoint3D(const QwtDoublePoint3D &);
    QwtDoublePoint3D(const QPointF &);

    bool isNull()    const;

    double x() const;
    double y() const;
    double z() const;

    double &rx();
    double &ry();
    double &rz();

    void setX(double x);
    void setY(double y);
    void setZ(double y);

    QPointF toPoint() const;

    bool operator==(const QwtDoublePoint3D &) const;
    bool operator!=(const QwtDoublePoint3D &) const;

private:
    double d_x;
    double d_y;
    double d_z;
};

/*! 
    Constructs a null point.
    \sa isNull()
*/
inline QwtDoublePoint3D::QwtDoublePoint3D():
    d_x(0.0),
    d_y(0.0),
    d_z(0.0)
{
}

//! Constructs a point with coordinates specified by x, y and z.
inline QwtDoublePoint3D::QwtDoublePoint3D(double x, double y, double z = 0.0):
    d_x(x),
    d_y(y),
    d_z(z)
{
}
    
/*! 
    Copy constructor. 
    Constructs a point using the values of the point specified.
*/
inline QwtDoublePoint3D::QwtDoublePoint3D(const QwtDoublePoint3D &other):
    d_x(other.d_x),
    d_y(other.d_y),
    d_z(other.d_z)
{
}

/*! 
    Constructs a point with x and y coordinates from a 2D point, 
    and a z coordinate of 0.
*/
inline QwtDoublePoint3D::QwtDoublePoint3D(const QPointF &other):
    d_x(other.x()),
    d_y(other.y()),
    d_z(0.0)
{
}

/*! 
    Returns true if the point is null; otherwise returns false.

    A point is considered to be null if x, y and z-coordinates 
    are equal to zero.
*/
inline bool QwtDoublePoint3D::isNull() const
{ 
    return d_x == 0.0 && d_y == 0.0 && d_z == 0; 
}

//! Returns the x-coordinate of the point.
inline double QwtDoublePoint3D::x() const
{ 
    return d_x; 
}

//! Returns the y-coordinate of the point.
inline double QwtDoublePoint3D::y() const
{   
    return d_y; 
}

//! Returns the z-coordinate of the point.
inline double QwtDoublePoint3D::z() const
{   
    return d_z; 
}

//! Returns a reference to the x-coordinate of the point.
inline double &QwtDoublePoint3D::rx()
{
    return d_x;
}

//! Returns a reference to the y-coordinate of the point.
inline double &QwtDoublePoint3D::ry()
{
    return d_y;
}

//! Returns a reference to the z-coordinate of the point.
inline double &QwtDoublePoint3D::rz()
{
    return d_z;
}

//! Sets the x-coordinate of the point to the value specified by x.
inline void QwtDoublePoint3D::setX(double x)
{ 
    d_x = x; 
}

//! Sets the y-coordinate of the point to the value specified by y.
inline void QwtDoublePoint3D::setY(double y)
{ 
    d_y = y; 
}

//! Sets the z-coordinate of the point to the value specified by z.
inline void QwtDoublePoint3D::setZ(double z)
{ 
    d_z = z; 
}

/*!
   Rounds 2D point, where the z coordinate is dropped.
*/
inline QPointF QwtDoublePoint3D::toPoint() const
{
    return QPointF(d_x, d_y);
}

//! Returns true if this point and other are equal; otherwise returns false. 
inline bool QwtDoublePoint3D::operator==(const QwtDoublePoint3D &other) const
{
    return (d_x == other.d_x) && (d_y == other.d_y) && (d_z == other.d_z);
}

//! Returns true if this rect and other are different; otherwise returns false. 
inline bool QwtDoublePoint3D::operator!=(const QwtDoublePoint3D &other) const
{
    return !operator==(other);
}

#endif

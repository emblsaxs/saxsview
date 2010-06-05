/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_spectrocurve.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include <qpainter.h>

class QwtPlotSpectroCurve::PrivateData
{
public:
    PrivateData():
        colorRange(0.0, 1000.0),
        paintAttributes(QwtPlotSpectroCurve::ClipPoints)
    {
        colorMap = new QwtLinearColorMap();
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    QwtColorMap *colorMap;
    QwtDoubleInterval colorRange;
    QVector<QRgb> colorTable;
    int paintAttributes;
};

/*!
  Constructor
  \param title Title of the curve   
*/
QwtPlotSpectroCurve::QwtPlotSpectroCurve(const QwtText &title):
    QwtPlotSeriesItem<QwtDoublePoint3D>(title)
{
    init();
}

/*!
  Constructor
  \param title Title of the curve   
*/
QwtPlotSpectroCurve::QwtPlotSpectroCurve(const QString &title):
    QwtPlotSeriesItem<QwtDoublePoint3D>(QwtText(title))
{
    init();
}

//! Destructor
QwtPlotSpectroCurve::~QwtPlotSpectroCurve()
{
    delete d_data;
}

/*!
  \brief Initialize data members
*/
void QwtPlotSpectroCurve::init()
{
    setItemAttribute(QwtPlotItem::Legend);
    setItemAttribute(QwtPlotItem::AutoScale);

    d_data = new PrivateData;
    d_series = new QwtPoint3DSeriesData();

    setZ(20.0);
}

//! \return QwtPlotItem::Rtti_PlotCurve
int QwtPlotSpectroCurve::rtti() const
{
    return QwtPlotItem::Rtti_PlotSpectroCurve;
}

/*!
  Specify an attribute how to draw the curve

  \param attribute Paint attribute
  \param on On/Off
  /sa PaintAttribute, testPaintAttribute()
*/
void QwtPlotSpectroCurve::setPaintAttribute(PaintAttribute attribute, bool on)
{
    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;
}

/*!
    \brief Return the current paint attributes
    \sa PaintAttribute, setPaintAttribute()
*/
bool QwtPlotSpectroCurve::testPaintAttribute(PaintAttribute attribute) const
{
    return (d_data->paintAttributes & attribute);
}

void QwtPlotSpectroCurve::setSamples(const QVector<QwtDoublePoint3D> &data)
{
    delete d_series;
    d_series = new QwtPoint3DSeriesData(data);
    itemChanged();
}

/*!
  Change the color map

  Often it is useful to display the mapping between intensities and
  colors as an additional plot axis, showing a color bar.

  \param colorMap Color Map

  \sa colorMap(), QwtScaleWidget::setColorBarEnabled(),
      QwtScaleWidget::setColorMap()
*/
void QwtPlotSpectroCurve::setColorMap(const QwtColorMap &colorMap)
{
    delete d_data->colorMap;
    d_data->colorMap = colorMap.copy();

    itemChanged();
}

/*!
   \return Color Map used for mapping the intensity values to colors
   \sa setColorMap()
*/
const QwtColorMap &QwtPlotSpectroCurve::colorMap() const
{
    return *d_data->colorMap;
}

void QwtPlotSpectroCurve::setColorRange(const QwtDoubleInterval &interval)
{
    if ( interval != d_data->colorRange )
    {
        d_data->colorRange = interval;
        itemChanged();
    }
}

QwtDoubleInterval &QwtPlotSpectroCurve::colorRange() const
{
    return d_data->colorRange;
}

void QwtPlotSpectroCurve::drawSeries(QPainter *painter, 
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from >= to )
        return;

    drawDots(painter, xMap, yMap, canvasRect, from, to);
}

void QwtPlotSpectroCurve::drawDots(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to) const
{
    if ( !d_data->colorRange.isValid() )
        return;

    const QwtColorMap::Format format = d_data->colorMap->format();
    if ( format == QwtColorMap::Indexed )
        d_data->colorTable = d_data->colorMap->colorTable(d_data->colorRange);

    for (int i = from; i <= to; i++)
    {   
        const QwtDoublePoint3D sample = d_series->sample(i);
    
        const double xi = xMap.transform(sample.x());
        const double yi = yMap.transform(sample.y());

        if ( d_data->paintAttributes & QwtPlotSpectroCurve::ClipPoints )
        {
            if ( !canvasRect.contains(xi, yi ) )
                continue;
        } 

        if ( format == QwtColorMap::RGB )
        {
            const QRgb rgb = d_data->colorMap->rgb(
                d_data->colorRange, sample.z());

            painter->setPen(QPen(QColor(rgb)));
        }
        else
        {
            const unsigned char index = d_data->colorMap->colorIndex(
                d_data->colorRange, sample.z());

            painter->setPen(QPen(d_data->colorTable[index]));
        }

        QwtPainter::drawPoint(painter, QPointF(xi, yi));
    }
    
    d_data->colorTable.clear();
}

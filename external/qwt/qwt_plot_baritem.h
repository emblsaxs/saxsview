/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_BAR_ITEM_H
#define QWT_PLOT_BAR_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_series_data.h"

class QwtColumnRect;
class QwtColumnSymbol;

class QWT_EXPORT QwtPlotBarItem: public QwtPlotSeriesItem
{
public:
    enum ChartAttribute
    {
        ShowLabels = 0x01
    };

    typedef QFlags<ChartAttribute> ChartAttributes;

    /*!
        \brief Mode how to calculate the bar width

        setLayoutPolicy(), setLayoutHint()
     */
    enum LayoutPolicy
    {
        /*!
          The sample width is calculated by deviding the bounding rectangle
          by the number of samples.

          \sa boundingRectangle()
          \note The layoutHint() is ignored
         */
        AutoAdjustSamples,

        /*!
          The barWidthHint() defines an interval in axis coordinates
         */
        ScaleSamplesToAxes,

        /*!
          The bar width is calculated by multiplying the barWidthHint()
          with the height or width of the canvas.

          \sa boundingRectangle()
         */
        ScaleSampleToCanvas,

        /*!
          The barWidthHint() defines a fixed width in paint device coordinates.
         */
        FixedSampleSize
    };

    explicit QwtPlotBarItem( const QwtText &title );
    virtual ~QwtPlotBarItem();

    void setChartAttribute( ChartAttribute, bool on = true );
    bool testChartAttribute( ChartAttribute ) const;

    void setLayoutPolicy( LayoutPolicy );
    LayoutPolicy layoutPolicy() const;

    void setLayoutHint( double );
    double layoutHint() const;

    void setSpacing( int );
    int spacing() const;

    void setMargin( int );
    int margin() const;

    void setBaseline( double );
    double baseline() const;

    virtual void getCanvasMarginHint( 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect,
        double &left, double &top, double &right, double &bottom) const;

protected:
    double sampleWidth( const QwtScaleMap &map,
        double canvasSize, double dataSize,
        double value ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotBarItem::ChartAttributes )

#endif

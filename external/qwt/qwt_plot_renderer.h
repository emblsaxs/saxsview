/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_RENDERER_H
#define QWT_PLOT_RENDERER_H

#include "qwt_global.h"
#include <qobject.h>

class QwtPlot;
class QwtScaleMap;
class QSizeF;
class QRectF;
class QPainter;
class QPrinter;
class QPaintDevice;
#ifdef QT_SVG_LIB
class QSvgGenerator;
#endif

/*!
    \brief Renderer for exporting a plot to a document, a printer
           or anything else, that is supported by QPainter/QPaintDevice
*/
class QWT_EXPORT QwtPlotRenderer: public QObject
{
    Q_OBJECT

public:
    enum DiscardFlag
    {
        DiscardNone             = 0x0,

        DiscardBackground       = 0x1,
        DiscardTitle            = 0x2,
        DiscardLegend           = 0x4,
        DiscardCanvasBackground = 0x8
    };

    Q_DECLARE_FLAGS(DiscardFlags, DiscardFlag)

    enum LayoutFlag
    {
        DefaultLayout   = 0x0,

        KeepMargins     = 0x1,
        KeepFrames      = 0x2,
        FrameWithScales = 0x4
    };

    Q_DECLARE_FLAGS(LayoutFlags, LayoutFlag)
    
    explicit QwtPlotRenderer(QObject * = NULL);
    virtual ~QwtPlotRenderer();

    void setDiscardFlag(DiscardFlag flag, bool on = true);
    bool testDiscardFlag(DiscardFlag flag) const;

    void setDiscardFlags(DiscardFlags flags);
    DiscardFlags discardFlags() const;

    void setLayoutFlag(LayoutFlag flag, bool on = true);
    bool testLayoutFlag(LayoutFlag flag) const;

    void setLayoutFlags(LayoutFlags flags);
    LayoutFlags layoutFlags() const;

    void renderDocument(QwtPlot *, const QString &format,
        const QSizeF &sizeMM, int resolution = 85);

    void renderDocument(QwtPlot *, 
        const QString &title, const QString &format,
        const QSizeF &sizeMM, int resolution = 85);
        
#ifdef QT_SVG_LIB
#if QT_VERSION >= 0x040500
    void renderTo(QwtPlot *, QSvgGenerator &) const;
#endif
#endif
    void renderTo(QwtPlot *, QPrinter &) const;
    void renderTo(QwtPlot *, QPaintDevice &p) const;

    virtual void render(QwtPlot *, 
        QPainter *, const QRectF &rect) const;

protected:
    virtual void renderLegendItem(QPainter *, 
        const QWidget *, const QRectF &) const;

    virtual void renderTitle(QPainter *, const QRectF &) const;

    virtual void renderScale(QPainter *, 
        int axisId, int startDist, int endDist,
        int baseDist, const QRectF &) const;

    virtual void renderCanvas(QPainter *, const QRectF &canvasRect,
        const QwtScaleMap* maps) const;

    virtual void renderLegend(QPainter *, const QRectF &) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotRenderer::DiscardFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotRenderer::LayoutFlags);

#endif

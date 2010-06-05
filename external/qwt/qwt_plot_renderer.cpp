/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_renderer.h"
#include "qwt_plot.h"
#include "qwt_painter.h"
#include "qwt_legend_item.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_legend.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_engine.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "qwt_math.h"
#include <qpainter.h>
#include <qpaintengine.h>
#include <qtransform.h>
#include <qprinter.h>
#include <qimagewriter.h>
#include <qfileinfo.h>
#ifdef QT_SVG_LIB
#include <qsvggenerator.h>
#endif

class QwtPlotRenderer::PrivateData
{
public:
    PrivateData():
        discardFlags(QwtPlotRenderer::DiscardBackground),
        layoutFlags(QwtPlotRenderer::DefaultLayout),
        plot(NULL)
    {
    }

    QwtPlotRenderer::DiscardFlags discardFlags;
    QwtPlotRenderer::LayoutFlags layoutFlags;
    QwtPlot *plot;
};

//! Constructor
QwtPlotRenderer::QwtPlotRenderer(QObject *parent):
    QObject(parent)
{
    d_data = new PrivateData;
}

//! Destructor
QwtPlotRenderer::~QwtPlotRenderer()
{
    delete d_data;
}

void QwtPlotRenderer::setDiscardFlag(DiscardFlag flag, bool on)
{
    if ( on )
        d_data->discardFlags |= flag;
    else
        d_data->discardFlags &= ~flag;
}

bool QwtPlotRenderer::testDiscardFlag(DiscardFlag flag) const
{
    return d_data->discardFlags & flag;
}

void QwtPlotRenderer::setDiscardFlags(DiscardFlags flags)
{
    d_data->discardFlags = flags;
}

QwtPlotRenderer::DiscardFlags QwtPlotRenderer::discardFlags() const
{
    return d_data->discardFlags;
}

void QwtPlotRenderer::setLayoutFlag(LayoutFlag flag, bool on)
{
    if ( on )
        d_data->layoutFlags |= flag;
    else
        d_data->layoutFlags &= ~flag;
}

bool QwtPlotRenderer::testLayoutFlag(LayoutFlag flag) const
{
    return d_data->layoutFlags & flag;
}   

void QwtPlotRenderer::setLayoutFlags(LayoutFlags flags)
{
    d_data->layoutFlags = flags;
}

QwtPlotRenderer::LayoutFlags QwtPlotRenderer::layoutFlags() const
{
    return d_data->layoutFlags;
}

void QwtPlotRenderer::renderDocument(QwtPlot *plot, 
    const QString &fileName, const QSizeF &sizeMM, int resolution)
{
    renderDocument(plot, fileName, 
        QFileInfo(fileName).suffix(), sizeMM, resolution);
}
        

void QwtPlotRenderer::renderDocument(QwtPlot *plot, 
    const QString &fileName, const QString &format,
    const QSizeF &sizeMM, int resolution)
{
    if ( plot == NULL || sizeMM.isEmpty() || resolution <= 0 )
        return;

    QString title = plot->title().text();
    if ( title.isEmpty() )
        title = "Plot Document";

    const double toInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * toInch * resolution; 

    const QRectF documentRect(0.0, 0.0, size.width(), size.height());

    const QString fmt = format.toLower();
    if ( format == "pdf" || format == "ps" )
    {
        QPrinter printer;
        printer.setResolution(resolution);
        printer.setFullPage(true);
        printer.setPaperSize(sizeMM, QPrinter::Millimeter);
        printer.setDocName(title);
        printer.setOutputFileName(fileName);
        printer.setOutputFormat( (format == "pdf") 
            ? QPrinter::PdfFormat : QPrinter::PostScriptFormat);
        
        QPainter painter(&printer);
        render(plot, &painter, documentRect);
    }
    else if ( format == "svg" )
    {
#ifdef QT_SVG_LIB
#if QT_VERSION >= 0x040500
        QSvgGenerator generator;
        generator.setTitle(title);
        generator.setFileName(fileName);
        generator.setResolution(resolution);
        generator.setViewBox( documentRect );

        QPainter painter(&generator);
        render(plot, &painter, documentRect);
#endif
#endif
    }
    else
    {
        if ( QImageWriter::supportedImageFormats().indexOf(
            format.toLatin1() ) >= 0 )
        {
            const QRect imageRect = documentRect.toRect();
            QImage image(imageRect.size(), QImage::Format_ARGB32);
            image.fill(QColor(Qt::white).rgb());

            QPainter painter(&image);
            render(plot, &painter, imageRect);
            painter.end();

            image.save(fileName, format.toLatin1());
        }
    }
}

/*!
  \brief Render the plot to a \c QPaintDevice 
  This function renders the contents of a QwtPlot instance to
  \c QPaintDevice object. The target rectangle is derived from 
  its device metrics.

  \param paintDevice device to paint on, f.e a QImage
*/

void QwtPlotRenderer::renderTo(
    QwtPlot *plot, QPaintDevice &paintDevice) const
{
    int w = paintDevice.width();
    int h = paintDevice.height();

    QPainter p(&paintDevice);
    render(plot, &p, QRectF(0, 0, w, h));
}

/*!
  \brief Render the plot to a QPrinter

  This function renders the contents of a QwtPlot instance to
  \c QPaintDevice object. The size is derived from the printer
  metrics.

  \param printer Printer to paint on

  \sa QwtPlotPrintFilter
*/

void QwtPlotRenderer::renderTo(
    QwtPlot *plot, QPrinter &printer) const
{
    int w = printer.width();
    int h = printer.height();

    QRectF rect(0, 0, w, h);
    double aspect = rect.width() / rect.height();
    if ((aspect < 1.0))
        rect.setHeight(aspect * rect.width());

    QPainter p(&printer);
    render(plot, &p, rect);
}

/*!
  \brief Render the plot to a QSvgGenerator

  If the generator has a view box, the plot will be rendered into it.
  If it has no viewBox but a valid size the target coordinates
  will be (0, 0, generator.width(), generator.height()). Otherwise
  the target rectangle will be QRectF(0, 0, 800, 600);

  \param generator SVG generator
*/

#ifdef QT_SVG_LIB 
#if QT_VERSION >= 0x040500
void QwtPlotRenderer::renderTo(
    QwtPlot *plot, QSvgGenerator &generator) const
{
    QRectF rect = generator.viewBoxF();
    if ( rect.isEmpty() )
        rect.setRect(0, 0, generator.width(), generator.height());

    if ( rect.isEmpty() )
        rect.setRect(0, 0, 800, 600); // something
        
    QPainter p(&generator);
    render(plot, &p, rect);
}
#endif
#endif

/*!
  \brief Paint the plot into a given rectangle.
  Paint the contents of a QwtPlot instance into a given rectangle.

  \param painter Painter
  \param plotRect Bounding rectangle
*/
void QwtPlotRenderer::render(QwtPlot *plot, 
    QPainter *painter, const QRectF &plotRect) const
{
    int axisId;

    if ( painter == 0 || !painter->isActive() ||
            !plotRect.isValid() || plot->size().isNull() )
       return;

    if ( !(d_data->discardFlags & DiscardBackground) )
    {
        const QBrush brush = plot->palette().brush(plot->backgroundRole());
        painter->fillRect(plotRect, brush);
    }

    /* 
      The layout engine uses the same methods as they are used
      by the Qt layout system. Therefore we need to calculate the 
      layout in screen coordinates and paint with a scaled painter.
     */
    QTransform transform;
    transform.scale(
        double(painter->device()->logicalDpiX()) / plot->logicalDpiX(),
        double(painter->device()->logicalDpiY()) / plot->logicalDpiY() );
        
    d_data->plot = plot;

    painter->save();

    int baseLineDists[QwtPlot::axisCnt];
    if ( d_data->layoutFlags & FrameWithScales )
    {
        for (axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
        {
            QwtScaleWidget *scaleWidget = 
                (QwtScaleWidget *)plot->axisWidget(axisId);
            if ( scaleWidget )
            {
                baseLineDists[axisId] = scaleWidget->margin();
                scaleWidget->setMargin(0);
            }
        }
    }
    // Calculate the layout for the print.

    int layoutOptions = QwtPlotLayout::IgnoreScrollbars 
        | QwtPlotLayout::IgnoreFrames;
    if ( !d_data->layoutFlags & KeepMargins )
        layoutOptions |= QwtPlotLayout::IgnoreMargin;
    if ( d_data->discardFlags & DiscardLegend )
        layoutOptions |= QwtPlotLayout::IgnoreLegend;

    const QRectF layoutRect = transform.inverted().mapRect(plotRect);
    plot->plotLayout()->activate(plot, layoutRect, layoutOptions);

    painter->setWorldTransform(transform);

    if ( !(d_data->discardFlags & DiscardTitle)
        && (!plot->titleLabel()->text().isEmpty()))
    {
        renderTitle(painter, plot->plotLayout()->titleRect());
    }

    if ( !(d_data->discardFlags & DiscardLegend)
        && plot->legend() && !plot->legend()->isEmpty() )
    {
        renderLegend(painter, plot->plotLayout()->legendRect());
    }

    for ( axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
    {
        QwtScaleWidget *scaleWidget = plot->axisWidget(axisId);
        if (scaleWidget)
        {
            int baseDist = scaleWidget->margin();

            int startDist, endDist;
            scaleWidget->getBorderDistHint(startDist, endDist);

            renderScale(painter, axisId, startDist, endDist,
                baseDist, plot->plotLayout()->scaleRect(axisId));
        }
    }

    QRectF canvasRect = plot->plotLayout()->canvasRect();

    QwtScaleMap map[QwtPlot::axisCnt];
    for (axisId = 0; axisId < QwtPlot::axisCnt; axisId++)
    {
        map[axisId].setTransformation(
            plot->axisScaleEngine(axisId)->transformation());

        const QwtScaleDiv &scaleDiv = *plot->axisScaleDiv(axisId);
        map[axisId].setScaleInterval(
            scaleDiv.lowerBound(), scaleDiv.upperBound());

        double from, to;
        if ( plot->axisEnabled(axisId) )
        {
            const int sDist = plot->axisWidget(axisId)->startBorderDist();
            const int eDist = plot->axisWidget(axisId)->endBorderDist();
            const QRectF &scaleRect = plot->plotLayout()->scaleRect(axisId);

            if ( axisId == QwtPlot::xTop || axisId == QwtPlot::xBottom )
            {
                from = scaleRect.left() + sDist;
                to = scaleRect.right() - eDist;
            }
            else
            {
                from = scaleRect.bottom() - eDist;
                to = scaleRect.top() + sDist;
            }
        }
        else
        {
            int margin = plot->plotLayout()->canvasMargin(axisId);
            if ( axisId == QwtPlot::yLeft || axisId == QwtPlot::yRight )
            {
                from = canvasRect.bottom() - margin;
                to = canvasRect.top() + margin;
            }
            else
            {
                from = canvasRect.left() + margin;
                to = canvasRect.right() - margin;
            }
        }
        map[axisId].setPaintInterval(from, to);
    }

    // canvas 
    renderCanvas(painter, canvasRect, map);

    plot->plotLayout()->invalidate();

    // reset all widgets with their original attributes.
    if ( d_data->layoutFlags & FrameWithScales )
    {
        // restore the previous base line dists

        for (axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
        {
            QwtScaleWidget *scaleWidget = plot->axisWidget(axisId);
            if ( scaleWidget  )
                scaleWidget->setMargin(baseLineDists[axisId]);
        }
    }

    painter->restore();

    d_data->plot = NULL;
}

/*!
  Print the title into a given rectangle.

  \param painter Painter
  \param rect Bounding rectangle
*/

void QwtPlotRenderer::renderTitle(QPainter *painter, const QRectF &rect) const
{
    const QwtPlot *plot = d_data->plot;

    painter->setFont(plot->titleLabel()->font());

    const QColor color = plot->titleLabel()->palette().color(
            QPalette::Active, QPalette::Text);

    painter->setPen(color);
    plot->titleLabel()->text().draw(painter, rect);
}

/*!
  Print the legend into a given rectangle.

  \param painter Painter
  \param rect Bounding rectangle
*/

void QwtPlotRenderer::renderLegend(QPainter *painter, const QRectF &rect) const
{
    const QwtPlot *plot = d_data->plot;

    if ( !plot->legend() || plot->legend()->isEmpty() )
        return;

    QLayout *l = plot->legend()->contentsWidget()->layout();
    if ( l == 0 || !l->inherits("QwtDynGridLayout") )
        return;

    QwtDynGridLayout *legendLayout = (QwtDynGridLayout *)l;

    uint numCols = legendLayout->columnsForWidth(rect.width());
    QList<QRect> itemRects = 
        legendLayout->layoutItems(rect.toRect(), numCols);

    int index = 0;

    for ( int i = 0; i < legendLayout->count(); i++ )
    {
        QLayoutItem *item = legendLayout->itemAt(i);
        QWidget *w = item->widget();
        if ( w )
        {
            painter->save();

            painter->setClipRect(itemRects[index]);
            renderLegendItem(painter, w, itemRects[index]);

            index++;
            painter->restore();
        }
    }
}

/*!
  Print the legend item into a given rectangle.

  \param painter Painter
  \param w Widget representing a legend item
  \param rect Bounding rectangle
*/

void QwtPlotRenderer::renderLegendItem(QPainter *painter, 
    const QWidget *w, const QRectF &rect) const
{
    const QwtPlot *plot = d_data->plot;

    if ( w->inherits("QwtLegendItem") )
    {
        QwtLegendItem *item = (QwtLegendItem *)w;

        const QRect identifierRect(
            rect.x() + item->margin(), rect.y(),
            item->identifierSize().width(), rect.height());

        QwtLegendItemManager *itemManger = plot->legend()->find(item);
        if ( itemManger )
        {
            painter->save();
            itemManger->drawLegendIdentifier(painter, identifierRect);
            painter->restore();
        }

        // Label
    
        QRectF titleRect = rect;
        titleRect.setX(identifierRect.right() + 2 * item->spacing());

        painter->setFont(item->font());
        item->text().draw(painter, titleRect);
    }
}

/*!
  \brief Paint a scale into a given rectangle.
  Paint the scale into a given rectangle.

  \param painter Painter
  \param axisId Axis
  \param startDist Start border distance
  \param endDist End border distance
  \param baseDist Base distance
  \param rect Bounding rectangle
*/

void QwtPlotRenderer::renderScale(QPainter *painter,
    int axisId, int startDist, int endDist, int baseDist, 
    const QRectF &rect) const
{
    const QwtPlot *plot = d_data->plot;

    if (!plot->axisEnabled(axisId))
        return;

    const QwtScaleWidget *scaleWidget = plot->axisWidget(axisId);
    if ( scaleWidget->isColorBarEnabled() 
        && scaleWidget->colorBarWidth() > 0)
    {
        scaleWidget->drawColorBar(painter, scaleWidget->colorBarRect(rect));

        const int off = scaleWidget->colorBarWidth() + scaleWidget->spacing();
        if ( scaleWidget->scaleDraw()->orientation() == Qt::Horizontal )
            baseDist += off;
        else
            baseDist += off;
    }

    painter->save();

    QwtScaleDraw::Alignment align;
    double x, y, w;

    switch(axisId)
    {
        case QwtPlot::yLeft:
        {
            x = rect.right() - 1.0 - baseDist;
            y = rect.y() + startDist;
            w = rect.height() - startDist - endDist;
            align = QwtScaleDraw::LeftScale;
            break;
        }
        case QwtPlot::yRight:
        {
            x = rect.left() + baseDist;
            y = rect.y() + startDist;
            w = rect.height() - startDist - endDist;
            align = QwtScaleDraw::RightScale;
            break;
        }
        case QwtPlot::xTop:
        {
            x = rect.left() + startDist;
            y = rect.bottom() - 1.0 - baseDist;
            w = rect.width() - startDist - endDist;
            align = QwtScaleDraw::TopScale;
            break;
        }
        case QwtPlot::xBottom:
        {
            x = rect.left() + startDist;
            y = rect.top() + baseDist;
            w = rect.width() - startDist - endDist;
            align = QwtScaleDraw::BottomScale;
            break;
        }
        default:
            return;
    }

    scaleWidget->drawTitle(painter, align, rect);

    painter->setFont(scaleWidget->font());

    QwtScaleDraw *sd = (QwtScaleDraw *)scaleWidget->scaleDraw();
    const QPointF sdPos = sd->pos();
    const double sdLength = sd->length();

    sd->move(x, y);
    sd->setLength(w);

    QPalette palette = scaleWidget->palette();
    palette.setCurrentColorGroup(QPalette::Active);
    sd->draw(painter, palette);

    // reset previous values
    sd->move(sdPos); 
    sd->setLength(sdLength); 

    painter->restore();
}

/*!
  Print the canvas into a given rectangle.

  \param painter Painter
  \param map Maps mapping between plot and paint device coordinates
  \param canvasRect Canvas rectangle
*/

void QwtPlotRenderer::renderCanvas(QPainter *painter, 
    const QRectF &canvasRect, const QwtScaleMap *map) const
{
    const QwtPlot *plot = d_data->plot;

    painter->save();

    QRectF r = canvasRect.adjusted(0.0, 0.0, -1.0, -1.0);

    if ( d_data->layoutFlags & FrameWithScales )
    {
        r.adjust(-1.0, -1.0, 1.0, 1.0);
        painter->setPen(QPen(Qt::black));
    }
    else
        painter->setPen(Qt::NoPen);

    if ( !( d_data->discardFlags & DiscardCanvasBackground) )
    {
        const QBrush bgBrush = 
            plot->canvas()->palette().brush(plot->backgroundRole());
        painter->setBrush(bgBrush);
    }

    QwtPainter::drawRect(painter, r);

    painter->restore();

    painter->setClipRect(canvasRect);
    d_data->plot->drawItems(painter, canvasRect, map);
}

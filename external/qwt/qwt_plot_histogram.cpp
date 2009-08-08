#include <qstring.h>
#include <qpainter.h>
#include "qwt_plot.h"
#include "qwt_painter.h"
#include "qwt_column_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_plot_histogram.h"

class QwtPlotHistogram::PrivateData
{
public:
    PrivateData():
        reference(0.0),
        curveStyle(NoCurve)
    {
        symbol = new QwtColumnSymbol(QwtColumnSymbol::NoSymbol);
    }

    ~PrivateData()
    {
        delete symbol;
    }

    double reference;

    QPen pen;
    QBrush brush;
    QwtPlotHistogram::CurveStyle curveStyle;
    QwtColumnSymbol *symbol;
};

QwtPlotHistogram::QwtPlotHistogram(const QwtText &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

QwtPlotHistogram::QwtPlotHistogram(const QString &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

QwtPlotHistogram::~QwtPlotHistogram()
{
    delete d_data;
}

void QwtPlotHistogram::init()
{
    d_data = new PrivateData();
    d_series = new QwtIntervalSeriesData();

    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, true);

    setZ(20.0);
}

void QwtPlotHistogram::setStyle(CurveStyle style)
{
    if ( style != d_data->curveStyle )
    {
        d_data->curveStyle = style;
        itemChanged();
    }
}

QwtPlotHistogram::CurveStyle QwtPlotHistogram::style() const
{
    return d_data->curveStyle;
}

void QwtPlotHistogram::setPen(const QPen &pen)
{
    if ( pen != d_data->pen )
    {
        d_data->pen = pen;
        itemChanged();
    }
}

const QPen &QwtPlotHistogram::pen() const
{
    return d_data->pen;
}

void QwtPlotHistogram::setBrush(const QBrush &brush)
{
    if ( brush != d_data->brush )
    { 
        d_data->brush = brush;
        itemChanged();
    }
}

const QBrush &QwtPlotHistogram::brush() const
{
    return d_data->brush; 
}

void QwtPlotHistogram::setSymbol(const QwtColumnSymbol &symbol)
{
    delete d_data->symbol;
    d_data->symbol = symbol.clone();
}

const QwtColumnSymbol &QwtPlotHistogram::symbol() const
{
    return *d_data->symbol;
}

void QwtPlotHistogram::setBaseline(double reference)
{
    if ( d_data->reference != reference )
    {
        d_data->reference = reference;
        itemChanged();
    }
}

double QwtPlotHistogram::baseline() const
{
    return d_data->reference;
}

QwtDoubleRect QwtPlotHistogram::boundingRect() const
{
    QwtDoubleRect rect = d_series->boundingRect();
    if ( !rect.isValid() ) 
        return rect;

    if ( orientation() == Qt::Horizontal )
    {
        rect = QwtDoubleRect( rect.y(), rect.x(), 
            rect.height(), rect.width() );

        if ( rect.left() > d_data->reference ) 
            rect.setLeft( d_data->reference );
        else if ( rect.right() < d_data->reference ) 
            rect.setRight( d_data->reference );
    } 
    else 
    {
        if ( rect.bottom() < d_data->reference ) 
            rect.setBottom( d_data->reference );
        else if ( rect.top() > d_data->reference ) 
            rect.setTop( d_data->reference );
    }

    return rect;
}


int QwtPlotHistogram::rtti() const
{
    return QwtPlotItem::Rtti_PlotHistogram;
}

void QwtPlotHistogram::setData(
    const QwtArray<QwtIntervalSample> &data)
{
    QwtPlotSeriesItem<QwtIntervalSample>::setData(
        QwtIntervalSeriesData(data));
}

void QwtPlotHistogram::setData(
    const QwtSeriesData<QwtIntervalSample> &data)
{
    QwtPlotSeriesItem<QwtIntervalSample>::setData(data);
}

void QwtPlotHistogram::drawSeries(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRect &, int from, int to) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = dataSize() - 1;

    switch (d_data->curveStyle)
    {
        case Outline:
            drawOutline(painter, xMap, yMap, from, to);
            break;
        case Lines:
            drawLines(painter, xMap, yMap, from, to);
            break;
        case Columns:
            drawColumns(painter, xMap, yMap, from, to);
            break;
        case NoCurve:
        default:
            break;
    }
}

void QwtPlotHistogram::drawOutline(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
    const int v0 = (orientation() == Qt::Horizontal) ?
        xMap.transform(baseline()) : yMap.transform(baseline());

    QwtIntervalSample previous;

#if QT_VERSION < 0x040000
    QValueList<QPoint> points;
#else
    QwtPolygon points;
#endif
    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = d_series->sample(i);

        if ( !sample.interval.isValid() )
        {
            flushPolygon(painter, v0, points);
            previous = sample;
            continue;
        }

        if ( previous.interval.isValid() && 
            previous.interval.maxValue() != sample.interval.minValue() )
        {
            flushPolygon(painter, v0, points);
        }

        if ( orientation() == Qt::Vertical )
        {
            const int x1 = xMap.transform( sample.interval.minValue());
            const int x2 = xMap.transform( sample.interval.maxValue());
            const int y = yMap.transform(sample.value);

            if ( points.size() == 0 )
                points += QPoint(x1, v0);

            points += QPoint(x1, y);
            points += QPoint(x2, y);
        }
        else
        {
            const int y1 = yMap.transform( sample.interval.minValue());
            const int y2 = yMap.transform( sample.interval.maxValue());
            const int x = xMap.transform(sample.value);

            if ( points.size() == 0 )
                points += QPoint(v0, y1);

            points += QPoint(x, y1);
            points += QPoint(x, y2);
        }
        previous = sample;
    }

    flushPolygon(painter, v0, points);
}

void QwtPlotHistogram::drawColumns(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
    painter->setPen(d_data->pen);
    painter->setBrush(d_data->brush);

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = d_series->sample(i);
        if ( !sample.interval.isNull() )
        { 
            QwtColumnSymbol::Direction direction;
            const QRect rect = columnRect(sample, xMap, yMap, direction);
            if ( !rect.isNull() )
                drawColumn(painter, rect, direction, sample);
        }
    }
}

void QwtPlotHistogram::drawLines(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
    painter->setPen(d_data->pen);
    painter->setBrush(Qt::NoBrush);

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = d_series->sample(i);
        if ( !sample.interval.isNull() )
        { 
            QwtColumnSymbol::Direction direction;
            const QRect rect = columnRect(sample, xMap, yMap, direction);
            if ( !rect.isNull() )
            {
                switch(direction)
                {
                    case QwtColumnSymbol::LeftToRight:
                    {
                        QwtPainter::drawLine(painter, 
                            rect.topRight(), rect.bottomRight());
                        break;
                    }
                    case QwtColumnSymbol::RightToLeft:
                    {
                        QwtPainter::drawLine(painter, 
                            rect.topLeft(), rect.bottomLeft());
                        break;
                    }
                    case QwtColumnSymbol::TopToBottom:
                    {
                        QwtPainter::drawLine(painter, 
                            rect.bottomRight(), rect.bottomLeft());
                        break;
                    }
                    case QwtColumnSymbol::BottomToTop:
                    {
                        QwtPainter::drawLine(painter, 
                            rect.topRight(), rect.topLeft());
                        break;
                    }
                }
            }
        }
    }
}

void QwtPlotHistogram::updateLegend(QwtLegend *) const
{
#if 0
#ifdef __GNUC__
#warning TODO
#endif
#endif
}

#if QT_VERSION < 0x040000
void QwtPlotHistogram::flushPolygon(QPainter *painter, 
    int baseLine, QValueList<QPoint> &points ) const
#else
void QwtPlotHistogram::flushPolygon(QPainter *painter, 
    int baseLine, QwtPolygon &points ) const
#endif
{
    if ( points.size() == 0 )
        return;

    if ( orientation() == Qt::Horizontal )
        points += QPoint(baseLine, points.last().y());
    else
        points += QPoint(points.last().x(), baseLine);

    if ( d_data->brush.style() != Qt::NoBrush )
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(d_data->brush);

        if ( orientation() == Qt::Horizontal )
        {
            points += QPoint(points.last().x(), baseLine);
            points += QPoint(points.first().x(), baseLine);
        }
        else
        {
            points += QPoint(baseLine, points.last().y());
            points += QPoint(baseLine, points.first().y());
        }
#if QT_VERSION < 0x040000
        drawPolygon(painter, points);
        points.pop_back();
        points.pop_back();
#else
        QwtPainter::drawPolygon(painter, points);
        points.resize(points.size() - 2);
#endif
    }
    if ( d_data->pen.style() != Qt::NoPen )
    {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(d_data->pen);
#if QT_VERSION < 0x040000
        drawPolygon(painter, points);
#else
        QwtPainter::drawPolyline(painter, points);
#endif
    }
    points.clear();
}

QRect QwtPlotHistogram::columnRect(const QwtIntervalSample &sample,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    QwtColumnSymbol::Direction &direction) const
{
    const int v0 = (orientation() == Qt::Horizontal) ?
        xMap.transform(baseline()) : yMap.transform(baseline());

    const QwtDoubleInterval &iv = sample.interval;
    if ( !iv.isValid() )
    {
        direction = QwtColumnSymbol::LeftToRight; // something
        return QRect();
    }

    int minOff = 0;
    if ( iv.borderFlags() & QwtDoubleInterval::ExcludeMinimum )
        minOff = 1;

    int maxOff = 0;
    if ( iv.borderFlags() & QwtDoubleInterval::ExcludeMaximum )
        maxOff = 1;

    QRect rect;
    if ( orientation() == Qt::Horizontal )
    {
        const int x = xMap.transform(sample.value);
        const int y1 = yMap.transform( iv.minValue()) - minOff;
        const int y2 = yMap.transform( iv.maxValue()) + maxOff;

        rect.setRect(v0, y1, x - v0, y2 - y1);
        direction = x < v0 ? QwtColumnSymbol::RightToLeft :
            QwtColumnSymbol::LeftToRight;
    }
    else
    {
        const int x1 = xMap.transform( iv.minValue()) + minOff;
        const int x2 = xMap.transform( iv.maxValue()) - maxOff;
        const int y = yMap.transform(sample.value);

        rect.setRect(x1, v0, x2 - x1, y - v0);
        direction = y < v0 ? QwtColumnSymbol::BottomToTop :
            QwtColumnSymbol::TopToBottom;
    }

    return rect;
}

void QwtPlotHistogram::drawColumn(QPainter *painter, 
    const QRect &rect, QwtColumnSymbol::Direction direction,
    const QwtIntervalSample &) const
{
    if ( d_data->symbol->style() != QwtColumnSymbol::NoSymbol)
        d_data->symbol->draw(painter, direction, rect);
    else
    {
        int pw = painter->pen().width();
        if ( pw == 0 )
            pw = 1;

#if QT_VERSION < 0x040000
        QRect r = rect.normalize();
        r.setLeft(r.left() + pw / 2);
        r.setTop(r.top() + pw / 2 + 1);
        r.setRight(r.right() - pw / 2 + 2 - pw % 2);
        r.setBottom(r.bottom() - pw / 2 + 1 - pw % 2 );
#else
        QRect r = rect.normalized();
        r.setLeft(r.left() + pw / 2);
        r.setRight(r.right() + pw / 2 + 1);
        r.setTop(r.top() + pw / 2 + 1);
        r.setBottom(r.bottom() + pw / 2);
#endif
        QwtPainter::drawRect(painter, r);
    }
}

#if QT_VERSION < 0x040000
void QwtPlotHistogram::drawPolygon(
    QPainter *painter, const QValueList<QPoint>& points) const
{
    int i = 0;

    QwtPolygon polygon(points.size());
    for ( QValueList<QPoint>::const_iterator it = points.begin();
        it != points.end(); ++it )
    {
        polygon[i++] = *it;
    }
    QwtPainter::drawPolyline(painter, polygon);
}
#endif

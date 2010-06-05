/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PICKER
#define QWT_PICKER 1

#include "qwt_global.h"
#include "qwt_text.h"
#include "qwt_event_pattern.h"
#include <qobject.h>
#include <qpen.h>
#include <qfont.h>
#include <qrect.h>

class QWidget;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QwtPickerMachine;

/*!
  \brief QwtPicker provides selections on a widget

  QwtPicker filters all enter, leave, mouse and keyboard events of a widget
  and translates them into an array of selected points. 

  The way how the points are collected depends on type of state machine
  that is connected to the picker. Qwt offers a couple of predefined
  state machines for selecting:

  - Nothing\n
    QwtPickerTrackerMachine
  - Single points\n
    QwtPickerClickPointMachine, QwtPickerDragPointMachine
  - Rectangles\n
    QwtPickerClickRectMachine, QwtPickerDragRectMachine
  - Polygons\n
    QwtPickerPolygonMachine

  While these state machines cover the most common ways to collect points
  it is also possible to implement individual machines as well.

  QwtPicker translates the picked points into a selection using the 
  adjustedPoints method. adjustedPoints is intended to be reimplemented
  to fixup the selection according to application specific requirements.
  (F.e. when an application accepts rectangles of a fixed aspect ratio only.)

  Optionally QwtPicker support the process of collecting points by a 
  rubberband and tracker displaying a text for the current mouse
  position.

  \par Example 
  \verbatim #include <qwt_picker.h>
#include <qwt_picker_machine.h>

QwtPicker *picker = new QwtPicker(widget);
picker->setStateMachine(new QwtPickerDragRectMachine);
picker->setTrackerMode(QwtPicker::ActiveOnly);
picker->setRubberBand(QwtPicker::RectRubberBand); \endverbatim\n

  The state machine triggers the following commands:

  - begin()\n
    Activate/Initialize the selection.
  - append()\n
    Add a new point 
  - move() \n
    Change the position of the last point. 
  - remove()\n
    Remove the last point. 
  - end()\n
    Terminate the selection and call accept to validate the picked points.

  The picker is active (isActive()), between begin() and end().
  In active state the rubberband is displayed, and the tracker is visible
  in case of trackerMode is ActiveOnly or AlwaysOn.

  The cursor can be moved using the arrow keys. All selections can be aborted
  using the abort key. (QwtEventPattern::KeyPatternCode)

  \warning In case of QWidget::NoFocus the focus policy of the observed
           widget is set to QWidget::WheelFocus and mouse tracking
           will be manipulated while the picker is active,
           or if trackerMode() is AlwayOn.
*/

class QWT_EXPORT QwtPicker: public QObject, public QwtEventPattern
{
    Q_OBJECT

    Q_ENUMS(RubberBand)
    Q_ENUMS(DisplayMode)
    Q_ENUMS(ResizeMode)

    Q_PROPERTY(bool isEnabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)

    Q_PROPERTY(DisplayMode trackerMode READ trackerMode WRITE setTrackerMode)
    Q_PROPERTY(QPen trackerPen READ trackerPen WRITE setTrackerPen)
    Q_PROPERTY(QFont trackerFont READ trackerFont WRITE setTrackerFont)

    Q_PROPERTY(RubberBand rubberBand READ rubberBand WRITE setRubberBand)
    Q_PROPERTY(QPen rubberBandPen READ rubberBandPen WRITE setRubberBandPen)

public:
    /*! 
      Rubberband style
      - NoRubberBand\n
        No rubberband.
      - HLineRubberBand & PointSelection\n
        A horizontal line.
      - VLineRubberBand & PointSelection\n
        A vertical line.
      - CrossRubberBand & PointSelection\n
        A horizontal and a vertical line.
      - RectRubberBand & RectSelection\n
        A rectangle.
      - EllipseRubberBand & RectSelection\n
        An ellipse.
      - PolygonRubberBand &PolygonSelection\n
        A polygon.
      - UserRubberBand\n
        Values >= UserRubberBand can be used to define additional
        rubber bands.

      The default value is NoRubberBand.
      \sa setRubberBand(), rubberBand()
    */

    enum RubberBand
    {
        NoRubberBand = 0,

        // Point
        HLineRubberBand,
        VLineRubberBand,
        CrossRubberBand,

        // Rect
        RectRubberBand,
        EllipseRubberBand,

        // Polygon
        PolygonRubberBand,

        UserRubberBand = 100
    };

    /*! 
      Display mode
      - AlwaysOff\n
        Display never.
      - AlwaysOn\n
        Display always.
      - ActiveOnly\n
        Display only when the selection is active.

      \sa setTrackerMode(), trackerMode(), isActive()
    */
    enum DisplayMode
    {
        AlwaysOff,
        AlwaysOn,
        ActiveOnly
    };

    /*! 
      Controls what to do with the selected points of an active
         selection when the observed widget is resized.
      - Stretch\n
         All points are scaled according to the new size, 
      - KeepSize\n
         All points remain unchanged.

      The default value is Stretch.
      \sa QwtPicker::setResizeMode(), QwtPicker::resize()
    */

    enum ResizeMode
    {
        Stretch,
        KeepSize
    };

    explicit QwtPicker(QWidget *parent);
    explicit QwtPicker(RubberBand rubberBand,
        DisplayMode trackerMode, QWidget *);

    virtual ~QwtPicker();

    void setStateMachine(QwtPickerMachine *);
    const QwtPickerMachine *stateMachine() const;
    QwtPickerMachine *stateMachine();

    void setRubberBand(RubberBand);
    RubberBand rubberBand() const;

    void setTrackerMode(DisplayMode);
    DisplayMode trackerMode() const;

    void setResizeMode(ResizeMode);
    ResizeMode resizeMode() const;

    void setRubberBandPen(const QPen &);
    QPen rubberBandPen() const;

    void setTrackerPen(const QPen &);
    QPen trackerPen() const;

    void setTrackerFont(const QFont &);
    QFont trackerFont() const;

    bool isEnabled() const;
    bool isActive() const;

    virtual bool eventFilter(QObject *, QEvent *);

    QWidget *parentWidget();
    const QWidget *parentWidget() const;

    virtual QRect pickRect() const;

    virtual void drawRubberBand(QPainter *) const;
    virtual void drawTracker(QPainter *) const;

    virtual QwtText trackerText(const QPoint &pos) const;
    QPoint trackerPosition() const;
    QRect trackerRect(const QFont &) const;

    QPolygon selection() const;

public Q_SLOTS:
    void setEnabled(bool);

Q_SIGNALS:
    /*!
      A signal indicating, when the picker has been activated.
      Together with setEnabled() it can be used to implement
      selections with more than one picker.

      \param pa Selected points
    */
    void activated(bool);

    /*!
      A signal emitting the selected points, 
      at the end of a selection.

      \param pa Selected points
    */
    void selected(const QPolygon &pa);

    /*!
      A signal emitted when a point has been appended to the selection

      \param pos Position of the appended point.
      \sa append(). moved()
    */
    void appended(const QPoint &pos);

    /*!
      A signal emitted whenever the last appended point of the 
      selection has been moved.

      \param pos Position of the moved last point of the selection.
      \sa move(), appended()
    */
    void moved(const QPoint &pos);

    /*!
      A signal emitted whenever the last appended point of the 
      selection has been removed.

      \sa remove(), appended()
    */
    void removed(const QPoint &pos);
    /*!
      A signal emitted when the active selection has been changed.
      This might happen when the observed widget is resized.

      \param pa Changed selection
      \sa stretchSelection()
    */
    void changed(const QPolygon &pa);

protected:
    virtual QPolygon adjustedPoints(const QPolygon &) const;

    virtual void transition(const QEvent *);

    virtual void begin();
    virtual void append(const QPoint &);
    virtual void move(const QPoint &);
    virtual void remove();
    virtual bool end(bool ok = true);

    virtual bool accept(QPolygon &) const;
    virtual void reset();

    virtual void widgetMousePressEvent(QMouseEvent *);
    virtual void widgetMouseReleaseEvent(QMouseEvent *);
    virtual void widgetMouseDoubleClickEvent(QMouseEvent *); 
    virtual void widgetMouseMoveEvent(QMouseEvent *); 
    virtual void widgetWheelEvent(QWheelEvent *);
    virtual void widgetKeyPressEvent(QKeyEvent *); 
    virtual void widgetKeyReleaseEvent(QKeyEvent *); 
    virtual void widgetEnterEvent(QEvent *); 
    virtual void widgetLeaveEvent(QEvent *); 

    virtual void stretchSelection(const QSize &oldSize, 
        const QSize &newSize);

    virtual void updateDisplay();

    const QWidget *rubberBandWidget() const;
    const QWidget *trackerWidget() const;

    const QPolygon &pickedPoints() const; 

private:
    void init(QWidget *, RubberBand rubberBand, DisplayMode trackerMode);

    void setMouseTracking(bool);

    class PickerWidget;
    class PrivateData;
    PrivateData *d_data;
};
            
#endif

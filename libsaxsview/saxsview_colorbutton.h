/*
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of saxsview.
 *
 * saxsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * saxsview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with saxsview. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SAXSVIEW_COLORBUTTON_H
#define SAXSVIEW_COLORBUTTON_H

#include <QToolButton>
class QColor;

/**
 * NOTE ColorButtons require an explicit StrongFocus to be set
 *      to work with models!
 */
class ColorButton : public QToolButton {
  Q_OBJECT

  Q_PROPERTY (QColor color
              READ color
              WRITE setColor
              NOTIFY colorChanged);

public:
  ColorButton(QWidget *parent = 0L);
  ~ColorButton();

  QColor color() const;

public slots:
  void getColor();
  void setColor(const QColor&);
  void setColor(const QString&);

signals:
  void colorChanged(const QColor&);

protected:
  void resizeEvent(QResizeEvent*);
  void updateIcon();

private:
  class ColorButtonPrivate;
  ColorButtonPrivate *p;
};

#endif // !SAXSVIEW_COLORBUTTON_H


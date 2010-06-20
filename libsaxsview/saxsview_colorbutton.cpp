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

#include "saxsview_colorbutton.h"

#include <QtGui>

static QIcon colorIcon(const QColor& color, const QSize& size) {
  QPixmap pixmap(size);

  QPainter painter;
  painter.begin(&pixmap);
  painter.setBrush(QBrush(color));
  painter.setPen(QPen(Qt::black));
  painter.drawRect(pixmap.rect());
  painter.end();

  return QIcon(pixmap);
}



class ColorButton::ColorButtonPrivate {
public:
  ColorButtonPrivate(ColorButton *btn)
   : color(Qt::white),
     menu(new QMenu),
     actionMapper(new QSignalMapper) {

    QSize size(16, 16);
    foreach (QString name, QColor::colorNames()) {
      QAction *action = menu->addAction(colorIcon(QColor(name), size), name);
      action->setIconVisibleInMenu(true);
      connect(action, SIGNAL(triggered()),
              actionMapper, SLOT(map()));
      actionMapper->setMapping(action, name);
    }

    connect(actionMapper, SIGNAL(mapped(const QString&)),
            btn, SLOT(setColor(const QString&)));
  }

  ~ColorButtonPrivate() {
    delete actionMapper;
    delete menu;
  }

  QColor color;
  QMenu *menu;
  QSignalMapper *actionMapper;
};


ColorButton::ColorButton(QWidget *parent)
 : QToolButton(parent), p(new ColorButtonPrivate(this)) {

  setMenu(p->menu);

  connect(this, SIGNAL(clicked()), SLOT(getColor()));
}

ColorButton::~ColorButton() {
  delete p;
}

QColor ColorButton::color() const {
  return p->color;
}

void ColorButton::getColor() {
  QColor color = QColorDialog::getColor(p->color, this);
  if (color.isValid())
    setColor(color);
}

void ColorButton::setColor(const QColor& color) {
  if (p->color != color) {
    p->color = color;
    updateIcon();
    emit colorChanged(p->color);
  }
}

void ColorButton::setColor(const QString& name) {
  setColor(QColor(name));
}

void ColorButton::resizeEvent(QResizeEvent *) {
  updateIcon();
}

void ColorButton::updateIcon() {
  setIcon(colorIcon(p->color, iconSize()));
}

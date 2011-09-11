/*
 * Copyright (C) 2011 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of saxsview.
 *
 * saxsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * saxsview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with saxsview. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SVIMAGEMPROPERTY_H
#define SVIMAGEMPROPERTY_H

#include <QObject>
class QString;
class QStringList;
class QVariant;

#include <QtVariantProperty>
class QtProperty;
class QtAbstractPropertyBrowser;
class QtVariantPropertyManager;


class ReadWriteProperty : public QObject {
  Q_OBJECT

public:
  ReadWriteProperty(QtAbstractPropertyBrowser *browser,
                    const QString& label,
                    const QVariant& value);

  ReadWriteProperty(QtAbstractPropertyBrowser *browser,
                    const QString& label,
                    const QStringList& value);

  ReadWriteProperty(QtAbstractPropertyBrowser *browser,
                    const QString& label,
                    int value, int min, int max, int step);

  QtVariantProperty* property() const;

  void setValue(const QVariant& value);

private slots:
  void valueChanged(QtProperty*, const QVariant& t);

signals:
  void valueChanged(const QVariant&);

private:
  void init(QtAbstractPropertyBrowser *browser);

  QtVariantProperty *prop;
  QtVariantPropertyManager *manager;
};


class ReadOnlyProperty : public QObject {
public:
  ReadOnlyProperty(QtAbstractPropertyBrowser *browser,
                   const QString& label,
                   const QVariant& value);

  QtVariantProperty* property() const;

  void setValue(const QVariant& value);

private:
  QtVariantProperty *prop;
  QtVariantPropertyManager *manager;
};


class PropertyGroup : public QObject {
public:
  PropertyGroup(QtAbstractPropertyBrowser *browser,
                const QString& label);

  template<class U>
  void addSubProperty(U *p) {
    group->addSubProperty(p->property());
  }

private:
  QtVariantProperty *group;
  QtVariantPropertyManager *manager;
};

#endif // !SVIMAGEMPROPERTY_H

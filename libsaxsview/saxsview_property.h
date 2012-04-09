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

#ifndef SAXSVIEW_PROPERTY_H
#define SAXSVIEW_PROPERTY_H

#include <QObject>
#include <QString>
#include <QMap>

class QtProperty;
class QtVariantProperty;
class QtVariantPropertyManager;
class QtAbstractPropertyBrowser;


class SaxsviewProperty : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(SaxsviewProperty)

public:
  SaxsviewProperty(const QString& label,
                   QtAbstractPropertyBrowser *browser,
                   SaxsviewProperty *parent = 0L);

  SaxsviewProperty(const QString& label, const QString& property,
                   QtAbstractPropertyBrowser *browser,
                   SaxsviewProperty *parent = 0L);
  ~SaxsviewProperty();

  void setValue(QObject *o);

  void setMinimum(const QVariant&);
  void setMaximum(const QVariant&);
  void setRange(const QVariant&, const QVariant&);
  void setEnumNames(const QVariant&);

public slots:
  void valueChanged(QtProperty*, const QVariant& value);

private:
  QtVariantProperty *mProperty;
  QtVariantPropertyManager *mManager;
  QtAbstractPropertyBrowser *mBrowser;
 
  QString mPropertyLabel;
  QString mPropertyName;
  QObject *mObj;
  QMap<QString,QVariant> mAttributes;
  SaxsviewProperty *mParentProperty;
};

#endif // !SAXSVIEW_PROPERTY_H

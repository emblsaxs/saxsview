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

#include "svimageproperty.h"

#include <QtGroupPropertyManager>
#include <QtTreePropertyBrowser>
#include <QtVariantProperty>

#include <QtGui>


ReadWriteProperty::ReadWriteProperty(QtAbstractPropertyBrowser *browser,
                                     const QString& label,
                                     const QVariant& value) {
  init(browser);
  prop = manager->addProperty(value.type(), label);
  setValue(value);
}

ReadWriteProperty::ReadWriteProperty(QtAbstractPropertyBrowser *browser,
                                     const QString& label,
                                     const QStringList& value) {
  init(browser);

  prop = manager->addProperty(manager->enumTypeId(), label);
  prop->setAttribute("enumNames", value);
}

ReadWriteProperty::ReadWriteProperty(QtAbstractPropertyBrowser *browser,
                                     const QString& label,
                                     int value, int min, int max, int step) {
  init(browser);

  prop = manager->addProperty(QVariant(value).type(), label);
  prop->setAttribute("minimum", min);
  prop->setAttribute("maximum", max);
  prop->setAttribute("singleStep", step);

  setValue(value);
}

QtVariantProperty* ReadWriteProperty::property() const {
  return prop;
}

void ReadWriteProperty::setValue(const QVariant& value) {
  manager->setValue(prop, value);
}

void ReadWriteProperty::valueChanged(QtProperty*, const QVariant& t) {
  emit valueChanged(t);
}

void ReadWriteProperty::init(QtAbstractPropertyBrowser *browser) {
  manager = new QtVariantPropertyManager(browser);
  browser->setFactoryForManager(manager, new QtVariantEditorFactory(this));

  connect(manager, SIGNAL(valueChanged(QtProperty*, const QVariant&)),
          this, SLOT(valueChanged(QtProperty*, const QVariant&)));
}


ReadOnlyProperty::ReadOnlyProperty(QtAbstractPropertyBrowser *browser,
                                   const QString& label,
                                   const QVariant& value) {
  manager = new QtVariantPropertyManager(browser);
  prop = manager->addProperty(value.type(), label);
  setValue(value);
}

QtVariantProperty* ReadOnlyProperty::property() const {
  return prop;
}

void ReadOnlyProperty::setValue(const QVariant& value) {
  manager->setValue(prop, value);
}


PropertyGroup::PropertyGroup(QtAbstractPropertyBrowser *browser,
                             const QString& label) {
  manager = new QtVariantPropertyManager(browser);
  group = manager->addProperty(manager->groupTypeId(), label);
  browser->addProperty(group);
}

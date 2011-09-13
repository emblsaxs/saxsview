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

#include "saxsview.h"
#include "saxsview_property.h"

#include <QtGroupPropertyManager>
#include <QtTreePropertyBrowser>
#include <QtVariantProperty>

#include <QtGui>

SaxsviewProperty::SaxsviewProperty(const QString& propertyLabel,
                                   QtAbstractPropertyBrowser *browser,
                                   SaxsviewProperty *parent)
 : QObject(browser),
   mProperty(0L),
   mManager(new QtVariantPropertyManager(browser)),
   mBrowser(browser),
   mPropertyLabel(propertyLabel),
   mPropertyName(QString()),
   mObj(0L),
   mParentProperty(parent) {

  mProperty = mManager->addProperty(mManager->groupTypeId(), mPropertyLabel);
  if (parent)
    mParentProperty->mProperty->addSubProperty(mProperty);
  else
    mBrowser->addProperty(mProperty);
}

SaxsviewProperty::SaxsviewProperty(const QString& propertyLabel,
                                   const QString& propertyName,
                                   QtAbstractPropertyBrowser *browser,
                                   SaxsviewProperty *parent)
 : QObject(browser),
   mProperty(0L),
   mManager(new QtVariantPropertyManager(browser)),
   mBrowser(browser),
   mPropertyLabel(propertyLabel),
   mPropertyName(propertyName),
   mObj(0L),
   mParentProperty(parent) {
}

void SaxsviewProperty::setValue(QObject *obj) {
  if (!mProperty) {
    //
    // Find the meta-property information; the passed object must
    // provide the Q_PROPERTY name passed to the constructor.
    //
    int indexOfProperty = obj->metaObject()->indexOfProperty(qPrintable(mPropertyName));
    QMetaProperty metaProperty = obj->metaObject()->property(indexOfProperty);

    //
    // Create an editor factory if and only if the property is writeable.
    //
    if (metaProperty.isWritable()) {
      mBrowser->setFactoryForManager(mManager, new QtVariantEditorFactory(this));
      connect(mManager, SIGNAL(valueChanged(QtProperty*, const QVariant&)),
              this, SLOT(valueChanged(QtProperty*, const QVariant&)));
    }

    if (!mManager->isPropertyTypeSupported(metaProperty.type()))
      qFatal("internal error: property '%s', property type not supported: '%s'",
             metaProperty.name(), metaProperty.typeName());

    //
    // Check if this is an enum and handle it specially if yes.
    //
    if (metaProperty.isEnumType()) {
      QStringList enumNames;
      QMetaEnum metaEnum = metaProperty.enumerator();

      for (int i = 0; i < metaEnum.keyCount(); ++i)
        enumNames << metaEnum.key(i);

      //
      // Avoid updates triggered during setup by blocking the changed
      // signal emitted when setting the attribute.
      //
      mManager->blockSignals(true);
      mProperty = mManager->addProperty(mManager->enumTypeId(), mPropertyLabel);
      mProperty->setAttribute("enumNames", enumNames);
      mManager->blockSignals(false);

    } else
      mProperty = mManager->addProperty(metaProperty.type(), mPropertyLabel);

    if (mParentProperty)
      mParentProperty->mProperty->addSubProperty(mProperty);
    else
      mBrowser->addProperty(mProperty);

    //
    // Make sure that e.g. font properties are not in an expanded state.
    // The property list would become unusable long as e.g. each font adds
    // another seven subitems.
    //
    // NOTE: Works only with QtTreePropertyBrowser - downcast?
    //
//     foreach (QtBrowserItem *item, mBrowser->items(mProperty))
//       mBrowser->setExpanded(item, false);
  }

  if (obj) {
    mObj = obj;
    mProperty->setValue(obj->property(qPrintable(mPropertyName)));
  }
}

void SaxsviewProperty::valueChanged(QtProperty*, const QVariant& value) {
  mObj->setProperty(qPrintable(mPropertyName), value);
}

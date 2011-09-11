/*
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "saxsview_image.h"

namespace Saxsview {

class Image::ImagePrivate {
public:
  QString fileName;
};


Image::Image(QObject *parent)
  : QObject(parent), p(new ImagePrivate) {
}

Image::~Image() {
  delete p;
}

QString Image::fileName() const {
  return p->fileName;
}

void Image::setFileName(const QString& fileName) {
  p->fileName = fileName;
}

} // end of namespace Saxsview

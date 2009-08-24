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

#ifndef SAXSVIEW_PLOTIMAGE_H
#define SAXSVIEW_PLOTIMAGE_H

#include <QObject>

#include <qwt_plot_spectrogram.h>


namespace Saxsview {

class Image : public QObject,
              public QwtPlotSpectrogram {
  Q_OBJECT

public:
  Image(QObject *parent = 0L);
  ~Image();

  QString fileName() const;

public slots:
  void setFileName(const QString&);

private:
  class ImagePrivate;
  ImagePrivate *p;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_PLOTIMAGE_H

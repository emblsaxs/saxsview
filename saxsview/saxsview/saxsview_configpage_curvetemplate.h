/*
 * Copyright (C) 2010 Daniel Franke <dfranke@users.sourceforge.net>
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

#ifndef SAXSVIEW_CONFIGPAGE_CURVETEMPLATE_H
#define SAXSVIEW_CONFIGPAGE_CURVETEMPLATE_H

#include "saxsview_configpage.h"
#include "ui_saxsview_configpage_curvetemplate.h"

namespace Saxsview {

class CurveTemplateConfigPage : public AbstractConfigPage ,
                                private Ui::CurveTemplateConfigPage {
Q_OBJECT

public:
  CurveTemplateConfigPage(QWidget *parent = 0L);
  ~CurveTemplateConfigPage();
  void apply();
  void reset();

private slots:
  void addTemplate();
  void removeTemplate();
  void setCurrentTemplateIndex(int);
  void setCurrentFileTypeIndex(int);

private:
  void setEditorEnabled(bool);

  class CurveTemplateConfigPagePrivate;
  CurveTemplateConfigPagePrivate *p;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_CONFIGPAGE_CURVETEMPLATE_H

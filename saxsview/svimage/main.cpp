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

#include <QtWidgets>

#include "svimagemainwindow.h"
#include "saxsview.h"

int main(int argc, char **argv) {
  Q_INIT_RESOURCE(saxsview);
//   qRegisterMetaType<QwtText>("QwtText");    // is this needed?

  QApplication app(argc, argv);
  app.setOrganizationName("sourceforge");
  app.setOrganizationDomain("sourceforge.net");
  app.setApplicationName("svimage");

  SVImageMainWindow w;
  w.show();

  QStringList args = app.arguments();
  for (int i = 1; i < args.size(); ++i)
    w.load(args[i]);

  return app.exec();
}

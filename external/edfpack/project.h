/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 2001-2010 European Synchrotron Radiation Facility
 *                           Grenoble, France
 *
 *   Principal authors: P. Boesecke (boesecke@esrf.fr)
 *                      R. Wilcke (wilcke@esrf.fr)
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   and the GNU Lesser General Public License  along with this program.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

/*+++***********************************************************************
NAME

    project.h

SYNOPSIS

    #include "project.h"

DESCRIPTION
    Header of the module "project.c"
***********************************************************************---*/

/****************************************************************************
*  Include                                                                  *
****************************************************************************/

# include <stdio.h>
# include <stdlib.h>
# include "ipol.h"
# include "reference.h"

/***************************************************************************
* Functions                                                                *
***************************************************************************/

#ifndef _PROJECT_
# define PROJECT_VERSION "project : V1.01 Peter Boesecke 2009-11-10"

  extern void           
   project_1               ( float *line, float *varline, int dim,
                             int imin, int imax, float initvalue, float factor,
                             float *data, float *vardat, int dim_1, int dim_2,
                             float f1_1, float f3_1, float Df_1,
                             float f1_2, float f3_2,
                             float dummy, float ddummy, int ave ),
   project_2               ( float *line, float *varline, int dim,
                             int imin, int imax, float initvalue, float factor,
                             float *data, float *vardat, int dim_1, int dim_2,
                             float f1_1, float f3_1,
                             float f1_2, float f3_2, float Df_2,
                             float dummy, float ddummy, int ave );

# define _PROJECT_
#endif /* _PROJECT_ */

/****************************************************************************
*                                                                           *
****************************************************************************/

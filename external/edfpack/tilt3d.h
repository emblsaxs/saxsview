/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *                      http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 2005-2010 European Synchrotron Radiation Facility
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

    tilt3d.h

SYNOPSIS

    #include "tilt3d.h"

DESCRIPTION
    Header of the module "tilt3d.c"
    Calculate a 3d rotation matrix for tilt1, tilt2, tilt3 or its inverse.
    Calculate tilt1, tilt2 and tilt3 from a 3d rotation matrix.

***********************************************************************---*/
#ifndef _tilt3d_
# define _tilt3d_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/

#ifndef PRIVATE
# define PRIVATE       static /* used to declare variables of private type */
# define PUBLIC                /* used to declare variables of public type */
#endif

/****************************************************************************
*  Include                                                                  *
****************************************************************************/

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <ctype.h>
# include <limits.h>
# include <errno.h>
# include <stdio.h>
# include <fcntl.h>
# include <math.h>
# include <float.h>

/****************************************************************************
* Enums and Structures                                                      *
****************************************************************************/

/****************************************************************************
* Functions                                                                 *
****************************************************************************/

PUBLIC extern const char *tilt3d_version ( void );

PUBLIC extern double tilt3d_determinante ( double A[3][3] );

PUBLIC extern int tilt3d_angles(double TILT[3], double R[3][3]);

PUBLIC extern int tilt3d_matrix(double TILT[3], double R[3][3]);

PUBLIC extern int tilt3d_inverse_matrix(double TILT[3], double R[3][3]);

/***************************************************************************/

#endif

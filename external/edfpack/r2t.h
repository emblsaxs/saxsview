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

    r2t.h

SYNOPSIS

    #include "r2t.h"

DESCRIPTION
    Header of the module "r2t.c"

    Routines to convert beam distance and beam center to
    sample distance and center and vice versa.

***********************************************************************---*/
#ifndef _r2t_
# define _r2t_

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

/***************************************************************************
* Definitions                                                              *
***************************************************************************/

/****************************************************************************
* Enums and Structures                                                      *
****************************************************************************/

/****************************************************************************
* Functions                                                                 *
****************************************************************************/

const char *r2t_version ( void );

int r2t_bcen1( double *bcen1, 
               double pix1, double cen1, double dis, double R[3][3] );

int r2t_bcen2( double *bcen2, 
               double pix2, double cen2, double dis, double R[3][3] );

int r2t_bdis ( double *bdis, 
               double dis, double R[3][3] );

int r2t_cen1 ( double *cen1,
               double pix1, double bcen1, double bdis, double R[3][3] );

int r2t_cen2 ( double *cen2,
               double pix2, double bcen2, double bdis, double R[3][3] );

int r2t_dis  ( double *dis,
               double bdis, double R[3][3] );

/***************************************************************************/

#endif

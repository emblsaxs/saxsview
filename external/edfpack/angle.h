/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
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

    angle.h

SYNOPSIS

    #include "angle.h"

DESCRIPTION

    Header of the module "angle.c"

***********************************************************************---*/
#ifndef _ANGLE_
# define ANGLE_VERSION      "angle : V1.4 Peter Boesecke 2009-10-02"

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/

#ifndef PRIVATE
# define PRIVATE       static /* used to declare variables of private type */
# define PUBLIC                /* used to declare variables of public type */
#endif

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

# include "reference.h"
# include "numio.h"
# include "ipol.h"

/*===========================================================================*/

/******************************************************************************
* Functions                                                                   *
******************************************************************************/

PUBLIC extern float
   angle_limits( int mode, int I1Dim_1, int I1Dim_2,
                 float Off_11, float Ps_11, float Off_21, float Ps_21,
                 float Wcenter_1, float Wcenter_2 );

PUBLIC extern void 
     angle_sum ( float * I0Data, float * E0Data, 
                 int I0Dim_1, int I0Dim_2, 
                 float Off_10, float Ps_10, float Off_20, float Ps_20, 
                 float I0Dummy, float I0DDummy,
                 float * I1Data, float * E1Data,
                 int I1Dim_1, int I1Dim_2,
                 float Off_11, float Ps_11, float Off_21, float Ps_21, 
                 float I1Dummy, float I1DDummy,
                 float Wcenter_1, float Wcenter_2,
                 int vsum, int ave, int testbit, int * pstatus );

# define _ANGLE_
#endif

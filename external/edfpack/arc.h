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

    arc.h

SYNOPSIS

    #include "arc.h"

DESCRIPTION

    Header of the module "arc.c"

***********************************************************************---*/
#ifndef _ARC_
# define _ARC_

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
# include "waxs.h"

/*===========================================================================*/

/***************************************************************************
* Functions                                                                *
***************************************************************************/
PUBLIC extern int
   ang_range ( int rsys, int proin, int proout,
               long  dim_1, long dim_2,
               float off_1, float pix_1, float cen_1,
               float off_2, float pix_2, float cen_2,
               float dis, float wvl,
               float rot1, float rot2, float rot3,
               WaxsCoord *Wmin, WaxsCoord *Wmax, int * pstatus);

PUBLIC extern void
   ang_limits( WaxsCoord Wmin, WaxsCoord Wmax, 
               WaxsCoord *Amin, WaxsCoord *Amax,
               int * pstatus);

PUBLIC extern void 
   arc_sum ( int rsys,
             float * I0Data,   float * E0Data,
             int   I0Dim_1,    int   I0Dim_2,
             float I0Offset_1, float I0PSize_1, float I0Center_1,
             float I0Offset_2, float I0PSize_2, float I0Center_2,
             float I0SampleDistance, float I0WaveLength,
             int I0Pro,
             float I0Dummy,    float I0DDummy,
             float * I1Data,   float * E1Data,
             int   I1Dim_1,    int   I1Dim_2,
             float I1Offset_1, float I1PSize_1, float I1Center_1,
             float I1Offset_2, float I1PSize_2, float I1Center_2,
             float I1SampleDistance, float I1WaveLength,
             float I1DetRot1,  float I1DetRot2, float I1DetRot3,
             int I1Pro,
             float I1Dummy,    float I1DDummy,
             float AngleMin,   float AngleMax,
             float Shift_1,    float Shift_2,
             int vsum, int ave, int testbit, int * pstatus ),
   ang_sum ( int rsys,
             float * I0Data,   float * E0Data,
             int   I0Dim_1,    int   I0Dim_2,
             float I0Offset_1, float I0PSize_1, float I0Center_1,
             float I0Offset_2, float I0PSize_2, float I0Center_2,
             float I0SampleDistance, float I0WaveLength,
             int I0Pro,
             float I0Dummy,    float I0DDummy,
             float * I1Data,   float * E1Data,
             int   I1Dim_1,    int   I1Dim_2,
             float I1Offset_1, float I1PSize_1, float I1Center_1,
             float I1Offset_2, float I1PSize_2, float I1Center_2,
             float I1SampleDistance, float I1WaveLength,
             float I1DetRot1,  float I1DetRot2, float I1DetRot3,
             int I1Pro,
             float I1Dummy,    float I1DDummy,
             float AngleMin,   float AngleMax,
             float Shift_1,    float Shift_2,
             int vsum, int ave, int testbit, int * pstatus );
#endif

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

    polarization.h

SYNOPSIS

    #include "polarization.h"

DESCRIPTION

    Header of the module "polarization.c"

***********************************************************************---*/
#ifndef _POLARIZATION_
# define _POLARIZATION_

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

# include "waxs.h"
# include "raster.h"

/******************************************************************************
* Type Defs                                                                   *
******************************************************************************/

typedef struct polarization_params {
  int    Init;
  long   Ori;          // orientation (1..16)
  double P;            // polarization (0<=P<=1) 
  double PChi;         // Poincaré angle chi (ellipticity) (-pi/4<=PChi<=pi/4)
  double PPsi;         // Poincaré angle psi (polarization direction)
  double Factor;       // Multiplication factor f*f = n/A * re*re
  int    Invert;       // Invert polarization factor
  // calculated values
  double halfOnePlusCos2ChiCos2Psi;   // 0.5*(1+cos(2*Chi)*cos(2*Psi)
  double halfOneMinusCos2ChiCos2Psi;  // 0.5*(1-cos(2*Chi)*cos(2*Psi)
  double Cos2ChiSin2Psi;              // Cos2ChiSin2Psi
  WParams wparams;     // Waxs parameters

} PParams;

/*===========================================================================*/

/***************************************************************************
* Functions                                                                *
***************************************************************************/
PUBLIC extern int 
       polarization_Init ( PParams * pParams, long ori,
                           double k, double rot1, double rot2, double rot3,
                           double P, double PChi, double PPsi, double Factor,
                           int Invert );

PUBLIC extern void
       polarization_PrintParams ( FILE * out, PParams Params );

PUBLIC extern double 
       polarization_factor ( PParams * pParams,
                             WaxsCoord wc, int projection);

#endif

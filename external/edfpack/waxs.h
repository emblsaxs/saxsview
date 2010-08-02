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

    waxs.h

SYNOPSIS

    #include "waxs.h"

DESCRIPTION

    Header of the module "waxs.c"

***********************************************************************---*/
#ifndef _WAXS_
# define _WAXS_

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

/******************************************************************************
* Public Type Defs                                                            *
******************************************************************************/

typedef struct waxs_vector {
  int    status;
  double s_1; // saxs vector 1
  double s_2; // saxs vector 2
  double s_3; // saxs vector 3
} WaxsVector;

typedef struct waxs_coordinate {
  int    status;
  double s_1; // saxs coordinate 1
  double s_2; // saxs coordinate 2
} WaxsCoord;

typedef struct waxs_dir {
  int    status;
  double sinTwoTheta;
  double cosTwoTheta;
  double sinAlpha;
  double cosAlpha;
} WaxsDir;

typedef struct waxs_params {
  int    Init;
  double Rot[3][3];    // rotation matrix
  double InvRot[3][3]; // inverse rotation matrix
  double k;            // absolute value of k-vector
  double halfdk2;      // 0.5/k^2
  int    SymType;      // symmetry type: 0: isotropic, 
                       //                1: cylindrical symmetry around x_1
                       //                2: cylindrical symmetry around x_2
  double SymRot[3][3]; // symmetry rotation matrix
  double InvSymRot[3][3]; // inverse symmetry rotation matrix

} WParams;

/*===========================================================================*/

/***************************************************************************
* Functions                                                                *
***************************************************************************/
PUBLIC extern int 
       waxs_Init     ( WParams * pParams,
                       double k, double rot_1, double rot_2, double rot_3 ),
       waxs_SymInit  ( WParams * pParams, int symtype,
                       double symrot_1, double symrot_2, double symrot_3 ),
       waxs_not_init ( WParams *pParams ),
       waxs_get_transform( int proin, int proout );

PUBLIC extern void
       waxs_PrintParams ( FILE * out, WParams Params ),
       waxs_PrintCoord  ( FILE * out, WaxsCoord sp ),
       waxs_PrintVector ( FILE * out, WaxsVector svec ),
       waxs_PrintDir    ( FILE * out, WaxsDir Beam );

PUBLIC extern int // calculate projection range
       waxs_Range( WParams * pParams, 
                   int proin, int proout,
                   long  dim_1, long dim_2,
                   float off_1, float pix_1, float cen_1,
                   float off_2, float pix_2, float cen_2,
                   float dis, float wvl,
                   WaxsCoord *Wmin, WaxsCoord *Wmax, int * pstatus);

PUBLIC extern WaxsCoord 
       waxs_Saxs ( WParams * pParams, 
                   WaxsCoord sp ),  // SAXS coordinate from SAXS projection
       waxs_Waxs ( WParams * pParams, 
                   WaxsCoord s ),   // SAXS projection from SAXS coordinate
       waxs_Uni2Iso ( WParams * pParams, 
                      WaxsCoord ssym ),  // uniaxial WAXS to isotropic WAXS 
       waxs_Iso2Uni ( WParams * pParams, 
                      WaxsCoord sp );    // isotropic WAXS to uniaxial WAXS 

PUBLIC extern WaxsCoord 
       waxs_Transform( WParams * pParams, int transform, WaxsCoord W );

PUBLIC extern WaxsVector
       waxs_Saxs2Vector ( WParams * pParams,
                          WaxsCoord s ); // Vector from SAXS coordinate

PUBLIC extern WaxsDir
       waxs_ssym2kdir ( WParams * pParams, WaxsCoord ssym ),
       waxs_sp2kdir   ( WParams * pParams, WaxsCoord sp ), 
       waxs_s2kdir    ( WParams * pParams, WaxsCoord s  );

PUBLIC extern WaxsCoord
       waxs_kdir2ssym ( WParams * pParams, WaxsDir kdir ),
       waxs_kdir2sp   ( WParams * pParams, WaxsDir kdir ),
       waxs_kdir2s    ( WParams * pParams, WaxsDir kdir );

PUBLIC extern WaxsVector
       waxs_kdir2svec ( WParams * pParams, WaxsDir kdir );

#endif

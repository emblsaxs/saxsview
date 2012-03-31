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

    sx.h

SYNOPSIS

    #include "sx.h"

DESCRIPTION

    Header of the module "sx.c"

***********************************************************************---*/
#ifndef _SX_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/

#ifndef PRIVATE
# define PRIVATE       static /* used to declare variables of private type */
# define PUBLIC                /* used to declare variables of public type */
#endif

# include <stdio.h>
# include <stdlib.h>
# include <limits.h>
# include <errno.h>
# include <stdio.h>
# include <fcntl.h>
# include <math.h>
# include <float.h>

# include "strlib.h"
# include "numio.h"
# include "raster.h"
# include "rot3d.h"
# include "tilt3d.h"
# include "r2t.h"
# include "reference.h"

/***************************************************************************
* MACROS                                                                   *
***************************************************************************/

/******************************************************************************
* Public Type Defs                                                            *
******************************************************************************/

typedef struct sx_short  { short  V; int    I; } SXS;

typedef struct sx_int    { int    V; int    I; } SXI;

typedef struct sx_long   { long   V; int    I; } SXL;

typedef struct sx_float  { float V;  int    I; } SXF;

typedef struct sx_double { double V; int    I; } SXD;

typedef struct sx_params {
  SXI pro;                         // projection type (IO_ProSaxs, IO_ProWaxs)
  SXL ori;                         // orientation number (1-16)
  SXI axis1; SXI axis2;            // axis types (IO_AxisTypeDistance,
                                   // IO_AxisTypeAngle, IO_AxisTypeNumerator)
  SXL dim1;  SXL dim2;             // dimensions of 2d array 
  SXD off1;  SXD off2;             // offsets of array coordinates
  SXD bis1;  SXD bis2;             // binning sizes
  SXD ras1;  SXD ras2;             // raster region of 2d array 
  SXD pix1;  SXD pix2;             // pixel sizes [m]
  SXD cen1;  SXD cen2;             // PONI (point of normal incidence)
  SXD dis;                         // distance sample-PONI [m]
  SXD rot1;  SXD rot2; SXD rot3;   // detector rotations [rad]
  SXD wvl;                         // wavelength [m]
  SXD bcen1; SXD bcen2;            // beam center (alt. cen1, cen2)
  SXD bdis;                        // distance sample-bcen [m] (alt. dis) 
  SXD tilt1; SXD tilt2; SXD tilt3; // detector tilts [rad]
} SXParams;

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern SXParams
  *sx_new    ( SXParams * params ), 
  *sx_free   ( SXParams * params ),
  *sx_init   ( SXParams * params ), 
  *sx_cp_params ( SXParams *params_out, const SXParams *params_in ),
  *sx_tf_params ( SXParams * params_out, const SXParams * params_in, 
                  long ori, int rot, int *perrval ),
  *sx_rd_params ( SXParams * params_out, char *argv[], int * perrval );

PUBLIC extern int 
   sx_pr_params( FILE * out, const SXParams * params ),
   sx_pr_params_line( FILE *out, const SXParams *params, int head );

PUBLIC extern int 
   sx_tf_img ( SXParams *params_out,  
              void *data_out, void *variance_out, size_t item_number,
              const SXParams *params_in, 
              const void *data_in, const void *variance_in, size_t item_size,
              long ori, int rot, int *perrval );

PUBLIC extern int 
   sx_debug_set( int debug ),
   sx_debug ( void ),
   sx_level ( void );

PUBLIC extern const char 
  *sx_version ( void ),
  *sx_usage2str( void );

PUBLIC extern char 
  *sx_errval2str ( char buffer[], size_t buflen, int errval );

/***************************************************************************
* Definitions                                                              *
***************************************************************************/

# define SXBUFLEN 1024

# define SX_VERBOSE      0x1
# define SX_LEVEL        0x6 // 0x2 0x4 -> level 0 -> 3
# define SX_SHOWDATA     0x8
# define SX_SHOWTEMP     0x10
# define SX_DEBUG        0x20
# define SX_RASTER_DEBUG 0x40

/***************************************************************************
* Error Values                                                             *
***************************************************************************/

# define SX_SUCCESS                         0
# define SX_NULL_POINTER                    1
# define SX_INVALID_PROJECTION              2 
# define SX_INVALID_ORIENTATION             3 
# define SX_INVALID_AXISTYPE                4 
# define SX_MEMORY_ALLOCATION_ERROR         5
# define SX_ORDER_CALCULATION_ERROR         6
# define SX_MATRIX_CALCULATION_ERROR        7 
# define SX_ANGLE_CALCULATION_ERROR         8 
# define SX_BEAMCENTER_CALCULATION_ERROR    9
# define SX_BEAMDISTANCE_CALCULATION_ERROR 10
# define SX_CENTER_CALCULATION_ERROR       11 
# define SX_DISTANCE_CALCULATION_ERROR     12
# define SX_COPY_ERROR                     13 
# define SX_ARRAY_TOOSMALL                 14
# define SX_RASTER_ERROR                   15 // must be the last

# define _SX_
#endif /* _SX_ */

/****************************************************************************
*                                                                           *
****************************************************************************/

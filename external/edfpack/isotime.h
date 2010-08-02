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

    isotime.h

SYNOPSIS

    #include "isotime.h"

DESCRIPTION
    Header of the module "isotime.c"
***********************************************************************---*/
#ifndef _ISOTIME_
# define _ISOTIME_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/

#ifndef PRIVATE
#  define PRIVATE static // used to declare variables of private type 
#endif

#ifndef PUBLIC
#  define PUBLIC         // used to declare variables of public type 
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

/******************************************************************************
* Public Definitions                                                          *
******************************************************************************/

# define EPOCHLEN   64  // sssssss...sssssssss.uuuuuu
# define ISOTIMELEN 64  // sssssss...sssssssss.uuuuuu

/******************************************************************************
* Public Type Defs                                                            *
******************************************************************************/

typedef struct isotime_epoch {
  int    status;
  long int sec; // integer part (in seconds since "1970-01-01 00:00:00")
  double fract; // decimal fraction (0<=fraction<1)
  long int offset; // offset from GMT
} IsotimeEpoch;

/*===========================================================================*/

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern 
       IsotimeEpoch isotime2epoch   (const char *isotime_s);
       IsotimeEpoch string2epoch    (const char *string);
       IsotimeEpoch setoffset2epoch (IsotimeEpoch epoch, const char *offset_s);
       IsotimeEpoch addoffset2epoch (IsotimeEpoch epoch, const char *offset_s);
       const char * epoch2isotime   (char buffer[], size_t buflen, 
                                     IsotimeEpoch epoch);
       const char * epoch2string    (char buffer[], size_t buflen, 
                                     IsotimeEpoch epoch);
       const char * isotime_version (void);
       void isotime_debug           (int debug);
#endif

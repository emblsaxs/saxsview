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

    gamma.h

SYNOPSIS

    #include "gamma.h"

DESCRIPTION
    Header of the module "gamma.c"
***********************************************************************---*/
#ifndef _GAMMA_
# define _GAMMA_

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

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern 
       double gamma( double X ), // gamma function
       loggamma( double X );     // logarithm of gamma function (X>0)

#endif

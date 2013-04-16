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

    statfunc.h

SYNOPSIS

    #include "statfunc.h"

DESCRIPTION

    Header of the module "statfunc.c"

***********************************************************************---*/
#ifndef _STATFUNC_

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

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern double 
       dmedian ( double a[], long n ),
     dmediance ( const double a[], long n, double m ),
       median  ( double a[], long n ),
       minimum ( double a[], long n ),
       maximum ( double a[], long n ),
      dquantil ( double a[], long n, double p ),
  dquantilance ( double a[], long n, double p ),
          mean ( double a[], long n ),
      variance ( double a[], long n, double m );

PUBLIC extern long
       minmaxfilter ( double a[], long n, double minval, double maxval );

PUBLIC extern const char 
  *statfunc_version ( void );

# define _STATFUNC_
#endif /* _STATFUNC_ */


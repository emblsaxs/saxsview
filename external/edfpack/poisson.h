/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 2000-2010 European Synchrotron Radiation Facility
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

    poisson.h

SYNOPSIS

    #include "poisson.h"

DESCRIPTION
    Header of the module "poisson.c"
***********************************************************************---*/
#ifndef _POISSON_
# define _POISSON_

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

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern double

       Poisson( long k, double ny ),           // poisson distribution
       SumPoisson( long k, double ny ),        // Sum(0,k,Poisson(k,ny))
       RandomNoise( void );          // random noise between 0.0 and 1.0

PUBLIC extern long
       InvSumPoisson ( double y, double ny ),  // inverted SumPoisson
       PoissonNoise( double ny );       // poissonian noise with mean ny


PUBLIC extern void 
       PoissonNoiseSeed( unsigned int seed );  // set random number seed

#endif

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

#ifndef _RASTER_
# define _RASTER_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/
#ifndef PRIVATE
# define PRIVATE       static /* used to declare variables of private type */
# define PUBLIC                /* used to declare variables of public type */
#endif

#ifndef TRUE
# define TRUE           1
# define FALSE          0
#endif

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

/***************************************************************************
* Definitions                                                              *
***************************************************************************/
# define MAX_RASTER_NUMBER_DIMENSION_32 9 /* for 32 bit long int */
# define MAX_RASTER_NUMBER_DIMENSION_64 16 /* for 64 bit long int */
# define MAX_RASTER_NUMBER_DIMENSION MAX_RASTER_NUMBER_DIMENSION_32

/***************************************************************************
* Functions                                                                *
***************************************************************************/

  PUBLIC extern long raster_numbers ( long n );

  PUBLIC extern int raster_normalization ( void * dest, const void * src,
                                           const long data_dim[],
                                           long raster_number, size_t item,
                                           int *perrval );

  PUBLIC extern long raster_multiplication ( long a, long x );

  PUBLIC extern long raster_inversion ( long x );

  PUBLIC extern long raster_order2number ( const long order[] );

  /* if buffer is NULL an output array with n+1 elements is allocated 
     and needs to be released */
  PUBLIC extern long * raster_number2order ( long * buffer, size_t nelem,
                                             long n, long raster_number );

  PUBLIC extern int raster_order_normalization ( void * dest, const void * src,
                                                 const long data_dim[],
                                                 const long order[], size_t item,
                                                 int *perrval);

  /* if buffer is NULL an output array with n+1 elements is allocated 
     and needs to be released */
  PUBLIC extern long * raster_order_multiplication (long *buffer, size_t nelem,
                                                    const long a_order[] , 
                                                    const long x_order[]);

  /* if buffer is NULL an output array with n+1 elements is allocated 
     and needs to be released */
  PUBLIC extern long * raster_order_inversion ( long *buffer, size_t nelem,
                                                const long x_order[] );

  PUBLIC extern long raster_order_determinante ( const long order[] );

  /* if buffer is NULL a sufficiently large output string is allocated 
     and needs to be released */
  PUBLIC extern char * raster_order2str( char * buffer, size_t nelem,
                                         const long order[], int *perrval );

  /* if buffer is NULL an output array with n+1 elements is allocated 
     and needs to be released */
  PUBLIC extern long * raster_str2order( long * buffer, size_t nelem,
                                         long n, const char *str,
                                         const char **tail, int *perrval );

  PUBLIC extern long raster_str2number( long n, const char *str,
                                        const char **tail, int *perrval );

  /* if buffer is NULL an output array with order[0]*order[0] elements
     is allocated and needs to be released */
  PUBLIC extern double * raster_order2matrix ( double * buffer, size_t nelem, 
                                               const long order[] );

  /* if buffer is NULL an output array with L*N elements is allocated 
     and needs to be released */
  PUBLIC extern double * raster_matrix_product ( double * buffer, size_t nelem,
                                                 double A[], double B[],
                                                 int L, int M, int N );

  /* print matrix  M[col][row] */
  PUBLIC extern int raster_fprint_matrix( FILE *out, 
                                          size_t nrows, size_t ncols, 
                                          double M[], const char * label );

  PUBLIC extern void raster_debug ( int debug );

  PUBLIC extern const char *raster_version ( void );

#endif

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

#ifndef _CMPR_
# define _CMPR_

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
# include <zlib.h>

/***************************************************************************
* Data Compression Methods (cmpr_method)                                   *
* The following enum start with 1. 0 is used to specify an invalid value.  *
* The enum list must end with EndDCompression.                             *
***************************************************************************/

  PUBLIC enum DCompression { InValidDCompression,
                             UnCompressed=1, 
                             GzipCompression,
                             ZCompression,
                             EndDCompression };

/***************************************************************************
* Functions                                                                *
* int cmpr_method is the compression method (DCompression),                *
* size_t *pconverted are the valid bytes in the output array               *
***************************************************************************/

  // compress array inp[inplen] to array out[outlen]
  PUBLIC extern int cmpr_deflate ( void * out, size_t outlen,
                const void * inp, size_t inplen,
                int cmpr_method, size_t * pconverted,
                int * perrval );

  // decompress array inp[inplen] to array out[outlen]
  PUBLIC extern int cmpr_inflate ( void * out, size_t outlen,
                const void * inp, size_t inplen,
                int cmpr_method, size_t * pconverted,
                int * perrval );

  // decompress not more than inplen bytes from file to array out[outlen]
  PUBLIC extern int cmpr_frinflate ( void * out, size_t outlen,
                FILE * channel, size_t inplen,
                int cmpr_method, size_t * pconverted,
                int * perrval );

  // decompress input file and write to output file
  PUBLIC extern int cmpr_inflatefile ( FILE * outp, FILE * inp,
                int cmpr_method, size_t * pconverted,
                int * perrval );

  // compress input file and write to output file
  PUBLIC extern int cmpr_deflatefile ( FILE * outp, FILE * inp,
                int cmpr_method, size_t * pconverted,
                int * perrval );

  PUBLIC extern void cmpr_debug ( int debug );

  PUBLIC extern const char *cmpr_errval2string(int errval);

  PUBLIC extern const char *cmpr_version ( void );

/****************************************************************************
*  Additional functions (CMPRUTILS)                                         *
****************************************************************************/

  // check the extension and return the DCompression type
  PUBLIC extern int cmpr_checkextension(const char *extension);

  // return the file DCompression type according to its extension
  PUBLIC extern int cmpr_checkfiletype(const char *filename);

  // adjust the file extension according to the DCompression (cmpr)
  PUBLIC extern char * cmpr_filename(char * buffer, size_t buflen, 
                                     const char *filename, int cmpr);

#endif

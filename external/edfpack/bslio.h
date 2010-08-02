/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 1998-2010 European Synchrotron Radiation Facility
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

   bslio.h

DESCRIPTION

   Include file for bslio.c

***************************************************************************/

#ifndef BSLIO

/***************************************************************************
* Includes                                                                 *
***************************************************************************/

#include "edfio.h"

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/

#ifndef PRIVATE
# define PRIVATE      static /* used to declare variables of private type */
# define PUBLIC               /* used to declare variables of public type */
#endif

/***************************************************************************
* Data Structure                                                           *
***************************************************************************/

   typedef struct Bsl_Data_Specification {
     char          *BinaryFileName;
     unsigned long BinaryFilePos;
     unsigned long BinaryFileLen;
     void          *Data;
     long          *Dim; /* Dim[0]=nn, Dim[1], ... Dim[nn] */
     int           DataType;
     int           ByteOrder;
     long          RasterConfiguration;
     int           Compression;
   } BslDataSpec;

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern int
   open_bsl_file ( const char * FileName , const char * mode ),
   close_bsl_file ( int stream ),

   bsl_memory_range( int stream, long * minmem, long *maxmem ),
   bsl_frame_range( int stream, long memnum, long * minfra, long * maxfra );

PUBLIC extern BslDataSpec
   *read_bsl_data_spec ( int stream, long memnum, long franum );

PUBLIC extern void
    read_bsl_file_headers ( int stream, char **first_header,
                                         char **second_header );

/***************************************************************************
* Debug routines                                                           *
***************************************************************************/

PUBLIC extern void
   print_bsl_data_spec ( FILE * out, const BslDataSpec * data_spec ),
   print_bsl_filetable ( FILE * out, int level, int verbose );

PUBLIC extern char
   *bslio_version ( void );

#endif
#ifndef BSLIO
#  define BSLIO
#endif
/***************************************************************************/


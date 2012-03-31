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

    readascii.h

SYNOPSIS

    #include "readascii.h"

DESCRIPTION
    Header of the module "readascii.c"

PURPOSE
  Conversion of ascii tables to float
  See PUBLIC functions for detail.

AUTHOR
  2007 Peter Boesecke (PB)

HISTORY
  2007-09-07 V1.0 PB

***************************************************************************/

#ifndef _READASCII_ 
#  define _READASCII_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/
#  ifndef PRIVATE
#    define PRIVATE   static
#    define PUBLIC
#  endif

/***************************************************************************
* Include Files                                                            *
***************************************************************************/
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <ctype.h>
# include <limits.h>
# include <errno.h>
# include <fcntl.h>
# include <math.h>
# include <float.h>

# include "edfio.h"
# include "numio.h"
# include "filename.h"

/***************************************************************************
* Structure Definitions                                                    *
***************************************************************************/
PUBLIC typedef struct RA_Descriptions {
  char * Key;                             // pointer to the key string
  struct RA_Descriptions *Previous,*Next; // the previous and next elements

  FILE * in;
  int verbose;
  char * filename;
  long  bskp;
  long  lskp;
  long  cskp;
  long  skipcol;
  long  skiprow;
  long  dim1;
  long  dim2;
  long  cnt;
  long  ori;
  float dummy;
  char * delimiterset;
  char * commentset;
  float  *buffer; // pointer to allocated buffer
  size_t bufsiz;  // size of allocated buffer

} RADescr;

/***************************************************************************
* PUBLIC Functions                                                         *
***************************************************************************/
PUBLIC RADescr *RA_openfile(const char *filename, int verbose, int *pstatus);
PUBLIC long RA_readfile( RADescr * descr, float ** pdata, int * pstatus );
PUBLIC void RA_closefile( RADescr * descr, int * pstatus );

PUBLIC int RA_setbskp( RADescr * descr, long bskp );
PUBLIC int RA_setlskp( RADescr * descr, long lskp );
PUBLIC int RA_setcskp( RADescr * descr, long cskp );
PUBLIC int RA_setskipcol( RADescr * descr, long skipcol );
PUBLIC int RA_setskiprow( RADescr * descr, long skiprow );
PUBLIC int RA_setdim1( RADescr * descr, long dim1 );
PUBLIC int RA_setdim2( RADescr * descr, long dim2 );
PUBLIC int RA_setori( RADescr * descr, long ori );
PUBLIC int RA_setdummy( RADescr * descr, float dummy );
PUBLIC int RA_setdelimiterset( RADescr * descr, const char *delimiterset );
PUBLIC int RA_setcommentset( RADescr * descr, const char *commentset );

PUBLIC const char * RA_version ( void );

#  endif
/************************************************************************---*/

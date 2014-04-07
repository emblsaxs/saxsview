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

    strlib.h

SYNOPSIS

    #include "strlib.h"

DESCRIPTION

    Header of the module "strlib.c"

***********************************************************************---*/
#ifndef _STRLIB_
# define _STRLIB_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/

#ifndef PRIVATE
# define PRIVATE       static /* used to declare variables of private type */
# define PUBLIC                /* used to declare variables of public type */
#endif

# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <limits.h>
# include <errno.h>
# include <string.h>
# include <ctype.h>
# include <fcntl.h>
# include <math.h>
# include <float.h>

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern int

  // returns 1 if c is white space
  strlib_is_white ( char c ),

  // returns 1 if s is not a skip pattern: "." | "..." | "-" 
  strlib_is_no_skip( const char * s ),

  // returns 1 if s either NULL, has length 0 or contains only white spaces
  strlib_is_empty( const char *s ),

  // splits s at each white space and write pointers to sv[]
  strlib_split ( char *sv[], int sc, char * s ),

  // non case sensitive comparison (compatible to strncasecmp)
  strlib_ncasecmp(const char *s1, const char *s2, size_t n);

PUBLIC extern char

  // concatenate strings a and b
  *strlib_concat( char * buffer, size_t buflen, 
                  const char * a, const char * b ),

  // terminates string at comment char '#'
  *strlib_uncomment ( char * s ),

  // removes leading and trailing white spaces
  *strlib_trim ( char * s ),

  // collapses multiple white spaces in s to a single space
  *strlib_collapse ( char * s ),

  // converts s to uppercase
  *strlib_toupper ( char * s ),

  // converts s to lowercase
  *strlib_tolower ( char * s ),

  // copies parameter parno to buffer and returns pointer
  *strlib_param ( char * buffer, size_t buflen,
                  const char *s, char separator, int parno ),

  // split a string into tokens
  *strlib_tok(char *s, const char *sep),

  // split a string into tokens (thread safe)
  *strlib_tok_r(char *s, const char *sep, char **lasts),

  // copy input string into new allocated memory
  *strlib_newstr( const char *string );

PUBLIC extern const char

  // return version string
  *strlib_version ( void );

#endif /* _STRLIB_ */

/****************************************************************************
*                                                                           *
****************************************************************************/

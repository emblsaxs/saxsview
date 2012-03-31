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

    filename.h

SYNOPSIS

    #include "filename.h"

DESCRIPTION
    Header of the module "filename.c"

    This module restricts possible filenames. Directories must be separated 
    in unix style with a slash. Backslashes are automatically converted to
    slashes. Filenames cannot not contain white spaces and commas. All 
    white spaces are collapsed. Filenames can be described by filepatterns 
    containing percent signs as placeholders for single decimal digits. All
    percent signs in a filename are filled from right to left with the file 
    number (default 0). Remaining percent signs are filled with 0. Parameters
    are separated with commas. In this sense, the filenpattern is parameter 
    zero of the whole pattern:
           
            <pattern>        ::= <filepattern>{,<params>}

            <params>         ::= <first>,<last>,<increment>

            <filepattern>    ::= <character>{<character>}

            <character>      ::= <alphanumerical> | <special>

            <alphanumerical> ::= '0' | '1' | ... | '9' | 
                                 'a' | 'b' | 'c' ... | 'z' | 
                                 'A' | 'B' | 'C' ... | 'Z' 
            <special>        ::= '/' | '.' | '_' | '-' | '+' | 
                                 '$' | '~' | '%'

            '$' and '~' are escape characters for environment parameters


      e.g.  $source/hyp01%%%%ccd,15,30,2

    The file pattern is hyp01%%%ccd, the parameters are 15, 30 and 2.

    The file pattern describes a sequence of file names (assuming $source
    is /users/data):

             /users/data/hyp010015ccd
             /users/data/hyp010017ccd
             /users/data/hyp010019ccd
             ...
             /users/data/hyp010029ccd

***************************************************************************/

#ifndef _FILENAME_ 
#  define _FILENAME_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/
#  ifndef PRIVATE
#    define PRIVATE   static
#    define PUBLIC
#  endif

PUBLIC extern char 
   // get unix-type filename
 * filename_unix ( char * buffer, size_t buflen, const char * pattern ),
   // get path
 * filename_path ( char * buffer, size_t buflen, const char * pattern ),
   // get filename
 * filename_name ( char * buffer, size_t buflen, const char * pattern ),
   // get filename without extension 
 * filename_body ( char * buffer, size_t buflen, const char * pattern ),
   // get extension
 * filename_extension ( char * buffer, size_t buflen, const char * pattern ),
   // get full filename
 * filename_full ( char * buffer, size_t buflen, const char * pattern ),
   // get parameter parno
 * filename_parameter ( char * buffer, size_t buflen,
                        const char *pattern, int parno ),
   // expand pattern with number and return filename
 * filename_pattern ( char * buffer, size_t buflen,
                      const char * pattern, long number );

PUBLIC extern int
   // return 0, if file does not exist
   filename_exists ( const char * filename ),
   // return 0, if files are equal
   filename_compare ( const char * filename1, const char * filename2 ),
   // return 0, if filename has not path
   filename_has_path ( const char * filename ),
   // return 0, if file name has no '%' pattern
   filename_has_pattern ( const char * filename );

PUBLIC extern const char
   // return version
 * filename_version ( void );

#endif
/************************************************************************---*/


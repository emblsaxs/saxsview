/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 2001-2010 European Synchrotron Radiation Facility
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

# define FILENAME_VERSION "filename : V1.9 Peter Boesecke 2008-05-07"
/*+++------------------------------------------------------------------------
NAME
   filename --- filename and directory routines

SYNOPSIS

   # include filename.h

INCLUDE FILES
   filename.h

RESTRICTIONS
   The maximum length of a filename or path is limited to BUFLEN
   which is set equal to PATH_MAX (usually defined in limits.h), or
   to MAX_FNAME (usually defined in stdio.h) or to 260 if none of both
   constants are defined.

DESCRIPTION

--------------------------------------------------------------------------*/

/***************************************************************************
* Private part                                                             *
***************************************************************************/

/***************************************************************************
* Include files                                                            *
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
# include <unistd.h>
#endif

#include <ctype.h>
#include <limits.h>

#include "filename.h"

/***************************************************************************
 * Defines                                                                 *
 ***************************************************************************/

#ifdef sun
# include <sys/param.h>
# define GETCWD(x,y) getwd (x)
#else
# ifdef WIN32
#  include <direct.h>
#  define GETCWD(x,y) _getcwd(x, y)
# else
#  define GETCWD(x,y) getcwd (x, y)
# endif
#endif

#ifdef WIN32
# // do not know, eventually special "dirent.h"
#else
# include <dirent.h>
# include <pwd.h>
#endif

#ifdef PATH_MAX
# define BUFLEN PATH_MAX
#else
# ifdef MAX_FNAME
#  define BUFLEN MAX_FNAME
# else
#  define BUFLEN 260
# endif
#endif

# ifndef MIN
#   define MIN( n1, n2) ( ( n1)<( n2) ? ( n1) : ( n2) )
# endif

# ifndef ABS
#   define ABS( n1) ( ( n1)<0 ? -( n1) : ( n1) )
# endif

/***************************************************************************
 * Global Uninitialised Variables                                          *
 ***************************************************************************/

#ifndef WIN32
static struct passwd *pwd_entry;
#endif

/******************************************************************************
* Private Constants                                                           *
******************************************************************************/

/* static char white_spaces[7] = { ' ', '\t', '\r', '\n', '\f', '\v', '\0' };*/

/******************************************************************************
* Private Type Defs                                                           *
******************************************************************************/

/******************************************************************************
* Private Variables                                                           *
******************************************************************************/

/******************************************************************************
* Routines                                                                    *
******************************************************************************/

/*+++------------------------------------------------------------------------
NAME

   filename_version --- return version string

SYNOPSIS

   const char * filename_version ( void );

DESCRIPTION

RETURN VALUE
   Pointer to a constant character string containing the version of
   this module.

----------------------------------------------------------------------------*/
const char * filename_version ( void )
{ return(FILENAME_VERSION);
} // filename_version

/*+++------------------------------------------------------------------------
NAME

   filename_parameter --- extract parameter string n from pattern 

SYNOPSIS

   char * filename_parameter ( char * buffer, size_t buflen,
                               const char *pattern, int parno );

DESCRIPTION
   Extracts parameter string parno from pattern. Parameter strings are
   terminated with commas. If pattern does not contain any comma the full
   pattern string is returned. If parameter parno is not available 
   (char *) NULL is returned. If pattern is a valid string parameter 
   string 0 does always exist.
   parno=0 returns the filename pattern, parno=1 returns the substring
   between the first and the second comma, parno=3 returns the 
   substring between the second and the third comma etc. The return value
   is NULL if the requested parameter cannot be found.

RETURN VALUE
   Pointer to the filled output buffer or NULL if parameter string is not
   available. 

----------------------------------------------------------------------------*/
char * filename_parameter ( char * buffer, size_t buflen, 
                            const char *pattern, int parno )
{ int cnt;
  size_t len;
  const char * str1, * str2, * strend;
  char Temp[BUFLEN];

  if (pattern == (char *) NULL) return ( (char *) NULL );

  if (parno<0) return ( (char *) NULL ); // parameter not available

  // copy pattern into temporary buffer
  len = MIN(BUFLEN,strlen(pattern)+1)-1;
  (void) strncpy (Temp, pattern, len+1);
  Temp[len] = '\0';

  strend = Temp+strlen(Temp);
  cnt = 0; str1 = Temp; str2 = str1-1;
  while ( cnt++ <= parno ) {
    // set str1 to start of parameter
    if ( str2 < strend ) {
      str1 = ++str2; // skip comma
    } else {
      return ( (char *) NULL ); // parameter not found
    }
    // search end of parameter
    if ( (str2 = strchr (str1, ',')) == (char *) NULL) {
      str2 = strend;
    } 
  }
  // copy parameter
  len = MIN(buflen,(unsigned long) (str2-str1+1))-1;
  (void) strncpy (buffer, str1, len);
  buffer[len] = '\0';

  return( buffer );

} // filename_parameter 

/*+++------------------------------------------------------------------------
NAME

   env_extract --- extract environment parameter

SYNOPSIS

   char * env_extract ( char * buffer, size_t buflen, const char ** pps );

DESCRIPTION
   Extract environment variables and copy to buffer. *pps must point
   to the beginning of an environment parameter, eventually in curly braces.

RETURN VALUE
   Pointer to the filled output buffer or NULL in case of an error.
   *ps is incremented and points after the end of the environment
   parameter or to the error position.

----------------------------------------------------------------------------*/
char * env_extract ( char * buffer, size_t buflen, const char ** pps )
{ size_t cnt;
  int run=1;
  int parcnt; // counts curly braces
  char *pb;

  if (**pps=='{') run=1; else run=2; // stop only after '}' if start is '{'
  for (cnt=0, pb=buffer, parcnt=0;
      (cnt<buflen-1) && (**pps) && (run) && (**pps!='/') && (**pps!='$');
      (*pps)++ ) {
    switch (**pps) {
      case '{' : parcnt++; break;
      case '}' : parcnt--; if (parcnt<=0) run--; break;
      default  : *pb++ = **pps; cnt++;
    }
  } // for
  *pb = '\0'; // terminate string

  if (parcnt!=0) return ( (char *) NULL );

  return( buffer );

} // env_extract

/*+++------------------------------------------------------------------------
NAME

   env_subst --- substitute environment variables

SYNOPSIS

   char * env_subst ( char * buffer, size_t buflen, const char * filepattern );

DESCRIPTION
   Substitute all environment variables in filepattern and write the resulting
   string to buffer.

RETURN VALUE

----------------------------------------------------------------------------*/
char * env_subst ( char * buffer, size_t buflen, const char * filepattern )
{
  size_t cnt, len;
  const char *ps; // pointer to input
  char *pb, // pointer to output
       *str, EnvStr[BUFLEN];

  if (filepattern == (char *) NULL) return ( (char *) NULL );

  // copy across each character as needed
  EnvStr[0] = '\0';
  for (cnt=0, pb=buffer, ps=filepattern; (cnt<buflen-1) && (*ps); ) {
    switch (*ps) {
#ifndef WIN32
      case '~': if (pb!=buffer) { // translate only a leading '~'
                  *pb++ = *ps++; cnt++;
                  break;
                  }
                ps++;
                str=env_extract ( EnvStr, BUFLEN, &ps );

                if (!str) return ( (char *) NULL );
                if (strlen(str)==0) {
                  pwd_entry = getpwuid (getuid ()); // current user's dir
                } else {
                  pwd_entry = getpwnam (str); // other user's dir
                }
                if ( (!pwd_entry) || (!(str = pwd_entry->pw_dir)) ) {
                  return ( (char *) NULL ); // no entry
                  }
                len = MIN(buflen,strlen(str)+1)-1;
                (void) strncpy (pb, str, len+1);
                pb += len; cnt += len;
                break;
#endif
      case '$': ps++;
                str=env_extract ( EnvStr, BUFLEN, &ps );

                if (!str) return ( (char *) NULL );

                if (strlen(str)==0) {
                  *pb++='$'; cnt++;
                } else {
                  str = getenv(str);
                  if (!str) return ( (char *) NULL ); // no entry
                  len = MIN(buflen,strlen(str)+1)-1;
                  (void) strncpy (pb, str, len+1);
                  pb += len; cnt += len;
                }
                break;
      case '{': str=env_extract ( EnvStr, BUFLEN, &ps );
                if (!str) return ( (char *) NULL );
                len = MIN(buflen,strlen(str)+1)-1;
                (void) strncpy (pb, str, len+1);
                pb += len; cnt += len;
                break;
      case '}': return ( (char *) NULL );  // invalid
                break;
      default : *pb++ = *ps++; cnt++;
    }
  } // for
  *pb = '\0'; // terminate string

  return( buffer );

} // env_subst

/*+++------------------------------------------------------------------------
NAME

   filename_unix --- return unix-type file pattern

SYNOPSIS

   char * filename_unix ( char * buffer, size_t buflen, const char * pattern );

DESCRIPTION
   Substitute environment variables, separate file pattern from parameters, 
   convert all back-slashes to slashes (pattern and buffer can be identical).

RETURN VALUE
   unix type file pattern or NULL in case of an error

----------------------------------------------------------------------------*/
char * filename_unix ( char * buffer, size_t buflen, const char * pattern )
{ const char * ps;
  char * pb, EnvPattern[BUFLEN], FilePattern[BUFLEN];
  size_t cnt;

  if (pattern == (char *) NULL) return ( (char *) NULL );

  // Substitute Environment Variables
  ps = env_subst( EnvPattern, BUFLEN, pattern );
  if (ps == (char *) NULL) return ( (char *) NULL );

  // Extract FilePattern
  ps = filename_parameter ( FilePattern, BUFLEN, EnvPattern, 0 );
  if (ps == (char *) NULL) return ( (char *) NULL );

  for (cnt=0, pb=buffer, ps=FilePattern; (cnt<buflen-1) && (*ps); ps++ ) {
    switch (*ps) {
      case '\\': *pb++ = '/'; cnt++; break;
       default : /* if (strchr(white_spaces,*ps)) break; // remove white space*/
                 *pb++ = *ps; cnt++;
      }
    } // for
  *pb = '\0'; 

  return( buffer );

} // filename_unix

/*+++------------------------------------------------------------------------
NAME

  filename_path --- get path from filename

SYNOPSIS

   char * filename_path ( char * buffer, size_t buflen, const char * pattern );

DESCRIPTION
   Separates file pattern from pattern, substitutes all environment 
   parameters and returns all characters of the resulting until the 
   last '/'. If it does not contain any '/'  "./" is returned (pattern
   and buffer can be identical).

SEE ALSO
   filename_name

RETURN VALUE
   path without filename or NULL in case of an error

----------------------------------------------------------------------------*/
char * filename_path ( char * buffer, size_t buflen, const char * pattern )
{ size_t len;
  char *str;

  str = filename_unix ( buffer, buflen, pattern );
  if (str == (char *) NULL) return ( (char *) NULL );

  if ((str = strrchr (str, '/')) == (char *) NULL) {
    str = "./";
    len = MIN(buflen,strlen(str)+1)-1;
    (void) strncpy (buffer, str, len+1);
    buffer[len] = '\0';
  } else {
    // Temp contains a path
    str++;
    *str = '\0';                          // terminate str after '/'
  }

  return( buffer );

} // filename_path

/*+++------------------------------------------------------------------------
NAME

   filename_name --- remove path from filename 

SYNOPSIS

   char * filename_name ( char * buffer, size_t buflen, const char * pattern );

DESCRIPTION
   Separates file pattern from parameters, substitutes all environment 
   parameters and returns all characters of the resulting string after 
   the last '/'. If it does not contain any '/' the full filepattern
   string is returned (pattern and buffer can be identical).

SEE ALSO
   filename_path

RETURN VALUE
   filepattern without path or NULL in case of an error

----------------------------------------------------------------------------*/
char * filename_name ( char * buffer, size_t buflen, const char * pattern )
{ size_t len;
  char *str, Temp[BUFLEN];

  str = filename_unix ( Temp, BUFLEN, pattern );
  if (str == (char *) NULL) return ( (char *) NULL );

  if ((str = strrchr (str, '/')) != (char *) NULL) {
    // Temp contains a path
    str++;
  } else {
    // Temp does not contain a path
    str = Temp;
  }

  len = MIN(buflen,strlen(str)+1)-1;
  (void) strncpy (buffer, str, len+1);
  buffer[len] = '\0';

  return( buffer );

} // filename_name

/*+++------------------------------------------------------------------------
NAME

   ext_split --- split string into body or extension. 

SYNOPSIS

   char * ext_split ( char * buffer, size_t buflen, const char * pattern );

DESCRIPTION
   This function searches the last character c in pattern and copies 
   the string before (mode=0) or after c (mode=1) into buffer.
   If pattern does not contain c the mode 0 returns pattern and mode 1
   returns an empty string.

RETURN VALUE
   Pointer to buffer or NULL in case of error
----------------------------------------------------------------------------*/
char * ext_split ( char * buffer, size_t buflen, const char * pattern, 
                   char c, int mode )
{ char Temp[BUFLEN];
  char *str, *s1,*s2;
  size_t len;

  if (pattern == (char *) NULL) return ( (char *) NULL );

  str = filename_parameter ( Temp, BUFLEN, pattern, 0 );

  /* Temp contains currently pattern */
  s1 = str;                            // s1 will hold the body
  s2 = str;                            // s2 will hold the extension 

  if ((str = strrchr (str, c)) == (char *) NULL) {
    //  Temp has no extension
    s1 = Temp;
    s2 = "";
  } else {
    // Temp contains an extension
    *str = '\0';                              // terminate s1 at '.'
    s2 = ++str;                  // point s2 to the character after the '.'
  }
  
  switch (mode) {
  case 0: str=s1; break; // return body
  case 1: str=s2; break; // return extension
  default: return( (char *) NULL );
  }

  // copy str to buffer
  len = MIN(buflen,strlen(str)+1)-1;
  (void) strncpy (buffer, str, len+1);
  buffer[len] = '\0';

  return( buffer );

} // ext_split

/*+++------------------------------------------------------------------------
NAME

   filename_body --- get filename without extension

SYNOPSIS

   char * filename_body ( char * buffer, size_t buflen, const char * pattern );

DESCRIPTION
   This function returns the filename without extension of the file
   given in the argument.

RETURN VALUE
   Pointer to buffer or NULL in case of error

----------------------------------------------------------------------------*/
char * filename_body ( char * buffer, size_t buflen, const char * pattern )
{ return( ext_split ( buffer, buflen, pattern, '.', 0 ));
} // filename_body

/*+++------------------------------------------------------------------------
NAME

   filename_extension --- get filename extension

SYNOPSIS

   char * filename_extension ( char * buffer, size_t buflen, 
                               const char * pattern );

DESCRIPTION
   This function returns the extension of the file given in the argument.

RETURN VALUE
   Pointer to buffer or NULL in case of error

----------------------------------------------------------------------------*/
char * filename_extension ( char * buffer, size_t buflen, 
                            const char * pattern )
{ return( ext_split ( buffer, buflen, pattern, '.', 1 ));
} // filename_extension

/*+++------------------------------------------------------------------------
NAME

   filename_full --- return full filename

SYNOPSIS

   char * filename_full ( char * buffer, size_t buflen, const char * pattern );

DESCRIPTION
   This function returns the full path name of the file 
   given in the argument. The argument is expanded to it's
   name - using expansions for ~, ~name and $name. Note that
   the file need not exist but the path to it must be readable
   by the user. If the routine worked the pointer to buffer
   is returned. A return value of zero means that some part of 
   the path to the file is invalid. If buflen is too small
   the filename is truncated (pattern and buffer can be identical).

RETURN VALUE
   Pointer to buffer or NULL in case of error

----------------------------------------------------------------------------*/
char * filename_full ( char * buffer, size_t buflen, const char * pattern )
{ size_t len;
  char *str, *s1, *s2,
       Path[BUFLEN], NewPath[BUFLEN], Temp[BUFLEN];

  str = filename_unix ( Temp, BUFLEN, pattern );
  if (str == (char *) NULL) return ( (char *) NULL );

  if ( strcmp(str,"/dev/null") == 0 ) { // /dev/null exists, no further check
    len = MIN(buflen,strlen(str)+1)-1;
    (void) strncpy (buffer, str, len+1);
    buffer[len] = '\0'; // truncate
    return( buffer );
  }

  (void) GETCWD (Path, BUFLEN);         // remember current directory

  /* Temp contains currently the filename */
  s1 = str;                            // s1 will hold leading directories
  s2 = str;                            // s2 will hold the filename

  if ((str = strrchr (str, '/')) == (char *) NULL) {
    // Temp has no path, use current working directory (Path)
    s1 = Path;
  } else {
    // Temp contains a path
    *str = '\0';                              // terminate s1 at '/' 
    if (*s1=='\0') s1="/";

    s2 = ++str;                  // point s2 to the character after the '/' 

    if (chdir (s1)) {
      chdir (Path);
      return ( (char *) NULL );
    }
    (void) GETCWD (NewPath, BUFLEN);	        // determine full path name
    if (chdir (Path)) return ( (char *) NULL );  // change back to old path
    s1 = NewPath;
  }
  // remove eventual last slash from s1
  str = &NewPath[strlen(s1)-1];
  if (*str=='/') *str='\0';

  // combine to full filename 
  (void) sprintf (Path, "%s/%s", s1, s2);
  len = MIN(buflen,strlen(Path)+1)-1;
  (void) strncpy (buffer, Path, len+1);
  buffer[len] = '\0'; // truncate

  return( buffer );

} // filename_full

/*+++------------------------------------------------------------------------
NAME

  filename_has_path --- return 0, if file name does not start with a path.

SYNOPSIS

  int filename_has_path ( const char * filename )

DESCRIPTION
  Tests whether filename contains a path. Should be used after filename_unix.

RETURN VALUE
  returns only 0, if file name has no path.
----------------------------------------------------------------------------*/
int filename_has_path ( const char * filename )
{ int has_path=0;

  if ((strchr (filename, '/')) != (char *) NULL)
    has_path=1;
  return(has_path);

} // filename_has_path

/*+++------------------------------------------------------------------------
NAME

  filename_has_pattern --- return 0, if file name has no '%' pattern 

SYNOPSIS

  int filename_has_pattern ( const char * filename )

DESCRIPTION
  Tests whether filename contains a '%'. 

RETURN VALUE
  returns only 0, if file name has no '%' pattern
----------------------------------------------------------------------------*/
int filename_has_pattern ( const char * filename )
{ 
  if ( strrchr (filename, '%') == (char *) NULL )
    return(0); else return(1);

} // filename_has_pattern

/*+++------------------------------------------------------------------------
NAME

   filename_pattern --- expand pattern with number and return filename

SYNOPSIS

   char * filename_pattern ( char * buffer, size_t buflen,
                             const char * pattern, long number );

DESCRIPTION
  Extracts file pattern from pattern and replaces all place holders 
  (percent signs '%' by number with leading zeros. Place holders do not
  need to be continuous. If number is negative the first '%' is replaced 
  by a minus sign '-'. If not enough place holders are given the result is 
  truncated.

RETURN VALUE
  Pointer to buffer or (char*) NULL in case of an error
----------------------------------------------------------------------------*/
char * filename_pattern ( char * buffer, size_t buflen,
                          const char * pattern, long number )
{ size_t len;
  char *pb, *str;
  char *start, *stop;
  char Temp[BUFLEN];

  if (pattern == (char *) NULL) return ( (char *) NULL );

  // copy pattern into temporary buffer
  len = MIN(BUFLEN,strlen(pattern)+1)-1;
  (void) strncpy (Temp, pattern, len+1);
  Temp[len] = '\0';

  // copy temporary buffer to output buffer
  len = MIN(buflen,strlen(Temp)+1)-1;
  (void) strncpy (buffer, Temp, len+1);
  buffer[len] = '\0';

  // determine begin and end of place holders in string
  if ( (stop = strrchr (buffer, '%')) == (char *) NULL) {
    // no placeholders, return unmodified string 
    return( buffer );
  } 
  start = strchr (buffer, '%');

  // fill place holders with number
  sprintf(Temp,"%lu",ABS(number));
  for ( pb=stop, str=Temp+strlen(Temp)-1; pb>=start; *pb-- ) {
    if (*pb=='%') {
      if ( (str>=Temp)&&(*str!=' ') ) {
        *pb=*str--;
      } else {
        *pb='0';
      }
    }
  }
  pb++;
  // if number is negative write sign
  if (number<0) *pb='-'; 

  return( buffer );

} // filename_pattern

/*+++------------------------------------------------------------------------
NAME

   filename_exists --- return 0, if file does not exist

SYNOPSIS

   int filename_exists ( const char * filename );

DESCRIPTION
   This function tests to see if the named file exists (True)
   or not (False). The Argument is first tested for a value as
   an environment variable and this value is substituted (if
   appropriate). The argument is then tested for it's existence.

RETURN VALUE
   Returns 0 if file does not exist

----------------------------------------------------------------------------*/
int filename_exists ( const char * filename )
{
#ifndef WIN32
  int Found = 0;
  DIR   *dirp;
  struct dirent *dp;

  char *Name, *str, Path[BUFLEN], Temp[BUFLEN];

  str = filename_unix ( Temp, BUFLEN, filename );
  if (str == (char *) NULL) return ( 0 );

  if ((str = strrchr ( Temp, '/')) == (char *) NULL) {
    (void) strcpy (Path, ".");
    Name = Temp;
  } else {
    str++;
    (void) strncpy(Path,Temp,str-Temp);
    Path[str-Temp]='\0';
    Name = str;
  }

  if ((dirp = opendir (Path)) != (DIR *) NULL) {
    while ((dp = readdir (dirp)) != (struct dirent *) NULL) {
      if (! strcmp (dp->d_name, Name)) {
        Found = 1;
        break;
      }
    }
    (void) closedir(dirp);
  }
  return ( Found );
#else
//  could probably be constructed with
//  char **findfiles(char *dir,int flags=0);
  return ( 1 );
#endif

} // filename_exists 

/*+++------------------------------------------------------------------------
NAME

   filename_compare --- return 0, if files are equal 

SYNOPSIS

   int filename_compare ( const char * filename1, const char * filename2 );

DESCRIPTION
   Expands both filenames with filename_full and compares the strings with
   strcmp. The result of strcmp is returned.
   Both filenames must be correct. If one of the filenames is invalid
   it is replaced by an empty string.

RETURN VALUE
   0, if files are equal or if at least one of the filenames are invalid.

----------------------------------------------------------------------------*/
int filename_compare ( const char * filename1, const char * filename2 )
{ 
  char *str1, *str2;
  char FileName1[BUFLEN], FileName2[BUFLEN];

  str1 = filename_full ( FileName1, BUFLEN, filename1 );
  if (str1==(char*) NULL) str1="";
  str2 = filename_full ( FileName2, BUFLEN, filename2 );
  if (str2==(char*) NULL) str2="";

  return ( strcmp(str1, str2) );

} // filename_compare 

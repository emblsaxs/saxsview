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
# define STRLIB_VERSION    "strlib : V1.05 Peter Boesecke 2011-12-12"
/*---------------------------------------------------------------------------
NAME

   strlib.c --- string functions 

DESCRIPTION

   Collection of string functions.

HISTORY

   2011-04-27 V1.0  Peter Boesecke
   2011-05-14 V1.01 PB functions renamed from str.. to strlib_..., 
                       strlib_ncasecmp, strlib_toupper, strlib_tolower added,
                       strlib_collapse collapses always to a single space.
   2011-05-25 V1.02 PB strlib_toupper, strlib_tolower: corrected
   2011-06-30 V1.03 PB strlib_concat added
   2011-10-22 V1.04 PB strlib_tok and strlib_tok_r added
   2011-12-12 V1.05 PB strlib_concat corrected for buffer==a
    
---------------------------------------------------------------------------*/

/****************************************************************************
*  Include                                                                  *
****************************************************************************/
# include <errno.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <ctype.h>

# include "strlib.h"

/***************************************************************************
 * Defines                                                                 *
 ***************************************************************************/

# define BUFLEN EdfMaxLinLen+1

# ifndef MIN
#   define MIN( n1, n2) ( ( n1)<( n2) ? ( n1) : ( n2) )
# endif

# ifndef ABS
#   define ABS( n1) ( ( n1)<0 ? -( n1) : ( n1) )
# endif

/****************************************************************************
* Static Variables                                                          *
****************************************************************************/

PRIVATE char white_spaces[7] = { ' ', '\t', '\r', '\n', '\f', '\v', '\0' };
  
/****************************************************************************
*  Functions                                                                *
****************************************************************************/

/*--------------------------------------------------------------------------
NAME

  strlib_version --- returns pointer to the version string

SYNOPSIS

  const char *strlib_version ( void );

DESCRPTION

  Returns pointer to the version string.

--------------------------------------------------------------------------*/
PUBLIC const char *strlib_version ( void )
{
  return ( STRLIB_VERSION );
} /* strlib_version */

/*---------------------------------------------------------------------------
NAME

    strlib_is_white

SYNOPSIS

    int strlib_is_white ( char c );

DESCRIPTION

Returns 1 if c is a white space, 0, if not.

HISTORY
18-Jan-1998 Peter Boesecke
---------------------------------------------------------------------------*/
PUBLIC int strlib_is_white ( char c )
{
   if (!c) return(0); // no white space
   if ( strchr( white_spaces, (int) c ) )
     return(1); // white space 
   else return(0); // no white space 

} /* strlib_is_white */

/*---------------------------------------------------------------------------
NAME

    strlib_is_no_skip --- returns 0 when the input string is a skip pattern. 

SYNOPSIS

   int strlib_is_no_skip( const char * s );

DESCRIPTION

   Checks, if the input string is a skip sign: "...", "-", ".".

RETURN VALUE

   Returns only 0, when the input string is a skip sign.

---------------------------------------------------------------------------*/
PUBLIC int strlib_is_no_skip( const char * s )
{ int value=0;

  value = strcmp ( s, "..." );
  if (value) value = strcmp ( s, "-" );
  if (value) value = strcmp ( s, "." );

  return ( value );
} // strlib_is_no_skip

/*---------------------------------------------------------------------------
NAME

    strlib_is_empty --- returns 1 if the input string is empty or not allocated

SYNOPSIS

   int strlib_is_empty( const char *s );

DESCRIPTION

   Rreturns 1 if the input string s contains only white space, if it
   has the length 0, or is the NULL pointer, otherwise 0.

RETURN VALUE

   1: if string is empty, 0: otherwise

---------------------------------------------------------------------------*/
PUBLIC int strlib_is_empty( const char *s )
{ const char *ps;
  int empty=1;

  if (s) {
    ps = s;
    while (*ps) { empty = empty&&strlib_is_white(*ps); ps++; }
  }
  return( empty );

} // strlib_is_empty

/*---------------------------------------------------------------------------
NAME

    strlib_uncomment --- removes comments 

SYNOPSIS

   char * strlib_uncomment ( char * s );

DESCRIPTION

   Truncates the string at the comment char ('#').

RETURN VALUE

   Pointer to the modified input string.

---------------------------------------------------------------------------*/
PUBLIC char * strlib_uncomment ( char * s )
{ char *ps;

  if (!s) return("");

  ps=s;

  // copy ps2 to ps1 until end of string or comment char
  while ( *ps ) {
    if (*ps == '#') break;
    ps++;
  }

  // terminate string
  *ps='\0';

  return( s );

} // strlib_uncomment

/*---------------------------------------------------------------------------
NAME

    strlib_trim --- removes leading and trailing white spaces

SYNOPSIS

   char * strlib_trim ( char * s );

DESCRIPTION

   Removes leading and trailing white spaces from the input string.

RETURN VALUE

   Pointer to the modified input string.

---------------------------------------------------------------------------*/
PUBLIC char * strlib_trim ( char * s )
{ char *ps1, *ps2;

  if (!s) return("");

  ps1=ps2=s;

  // search first non-white character (nul is not a white space)
  while ( strlib_is_white( *ps2 ) ) ps2++;

  // copy ps2 to ps1 until end of string
  while ( *ps2 ) {
    *ps1++=*ps2++;
  }

  // terminate string
  *ps1='\0';

  // remove trailing white spaces
  while ( --ps1>=s ) {
    if ( !strlib_is_white(*ps1) ) break;
    *ps1='\0';
  }

  return( s );

} // strlib_trim

/*---------------------------------------------------------------------------
NAME

    strlib_collapse --- collapses to single white spaces

SYNOPSIS

   char * strlib_collapse ( char * s );

DESCRIPTION

   Collapses multiple white spaces to a single space ' '.

RETURN VALUE

   Pointer to the modified input string.

---------------------------------------------------------------------------*/
PUBLIC char * strlib_collapse ( char * s )
{ 
  char *ps1, *ps2;

  if (!s) return("");

  ps1=ps2=s;

  while (*ps2) {
    // copy ps2 to ps1 until next white space
    while ( (*ps2) && (!strlib_is_white( *ps2 )) ) {
      *ps1++=*ps2++;
    }

    // write a single white space
    if (*ps2) {
      *ps1++ = ' ';
      ps2++;
    }

    // search first non-white character
    while ( strlib_is_white( *ps2 ) ) ps2++;

  }

  // terminate string
  *ps1='\0';

  return( s );

} // strlib_strcollapse

/*---------------------------------------------------------------------------
NAME

    strlib_split --- splits string at white spaces

SYNOPSIS

   int strlib_split ( char *sv[], int sc, char * s );

DESCRIPTION

   Splits s white spaces into substrings and writes the pointers to the
   first sc-1 substrings into subsequent cells of sv[]. All white spaces 
   in s are replaced with character '\0' as terminator.  The length 
   of sv is sc. The pointers in sv are valid as long as the string s exists.

RETURN VALUE

   Number of substring pointers in sv[].

---------------------------------------------------------------------------*/
PUBLIC int strlib_split ( char *sv[], int sc, char * s )
{ char *nul = (char *) NULL;
  int cnt=0;
  char *ps;

  if ((!s)||(!sv)||(!sc)) return(0);

  ps=s;

  cnt=0;

  // skip leading white spaces
  while (strlib_is_white( *ps )) ps++;

  while (*ps) {

    if (cnt+1<sc) sv[cnt++]=ps;

    // search next white space
    while ( (*ps) && (!strlib_is_white( *ps )) ) ps++;

    if ( strlib_is_white( *ps ) ) {
      *ps='\0';
      ps++;
    }

    // skip leading white spaces
    while (strlib_is_white( *ps )) ps++;

  }

  // terminate
  if (cnt<sc) sv[cnt]=nul;

  return( cnt );

} // strlib_split

/*---------------------------------------------------------------------------
 NAME

    strlib_concat --- return a pointer to the concatenation of a and b

 SYNOPSIS

    char *strlib_concat( char *buffer, size_t buflen, 
                         const char *a, const char *b )


 DESCRIPTION

    If the pointers to a and to buffer are different the function copies the 
    string a to the destination buffer and appends afterwards the string b.
    If the pointers to a and b are identical the string b is immediately
    appended to the destination buffer without copying a. Apart from that
    the destination buffer may not overlap with the input strings a and b.
    The output string is truncated at buflen-1. The output string is 
    terminated with zero. 

 RETURN VALUE

    pointer to buffer with concatenated strings

---------------------------------------------------------------------------*/
char * strlib_concat( char * buffer, size_t buflen, 
                      const char * a, const char * b )
{ size_t rest, len;
  char *pb;
  if ((!buffer)||(!buflen)) return( buffer );
  buffer[buflen-1] = '\0'; // terminating zero
  pb=buffer;
  rest=buflen-1;
  if (pb!=(char*)a) {
    strncpy( pb, a, rest );
    len=strlen(pb);
    pb+=len;
    rest-=len;
  } else {
   // set pb after last char in buffer
   len=strlen(buffer);
   pb = buffer+len;
   rest = buflen-1-len;
  }
  strncpy( pb, b, rest);
  return ( buffer );
} /* strlib_concat */

/*+++------------------------------------------------------------------------
NAME

   strlib_param --- extract parameter n from string 

SYNOPSIS

   char * strlib_param ( char * buffer, size_t buflen,
                         const char *s, char separator, int parno );

DESCRIPTION
   Extracts substrings from string that are separated by separator and copies 
   them to buffer. If the input string does not contain a comma the full 
   input string is copied. 
   If substring parno is not available (char *) NULL is returned. parno 0 
   returns always the full string.
   Different to the function filename_parameter, the substring before the 
   first comma has the number 1.

   <0> = "<1><separator><2><separator><3><separator>..."

RETURN VALUE
   Pointer to the filled output buffer or NULL if parameter string is not
   available. A parameter between two separators that immediately follow 
   each other is returned as an empty string.

----------------------------------------------------------------------------*/
PUBLIC char * strlib_param ( char * buffer, size_t buflen, 
                             const char *s, char separator, int parno )
{ size_t len;
  char *tmp=NULL;
  const char *str1, *str2, *strend;
  int cnt;

  if (!s) goto strlib_param_error; // missing input string 

  if (parno<0) goto strlib_param_error; // parameter is not available

  len = strlen(s);
  if ( !( tmp = (char*) malloc( sizeof(char)*(len+1) )) )
    goto strlib_param_error;

  // copy s into temporary buffer
  strcpy (tmp, s);

  strend = tmp+len;
  cnt = 0; str1 = tmp; str2 = str1-1;
  while ( cnt++ < parno ) { // start numbering before the first separator
    // set str1 to start of parameter
    if ( str2 < strend ) {
      str1 = ++str2; // skip separator
    } else {
      goto strlib_param_error; // no more parameter available
    }
    // search end of parameter
    if ( (str2 = strchr (str1, separator)) == (char *) NULL) {
      str2 = strend;
    } 
  }

  // copy parameter
  len = MIN(buflen,(size_t) (str2-str1+1))-1;
  strncpy (buffer, str1, len);
  buffer[len] = '\0';

  if (tmp) free( tmp );

  return( buffer );

strlib_param_error:

  if (tmp) free( tmp );

  return(  (char *) NULL );

} // strlib_parameter 

/*---------------------------------------------------------------------------
NAME

    strlib_toupper --- converts all characters to upper case 

SYNOPSIS

   char * strlib_toupper ( char * s );

DESCRIPTION

  Converts all characters to upper case and returns pointer to input string.

RETURN VALUE

  Pointer to input string 

---------------------------------------------------------------------------*/
PUBLIC char * strlib_toupper ( char * s )
{ char * ps;

  if(s) {
    ps=s;
    while ( *ps ) { *ps=(char)toupper(*ps); ps++; }
  }

  return(s);

} // strlib_toupper

/*---------------------------------------------------------------------------
NAME

    strlib_tolower --- converts all characters to lower case

SYNOPSIS

   char * strlib_tolower ( char * s );

DESCRIPTION

  Converts all characters to lower case and returns pointer to input string.
       
RETURN VALUE
       
  Pointer to input string
       
---------------------------------------------------------------------------*/
PUBLIC char * strlib_tolower ( char * s )
{ char * ps;

  if (s) {
    ps=s;
    while ( *ps ) { *ps=(char)tolower(*ps); ps++; }
  }

  return(s);

} // strlib_tolower

/*---------------------------------------------------------------------------
NAME

        strlib_ncasecmp ---  strncasecmp

SYNOPSIS

       int strlib_ncasecmp(const char *s1, const char *s2, size_t n);


DESCRIPTION
       The function compares the first n characters ot the two
       strings s1  and s2,  ignoring  the  case of the characters.
       It returns an integer less than, equal to, or greater than
       zero if s1 is less  than, matches, or is greater than s2.
       It can be used instead of the function strncasecmp if this
       function is not available.

       NULL pointers are treated as empty strings.

RETURN VALUE
       The function returns an integer less than, equal to, or
       greater than  zero.

---------------------------------------------------------------------------*/
PUBLIC int strlib_ncasecmp(const char *s1, const char *s2, size_t n)
{ int value;
  size_t s1len, s2len;
  char *_s1, *_s2;
  register unsigned int i;

  if (!s1) s1="";
  if (!s2) s2="";

  s1len = strlen(s1);
  s1len = (s1len<n)?s1len:n;
  _s1 = (char *) malloc( s1len+1 );
  for (i=0;i<s1len;i++) _s1[i]=(char)tolower(s1[i]);
  _s1[s1len]='\0';

  s2len = strlen(s2);
  s2len = (s2len<n)?s2len:n;
  _s2 = (char *) malloc( s2len+1 );
  for (i=0;i<s2len;i++) _s2[i]=(char)tolower(s2[i]);
  _s2[s2len]='\0';

  value = strcmp(_s1,_s2);

  free(_s2);
  free(_s1);

  return(value);

} /* strlib_ncasecmp */

/*---------------------------------------------------------------------------
NAME

      strlib_tok_r --- split a string into tokens 

SYNOPSIS

      char *strlib_tok_r(char *s, const char *sep, char **lasts);


DESCRIPTION

      The `strlib_tok_r' function is used to isolate sequential tokens in a 
      NULL-terminated string `*s'. It replaces the string.h function 
      `strtok_r' when it is not available. 

      These tokens are delimited in the string by at least one of the 
      characters in `*sep'. The first time that `strlib_tok_r' is called, 
      `*s' must be specified; subsequent calls, wishing to obtain further 
      tokens from the same string, must pass a NULL pointer instead. The 
      separator string, `*sep', must be supplied each time and may change 
      between calls.

      The  `strlib_tok_r'  function  returns  a pointer to the beginning of 
      each subsequent token in the string, after replacing the separator 
      character itself with a null character. When no more tokens remain, a 
      NULL pointer is returned. The placeholder `*lasts' must be supplied by 
      the caller and must not be modified.

      The `strlib_tok_r' function has the same behavior as `strlib_tok', 
      except that a pointer to placeholder `*LASTS' must be supplied by 
      the caller.

RETURN VALUE

     `strlib_tok_r' returns a pointer to the next token, or `NULL' if no 
     more tokens can be found.

---------------------------------------------------------------------------*/
PUBLIC char *strlib_tok_r(char *s, const char *sep, char **lasts)
{ char *pstart,*ps,*retval=(char*) NULL;

  if (s) {
    /* skip starting separators */
    for (pstart=s;*pstart;pstart++) {
      if (!(strchr(sep,(int) *pstart))) {
        retval=pstart;
        break;
      }
    }
  } else {
    pstart=(lasts)?*lasts:(char*)NULL;
    retval=pstart;
  }
  
  *lasts=(char*)NULL;
  if (pstart) {
    for (ps=pstart;*ps;ps++) {
      if (strchr(sep,(int) *ps)) {
        *ps='\0';
        *lasts = ps+1;
        break;
      }
    }
  }

  return( retval );
  
} /* strlib_tok_r */

/*---------------------------------------------------------------------------
NAME

      strlib_tok --- split a string into tokens

SYNOPSIS

      char *strlib_tok(char *s, const char *sep);


DESCRIPTION

      The `strlib_tok_r' function is used to isolate sequential tokens in a
      NULL-terminated string `*s'. It replaces the string.h function `strtok'
      when it is not available.

      These tokens are delimited in the string by at least one of the
      characters in `*sep'. The first time that `strlib_tok_r' is called,
      `*s' must be specified; subsequent calls, wishing to obtain further
      tokens from the same string, must pass a NULL pointer instead. The
      separator string, `*sep', must be supplied each time and may change
      between calls.

      The  `strlib_tok'  function  returns  a pointer to the beginning of
      each subsequent token in the string, after replacing the separator
      character itself with a null character. When no more tokens remain, a
      NULL pointer is returned. 

      The `strlib_tok' function has the same behavior as `strlib_tok_r', 
      except that the placeholder `*LASTS' cannot be specified. It is kept 
      in an internal static buffer for subsequent calls and consequently the 
      function is not thread safe.

RETURN VALUE

     `strlib_tok' returns a pointer to the next token, or `NULL' if no
     more tokens can be found.

---------------------------------------------------------------------------*/
PUBLIC char *strlib_tok(char *s, const char *sep)
{ static char *lasts;

  return( strlib_tok_r(s, sep, &lasts) );

} /* strlib_tok */

/*---------------------------------------------------------------------------
NAME

     strlib_newstr --- copy input string into new allocated memory

SYNOPSIS

     char *strlib_newstr( const char *string );

DESCRIPTION

     Makes a copy of the input string into new allocated memory. The memory 
     must be released with free( ) when it is not needed any more.

RETURN VALUE

     Pointer to allocated string or NULL in case of error.

---------------------------------------------------------------------------*/
PUBLIC char *strlib_newstr( const char *string )
{ char *new;

  if ( (new=malloc( strlen(string)+1 )) )
    strcpy(new,string);
  return(new);

} /* strlib_newstr */


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

# define ISOTIME_VERSION      "isotime : V1.6 Peter Boesecke 2012-10-28"
/*+++------------------------------------------------------------------------
NAME
   isotime --- routines for isotime conversion 

SYNOPSIS

   # include isotime.h

HISTORY
  2006-05-30 V1.0 Peter Boesecke
  2006-06-05 V1.1 PB
  2007-04-19 V1.2 PB code corrected to avoid compiler warnings with -Wall
  2010-05-27 V1.3 PB trim: unsigned long -> long, otherwise the condition
                           i>=0 in for loop would always be TRUE
  2010-12-12 V1.4 PB _convert2epoch: all epoch value preset
  2012-10-27 V1.5 PB cpo: skip leading white spaces,
                     shiftepoch added
  2012-10-28 V1.6 PB isotime_mode, isotime_str2mode, isotime_mode2str

PUBLIC extern
       IsotimeEpoch isotime2epoch( const char * isotime_s ),
       const char * epoch2isotime( char buffer[], size_t buflen,
                                   IsotimeEpoch epoch );

LIMITATION
  earliest date: 1901-12-13T20:45:52.000000+0000
    latest date: 2038-01-19T03:14:07.999999+0000

  To extend this range the structure IsotimeEpoch needs to be extended. It
  would be possible to add the number of days since 1970-01-01. This would
  increase the time to +-~5e6 years.

  isotime modes: 
    IsotimeModeNoSpace: write isotime string without spaces (default), e.g.
                        2012-01-01T11:23:44.234000+0200
    IsotimeModeSpace:   write isotime string with spaces, e.g.
                        2012-01-01 11:23:44.234000 +0200

----------------------------------------------------------------------------*/
/******************************************************************************
* Include Files                                                               *
******************************************************************************/

# include "isotime.h"
# include "strlib.h"

/******************************************************************************
* Private Definitions                                                         *
******************************************************************************/
#ifndef MIN2
# define MIN2( x, y ) (( x ) > ( y ))?( y ):( x )
#endif

/******************************************************************************
* Private Constants                                                           *
******************************************************************************/
# define TRIMLEN 64         // YYYYMMDDhhmmss.uuuuuu+HhMmSs
# define EPOLEN EPOCHLEN    // sssssss...sssssssss.uuuuuu
# define ISOLEN ISOTIMELEN  // sssssss...sssssssss.uuuuuu
# define SLEN 64            // sssssss...ssssssssssssssss
# define DLEN 9             // YYYYMMDD
# define TLEN 7             // hhmmss
# define FLEN 8             // .uuuuuu
# define OLEN 8             // +HhMmSs

# define DAYS_19700101 719528l

/****************************************************************************
* Static Variables                                                          *
****************************************************************************/
static int ISOTIME_debug = 0;
static int ISOTIME_mode = IsotimeModeNoSpace;

/*--------------------------------------------------------------------------*/
/* long leap_days: number of accumulated leap days until 1st day of the year*/
/*--------------------------------------------------------------------------*/
long leap_days ( long year )
{  long ldays;
   year-=1;
   ldays = floor(year/4)-floor(year/100)+floor(year/400);
   return( ldays );
} // leap_days

/*--------------------------------------------------------------------------*/
/* int leap_year: returns 1 for leap years, 0 otherwise                     */
/*--------------------------------------------------------------------------*/
int leap_year ( long year )
{ int lyear;
  if (((!(year%4))&&(year%100))||(!(year%400)))
    lyear=1; else lyear=0;
  return ( lyear );
} // leap_year

/*--------------------------------------------------------------------------*/
/* char *trim : copy to buffer, to uppercase and trim                       */
/*--------------------------------------------------------------------------*/
char *trim( char buffer[], size_t buflen, const char * s )
{ const char *pstart, *pend, *ps;
  size_t slen=0;
  // unsigned long i; condition i>=0 in for loop would always be true 
  long i;

  if ( s ) {
    pstart = s;

    while (isspace(*pstart)) pstart++;  // skip leading white spaces
    for (i = strlen (s) -1; i>=0 && isspace(s[i]); i--);
    pend = &s[i+1];                 // end position

    slen = pend-pstart;
    slen = MIN2(slen,buflen-1);

    ps = pstart;
    for ( i=0;i<slen;i++ ) buffer[i] = toupper( *ps++ ); // uppercase copy
  }
  buffer[slen]='\0'; // terminate

  return ( buffer ); // return buffer

} // trim

/*--------------------------------------------------------------------------*/
/* cpd: copy date from s to buffer, return pointer to next char in s        */
/*      In the case of an error the buffer contains an empty string and     */
/*      the pointer to the input string s is returned.                      */
/*--------------------------------------------------------------------------*/
char * cpd( char buffer[], size_t buflen, char * s )
{ char *ps, *pb;
  size_t cnt;
  long i;

  ps = s;
  pb = buffer; *pb='\0';
  cnt = buflen-1;

  if ( s ) {
    // copy year (4 digits)
    for (i=0;(i<4)&&(*ps)&&(cnt>0);i++) {
      if (isdigit(*ps)) {
        *pb=*ps;
        pb++;ps++;cnt--;
      } else break;
    }
    if (i!=4) goto cpderr;

    // skip separator
    if (!isdigit(*ps)) { if (*ps=='-') ps++; else goto cpderr; }
    // copy month (2 digits)
    for (i=0;(i<2)&&(*ps)&&(cnt>0);i++) {
      if (isdigit(*ps)) {
        *pb=*ps;
        pb++;ps++;cnt--;
      } else break;
    }
    if (i!=2) goto cpderr;

    // skip separator
    if (!isdigit(*ps)) { if (*ps=='-') ps++; else goto cpderr; }
    // copy day (2 digits)
    for (i=0;(i<2)&&(*ps)&&(cnt>0);i++) {
      if (isdigit(*ps)) {
        *pb=*ps;
        pb++;ps++;cnt--;
      } else break;
    }
    if (i!=2) goto cpderr;

    *pb='\0';
  }

  return ( ps );

cpderr : buffer[0]='\0'; return( s );

} // cpd

/*--------------------------------------------------------------------------*/
/* cpt: copy time from s to buffer, return pointer to next char in s        */
/*      In the case of an error the buffer contains an empty string and     */
/*      the pointer to the input string s is returned.                      */
/*      The string s can start with 'T' or space or with a digit            */
/*--------------------------------------------------------------------------*/
char * cpt( char buffer[], size_t buflen, char * s )
{ char *ps, *pb;
  size_t cnt;
  long i;

  ps = s;
  pb = buffer; *pb='\0';
  cnt = buflen-1;

  if ( s ) {
    // skip start character 'T' or white space
    if (!isdigit(*ps)) {
      if ((*ps=='T')||(*ps==' ')) ps++; else goto cpterr;
    }

    // copy hour (2 digits)
    for (i=0;(i<2)&&(*ps)&&(cnt>0);i++) {
      if (isdigit(*ps)) {
        *pb=*ps;
        pb++;ps++;cnt--;
      } else break;
    }
    if (i!=2) goto cpterr;

    // skip separator
    if (!isdigit(*ps)) { if (*ps==':') ps++; else goto cpterr; }
    // copy minute (2 digits)
    for (i=0;(i<2)&&(*ps)&&(cnt>0);i++) {
      if (isdigit(*ps)) {
        *pb=*ps;
        pb++;ps++;cnt--;
      } else break;
    }
    if (i!=2) goto cpterr;

    // skip separator
    if (!isdigit(*ps)) { if (*ps==':') ps++; else goto cpterr; }
    // copy second (2 digits)
    for (i=0;(i<2)&&(*ps)&&(cnt>0);i++) {
      if (isdigit(*ps)) {
        *pb=*ps;
        pb++;ps++;cnt--;
      } else break;
    }
    if (i!=2) goto cpterr;

    *pb='\0';
  }

  return ( ps );

cpterr : buffer[0]='\0'; return( s );

} // cpt

/*--------------------------------------------------------------------------*/
/* cpf: copy a fraction from s to buffer, return pointer to next char in s  */
/*      In the case of an error the buffer contains an empty string and     */
/*      the pointer to the input string s is returned.                      */
/*      The string s must start with '.' or ','.                            */
/*      The copying stops at the first non digit character                  */
/*--------------------------------------------------------------------------*/
char * cpf( char buffer[], size_t buflen, char * s )
{ char *ps, *pb;
  size_t cnt;
  long i;

  ps = s;
  pb = buffer; *pb='\0';
  cnt = buflen-1;

  if ( s ) {
    // check start character '.' or ','
    if ((*ps!='.')&&(*ps!=',')) goto cpferr;

    // copy start character (1 character)
    if ( (*ps)&&(cnt>0) ) {
      *pb='.';
      pb++;ps++;cnt--;
    }

    // copy fraction (undetermined length)
    for (i=0;(*ps)&&(cnt>0);i++) {
      if (isdigit(*ps)) {
        *pb=*ps;
        pb++;ps++;cnt--;
      } else break;
    }

    // skip the rest
    for (;(*ps);) {
      if (isdigit(*ps)) {
        ps++;
      } else break;
    }

    *pb='\0';
  }

  return ( ps );

cpferr : buffer[0]='\0'; return( s );

} // cpf

/*--------------------------------------------------------------------------*/
/* cps: copy integer part of seconds from s to buffer, return pointer to    */
/*      next char in s.                                                     */
/*      In the case of an error the buffer contains an empty string and     */
/*      the pointer to the input string s is returned.                      */
/*      The string s must start with '-', '+' or a digit.                   */
/*      The copying stops at the first non digit character after the start. */
/*--------------------------------------------------------------------------*/
char * cps( char buffer[], size_t buflen, char * s )
{ char *ps, *pb;
  size_t cnt;
  long i;

  ps = s;
  pb = buffer; *pb='\0';
  cnt = buflen-1;

  if ( s ) {
    // check start character '-' or '+'
    if ((*ps!='-')&&(*ps!='+')&&(!isdigit(*ps))) goto cpserr;

    // copy start character (1 character)
    if ( (*ps)&&(cnt>0) ) {
      *pb=*ps;
      pb++;ps++;cnt--;
    }

    // copy seconds (undetermined length)
    for (i=0;(*ps)&&(cnt>0);i++) {
      if (isdigit(*ps)) {
        *pb=*ps;
        pb++;ps++;cnt--;
      } else break;
    }

    // check that all digits are read
    if (isdigit(*ps)&&(strlen(ps))) goto cpserr;

    *pb='\0';
  }

  return ( ps );

cpserr : buffer[0]='\0'; return( s );

} // cps

/*--------------------------------------------------------------------------*/
/* cpo: copy an offset from s to buffer, return pointer to next char in s   */
/*      In the case of an error the buffer contains an empty string and     */
/*      the pointer to the input string s is returned.                      */
/*      The string s must start with '+' or '-'.                            */
/*      The string can contain ':' as separator.                            */
/*      The copying stops at the end of the line or after 7 copied digits.  */
/*--------------------------------------------------------------------------*/
char * cpo( char buffer[], size_t buflen, char * s )
{ char *ps, *pb;
  size_t cnt;
  long i;
  int stop=0;

  ps = s;
  pb = buffer; *pb='\0';
  cnt = buflen-1;

  if ( s ) {

    // skip leading white spaces
    while (isspace(*ps)) ps++;

    // check start character '-' or '+'
    if ((*ps!='-')&&(*ps!='+')&&(*ps!='Z')) goto cpoerr;

    // copy start character (1 character)
    if ( (*ps)&&(cnt>0) ) {
      *pb=*ps;
      pb++;ps++;cnt--;
    }

    if (pb[-1]=='Z') {
      pb[-1]='+'; pb[0]='0'; pb++; // UTC

    } else {
      // copy hour (2 digits)
      for (i=0;(i<2)&&(*ps)&&(cnt>0);i++) {
        if (isdigit(*ps)) {
          *pb=*ps;
          pb++;ps++;cnt--;
        } else break;
      }
      if (i!=2) { if (i==1) stop=1; else  goto cpoerr; }
  
      if (!stop) {
        // skip separator
        if (!isdigit(*ps)) { if (*ps==':') ps++; else stop=1; }
        if (isdigit(*ps)&&!stop) { // read only 2 character if *ps is a digit
          // copy minute (2 digits)
          for (i=0;(i<2)&&(*ps)&&(cnt>0);i++) {
            if (isdigit(*ps)) {
              *pb=*ps;
              pb++;ps++;cnt--;
            } else break;
          }
          if (i!=2) goto cpoerr;
        }
      }
  
      if (!stop) {
        // skip separator
        if (!isdigit(*ps)) { if (*ps==':') ps++; else stop=1; }
        if (isdigit(*ps)&&!stop) { // read only 2 character if *ps is a digit
          // copy second (2 digits)
          for (i=0;(i<2)&&(*ps)&&(cnt>0);i++) {
            if (isdigit(*ps)) {
              *pb=*ps;
              pb++;ps++;cnt--;
            } else break;
          }
          if (i!=2) goto cpoerr;
        }
      }
    }

    *pb='\0';
  }

  return ( ps );

cpoerr : buffer[0]='\0'; return( s );

} // cpo

/*--------------------------------------------------------------------------*/
/* _convert2epoch: Convert to IsotimeEpoch,                                 */
/* returns epoch.status!=0 in case of an error                              */
/*--------------------------------------------------------------------------*/
IsotimeEpoch _convert2epoch(long year, long month, long day, 
                            long hh, long mm, long ss, double uuuuuu, 
                            long osign, long Hh, long Mm, long Ss) 
{ long days, base;
  IsotimeEpoch epoch;

  if (ISOTIME_debug>0) fprintf(stderr,"_convert2epoch BEGIN\n");

  epoch.offset = (long int) 0;
  epoch.fract  = (double) 0.0;
  epoch.sec    = (long int) 0;

  epoch.status = -1;

  days = year*365+leap_days(year) - DAYS_19700101;

  switch (month) {
    case  1: days+=day; break;
    case  2: days+=day+31; break;
    case  3: days+=day+59; break;
    case  4: days+=day+90; break;
    case  5: days+=day+120; break;
    case  6: days+=day+151; break;
    case  7: days+=day+181; break;
    case  8: days+=day+212; break;
    case  9: days+=day+243; break;
    case 10: days+=day+273; break;
    case 11: days+=day+304; break;
    case 12: days+=day+334; break;
    default : month=0; // invalid
  }

  if ( month ) {

    // correct march to december leap years
    if ( leap_year ( year ) ) {
      if (month>2) days+=1;
    }

    epoch.offset = osign*( (Hh*60+Mm)*60+Ss );

    base = floor(uuuuuu);
    epoch.fract = uuuuuu - (double) base;
    epoch.sec   = ((days*24+hh)*60+mm)*60+ss+base-epoch.offset;

    // normalize fract and sec
    base = floor(epoch.fract);
    epoch.fract -= (double) base;
    epoch.sec   += base;

    epoch.status = 0; // success
  } 

  if (ISOTIME_debug>2)
    fprintf(stderr,"  %ld (DAYS_%04ld%02ld%02ld) - %ld (DAYS_19700101) = %ld\n",
      days+DAYS_19700101,year,month,day,DAYS_19700101,days);

  if (ISOTIME_debug>0) fprintf(stderr,"_convert2epoch END\n");

  return ( epoch );

} // _convert2epoch

/*--------------------------------------------------------------------------*/
/* Scans isotime_s and converts it to epoch, .status!=0 in case of an error.*/
/* The input string must describe a full date and time in seconds.          */
/* returns epoch, epoch.status!=0 in case of an error                       */
/*--------------------------------------------------------------------------*/
IsotimeEpoch _isotime2epoch(const char * isotime_s) 
{ char osign_c;
  long osign;
  long year=0, month=0, day=0, hh=0, mm=0, ss=0;
  long Hh=0, Mm=0, Ss=0;
  double uuuuuu=0.0;
  char trimmed[TRIMLEN], *pd, *pt, *pf, *po, *pr;
  char datbuf[DLEN];
  char timbuf[TLEN];
  char frabuf[FLEN];
  char offbuf[OLEN];
  IsotimeEpoch epoch;

  if (ISOTIME_debug>0) fprintf(stderr,"_isotime2epoch >>%s<< BEGIN\n",
    isotime_s);

  // trim isotime_s, convert to uppercase and copy to trimmed
  pd = trim( trimmed, TRIMLEN, isotime_s );
  if (ISOTIME_debug>1) fprintf(stderr," trim returns >>%s<<\n",pd);

  // copy date
  pt = cpd( datbuf, DLEN, pd );
  if (ISOTIME_debug>1) fprintf(stderr," date >>%s<<\n",datbuf);

  // copy time
  pf = cpt( timbuf, TLEN, pt );
  if (ISOTIME_debug>1) fprintf(stderr," time >>%s<<\n",timbuf);

  // copy fraction
  po = cpf( frabuf, FLEN, pf );
  if (ISOTIME_debug>1) fprintf(stderr," fraction >>%s<<\n",frabuf);

  // copy offset
  pr = cpo( offbuf, OLEN, po );
  if (ISOTIME_debug>1) fprintf(stderr," offset >>%s<<\n",offbuf);
  
  sscanf(datbuf,"%4ld%2ld%2ld",&year,&month,&day);
  sscanf(timbuf,"%2ld%2ld%2ld",&hh,&mm,&ss);
  sscanf(frabuf,"%lf",&uuuuuu);
  sscanf(offbuf,"%c%2ld%2ld%2ld",&osign_c,&Hh, &Mm, &Ss);

  if ( strlen(pr)||(day==0) ) month=0; // error, if rest is not empty or day 0

  osign = (osign_c=='-')?-1:+1;
  epoch =  _convert2epoch(year,month,day,hh,mm,ss,uuuuuu,osign,Hh,Mm,Ss);

  if ( epoch.status ) {
    fprintf( stderr, "ERROR: Cannot read time \"%s\"\n",trimmed );
    fprintf( stderr, "       Format: YYYY-MM-DDThh:mm:ss[.uuuuuu][+Hh:Mm]\n" );
  }

  if (ISOTIME_debug>0) fprintf(stderr,"_isotime2epoch >>%s<< END\n",
    isotime_s);

  return( epoch );

} // _isotime2epoch

/*--------------------------------------------------------------------------*/
/* Scans epoch string and converts it to epoch. The input string has the    */
/* format <seconds>[.<fraction>].                                           */
/* Returns epoch, epoch.status!=0 in case of an error                       */
/*                                                                          */
/* Positive and negative times equal to epoch.sec + epoch.fract.            */
/* The fraction is normalized to 0.0 <= epoch.fract < 1.0                   */
/*                                                                          */
/* Example:                                                                 */
/* time  = floor(time) + time-floor(time)                                   */
/*       = time.sec    + time.fract                                         */
/*  10.7 = 10         + 0.7                                                 */
/* -10.7 = -11        + 0.3                                                 */
/*--------------------------------------------------------------------------*/
IsotimeEpoch _string2epoch(const char *epoch_s)
{ long base;
  IsotimeEpoch epoch;
  char epobuf[EPOLEN], *pr;
  char secbuf[SLEN], *ps;
  char frabuf[SLEN], *pf;

  long sec=0;
  double fract=0.0;

  if (ISOTIME_debug>0) fprintf(stderr,"_string2epoch >>%s<<\n",epoch_s);

  epoch.status = -1;
  epoch.sec    = 0;
  epoch.fract  = 0.0;
  epoch.offset = 0;

  // trim isotime_s, convert to uppercase and copy to trimmed
  ps = trim( epobuf, EPOLEN, epoch_s );
  if (ISOTIME_debug>1) fprintf(stderr," trim returns >>%s<<\n",ps);

  // copy seconds
  pf = cps( secbuf, SLEN, ps );
  if (ISOTIME_debug>1) fprintf(stderr," section >>%s<<\n",secbuf);

  // copy fraction
  pr = cpf( frabuf, SLEN, pf );
  if (ISOTIME_debug>1) fprintf(stderr," fraction >>%s<<\n",frabuf);

  if ( strlen(pr) ) { 
    // error, if rest is not empty
    fprintf( stderr, "ERROR: Cannot read epoch \"%s\"\n",epobuf );
    fprintf( stderr, "       Format: [+|-]sssssssssss[.uuuuuuu]\n" );
    fprintf( stderr, "               e.g. \"1149254287\", \"+1149254287.1\"\n");
    goto _string2epocherr;
  }

  sscanf(secbuf,"%ld",&sec);
  sscanf(frabuf,"%lf",&fract);

  epoch.sec = sec;
  if (sec>=0) epoch.fract = fract;
  epoch.fract = -fract;

  // normalize fract and sec
  base = floor(epoch.fract);
  epoch.fract -= (double) base;
  epoch.sec   += base;

  epoch.status = 0;

  if (ISOTIME_debug>0) fprintf(stderr,"_string2epoch END\n");

  return ( epoch );

_string2epocherr:
  if (ISOTIME_debug>0) fprintf(stderr,"_string2epoch END\n");
  return ( epoch );

} // _string2epoch

/*+++------------------------------------------------------------------------
NAME
  string2epoch --- Convert epoch string to epoch

SYNOPSIS

  IsotimeEpoch string2epoch(const char *string);

DESCRIPTION

  Scans epoch string and converts it to epoch. The input string has the
  format <seconds>[.<fraction>].

RETURN VALUE

  Returns epoch, epoch.status!=0 in case of an error

----------------------------------------------------------------------------*/
IsotimeEpoch string2epoch(const char *string)
{
  return ( _string2epoch( string ) );
} // string2epoch

/*+++------------------------------------------------------------------------
NAME
  addoffset2epoch --- Relative change of the time zone 

SYNOPSIS

  IsotimeEpoch addoffset2epoch( IsotimeEpoch epoch, const char * offset_s );

DESCRIPTION

  Changing the time zone relatively by incrementing the offset 
  and decrementing epoch.

RETURN VALUE

  updated epoch

----------------------------------------------------------------------------*/
IsotimeEpoch addoffset2epoch( IsotimeEpoch epoch, const char * offset_s )
{ char osign_c;
  char trimmed[TRIMLEN], *po, *pr;
  char offbuf[OLEN];
  long offset=0, Hh=0, Mm=0, Ss=0;

  if (ISOTIME_debug>0) 
    fprintf(stderr,"addoffset2epoch >>%s<< BEGIN\n",offset_s);

  if ( epoch.status ) goto offerr;

  // trim offset_s, convert to uppercase and copy to trimmed
  po = trim( trimmed, TRIMLEN, offset_s );
  if (ISOTIME_debug>1) fprintf(stderr," trim returns >>%s<<\n",po);

  // copy offset
  pr = cpo( offbuf, OLEN, po );
  if (ISOTIME_debug>1) fprintf(stderr," offset >>%s<<\n",offbuf);
 
  if ( strlen(pr) ) { 
    epoch.status=-1; 
    fprintf( stderr, "ERROR: Cannot read offset \"%s\"\n",trimmed );
    fprintf( stderr, "       Format: +HhMm | -HhMm, e.g. \"+0200\"\n" );
    goto offerr; 
  }

  if ( !epoch.status ) { 
    sscanf(offbuf,"%c%2ld%2ld%2ld",&osign_c,&Hh, &Mm, &Ss);
    offset = (Hh*60+Mm)*60+Ss;
    if (osign_c=='-') offset*=-1;
    epoch.offset += offset;
    epoch.sec    -= offset;
  }

  if (ISOTIME_debug>0) fprintf(stderr,"addoffset2epoch END\n");

  return ( epoch );

offerr: if (ISOTIME_debug>0) fprintf(stderr,"addoffset2epoch END\n");
        return ( epoch );

} // addoffset2epoch 

/*+++------------------------------------------------------------------------
NAME
  setoffset2epoch --- Set time zone

SYNOPSIS

  IsotimeEpoch setoffset2epoch( IsotimeEpoch epoch, const char * offset_s );

DESCRIPTION

  Setting the time zone offset.

RETURN VALUE

  updated epoch

----------------------------------------------------------------------------*/
IsotimeEpoch setoffset2epoch( IsotimeEpoch epoch, const char * offset_s )
{ char osign_c;
  char trimmed[TRIMLEN], *po, *pr;
  char offbuf[OLEN];
  long offset=0, Hh=0, Mm=0, Ss=0;

  if (ISOTIME_debug>0) fprintf(stderr,"setoffset2epoch BEGIN\n");

  if ( epoch.status ) goto offerr;

  // trim offset_s, convert to uppercase and copy to trimmed
  po = trim( trimmed, TRIMLEN, offset_s );
  if (ISOTIME_debug>1) fprintf(stderr," trim returns >>%s<<\n",po);

  // copy offset
  pr = cpo( offbuf, OLEN, po );
  if (ISOTIME_debug>1) fprintf(stderr," offset >>%s<<\n",offbuf);

  if ( strlen(pr) ) {
    epoch.status=-1;
    fprintf( stderr, "ERROR: Cannot read offset \"%s\"\n",trimmed );
    fprintf( stderr, "       Format: +HhMm | -HhMm, e.g. \"+0200\"\n" );
    goto offerr;
  }

  if ( !epoch.status ) {
    sscanf(offbuf,"%c%2ld%2ld%2ld",&osign_c,&Hh, &Mm, &Ss);
    offset = (Hh*60+Mm)*60+Ss;
    if (osign_c=='-') offset *= -1;
    epoch.offset = offset;
  }

  if (ISOTIME_debug>0) fprintf(stderr,"setoffset2epoch END\n");

  return ( epoch );

offerr: if (ISOTIME_debug>0) fprintf(stderr,"setoffset2epoch END\n");
        return ( epoch );

} // setoffset2epoch

/*+++------------------------------------------------------------------------
NAME
  shiftepoch --- shifts epoch by shift 

SYNOPSIS

  IsotimeEpoch shiftepoch(IsotimeEpoch epoch, IsotimeEpoch shift);

DESCRIPTION

  Adds shift to epoch. shift.offset is ignored.

RETURN VALUE

  updated epoch

----------------------------------------------------------------------------*/
IsotimeEpoch shiftepoch(IsotimeEpoch epoch, IsotimeEpoch shift)
{ long base;
  char epobuf[EPOLEN];

  if (ISOTIME_debug>0) fprintf(stderr,"shiftepoch BEGIN\n");

  if ( epoch.status ) goto shifterr;
  if ( shift.status ) goto shifterr;

  if (ISOTIME_debug>1) {
    fprintf( stderr,"epoch >>%s<<, ", epoch2string(epobuf,EPOLEN,epoch));
    fprintf( stderr,"shift >>%s<<\n", epoch2string(epobuf,EPOLEN,shift));
  }

  epoch.sec   += shift.sec;
  epoch.fract += shift.fract;

  // normalize fract and sec
  base = (long) floor(epoch.fract);
  epoch.fract -= (double) base;
  epoch.sec   += base;

  if (ISOTIME_debug>1) {
    fprintf( stderr,"result >>%s<<, ", epoch2string(epobuf,EPOLEN,epoch));
  }

  if (ISOTIME_debug>0) fprintf(stderr,"shiftepoch END\n");

  return ( epoch );

shifterr: if (ISOTIME_debug>0) fprintf(stderr,"shiftepoch END\n");
  epoch.status = -1;
  return ( epoch );

} // shiftepoch

/*+++------------------------------------------------------------------------
NAME
  isotime2epoch --- convert isotime string to IsotimeEpoch

SYNOPSIS

  IsotimeEpoch isotime2epoch( const char * isotime_s )

DESCRIPTION

  Scans isotime_s and converts it to epoch, .status!=0 in case of an error.
  The input string must describe a full date and time in seconds.

  Supported input formats:

   123456789A123456789B123456789C12345

   YYYY-MM-DD hh:mm:ss.uuuuuu+Hh:Mm:Ss (space is not ISO standard, but used)
   YYYY-MM-DD hh:mm:ss.uuuuuu-Hh:Mm:Ss (space is not ISO standard, but used)
   YYYY-MM-DDThh:mm:ss.uuuuuu+Hh:Mm:Ss
   YYYY-MM-DDThh:mm:ss.uuuuuu-Hh:Mm:Ss
   YYYY-MM-DD hh:mm:ss.uuuuuu          (space is not ISO standard, but used)
   YYYY-MM-DDThh:mm:ss.uuuuuu
   YYYY-MM-DD hh:mm:ss                 (space is not ISO standard, but used)
   YYYY-MM-DDThh:mm:ss

   short forms

   YYYYMMDDThhmmss.uuuuuu+HhMmSs
   YYYYMMDDhhmmss.uuuuuu+HhMmSs
   YYYYMMDDThhmmss+HhMmSs
   YYYYMMDDhhmmss+HhMmSs
   YYYYMMDDThhmmss.uuuuuu
   YYYYMMDDhhmmss.uuuuuu
   YYYYMMDDThhmmss
   YYYYMMDDhhmmss

   YYYYMMDD date (8 bytes)
   hhmmss   time (6 bytes)
   .uuuuuu   fraction (not limited)
   +HhMmSs   offset (7 bytes)

   The UTC time offset (+00:00) can be abbreviated with Z

RETURN VALUE

  .status==0 : success, the returned value contains epoch
  .status<0  : error

----------------------------------------------------------------------------*/
IsotimeEpoch isotime2epoch( const char * isotime_s )
{
  if (ISOTIME_debug>0) fprintf(stderr,"isotime2epoch\n");
  return ( _isotime2epoch(isotime_s) );
} // isotime2epoch

/*+++------------------------------------------------------------------------
NAME
  epoch2isotime --- convert IsotimeEpoch to isotime string

SYNOPSIS

  const char * epoch2isotime( char buffer[], size_t buflen, IsotimeEpoch epoch )

DESCRIPTION

  (not implemented)

RETURN VALUE

  pointer to isotime string in buffer

----------------------------------------------------------------------------*/
const char * epoch2isotime( char buffer[], size_t buflen, IsotimeEpoch epoch )
{ char epobuf[EPOLEN]; 
  char osign_c;
  long year=0, month=0, day=0;
  long hh=0, mm=0, ss=0;
  double fract=0.0;
  long Hh=0, Mm=0, Ss=0;

  long days, yd, ts, tz, leap;

  long base;

  if (ISOTIME_debug>0) fprintf(stderr,"epoch2isotime %s BEGIN\n",
     epoch2string( epobuf, EPOLEN, epoch ) );

  if ( (!buffer)||(buflen<ISOLEN) ) {
    if (ISOTIME_debug>0) fprintf(stderr,"epoch2isotime END\n");
    return ( ( const char *) NULL );
  }

  buffer[0] ='\0';

  if (!epoch.status) {
    // normalize fract and sec
    base = floor(epoch.fract);
    epoch.fract -= (double) base;
    epoch.sec   += base;

    // convert2time
    fract = epoch.fract;

    if (epoch.offset<0) osign_c='-'; else osign_c='+';
    tz    = epoch.offset; 
    Hh    = floor(tz/3600);
    tz   -= Hh*3600;
    Mm    = floor(tz/60);
    tz   -= Mm*60;
    Ss    = tz;

    Hh=labs(Hh); Mm=labs(Mm); Ss = labs(Ss);

    if (ISOTIME_debug>3)
      fprintf(stderr," osign=%c1, Hh=%ld, Mm=%ld, Ss=%ld\n",osign_c,Hh,Mm,Ss);

    ts    = epoch.sec + epoch.offset; // add time zone offset

    days  = floor( ts/3600/24 );

    ts   -= days*3600*24;

    if (ts<0) { days-=1; ts+=3600*24; }

    hh    = floor(ts/3600);
    ts   -= hh*3600;
    mm    = floor(ts/60);
    ts   -= mm*60;
    ss    = ts;

    days += DAYS_19700101; // == year*365+leap_days

    year  = floor( days/365 );
    yd    = days - year*365 - leap_days( year ); // day in the year

    if (ISOTIME_debug>3)
      fprintf(stderr," hh=%ld, hm=%ld, ss=%ld\n",hh,mm,ss);

    // days contains all leap_days => year is correct or too high 
    while ( yd <= 0 ) {
      year--;
      yd    = days - year*365 - leap_days( year ); // day in the year
    }

    if (ISOTIME_debug>3)
      fprintf(stderr," year=%ld, yd=%ld\n",year,yd);

    // get number of leap days in the year
    if ( leap_year ( year ) ) leap=1; else leap=0;

         if (yd<=31)       { month= 1; day=yd; }          // Jan
    else if (yd<=59+leap)  { month= 2; day=yd-31; }       // Feb 
    else if (yd<=90+leap)  { month= 3; day=yd-59-leap; }  // Mar
    else if (yd<=120+leap) { month= 4; day=yd-90-leap; }  // Apr
    else if (yd<=151+leap) { month= 5; day=yd-120-leap; } // May
    else if (yd<=181+leap) { month= 6; day=yd-151-leap; } // Jun
    else if (yd<=212+leap) { month= 7; day=yd-181-leap; } // Jul
    else if (yd<=243+leap) { month= 8; day=yd-212-leap; } // Aug
    else if (yd<=273+leap) { month= 9; day=yd-243-leap; } // Sep
    else if (yd<=304+leap) { month=10; day=yd-273-leap; } // Oct
    else if (yd<=334+leap) { month=11; day=yd-304-leap; } // Nov
    else                   { month=12; day=yd-334-leap; } // Dec

    // print
    switch (ISOTIME_mode) {
      case IsotimeModeSpace:
        sprintf(buffer,"%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %c%02ld%02ld",
                year,month,day,hh,mm,ss,(long) floor(epoch.fract*1e6+0.5),
                osign_c,labs(Hh),labs(Mm) );
        break;
      default: // IsotimeModeNoSpace
        sprintf(buffer,"%04ld-%02ld-%02ldT%02ld:%02ld:%02ld.%06ld%c%02ld%02ld",
             year,month,day,hh,mm,ss,(long) floor(epoch.fract*1e6+0.5),
             osign_c,labs(Hh),labs(Mm) );
    }
  }

  if (ISOTIME_debug>0) fprintf(stderr,"epoch2isotime %s END\n",buffer);

  return( buffer );

} // epoch2isotime

/*+++------------------------------------------------------------------------
NAME
  epoch2string --- write epoch to string

SYNOPSIS

  const char *epoch2string ( char buffer[], size_t buflen, IsotimeEpoch epoch );

DESCRIPTION


RETURN VALUE

   pointer to output string in buffer

----------------------------------------------------------------------------*/
const char * epoch2string ( char buffer[], size_t buflen, IsotimeEpoch epoch )
{ 
  long base;

  if ( (!buffer)||(buflen<EPOLEN) ) {
    return ( ( const char *) NULL );
  }

  if (!epoch.status) {
    // normalize fract and sec
    base = floor(epoch.fract);
    epoch.fract -= (double) base;
    epoch.sec   += base;
    if (epoch.sec>=0)
      sprintf( buffer," %ld.%06ld",epoch.sec,(long) floor(epoch.fract*1e6+0.5));
    else { // invert sign
      epoch.fract = -epoch.fract;
      epoch.sec   = -epoch.sec;
      // normalize fract and sec
      base = floor(epoch.fract);
      epoch.fract -= (double) base;
      epoch.sec   += base;
      sprintf( buffer,"-%ld.%06ld",epoch.sec,(long) floor(epoch.fract*1e6+0.5));
    }
  }

  return( buffer );

} // epoch2string

/*+++------------------------------------------------------------------------
NAME
  isotime_mode2str --- return isotime mode as string 

SYNOPSIS

  void isotime_mode2str( int mode );

DESCRIPTION
  int mode : isotime mode

RETURN VALUE

  const char* : isotime mode string

----------------------------------------------------------------------------*/
const char *isotime_mode2str ( int mode )
{ switch (mode ) {
    case IsotimeModeNoSpace: return("nospace");
    case IsotimeModeSpace: return("space");
  }
  return("invalid"); 
} /* isotime_mode2str */

/*+++------------------------------------------------------------------------
NAME
  isotime_str2mode --- return input string as isotime mode

SYNOPSIS

  int isotime_str2mode ( const char *mode_s )

DESCRIPTION
  const char *mode_s : isotime mode string

RETURN VALUE

  const char*

----------------------------------------------------------------------------*/
int isotime_str2mode ( const char *mode_s )
{ int mode = IsotimeModeInvalid;
  char *str;

  str = strlib_newstr( mode_s );

  if (str) {
    strlib_trim ( str );
    strlib_tolower ( str );

    if (!strcmp(str,"nospace")) mode = IsotimeModeNoSpace;
    else if (!strcmp(str,"space")) mode = IsotimeModeSpace;
    else mode = IsotimeModeInvalid;

    free(str);
  }

  return(mode);

} // isotime_str2mode

/*+++------------------------------------------------------------------------
NAME
  isotime_version --- return version

SYNOPSIS

  char *isotime_version ( void )

DESCRIPTION


RETURN VALUE

   pointer to the version string

----------------------------------------------------------------------------*/
const char *isotime_version ( void )
{ return ( ISOTIME_VERSION );
} /* isotime_version */

/*+++------------------------------------------------------------------------
NAME
  isotime_mode --- set isotime mode

SYNOPSIS

  void isotime_mode ( int mode );

DESCRIPTION
  void isotime_mode : selects IsotimeMode

RETURN VALUE

   void

----------------------------------------------------------------------------*/
void isotime_mode ( int mode )
{  if (ISOTIME_debug>0) fprintf(stderr,"isotime_mode BEGIN\n");
  ISOTIME_mode = mode;
  if (ISOTIME_debug>1) fprintf(stderr," ISOTIME_mode >>%s<<\n",
                         isotime_mode2str(ISOTIME_mode));
  if (ISOTIME_debug>0) fprintf(stderr,"isotime_mode END\n");
} /* isotime_mode */

/*+++------------------------------------------------------------------------
NAME 
  isotime_debug --- set debug mode
     
SYNOPSIS
    
  void isotime_debug ( int debug );

DESCRIPTION
  void isotime_debug : selects debug mode 0|1

   0: off, 1: on 
     
RETURN VALUE

   void 

----------------------------------------------------------------------------*/
void isotime_debug ( int debug )
{ ISOTIME_debug = debug;
} /* isotime_debug */

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

/*+++
NAME

   gamma.c --- Gamma Function

SYNOPSIS

   double gamma( double X );

HISTORY
  2000-11-28 V1.0 Peter Boesecke
                  translated from PEARL function GAMMA PB 19-APR-1988
                  from COLLECTED ALGORITHMS FROM CACM 31-P 1- 0
                  ALGORITHM 31 (in ALGOL )
  2000-12-01 V1.01 calculation corrected
  2000-12-04 V1.02 -> gamma.c, .h
  2000-12-06 V1.03 Stirling's formula for x>20
---*/

/****************************************************************************
*  Include                                                                  *
****************************************************************************/

# include "gamma.h"
# define GAMMA_PI        3.1415926535897932384626

double _loggamma( double x ) 
{ // by Stirling's formula Knuth I: 111
  double invx, invx2, invx3, invx5, invx7;
  double sum=0.0;

  x -= 1.0;
  if (x <= 1.0) return( 1.0 );

  invx =  1.0 / x;
  invx2 = invx * invx;
  invx3 = invx2 * invx;
  invx5 = invx3 * invx2;
  invx7 = invx5 * invx2;

  sum = ((x + 0.5) * log(x)) - x;
  sum += log(2*GAMMA_PI) * 0.5;
  sum += (invx / 12.0) - (invx3 / 360.0);
  sum += (invx5 / 1260.0) - (invx7 / 1680.0);

  return ( sum );

} /* _loggamma */


/*+++------------------------------------------------------------------------
NAME
   gamma --- gamma function 

SYNOPSIS

   double gamma( double X );

DESCRIPTION

  For 2<=x<=3 gamma is approximated. In this interval the absolut
  error abs(eps(x))<0.25*1e-7. For x>3 gamma(x) is calculated 
  by iteration gamma(x) = (x-1) * (x-2) * ... * (x-n)*gamma(x-n),
  with 2<=(x-n)<=3. For x<2 gamma(x) = gamma(x+n) / ( x*(x+1)...(x+n-1) )
  again with 2<=(x-n)<=3. For x=0 or a negative integer gamma(x) is
  set to DBL_MAX.

RETURN VALUE
 
  gamma(x)

----------------------------------------------------------------------------*/
double gamma( double X ) 
{
  const double EPSMIN = 1e-30;
  const double EPS    = 1e-6;
  const double DUMVAL = DBL_MAX; 
  const double a0=.9999999758, a1=.4227874605, a2=.4117741955, a3=.0821117404;
  const double a4=.0721101567, a5=.0044511400, a6=.0051589951, a7=.0016063118;
  double GAMMA_WERT;
  double H,Y;

  H = 1.0; Y = X;

  while ( (fabs(Y-2.0)) >= EPS ) {
    if ( fabs(Y)<EPSMIN ) { H=DUMVAL; break; }
      else if (Y<2.0) { H = H/Y; Y=Y+1.0; }
      else if (Y>=20.0) { H = exp(_loggamma( Y )); break; }
      else if (Y>=3.0) { Y = Y-1.0;H=H*Y; } 
      else { Y=Y-2.0;
             H = (((((((a7*Y+a6)*Y+a5)*Y+a4)*Y+a3)*Y+a2)*Y+a1)*Y+a0)*H;
             break; }
    }
  GAMMA_WERT = H;

  return ( GAMMA_WERT );

} /* gamma */

/*+++------------------------------------------------------------------------
NAME
   loggamma --- natural logarithm of gamma function

SYNOPSIS

   double loggamma( double X );

DESCRIPTION

  Calculation of the natural logarithm of gamma for positive X.
  For X==0 or negative X DBL_MAX is returned.

RETURN VALUE

  loggamma(x)

----------------------------------------------------------------------------*/
double loggamma( double X )
{
  const double EPSMIN = 1e-30;
  const double EPS    = 1e-6;
  const double DUMVAL = DBL_MAX;
  const double a0=.9999999758, a1=.4227874605, a2=.4117741955, a3=.0821117404;
  const double a4=.0721101567, a5=.0044511400, a6=.0051589951, a7=.0016063118;
  double LOGGAMMA_WERT;
  double logH, Y;

  logH = 0.0; Y = X;

  while ( (fabs(Y-2.0)) >= EPS ) {
    if ( Y<EPSMIN ) { logH=DUMVAL; break; }
      else if (Y<2.0) { logH = logH-log(Y); Y=Y+1.0; }
      else if (Y>=20.0) { logH = _loggamma( Y ); break; }
      else if (Y>=3.0) { Y = Y-1.0;logH=logH+log(Y); }
      else { Y=Y-2.0;
             logH = logH + log(((((((a7*Y+a6)*Y+a5)*Y+a4)*Y+a3)*Y+a2)*Y+a1)*Y+a0);
             break; }
    }
  LOGGAMMA_WERT = logH;

  return ( LOGGAMMA_WERT );

} /* loggamma */


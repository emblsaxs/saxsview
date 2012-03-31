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

# define POISSON_VERSION      "poisson : V1.1 Peter Boesecke 2007-02-21"
/*+++------------------------------------------------------------------------
NAME
   gauss --- routines for poissonian distributions

SYNOPSIS

   # include poisson.h

HISTORY
  2000-11-17 V0.0 Peter Boesecke creation, problem in SumPoisson20
                  All routines to approximate SumPoisson taken from
                  http://www.io.com/~ritter/JAVASCRP/BINOMPOI.HTM
  2000-11-18 V0.1 PB InvSumPoisson loop improved
  2000-11-18 V0.2 SumPoisson works now correctly 
  2000-11-22 V1.0 Generates now correct mean and sigma
  2007-02-21 V1.1 POISSON_VERSION updated and pi defined as constant. 
                  SaxsDefinition.h is not needed any more
----------------------------------------------------------------------------*/
# include "poisson.h"

#ifndef ROUND
# define ROUND( x ) floor( ( x ) + 0.5 )
#endif
#ifndef MAX2
# define MAX2( x, y ) (( x ) < ( y ))?( y ):( x )
#endif


long fac( long x )  // x!
{ 
  long t = 1l;
  while (x > 1l)
    t *= x--;
  return( t );
} /* fac */

double logfac( long x ) // log(x!)
{ // by Stirling's formula Knuth I: 111
  const double pi = 3.1415926535897932384626;
  double invx, invx2, invx3, invx5, invx7;
  double sum;

  if (x <= 1l)  x = 1l;

  if (x < 12l)
    return log( fac( x ) );
   else {
    invx = (double) 1.0 / (double) x;
    invx2 = invx * invx;
    invx3 = invx2 * invx;
    invx5 = invx3 * invx2;
    invx7 = invx5 * invx2;

    sum = ((x + 0.5) * log(x)) - x;
    sum += log(2*pi) * 0.5;
    sum += (invx / 12.0) - (invx3 / 360.0);
    sum += (invx5 / 1260.0) - (invx7 / 1680.0);

    return ( sum );
    }
} /* logfac */

double g( double x ) 
{ // Peizer & Pratt 1968, JASA 63: 1416-1456
  const double eps = 1e-10;
  const double  switchlev = 0.1;
  double z, d, di;
  long i;

  if (x == 0)  
    z = 1;
   else 
    if (fabs(x-1.0)<eps) z = 0;
     else {
      d = 1.0 - x;

      if (fabs(d) > switchlev)
        z = (1.0 - (x * x) + (2.0 * x * log(x))) / (d * d);
       else {
        z = d / 3.0;  // first term
        di = d;  // d**1

        for (i = 2l; i <= 7l; i++) {
          di *= d;  // d**i
          z += (2.0 * di) / ((i+1) * (i+2));
          }
        }
      }

  return ( z );

} /* g */

double IntGauss1( double x ) 
{ // Abramowitz & Stegun 26.2.19
  double 
    d1 = 0.0498673470,
    d2 = 0.0211410061,
    d3 = 0.0032776263,
    d4 = 0.0000380036,
    d5 = 0.0000488906,
    d6 = 0.0000053830;
  double a,t;

  a = fabs(x),

  t = 1.0 + a*(d1+a*(d2+a*(d3+a*(d4+a*(d5+a*d6)))));

  // to 16th power
  t *= t;  t *= t;  t *= t;  t *= t;
  t = 1.0 / (t+t);  // the MINUS 16th

  if (x >= 0)  t = 1-t;
  return( t );

} /* IntGauss1 */

double SumPoisson20( long k, double u ) // Integral(0,k,Poisson(k,u)) for k>20
{ // Peizer & Pratt 1968, JASA 63: 1416-1456
 
  double s;
  double d1, d2;
  double z;

  s = (double) k + (double) (1.0/2.0); 

  d1 = (double) k - u + (double) (2.0/3.0);
  d2 = d1 + (double) 0.02/(double) (k+1l);

  z = (1.0 + g(s/u)) / u;
  z = d2 * sqrt(z);
  z = IntGauss1( z );

  return( z );

} /* SumPoisson20 */

double Poisson( long k, double ny )   // poisson distribution
{
  double value = 1.0;
  long i;
  double logsum = 0.0;

  for (i=1;i<=k;i++) {
   logsum += log(ny/i);
   }

  value *= exp(-ny+logsum);

  return ( value );

} /* Poisson */

double Poisson1( long k, double ny )   // poisson distribution Sterling
{ 
  return ( exp(-ny+log(ny)*k-logfac(k)) );
} /* Poisson */

double SumPoisson( long k, double ny ) // Sum(0,k,Poisson(k,ny))
/* cumulative sum of the poisson distribution */
{ 
  double sum;
  long j;

  if (k >= 20) 
    sum = SumPoisson20( k, ny );
   else 
  {
    sum = 0.0; j = 0;
    while (j <= k)
      if (j<12) sum += Poisson( j++, ny ); else sum += Poisson1( j++, ny );
    if (sum > 1.0) sum = 1.0;
  }
  return( sum );
} /* SumPoisson */

double IntPoisson( double k, double ny ) // Integral(0,k,Poisson(k,ny))
{ // interpolation of SumPoisson
  long k1, k2;
  double y1;
  double value;

  k1    = floor(k); k2 = k1+1;
  y1    = SumPoisson( k1 , ny );
  value =  y1 + Poisson( k2 , ny ) * (k-k1);

  return( value );

} /* IntPoisson */

long InvSumPoisson ( double y, double ny ) // Inverted SumPoisson
{ // Newton tangential approximation
  const int imax = 200;
  const double diffeps = 1e-14;
  const double amin = 1e-16;
  double k, kold;
  double yn;
  double a, b;
  double eps;
  int i=0;

  if (ny<1e-6) return( 0.0 );

  if (ny>1) eps = sqrt(ny)/10.0;
   else eps = ny/10.0;

  k = ny;
  for (i=1;i<imax;i++) {
    yn = IntPoisson( k, ny );

    if ( ny < 100 ) a = Poisson( ceil(k), ny );
      else a = Poisson( floor(k), ny );


    if (fabs(a)<amin) break;
    if ((a>0) && ( 1.0-yn < diffeps )) break;
    if ((a<0) && (     yn < diffeps )) break;

    b = yn-a*k;
    kold =k; k = (y - b)/a; if (k<0) k=0;

    if (fabs(k-kold)<eps) break;
     
    }

  return( ceil(k) );

} /* InvSumPoisson */

void PoissonNoiseSeed( unsigned int seed ) // set random number seed
{  srand( seed );
} /* PoissonNoiseSeed */

long PoissonNoise( double ny ) // create poissonian distributed noise
{
  int rannum = rand();
  double p;
  long value;

  /* create random numbers between 0 and 1 */
  p = rannum/(RAND_MAX+1.0);

  /* project the range 0 to 1 to the x-range of the poisson distribution */
  value =  InvSumPoisson ( p, ny );
  return ( value );

} /* PoissonNoise */

double RandomNoise( void ) // create random noise between 0.0 and 1.0 
{
  int rannum = rand();
  double value;

  /* create random numbers between 0 and 1 */
  value = rannum/(RAND_MAX+1.0);

  return ( value );

} /* PoissonNoise */


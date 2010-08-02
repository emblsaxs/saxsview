/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 2000-2010 European Synchrotron Radiation Facility
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

# define GAUSS_VERSION      "gauss : V1.2 Peter Boesecke 2007-04-23"
/*+++------------------------------------------------------------------------
NAME
   gauss --- routines for gaussian distributions 

SYNOPSIS

   # include gauss.h

----------------------------------------------------------------------------*/

# define GAUSS_LEN 201  /* number of interpolation points */
# define GAUSS_RANGE 8  /* range of LUT in multiples of sigma */

# include "gauss.h"

static int    GaussDebugMode = 0;
static double GaussLut_X[GAUSS_LEN];
static double GaussLut_Y[GAUSS_LEN];
static int GaussLutInit = 0;
static double GaussS2Pi = 2.506628274631;    // sqrt(2.0*SAXS_PI);

void GaussDebug( int mode )
{ GaussDebugMode = mode; }

double GaussPhi( double X )
{ return( exp ( - X*X*0.5 ) );
} /* GausPhi */

/* initializes GaussLut with the integrated values Y of a Gaussian
   GaussPhi( X ) = exp(-Y^2/2), Y = Integral(0,X,GaussPhi(X) */
void GaussInit( void )
{ const int N = GAUSS_LEN-1;

  int i;

  double X1, X2;
  double Y1, Y2;
  double range = GAUSS_RANGE;
  double step;
  double value;

  step = range/N;

  X2 = 0.0; Y2 = GaussPhi( X2 );

  value = 0.0;

  GaussLut_X[0] = X2;
  GaussLut_Y[0] = value;

  for (i=1;i<=N;i++) {
    X1 = X2; Y1 = Y2;
    X2 = X2 + step; Y2 = GaussPhi( X2 );

    value += (Y1+Y2)*0.5*step;
 
    GaussLut_X[i] = X2;
    GaussLut_Y[i] = value;
    
    }

  GaussLutInit = 1;

} /* GaussInit */

void GaussPrintLut( FILE * out, double X[], double Y[] )
{ const int N=GAUSS_LEN-1;
  int i;

  fprintf( out, "\n%s\n\n", GAUSS_VERSION ); 
  for (i=0;i<=N;i++) {
    fprintf(out,"X[%u] = %10.5g, Y[%u] = %10.5g\n",
               i, GaussLut_X[i],i,GaussLut_Y[i]);
    }
} /* GaussPrintLut */

/* returns the interpolated values of the LUT 
   Xn = XX[n]; Yn = YY[n];

   Monoton increasing values are required:
   X1<X2 <=> Y1<Y2
   
         X < X0   -> Y = Y0 
   Xn <= X < Xn+1 -> Y = Yn + (Yn+1-Yn)/(Xn+1-Xn) * (X-Xn) 
   XN <= X        -> Y = YN                                           */
double Ipol_LUT2( double XX[], double YY[], double X )
{ double Y; 
  int i, N = GAUSS_LEN-1;

  if ( X < XX[0] ) Y = YY[0]; 
    else if ( XX[N] <= X ) Y = YY[N];
    else { for (i=1; i<=N; i++) {
             if (X < XX[i]) break;  
             }
           Y = YY[i-1] + (YY[i]-YY[i-1])/(XX[i]-XX[i-1]) * (X-XX[i-1]);
         }
  return( Y );

} /* Ipol_LUT2 */

/* Gauss(x) = 1.0/(sqrt(2*pi)*sigma) * exp(- x^2/(2*sigma^2) */
double Gauss( double x, double sigma )
{ return( (GaussS2Pi/sigma) * GaussPhi( x/sigma ) );
} /* Gauss */

/* IntGauss(x,sigma)=1.0/(sqrt(2*pi)*sigma)*Integral(-Inf,x,Gauss(x,sigma)) */
double IntGauss ( double x, double sigma ) 
{ double value;

  if (!GaussLutInit) {
    GaussInit();
    if (GaussDebugMode) GaussPrintLut( stdout , GaussLut_X , GaussLut_Y );
    }

  if (x<0) value = 0.5-(Ipol_LUT2(GaussLut_X, GaussLut_Y, -x/sigma))/GaussS2Pi;
    else value = 0.5+(Ipol_LUT2(GaussLut_X, GaussLut_Y, x/sigma))/GaussS2Pi;

  return( value );

} /* IntGauss */

/* InvIntGauss(y,sigma) = Inverted IntGauss */
double InvIntGauss ( double y, double sigma )
{ double value;

  if (!GaussLutInit) {
    GaussInit();
    if (GaussDebugMode) GaussPrintLut( stdout , GaussLut_X , GaussLut_Y );
    }

  if (y<0.5) value=(-Ipol_LUT2(GaussLut_Y,GaussLut_X,(0.5-y)*GaussS2Pi))*sigma;
    else value=(Ipol_LUT2(GaussLut_Y,GaussLut_X,(y-0.5)*GaussS2Pi))*sigma;

  return( value );

} /* InvIntGauss */

/* set random number seed */
void GaussNoiseSeed( unsigned int seed )
{  srand( seed );
} /* GaussNoiseSeed */

/* create gaussian distributed noise */
double GaussNoise( double sigma )
{
  int rannum = rand();
  double p;
  double value;

  /* create random numbers between 0 and 1 */
  p = rannum/(RAND_MAX+1.0); 

  /* project the range 0 to 1 to the x-range of the gauss distribution */
  value =  InvIntGauss ( p, sigma ); 
  return ( value );

} /* GaussNoise */

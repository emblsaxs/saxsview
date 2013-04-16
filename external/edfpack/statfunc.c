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

# define STATFUNC_VERSION      "statfunc : V1.0 Peter Boesecke 2012-02-09"
/*+++------------------------------------------------------------------------
NAME
  statfunc.c --- statfunc calculation

INCLUDE FILES
  # include statfunc.h

PURPOSE
  Calculation of statistical functionc from a table of double float elements.

DESCRIPTION

  Implementation of the BFPRT algorithm following the description given in
  http://www.inf.fh-flensburg.de/lang/algorithmen/sortieren/median.htm
 
  M. BLUM, R.W. FLOYD, V.R. PRATT, R.L. RIVEST, R.E. TARJAN: 
  Time Bounds for Selection. 
  Journal of Computer and System Sciences, 7, 448-461 (1972)

AUTHOR
  2012 Peter Boesecke (PB)

HISTORY
  2012-02-09 V1.0  PB median  -> dmedian (averaging if even number)
                      median1 -> median (no averaging)
                      dmediance, dquantilance,
                      mean, variance
--------------------------------------------------------------------------*/

/****************************************************************************
*  Include                                                                  *
****************************************************************************/
# include "statfunc.h" 

/******************************************************************************
* Constants                                                                   *
******************************************************************************/
const double statfunc_eps = 1e-30;

/******************************************************************************
* Functions                                                                   *
******************************************************************************/

/*--------------------------------------------------------------------------
NAME

  statfunc_version --- returns pointer to the version string

SYNOPSIS

  const char *statfunc_version ( void );

DESCRPTION

  Returns pointer to the version string.

--------------------------------------------------------------------------*/
PUBLIC const char *statfunc_version ( void )
{ 
  return ( STATFUNC_VERSION );
} // statfunc_version 

/*
 * Implementation of the BFPRT algorithm following the description given in
 * http://www.inf.fh-flensburg.de/lang/algorithmen/sortieren/median.htm
 * 
 * M. BLUM, R.W. FLOYD, V.R. PRATT, R.L. RIVEST, R.E. TARJAN: 
 * Time Bounds for Selection. 
 * Journal of Computer and System Sciences, 7, 448-461 (1972)
 */

/*--------------------------------------------------------------------------
NAME

  upsort --- sorts the elements of array a[] by increasing values

SYNOPSIS

  int upsort ( double a[], long n )

ARGUMENTS

  double a[]  (i,o) : array with n elements
  n           (i)   : number of table elements


  return 0          : always

DESCRPTION

  Sorts the elements of the array a[] by increasing values.

--------------------------------------------------------------------------*/
int upsort ( double a[], long n )
{ long i,j;
  double aip1;

  double *pai, *paip1, *paj, *pajp1;

  pai=a; paip1=a+1;
  for (i=0;i<n-1;i++) {
    aip1=*paip1;

    if (aip1<*pai) {
      paj=pai; pajp1=pai+1;
      for (j=i;j>=0;j--) {
        if (aip1<*paj)
          *pajp1 = *paj; // move up element a[j]
        else break;
        paj--; pajp1--;
      }
      *pajp1 = aip1; // set last element to a[i+1]
    }
    pai++; paip1++;
  }

  return(0);

} // upsort

/*--------------------------------------------------------------------------
NAME

  upsort5 --- calculates the m5 medians of m/5 elements of b[]

SYNOPSIS

  int upsort5 ( double b[], long m )

ARGUMENTS

  double a[]  (i,o) : array with n elements
  n           (i)   : number of table elements

  return 0          : always

DESCRPTION

  Sorts every m/5-th element of the array b[] by increasing values.
  The middle row starting at &(b[(m/5)*2]) contains (m/5) m5 medians. 
  If the arraylength m is not a multiple of 5 up to 4 elements 
  starting &(b[(m/5)*5]) are not taken into account. 
  This can shift the m5 median by not more than +-2 element values.
  This is in the expected error of m5 compared to the true median and
  can therefore be ignored.

                               b[(m/5)*5] ... b[m-1] (max. 4 ignored elements)

                               b[(m/5)*4] ... b[(m/5)*5-1]   i
                               b[(m/5)*3] ... b[(m/5)*4-1]   ^
  m5 medians after upsort5  -> b[(m/5)*2] ... b[(m/5)*3-1]   |
                               b[(m/5)]   ... b[(m/5)*5-1]   |
                               b[0]       ... b[(m/5)-1]     |

                               --------> l
--------------------------------------------------------------------------*/
int upsort5 ( double b[], long m )
{ long h = m/5;
  long i,j,l;
  double bip1;

   for (l=0;l<h;l++) {
     // sort h "columns" of b[] : l, l+h, l+2h, l+3h, l+4h
     for (i=0;i<4;i++) {
       bip1 = b[l+(i+1)*h];
       if (bip1<b[l+i*h]) {
         for (j=i;j>=0;j--) {
           if (bip1<b[l+j*h])
             b[l+(j+1)*h] = b[l+j*h]; //  move up element b[l+j*h]
           else break;
         }
         b[l+(j+1)*h] = bip1; // set last element to b[l+(i+1)*h]
       }
     }
   }

  // h m5 medians starting at &(b[h*2])
  // call rangk12 ( b+2*h, h, (h-1)/2 , (h-1)/2 );

  return(0);

} // upsort5

/*--------------------------------------------------------------------------
NAME

  exchange --- exchanges element a[j] with a[k]

SYNOPSIS

  int exchange( double a[], long j, long k )

ARGUMENTS

  double a[]  (i,o) : array
  long j      (i)   : element index
  long k      (i)   : element index

  return 0          : always

DESCRIPTION

  Exchanges element a[j] with a[k].

--------------------------------------------------------------------------*/
int exchange( double a[], long j, long k )
{ double tmp;

  tmp = a[j];
  a[j] = a[k];
  a[k] = tmp;

  return(0);

} // exchange 

/*--------------------------------------------------------------------------
NAME

  partition --- distribute the elements of a into three partitions 

SYNOPSIS

  int partition( double a[], long n, double x, long *pp, long *qq );

ARGUMENTS

  double a[]  (i,o) : array with n elements
  long n      (i)   : number of table elements
  double x    (i)   : pivot element
  double *pp  (o)   : begin of the equal x section
  double *qq  (o)   : begin of the larger than x section

  return 0          : always

DESCRIPTION

  Distributes the elements of array a[] into three partitions and returns
  the indices *pp and *qq describing the start of the "equal" section and 
  the "larger than" x section:

    0..*pp-1 contains elements less than x,
    *pp..*qq-1 contains elements equal to x,
    *qq..n-1 contains elements larger than x.

--------------------------------------------------------------------------*/
int partition( double a[], long n, double x, long *pp, long *qq )
{ long i, p, q;
  double y;

  p=0; // the "equal" partition ends at element q-1
  q=0; // the "larger than" partition ends at element n-1
  for (i=0;i<n;i++) {
    y=a[i];
    if (y<=x) {      // if y>x y is in the larger partition, nothing to do
      exchange(a,i,q++); // exchange the first element of the "larger than"
                         // partition with a[i] and increment the start 
                         // index q of the "larger than" partition, the new 
                         // element must be either equal or less than x. 
      if (y<x) {
        exchange(a,q-1,p++); // if y (a[q-1]) is less than x exchange it
                             // with the first element of the "equal" 
                             // partition and increment the start index p 
                             // of the "equal" partition.
      }
    }
  }
  if (pp) *pp=p;
  if (qq) *qq=q;
  
  return(0);

} // partition

/*--------------------------------------------------------------------------
NAME

  rangk12 --- returns the average of the elements in a[] with rang k1 and k2

SYNOPSIS

  double rangk12 ( double a[], long n, long k1 , long k2 );

ARGUMENTS

  double a[]  (i,o) : array with n elements, rearranged
  n           (i)   : number of table elements
  long k1     (i)   : rang k1 (0=<k1<n-1)
  long k2     (i)   : rang k2 (0=<k2<n-1)

  return double (o) : average of the elements with rang k1 and rang k2 
                      in table a[].

DESCRPTION

  Returns the average of the elements with rang k1 and rang k2 in table a[].
  k1 and k2 must be smaller than n. There is no range check,
  If a is NULL or n<=0 the returned value is 0.0.

--------------------------------------------------------------------------*/
double rangk12 ( double a[], long n, long k1 , long k2 ); // for recursion
PUBLIC double rangk12 ( double a[], long n, long k1 , long k2 )
{ double value;
  long tmp;

  // assure that k2 is not smaller than k1
  if (k1>k2) {tmp=k1;k1=k2;k2=tmp;}

  if (n==1) value=a[0];
  else if (n<15) {
    // sort table a
    upsort(a,n);
    if (k1==k2)
      value = a[k1];
    else value = (a[k1]+a[k2])*0.5; 
  } else {
    double *b; 
    long g, h, p, q;
    double m5;
    // to find a suitable pivot calculate the 
    // m5 medians of 5 subsequent table a elements
    upsort5 ( a, n ); 
    h = n/5;
    g = (h-1)/2;
    b = a+2*h; // h m5 medians start at &(a[h*2])
    m5 = rangk12 ( b, h, g, g ); // m5 is a median of medians

    // now partition with m5 as pivot
    // partition a into sections lower, equal, larger than m5
    partition( a, n, m5, &p, &q );

    // k2 is never smaller than k1
    if (k1<p) {
      if (k2<p) {
        value = rangk12( a, p, k1 , k2 );
      } else if (k2<q) {
        value = (rangk12( a, p, k1 , k1 ) + a[p])*0.5;
      } else value = (rangk12( a, p, k1 , k1 ) + 
                      rangk12( a+q, n-q, k2-q , k2-q ))*0.5;
    } else if (k1<q) {
      if (k2<q) {
        value = a[p];
      } else value = (a[p]+ rangk12( a+q, n-q, k2-q , k2-q ))*0.5;
    } else value = rangk12( a+q, n-q, k1-q , k2-q );
  }

  return( value );

} // rangk12

/*--------------------------------------------------------------------------
NAME

  median --- returns the median of table a[]

SYNOPSIS

  double median ( const double a[], long n );

ARGUMENTS

  return double (o) : median of table a[] 

DESCRPTION

  Returns the median of table a[] containing n elements. 
  The median is the element with rang k=(n-1)/2.
  If a is NULL or n<=0 the returned value is 0.0.

--------------------------------------------------------------------------*/
PUBLIC double median ( double a[], long n )
{ double value=0.0;
  long k= (n-1) / 2;

  if ( a && (n>0) ) {
     value = rangk12 ( a, n, k, k );
  }

  return(value);

} // median

/*--------------------------------------------------------------------------
NAME

  dmedian --- returns the median of table a[]

SYNOPSIS

  double dmedian ( const double a[], long n );

ARGUMENTS

  return double (o) : median of table a[] 

DESCRPTION

  Returns the median of table a[] containing n elements. 
  If n is odd the median is the element with rang k=(n-1)/2.
  If n is even the average of the lower and upper median value is 
  returned, e.g the average of the elements with rang k1=(n-1)/2 and 
  rang k2=n/2.
  If a is NULL or n<=0 the returned value is 0.0.

--------------------------------------------------------------------------*/
PUBLIC double dmedian ( double a[], long n )
{ double value=0.0;
  long k1= (n-1) / 2;
  long k2 = n / 2;

  if ( a && (n>0) ) {
     value = rangk12 ( a, n, k1, k2 );
  }

  return(value);

} // dmedian

/*--------------------------------------------------------------------------
NAME

  dquantil --- returns the p-quantil of table a[]

SYNOPSIS

  double dquantil ( double a[], long n, double p );

ARGUMENTS

  double a[] : table with n-elements
  long n: number of table elements
  double p: 0<=p<=1

  return double (o) : p-quantil of table a[] 

  p is a value between 0 and 1 giving the ratio between
  the number of elements with values lower than the returned
  value and the total number of elements.

  If p<=0 the minimum value is returned, if p>=1 the
  maximum value is returned.

DESCRPTION

  Returns the p-quantil of table a[] containing n elements. 
  If a is NULL or  n<=0 the returned value is 0.0.

--------------------------------------------------------------------------*/
PUBLIC double dquantil ( double a[], long n, double p )
{ double value=0.0, pn;
  long k1, k2;

  if (p<=0.0) value = minimum(a,n);
  else if (p>=1.0) value = maximum(a,n);
  else {

    pn=p*(double) n;

    if ( a && (n>0) ) {
      if ( fabs( (pn - floor(pn)) < statfunc_eps ) ) {
        k2 = (long) pn; k1 = k2-1;
      } else k1 = k2 = ceil(pn)-1;

      value = rangk12 ( a, n, k1, k2 );
    }
  }

  return(value);

} // dquantil

/*--------------------------------------------------------------------------
NAME

  minimum --- returns the minimum of table a[]

SYNOPSIS

  double minimum ( const double a[], long n );

ARGUMENTS

  return double (o) : minimum of table a[]

DESCRPTION

  Returns the minimum of table a[] containing 
  n elements. If a is NULL or n<=0 the returned 
  value is 0.0.

--------------------------------------------------------------------------*/
PUBLIC double minimum ( double a[], long n )
{ double value=0.0;

  if ( a ) {
    long i;
    if (n>0)
      value = a[0];
    for (i=1;i<n;i++) {
      if (a[i]<value)
        value = a[i];
    }
  }

  return(value);

} // minimum

/*--------------------------------------------------------------------------
NAME

  maximum --- returns the maximum of table a[]

SYNOPSIS

  double maximum ( const double a[], long n );

ARGUMENTS

  return double (o) : maximum of table a[]

DESCRPTION

  Returns the maximum of table a[] containing
  n elements. If a is NULL or n<=0 the returned
  value is 0.0.

--------------------------------------------------------------------------*/
PUBLIC double maximum ( double a[], long n )
{ double value=0.0;

  if ( a ) {
    long i;
    if (n>0)
      value = a[0];
    for (i=1;i<n;i++) {
      if (a[i]>value)
        value = a[i];
    }
  }

  return(value);

} // maximum

/*--------------------------------------------------------------------------
NAME

  minmaxfilter --- shortens the table a[] to values between minval and maxval

SYNOPSIS

  long minmaxfilter ( double a[], long n, 
                      double minval, double maxval );

ARGUMENTS

  return long (o) : new length

DESCRPTION

  Modifies a[] in such a way that the first n elements fall between
  minval<=a[i]<=maxval.  Returns the new length of a[].
  If a is NULL or n<=0 the returned value is 0.

--------------------------------------------------------------------------*/
PUBLIC long minmaxfilter ( double a[], long n,
                           double minval, double maxval )
{ long i, last=0;

  if ( a ) {
    i=0; last=n;
    if (last<0) last=0;
    while (i<last) {
      double ai;
      ai=a[i];
      if ( (ai < minval) || (maxval < ai) ) {
        exchange( a, i, last-1 );
        last--;
      } else i++;
    }
  }
  return(last);

} // minmaxfilter

/*--------------------------------------------------------------------------
NAME

  dmediance --- returns the median of the squared differences a[]-m 

SYNOPSIS

  double dmediance ( const double a[], long n, double m );

ARGUMENTS

  const double a[]  (i) : input table with n elements
  long n            (i) : number of elements
  double m          (i) : median

  return double (o) : median of the squared differences a[]-m 

DESCRPTION

  Calculates and returns the median of the squared differences a[]-m.
  If more than 50% of the elements of a[] are equal to m the returned
  value is zero.
  If a is NULL or n<=0 the returned value is 0.

--------------------------------------------------------------------------*/
PUBLIC double dmediance ( const double a[], long n, double m )
{ double *amm=NULL;
  double value=0.0;
  long i;

  if ( (a) && (n>0) ) {
    // allocate amm
    if (!(amm = (double*) malloc( sizeof(double)*n )))
      goto dmediance_error;

    // calculate amm[] = (a[]-m)
    for (i=0;i<n;i++)
      amm[i]=fabs(a[i]-m);

    // dmediance = dmedian ( amm[], n )^2
    value = dmedian( amm, n );

    if (amm)
      free(amm);
  }

  return(value*value);

dmediance_error:

  if (amm)
    free(amm);

  return(value*value);

} // dmediance

/*--------------------------------------------------------------------------
NAME

  dquantilance --- returns the squared p-quantil distance of a[]

SYNOPSIS

  double dquantilance ( double a[], long n, double p );

ARGUMENTS

  const double a[]  (i) : input table with n elements (modified)
  long n            (i) : number of elements
  double p          (i) : 0<p<=1 dquantil(..,1-p)-dquantil(..,p)

  return double (o) : squared p-quantil distance of a[]

DESCRPTION

  Calculates the squared p-quantil distance of a[]. If p<=0 o p>=1 the 
  squared half difference between the maximum and the minimum value of a[] 
  is returned. If a is NULL or n<=0 the returned value is 0.

--------------------------------------------------------------------------*/
PUBLIC double dquantilance ( double a[], long n, double p )
{ double value=0.0;

  if ( (a) && (n>0) ) {

    // dquantilance = dmedian ( amm[], n )^2
    value = (dquantil( a, n, 1.0-p ) - dquantil( a, n, p ))*0.5;

  }

  return(value*value);

dquantilance_error:

  return(value*value);

} // dquantilance

/*--------------------------------------------------------------------------
NAME

  mean --- returns the mean of table a[]

SYNOPSIS

  double mean ( const double a[], long n );

ARGUMENTS

  return double (o) : mean of table a[] 

DESCRPTION

  Returns the mean of table a[] containing n elements. 
  If a is NULL or n<=0 the returned value is 0.0.

--------------------------------------------------------------------------*/
PUBLIC double mean ( double a[], long n )
{ double value=0.0;

  if ( ( a ) && (n>0) ) {
    long i;
    for (i=0;i<n;i++) {
      value += a[i];
    }
    value /= n;
  }

  return(value);

} // mean

/*--------------------------------------------------------------------------
NAME

  variance --- returns the variance of the values in table a[] around a mean

SYNOPSIS

  double variance ( const double a[], long n, double m );

ARGUMENTS

  return double (o) : sum of (a[]-m)^2

DESCRPTION

  Returns the mean value of (a[i]-m)^2. If m is the mean-value, this is the 
  expected variance for each element of a[]. To calculate the variance of 
  the mean value that is statistically independent of the mean of all 
  elements of a[] the result of this function must be divided by n-1.
  If m is independent of a[], e.g. if it is exactly known, the result must
  be devided by n to calculate the variance of m.
  If a is NULL or n<=0 the returned value is 0.0.

--------------------------------------------------------------------------*/
PUBLIC double variance ( double a[], long n, double m )
{ double value=0.0;

  if ( ( a ) && (n>0) ) {
    long i;
    for (i=0;i<n;i++) {
      double diff;
      diff = a[i]-m;
      value += diff*diff;
    }
    value /= n;
  }

  return(value);

} // variance

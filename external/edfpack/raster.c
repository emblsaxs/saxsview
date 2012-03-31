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

/*--------------------------------------------------------------------------
NAME

   raster.c --- Raster configuration functions 

AUTHOR
  1998-03-13 Peter Boesecke

HISTORY
  2010-07-07 V1.0 Peter Boesecke extracted from edfio.c V2.25 and adapted.
  2011-04-11 V1.1 PB matrix functions added: raster_fprint_matrix,
                     raster_matrix_product, raster_order2matrix,
                     raster_order_determinante,
                     strtol replaced with num_str2long with errval+1000
                     9 -> MAX_RASTER_NUMBER_DIMENSION
  2011-05-25 V1.2 PB order2wrap: only N+1 elements accessed,
                     ovlpchk added, raster_order_normalization: src and 
                     dest can overlap. 
  2011-06-01 V1.3 PB raster_order_normalization: if src and dest are
                     identical and if raster orientation is 1 the array
                     is not recalculated
  2011-06-07 V1.4 PB fprintf -> printf
  2011-12-14 V1.5 PB raster_order2str: long order[] -> const long order[]
                     Documentation corrected in raster_order_normalization 
                     and raster_normalization: data_dim[] must contain the 
                     output array dimensions.

--------------------------------------------------------------------------*/
# define RASTER_VERSION "raster : V1.5 Peter Boesecke 2011-12-14"
/****************************************************************************
*  Include                                                                  *
****************************************************************************/
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

# include "raster.h"
# include "numio.h"

/****************************************************************************
*  PRIVATE part                                                             *
****************************************************************************/

/****************************************************************************
* Static Variables                                                          *
****************************************************************************/
static int RN_debug = 0;

/*--------------------------------------------------------------------------
NAME

  raster_debug --- set / reset module raster_conversion into debug mode

SYNOPSIS

  void raster_debug ( int debug );

DESCRPTION

  Writes ´debug´ into RN_debug.

--------------------------------------------------------------------------*/
void raster_debug ( int debug )
{ RN_debug = debug;
} /* raster_debug */

/*--------------------------------------------------------------------------
NAME

  ovlpchk --- returns 0 if the memory regions of a and b are distinct

SYNOPSIS

  inv ovlpchk( void *a, size_t a_size, void *b, size_t b_size );


ARGUMENTS

   void *a              (i) : start memory region a
   size_t a_size        (i) : size of memory region a
   void *b              (i) : start memory region b
   size_t b_size        (i) : size of memory region b

DESCRPTION

RETURN VALUE

   0 no overlap
   1 overlap

--------------------------------------------------------------------------*/
int ovlpchk( const void *a, size_t a_size, const void *b, size_t b_size )
{ const void *ll, *uu;
  size_t ll_size;

  if (a<=b) {
    ll=a; uu=b; ll_size=a_size;
  } else {
    ll=b; uu=a; ll_size=b_size;
  }

  if ( (const char *) ll+ll_size<=(const char *) uu ) return(0); // no overlap
  else return(1); // overlap

} // ovlpchk

/*--------------------------------------------------------------------------
NAME

  raster_numbers --- number of raster configurations of an n-dimensional array
 
SYNOPSIS

  long raster_numbers ( long n );

DESCRIPTION
  Returns the number A(n) of raster configurations of an n-dimensional 
  array:

      A(n) = 2^n * (n!)

  For n>9 the number of configurations are out of the value range of long int.
  In this case the returned value is 0.

ARGUMENTS
  long n  dimension  (number of coordinates)

RETURN VALUE
  long  A(n) = 2^n * (n!)  (for 0<n<=MAX_RASTER_NUMBER_DIMENSION, otherwise 0)

HISTORY
  11-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
long raster_numbers ( long n )
{ const long max_n = MAX_RASTER_NUMBER_DIMENSION;
  register long i;
  long A;

  if ((n<0) || (n>max_n)) return ( 0 );
  A=1; 
  for (i=1;i<=n;i++) A*=2*i; 
  return( A );

} /* raster_numbers */

/*--------------------------------------------------------------------------
NAME

  value2index --- return index of value in order

SYNOPSIS

  long value2index( const long order[], long n, long value );

DESCRIPTION
  Starting with ´index=1´ at ´order[1]´ (´order[0]´ contains the length of the 
  array) it increments ´index´ at each array element which absolute value
  is smaller than n until an array element with abs(´order[i]´) = ´value´ 
  is found. The ´index´ of this array element is returned, positive if 
  ´value´ is positive, negative otherwise.

ARGUMENTS
  const long order[0] == N dimension (number of coordinates)
  long       order[n+1]    raster configuration array
  long       value         value to be found 

RETURN VALUE
  long       index          

HISTORY
  12-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
long value2index( const long order[], long n, long value )
{ long i;
  long N, index;

  value = labs(value);
  if ( (!order) || (value<1) || (n<value) ) return(0);

  N = order[0];
  index=1;
  for (i=1;i<=N;i++) {
    if (labs(order[i])<=n) {
      if (order[i]==value) break; 
      else if (order[i]==-value) { index=-index; break; }
      index++;
      }
   } /* for */

  if (index>n) index=0;

  return(index);

} /* value2index */

/*--------------------------------------------------------------------------
NAME

  value2order --- insert value at n-th free index

SYNOPSIS

  void value2order( long order[], long n, long index, long value );

DESCRIPTION
  Starting with order[1] (order[0] contains the length of the array) it skips 
  all non-zero array elements and decrements ´index´ at each zero array 
  element. It replaces the ´index´ed zero array element with ´value´. 

ARGUMENTS
  long      order[n+1]   raster configuration array
  long      n            dimension (number of coordinates)
  long      index        index of FREE cell at which the value will be written.
  long      value        value to be written 

RETURN VALUE
  void

HISTORY
  11-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
void value2order( long order[], long N, long index, long value )
{ long i;

  if (index==0) return;
  for (i=1;i<=N;i++) {
    if (order[i]==0) 
      if (--index==0) { order[i] = value; break; }
   } 

  return;

} /* value2order */

/*--------------------------------------------------------------------------
NAME

  raster_number2order --- returns the coordinate order array of a raster configuration

SYNOPSIS

  long * raster_number2order ( long * buffer, size_t nelem, 
                               long n , long raster_number );

DESCRIPTION
  The n-dimensional configuration order array corresponding to 
  ´raster_number´ is returned. For n>9 not all configurations 
  can be calculated due to the limited value range of long int. 
  The highest raster_number that is accepted is 
  raster_numbers(MAX_RASTER_NUMBER_DIMENSION). Configuration order
  arrays for n>MAX_RASTER_NUMBER_DIMENSION are calculated until the
  highest accepted raster number.

ARGUMENTS
  long *buffer            input buffer (or NULL if not supplied)
  size_t nelem            input buffer length (needed nelem>=n+1) 
  long      n             dimension (number of coordinates)
  long      raster_number raster configuration number

  If buffer is NULL an array with n+1 elements is allocated and 
  must be free´ed by the calling program.

RETURN VALUE
  long * raster_coordinate      (allocated array with n+1 elements
         raster_coordinate[0]   dimension of array
         raster_coordinate[1]   coordinate corresponding to fastest index (1)
            ...
         raster_coordinate[n]   coordinate corresponding to slowest index (n) 
   In case of an error NULL is returned.

HISTORY
  14-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
long * raster_number2order ( long * buffer, size_t nelem,
                             long n , long raster_number )
{ static const char * R2O_Error     = "ERROR: raster_number2order:";
  static const char * R2O_MisMatch  = "dimension mismatch";
  const long max_n = MAX_RASTER_NUMBER_DIMENSION;
  long AN, ANm1x2, ANm1;
  long D= raster_number;
  long N = n;
  int  inverse=0;

  long * order = (long *) NULL;

  long * output = (long *) NULL;

  long * output_buffer =  (long *) NULL;
  long * internal_buffer =  (long *) NULL;

  long index;
  long value;

  long i, m;

  // Calculate inverse order array if raster_number is negative
  if (raster_number<0) {
    D = -raster_number;
    inverse=1;
  }

  if (buffer) {
    if (nelem<=N) {
      fprintf(stderr,"%s %s\n",R2O_Error,R2O_MisMatch);
      goto raster_number2order_error;
    }
    output = buffer;
  } else {
    /* allocate output buffer and clear it */
    if ( !(output_buffer = (long*) calloc( (N+1), sizeof(long) )) ) 
      goto raster_number2order_error;
    output = output_buffer;
  }

  if (inverse) {
    /* allocate temporary buffer and clear it */
    if ( !(internal_buffer = (long*) calloc( (N+1), sizeof(long) )) )
      goto raster_number2order_error;
    order = internal_buffer;
  } else order = output;

  /* clear buffer 0..N */
  for (i=0l;i<=N;i++) order[i]=0l;

  order[0] = N;

  /* special case for n>max_n */
  if ( (D <= raster_numbers( max_n )) && ( n>max_n ) ) {
    for (index=max_n+1;index<=n;index++) order[index]=index; n=max_n; 
  }

  AN     = raster_numbers ( n );

  /* check parameters */
  if ( !((0l<D) && (D<=AN)) ) 
    goto raster_number2order_error; /* undefined */

  for (n=n;n>0;--n) {
    ANm1   = AN/(n+n); if (AN==0) AN=1;
    ANm1x2 = ANm1+ANm1;

    m = (D-1) / ANm1x2;   /* m = 0, ... ,n-1 */
    D = D - m * ANm1x2;
    index = n - m;        /* index = 1, ..., n */
    if (D<=ANm1) value = n; else { value = -n;  D=D-ANm1; }
    value2order( order, N , index, value ); 

    AN     = ANm1;

    } /* for */

  if (inverse) order = raster_order_inversion ( output, nelem, order );

  if (internal_buffer) free( internal_buffer );

  return( order );

raster_number2order_error:

  if (internal_buffer) free( internal_buffer );

  if (output_buffer) free( output_buffer );

  return( NULL );

} /* raster_number2order */

/*--------------------------------------------------------------------------
NAME

  raster_order2number --- raster configuration number of array order

SYNOPSIS

  long raster_order2number ( long order[] );

DESCRIPTION
  The raster configuration number of the order[0]-dimensional configuration 
  array ´order´ is returned. For order[0]>]>MAX_RASTER_NUMBER_DIMENSION not
  all configuration numbers can be calculated due to the limited value 
  range of long int. In this case a zero is returned.

ARGUMENTS
  long      order[0]  number of dimensions (long order[order[0]+1]) 
  long      order[n]  configuration order array 
 
The element ´order[0]´ must contain the dimension. At least ´order[0]+1´ 
elements must be allocated for ´order[]´. 

RETURN VALUE
  success   long    raster_number
  error     long    0 

HISTORY
  12-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
long raster_order2number ( const long order[] )
{ long i, n;
  long ANm1, D;
  long index;

  if (!order) return(0);

  n = order[0];

  if (MAX_RASTER_NUMBER_DIMENSION < n) {
    for (i=MAX_RASTER_NUMBER_DIMENSION+1; i<=n; i++)
      if (order[i]!=i) return(0); 
    n = MAX_RASTER_NUMBER_DIMENSION;
  }

  D = 1;

  ANm1 = raster_numbers ( 0 );
  for (i=1;i<=n;i++) {
    index = value2index( order, i, i );
    if ( index == 0 ) return(0);
    if ( index > 0 ) D += (i-index)*2*ANm1; 
    else D += (i+index) * 2*ANm1 + ANm1;

    ANm1 = ANm1 * 2 * i;   /* raster_numbers ( i ); */
  }

  return( D );
 
} /* raster_order2number */

/*--------------------------------------------------------------------------
NAME

  order2wrap --- calculates the wrap array from order and dim 

SYNOPSIS

  int order2wrap ( long wrap[], const long dim[], const long order[] );

DESCRIPTION
  Calculates the array ´wrap´ that contains for each index the displacement
  in number of array elements when it is incremented by 1. For the fastest 
  index wrap[fast] is 1.

ARGUMENTS
  long wrap[N+1]     output array element, must be sufficiently large
  long dim[N+1]      data array dimensions, dim[0] is N
  long order[N+1]    input order of dimensions
  All three array use only index 1 to index N. The values for the n_th 
  dimension is found in array[n], array[0] is used for the number of 
  dimensions. 

  array[n+1] is not accessed any more (for the total length of the data array.

RETURN VALUE
int status
success : 0
error   : -1

AUTHOR
  15-Mar-1998 Peter Boesecke

--------------------------------------------------------------------------*/
int order2wrap ( long wrap[], const long dim[], const long order[] )
{ long N = dim[0];
  long i,j; 

  wrap[0] = dim[0]; 
  // wrap[N+1] = dim[N+1]; // generally, element N+1 is not allocated

  for (i=1;i<=N;i++) {
    wrap[i]=1l;
    for (j=1;j<labs(order[i]);j++) wrap[i] *= dim[j];
  }

  return( 0 );

} /* order2wrap */

/*--------------------------------------------------------------------------
NAME

  reorder_raster --- reorder recursively the array src and write to dest 

SYNOPSIS

  void reorder_raster ( long n, void * dest, const void ** pps, size_t item, 
                        long I[], const long IS[], const long ILoop[], 
                        const long IInc[], const long IWrap[] );

DESCRIPTION
  Recursive function for reordering of a n-dimensional array. 
  The source array is read element by element. The elements of the 
  destination array are written according to the numbers in IS, ILoop, 
  IInc and IWrap.

ARGUMENTS
      long n           dimension of the data arrays
      void * dest      destination array (must be allocated)
const void ** pps      pointer to source array of dimension n 
                       (must contain useful data)
      t_size item      length of a single array element
      long I[n+2]      array that contains the current indices 
                       (must be allocated, will be used by this routine)
const long IS[n+2]     array that contains the start value of each index 
const long ILoop[n+2]  array that contains the number of loops of each index
const long IInc[n+2]   array that contains the increment of each index
const long IWrap[n+2]  array that contains the distance between elements in
                       the destination array. For no reordering (type 1
                       to type 1) IWrap[1] would be equal to 1, IWrap[2]
                       would be equal to Dim[1], IWrap[3] would be equal
                       to Dim[1]*Dim[2] etc. IWrap[0] must contain the
                       number of dimensions.

AUTHOR
  15-Mar-1998 Peter Boesecke
  17-May-1998 PB calculation of pd corrected: I[nn]  -> (I[nn]-1l)
  26-Nov-1999 PB cc on dec alpha: statements of the form
                 (void *) pp + (size_t) item; are not allowed, therefore
                 changed to (char *) pp1 + (size_t) item; pp = (void *) pp1;
  19-Dec-1999 PB pps1 correctly defined as char **pps1
-----------------------------------------------------------------------------*/
void reorder_raster ( long n, void * dest, const void ** pps, size_t item,
                      long I[], const long IS[], const long ILoop[],
                      const long IInc[], const long IWrap[] );
void reorder_raster ( long n, void * dest, const void ** pps, size_t item, 
                      long I[], const long IS[], const long ILoop[], 
                      const long IInc[], const long IWrap[] )
{ long ii, nn;
  long N = IWrap[0];
  char *pd, **pps1;

  if (n>0) {
    for (ii=0,I[n]=IS[n]; ii<ILoop[n]; ii++,I[n]+=IInc[n])
      reorder_raster ( n-1, dest, pps, item, I, IS, ILoop, IInc, IWrap ); 
  } else {
    pd = (char *) dest;
    for (nn=1;nn<=N;nn++) pd += item * (I[nn]-1l) * IWrap[nn];

    memcpy ( (void *) pd, *pps, item );
    pps1 = (char **) pps;
    *pps1 += item; 
  }

} /* reorder_raster */

/*+++--------------------------------------------------------------------------
NAME

  raster_normalization --- conversion to raster configuration 1

SYNOPSIS

  int raster_normalization ( void * dest, const void * src,
                             const long data_dim[],
                             long raster_number, size_t item, int *perrval );

DESCRIPTION
  Conversion of the multi-dimensional array src with raster configuration 
  number ´raster_number´ to the n-dimensional array dest with 
  raster configuration number 1. The length n of data_dim is stored in 
  data_dim[0]. data_dim[i] is the length of coordinate i. The input and 
  output arrays can overlap.
  Sufficient memory must have been allocated for both arrays. 

  The raster configuration specifies only the way how data is stored. It does
  not influence the number of dimensions. Therefore, the dimension array is
  not changed even if, apparently, horizontal and vertical axes were changed.

  To convert array src from any raster configuration a to any other raster 
  configuration b the raster_number argument is given by 

  raster_number= 
      raster_multiplication( raster_inversion(b), a );

ARGUMENTS
  void * dest                  output array (must be allocated), must
                               be different from source array
  const void * src             source array
  const long data_dim[]        dimensions of output array
  long raster_configuaration   raster configuration number of source array
  size_t item                  size of an array element
  
GENERAL
  An n-dimensional array has ´N = 2^n * (n!)´ different ways of storing its
  data in a regular raster. Each of the n axes can be stored in two different
  ways: up and down. This results in 2^n different possibilities of data
  storage. The n axes can be stored in any of the (n!) possible permutations.
  This results in N = 2^n * (n!) different ways of storing the data. A
  two dimensional array (n=2) can be stored in 2^2 * 2! = 8 different ways, a
  three dimensional array (n=3) can be stored in 2^3 * 3! = 8*6 = 48 different
  ways, and so on.

  The data elements are stored in an array with dim_1 * dim_2 * ... * dim_n
  cells with identical size. The number of dimension is n. The first
  index (i_1) is the fastest. A specific raster configuration is given by 
  the n-tupel (k_2, -k_3, -k_1), which means that the fastest index 
  i_1 corresponds to the coordinate k_2, the medium fast index i_2 
  corresponds to the invers k_3 direction and the slowest coordinate i_3 
  corresponds to the invers k_1 direction. 

RASTER CONFIGURATION 
  Arrays can be stored in different ways depending on the relationship
  between offsets and fast and slow array indices. The configuration of
  the indices is specified by a raster configuration number D.

  A unique raster configuration number for multi dimensional arrays is 
  used that is defined on the basis of the following demands. The raster
  configuration number is called D in the text:

  - Array indices are numbered from 1 to N. 

  - A one dimensional array can be stored from low array indices to high 
    array indices (D=1) or from high array indices to low array indices
    (D=2).
 
  - The array element Array(k_1, k_2, k_3, ... , k_N) is accessed with an
    offset I from the first element measured in element size

    I = (i_1-1) + (i_2-1) * Dim[1] + (i_3-1) * Dim[2] * Dim[1] + ...

      = Sum[ J=1,J<=N,J++ ]( i_J * Product[i=1,i<=J,i++](Dim[i]) )

      (Dim[0]==1)
   
    with i_1, i_2, i_3, ...  replaced by k_1, k_2, k_3, ... in a 
    special order that is specified by the raster configuration number D.

    The raster configuration is described by grouping the array indices 
    k_nn and the offset indices i_nn, e.g. for a 2-dimensional array where 
    the fast and slow indices are interchanged. 

    Example for N=2 and D=5: 

    i_n   1, 2 
    k_nn  2, 1 

  - The raster configuration number D is 1, when the array indices are ordered
    from "fast" (i_1) to "slow" (i_N), e.g. when i_1 = k_1, i_2 = k_2, etc. 
    
  - The raster configuration D of the n-dimensional sub-array 
    Array[Dim_1, Dim_2, ... Dim_n] is the same as for the (n+1) 
    dimensional array Array[Dim_1, Dim_2, ... Dim_n, Dim_(n+1)] if 
    Dim_(n+1) == 1.
    
       D ( Array[Dim_1, Dim_2, ... Dim_n, 1 ] ) 
        == D ( Array[Dim_1, Dim_2, ... Dim_n] )

  These demands give a unique description of multi-dimensional raster 
  conformations with a configuration number D.

  The raster configuration is defined as follows:

  For the definition it is necessary to distinguish strictly between the 
  array indices k_1, k_2, etc. and the offset indices i_1, i_2, etc.

  An offset index with a small number, e.g. i_1 runs faster than an index 
  with a higher number, e.g. i_3. 

  An n-dimensional array has ´N = 2^n * (n!)´ different ways of storing its 
  data in a regular raster. Each of the n axes can be stored in two different 
  ways: up and down. This results in 2^n different possibilities of data
  storage. The n axes can be stored in any of the (n!) possible permutations.
  This results in N = 2^n * (n!) different ways of storing the data. A
  two dimensional array (n=2) can be stored in 2^2 * 2! = 8 different ways, a
  three dimensional array (n=3) can be stored in 2^3 * 3! = 8*6 = 48 different
  ways, and so on.

  The data elements are stored in an array with dim_1 * dim_2 * ... * dim_n 
  cells with identical size. The number of data elements is n. The first 
  index is the fastest. A specific raster configuration is given by the n-tupel
  (x_2, -x_3, -x_1), which means that the fastest index i_1 corresponds to 
  the coordinate x_2, the medium fast index i_2 corresponds to the invers x_3 
  direction and the slowest coordinate i_3 corresponds to the invers x_1 
  direction. If the array has n_1, n_2 and n_3 elements in each direction the 
  data origin X0 = (0,0,0) in the real world corresponds to the array element
  IX0 = (0,n_2,n_3). 

RASTER ORIENTATION

  In the same way as the raster configuration describes the order of slow 
  and fast indices of a stored array the raster orientation can describe
  the orientation of an n-dimensional coordinate system with respect to a 
  standard coordinate system.

  The definition of a 3-dimensional raster orientation is based on a 
  standard orthogonal coordinate system with the first coordinate (x_1) 
  horizontal and the second coordinate (x_2) vertical with respect to the 
  observer. In standard scattering geometry the observer is located at 
  the sample position and is looking against the detector. The origin 
  of the coordinate system is at the lower left corner of the detector. 
  The direction of the third coordinate x_3 is found with the vector 
  product x_1 X x_2 = x_3. It is pointing against the observer.  

  With respect to the laboratory the standard coordinate system is defined 
  with respect to the primary beam. Axis x_3 is always opposite to the 
  travel direction of the primary beam. The definition of the other two 
  coordinate axes is less strict. They can be defined with respect to a 
  reference plane which is usually the plane of the storage ring or, most 
  of the times simpler, the ground floor. As demanded above the axes x_1,  
  x_2 and x_3 form a righthanded orthogonal coordinate system. 

  On earth, the reference plane is usually the ground floor, perpendicular to 
  "gravity", which coincides with the storage ring plane, but this is not a 
  must. If  the primary beam is propagating parallel to the reference plane 
  axis x_1 lies in the reference plane ("horizontal") and axis x_2 is normal 
  to the reference plane ("vertical"). If the primary beam is inclined with 
  respect to the reference plane, e.g. when the beam is reflected by a mirror, 
  the directions x_1 and x_2 are calculated with the cross product of x_3 with 
  the normal-to-plane ("gravity") (e_n) direction (e means a unit vector with 
  length 1):

  (1)   e_1 = e_n x e_3/ABS(e_n x e_3)
  (2)   e_2 = e_3 x e_1

CONFIGURATION NUMBERS

  In the standard configuration (D=1) all array indices k_nn are identical
  to the offset indices i_nn. 

  In the 1-dimensional case the standard configuration (numbered 1) is the 
  configuration in which the array index increases with the coordinate. The 
  second configuration is where the index is antiparallel to the coordinate 
  (numbered 2).

  n=1 has two configurations (A(2) = 2): 

  n  raster_number D                Configuration
  1    1                              1
  1    2                             -1

  If the number of configurations for n coordinates is A(n) = 2^n * (n!),
  the number of configurations for n+1 coordinates is given by

     2 * (n+1) * A(n) = 2^(n+1) * (n+1)!
  
  C(n) is the group of all possible configurations for n coordinates. If it is
  given, the configurations for n+1 coordinates can be built by inserting the
  configurations of the new coordinate (1 and -1) at each of the n+1 possible 
  positions (n before each coordinate and 1 after the last coordinate). To
  have a well defined ordering the new coordinate is first added non-inverted 
  after the end of all A(n) configurations and then inverted (A(n)*2). This is 
  repeated subsequently from the end to the start before all n remaining 
  positions.   

    (x1, x2, x3, ... , xn) -> (x1, x2, x3, ... , xn,  xn+1) 
                              (x1, x2, x3, ... , xn, -xn+1)          +2 

                              (x1, x2, x3, ... ,  xn+1, xn)
                              (x1, x2, x3, ... , -xn+1, xn)          +2

                                          ...

                              ( xn+1, x1, x2, x3, ... , xn) 
                              (-xn+1, x1, x2, x3, ... ,-xn)          +2

                                                                   (n+1)*2
                                       A(n) = 2^n * (n!)


  The total number of configuration for n+1 coordinates is then

           2 * (n+1) * A(n) = 2^(n+1) * (n+1)! ,

  which is equal to A(n+1).   

EXAMPLE

  n  raster_number D               Configuration

  1    1                              1
  1    2                             -1

  2    1                              1, 2
  2    2                             -1, 2
  2    3                              1,-2
  2    4                             -1,-2
  2    5                              2, 1
  2    6                              2,-1
  2    7                             -2, 1
  2    8                             -2,-1

  3    1                              1, 2, 3
  3    2                             -1, 2, 3
  3    3                              1,-2, 3
  3    4                             -1,-2, 3
  3    5                              2, 1, 3
  3    6                              2,-1, 3
  3    7                             -2, 1, 3
  3    8                             -2,-1, 3
  3    9                              1, 2,-3
  3   10                             -1, 2,-3
  3   11                              1,-2,-3
  3   12                             -1,-2,-3
  3   13                              2, 1,-3
  3   14                              2,-1,-3
  3   15                             -2, 1,-3
  3   16                             -2,-1,-3
  3   17                              1, 3, 2
              ...
  3   32                             -2,-3,-1
  3   33                              3, 1, 2
              ...
  3   48                             -3,-2,-1

  The raster configuration 13 for n=3 (2, 1,-3) means that the first offset 
  index of the array (which is the fastest) corresponds to the coordinate 
  k_2, the second index corresponds to the coordinate k_1 and the third index 
  to the inverted coordinate k_3.

  The largest D (D = A(n)) is always the conformation where the direction
  and order of all array indices are inverted. 

RETURN VALUE
int status
success : 0
error   : -1

AUTHOR
  07-Jul-2010 Peter Boesecke
-----------------------------------------------------------------------------*/
int raster_normalization ( void * dest, const void * src, 
                           const long data_dim[], 
                           long raster_number, size_t item, int * perrval )
{ static const char * RN_Error  = "ERROR: raster_normalization:";
  static const char * RN_NoDim  = "no or zero dimension"; // 1
  static const char * RN_NoRas  = "bad raster configuration"; // 2

  long * order=NULL;
  long   n;

  int status=0;
  int errval=0;

  if (RN_debug) printf(" raster_normalization BEGIN\n");

  if (!data_dim) {
    fprintf(stderr,"SEVERE %s Data dimension pointer missing!",RN_Error);
    exit(-1);
  }
 
  /* get dimension */
  if (data_dim) n = data_dim[0]; else n = 0;
  if (n<=0) {
    errval=1;
    fprintf(stderr,"%s data_dim %s\n",RN_Error, RN_NoDim); 
    goto raster_normalization_error;
  }

  /* get index order */
  if (!(order = raster_number2order ( NULL, 0, n , raster_number )) ) {
    errval=2;
    fprintf(stderr,"%s raster_number=%ld %s\n",RN_Error, raster_number, RN_NoRas);
    goto raster_normalization_error;
  }

  if ( raster_order_normalization ( dest, src, data_dim, order, item, &errval ) )
    goto raster_normalization_error;

  if (order) free ( order );

  if (perrval) *perrval=errval;

  if (RN_debug) printf(" raster_normalization END\n");

  return ( status );

raster_normalization_error:

  if (order) free ( order );

  if (perrval) *perrval=errval;

  if (RN_debug) printf(" raster_normalization END (errval=%d)\n",errval);

  return ( -1 );

} /* raster_normalization */

/*+++--------------------------------------------------------------------------
NAME

  raster_order_normalization --- conversion to raster configuration 1

SYNOPSIS

  int raster_order_normalization ( void * dest, const void * src,
                                   const long data_dim[],
                                   const long order[], size_t item, int *perrval );

ARGUMENTS
  void * dest                  output array (must be allocated), must
                               be different from source array
  const void * src             source array
  const long data_dim[]        dimensions of output array
  const long order[]           index order of source array
  size_t item                  size of an array element

DESCRIPTION

  Like raster_normalization, uses directly configuration order array instead 
  calculating it from configuration raster number.

RETURN VALUE
int status
success : 0
error   : -1

AUTHOR 

AUTHOR
  13-Mar-1998 Peter Boesecke
  17-May-1998 PB calculation of IS for positive order corrected:
  07-Jul-2010 PB extracted from raster_normalization

-----------------------------------------------------------------------------*/
int raster_order_normalization ( void * dest, const void * src,
                                 const long data_dim[],
                                 const long order[], size_t item, int *perrval )
{ static const char * RON_Error  = "ERROR: raster_order_normalization:";
  static const char * RON_NoDim  = "no or zero dimension"; // 1
  static const char * RON_NoMem  = "cannot allocate memory"; // 3
  static const char * RON_Wrap   = "bad wrapping"; // 4
  static const char * RON_NoCpy  = "cannot copy memory";

  long * IS=NULL, * IInc=NULL, * ILoop=NULL, * IWrap=NULL;
  long * I=NULL;

  const void * ps;
  long   n, n_dim;

  long n_element;
  size_t used_size;
  void * tmp_src=NULL;

  long raster_number=0l; // info only

  int errval=0;

  if (RN_debug) printf(" raster_order_normalization BEGIN\n");

  if (!data_dim) {
    fprintf(stderr,"SEVERE %s Data dimension pointer missing!",RON_Error);
    exit(-1);
  }

  if (!order) {
    fprintf(stderr,"SEVERE %s Order array  pointer missing!",RON_Error);
    exit(-1);
  }

  if (!dest) {
    fprintf(stderr,"SEVERE %s Destination pointer missing!",RON_Error);
    exit(-1);
  }

  if (!src) {
    fprintf(stderr,"SEVERE %s Source pointer missing!",RON_Error);
    exit(-1);
  }

  /* get dimension */
  if (data_dim) n = data_dim[0]; else n = 0;
  if (n<=0) { 
    errval=1;
    fprintf(stderr,"%s data_dim %s\n",RON_Error, RON_NoDim);
    goto raster_order_normalization_error;
  } 

  // raster_number 1 is calculated without dimension limit 
  if ( ( src==dest ) && ( raster_order2number( order )==1l ) ) {
    if (RN_debug) {
      printf("         raster_number = %ld\n", 1l);
      printf("         src and dest arrays are identical\n");
      printf("         normalization not required\n");
    }
  } else {

    /* use buffer if src and dest overlap */
    n_element = 1l;
    for (n_dim=1;n_dim<=n;n_dim++) n_element *= data_dim[n_dim];
    used_size = item * (size_t) n_element;

    if ( ovlpchk( dest, used_size, src, used_size ) ) {
      if (RN_debug) 
        printf(" raster_order_normalization: creating temporory copy \n");

      if ( !(tmp_src = malloc( used_size )) ) {
        fprintf(stderr,"%s IS %s\n",RON_Error, RON_NoMem);
        goto raster_order_normalization_error;
      }
      if ( !(memcpy(tmp_src, src, used_size)) ) {
        fprintf(stderr,"%s IS %s\n",RON_Error, RON_NoCpy);
        goto raster_order_normalization_error;
      }
      src = tmp_src;
    }

    /* calculate loop parameters */
    if (!(IS = (long *) malloc ( sizeof(long) * (n+2) )) ) {
      errval=3;
      fprintf(stderr,"%s IS %s\n",RON_Error, RON_NoMem); 
      goto raster_order_normalization_error;
    }
    if (!(IInc = (long *) malloc ( sizeof(long) * (n+2) )) ) {
      errval=3;
      fprintf(stderr,"%s IInc %s\n",RON_Error, RON_NoMem); 
      goto raster_order_normalization_error;
    }
    if (!(ILoop = (long *) malloc ( sizeof(long) * (n+2) )) ) {
      errval=3;
      fprintf(stderr,"%s ILoop %s\n",RON_Error, RON_NoMem); 
      goto raster_order_normalization_error;
    }
    if (!(IWrap = (long *) malloc ( sizeof(long) * (n+2) )) ) {
      errval=3;
      fprintf(stderr,"%s IWrap %s\n",RON_Error, RON_NoMem); 
      goto raster_order_normalization_error;
    }
    if (!(I = (long *) malloc ( sizeof(long) * (n+2) )) ) {
      errval=3;
      fprintf(stderr,"%s I %s\n",RON_Error, RON_NoMem); 
      goto raster_order_normalization_error;
    }

    /* calculate start, increment, loop count */
    for (n_dim=1;n_dim<=n;n_dim++)
      if (order[n_dim]<0) {
        IS[n_dim] = data_dim[-order[n_dim]]; 
        IInc[n_dim] = -1; 
        ILoop[n_dim] = data_dim[-order[n_dim]]; 
      } else { 
        IS[n_dim] = 1;
        IInc[n_dim] = 1; 
        ILoop[n_dim] = data_dim[order[n_dim]]; 
      }

    if ( order2wrap ( IWrap, data_dim, order ) ) {
      fprintf(stderr,"%s %s\n",RON_Error, RON_Wrap); 
      return(-1);
    }

    if (RN_debug) {
      // raster_number is only used as debug info for n<=MAX_RASTER_NUMBER_DIMENSION
      raster_number = raster_order2number ( order );

      for (n_dim=1;n_dim<=n;n_dim++) {
        printf("         raster_number = %ld\n", raster_number);
        printf("         data_dim[%ld] = %ld\n", n_dim, data_dim[n_dim]);
        printf("            order[%ld] = %ld\n", n_dim, order[n_dim]);
        printf("            IWrap[%ld] = %ld\n", n_dim, IWrap[n_dim]);
        printf("               IS[%ld] = %ld\n", n_dim, IS[n_dim]);
        printf("             IInc[%ld] = %ld\n", n_dim, IInc[n_dim]);
        printf("            ILoop[%ld] = %ld\n", n_dim, ILoop[n_dim]);
      } 
    }

    ps = src;
    reorder_raster ( n, dest, &ps, item, I, IS, ILoop, IInc, IWrap );

    free ( I ); free ( IWrap ); free ( ILoop ); free ( IInc ); free ( IS );
    if (tmp_src) free (tmp_src);

  } // if ( src==dest ) ...

  if (perrval) *perrval=errval;

  if (RN_debug) printf(" raster_order_normalization END\n");

  return(0);

raster_order_normalization_error:

  if (I) free ( I ); if (IWrap) free ( IWrap ); if (ILoop) free ( ILoop ); 
  if (IInc) free ( IInc ); if (IS) free ( IS );
  if (tmp_src) free (tmp_src);

  if (perrval) *perrval=errval;

  if (RN_debug) printf(" raster_order_normalization END (errval=%d)\n",errval);

  return(-1);

} /* raster_order_normalization */

/*--------------------------------------------------------------------------
NAME

  raster_order_multiplication --- raster transformation of a configuration order array

SYNOPSIS

  long * raster_order_multiplication (long *buffer, size_t nelem,
                                      const long a_order[] , 
                                      const long x_order[]);

DESCRIPTION

ARGUMENTS
  long *buffer            input buffer (or NULL if not supplied)
  size_t nelem            input buffer length (needed nelem>=a_order[0]+1)
  const long a_order[n]   raster transformation array
  const long x_order[n]   raster configuration array

  The elements 'a_order[0]' and 'x_order[0]' must contain the dimensions. At
  least 'a_order[0]+1' and 'x_order[0]+1' elements must be allocated for
  each array. 

  If buffer is NULL an array with a_order[0]+1 elements is allocated and 
  must be free'd  by the calling program.


RETURN VALUE
  success   long    * x_order
  error     long    NULL

  If the return value is not equal to NULL a pointer to allocated memory is 
  returned that must be free'd by the calling routine.

HISTORY
  2000-07-29 Peter Boesecke
--------------------------------------------------------------------------*/
long * raster_order_multiplication (long *buffer, size_t nelem,
                                    const long a_order[] , const long x_order[])
{ static const char * ROM_Error     = "ERROR: raster_order_multiplication:";
  static const char * ROM_MisMatch  = "dimension mismatch";
  register int i;
  long I;
  long * y_order = (long *) NULL;
  long N = 0l;

  if (!(a_order && x_order)) goto raster_order_multiplication_error;
  if (a_order[0]>x_order[0]) goto raster_order_multiplication_error;
  N = a_order[0];

  if (buffer) {
    if (nelem<=N) {
      fprintf(stderr,"%s %s\n",ROM_Error,ROM_MisMatch);
      goto raster_order_multiplication_error;
    }
    y_order = buffer;
    /* clear y_order */
    for ( i=0;i<=N;i++ ) y_order[i]=0l;
  } else {
    /* allocate array and clear it */
    if ( !(y_order = (long*) calloc( (N+1), sizeof(long) )) )
      goto raster_order_multiplication_error;
  }
  y_order[0] = N;

  for (i=1;i<=N;i++) {
    I = a_order[i];
    if ( labs(I)<=x_order[0] ) {
      if (I>0) y_order[i] = x_order[I];
        else y_order[i] = - x_order[-I];
      } else y_order[i] = 0l; // not defined
    } // for

  return( y_order );

raster_order_multiplication_error:

  if ( (y_order)&&(!buffer) ) free( y_order );

  return( NULL );

} /* raster_order_multiplication */

/*--------------------------------------------------------------------------
NAME

  raster_multiplication --- raster number after a raster transformation

SYNOPSIS

  long raster_multiplication ( long a, long x );

DESCRIPTION
  The resulting raster configuration number of the transformations a*x 
  is returned. x is the input raster configuration number, a is the 
  raster transformation which is applied to x. 

ARGUMENTS
  long      a         raster transformation
  long      x         input raster configuration number

  a and x must be smaller or equal to A(n) = 2^n * (n!)

RETURN VALUE
  success   long    raster_number
  error     long    0

HISTORY
  2000-07-29 Peter Boesecke
--------------------------------------------------------------------------*/
long raster_multiplication ( long a, long x )
{ long * a_order;
  long * x_order;
  long * y_order;
  long value;

  long n = MAX_RASTER_NUMBER_DIMENSION;

  a_order = raster_number2order ( NULL, 0, n , a );
  x_order = raster_number2order ( NULL, 0, n , x );

  y_order = raster_order_multiplication ( NULL, 0, a_order , x_order );
  
  value = raster_order2number ( y_order );

  if (a_order) free ( a_order );
  if (x_order) free ( x_order );
  if (y_order) free ( y_order );

  return ( value );

} /* raster_multiplication */

/*--------------------------------------------------------------------------
NAME

  raster_order_inversion --- returns inverse raster order array 

SYNOPSIS

  long * raster_order_inversion ( long *buffer, size_t nelem,
                                  const long x_order[] );


DESCRIPTION
  Returns the inverted raster order array a_order = x_order^-1,
  with a_order[] * x_order[] = 1[];

ARGUMENTS
  long *buffer           output buffer (or NULL if not supplied)
  size_t nelem           output buffer length (needed nelem>=x_order[0]+1) 
  const long x_order[n]  input raster order array

The element ´x_order[0]´ must contain the dimension. At least ´x_order[0]+1 
elements must be allocated for the array. 

RETURN VALUE
  success   long    * a_order 
  error     long    NULL

If the return value is not equal to NULL a pointer to allocated memory is 
returned that must be free´d by the calling routine.

HISTORY
  2000-07-29 Peter Boesecke
--------------------------------------------------------------------------*/
long * raster_order_inversion ( long *buffer, size_t nelem,
                                const long x_order[] )
{ static const char * ROI_Error     = "ERROR: raster_order_inversion:";
  static const char * ROI_MisMatch  = "dimension mismatch";
  register int i;
  long I;
  long * a_order = (long *) NULL;
  long N = 0l;

  if (!(x_order)) return( 0 );
  N = x_order[0];

  if (buffer) {
    if (nelem<=N) {
      fprintf(stderr,"%s %s\n",ROI_Error,ROI_MisMatch);
      goto raster_order_inversion_error;
    }
    a_order = buffer;
    /* clear a_order */
    for ( i=0;i<=N;i++ ) a_order[i]=0l;
  } else {
    /* allocate array and clear it */
    if ( !(a_order = (long*) calloc( (N+1), sizeof(long) )) )
      goto raster_order_inversion_error;
  }
  a_order[0] = N;

  for ( i=1;i<=N;i++ ) {
    I = x_order[i];
    if ( labs(I)<=x_order[0] ) {
      if (I>0) a_order[I] = i;
        else a_order[-I] = -i;
    } else a_order[i] = 0l; // not defined
  } // for

  return( a_order );

raster_order_inversion_error:

  if ( (a_order)&&(!buffer) ) free( a_order );

  return( NULL );

} /* raster_order_inversion */

/*--------------------------------------------------------------------------
NAME

  raster_order_determinante --- returns determinante

SYNOPSIS

  long raster_order_determinante ( const long order[] );


DESCRIPTION
  Returns the determinante of raster order array order[]. 

ARGUMENTS
  const long order[]  input raster order array

The element order[0] must contain the dimension. At least order[0]+1
elements must be allocated for the array.

RETURN VALUE
  success   long    determinante
  error     long    0

HISTORY
  2011-02-25 Peter Boesecke
--------------------------------------------------------------------------*/
long raster_order_determinante ( const long order[] )
{ long N;
  long l, m, n, sign;
  long o, osign;
  long *order_copy=NULL;
  long Aln, determinante=0l;

  N=order[0];

  // duplicate order array
  if ( !(order_copy=malloc(sizeof(long)*(N+1))) )
    goto order_determinante_error;
  for (l=0;l<=N;l++) order_copy[l]=order[l];

  determinante=1;

  // development after Laplace using first column of each adjunct matrix
  for (l=1;l<=N;l++) {
    // l-th column, n-th row
    n = order_copy[l];
    if (n<0) {
      sign=-1l;n=-n;
    } else sign=1l;
    Aln = (n%2)?1l:-1l; // it is always the first column

    determinante*=Aln*sign;

    // calculate adjunct matrix 
    for (m=l+1;m<=N;m++) {
      o=order_copy[m];
      if (o<0l) {
        osign=-1l; o=-o;
      } else osign=1l;
      if (o>n) order_copy[m]=(o-1l)*osign;
    }
  }

  free(order_copy);

  return(determinante);

order_determinante_error:

  if (order_copy) free(order_copy);

  return(0l);

} // raster_order_determinante

/*--------------------------------------------------------------------------
NAME

  raster_inversion --- returns raster number of the inverse transformation

SYNOPSIS

  long raster_inversion ( long x  );

DESCRIPTION
  The raster number of the transformation a = x^-1 is returned
  that transforms configuration x to 1: a * x = 1 

ARGUMENTS
  long      x         input raster configuration number

RETURN VALUE
  success   long    inverse raster configuration number
  error     long    0

HISTORY
  2000-07-29 Peter Boesecke
--------------------------------------------------------------------------*/
long raster_inversion ( long x )
{ long * a_order;
  long * x_order;
  long value;

  long n = MAX_RASTER_NUMBER_DIMENSION;

  x_order = raster_number2order ( NULL, 0, n , x );

  a_order = raster_order_inversion ( NULL, 0, x_order );

  value = raster_order2number ( a_order );

  if (a_order) free ( a_order );
  if (x_order) free ( x_order );

  return ( value );

} /* raster_inversion */

/*---------------------------------------------------------------------------
NAME

   raster_str2order --- read an order array from a string

SYNOPSIS

  long * raster_str2order( long * buffer, size_t nelem,
                           long n, const char *str,
                           const char **tail, int *perrval )


ARGUMENTS

  long * buffer (NULL to allocate new output buffer)

    If buffer is NULL an output order array with n+1 long elements is allocated
    and must be released explicitely. If buffer is not NULL n+1 long elements 
    will be used for the outut order array.

  long n
  
    number of axes

  const char *str

    input character string

  const char **tail (NULL to ignore)

    outputs remaining string

  int *perrval (NULL to ignore)

    output error codes 

    *perrval : 0 OK
               1 memory allocation error (output array)
               2 memory allocation error (internal)
               3 explicit duplication
               4 outside range
               5 implicit duplication
               6 not enough parameters
               7 conversion OK, but too many parameters
               8 conversion from raster number failed
               9 buffer size too small (nelem<n+1)

DESCRIPTION

  reads n comma separated values as indices from a string or a single 
  raster_number

  special cases:
  for n=1: only positive raster numbers (1 or 2) or indices (1, -1) are accepted.
  allowed input: 1, -1 (first axis inverted, i.e. raster number 2)
                 and 2 (2 is interpreted as raster number)
                 (maximum 1 number, everything that follows is ignored)
 
  for n>1: positive and negative (inverse) raster numbers or comma separated
           indices are accepted.
  If 2 or more numbers are given, they are used as indices in an order array, 
  if only 1 number is given it is interpreted as raster number and is 
  converted to an order array. Single negative numbers are interpreted as 
  inverse raster number.

  allowed input: <index>,<index>[,<index>] (maximum n indices)
             or  <raster number>
             (everything that follows is ignored)

  Whenever the conversion was successful a pointer different from NULL is returned.
  To check that the rest of the string is clean, check the length of **tail or *perrval.

RETURN VALUE
  Returns the pointer to the order array or NULL in case of a conversion 
  error. In case of NULL **tail returns the original pointer to the string.

HISTORY
  2010-07-06 Peter Boesecke
---------------------------------------------------------------------------*/
long * raster_str2order( long * buffer, size_t nelem,
                         long n, const char *str,
                         const char **tail, int *perrval )
{ // static const char * RSO_Error     = "ERROR: raster_str2order:";
  long *order=NULL, *index=NULL;
  long tmp, raster_number=0l;
  const char *ps="";
  int errval=0;
  int i, cnt=0;
  int no_more_parameters_allowed=0;
  
  if (buffer) {
    if (nelem<=n) {
      errval=9;
      goto raster_string2order_error;
    }
    order=buffer;
  } else {
    if ( !(order = (long*)  calloc( n+1l, sizeof(long) )) ) {
      errval=1;  // allocation error
      goto raster_string2order_error;
    }
  }

  if ( !(index = (long*)  calloc( n+1l, sizeof(long) )) ) {
    errval=2;  // allocation error
    goto raster_string2order_error;
  }

  if (str) ps=str;

  if (n>=0) order[0] = n;
  for (i=1;i<=n;i++) {
    if (*ps) {
      if (no_more_parameters_allowed) {
        errval=7; // too many parameters, but result OK
        break;
      } else {
        tmp = num_str2long( ps, &ps, &errval);
        if (errval) {
          errval+=1000;
          goto raster_string2order_error;
        }
        cnt++;
        if (cnt==n) no_more_parameters_allowed=1;
        if (i==1) {
          // keep in mind 1st value, it could be the raster number
          // for n>=2 a negative raster number is the inverse raster number
          if (n>1) { 
            if (tmp) raster_number = tmp; 
          } else if (0<tmp) raster_number = tmp;
        }
        if (( 0<labs(tmp) ) && ( labs(tmp)<=n )) {
          if (!index[labs(tmp)]) {
            order[i] = tmp;
            index[labs(tmp)] = i*(tmp<0)?-1:1;
          } else {
            errval=3; // duplication
            goto raster_string2order_error;
          }
        } else {
          if ( (i==1) && (raster_number!=0l) ) {
            no_more_parameters_allowed=1;
            // 1st parameter is raster number (parameter could follow)
            if ( !raster_number2order ( order , n+1, n, raster_number ) ) {
               errval=8;
              goto raster_string2order_error;
            }
            cnt=n;
          } else {
            errval=4; // outside range
            goto raster_string2order_error;
          }
        }
        while (isspace(*ps)) ps++; // skip trailing white space
        if ( (*ps)&&(no_more_parameters_allowed) ) {
          errval=7; // too many parameters, but result OK 
          break;
        }
        if (*ps==',') ps++; // skip a comma
      }
    } else {
      // no more parameters
      if ( (i==2) && (raster_number!=0) ) {
        // 1st parameter is raster number (last parameter)
        if ( !raster_number2order ( order , n+1, n, raster_number ) ) {
           errval=8;
          goto raster_string2order_error;
        }
        cnt=n;
        break; // ready, order calculated from raster_number  
      }
      if (!index[i]) {
        order[i] = i;
        index[i] = i;
      } else {
        errval=5; // duplication
        goto raster_string2order_error;
      }
    }
  }

  if (cnt<n) {
    errval=6; // not enough parameters
    goto raster_string2order_error;
  }

  free( index );

  if (tail) *tail = ps;

  if (perrval) *perrval=errval;
  
  return( order );

raster_string2order_error:

  // String could not be read

  if ( (order)&&(!buffer) ) free( order );

  if (index) free( index );

  if (tail) *tail = str; // the whole string could not be read

  if (perrval) *perrval=errval;

  return( NULL );

} // raster_str2order

/*---------------------------------------------------------------------------
NAME

   raster_str2number --- convert a string to a raster number

SYNOPSIS

  long raster_str2number( long n, const char *str, const char **tail, int *perrval );

ARGUMENTS

  long n
  
    number of axes

  const char *str

    input character string

  const char **tail (NULL to ignore)

    outputs remaining string

  int *perrval (NULL to ignore)

    output error codes 

    see: raster_str2order

DESCRIPTION

  Combines the functions raster_str2order and raster_order2number and
  converts a string directly to a raster number.

  Reads n comma separated values as indices from a string or a single 
  raster_number

  special cases:
  for n=1: only positive raster numbers (1 or 2) or indices (1, -1) are accepted.
  allowed input: 1, -1 (first axis inverted, i.e. raster number 2)
                 and 2 (2 is interpreted as raster number)
                 (maximum 1 number, everything that follows is ignored)

  for n>1: positive and negative (inverse) raster numbers or comma separated
           indices are accepted.
  If 2 or more numbers are given, they are used as indices in an order array, 
  if only 1 number is given it is interpreted as raster number and is 
  converted to an order array. Single negative numbers are interpreted as 
  inverse raster number.

  allowed input: <index>,<index>[,<index>] (maximum n indices)
             or  <raster number>
             (everything that follows is ignored)

  Whenever the conversion was successful a pointer different from NULL is returned.
  To check that the rest of the string is clean, check the length of **tail or *perrval.

RETURN VALUE
  Returns a (positive) raster number. In case of a conversion error the returned value
  is 0 and **tail returns the original pointer to the string.

HISTORY
  2011-04-04 Peter Boesecke
---------------------------------------------------------------------------*/
long raster_str2number( long n, const char *str, const char **tail, int *perrval )
{
  long *order=NULL;
  long raster_number=0l;
  int errval=-1;

  order = raster_str2order( NULL, 0, n, str, tail, &errval );
  if (errval) goto raster_str2number_error;

  raster_number = raster_order2number ( order );

  free(order);

  if (perrval) *perrval=errval;

  return( raster_number );

raster_str2number_error:

  if (perrval) *perrval=errval;

  if (order) free(order);

  return(0l);

} // raster_str2number

/*--------------------------------------------------------------------------
NAME

  raster_order2str --- writes order into a comma separated string 

SYNOPSIS

  char * raster_order2str( char * buffer, size_t nelem,
                           const long order[], int *perrval );


DESCRIPTION
  The contents of the array order[] is written to a string and returned. If no 
  buffer is NULL the required memory is allocated and must be released externally. 
  nelem is the number of allocated char elements (ignored in case of buffer==NULL).

ARGUMENTS
  char *    buffer    output buffer with nelem characters (ignored if NULL)
  size_t    nelem     number of elements in buffer (ignored if buffer is NULL)
  const char * str    output order string (buffer or allocated string, if buffer is NULL)
  int * perrval       output error value
                      0: OK
                      1: allocation error
                      2: string buffer too small

RETURN VALUE
  NULL      error
  string    success    
  
HISTORY
  2010-07-06 Peter Boesecke
--------------------------------------------------------------------------*/
char * raster_order2str( char * buffer, size_t nelem,
                         const long order[], int *perrval )
{
# define BUFLEN 1024 
  register int i;
  int  errval=0;

  char * value, * ps;

  size_t valsiz, psiz, siz=0l;
  long N, log10, n;

  // calculate needed string length
  N=order[0];
  log10=floor(log(N)/log(10)); 
  n=exp(log(10)*log10);

  while (log10>=0) {
    siz+=(1l+N-n)*(log10+3l);

    N=n-1l;
    log10-=1l;
    n/=10l;

  } // while

  if (buffer) {
    value=buffer;
    valsiz=nelem;
  } else {
    if ( !(value = (char*)  malloc((siz+1l)*sizeof(char)) ) ) {
      errval=1;  // allocation error
      goto raster_order2str_error;
    }
    valsiz=siz;
  }

  if (valsiz<1) {
    errval=2; // string buffer too small
    goto raster_order2str_error;
  }

  value[0]='\0';
  ps=value;
  psiz=valsiz;
  for (i=1;i<=order[0];i++) {
    if (i==1) snprintf(ps,valsiz,"%ld",order[i]);
    else snprintf(ps,valsiz,",%ld",order[i]);
    ps=value+strlen(value);
    psiz=valsiz-strlen(value);
  }

  if (perrval) *perrval=errval;

  return( value );

raster_order2str_error:

  // String could not be created

//  if ( (order)&&(!buffer) ) free( order ); ++++++++??????????????
  if ( (value)&&(!buffer) ) free( value );

  if (perrval) *perrval=errval;

  return( NULL );

} // raster_order2str

/*--------------------------------------------------------------------------
NAME

  order2matrix --- representation of a configuration order array as a NxN matrix

SYNOPSIS

  double * raster_order2matrix ( double * buffer, size_t nelem, 
                                 const long order[N+1] );

DESCRIPTION

  Transforms a configuration order array for N dimensions into a NxN matrix
  that can be used for coordinate transformations.

ARGUMENTS
  long      *buffer      output buffer (or NULL if not supplied)
  size_t    nelem        output buffer length (needed nelem>=(order[0]*order[0])
  long      order[N+1]   raster configuration array

  The element 'order[0]' must contain the dimension N (number of coordinates)

  If buffer is NULL an array with N*N elements (N=order[0]) is allocated and
  must be released by the calling program.

RETURN VALUE
  success   double    mat[N][N]
  error     double    NULL

HISTORY
  2011-02-23 Peter Boesecke
--------------------------------------------------------------------------*/
double * raster_order2matrix ( double * buffer, size_t nelem,
                               const long order[] )
{ static const char * R2M_Error="ERROR: raster_order2matrix:";
  static const char * R2M_NoOrder="order is NULL pointer:";
  static const char * R2M_MisMatch="dimension mismatch";
  static const char * R2M_OutOfBounds="order index out of bounds";
  long l, n;
  long N;

  double *Out = (double *) NULL, *pOut;
  double sign;

  if (RN_debug) printf(" raster_order2matrix BEGIN\n");

  if (!order) {
    fprintf(stderr,"%s %s\n",R2M_Error,R2M_NoOrder);
    goto raster_order2matrix_error;
  }

  N = order[0];

  if (buffer) {
    if (nelem<N*N) {
      fprintf(stderr,"%s %s\n",R2M_Error,R2M_MisMatch);
      goto raster_order2matrix_error;
    }
    Out=buffer;
  } else {
    /* allocate array */
    if ( !(Out = (double*) malloc( sizeof(double) * (N*N) )) )
      goto raster_order2matrix_error;
  }

  // Clear matrix Out
  pOut=Out;
  for (n=0;n<N;n++)
    for (l=0;l<N;l++) *pOut++=0.0;

  // Set matrix elements
  for (l=0;l<N;l++) {
    n = order[l+1l];
    if (n<0) {
      sign=-1.0;n=-n;
    } else sign=1.0;
    if (n>N) {
      fprintf(stderr,"%s %s\n",R2M_Error,R2M_OutOfBounds);
      goto raster_order2matrix_error;
    }
    n--; // first index is 0
    Out[l*N+n] = sign; // Out[l][n]
  }

  if (RN_debug) printf(" raster_order2matrix END\n");

  return( Out );

raster_order2matrix_error:

  if (Out) free( Out );

  if (RN_debug) printf(" raster_order2matrix END (error)\n");

  return( NULL );

} // raster_order2matrix

/*+++------------------------------------------------------------------------
NAME
  raster_matrix_product --- product of two matrices

SYNOPSIS

  double * raster_matrix_product ( double * buffer, size_t nelem, 
                                   double A[M][N], double B[L][M],
                                   int L, int M, int N );

DESCRIPTION

  Out[L][N] = A[M][N]*B[L][M] | summed over M

ARGUMENTS
  long      *buffer      output buffer (or NULL if not supplied)
  size_t    nelem        output buffer length (needed nelem>=L*N)
  double    A[M][N]      MxN matrix
  double    B[L][M]      LxM matrix

  If buffer is NULL an array with N*N elements (N=order[0]) is allocated and
  must be released by the calling program.

RETURN VALUE
  success:   Out[L][N]   LxN matrix ( Out[l][n] += A[m][n]*B[l][m] )
  otherwise: NULL

----------------------------------------------------------------------------*/
double * raster_matrix_product ( double * buffer, size_t nelem,
                                 double A[], double B[], int L, int M, int N )
{ static const char * RMP_Error="ERROR: raster_matrix_product:";
  static const char * RMP_MisMatch  = "dimension mismatch";
  double *Out = (double *) NULL;
  int l, m, n;

  if (RN_debug) printf(" raster_matrix_product BEGIN\n");

  if (buffer) {
    if (nelem<L*N) {
      fprintf(stderr,"%s %s\n",RMP_Error,RMP_MisMatch);
      goto raster_matrix_product_error;
    }
    Out=buffer;
  } else {
    /* allocate array */
    if ( !(Out = (double*) malloc( sizeof(double) * (L*N) )) )
      goto raster_matrix_product_error;
  }

  for (n=0;n<N;n++)
    for (l=0;l<L;l++) {
      Out[l*N+n]=0.0; // Out[l][n] = 0.0
      for  (m=0;m<M;m++)
        Out[l*N+n]+=A[m*N+n]*B[l*M+m]; // Out[l][n] += A[m][n]*B[l][m]
    }

  if (RN_debug) printf(" raster_matrix_product END\n");

  return( Out );

raster_matrix_product_error:

  if ( Out ) free ( Out );

  if (RN_debug) printf(" raster_matrix_product END (error)\n");

  return( NULL );

} // raster_matrix_product

/*+++------------------------------------------------------------------------
NAME

  raster_fprint_matrix --- print the M[L][N] matrix

SYNOPSIS

  int raster_fprint_matrix( FILE *out, size_t nrows, size_t ncols,
                            double M[], const char * label );

DESCRIPTION

  Print the M[L][N] matrix with a label

ARGUMENTS
  FILE      *out         output file
  size_t    nrows        N number or rows
  size_t    ncols        L number of columns
  double    M[L][N]      LxN matrix
  const char * label     label

RETURN VALUE
  success:   0

----------------------------------------------------------------------------*/
int raster_fprint_matrix( FILE *out, size_t nrows, size_t ncols,
                          double M[], const char * label )
{ size_t row, col;
  if (RN_debug) printf(" raster_fprint_matrix BEGIN\n");
  // show data
  fprintf(out,"%14s \n",label);
  for (row=0;row<nrows;row++) {
    for (col=0;col<ncols;col++)
      fprintf(out,"%14g ",M[col*nrows+row]); // M[col][row]
    fprintf(out,"\n");
  }
  printf("\n");

  if (RN_debug) printf(" raster_fprint_matrix END\n");
  return(0);

} // raster_fprint_matrix

const char *raster_version ( void )
{ return(RASTER_VERSION); }

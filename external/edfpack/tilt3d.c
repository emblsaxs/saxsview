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

# define TILT3D_VERSION "tilt3d : Peter Boesecke V1.11 2011-06-16"
/*---------------------------------------------------------------------------
NAME

   tilt3d - Conversion between tilt1f, tilt2f, tilt3f and 3d rotation matrix.


SYNOPSIS


DESCRIPTION

The angles tilt1f and tilt2f are defined by fit2d to calibrate the
inclination of a detector with respect to an ideal orientation.

The axes 1, 2 and 3 define a right-handed orthogonal system in the 
laboratory space. The angle tilt1f is the ccw rotation of the inclination 
axis around lab axis 3, starting from axis 2 (see *). Tilt2f is the angle 
between the detector normal and axis 3. tilt3f is a ccw rotation around 
axis 3. 

The third angle ("tilt3f") that rotates the detector in the laboratory 
around the primary beam is missing in the fit2d description. It is 
added here to define bijective transformations.

*) This is equivalent to a ccw rotation of the tilting plane around axis 3, 
starting from axis 1. Because tilt1 is defined here as the rotation of the
tilt axis starting from axis 1 there is a difference by pi/2 to the fit2d 
definition (rotation of the tilt "plane" starting from axis 1). The 
internally used angle tilt1 needs therefore to be redefined as

  tilt1=tilt1f+pi/2.

(see A. Hammersley, High Pressure Research, 1996, Vol. 14, pp. 235-248)

tilt3d_matrix calculates the rotation matrix is calculated for the 
angles tilt1f, tilt2f and tilt3f. 

tilt3d_inverse_matrix calculates the inverse rotation matrix.

tilt3d_angles determines the tilt from a given rotation matrix.

fit2d (rotations around detector axes, origin in detector plane)
  with tilt1=tilt1f+pi/2, tilt2=tilt2f, tilt3=tilt3f:

    rotation angle of tilt axis on detector (ccw around axis3):

                    |  cos(tilt1) -sin(tilt1)         0.0 |
      T1(tilt1) =   |  sin(tilt1)  cos(tilt1)         0.0 |
                    |        0.0         0.0          1.0 |

    inclination of detector plane (ccw around axis1' after Tilt1):

                    |         1.0        0.0          0.0 |
      T2(tilt2) =   |         0.0  cos(tilt2) -sin(tilt2) |
                    |         0.0  sin(tilt2)  cos(tilt2) |

    rotation angle of detector (ccw around axis 3):

                    |  cos(tilt3) -sin(tilt3)         0.0 | 
      T3(tilt3) =   |  sin(tilt3)  cos(tilt3)         0.0 |
                    |        0.0         0.0          1.0 |                  

    principal (output) ranges:

     tilt1f=Tilt[0] ] -Pi .. +Pi ]
     tilt2f=Tilt[1] [  0  .. +Pi ]
     tilt3f=Tilt[2] ] -Pi .. +Pi ]

 all rotations:

  T[tilt1, tilt2, tilt3] = T3(tilt3).T1(tilt1).T2(tilt2).T1(-tilt1)

  t11 = T[0][0] =  cos(tilt1)             cos(tilt1+tilt3) + sin(tilt1) cos(tilt2) sin(tilt1+tilt3)
  t12 = T[1][0] =  sin(tilt1)             cos(tilt1+tilt3) - cos(tilt1) cos(tilt2) sin(tilt1+tilt3)
  t13 = T[2][0] =              sin(tilt2) sin(tilt1+tilt3)

  t21 = T[0][1] = -sin(tilt1)  cos(tilt2) cos(tilt1+tilt3) + cos(tilt1)            sin(tilt1+tilt3)
  t22 = T[1][1] =  cos(tilt1)  cos(tilt2) cos(tilt1+tilt3) + sin(tilt1)            sin(tilt1+tilt3)
  t23 = T[2][1] =             -sin(tilt2) cos(tilt1+tilt3)

  t31 = T[0][2] = -sin(tilt1)  sin(tilt2)
  t32 = T[1][2] =  cos(tilt1)  sin(tilt2)
  t33 = T[2][2] =  cos(tilt2)

                         | T[0][0] T[1][0] T[2][0] |
  T(tilt1,tilt2,tilt3) = | T[0][1] T[1][1] T[2][1] |
                         | T[0][2] T[1][2] T[2][2] |

                         | t11     t12     t13 |
                       = | t21     t22     t23 |
                         | t31     t32     t33 |

                         |  c1 c13 + s1 c2 s13     s1 c13 - c1 c2 s13      s2 s13 |
                       = | -s1 c2 c13 + c1 s13     c1 c2 c13 + s1 s13     -s2 c13 |
                         | -s1 s2                  c1 s2                   c2     |

  (c13=cos(tilt1+tilt3), s13=sin(tilt1+tilt3), c1=cos(tilt1), s1=sin(tilt1), c2=cos(tilt2) etc.)

  principal ranges of the angles tilt1, tilt2, tilt3:

    tilt1  ] -Pi .. +Pi ]
    tilt2  [   0 .. +Pi ]
    tilt3  ] -Pi .. +Pi ]

Tilt1[R_] := 
 If[1 - Abs[R[[3, 3]]] > Eps, ArcTan[R[[3, 2]], -R[[3, 1]]], 
   If[Abs[1 - R[[3, 3]]] < Eps, 
    0, (ArcTan[R[[1, 1]], R[[1, 2]]] - tilt3 + 2 Pi)/2]] /. {tilt3 -> 
    0}
Tilt2[R_] := 
 If[Abs[Sin[Tilt1[R]]] > Eps, 
  ArcTan[R[[3, 3]], -R[[3, 1]]/Sin[Tilt1[R]]], 
  ArcTan[R[[3, 3]], R[[3, 2]]/Cos[Tilt1[R]]]]
Tilt3[R_] := 
 If[Abs[Sin[Tilt2[R]]] > Eps, 
  ArcTan[-R[[2, 3]]/Sin[Tilt2[R]], R[[1, 3]]/Sin[Tilt2[R]]] - 
   Tilt1[R], 
  If[Cos[Tilt2[R]] > 0, ArcTan[R[[2, 2]], R[[2, 1]]], 
   ArcTan[R[[1, 1]], R[[1, 2]]] - 2 Tilt1[R]]]

History

  2010-05-10 Peter Boesecke creation
  2010-05-17 PB V1.0
  2011-04-18 PB V1.0 tilt3d_version() added
  2011-06-01 PB V1.01 suggested parentheses added 
  2011-06-15 PB V1.1 tilt1 axis -> tilt1f plane (pi/2 difference)
  2011-06-15 PB V1.11 tilt1 calculation corrected

---------------------------------------------------------------------------*/

/***************************************************************************
* Include                                                                  *
***************************************************************************/
# include "tilt3d.h"

/***************************************************************************
* Internal                                                                 *
***************************************************************************/

/****************************************************************************
* Static Variables and Numbers                                              *
****************************************************************************/
static double tilt3d_eps=1e-8;
static double tilt3d_pi=3.1415926535897932384626;

/****************************************************************************
* Routines                                                                  *
****************************************************************************/

/*--------------------------------------------------------------------------
NAME

  tilt3d_version --- returns pointer to the version string

SYNOPSIS

  const char *tilt3d_version ( void );

DESCRPTION

  Returns pointer to the version string.

--------------------------------------------------------------------------*/
const char *tilt3d_version ( void )
{
  return ( TILT3D_VERSION );
} /* tilt3d_version */

/*---------------------------------------------------------------------------
NAME

  tilt3d_determinante

SYNOPSIS

  double tilt3d_determinante ( double A[3][3] )

DESCRIPTION

  input: double A[3][3] (3d matrix)

RETURN VALUE

  determinante of A 

---------------------------------------------------------------------------*/
double tilt3d_determinante ( double A[3][3] )
{ double determinante=0.0;

  if (!A) {
    fprintf( stderr, "ERROR: tilt3d_determinante: NULL pointer\n" );
    goto tilt3d_determinante_error;
  }

  determinante  =  A[0][0]*(A[1][1]*A[2][2]-A[2][1]*A[1][2]);
  determinante -=  A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0]);
  determinante +=  A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);

tilt3d_determinante_error:

  return( determinante );

} // tilt3d_determinante

/*---------------------------------------------------------------------------
NAME

  tilt3d_mat_scale

SYNOPSIS

  int tilt3d_mat_scale ( double A[3][3], double scale )

DESCRIPTION

  Multiplies each element of A with scale.

  input: double A[3][3] (3d matrix)
         double scale (scale factor)
  updated: A

RETURN VALUE

  status

---------------------------------------------------------------------------*/
int tilt3d_mat_scale ( double A[3][3], double scale )
{ int status=-1;
  int i,j;

  if (!A) {
    fprintf( stderr, "ERROR: tilt3d_mat_scale: NULL pointer\n" );
    goto tilt3d_mat_scale_error;
  }

  if (scale!=1.0)
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
        A[i][j] *= scale;

  status = 0;

tilt3d_mat_scale_error:

  return( status );

} // tilt3d_mat_scale

/*+++------------------------------------------------------------------------
NAME
  tilt3d_mat_transpose --- transpose a 3-dimensional matrix

SYNOPSIS

  int tilt3d_mat_transpose ( double A[3][3] )

DESCRIPTION

  A[3][3] = Transpose(A[3][3])

RETURN VALUE
  status

----------------------------------------------------------------------------*/
int tilt3d_mat_transpose ( double A[3][3] )
{ int j,k;
  double tmp;
  for (j=0;j<3;j++)
    for (k=0;k<j;k++) {
      tmp=A[j][k];
      A[j][k] = A[k][j];
      A[k][j] = tmp;
    }
  return(0);
} // tilt3d_mat_transpose

/*---------------------------------------------------------------------------
NAME

  tilt3d_isnottilt

SYNOPSIS

  int tilt3d_isnottilt ( double R[3][3] )

DESCRIPTION

  Returns 1 if R is a rotation

RETURN VALUE

  is rotation 0
  no rotation 1
  error      -1

---------------------------------------------------------------------------*/
int tilt3d_isnottilt ( double R[3][3] )
{ int is_not_tilt=-1;
  int i,j; 
  double len;

  if (!R) {
    fprintf( stderr, "ERROR:  tilt3d_isnottilt: NULL pointer\n" );
    goto  tilt3d_isnottilt_error;
  }

  if ( fabs(tilt3d_determinante(R)-1.0)>tilt3d_eps )
    is_not_tilt=1;
  else {
    is_not_tilt=0;
    for (i=0;i<3;i++) {
      len=0.0;
      for (j=0;j<3;j++)
        len+=R[i][j]*R[i][j];
      if (fabs(len-1.0)>tilt3d_eps) { 
        is_not_tilt=1; 
        break; 
      }
    }
  }

tilt3d_isnottilt_error:

  return( is_not_tilt );
  
} // tilt3d_isnottilt

/*---------------------------------------------------------------------------
NAME

  tilt3d_matrix 

SYNOPSIS

  int tilt3d_matrix(double TILT[3], double R[3][3]);

DESCRIPTION

  input:   double TILT[3] (angles tilt1f, tilt2f, tilt3f)
  updated: double R[3][3] (3d rotation matrix)

RETURN VALUE

  status

---------------------------------------------------------------------------*/
int tilt3d_matrix(double TILT[3], double R[3][3])
{ int status=-1;
  double c1, c2, c3, c13;
  double s1, s2, s3, s13;
  double tilt1, tilt2, tilt3;

  if ((!TILT)||(!R)) {
    fprintf( stderr, "ERROR: tilt3d_matrix: NULL pointer\n" );
    goto tilt3d_matrix_error;
  }

  // tilt1 = TILT[0]+pi/2 = tilt1f+pi/2
  tilt1=TILT[0]+tilt3d_pi*0.5;
  tilt2=TILT[1];
  tilt3=TILT[2];

  c1 = cos(tilt1); c2 = cos(tilt2); c3 = cos(tilt3);
  s1 = sin(tilt1); s2 = sin(tilt2); s3 = sin(tilt3);
  c13 = cos(tilt1+tilt3); s13 = sin(tilt1+tilt3);

  R[0][0] =  c1 * c13 + s1 * c2 * s13;
  R[1][0] =  s1 * c13 - c1 * c2 * s13;
  R[2][0] =  s2 * s13;

  R[0][1] = -s1 * c2 * c13 + c1 * s13;
  R[1][1] =  c1 * c2 * c13 + s1 * s13;
  R[2][1] = -s2 * c13;

  R[0][2] = -s1 * s2;
  R[1][2] =  c1 * s2;
  R[2][2] =  c2;

  status = 0;

tilt3d_matrix_error:

  return( status );

} //  tilt3d_matrix

/*---------------------------------------------------------------------------
NAME
  
  tilt3d_inverse_matrix 

SYNOPSIS 

  int tilt3d_inverse_matrix(double TILT[3], double R[3][3]);

DESCRIPTION

  input:   double TILT[3]  (angles tilt1f, tilt2, tilt3)
  updated: double R[3][3] (inverse 3d rotation matrix)

RETURN VALUE

  status

---------------------------------------------------------------------------*/
int tilt3d_inverse_matrix(double TILT[3], double R[3][3])
{ int status=-1;

  if ( (status=tilt3d_matrix(TILT, R)) ) goto tilt3d_inverse_matrix_error;
  if ( (status=tilt3d_mat_transpose (R)) ) goto tilt3d_inverse_matrix_error;

tilt3d_inverse_matrix_error:

  return( status );

} //  tilt3d_inverse_matrix

/*---------------------------------------------------------------------------
NAME

   tilt3d_angles

SYNOPSIS

  int tilt3d_angles(double TILT[3], double R[3][3]);

DESCRIPTION

  input:    double R[3][3]  (rotation matrix)
  output:   double TILT[3]  (angles tilt1f, tilt2f, tilt3f)

  Calculate Tilt[] from an arbitrary 3d rotation matrix R[][].

  t11 -> R[0][0]
  t12 -> R[1][0]
  t13 -> R[2][0] 

  t21 -> R[0][1]
  t22 -> R[1][1]
  t23 -> R[2][1] 

  t31 -> R[0][2]
  t32 -> R[1][2]
  t33 -> R[2][2]

  Tilt1[R_] := 
   If [ ( 1 - Abs[t33] ) > Eps, 
        ArcTan[t32, -t31], 
        If [ t33 > 0, 
             0, 
             (If [ t12 >= 0, 
                   ArcTan[t11, t12], 
                   ArcTan[t11, t12] + 2 Pi
                 ] - tilt3)/2
           ]
      ] /. {tilt3 -> 0}

  Tilt2[R_] := 
   If [ Abs[Sin[Tilt1[R]]] > Eps, 
        ArcTan[t33, -t31/Sin[Tilt1[R]]], 
        ArcTan[t33,  t32/Cos[Tilt1[R]]]
      ]

  if (tilt2<0) tilt1+=Pi and recalculate tilt2

  Tilt3[R_] := 
   If [ Abs[Sin[Tilt2[R]]] > Eps, 
        ArcTan[-t23/Sin[Tilt2[R]], t13/Sin[Tilt2[R]]] - Tilt1[R], 
        If [ t33 > 0, 
             ArcTan[t22, t21], 
             If [ t12 > 0, 
                  ArcTan[t11, t12] - 2 Tilt1[R], 
                  ArcTan[t11, t12] - 2 Tilt1[R] + 2 Pi
                ]
           ]
      ]
  
  updated:   double TILT[3]  (angles tilt1f, tilt2, tilt3)
  input:     double R[3][3] (3d rotation matrix)

  principal ranges of the output Tilt[] angles:

    Tilt[0] ] -Pi .. +Pi ] (Tilt[0]=tilt1-pi/2)
    Tilt[1] [  0  .. +Pi ] (Tilt[1]=tilt2)
    Tilt[2] ] -Pi .. +Pi ] (Tilt[2]=tilt3)

RETURN VALUE

  status

---------------------------------------------------------------------------*/
int tilt3d_angles(double TILT[3], double R[3][3])
{ int status=-1;
  double tilt1=0.0, tilt2=0.0, tilt3=0.0; // set defaults
  double c1;
  double s1, s2;
  double determinante=0.0, scale=1.0;

  if ((!TILT)||(!R)) {
    fprintf( stderr, "ERROR: tilt3d_angles: NULL pointer\n" );
    goto tilt3d_angle_error;
  }

  determinante =  tilt3d_determinante( R );

  if ( determinante < tilt3d_eps ) {
    fprintf( stderr, "ERROR: tilt3d_angles: Det(R) = %lg is not positive\n", 
             determinante );
    goto tilt3d_angle_error;
  }

  scale = pow( determinante, 1.0/3.0 );

  // TILT[0] = tilt1f = tilt1-pi/2
  TILT[0] = tilt1-tilt3d_pi*0.5;
  TILT[1] = tilt2;
  TILT[2] = tilt3;

  if ( (status=tilt3d_mat_scale ( R, scale )) ) {
    goto tilt3d_angle_error;
  }

  if ( tilt3d_isnottilt ( R ) ) {
    fprintf( stderr, "ERROR: tilt3d_angles: R is not a rotation matrix\n");
    goto tilt3d_angle_error;
  }

  // tilt1 (default tilt3->0)
   if ( ( 1.0 - fabs(R[2][2]) ) > tilt3d_eps ) {
     tilt1 = atan2(-R[0][2],R[1][2]);
   } else {
     if ( R[2][2] > 0.0 ) {
       tilt1 = 0.0;
     } else {
       if ( R[1][0] >= 0.0 ) {
         tilt1 = (atan2(R[1][0],R[0][0]) - tilt3)*0.5;
       } else {
         tilt1 = (atan2(R[1][0],R[0][0]) + 2.0*tilt3d_pi - tilt3)*0.5 ;
       }
     } 
   }
  if (fabs(tilt1+tilt3d_pi)<tilt3d_eps) tilt1=-tilt3d_pi;
  if (tilt1>tilt3d_pi) tilt1-=tilt3d_pi*2.0;
  if (tilt1<=-tilt3d_pi) tilt1+=tilt3d_pi*2.0;
  c1 = cos(tilt1); s1 = sin(tilt1);

  // tilt2
  do {
    if ( fabs(s1) > tilt3d_eps ) {
      tilt2 = atan2(-R[0][2]/s1, R[2][2]);
    } else {
      tilt2 = atan2(R[1][2]/c1, R[2][2]);
    }
    if (fabs(tilt2+tilt3d_pi)<tilt3d_eps) tilt2=-tilt3d_pi;
    if ( tilt2>tilt3d_pi ) tilt2-=tilt3d_pi*2.0;
    if (tilt2<=-tilt3d_pi) tilt2+=tilt3d_pi*2.0;
    if ( tilt2<0.0 ) {
      if (tilt2<=-tilt3d_eps) {
        tilt1 += tilt3d_pi;
        if (tilt1>tilt3d_pi) tilt1-=tilt3d_pi*2.0;
      } else tilt2=0.0;
    }
  } while ( tilt2<0.0 );
  s2 = sin(tilt2);

  // tilt3
  if ( fabs(s2) > tilt3d_eps ) {
    tilt3 = atan2(R[2][0]/s2,-R[2][1]/s2) - tilt1;
  } else {
    if ( R[2][2] > 0.0 ) {
      tilt3 = atan2(R[0][1],R[1][1]);
    } else {
      tilt3 = atan2(R[1][0],R[0][0]) - 2.0*tilt1;               
    }
  }
  if (fabs(tilt3+tilt3d_pi)<tilt3d_eps) tilt3=-tilt3d_pi;
  if (tilt3>tilt3d_pi) tilt3-=tilt3d_pi*2.0;
  if (tilt3<=-tilt3d_pi) tilt3+=tilt3d_pi*2.0;

  // TILT[0] = tilt1f = tilt1-pi/2
  TILT[0] = tilt1-tilt3d_pi*0.5;
  TILT[1] = tilt2;
  TILT[2] = tilt3;

  if ( (TILT[0]) > tilt3d_pi ) TILT[0]-=tilt3d_pi*2.0;
  else if ( (TILT[0]) <= -tilt3d_pi ) TILT[0]+=tilt3d_pi*2.0;

  status = 0;

tilt3d_angle_error:

  return( status );

} // tilt3d_angles

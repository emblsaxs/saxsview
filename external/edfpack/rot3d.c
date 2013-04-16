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

# define ROT3D_VERSION  "rot3d : V1.02 Peter Boesecke 2012-06-04"
/*---------------------------------------------------------------------------
NAME

   rot3d - Conversion between rot1, rot2, rot3 and 3d rotation matrix.


SYNOPSIS


DESCRIPTION

Consecutive rotations, first around lab axis 1, second around lab axis 2 and 
third around lab axis 3. The axes 1, 2 and 3 define a right-handed orthogonal
system in the laboratory space:

 around lab axis 1:
                    |        1.0        0.0        0.0 |
       R1(rot1) =   |        0.0  cos(rot1) -sin(rot1) |
                    |        0.0  sin(rot1)  cos(rot1) |
 
 around lab axis 2:
                    |  cos(rot2)        0.0  sin(rot2) |
       R2(rot2) =   |        0.0        1.0        0.0 |
                    | -sin(rot2)        0.0  cos(rot2) |
 
 around lab axis 3:
                    |  cos(rot3) -sin(rot3)        0.0 | 
       R3(rot3) =   |  sin(rot3)  cos(rot3)        0.0 |
                    |        0.0        0.0        1.0 |                                                           
 all rotations:

  R(rot1,rot2,rot3) = R3(rot3).R2(rot2).R1(rot1)

  r11 = R[0][0] =           cos(rot2) cos(rot3)
  r12 = R[1][0] = sin(rot1) sin(rot2) cos(rot3) - cos(rot1)           sin(rot3)
  r13 = R[2][0] = cos(rot1) sin(rot2) cos(rot3) + sin(rot1)           sin(rot3)
  r21 = R[0][1] =           cos(rot2) sin(rot3)
  r22 = R[1][1] = cos(rot1)           cos(rot3) + sin(rot1) sin(rot2) sin(rot3)
  r23 = R[2][1] = cos(rot1) sin(rot2) sin(rot3) - sin(rot1)           cos(rot3)
  r31 = R[0][2] =          -sin(rot2)
  r32 = R[1][2] = sin(rot1) cos(rot2)
  r33 = R[2][2] = cos(rot1) cos(rot2)

                      | R[0][0] R[1][0] R[2][0] |
  R(rot1,rot2,rot3) = | R[0][1] R[1][1] R[2][1] |
                      | R[0][2] R[1][2] R[2][2] |

                      | r11     r12     r13 |
                    = | r21     r22     r23 |
                      | r31     r32     r33 |

                      | c2 c3   s1 s2 c3 - c1 s3     c1 s2 c3 + s1 s3 |
                    = | c2 s3   c1 c3    + s1 s2 s3  c1 s2 s3 - s1 c3 |
                      | -s2     s1 c2                c1 c2            |

  (c1=cos(rot1), s1=sin(rot1), c2=cos(rot2) etc.)

  principal ranges of the angles rot1, rot2, rot3:

    rot1  ] -Pi   .. +Pi   ]
    rot2  [ -Pi/2 .. +Pi/2 ]
    rot3  ] -Pi   .. +Pi   ]

History

  2010-04-26 Peter Boesecke creation V0.0
  2010-05-17 PB V1.0
  2011-04-18 PB V1.0 rot3d_version() added
  2011-05-17 PB V1.01 Suggested parentheses added
  2012-06-04 PB V1.02 rot3d_eps: 1e-8 => 1e-10

---------------------------------------------------------------------------*/

/***************************************************************************
* Include                                                                  *
***************************************************************************/
# include "rot3d.h"

/***************************************************************************
* Internal                                                                 *
***************************************************************************/

/****************************************************************************
* Static Variables and Numbers                                              *
****************************************************************************/
static double rot3d_eps=1e-10;
static double rot3d_pi=3.1415926535897932384626;

/****************************************************************************
* Routines                                                                  *
****************************************************************************/

/*--------------------------------------------------------------------------
NAME

  rot3d_version --- returns pointer to the version string

SYNOPSIS

  const char *rot3d_version ( void );

DESCRPTION

  Returns pointer to the version string.

--------------------------------------------------------------------------*/
const char *rot3d_version ( void )
{
  return ( ROT3D_VERSION );
} /* rot3d_version */

/*---------------------------------------------------------------------------
NAME

  rot3d_determinante

SYNOPSIS

  double rot3d_determinante ( double A[3][3] )

DESCRIPTION

  input: double A[3][3] (3d matrix)

RETURN VALUE

  determinante of A 

---------------------------------------------------------------------------*/
double rot3d_determinante ( double A[3][3] )
{ double determinante=0.0;

  if (!A) {
    fprintf( stderr, "ERROR: rot3d_determinante: NULL pointer\n" );
    goto rot3d_determinante_error;
  }

  determinante  =  A[0][0]*(A[1][1]*A[2][2]-A[2][1]*A[1][2]);
  determinante -=  A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0]);
  determinante +=  A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);

rot3d_determinante_error:

  return( determinante );

} // rot3d_determinante

/*---------------------------------------------------------------------------
NAME

  rot3d_mat_scale

SYNOPSIS

  int rot3d_mat_scale ( double A[3][3], double scale )

DESCRIPTION

  Multiplies each element of A with scale.

  input: double A[3][3] (3d matrix)
         double scale (scale factor)
  updated: A

RETURN VALUE

  status

---------------------------------------------------------------------------*/
int rot3d_mat_scale ( double A[3][3], double scale )
{ int status=-1;
  int i,j;

  if (!A) {
    fprintf( stderr, "ERROR: rot3d_mat_scale: NULL pointer\n" );
    goto rot3d_mat_scale_error;
  }

  if (scale!=1.0)
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
        A[i][j] *= scale;

  status = 0;

rot3d_mat_scale_error:

  return( status );

} // rot3d_mat_scale

/*+++------------------------------------------------------------------------
NAME
  rot3d_mat_transpose --- transpose a 3-dimensional matrix

SYNOPSIS

  int rot3d_mat_transpose ( double A[3][3] )

DESCRIPTION

  A[3][3] = Transpose(A[3][3])

RETURN VALUE
  status

----------------------------------------------------------------------------*/
int rot3d_mat_transpose ( double A[3][3] )
{ int j,k;
  double tmp;
  for (j=0;j<3;j++)
    for (k=0;k<j;k++) {
      tmp=A[j][k];
      A[j][k] = A[k][j];
      A[k][j] = tmp;
    }
  return(0);
} // rot3d_mat_transpose

/*---------------------------------------------------------------------------
NAME

  rot3d_isnotrot

SYNOPSIS

  int rot3d_isnotrot ( double R[3][3] )

DESCRIPTION

  Returns 1 if R is a rotation

RETURN VALUE

  is rotation 0
  no rotation 1
  error      -1

---------------------------------------------------------------------------*/
int rot3d_isnotrot ( double R[3][3] )
{ int is_not_rot=-1;
  int i,j; 
  double len;

  if (!R) {
    fprintf( stderr, "ERROR:  rot3d_isnotrot: NULL pointer\n" );
    goto  rot3d_isnotrot_error;
  }

  if ( fabs(rot3d_determinante(R)-1.0)>rot3d_eps )
    is_not_rot=1;
  else {
    is_not_rot=0;
    for (i=0;i<3;i++) {
      len=0.0;
      for (j=0;j<3;j++)
        len+=R[i][j]*R[i][j];
      if (fabs(len-1.0)>rot3d_eps) { 
        is_not_rot=1; 
        break; 
      }
    }
  }

rot3d_isnotrot_error:

  return( is_not_rot );
  
} // rot3d_isnotrot

/*---------------------------------------------------------------------------
NAME

  rot3d_matrix 

SYNOPSIS

  int rot3d_matrix(double ROT[3], double R[3][3]);

DESCRIPTION

  input:   double ROT[3]  (angles rot1, rot2, rot3)
  updated: double R[3][3] (3d rotation matrix)

RETURN VALUE

  status

---------------------------------------------------------------------------*/
int rot3d_matrix(double ROT[3], double R[3][3])
{ int status=-1;
  double c1, c2, c3;
  double s1, s2, s3;

  if ((!ROT)||(!R)) {
    fprintf( stderr, "ERROR: rot3d_matrix: NULL pointer\n" );
    goto rot3d_matrix_error;
  }

  c1 = cos(ROT[0]); c2 = cos(ROT[1]); c3 = cos(ROT[2]);
  s1 = sin(ROT[0]); s2 = sin(ROT[1]); s3 = sin(ROT[2]);

  R[0][0] =      c2 * c3;
  R[1][0] = s1 * s2 * c3 - c1      * s3;
  R[2][0] = c1 * s2 * c3 + s1      * s3;
  R[0][1] =      c2 * s3;
  R[1][1] = c1      * c3 + s1 * s2 * s3;
  R[2][1] = c1 * s2 * s3 - s1      * c3;
  R[0][2] =     -s2;
  R[1][2] = s1 * c2;
  R[2][2] = c1 * c2;

  status = 0;

rot3d_matrix_error:

  return( status );

} //  rot3d_matrix

/*---------------------------------------------------------------------------
NAME
  
  rot3d_inverse_matrix 

SYNOPSIS 

  int rot3d_inverse_matrix(double ROT[3], double R[3][3]);

DESCRIPTION

  input:   double ROT[3]  (angles rot1, rot2, rot3)
  updated: double R[3][3] (inverse 3d rotation matrix)

RETURN VALUE

  status

---------------------------------------------------------------------------*/
int rot3d_inverse_matrix(double ROT[3], double R[3][3])
{ int status=-1;

  if ( (status=rot3d_matrix(ROT, R)) ) goto rot3d_inverse_matrix_error;
  if ( (status=rot3d_mat_transpose (R)) ) goto rot3d_inverse_matrix_error;

  return( status );

rot3d_inverse_matrix_error:

  return( status );

} //  rot3d_inverse_matrix

/*---------------------------------------------------------------------------
NAME

   rot3d_angles

SYNOPSIS

  int rot3d_angles(double ROT[3], double R[3][3]);

DESCRIPTION

  Calculate rot1, rot2 and rot3 for an arbitrary 3d rotation matrix.

  updated:   double ROT[3]  (angles rot1, rot2, rot3)
  input:     double R[3][3] (3d rotation matrix)

  principal ranges of the angles rot1, rot2, rot3:

    rot1  ] -Pi   .. +Pi   ]
    rot2  [ -Pi/2 .. +Pi/2 ] => c2 >= 0.0
    rot3  ] -Pi   .. +Pi   ]

RETURN VALUE

  status

---------------------------------------------------------------------------*/
int rot3d_angles(double ROT[3], double R[3][3])
{ int status=-1;
  double rot1=0.0, rot2=0.0, rot3=0.0; // set defaults
  double c1, c2;
  double s1;
  double determinante=0.0, scale=1.0;

  if ((!ROT)||(!R)) {
    fprintf( stderr, "ERROR: rot3d_angles: NULL pointer\n" );
    goto rot3d_angle_error;
  }

  determinante =  rot3d_determinante( R );

  if ( determinante < rot3d_eps ) {
    fprintf( stderr, "ERROR: rot3d_angles: Det(R) = %lg is not positive\n", 
             determinante );
    goto rot3d_angle_error;
  }

  scale = pow( determinante, 1.0/3.0 );

  ROT[0] = rot1;
  ROT[1] = rot2;
  ROT[2] = rot3;

  if ( (status=rot3d_mat_scale ( R, scale )) ) {
    goto rot3d_angle_error;
  }

  if ( rot3d_isnotrot ( R ) ) {
    fprintf( stderr, "ERROR: rot3d_angles: R is not a rotation matrix\n");
    goto rot3d_angle_error;
  }

  // rot1
  if ( (1.0-fabs(R[0][2])) > rot3d_eps ) { // c2 > 0
    rot1 = atan2( R[1][2], R[2][2]);
  } else { // c2 == 0
    if ( R[0][2] < 0) { // sin(rot2)==+1.0
      rot1 = atan2( R[1][0],R[2][0])+rot3;
    } else { // sin(rot2)==-1.0
      rot1 = atan2( -R[1][0],-R[2][0])-rot3;
    }
  }
  c1 = cos(rot1); s1 = sin(rot1);
  if (fabs(rot1+rot3d_pi)<rot3d_eps) rot1=-rot3d_pi;
  if (rot1>rot3d_pi) rot1-=rot3d_pi*2.0;
  if (rot1<=-rot3d_pi) rot1+=rot3d_pi*2.0;

  // rot2
  if ( fabs(s1) > rot3d_eps ) { // s1 != 0.0 
    rot2 = atan2(-R[0][2],R[1][2]/s1);
  } else { // c2 != 0.0
    rot2 = atan2(-R[0][2],R[2][2]/c1);
  }
  c2 = cos(rot2);
  if (fabs(rot2+rot3d_pi)<rot3d_eps) rot2=-rot3d_pi;
  if (rot2>rot3d_pi) rot2-=rot3d_pi*2.0;
  if (rot2<=-rot3d_pi) rot2+=rot3d_pi*2.0;

  // rot3
  if ( fabs(c2) > rot3d_eps ) {
    rot3 = atan2(R[0][1]/c2,R[0][0]/c2);
  } else { // default of rot3
    rot3 = atan2(-(R[1][1]*s1+R[2][1]*c1)/R[0][2],
                 -(R[1][0]*s1+R[2][0]*c1)/R[0][2]);
  }
  if (fabs(rot3+rot3d_pi)<rot3d_eps) rot3=-rot3d_pi;
  if (rot3>rot3d_pi) rot3-=rot3d_pi*2.0;
  if (rot3<=-rot3d_pi) rot3+=rot3d_pi*2.0;

  ROT[0] = rot1;
  ROT[1] = rot2;
  ROT[2] = rot3;

  status = 0;

rot3d_angle_error:

  return( status );

} // rot3d_angles

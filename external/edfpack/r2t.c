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
# define R2T_VERSION  "r2t : V1.0 Peter Boesecke 2010-05-25"

/*---------------------------------------------------------------------------
NAME

   r2t - conversion between beam distance/center and sample distance/center

SYNOPSIS


DESCRIPTION

  This modul provides routines to translate between saxs parameters and
  fit2d parameters: 

    pixel size, center (PoNI), sample distance, detector rotation angles

    pixel size, beam center, beam distance, detector tilt angles
  
  The rotations matrices and angles are calculated with the modules
  rot3d and tilt3d.

  Both parameter sets have been chosen for different purposes. The saxs 
  rotation angles have been chosen to describe scattering patterns that 
  are obvserved with an arbitrarily rotated flat 2 dimensional ideal 
  detector. The fit2d parameters have been chosen to describe scattering 
  patterns that are observed with a flat 2 dimensional ideal detector 
  that is slightly misoriented with respect to the primary beam. 

  The saxs description assumes that the rotation angles of the detector are 
  exact, e.g. have been calibrated. The scattering pattern in the laboratory 
  space is interpreted according to them.

  After adding a third tilt angle to the fit2d parameters both descriptions 
  allow to calculate the exact positions of all detector pixels in 
  the laboratory space. The transformations between both parameter
  sets are bijective for tilts less pi/2. In the fit2d description 
  the beam center diverges to infinity for tilt2=pi/2 (detector plane 
  parallel to 90 deg).

  Both programs use the same laboratory and detector coordinates:
  The coordinate system is right-handed with axis 1 horizontal, axis 2 
  vertical and axis 3 pointing against the primary beam. Rotations and
  tilts are ccw. The pixel sizes (pix and bpix) are identical.

  The coordinate system with orientation 1 is defined in the following way:
  The unit vectors along the three axes are: e1^, e2^, e3^. 
  The detector in rest position (tilts and rotations are 0) is in the 
  plane e1^, e2^.

  origin (0,0) at lower left corner of the detector
  axis 1: horizontally to the right
  axis 2: vertically up
  axis 3: against primary beam

             ^ 2 (vertical)
             |
             |
             |
             |
             +------> 1 (horizontal)
             \
              \
               \
                _| 3 (against primary beam)

  The fit2d tilts calibrate the detector pattern for the rotation angles
  zero, while the saxs rotation angles orient the detector in space, e.g. 
  with a goniometer. The fit2d description is therefore not very well 
  suited for large tilts, e.g. when the detector is turned pi/2 around 
  the goniometer center, where the primary beam does not intersect with 
  the detector plane.

  Due to that different parameters have been chosen:

   -fit2d:

    The intersection point of the beam with the detector is the beam
    center. The tilts are done around this point. The tilt correction
    calculates an image like it would be seen by a detector that is
    exactly perpendicular to the primary beam. Then, the detector 
    normal at the beam center points to the sample, or more generally 
    spoken, to the goniometer center. It corresponds to the point of 
    normal incidence of the ideal perpendicularly oriented detector.
    The fit2d parameters are: the intersection point of the primary 
    beam with the detector (here called "beam center"), the distance 
    between the goniometer center and this point (here called "beam 
    distance"), the tilt of the detector plane against the sample 
    ("tilt2") and the azimuthal angle ("tilt1") of the tilt2-axis on the 
    detector plane. Both axes (tilt1 and tilt2) can be determined with 
    image analysis by interpreting the shape of powder rings on the 
    detector. The third angle ("tilt3") that rotates the detector in the 
    laboratory around the primary beam is missing in the fit2d description. 
    It is added here to define bijective transformations between saxs and 
    fit2d parameters.

    fit2d: bpix, beam center, beam distance, tilt1, tilt2, tilt3

fit2d (rotations around detector axes, origin in detector plane):

    rotation angle of tilting plane on detector (ccw around axis3):

                    |  cos(tilt1) -sin(tilt1)         0.0 | 
      T1(tilt1) =   |  sin(tilt1)  cos(tilt1)         0.0 |
                    |        0.0         0.0          1.0 |

    inclination detector plane (ccw around axis1' after Tilt1):

                    |         1.0        0.0          0.0 |
      T2(tilt2) =   |         0.0  cos(tilt2) -sin(tilt2) |
                    |         0.0  sin(tilt2)  cos(tilt2) |

    rotation angle of detector (ccw around axis 3):

                    |  cos(tilt3) -sin(tilt3)         0.0 | 
      T3(tilt3) =   |  sin(tilt3)  cos(tilt3)         0.0 |
                    |        0.0         0.0          1.0 |                  

    principal (output) ranges:

      tilt1  ] - Pi .. +Pi]
      tilt2  [    0 .. +Pi]
      tilt3  ] - Pi .. +Pi]

 all tilts: 

    Tilt(tilt1,tilt2,tilt3) = T3(tilt3).T1(tilt1).T2(tilt2).T1(-tilt1)


   -saxs:

    The point on the detector plane where the normal intersects the 
    goniometer center (sample) is called point of normal incidence ("PoNI", 
    historically called "center"). The distance between this point and the
    sample is the "sample distance". The detector rotations are done 
    sequentially in the laboratory space around the goniometer center, first 
    rotation around laboratory axis 1 ("detector rotation 1"), second 
    rotation around laboratory axis 2 ("detector rotation 2") and third 
    rotation around laboratory axis 3 ("detector rotation 3").

    saxs: pix, PoNI ("center"), sample distance, rot1, rot2, rot3

saxs (rotations around lab axes, origin in sample (goniometer center)):

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

   ROT(rot1,rot2,rot3) = R3(rot3).R2(rot2).R1(rot1)

  r11 = R[0][0] =           cos(rot2) cos(rot3)
  r12 = R[1][0] = sin(rot1) sin(rot2) cos(rot3) - cos(rot1)           sin(rot3)
  r13 = R[2][0] = cos(rot1) sin(rot2) cos(rot3) + sin(rot1)           sin(rot3)
  r21 = R[0][1] =           cos(rot2) sin(rot3)
  r22 = R[1][1] = cos(rot1)           cos(rot3) + sin(rot1) sin(rot2) sin(rot3)
  r23 = R[2][1] = cos(rot1) sin(rot2) sin(rot3) - sin(rot1)           cos(rot3)
  r31 = R[0][2] =          -sin(rot2)
  r32 = R[1][2] = sin(rot1) cos(rot2)
  r33 = R[2][2] = cos(rot1) cos(rot2)

    principal (output) ranges:

      rot1  ] -Pi   .. +Pi   ]
      rot2  [ -Pi/2 .. +Pi/2 ]
      rot3  ] -Pi   .. +Pi   ]

  Bcen1[pix1_, cen1_, dis_, R_] :=
    (cen1 pix1 ( R[1][0] R[0][1] - R[0][0] R[1][1]) +
           dis (-R[2][0] R[1][1] + R[1][0] R[2][1]))/ 
    (pix1 (R[1][0] R[0][1] - R[0][0] R[1][1]))

  Bcen2[pix2_, cen2_, dis_, R_] :=
    (cen2 pix2 (R[1][0] R[0][1] - R[0][0] R[1][1]) +
           dis (R[2][0] R[0][1] - R[0][0] R[2][1]))/
    (pix2 (R[1][0] R[0][1] - R[0][0] R[1][1]))

  Bdis[dis_, R_] :=
    dis ((R[2][1] ( R[1][0] R[0][2] - R[0][0] R[1][2]) + 
          R[2][0] (-R[1][1] R[0][2] + R[0][1] R[1][2]))/
         (-R[1][0] R[0][1] + R[0][0] R[1][1]) + 
         R[2][2])

  Cen1[pix1_, bcen1_, bdis_, R_] :=
    bcen1 + (bdis (R[2][0] R[1][1] - R[1][0] R[2][1]))/
            (pix1 (R[2][0] ( R[1][1] R[0][2] - R[0][1] R[1][2]) +
                   R[1][0] (-R[2][1] R[0][2] + R[0][1] R[2][2]) + 
                   R[0][0] ( R[2][1] R[1][2] - R[1][1] R[2][2])))

  Cen2[pix2_, bcen2_, bdis_, R_] :=
    bcen2 + (bdis (-R[2][0] R[0][1] + R[0][0] R[2][1]))/
            (pix2 ( R[2][0] ( R[1][1] R[0][2] - R[0][1] R[1][2]) +
                    R[1][0] (-R[2][1] R[0][2] + R[0][1] R[2][2]) + 
                    R[0][0] ( R[2][1] R[1][2] - R[1][1] R[2][2])))

  Dis[bdis_, R_] := 
    (bdis (-R[1][0] R[0][1] + R[0][0] R[1][1]))/
    (R[2][0] (-R[1][1] R[0][2] + R[0][1] R[1][2]) +
     R[1][0] ( R[2][1] R[0][2] - R[0][1] R[2][2]) + 
     R[0][0] (-R[2][1] R[1][2] + R[1][1] R[2][2]))

  Bcen1[pix1_, cen1_, dis_, R_] :=
    (cen1 pix1 ( R12 R21 - R11 R22) +
           dis (-R13 R22 + R12 R23))/
    (pix1 (R12 R21 - R11 R22))

  Bcen2[pix2_, cen2_, dis_, R_] :=
    (cen2 pix2 (R12 R21 - R11 R22) +
           dis (R13 R21 - R11 R23))/
    (pix2 (R12 R21 - R11 R22))

  Bdis[dis_, R_] :=
    dis ((R23 ( R12 R31 - R11 R32) + 
          R13 (-R22 R31 + R21 R32))/
         (-R12 R21 + R11 R22) + 
         R33)

  Cen1[pix1_, bcen1_, bdis_, R_] :=
    bcen1 + (bdis (R13 R22 - R12 R23))/
            (pix1 (R13 ( R22 R31 - R21 R32) +
                   R12 (-R23 R31 + R21 R33) + 
                   R11 ( R23 R32 - R22 R33)))

  Cen2[pix2_, bcen2_, bdis_, R_] :=
    bcen2 + (bdis (-R13 R21 + R11 R23))/
            (pix2 (R13 ( R22 R31 - R21 R32) +
                   R12 (-R23 R31 + R21 R33) + 
                   R11 ( R23 R32 - R22 R33)))

  Dis[bdis_, R_] := 
    (bdis (-R12 R21 + R11 R22))/
    (R13 (-R22 R31 + R21 R32) +
     R12 ( R23 R31 - R21 R33) + 
     R11 (-R23 R32 + R22 R33))

History

  2010-05-17 Peter Boesecke creation
  2010-05-25 V1.0 Peter Boesecke
  2011-04-18 V1.0 PB r2t_version() added

---------------------------------------------------------------------------*/

/***************************************************************************
* Include                                                                  *
***************************************************************************/
# include "r2t.h"

/****************************************************************************
* Static Variables and Numbers                                              *
****************************************************************************/

static double r2t_eps=1e-8;

/****************************************************************************
* Routines                                                                  *
****************************************************************************/

/*--------------------------------------------------------------------------
NAME

  r2t_version --- returns pointer to the version string

SYNOPSIS

  const char *r2t_version ( void );

DESCRPTION

  Returns pointer to the version string.

--------------------------------------------------------------------------*/
const char *r2t_version ( void )
{
  return ( R2T_VERSION );
} /* r2t_version */

/*+++------------------------------------------------------------------------
NAME

  r2t_bcen1 --- calculate beam center 1

SYNOPSIS

  int r2t_bcen1( double *bcen1,
                 double pix1, double cen1, double dis, double R[3][3] );

DESCRIPTION

  bcen1 =
    (cen1 pix1 ( R[1][0] R[0][1] - R[0][0] R[1][1]) +
           dis (-R[2][0] R[1][1] + R[1][0] R[2][1]))/ 
    (pix1 (R[1][0] R[0][1] - R[0][0] R[1][1]))

RETURN VALUE
  status

----------------------------------------------------------------------------*/
int r2t_bcen1( double *bcen1,
               double pix1, double cen1, double dis, double R[3][3] )
{ int status=-1;
  double denom;

  if (!R||!bcen1) {
    fprintf( stderr, "ERROR: r2t_bcen1: NULL pointer\n" );
    goto r2t_bcen1_error;
  }

  denom = pix1*(R[1][0]*R[0][1] - R[0][0]*R[1][1]);

  if ( fabs(denom)<r2t_eps ) goto r2t_bcen1_error;

  *bcen1 = ( cen1*pix1*( R[1][0]*R[0][1] - R[0][0]*R[1][1]) +
                   dis*(-R[2][0]*R[1][1] + R[1][0]*R[2][1]) ) / denom;

  status = 0;

r2t_bcen1_error:


  return( status );

} // r2t_bcen1

/*+++------------------------------------------------------------------------
NAME

  r2t_bcen2 --- calculate beam center 2

SYNOPSIS

  int r2t_bcen2( double *bcen2,
                 double pix2, double cen2, double dis, double R[3][3] );

DESCRIPTION

  bcen2 =
    (cen2 pix2 (R[1][0] R[0][1] - R[0][0] R[1][1]) +
           dis (R[2][0] R[0][1] - R[0][0] R[2][1]))/
    (pix2 (R[1][0] R[0][1] - R[0][0] R[1][1]))

RETURN VALUE
  status

----------------------------------------------------------------------------*/
int r2t_bcen2( double *bcen2,
               double pix2, double cen2, double dis, double R[3][3] )
{ int status=-1;
  double denom;

  if (!R||!bcen2) {
    fprintf( stderr, "ERROR: r2t_bcen2: NULL pointer\n" );
    goto r2t_bcen2_error;
  }

  denom = pix2*(R[1][0]*R[0][1] - R[0][0]*R[1][1]);

  if ( fabs(denom)<r2t_eps ) goto r2t_bcen2_error;

  *bcen2 = ( cen2*pix2*(R[1][0]*R[0][1] - R[0][0]*R[1][1]) +
                   dis*(R[2][0]*R[0][1] - R[0][0]*R[2][1]) ) / denom;

  status = 0;

r2t_bcen2_error:

  return( status );

} // r2t_bcen2

/*+++------------------------------------------------------------------------
NAME

  r2t_bdis --- calculate beam distance

SYNOPSIS

  int r2t_bdis ( double *bdis,
                 double dis, double R[3][3] );

DESCRIPTION

  bdis =
    dis ((R[2][1] ( R[1][0] R[0][2] - R[0][0] R[1][2]) + 
          R[2][0] (-R[1][1] R[0][2] + R[0][1] R[1][2]))/
         (-R[1][0] R[0][1] + R[0][0] R[1][1]) + 
         R[2][2])

RETURN VALUE
  status

----------------------------------------------------------------------------*/
int r2t_bdis ( double *bdis,
               double dis, double R[3][3] )
{ int status=-1;
  double denom;

  if (!R||!bdis) {
    fprintf( stderr, "ERROR: r2t_bdis: NULL pointer\n" );
    goto r2t_bdis_error;
  }

  denom = -R[1][0]*R[0][1] + R[0][0]*R[1][1];

  if ( fabs(denom)<r2t_eps ) goto r2t_bdis_error;

  *bdis = dis*(R[2][2] + (R[2][1]*( R[1][0]*R[0][2] - R[0][0]*R[1][2]) + 
                          R[2][0]*(-R[1][1]*R[0][2] + R[0][1]*R[1][2]))/denom);

  status = 0;

r2t_bdis_error:

  return( status );

} // r2t_bdis

/*+++------------------------------------------------------------------------
NAME

  r2t_cen1 --- calculate center 1 (PoNI 1) 

SYNOPSIS

  int r2t_cen1 ( double *cen1,
                 double pix1, double bcen1, double bdis, double R[3][3] );

DESCRIPTION

  cen1 =
    bcen1 + (bdis (R[2][0] R[1][1] - R[1][0] R[2][1]))/
            (pix1 (R[2][0] ( R[1][1] R[0][2] - R[0][1] R[1][2]) +
                   R[1][0] (-R[2][1] R[0][2] + R[0][1] R[2][2]) + 
                   R[0][0] ( R[2][1] R[1][2] - R[1][1] R[2][2])))

RETURN VALUE
  status

----------------------------------------------------------------------------*/
int r2t_cen1 ( double *cen1,
               double pix1, double bcen1, double bdis, double R[3][3] )
{ int status=-1;
  double denom;

  if (!R||!cen1) {
    fprintf( stderr, "ERROR: r2t_cen1: NULL pointer\n" );
    goto r2t_cen1_error;
  }

  denom = pix1*(R[2][0]*( R[1][1]*R[0][2] - R[0][1]*R[1][2]) +
                R[1][0]*(-R[2][1]*R[0][2] + R[0][1]*R[2][2]) + 
                R[0][0]*( R[2][1]*R[1][2] - R[1][1]*R[2][2]));

  if ( fabs(denom)<r2t_eps ) goto r2t_cen1_error;

  *cen1 = bcen1 + (bdis*(R[2][0]*R[1][1] - R[1][0]*R[2][1]))/denom;

  status = 0;

r2t_cen1_error:

  return( status );

} // r2t_cen1

/*+++------------------------------------------------------------------------
NAME

  r2t_cen2 --- calculate center 2 (PoNI 2) 

SYNOPSIS

  int r2t_cen2 ( double *cen2,
                 double pix2, double bcen2, double bdis, double R[3][3] );

DESCRIPTION

  cen2 =
    bcen2 + (bdis (-R[2][0] R[0][1] + R[0][0] R[2][1]))/
            (pix2 ( R[2][0] ( R[1][1] R[0][2] - R[0][1] R[1][2]) +
                    R[1][0] (-R[2][1] R[0][2] + R[0][1] R[2][2]) + 
                    R[0][0] ( R[2][1] R[1][2] - R[1][1] R[2][2])))

RETURN VALUE
  status

----------------------------------------------------------------------------*/
int r2t_cen2 ( double *cen2,
               double pix2, double bcen2, double bdis, double R[3][3] )
{ int status=-1;
  double denom;

  if (!R||!cen2) {
    fprintf( stderr, "ERROR: r2t_cen2: NULL pointer\n" );
    goto r2t_cen2_error;
  }

  denom = pix2*(R[2][0]*( R[1][1]*R[0][2] - R[0][1]*R[1][2]) +
                R[1][0]*(-R[2][1]*R[0][2] + R[0][1]*R[2][2]) +
                R[0][0]*( R[2][1]*R[1][2] - R[1][1]*R[2][2]));

  if ( fabs(denom)<r2t_eps ) goto r2t_cen2_error;

  *cen2 = bcen2 + (bdis*(-R[2][0]*R[0][1] + R[0][0]*R[2][1]))/denom;

  status = 0;

r2t_cen2_error:

  return( status );

} // r2t_cen2

/*+++------------------------------------------------------------------------
NAME

  r2t_dis --- calculate sample distance 

SYNOPSIS

  int r2t_dis  ( double *dis,
                 double bdis, double R[3][3] );

DESCRIPTION

  dis =
    (bdis (-R[1][0] R[0][1] + R[0][0] R[1][1]))/
    (R[2][0] (-R[1][1] R[0][2] + R[0][1] R[1][2]) +
     R[1][0] ( R[2][1] R[0][2] - R[0][1] R[2][2]) + 
     R[0][0] (-R[2][1] R[1][2] + R[1][1] R[2][2]))

RETURN VALUE
  status

----------------------------------------------------------------------------*/
int r2t_dis  ( double *dis,
               double bdis, double R[3][3] )
{ int status=-1;
  double denom;

  if (!R||!dis) {
    fprintf( stderr, "ERROR: r2t_dis: NULL pointer\n" );
    goto r2t_dis_error;
  }

  denom = (R[2][0]*(-R[1][1]*R[0][2] + R[0][1]*R[1][2]) +
           R[1][0]*( R[2][1]*R[0][2] - R[0][1]*R[2][2]) + 
           R[0][0]*(-R[2][1]*R[1][2] + R[1][1]*R[2][2]));

  if ( fabs(denom)<r2t_eps ) goto r2t_dis_error;

  *dis = (bdis*(-R[1][0]*R[0][1] + R[0][0]*R[1][1]))/denom;

  status = 0;

r2t_dis_error:

  return( status );

} // r2t_dis

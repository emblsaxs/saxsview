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

# define WAXS_VERSION      "waxs : V1.10 Peter Boesecke 2010-05-31"
/*+++------------------------------------------------------------------------
NAME
   waxs --- routines for waxs detector projections 

SYNOPSIS

   # include waxs.h

HISTORY
  2001-04-19 V1.0  Peter Boesecke
  2002-05-09 V1.1  PB kdir2sp, waxs_Waxs
  2004-07-27 V1.2  PB sp2kdir and kdir2sp renamed to waxs_sp2kdir and waxs_kdir2sp,
                      _fprint_dir renamed to waxs_PrintDir,
                      WaxsDir, waxs_sp2kdir, waxs_kdir2sp, waxs_PrintDir,
                      waxs_s2kdir and waxs_kdir2s defined in waxs.h 
  2004-12-07 V1.3  PB cylindrical symmetry:
                      SymType, SymRot and InvSymRot added to WaxsParams,
                      new waxs_SymInit, waxs_ssym2kdir, waxs_kdir2ssym
                      waxs_saxs and waxs_waxs call waxs_ssym2kdir and 
                      waxs_kdir2ssym if SymType is set.
                      waxs_Init initializes SymType, SymRot and InvSymRot with 0.
                      The symmetry axis is always projected to axis 1.
  2004-12-08 V1.4  PB SymType 1 and 2
  2004-12-11 V1.5  PB waxs_Uni2Iso, waxs_Iso2Uni
                      -> test status <-
  2007-04-19 V1.6  PB -Wall compiler warnings resolved
  2009-01-09 V1.7a PB waxs_PrintVector, waxs_kdir2svec, waxs_Saxs2Vector,
                     _svec_set added
  2010-03-18 V1.8  PB waxs_Range, waxs_Transform, waxs_not_init added
                      waxs_Range and waxs_Transform arguments changed
  2010-05-31 V1.9  PB waxs_kdir2svec: calculation of s corrected
  2010-05-31 V1.10 PB work: no global WaxsParams
                      
DESCRIPTION

These routines calculate the projection of the Ewald sphere for a scattering 
pattern that was taken with an inclined 2d-detector and project it to a plane 
surface. The radial distance from the center of this surface is the length of 
the scattering vector s. The azimuthal angle is proportional to the azimuthal 
angle of the scattered beam. 

In the following, 3d vectors are followed by '~', the length of a vector 
is just its name: e.g. kin = ||kin~||. Vectors with unit length are
followed by '^', e.g. kin^ = kin~/kin. 

      kin~              : wavevector of incident beam
(i)   kout~             : wavevector of scattered beam
      s~ = kout~ - kin~ : scattering vector

The scattering is elastic. The wavenumber k of kin~ and kout~ is:

(ii) k = 1/wavelength = kin = kout

Scattering Geometry and Orientation of Detector
The angles rot1, rot2 and rot3 define the orientation of the detector
in the lab coordinate system with orientation 1. The unit vectors along the 
three axes are: e1^, e2^, e3^. The rotations are applied sequentially. The 
first rotation is around axis 1, the second around axis 2 and the third around 
axis3. All rotations are counter clockwise. The coordinate system with 
orientation 1 is defined in the following way:

  origin (0,0) at lower left corner of the image
  axis 1: horizontally to the right
  axis 2: vertically up
  axis 3: against beam direction

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

ID01:  -x <=> axis 1,  y <=> axis 2, -z <=> axis 3
ID02:   x <=> axis 1, -y <=> axis 3,  z <=> axis 2

The primary beam (kin~) is antiparallel to axis3.

(iii) kin~ = -kin * e3^

The direction of the scattered beam relative to the coordinate system
of the inclined detector image is:

The direction of the scattered beam expressed in saxs-coordinates (see *)
of the inclined detector is:

            (  sc_1 )
(iv) sc~ =  |  sc_2 | 
            (   -k  ) 

Here, sc_1 and sc_2 are saxs-coordinates of the detector image with respect to
the point of normal incidence ("center"). k is the wavenumber.
The direction kout^ is calculated by

(v) kout^ = A * sc^ 

where A is the rotation matrix that describes the inclination of the detector.

With eq. i to eq. v the scattering vector becomes

(vi) s~ = k * kout^  + k * e3^

kout^ can also be expressed by the scattering angle 2Theta and the
azimuthal angle alpha:

                         ( sin(2Theta)*cos(alpha) )
(vii) kout^ = A * sc^ =  | sin(2Theta)*sin(alpha) |
                         (     -cos(2Theta)       )

The output image is the projection of s on a plane surface. The coordinates
are sp_1 and sp_2: 

(viii)  sp_1 = s * cos(alpha)
        sp_2 = s * sin(alpha)

From eq. viii s and cos(alpha) can be calculated:

(ix)   s = sqrt(sp_1^2+sp_2^2)
       cos(alpha) = sp_1/s, sin(alpha) = sp_2/s

Eq. vi allows to calculate sin(2Theta) and cos(2Theta) from s and k:

(x)                   s^2 = k^2 * (kout^ + e3^)^2
                          = k^2 * (1 + 2*kout^*e3^ + 1)
                          = k^2 * 2 * (1 + kout^*e3^)
                          = k^2 * 2 * (1 - cos(2Theta))
                          = k^2 * 2 * 2*sin(Theta)^2

(xi)       2*sin(Theta)^2 = s^2/(2*k^2)

(xii)      cos(2Theta) = 1-s^2/(2*k^2) 
           sin(2Theta) = sqrt( 2*s^2/(2*k^2) - (s^2/(2*k^2))^2 )  

Eq. ix and xii allow the calculation of 2Theta and alpha from the saxs-
coordinates (sp_1, sp_2) of the projection. kout can then be calculated with 
eq. vii. The saxs-coordinates (sc_1,sc_2) on the inclined detector are given by:

                                       ( sin(2Theta)*cos(alpha) )   ( sc^_1 )
(xiii)     sc^ = InvA * kout^ = InvA * | sin(2Theta)*sin(alpha) | = | sc^_2 |
                                       (     -cos(2Theta)       )   ( sc^_3 )

(xiv)            ( k * sc^_1/sc^_3 )   ( sc_1 )
           sc~ = | k * sc^_2/sc^_3 | = | sc_2 |
                 (      -k         )   ( sc_3 )

*) SAXS-Coordinates

     sc_1 =  k * ((x_1+off_1) - cen_1) * (pix_1/dis)    
     sc_2 =  k * ((x_2+off_2) - cen_2) * (pix_2/dis)

where x_1, x_2 are the pixel coordinates, off_1,off_2, the offsets, 
cen_1,cen_2 the point of normal incidence ("poni", "center"), pix_1,pix_2, 
the pixel sizes, dis the distance between the sample and the point of 
normal incidence and k the wavenumber (1/wavelength).
For small scattering angles (sqrt(sc_1^2+sc_2^2) << k) (sc_1,sc_2,-k) 
approximates the scattering vector.

The detector orientation is defined by three sequential ccw-rotations around
axis 1 (rot1), axis 2 (rot2) and axis 3 (rot3).
 
rotation around
 
 axis 1:
                 |         1.0         0.0         0.0 |
       ROT_1 =   |         0.0  cos(angle) -sin(angle) |
                 |         0.0  sin(angle)  cos(angle) |
 
 axis 2:
                 |  cos(angle)         0.0  sin(angle) |
       ROT_2 =   |        0.0          1.0         0.0 |
                 | -sin(angle)         0.0  cos(angle) |
 
 axis 3:
                 |  cos(angle) -sin(angle)         0.0 |
       ROT_3 =   |  sin(angle)  cos(angle)         0.0 |
                 |         0.0         0.0         1.0 |                                                                                      
Extension to Cylindrical Symmetry

The above described projection can also be interpreted as a transformation 
of the scattering pattern to the s_1-s_2-plane in reciprocal space. In this
interpretation the described pattern is only correct if the sample does not 
have any preferred orientation, i.e. if the scattering is isotropic, like
for a powder. If the sample has cylindrical symmetry around a symmetry axis
sym^ some parts of the reciprocal space close to the symmetry axis are not 
visible by the detector and cannot be projected. 
If this type of interpretation is chosen, the symmetry type symtype must be 
given and the symmetry axis must be defined with a rotation matrix SymRot in 
analogy to the detector rotation: symrot1, symrot2, symrot3. The default 
is 0 for all rotations. symtype 0 means isotropic sample (default), 
symtype 1 means cylindrical symmetry around axis sym_1^. 

The scattering pattern is projected to the ssym_1-ssym_2 plane defined by 
the symmetry rotation matrix.

a) Scattering pattern to projection (saxs->waxs)

The scattering vector s~ in lab space is calculated with eq. vi. It is 
projected to the symmetry axis sym^ using the scalar product:

(I)        ssym_1 = sym_1^ * s~ 

where sym_1^ is
                             (1)
(II)       sym_1^ = SymRot * |0|
                             (0)

In this case axis 1 of SymRot is the symmetry axis. 

The length perpendicular to sym_1^ is calculated from the condition that the 
length of the projected vector ssym_1 must be s:

(III)      ssym_2 = +-sqrt(s^2-ssym_1^2) (positive and negative solution)

The third component is zero.

b) Projection to scattering pattern (waxs->saxs)

The backprojection from a projection has to fulfill the following conditions:

  - the length of the scattering vector s in the projection is constant
  - the component sp~ of the scattering vector s that is parallel to the 
    axis of cylindrical symmetry sym_1~ symmetry is constant
  - the k-vector lengths are constant: k1=k0=k
  

(IV)          s~ = sp~ + ss~   =>    s^2 = sp^2 + ss^2

(V)          k1~ = k0~ + s~    =>    k^2 = k^2 + 2*k0~*s~ + s^2

In the system of the symmetry axis sym_1~ sp~ and ss~ can be written as

                   ( ssym_1 )              (   0    )
(VI)         sp~ = |   0    |  and   ss~ = | ssym_2 |
                   (   0    )              ( ssym_3 )

             ssym_1 = s~*sym_1^

             k0~ =  InvSymRot * kin~

By substitution of sp~ and ss~ in eq. IV using eq. VI and by removing k^2 
from both sides of eq. V and by rearranging both sides of the equations one
gets:

(IVa)           ssym_2^2 + ssym_3^2    =      s^2 - sp^2     = B

(Va)         k0_2*ssym_2 + k0_3*ssym_3 = - (s^2/2 + k0~*sp~) = A

A, B and k0~ are known numbers. The equation IVa and Va can be used to 
calculate ss~ (s_2 and s_3).

The solutions for ssym_3 are:

                      A*k0_3 +- k0_2*sqrt( (k0_2^2+k0_3^2)*B-A^2 )
(VII)        ssym_3 = -------------------------------------------
                                   k0_2^2+k0_3^2

ssym_2 can be calculated with eq. Va, s_1 is known.

Finally, the components of s~ must be expressed in the lab coordinate system:

                           ( ssym_1 )
(VIII)       s~ = SymRot * | ssym_2 |
                           ( ssym_3 )

Extension to General Case in 3 Dimensions 
...

----------------------------------------------------------------------------*/
/******************************************************************************
* Include Files                                                               *
******************************************************************************/

# include "waxs.h"

/******************************************************************************
* Private Constants                                                           *
******************************************************************************/

# define R_PI 3.1415926535897932384626

// static const double deg2rad = R_PI/180.0;
static const double rad2deg = 180.0/R_PI;
// static const double pi      = R_PI;
// static const double halfpi  = R_PI*0.5;
// static const double twopi   = R_PI*2.0;
// static const double one     = 1.0;
static const double eps     = 1e-30;

/******************************************************************************
* Private Variables                                                           *
******************************************************************************/

/******************************************************************************
* Routines                                                                    *
******************************************************************************/

void _fprint_mat ( FILE * out, double A[3][3] ) 
{ int i,j;
  for (j=0;j<3;j++) {
    for (i=0;i<3;i++) {
      fprintf(out," %15.3f", A[i][j]);
      }
    fprintf(out,"\n");
    }
} // _fprint_mat

void _fprint_vec ( FILE * out, double V[3] )
{ int i;
  for (i=0;i<3;i++)
    fprintf(out," %15g\n", V[i] );
} // _fprint_vec                                                                                                                              
void waxs_PrintDir ( FILE * out, WaxsDir Beam )
{
  fprintf(out," sinAlpha           = %g\n", Beam.sinAlpha );
  fprintf(out," cosAlpha           = %g   (%g deg)\n",
          Beam.cosAlpha, atan2(Beam.sinAlpha,Beam.cosAlpha)*rad2deg );
  fprintf(out," sinTwoTheta        = %g\n", Beam.sinTwoTheta );
  fprintf(out," cosTwoTheta        = %g   (%g deg)\n",
          Beam.cosTwoTheta, atan2(Beam.sinTwoTheta,Beam.cosTwoTheta)*rad2deg );
} // waxs_PrintDir                                                                                                                              
void waxs_PrintParams ( FILE * out, WParams Params )
{
  WParams *pParams = &Params;
  if ( !pParams->Init) return;
  fprintf(out," Init                 = %d\n", pParams->Init); 
  _fprint_mat ( out, pParams->Rot );
  _fprint_mat ( out, pParams->InvRot );
  fprintf(out," k                    = %g\n", pParams->k);   
  fprintf(out," halfdk2              = %g\n", pParams->halfdk2);
  fprintf(out," SymType              = %d\n", pParams->SymType);
  _fprint_mat ( out, pParams->SymRot );
  _fprint_mat ( out, pParams->InvSymRot );
} //  waxs_PrintParams 

void waxs_PrintCoord ( FILE * out, WaxsCoord sp )
{
  WaxsCoord *pCoord = &sp;
  fprintf(out," status               = %d\n", pCoord->status);
  fprintf(out," s_1                  = %g\n", pCoord->s_1);
  fprintf(out," s_2                  = %g\n", pCoord->s_2);
} // waxs_PrintCoord 

void waxs_PrintVector ( FILE * out, WaxsVector svec )
{
  WaxsVector *pVector = &svec;
  fprintf(out," status               = %d\n", pVector->status);
  fprintf(out," s_1                  = %g\n", pVector->s_1);
  fprintf(out," s_2                  = %g\n", pVector->s_2);
  fprintf(out," s_3                  = %g\n", pVector->s_3);
} // waxs_PrintVector

/*+++------------------------------------------------------------------------
NAME
  rotation_matrix_3 --- calculates the 3-dimensional rotation matrix

SYNOPSIS

  void rotation_matrix_3 ( double Rot[3][3], int axis, double angle )

DESCRIPTION

  Calculates the 3-dimensional rotation matrix for a ccw rotation of
  angle degrees around axis (axis = 1 | 2 | 3, double Rot[3][3]).

RETURN VALUE
  none

----------------------------------------------------------------------------*/
void rotation_matrix_3 ( double Rot[3][3], int axis, double angle )
{
  switch (axis) {
    case 1:
      Rot[0][0] =  1.0;        Rot[1][0] =  0.0;        Rot[2][0] =  0.0;
      Rot[0][1] =  0.0;        Rot[1][1] =  cos(angle); Rot[2][1] = -sin(angle);
      Rot[0][2] =  0.0;        Rot[1][2] =  sin(angle); Rot[2][2] =  cos(angle);
      break;
    case 2:
      Rot[0][0] =  cos(angle); Rot[1][0] =  0.0;        Rot[2][0] =  sin(angle);
      Rot[0][1] =  0.0;        Rot[1][1] =  1.0;        Rot[2][1] =  0.0;
      Rot[0][2] = -sin(angle); Rot[1][2] =  0.0;        Rot[2][2] =  cos(angle);
      break;
    case 3:
      Rot[0][0] =  cos(angle); Rot[1][0] = -sin(angle); Rot[2][0] =  0.0;
      Rot[0][1] =  sin(angle); Rot[1][1] =  cos(angle); Rot[2][1] =  0.0;
      Rot[0][2] =  0.0;        Rot[1][2] =  0.0;        Rot[2][2] =  1.0;
      break;
    default:
      printf("ERROR in rotation_matrix_3: axis = %d, 1<=axis<=3 required.\n",
         axis);
      exit(-1);
    }

  return;

} // rotation_matrix_3

/*+++------------------------------------------------------------------------
NAME
  mat_mul_3 --- product of two 3-dimensional matrices

SYNOPSIS

  void mat_mul_3 ( double Out[3][3], double A[3][3], double B[3][3] )

DESCRIPTION

  Out[3][3] = A[3][3]*B[3][3]

RETURN VALUE
  none

----------------------------------------------------------------------------*/
void mat_mul_3 ( double Out[3][3], double A[3][3], double B[3][3] )
{ int i,j,k;
  for (j=0;j<3;j++)
    for (k=0;k<3;k++) {
      Out[j][k] = 0.0;
      for (i=0;i<3;i++)
        Out[j][k] += A[i][k] * B[j][i];
      }
  return;
} // mat_mul_3

/*+++------------------------------------------------------------------------
NAME

  vec_mul --- multiplication of a 3x3 matrix with a 3d vector

SYNOPSIS

  void vec_mul ( double VOut[3], double A[3][3], double V[3] )

DESCRIPTION

  VOut[3] = A[3][3]*V[3]

RETURN VALUE
  none

----------------------------------------------------------------------------*/
void vec_mul ( double VOut[3], double A[3][3], double V[3] )
{ int i,j;
  for (j=0;j<3;j++) {
    VOut[j] = 0.0;
    for (i=0;i<3;i++)
      VOut[j] += A[i][j] * V[i];
    }
  return;
} // vec_mul

/*+++------------------------------------------------------------------------
NAME

  scalar_product --- scalar product of two 3-dimensional vectors 

SYNOPSIS

  double scalar_product ( double V[3], double W[3] );

DESCRIPTION
  Calculates the scalar product of V and W

RETURN VALUE
  V[0]*W[0]+V[1]*W[1]+V[2]*W[2] 

----------------------------------------------------------------------------*/
double scalar_product ( double V[3], double W[3] )
{ double value;
  int i;
  value=0.0;
  for (i=0;i<3;i++)
    value += V[i] * W[i];
  return(value);
} // scalar_product

/*+++------------------------------------------------------------------------
NAME
  _beam_set --- set WaxsDir

SYNOPSIS

  WaxsDir _beam_set( WaxsDir *pbeam, int status )

DESCRIPTION

  Changes error status in WaxsDir to status.
  Other parameters are not changed.

RETURN VALUE

  WaxsDir *pBeam with error status

----------------------------------------------------------------------------*/
WaxsDir _beam_set( WaxsDir *pbeam, int status )
{
  pbeam->status = status;
  return( *pbeam );

} // _beam_set

/*+++------------------------------------------------------------------------
NAME
  _s_set --- set error status in WaxsCoord

SYNOPSIS

  WaxsCoord _s_set( WaxsCoord *ps, int status )

DESCRIPTION

  Changes error status in WaxsCoord to status.
  Other parameters are not changed.

RETURN VALUE

  WaxsCoord *ps with error status

----------------------------------------------------------------------------*/
WaxsCoord _s_set( WaxsCoord *ps, int status )
{
  ps->status = status;
  return( *ps );

} // _s_set

/*+++------------------------------------------------------------------------
NAME
  _svec_set --- set error status in WaxsVector

SYNOPSIS

  WaxsVector _svec_set( WaxsVector *psvec, int status )

DESCRIPTION

  Changes error status in WaxsVector to status.
  Other parameters are not changed.

RETURN VALUE

  WaxsVector *psvec with error status

----------------------------------------------------------------------------*/
WaxsVector _svec_set( WaxsVector *psvec, int status )
{
  psvec->status = status;
  return( *psvec );

} // _svec_set


/*+++------------------------------------------------------------------------
NAME
  waxs_sp2kdir --- calculates the angles of kout 

SYNOPSIS

  WaxsDir waxs_sp2kdir ( WaxsCoord sp )

DESCRIPTION

  Calculates the unit vector of the scattered beam in lab coordinates from 
the saxs-coordinates (sp_1, sp_2) of the Ewald-sphere projection.

RETURN VALUE
  
  .status==0 : sinTwoTheta, cosTwoTheta, sinAlpha, cosAlpha angles in rad
               (external angles)
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsDir waxs_sp2kdir ( WParams * pParams, WaxsCoord sp )
{
  WaxsDir Beam;

  double s, s2, s2d2k2;
  double tmp;

  if (!pParams) return(_beam_set(&Beam,-2));

  // pParams initialized
  if (!pParams->Init) return(_beam_set(&Beam,-1));

  s2     = sp.s_1*sp.s_1+sp.s_2*sp.s_2;
  s      = sqrt(s2);
  s2d2k2 = s2*pParams->halfdk2;

  Beam.cosTwoTheta = 1.0 - s2d2k2;
  tmp = 2.0*s2d2k2-s2d2k2*s2d2k2;
  if (tmp<0.0) {
    if (tmp>-eps) { tmp=0.0;
    } else { return(_beam_set(&Beam,-2)); }
  }
  Beam.sinTwoTheta = sqrt(tmp);

  if (s>eps) {
   Beam.cosAlpha = sp.s_1/s;
   Beam.sinAlpha = sp.s_2/s;
   } else {
   Beam.cosAlpha = 0.0;
   Beam.sinAlpha = 0.0;
   }

  return( _beam_set( &Beam, 0 ) );

} // waxs_sp2kdir

/*+++------------------------------------------------------------------------
NAME
  waxs_s2kdir --- calculates the angles of kout

SYNOPSIS

  WaxsDir waxs_s2kdir ( WaxsCoord s )

DESCRIPTION

  Calculates the unit vector of the scattered beam in lab coordinates
  from the saxs-coordinate (s_1, s_2) of the inclined detector image

RETURN VALUE
 
  .status==0 : sinTwoTheta, cosTwoTheta, sinAlpha, cosAlpha angles in rad
               (external angles)
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsDir waxs_s2kdir ( WParams * pParams, WaxsCoord s ) 
{
  WaxsDir Beam;

  double veclen;
  double kvec[3];
  double kvecout[3];

  if (!pParams) return(_beam_set(&Beam,-2));

  // pParams initialized
  if (!pParams->Init) return( _beam_set( &Beam,-1 ) );

  veclen   = sqrt(s.s_1*s.s_1+s.s_2*s.s_2+pParams->k*pParams->k);
  kvec[0]  = s.s_1/veclen;
  kvec[1]  = s.s_2/veclen;
  kvec[2]  = -pParams->k/veclen;

  vec_mul ( kvecout, pParams->Rot, kvec );

  Beam.cosTwoTheta = -kvecout[2];
  Beam.sinTwoTheta = sqrt(kvecout[0]*kvecout[0]+kvecout[1]*kvecout[1]);

  if (fabs(Beam.sinTwoTheta)>eps) {
    Beam.cosAlpha    = kvecout[0]/Beam.sinTwoTheta;
    Beam.sinAlpha    = kvecout[1]/Beam.sinTwoTheta;
  } else {
    Beam.cosAlpha    = 0.0;
    Beam.sinAlpha    = 0.0;
  }

  return( _beam_set( &Beam, 0 ) );

} // waxs_s2kdir

/*+++------------------------------------------------------------------------
NAME
  waxs_kdir2sp --- calc's the saxs-coordinates of the Ewald-sphere projection 
 
SYNOPSIS
 
  WaxsCoord waxs_kdir2sp ( WaxsDir kdir )
 
DESCRIPTION
 
  Calculates the saxs-coordinates (sp_1, sp_2) of the Ewald-sphere projection 
  from the unit vector Beam of the scattered beam in lab coordinates

RETURN VALUE
 
  .status==0 : sp_1, sp_2 
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsCoord waxs_kdir2sp ( WParams * pParams, WaxsDir Beam )
{
  WaxsCoord sp;

  double s;
 
  if (!pParams) return(_s_set(&sp,-2));                                                                                            
  // pParams initialized
  if (!pParams->Init) return(_s_set(&sp,-1));                                                                                            
  s = sqrt(2*(1.0-Beam.cosTwoTheta))*pParams->k; 
  sp.s_1 = s*Beam.cosAlpha;
  sp.s_2 = s*Beam.sinAlpha;

  return( _s_set( &sp, 0 ) );

} // waxs_kdir2sp

/*+++------------------------------------------------------------------------
NAME
  waxs_kdir2s --- calculates the saxs-coordinates

SYNOPSIS

  WaxsCoord waxs_kdir2s ( WaxsDir Beam )
 
DESCRIPTION
  
  Calculates the saxs-coordinates (s_1, s_2) from the unit vector Beam 
  of the scattered beam in lab coordinates

RETURN VALUE
  
  .status==0 : s_1, s_2
  .status<0  : error
  .status<-1 : no solution
 
----------------------------------------------------------------------------*/
WaxsCoord waxs_kdir2s ( WParams * pParams, WaxsDir Beam )
{
  WaxsCoord sout;
  double kvec[3];
  double kvecout[3];

  if (!pParams) return(_s_set( &sout,-2));

  // pParams initialized
  if (!pParams->Init) return(_s_set( &sout,-1));

  kvec[0] =  Beam.sinTwoTheta*Beam.cosAlpha;
  kvec[1] =  Beam.sinTwoTheta*Beam.sinAlpha;
  kvec[2] = -Beam.cosTwoTheta;

  vec_mul ( kvecout, pParams->InvRot, kvec );

  // no solution for positive kvecout[2]
  if (kvecout[2]>-eps) return(_s_set( &sout,-3));
  
  sout.s_1 = -(kvecout[0]/kvecout[2])*pParams->k;
  sout.s_2 = -(kvecout[1]/kvecout[2])*pParams->k;

  return(_s_set( &sout,0));

} // waxs_kdir2s

/*+++------------------------------------------------------------------------
NAME
  waxs_ssym2kdir --- calculates the angles of kout

SYNOPSIS

  WaxsDir waxs_ssym2kdir ( WaxsCoord ssym )

DESCRIPTION

  Calculates the unit vector of the scattered beam in lab coordinates from
the saxs-coordinates (ssym_1, ssym_2) of the cylindrical symmetric
Ewald-sphere projection.

RETURN VALUE
 
  .status==0 : sinTwoTheta, cosTwoTheta, sinAlpha, cosAlpha angles in rad
               (external angles)
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsDir waxs_ssym2kdir ( WParams * pParams, WaxsCoord ssym )
{
  WaxsDir Beam;

  double kin[3], k0[3], s[3], s0[3], kout[3];

  double sp2, ss2, s2;
  double A, B;

  double k022pk032, arg, tmp;
   
  if (!pParams) return(_beam_set(&Beam,-2));

  // pParams initialized
  if (!pParams->Init) return(_beam_set(&Beam,-1));

  if ( pParams->SymType == 2 ) {
    // axis 2 is symmetry axis (rotate -90_deg)
    tmp      =  ssym.s_1; 
    ssym.s_1 =  ssym.s_2;
    ssym.s_2 = -tmp;
  }

  kin[0] = 0.0; kin[1] = 0.0; kin[2] = -pParams->k;
  vec_mul ( k0, pParams->InvSymRot, kin );

  sp2    = ssym.s_1*ssym.s_1; // s-parallel to sym
  ss2    = ssym.s_2*ssym.s_2; // s-perpendicular to sym
  s2     = ss2 + sp2;

  A      = - (s2*0.5+k0[0]*ssym.s_1); 
  B      = ss2; 

  k022pk032 = k0[1]*k0[1]+k0[2]*k0[2];
  if (fabs(k022pk032) < eps) 
    return( _beam_set( &Beam, -4 ) );

  arg    = k022pk032*B-A*A;
  if (arg<0.0) return( _beam_set( &Beam, -5 ) );

  arg    = sqrt(arg);

  s0[0]   = ssym.s_1;
  if (ssym.s_2<0) {
    s0[1]   = (A*k0[1] + k0[2]*arg)/k022pk032;
    s0[2]   = (A*k0[2] - k0[1]*arg)/k022pk032;
  } else {
    s0[1]   = (A*k0[1] - k0[2]*arg)/k022pk032;
    s0[2]   = (A*k0[2] + k0[1]*arg)/k022pk032;
  }

  vec_mul ( s, pParams->SymRot, s0 );

  // kout^ = (s~ + kin~)/k
  kout[0] = s[0]/pParams->k;
  kout[1] = s[1]/pParams->k;
  kout[2] = s[2]/pParams->k - 1.0;

  if ( pParams->SymType == 2 ) {
    // axis 2 is symmetry axis (rotate +90_deg)
    tmp     =  kout[0];
    kout[0] = -kout[1];
    kout[1] =  tmp;
  }
 
  Beam.cosTwoTheta = -kout[2];
  Beam.sinTwoTheta = sqrt(kout[0]*kout[0]+kout[1]*kout[1]);

  if (fabs(Beam.sinTwoTheta)>eps) {
    Beam.cosAlpha    = kout[0]/Beam.sinTwoTheta;
    Beam.sinAlpha    = kout[1]/Beam.sinTwoTheta;
  } else {
    Beam.cosAlpha    = 0.0;
    Beam.sinAlpha    = 0.0;
  }

  return( _beam_set( &Beam, 0 ) );

} // waxs_ssym2kdir

/*+++------------------------------------------------------------------------
NAME
  waxs_kdir2ssym --- calc's the saxs-coordinates of the cylindrical projection

SYNOPSIS

  WaxsCoord waxs_kdir2sym ( WaxsDir kdir )

DESCRIPTION

  Calculates the saxs-coordinates (ssym_1, ssym_2) of the cylindrical
  Ewald-sphere projection from the unit vector Beam of the scattered beam 
  in lab coordinates

RETURN VALUE

  .status==0 : ssym_1, ssym_2
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsCoord waxs_kdir2ssym ( WParams * pParams, WaxsDir Beam )
{
  WaxsCoord ssym;
  double sym[3], e1[3];
  double kvec[3];
  double svec[3];
  double s1, ssym2, tmp;

  if (!pParams) return(_s_set(&ssym,-2));

  // pParams initialized
  if (!pParams->Init) return(_s_set(&ssym,-1));                                     
// s~ = k * kout^  + k * e3^

  kvec[0] =  Beam.sinTwoTheta*Beam.cosAlpha;
  kvec[1] =  Beam.sinTwoTheta*Beam.sinAlpha;
  kvec[2] = -Beam.cosTwoTheta;

  if ( pParams->SymType == 2 ) {
    // axis 2 is symmetry axis (rotate -90_deg)
    tmp     =  kvec[0];
    kvec[0] =  kvec[1];
    kvec[1] = -tmp;
  }

  svec[0] =  kvec[0]      * pParams->k;
  svec[1] =  kvec[1]      * pParams->k;
  svec[2] = (kvec[2]+1.0) * pParams->k;

  e1[0]=1.0; e1[1]=0.0; e1[2]=0.0;
  vec_mul(sym, pParams->SymRot, e1);
  s1    = scalar_product( svec, sym );
  ssym2 = scalar_product( svec, svec );

  ssym.s_1 = s1;
  if (svec[1]*sym[0]-svec[0]*sym[1]>0) 
    ssym.s_2 = sqrt(ssym2-s1*s1); 
  else 
    ssym.s_2 = -sqrt(ssym2-s1*s1);

  if ( pParams->SymType == 2 ) {
    // axis 2 is symmetry axis (rotate +90_deg)
    tmp      =  ssym.s_1;
    ssym.s_1 = -ssym.s_2;
    ssym.s_2 =  tmp;
  }

  return( _s_set( &ssym, 0 ) );

} // waxs_kdir2ssym

/*+++------------------------------------------------------------------------
NAME
  waxs_kdir2svec --- calc's the s-vector of the scattered beam kdir

SYNOPSIS

  WaxsVector waxs_kdir2svec ( WaxsDir Beam )

DESCRIPTION

  Calculates the s-vector svec = (svec_1, svec_2, svec_3) from the 
  unit vector Beam of the scattered beam in lab coordinates

RETURN VALUE

  .status==0 : svec_1, svec_2, svec_3
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsVector waxs_kdir2svec ( WParams * pParams, WaxsDir Beam )
{
  WaxsVector svec;

  if (!pParams) return(_svec_set(&svec,-2));

  // pParams initialized
  if (!pParams->Init) return(_svec_set(&svec,-1));                                    
  svec.s_1 =  pParams->k*Beam.sinTwoTheta*Beam.cosAlpha;
  svec.s_2 =  pParams->k*Beam.sinTwoTheta*Beam.sinAlpha;
  svec.s_3 =  pParams->k*(1.0-Beam.cosTwoTheta);

  return( _svec_set ( &svec, 0 ) ); 

} // waxs_kdir2svec

/*+++------------------------------------------------------------------------
NAME
  waxs_SymInit --- Initialisation of parameters for cylindrical symmetry

SYNOPSIS

  int waxs_SymInit ( int symtype, 
                     double symrot_1, double symrot_2, double symrot_3 )

DESCRIPTION
It initializes the static parameters for cylindrical symmetry. Must be
called after waxs_Init

ARGUMENTS
k     : wavenumber                                                                
symrot_1 : ccw rotation around axis 1
symrot_2 : ccw rotation around axis 2
symrot_3 : ccw rotation around axis 3

RETURN VALUE
  returns 0 if OK

----------------------------------------------------------------------------*/
int waxs_SymInit ( WParams * pParams, int symtype, 
                    double symrot_1, double symrot_2, double symrot_3 )
{
  double Rot_1[3][3], Rot_2[3][3], Rot_3[3][3];
  double tmp[3][3];

  if (!pParams) return(-2);

  // pParams initialized
  if (!pParams->Init) return( -1 );

  // symmetry type
  pParams->SymType = symtype;

  // symmetry rotation matrix
  if (symtype != 2) {
    rotation_matrix_3 ( Rot_1, 1, symrot_1 );
    rotation_matrix_3 ( Rot_2, 2, symrot_2 );
  } else {
    rotation_matrix_3 ( Rot_1, 1,  symrot_2 );
    rotation_matrix_3 ( Rot_2, 2, -symrot_1 );
  }
  rotation_matrix_3 ( Rot_3, 3, symrot_3 );

  mat_mul_3 ( tmp, Rot_2, Rot_1 );
  mat_mul_3 ( pParams->SymRot, Rot_3, tmp );

  // inverse symmetry rotation matrix
  if (symtype != 2) {
    rotation_matrix_3 ( Rot_1, 1, -symrot_1 );
    rotation_matrix_3 ( Rot_2, 2, -symrot_2 );
  } else {
    rotation_matrix_3 ( Rot_1, 1, -symrot_2 );
    rotation_matrix_3 ( Rot_2, 2,  symrot_1 );
  }
  rotation_matrix_3 ( Rot_3, 3, -symrot_3 );

  mat_mul_3 ( tmp, Rot_2, Rot_3 );
  mat_mul_3 ( pParams->InvSymRot, Rot_1, tmp );

  return( 0 );

} // waxs_SymInit

/*+++------------------------------------------------------------------------
NAME
  waxs_Init --- Initialisation of parameters

SYNOPSIS

  int waxs_Init ( double k, double rot_1, double rot_2, double rot_3 )

DESCRIPTION
It initializes all static parameters.

ARGUMENTS
k     : wavenumber                                                                                                                  
rot_1 : ccw rotation around axis 1
rot_2 : ccw rotation around axis 2
rot_3 : ccw rotation around axis 3

RETURN VALUE
  returns 0 if OK

----------------------------------------------------------------------------*/
int waxs_Init ( WParams * pParams,
                 double k, double rot_1, double rot_2, double rot_3 )
{ 
  double Rot_1[3][3], Rot_2[3][3], Rot_3[3][3];
  double tmp[3][3];

  if (!pParams) return(-2);

  pParams->Init   = 0;

  // rotation matrix
  rotation_matrix_3 ( Rot_1, 1, rot_1 );
  rotation_matrix_3 ( Rot_2, 2, rot_2 );
  rotation_matrix_3 ( Rot_3, 3, rot_3 );

  mat_mul_3 ( tmp, Rot_2, Rot_1 );
  mat_mul_3 ( pParams->Rot, Rot_3, tmp );

  // inverse rotation matrix
  rotation_matrix_3 ( Rot_1, 1, -rot_1 );
  rotation_matrix_3 ( Rot_2, 2, -rot_2 );
  rotation_matrix_3 ( Rot_3, 3, -rot_3 );

  mat_mul_3 ( tmp, Rot_2, Rot_3 );
  mat_mul_3 ( pParams->InvRot, Rot_1, tmp );

  // wavevector k
  pParams->k = k;
  pParams->halfdk2 = 0.5/(k*k);

  // symmetry type default // isotropic scattering
  pParams->SymType = 0;

  // symmetry rotation matrix default (no rotation)
  rotation_matrix_3 ( pParams->SymRot, 1, 0.0 );
  rotation_matrix_3 ( pParams->InvSymRot, 1, 0.0 );

  pParams->Init = 1;

  return( 0 );

} // waxs_Init

/*+++------------------------------------------------------------------------
NAME
  waxs_not_init --- check initialization

SYNOPSIS

  int waxs_not_init ( void );

DESCRIPTION
  Checks whether the parameters have been initialized.

ARGUMENTS
  void

RETURN VALUE
  returns 0 if initialized, otherwise 1
----------------------------------------------------------------------------*/
int waxs_not_init ( WParams *pParams )
{ if (!pParams) return(0);
  else return( pParams->Init?0:1 );
} // waxs_not_init

/*+++------------------------------------------------------------------------
NAME
  waxs_get_transform --- return transformation mode 

SYNOPSIS

  int waxs_get_transform( int proin, int proout );

DESCRIPTION
  Determines the transformation mode from the input and output projection types

ARGUMENTS
  input projection type proin (IO_ProSaxs, IO_ProWaxs)
  output projection type proout (IO_ProSaxs, IO_ProWaxs)

RETURN VALUE
  -1: inverse transformation (WAXS->SAXS)
   0: no transformation
   1: normal transformation (SAXS->WAXS)
----------------------------------------------------------------------------*/
int waxs_get_transform( int proin, int proout )
{ int transform=0;

  if ( proin!=proout ) {
    /* There can be more projections defined as saxs and waxs */
    if ((proin==IO_ProSaxs)&&(proout==IO_ProWaxs)) transform=1; // normal transformation
     else if ((proin==IO_ProWaxs)&&(proout==IO_ProSaxs)) transform=-1; // inverse transformation
  }

  return( transform );

} // waxs_to_transform()

/*+++------------------------------------------------------------------------
NAME

  waxs_Saxs --- calculation of saxs coordinate from s-projection 

SYNOPSIS

  WaxsCoord waxs_Saxs ( WaxsCoord sp )

DESCRIPTION

  Calculates the saxs-coordinate s of the inclined detector image 
  from the saxs-coordinate sp of the Ewald sphere-projection.

ARGUMENT

  WaxsCoord sp : saxs-coordinate of the Ewald sphere projection

RETURN VALUE
  WaxsCoord s  : saxs-coordinate of the inclined detector image
  s.status==0 in case of success 
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsCoord waxs_Saxs ( WParams * pParams, WaxsCoord sp ) 
{
  WaxsDir kdir;
  WaxsCoord sout;

  if (!pParams) return(_s_set( &sout,-2));

  // pParams initialized
  if (!pParams->Init) return(_s_set( &sout,-1));
  
  if (pParams->SymType)
    kdir = waxs_ssym2kdir ( pParams, sp );
  else kdir = waxs_sp2kdir ( pParams, sp );
  if (kdir.status) return( _s_set( &sout,kdir.status*10-2) );

  sout = waxs_kdir2s ( pParams, kdir );
  if (sout.status) return( _s_set( &sout,sout.status*10-2) );

  return( _s_set( &sout, 0 ) );

} // waxs_Saxs

/*+++------------------------------------------------------------------------
NAME
 
  waxs_Waxs --- calculation of s-projection from saxs coordinate s
 
SYNOPSIS
 
  WaxsCoord waxs_Waxs ( WaxsCoord s )
 
DESCRIPTION
 
  Calculates the saxs-coordinate sp of the Ewald sphere-projection
  from the saxs-coordinate s of the inclined detector image.
 
ARGUMENT
 
  WaxsCoord s : saxs-coordinate s of the inclined detector image
 
RETURN VALUE
  WaxsCoord sp : saxs-coordinate sp of the Ewald sphere-projection
  s.status==0 in case of success 
  .status<0  : error
  .status<-1 : no solution
 
----------------------------------------------------------------------------*/
WaxsCoord waxs_Waxs ( WParams * pParams, WaxsCoord s )
{
  WaxsDir kdir;
  WaxsCoord spout;

  if (!pParams) return(_s_set( &spout,-2));
 
  // pParams initialized
  if (!pParams->Init) return(_s_set( &spout,-1));

  kdir = waxs_s2kdir ( pParams, s );
  if (kdir.status) return( _s_set( &spout,kdir.status*10-2) );

  if (pParams->SymType)
    spout = waxs_kdir2ssym ( pParams, kdir );
  else spout = waxs_kdir2sp ( pParams, kdir );
  if (spout.status) return( _s_set( &spout,spout.status*10-2) );

  return( _s_set( &spout, 0 ) );
 
} // waxs_Waxs

/*+++------------------------------------------------------------------------
NAME

  waxs_Saxs2Vector --- calculation of s-projection from saxs coordinate s

SYNOPSIS

  WaxsVector waxs_Saxs2Vector ( WaxsCoord s )

DESCRIPTION

  Calculates the s-vector svec from the saxs-coordinate s 
  of the inclined detector image.

ARGUMENT

  WaxsCoord s : saxs-coordinate s of the inclined detector image
  
RETURN VALUE

  .status==0 : svec_1, svec_2, svec_3
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsVector waxs_Saxs2Vector ( WParams * pParams, WaxsCoord s )
{
  WaxsDir kdir;
  WaxsVector svec;

  if (!pParams) return( _svec_set ( &svec, -2 ) );

  // pParams initialized
  if (!pParams->Init) return(_svec_set( &svec,-1));

  kdir = waxs_s2kdir ( pParams, s );
  if (kdir.status) return( _svec_set( &svec,kdir.status*10-2) );

  svec = waxs_kdir2svec ( pParams, kdir );
  if (svec.status) return( _svec_set( &svec,svec.status*10-2) );

  return( _svec_set ( &svec, 0 ) );

} // waxs_Saxs2Vector

/*+++------------------------------------------------------------------------
NAME

  waxs_Uni2Iso --- uniaxial WAXS projection to isotropic WAXS projection

SYNOPSIS

  WaxsCoord waxs_Uni2Iso ( WaxsCoord ssym );

DESCRIPTION

  Calculates the saxs-coordinate sp of an isotropic WAXS projection
  from the saxs-coordinate ssym of an uniaxial symmetric WAXS projection.

ARGUMENT

  WaxsCoord ssym : saxs-coordinate of the uniaxial symmetric 
                   Ewald sphere projection 

RETURN VALUE
  WaxsCoord sp : saxs-coordinate of the isotropic Ewald sphere projection
  s.status==0 in case of success
  .status<0  : error
  .status<-1 : no solution

----------------------------------------------------------------------------*/
WaxsCoord waxs_Uni2Iso ( WParams * pParams, WaxsCoord ssym )
{
  WaxsDir kdir;
  WaxsCoord spout;
  
  if (!pParams) return(_s_set( &spout,-1));

  // pParams initialized
  if (!pParams->Init) return(_s_set( &spout,-1));
  
  if (pParams->SymType) {
    kdir = waxs_ssym2kdir ( pParams, ssym );
    if (kdir.status) return( _s_set( &spout,kdir.status*10-2) );
    spout = waxs_kdir2sp ( pParams, kdir );
    if (spout.status) return( _s_set( &spout,spout.status*10-2) );
  } else spout = ssym;
  
  return( _s_set( &spout, 0 ) );
  
} // waxs_Uni2Iso

/*+++------------------------------------------------------------------------
NAME
  
  waxs_Iso2Uni --- isotropic WAXS projection to uniaxial WAXS projection

SYNOPSIS
  
  WaxsCoord waxs_Iso2Uni ( WaxsCoord sp );

DESCRIPTION

  Calculates the saxs-coordinate ssym of an uniaxial symmetric WAXS projection
  from the saxs-coordinate sp of an isotropic WAXS projection.

ARGUMENT

  WaxsCoord sp : saxs-coordinate of the isotropic Ewald sphere projection

RETURN VALUE
  WaxsCoord ssym : saxs-coordinate of the uniaxial symmetric
                   Ewald sphere projection
  s.status==0 in case of success
  .status<0  : error
  .status<-1 : no solution
  
----------------------------------------------------------------------------*/
WaxsCoord waxs_Iso2Uni ( WParams * pParams, WaxsCoord sp )
{
  WaxsDir kdir;
  WaxsCoord ssymout;
  
  if (!pParams) return(_s_set( &ssymout,-2));

  // pParams initialized
  if (!pParams->Init) return(_s_set( &ssymout,-1));
  
  if (pParams->SymType) {
    kdir = waxs_sp2kdir ( pParams, sp );
    if (kdir.status) return( _s_set( &ssymout,kdir.status*10-2) );
    ssymout = waxs_kdir2ssym ( pParams, kdir );
    if (ssymout.status) return( _s_set( &ssymout,ssymout.status*10-2) );
  } else ssymout = sp;

  return( _s_set( &ssymout, 0 ) );

} // waxs_Iso2Uni

/*---------------------------------------------------------------------------
 NAME

    waxs_Transform --- return transformed coordinate 

 SYNOPSIS

    WaxsCoord waxs_Transform(int transform, WaxsCoord W);

 DESCRIPTION

    The routine calculates the transformed coordinate of W. 
    If transform is 0 the coordinate W is returned, 
    if transform is -1 the Waxs coordinate of W is returned,
    if transform is  1 the SAXS coordinate of W is returned

    The arguemtn transform can be multiplied by -1 to
    invert the calculation.

 RETURN VALUE

    transformed coordinate
 
---------------------------------------------------------------------------*/
WaxsCoord waxs_Transform(WParams * pParams, int transform, WaxsCoord W)
{ WaxsCoord WT;

  if (transform) {
    switch (transform) {
      case -1: WT = waxs_Waxs ( pParams, W ); // inverse
               break;
      case  1: WT = waxs_Saxs ( pParams, W ); // direct
               break;
      default: W.status=1; WT = W;   // error
    }
  } else { W.status=0; WT = W; }     // no projection transformation

  return(WT);

} // waxs_Transform 

/*---------------------------------------------------------------------------
 NAME

   waxs_Range --- calculates waxs range from saxs image parameters

 SYNOPSIS

  int waxs_Range( int proin, int proout,
                  long  dim_1, long dim_2,
                  float off_1, float pix_1, float cen_1,
                  float off_2, float pix_2, float cen_2,
                  float dis, float wvl,
                  WaxsCoord *Wmin, WaxsCoord *Wmax, int * pstatus);


 DESCRIPTION
   off_1 to wvl are the parameters of the untransformed image (projection
   type Saxs). The range in saxs coordinates of the transformed image
   (projection type Waxs) are calculated and returned in Wmin and Wmax.
   Because the output area is not necessarily rectangular, parts of the
   output can be outside the range described by Wmin and Wmax.

 RETURN VALUE
   (returns value determined with waxs_get_transform)
  -1: inverse transformation (WAXS->SAXS)
   0: no transformation
   1: normal transformation (SAXS->WAXS)

   status returned in *pstatus : 0: success, otherwise failed

---------------------------------------------------------------------------*/
int waxs_Range( WParams * pParams, int proin, int proout,
                 long  dim_1, long dim_2,
                 float off_1, float pix_1, float cen_1,
                 float off_2, float pix_2, float cen_2,
                 float dis, float wvl,
                 WaxsCoord *Wmin, WaxsCoord *Wmax, int * pstatus)

{ const float eps=1e-32;

  WaxsCoord W, WOut;
  float s_11, s_12, s_21, s_22, smin_1, smax_1, smin_2, smax_2;

  int transform;

  if (!pParams) return(-2);

  *pstatus = -1;

  transform = waxs_get_transform(proin,proout);

  if (fabs(pix_1)<=eps) goto waxs_Range_error;
  if (fabs(pix_2)<=eps) goto waxs_Range_error;
  if (fabs(wvl)<=eps) goto waxs_Range_error;
  if (fabs(dis)<=eps) goto waxs_Range_error;

  /* WSaxs   = INDEX2S(IIndex,Offset,Psize,Center,SampleDistance,WaveLength); */
  s_11 = INDEX2S(INDEXSTART+LOWERBORDER,off_1,pix_1,cen_1,dis,wvl);
  s_12 = INDEX2S(INDEXSTART+LOWERBORDER+dim_1,off_1,pix_1,cen_1,dis,wvl);
  s_21 = INDEX2S(INDEXSTART+LOWERBORDER,off_2,pix_2,cen_2,dis,wvl);
  s_22 = INDEX2S(INDEXSTART+LOWERBORDER+dim_2,off_2,pix_2,cen_2,dis,wvl);

  W.s_1 = s_11; W.s_2 = s_21;
  WOut = waxs_Transform(pParams, -transform, W);
  if (WOut.status) goto waxs_Range_error;
  smin_1 = WOut.s_1; smax_1 = WOut.s_1;
  smin_2 = WOut.s_2; smax_2 = WOut.s_2;

  W.s_1 = s_12; W.s_2 = s_21;
  WOut = waxs_Transform(pParams, -transform, W);
  if (WOut.status) goto waxs_Range_error;
  smin_1 = MIN2(smin_1,WOut.s_1); smax_1 = MAX2(smax_1,WOut.s_1);
  smin_2 = MIN2(smin_2,WOut.s_2); smax_2 = MAX2(smax_2,WOut.s_2);

  W.s_1 = s_12; W.s_2 = s_22;
  WOut = waxs_Transform(pParams, -transform, W);
  if (WOut.status) goto waxs_Range_error;
  smin_1 = MIN2(smin_1,WOut.s_1); smax_1 = MAX2(smax_1,WOut.s_1);
  smin_2 = MIN2(smin_2,WOut.s_2); smax_2 = MAX2(smax_2,WOut.s_2);

  W.s_1 = s_11; W.s_2 = s_22;
  WOut = waxs_Transform(pParams, -transform, W);
  if (WOut.status) goto waxs_Range_error;
  smin_1 = MIN2(smin_1,WOut.s_1); smax_1 = MAX2(smax_1,WOut.s_1);
  smin_2 = MIN2(smin_2,WOut.s_2); smax_2 = MAX2(smax_2,WOut.s_2);

  /* backward or forward projection */
  if (transform==1) { // direct transformation SAXS->WAXS
    /* In case that the input image coordinates are transformed from Saxs to
       Waxs (Inverse is FALSE) it must be checked whether the original
       pattern contains the backscattering vector (180 degree scattering)
       This is the case if the origin (0,0) cannot be projected on the 
       detector plane (transform(transform, FALSE, W) fails) and if the
       the origin (0,0) lies inside the found edges (smin and smax).
       In this case the modulus of the maximum scattering vector is 
       (2*WAVENUMBER(WaveLength). */

    W.s_1 = 0.0; W.s_2 = 0.0;
    WOut = waxs_Transform(pParams, transform, W);

    if (WOut.status) {
      /* backward projection */
      if ( (smin_1*smax_1<0)&&(smin_2*smax_2<0) ) {
        smax_1 = 2.0*WAVENUMBER(wvl); smin_1 = -smax_1;
        smax_2 = 2.0*WAVENUMBER(wvl); smin_2 = -smax_2;
      }
    }
  }

  Wmin->s_1 = smin_1; Wmin->s_2 = smin_2;
  Wmax->s_1 = smax_1; Wmax->s_2 = smax_2;

  *pstatus = 0;

  return( transform );

waxs_Range_error:

  return( transform );

} // waxs_Range

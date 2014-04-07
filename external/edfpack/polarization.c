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
# define POLARIZATION_VERSION "polarization : V1.62 Peter Boesecke 2013-04-16"
/*+++------------------------------------------------------------------------
NAME

   polarization --- routines for polarization correction 

SYNOPSIS

   # include waxs.h
   # include polarization.h

HISTORY

  2004-07-27 V1.0 
  2004-07-28 V1.1 polarization_Init(..., int Invert) added
  2004-10-31 V1.2 polarization_Init(..., double Factor, ...) added
  2010-03-18 V1.3 include reference.h for ProjectionType
  2010-12-12-V1.5 unused PolarizationParams removed
  2011-06-22 V1.6 orientation recalculation 1..16 added
  2011-07-11 V1.61 polarization.h: Ori was double, defined as long int
  2013-04-16 V1.62 Description updated with Thomson scattering

DESCRIPTION

The function polarization_factor calculates the polarization factor for
coordinates in the SAXS reference system (see *). The image can 
either be an Ewald-sphere projection of a scattering pattern 
(waxs-projection) or a flat detector pattern (saxs-projection).

Three parameters describe the polarization of the incident beam: the 
polarization P (0<=P<=1), the ellipticity PChi (-pi/4<=PChi<=pi/4) and 
the inclination PPsi (0<=PPsi<=pi) of the polarization plane. The 
ellipticity PChi is zero for linear polarization. For circular
polarization its absolute value is pi/4 and smaller for elliptical
polarization. The polarization factor is symmetric for PChi and 
therefore independent of the helicity.

The angle PPsi describes a ccw rotation of the polarization plane 
around axis x_3 with respect to the x_1 axis in orientation 1. 

Orientation 1 corresponds to a righhanded coordinate system with the axes
x_1, x_2 and x_3 where x_1 is horizontal and pointing to the right,
axis x_2 is pointing upwards and axis x_3 against the observer and against
the travelling direction of the incident beam. The image is observed in
the x_1, x_2 plane.

For a different orientation an internal OPsi is recalculated accordingly:

orientation      OPsi

1 :  1, 2, 3     PPsi
2 : -1, 2, 3    -PPsi+pi
3 :  1,-2, 3    -PPsi
4 : -1,-2, 3     PPsi-pi
5 :  2, 1, 3    -PPsi+pi/2
6 :  2,-1, 3     PPsi-pi/2
7 : -2, 1, 3     PPsi+pi/2
8 : -2,-1, 3    -PPsi-pi/2
9 :  1, 2,-3     PPsi
10: -1, 2,-3    -PPsi+pi
11:  1,-2,-3    -PPsi
12: -1,-2,-3     PPsi-pi
13:  2, 1,-3    -PPsi+pi/2
14:  2,-1,-3     PPsi-pi/2
15: -2, 1,-3     PPsi+pi/2
16: -2,-1,-3    -PPsi-pi/2

In the following, 3d vectors are followed by '~', the length of a vector 
is just its name: e.g. kin = ||kin~||. Vectors with unit length are
followed by '^', e.g. kin^ = kin~/kin. 

      kin~              : wavevector of incident beam
(i)   kout~             : wavevector of scattered beam
      s~ = kout~ - kin~ : scattering vector

The scattering is elastic. The wavenumber k of kin~ and kout~ is:

(ii) k = 1/wavelength = kin = kout

Scattering Geometry

The input image must be an Ewald sphere projection of the scattering pattern
as created with saxs_waxs. The unit vectors in lab space are: e1^, e2^, e3^. 
Axis 3 in lab space is parallel to axis 3 of the projection. The azimuths of 
the axes 1 and 2 of the projections are identical to the azimuths of the axes 1
and 2 in lab space. For details see waxs.c.

The incident beam (kin~) is antiparallel to axis3.

(iii) kin~ = -kin * e3^

The saxs-coordinates sp_1 and sp_2 of the input image are:

(iv)  sp_1 = s * cos(alpha)
      sp_2 = s * sin(alpha)

From sp_1 and sp_2 the direction kout^ of the scattered beam is calculated
using the routine waxs_sp2kdir:

               ( sin(2Theta)*cos(alpha) )   ( kout1 )
(v)   kout^ =  | sin(2Theta)*sin(alpha) | = | kout2 |
               (     -cos(2Theta)       )   ( kout3 )

*) coordinates in the SAXS reference system:

     sp_1 =  k * ((x_1+off_1) - cen_1) * (pix_1/dis)    
     sp_2 =  k * ((x_2+off_2) - cen_2) * (pix_2/dis)

where x_1, x_2 are the pixel coordinates, off_1,off_2, the offsets, 
cen_1,cen_2 the point of normal incidence ("poni", "center"), pix_1,pix_2, 
the pixel sizes, dis the distance between the sample and the point of 
normal incidence and k the wavenumber (1/wavelength). SAXS-coordinates are
correspondingly defined in the unprojected and in the projected image.

Polarization factor

At low energies (E/c^2 << electron mass) the scattering of photons by
atomically bound electrons is elastic and is described by the Thomson 
differential cross section. It relates the flux density Phiin [ph/s/m2] of 
the incoming photons with the spherical flux Iout [ph/s/sr] of the scattered 
photons per steradian:

  dsigmaThomson/dOmega = Iout/Phiin = re^2 * P(Ein,Eout)

re = ec^2/(4*pi*e0*me*c^2) is the classical electron radius and P(Ein,kin,kout)
is the polarization factor. It is a function of the plane of polarization and 
the direction of the incoming photon (Ein, kin) and the direction of the 
scattered photon (kout).

Integrating the Thomson differential cross over all directions (4 pi) 
yields the Thomson cross section sigmaThompson:

  sigmaThompson = 8/3 pi re^2

The E-vector Eout of the scattered beam kout is proportional to the projection 
of the E-vector Ein of the incident beam kin to the transversal plane of the 
scattered beam:

(vi)  Eout = f * ( Ein - kout^ (kout^*Ein) )

The scattered intensity Iout is given by

(vii) Iout = <Eout * Eout*>

where Eout* denotes the complex conjugate of Eout. Applying eq. vii to eq. vi
gives the result

(viii) Iout = f*f ( <Ein * Ein*> - <(kout^ * Ein)*(kout^ * Ein*)> )

Die incident wave is transverse to axis 3 and can be described with

                ( a1*exp(i*(phi1-2*pi*ny*t)) )
(ix)   Ein   =  | a2*exp(i*(phi2-2*pi*ny*t)) |
                (             0              )

where a1 and a2 are the real electric amplitudes, phi1 and phi2 the phases, 
ny the frequency and t the time. The description of polarization follows the 
notation of Born and Wolf [1].

Inserting eq. 9 into eq. viii returns the result

      Iout = f*f * (   (1-kout1*kout1) * (s0+s1)/2
(x)                  + (1-kout2*kout2) * (s0-s1)/2
                     -    kout1*kout2  *    s2      )

where s0, s1 and s2 are the Stokes parameter of the incident wave. The Stokes 
parameters describe the polarization of the incident wave and are related
to a1, a2 and delta=phi1-phi2 in the following way.

(xi a)                <a1*a1> = (so+s1)/2 
(xi b)                <a2*a2> = (so-s1)/2 
(xi c)   <2*a1*a2*cos(delta)> =    s2 
(xi d)   <2*a1*a2*sin(delta)> =    s3

The Stokes parameter of the incident wave can be separated into an unpolarized 
s(1) and a polarized part s(2):

(xii a) s = s(1) + s(2) = (       s0, s1, s2, s3 )
(xii b)            s(1) = ( (1-P)*s0,  0,  0,  0 )
(xii c)            s(2) = (     P*s0, s1, s2, s3 )

where P is the polarization (0<=P<=1). If P is 0 the wave is totally 
unpolarized, if P is 1 the wave is totally polarized.

The polarized portion s(2) can be described by the Poincaré notation:

(xiii a) s0(2)+s1(2) = P*so+s1 = P*so*(1+cos(2*PChi)*cos(2*PPsi))
(xiii b) s0(2)-s1(2) = P*so-s1 = P*so*(1-cos(2*PChi)*cos(2*PPsi))
(xiii c)       s2(2) =      s2 = P*so*cos(2*PChi)*sin(2*PPsi)

with

P     : degree of polarization (0<=P<=1)
PChi  : ellipticity (after Poincaré) (-pi/4<=PChi<=+pi/4)
        PChi=-pi/4 left hand (cw) circular polarization
        PChi<0 left hand polarization
        PChi==0 linear polarization
        PChi>0 right hand polarization
        PChi=pi/4 right hand (ccw) circular polarization
PPsi  : inclination of the plane of polarization (after Poincaré) (0<=PPsi<pi)
        PPsi is the angle between axis x_1 and the plane of polarization 

Because the angles PChi and PPsi are defined in a mirrored coordinate system 
with the incident beam parallel to e3^ (and not antiparallel) the signs of the
angle PPsi must be altered if used in the standard SAXS coordinate system. The
result is independent of the sign of PChi.

The scattering intensity Iout can also be splitted into an unpolarized and 
a polarized part:

(xiv a) Iout = Iout(1) + Iout(2)
(xiv b)        Iout(1) = f*f * (1-P)*s0 * ( 1 + kout3*kout3 )/2
(xiv c)        Iout(2) = f*f *    P *s0 * (
                            (1-kout1*kout1) * 0.5 * (1+cos(2*PChi)*cos(2*PPsi))
                          + (1-kout2*kout2) * 0.5 * (1-cos(2*PChi)*cos(2*PPsi))
                          -   kout1*kout2   *   cos(2*PChi)*sin(2*PPsi)  )

The number of scattered photons per steradian Iout scattered by a cloud of 
n/A independent electrons n per area A that is illuminated with Iin incident 
photons is:

(xv)  Iout = Iin * (n/A * re*re) * polarization_factor(sp)

where Iin=s0 is the incident beam intensity, n/A the number of electrons per
cross section and re the classical electron radius.

For small scattering angles ||sp||/k<<1 the polarization_factor approaches 1. 

The factor f*f is equal to n/A*re*re, where n/A is area density of electrons
and re is the classical electron radius.

[1] Born&Wolf, Principles of Optics, 6th (corrected) edition 1997, 
    Cambridge University Press 1980, §10.8 and §1.4.

USAGE

  include "waxs.h"
  include "polarization.h"
  include "reference.h"

  PParams pparams;
  double factor;
  int invert, projection;
  WaxsCoord wc;
  double K, rot1, rot2, rot3;
  double pol, pchi, ppsi;
  double polfac;  // optional variable for polarization factor


  factor = 1.0; // to calculate #electrons/nm^3 set factor to re^2/nm^3, e.g.
                // factor = num_str2double ( "re*re/nm3", NULL, &errval );

  invert=0;     // 1 to invert the calculated polarization factor

  projection = IO_ProSaxs;  // IO_ProSaxs (flat detector pattern) or
                            // IO_ProWaxs (Ewald sphere projection)

  K = WAVENUMBER(wavelength_m);
  rot1 = 0.0; // rotation around lab axis 1 [rad]
  rot2 = 0.0; // rotation around lab axis 2 [rad]
  rot3 = 0.0; // rotation around lab axis 3 [rad]

  pol  = 1.0; // degree of polarization (0<=P<=1)
  pchi = 0.0; // ellipticity (after Poincaré) (-pi/4<=PChi<=+pi/4)
      //    PChi=-pi/4 left hand (cw) circular polarization
      //    PChi<0 left hand polarization
      //    PChi==0 linear polarization
      //    PChi>0 right hand polarization
      //    PChi=pi/4 right hand (ccw) circular polarization
  ppsi = 0.0; // inclination of the plane of polarization  (after Poincaré) 
      //    (0<=PPsi<pi)

  Because the angles pchi and ppsi are defined for a mirrored coordinate system
  with the incident beam parallel to e3^ (and not antiparallel) the sign of the
  angle ppsi must be altered during initialization if the Saxs convention is
  used (beam antiparallel to e3^). The polarization factor is independent of
  the sign of pchi.

  polarization_Init(&pparams, ori,
                    K, rot1, rot2, rot3, pol, pchi, -ppsi, factor, invert);

  for (i2) {
    for (i1) {
      // The reference system of wc must be IO_Saxs
      wc.s_1 = INDEX2S(i1,Offset_1,PSize_1,Center_1,SampleDistance,WaveLength);
      wc.s_2 = INDEX2S(i2,Offset_2,PSize_2,Center_2,SampleDistance,WaveLength);

      polfac = polarization_factor ( &pparams, wc, projection );
      if (polfac<0) continue; // error

      ...

    }
  }

----------------------------------------------------------------------------*/

/******************************************************************************
* Include Files                                                               *
******************************************************************************/

# include "polarization.h"
# include "reference.h" // for ProjectionType

/******************************************************************************
* Private Constants                                                           *
******************************************************************************/

# define R_PI 3.1415926535897932384626

// static const double deg2rad   = R_PI/180.0;
// static const double rad2deg   = 180.0/R_PI;
// static const double pi        = R_PI;
// static const double halfpi    = R_PI*0.5;
static const double quarterpi = R_PI*0.25;
// static const double twopi     = R_PI*2.0;
// static const double one       = 1.0;
// static const double eps       = 1e-30;
static const double qpi_eps   = 1e-6;

/******************************************************************************
* Routines                                                                    *
******************************************************************************/

void polarization_PrintParams ( FILE * out, PParams Params )
{ PParams * pParams = &Params;
  if (!pParams->Init) return;
  fprintf(out," Init                       = %d\n", pParams->Init);
  fprintf(out," Ori                        = %ld\n", pParams->Ori);
  fprintf(out," P                          = %lg\n", pParams->P);
  fprintf(out," PChi                       = %lg\n", pParams->PChi);
  fprintf(out," PPsi                       = %lg\n", pParams->PPsi);
  fprintf(out," Factor                     = %lg\n", pParams->Factor);
  fprintf(out," Invert                     = %d\n", pParams->Invert);
  fprintf(out," halfOnePlusCos2ChiCos2Psi  = %lg\n",
    pParams->halfOnePlusCos2ChiCos2Psi);
  fprintf(out," halfOneMinusCos2ChiCos2Psi = %lg\n",
    pParams->halfOneMinusCos2ChiCos2Psi);
  fprintf(out," Cos2ChiSin2Psi             = %lg\n", pParams->Cos2ChiSin2Psi);

  waxs_PrintParams( out, pParams->wparams );

} //  polarization_PrintParams 

/*+++------------------------------------------------------------------------
NAME
  polarization_Init --- Initialisation of parameters

SYNOPSIS

  int polarization_Init ( PParams * pParams, long ori,
                          double k, double rot1, double rot2, double rot3,
                          double P, double PChi, double PPsi, double Factor,
                          int Invert );

DESCRIPTION
It initializes all static parameters.

ARGUMENTS
ori   : orientation (default: 1)
k     : wavenumber
rot1, 
rot2, 
rot3  : detector rotations as defined in waxs.c
P     : degree of polarization (0<=P<=1)
PChi  : ellipticity (after Poincaré) (-pi/4<=PChi<=+pi/4)
        PChi=-pi/4 left hand (cw) circular polarization
        PChi<0 left hand polarization
        PChi==0 linear polarization
        PChi>0 right hand polarization
        PChi=pi/4 right hand (ccw) circular polarization
PPsi  : inclination of the plane of polarization  (after Poincaré) (0<=PPsi<pi)
Factor: positive multiplication factor larger than 0
Invert: switch for the function polarization_factor:
        0: the polarization factor P multiplied with Factor is calculated
           (P*Factor),
           range of returned values (0..Factor)
        1: the inverse of the polarization factor P divided by Factor is
           calculated (1/(P*Factor)),
           range of returned value (1/Factor..inf), instead of inf the
           value 0 is returned.

RETURN VALUE
  returns 0 if OK
  otherwise error

----------------------------------------------------------------------------*/
int polarization_Init ( PParams * pParams, long ori,
                        double k, double rot1, double rot2, double rot3,
                        double P, double PChi, double PPsi, double Factor,
                        int Invert )
{
  double Cos2Chi, Sin2Chi, Cos2Psi, Sin2Psi;
  double OPsi;

  if (!pParams) return(-2);

  pParams->Init   = 0;

  // Initialize waxs (not rotated)
  if ( waxs_Init ( &(pParams->wparams), k, rot1, rot2, rot3 ) ) return( -1 );

  // Polarization
  if ( ( P < 0.0 ) || ( P > 1.0 ) ) return( -1 );
  pParams->P    = P;

  // Orientation change
  if (ori<0) ori=raster_inversion ( -ori  );
  pParams->Ori = ori;

  switch (ori) {
    case 2: // 2 : -1, 2, 3     -PPsi+R_PI
    case 10: // 10: -1, 2,-3    -PPsi+R_PI
      OPsi=-PPsi+R_PI;
      break;
    case 3: // 3 :  1,-2, 3     -PPsi
    case 11: // 11:  1,-2,-3    -PPsi 
      OPsi=-PPsi;
      break;
    case 4: // 4 : -1,-2, 3      PPsi-R_PI
    case 12: // 12: -1,-2,-3     PPsi-R_PI
      OPsi=PPsi-R_PI;
      break;
    case 5: // 5 :  2, 1, 3     -PPsi+R_PI/2
    case 13: // 13:  2, 1,-3    -PPsi+R_PI/2
      OPsi=-PPsi+R_PI/2;
    case 6: // 6 :  2,-1, 3      PPsi-R_PI/2
    case 14: // 14:  2,-1,-3     PPsi-R_PI/2
      OPsi=PPsi-R_PI/2;
      break;
    case 7: // 7 : -2, 1, 3      PPsi+R_PI/2
    case 15: // 15: -2, 1,-3     PPsi+R_PI/2
      OPsi=PPsi+R_PI/2;
      break;
    case 8: // 8 : -2,-1, 3     -PPsi-R_PI/2
    case 16: // 16: -2,-1,-3    -PPsi-R_PI/2
      OPsi=-PPsi-R_PI/2;
      break;
    default: // 1 :  1, 2, 3     PPsi
             // 9 :  1, 2,-3     PPsi
      OPsi=PPsi;
  }

  // Poincaré parameters
  if ( ( PChi<-quarterpi-qpi_eps ) || ( PChi>quarterpi+qpi_eps ) ) return( -1 );
  pParams->PChi = PChi;
  pParams->PPsi = OPsi;

  if ( Factor <= 0 ) return( -1 );
  pParams->Factor=Factor;

  pParams->Invert=Invert;

  Cos2Chi=cos(2.0*PChi);
  Sin2Chi=sin(2.0*PChi);
  Cos2Psi=cos(2.0*OPsi);
  Sin2Psi=sin(2.0*OPsi);

  pParams->halfOnePlusCos2ChiCos2Psi  = (1.0+Cos2Chi*Cos2Psi)*0.5;
  pParams->halfOneMinusCos2ChiCos2Psi = (1.0-Cos2Chi*Cos2Psi)*0.5;
  pParams->Cos2ChiSin2Psi             = Cos2Chi*Sin2Psi;

  pParams->Init = 1;

  return( 0 );

} // polarization_Init

/*+++------------------------------------------------------------------------
NAME

  polarization_factor --- calculates the polarization factor

SYNOPSIS

  double polarization_factor ( PParams * pParams,
                               WaxsCoord wc, int projection )

DESCRIPTION

  Calculates the polarization factor from the saxs-coordinate wc of
  the Ewald sphere-projection.

ARGUMENT

  WaxsCoord wc   : World Coordinate (SAXS or WAXS)
  int projection : World Coordinate type (ProjectionType)

                 IO_NoPro   : invalid (0)
                 IO_ProSaxs : flat detector (SAXS-coordinate) (1)
                 IO_ProWaxs : Ewald sphere-projection (WAXS-coordinate) (2)

  Attention, analogue definition of ProjectionType in SaxsImage.h, do not
  change order.

RETURN VALUE
  double polarization factor >= 0:
          in case of an error the returned value is negative

----------------------------------------------------------------------------*/
double polarization_factor ( PParams * pParams,
                             WaxsCoord wc, int projection )
{ 
  double kvec[3];
  WaxsDir kdir;
  double Iu, Ip; 
  double Value;

  if (!pParams) return(-2);

  // pParams initialized
  if (!pParams->Init) return( -1 );
  
  switch ( projection ) {
    case IO_ProSaxs: // calculate kdir from scattering vector s
            kdir = waxs_s2kdir  ( &(pParams->wparams), wc );
            break;
    case IO_ProWaxs: // calculate kdir from Ewald - sphere projection 
            kdir = waxs_sp2kdir ( &(pParams->wparams), wc );
            break;
    default: return( -1 );
  }
  if (kdir.status) return( -1 );

  kvec[0] =  kdir.sinTwoTheta*kdir.cosAlpha;
  kvec[1] =  kdir.sinTwoTheta*kdir.sinAlpha;
  kvec[2] = -kdir.cosTwoTheta;

  // unpolarized part 
  Iu = (1.0-pParams->P)*0.5*(1.0+kvec[2]*kvec[2]);

  Ip = pParams->P*(   (1.0-kvec[0]*kvec[0])*pParams->halfOnePlusCos2ChiCos2Psi
                    + (1.0-kvec[1]*kvec[1])*pParams->halfOneMinusCos2ChiCos2Psi
                    +    kvec[0]*kvec[1] *pParams->Cos2ChiSin2Psi   );

  Value = (Iu+Ip)*pParams->Factor;

  if (pParams->Invert) {
    if (Value>0) Value=1.0/Value;
    else return( -1 );
  }

  return( Value );

} // polarization_factor

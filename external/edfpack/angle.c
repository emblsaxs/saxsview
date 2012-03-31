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

/*+++------------------------------------------------------------------------
NAME

   angle --- routines for angular averaging 

SYNOPSIS

   # include angle.h

HISTORY
  2005-10-08 V1.0  Peter Boesecke
  2005-10-29 V1.1  PB *pstatus = Success; added
  2006-05-13 V1.2  PB angle_limits and checks for Min/MaxAngle and Min/MaxRadius
                      added
  2007-04-19 V1.3  PB -Wall compiler warnings resolved
  2009-10-02 V1.4  PB Success -> 0, SaxsAngle -> angle
                      include only ipol.h and reference.h

DESCRIPTION

   * Attention:
   * - all values of the output array must have been preset with the value
   *   "I0Dummy"

----------------------------------------------------------------------------*/
/******************************************************************************
* Include Files                                                               *
******************************************************************************/

# include "angle.h"

/******************************************************************************
* Private Constants                                                           *
******************************************************************************/

static const double rad2deg = 180.0/NUM_PI;
static const float  twopi = 2.0*NUM_PI;

/******************************************************************************
* Private Definitions                                                         *
******************************************************************************/

# define CALC_DIST(A) sqrt((A[0])*(A[0])+(A[1])*(A[1]))

/******************************************************************************
* Routines                                                                    *
******************************************************************************/

/*+++------------------------------------------------------------------------
NAME

   angle_limits 

SYNOPSIS

   float angle_limits( int mode, int I1Dim_1, int I1Dim_2,
                       float Off_11, float Ps_11, float Off_21, float Ps_21,
                       float Wcenter_1, float Wcenter_2 )

DESCRIPTION
  Returns the maximum or minimum distances and the maximum or minimum azimuthal
  angles of the edges in world coordinates from the center,
  depending on mode. The center is (Wcenter_1,Wcenter_2) in the world system.

  The returned distances are absolute values in world distances,
  the angles are returned in radian.

  mode  1: minimum radius
  mode  2: maximum radius
  mode -1: minimum angle
  mode -2: maximum angle

                     |         |
                     |         |
                G    |    H    |    I
                     |         |
            _________|_________|_________
                edge4|         |edge3
                     |         |
                D    |    E    |    F
                     |         |
            _________|_________|_________
                edge1|         |edge2
                     |         |
                A    |    B    |    C
                     |         |
                     |         |

RETURN VALUE
   value depending on mode, -999.0 is error
-------------------------------------------------------------------------+*/
float angle_limits( int mode, int I1Dim_1, int I1Dim_2,
                    float Off_11, float Ps_11, float Off_21, float Ps_21,
                    float Wcenter_1, float Wcenter_2 )
{
  float i10, i11, i20, i21;
  float edge1[2], edge2[2], edge3[2], edge4[2];
  float dist1, dist2, dist3, dist4;
  float min_radius, max_radius, min_angle, max_angle;
  float value;

  i10 = i20 = A2INDEX(ARRAYSTART+LOWERBORDER);
  i11 = A2INDEX(ARRAYSTART+LOWERBORDER+I1Dim_1);
  i21 = A2INDEX(ARRAYSTART+LOWERBORDER+I1Dim_2);

  edge1[0] = WORLD(i10,Off_11,Ps_11)-Wcenter_1;
  edge1[1] = WORLD(i20,Off_21,Ps_21)-Wcenter_2;

  edge2[0] = WORLD(i11,Off_11,Ps_11)-Wcenter_1;
  edge2[1] = edge1[1];

  edge3[0] = edge2[0];
  edge3[1] = WORLD(i21,Off_21,Ps_21)-Wcenter_2;

  edge4[0] = edge1[0];
  edge4[1] = edge3[1];

  dist1 = CALC_DIST(edge1);
  dist2 = CALC_DIST(edge2);
  dist3 = CALC_DIST(edge3);
  dist4 = CALC_DIST(edge4);

  if ( 0.0 < edge1[0] ) {
    // A, D, G
    if ( 0.0 < edge1[1] ) {
      // A
      min_radius = dist1;
      max_radius = dist3;
      min_angle = atan2(edge2[1],edge2[0]);
      max_angle = atan2(edge4[1],edge4[0]);
    } else {
      if ( 0.0 < edge4[1] ) {
        // D
        min_radius = edge1[0];
        max_radius = MAX2(dist2,dist3);
        min_angle = atan2(edge1[1],edge1[0]);
        max_angle = atan2(edge4[1],edge4[0]);
      } else {
        // G
        min_radius = dist4;
        max_radius = dist2;
        min_angle = atan2(edge1[1],edge1[0]);
        max_angle = atan2(edge3[1],edge3[0]);
      }
    }
  } else {
    if ( 0.0 < edge2[0] ) {
      // B, E, H
      if ( 0.0 < edge2[1] ) {
        // B
        min_radius = edge1[1];
        max_radius = MAX2(dist4,dist3);
        min_angle = atan2(edge2[1],edge2[0]);
        max_angle = atan2(edge1[1],edge1[0]);
      } else {
        if ( 0.0 < edge3[1] ) {
          // E
          min_radius = 0.0;
          max_radius = MAX4(dist1, dist2, dist3, dist4);
          min_angle = 0.0;
          max_angle = twopi;
        } else {
          // H
          min_radius = -edge3[1];
          max_radius = MAX2(dist1,dist2);
          min_angle = atan2(edge4[1],edge4[0]);
          max_angle = atan2(edge3[1],edge3[0]);
        }
      }
    } else {
      // C, F, I
      if ( 0.0 < edge2[1] ) {
        // C
        min_radius = dist2;
        max_radius = dist4;
        min_angle = atan2(edge3[1],edge3[0]);
        max_angle = atan2(edge1[1],edge1[0]);
      } else {
        if ( 0.0 < edge3[1] ) {
          // F
          min_radius = -edge2[0];
          max_radius = MAX2(dist1,dist4);
          min_angle = atan2(edge3[1],edge3[0]);
          max_angle = atan2(edge2[1],edge2[0]);
        } else {
          // I
          min_radius = dist3;
          max_radius = dist1;
          min_angle = atan2(edge4[1],edge4[0]);
          max_angle = atan2(edge2[1],edge2[0]);
        }
      }
    }
  }

  // output range of min_angle [0..2*pi]
  if (min_angle< 0.0) min_angle += twopi;
  if (max_angle<=0.0) max_angle += twopi;

  switch (mode ) {
    case 1: // minimum radius
      value = min_radius;
      break;
    case 2: // maximum radius 
      value = max_radius;
      break;
    case -1: // minimum angle
      value = min_angle;
      break;
    case -2: // maximum angle
      value = max_angle;
      break;
    default : // error
      value = -999.0;
  }

  return ( value );

} // angle_limits

/*+++------------------------------------------------------------------------
NAME

  angle_sum --- azimuthal summation/averaging of an image

SYNOPSIS

  void angle_sum ( float * I0Data, float * E0Data,
                   int I0Dim_1, int I0Dim_2,
                   float Off_10, float Ps_10, float Off_20, float Ps_20,
                   float I0Dummy, float I0DDummy,
                   float * I1Data, float * E1Data,
                   int I1Dim_1, int I1Dim_2,
                   float Off_11, float Ps_11, float Off_21, float Ps_21,
                   float I1Dummy, float I1DDummy,
                   float Wcenter_1, float Wcenter_2,
                   int vsum, int ave, int testbit, int * pstatus );

DESCRIPTION
  Azimuthal summation/averaging of an image.
  Before using this function the output array must have been preset with
  I0Dummy.

NO RETURN VALUE
-------------------------------------------------------------------------+*/
void angle_sum ( float * I0Data, float * E0Data, 
                 int I0Dim_1, int I0Dim_2, 
                 float Off_10, float Ps_10, float Off_20, float Ps_20, 
                 float I0Dummy, float I0DDummy,
                 float * I1Data, float * E1Data,
                 int I1Dim_1, int I1Dim_2,
                 float Off_11, float Ps_11, float Off_21, float Ps_21, 
                 float I1Dummy, float I1DDummy,
                 float Wcenter_1, float Wcenter_2,
                 int vsum, int ave, int testbit, int * pstatus )
{

  float *pI0Data, *pE0Data;
  float I1Value, I1Sum, I1Weight, I1CircleSum, I1CircleSumWeight;
  float E1Value, E1Sum, E1Weight, E1CircleSum, E1CircleSumWeight;

  int i_1, i_2;
  float W_1, W_2;
  float f_11, f_21;

  float DDAngle, DAngle, Angle, Radius;
  int NAngle, iangle;

  float MinRadius, MaxRadius, MinAngle, MaxAngle;
  float NormAngle;

  float VarDDummy=DDSET(VarDummy);

  float factor;

  int cnt, varcnt;

  if (pstatus) *pstatus = 0;

  /* loop over I0Data */
  /*
   * Detailed descriptoin by R. Wilcke
   * Calculate the number of input image pixels that will have to be averaged
   * over for one output image pixel.
   *
   * Ps_11 and Ps_21 are the input image pixel sizes in x and y direction;
   * Ps_10 is the output image pixel size in radial direction, it is set to
   *         be the smaller one of the input image sizes in x and y direction;
   * Ps_20 is the output image pixel size in angular direction, this is an
   *         input parameter of the routine.
   *
   * The size of an "unit" pixel is therefore Ps_11 * Ps_21 (= dx * dy) in
   * the input image and Ps_10 * Ps_20 (= dr * da) in the output image.
   *
   * However, dx * dy is the same area over the whole image, whereas dr * da is
   * smaller for small values of r and bigger for big values of r.
   *
   * The area corresponding to dr * da at a given value of r is F = r * dr *
   * da.
   * Therefore, the number of input image pixels that will fit into an "unit"
   * output pixel is
   *    N = F / (dx * dy), or because of the definition of dr
   *    N = r * da / MAX(dx,dy)
   *
   * This is therefore the number of input image pixels that will have to be
   * averaged over for one output pixel. The actual value used (variable NAngle
   * further below) is an overestimate of this, to make sure that all possible
   * input pixels are actually taken into account.
   *
   * DDAngle is an intermediate variable, it corresponds to the number of input
   * image pixels at r = 1. It will be used to calculate NAngle later.
   */

  DDAngle = Ps_20/(MIN2(Ps_11,Ps_21));

  if (testbit) printf("DDAngle   = % g_deg\n", DDAngle * rad2deg);

  /*
   * Calculate the minimum and maximum radius and angle 
   * values for the input image.
   */

  MinRadius =  angle_limits(  1, I1Dim_1, I1Dim_2,
                    Off_11, Ps_11, Off_21, Ps_21, Wcenter_1, Wcenter_2 );
  MaxRadius =  angle_limits(  2, I1Dim_1, I1Dim_2,
                    Off_11, Ps_11, Off_21, Ps_21, Wcenter_1, Wcenter_2 );
  MinAngle  =  angle_limits( -1, I1Dim_1, I1Dim_2,
                    Off_11, Ps_11, Off_21, Ps_21, Wcenter_1, Wcenter_2 );
  MaxAngle  =  angle_limits( -2, I1Dim_1, I1Dim_2,
                    Off_11, Ps_11, Off_21, Ps_21, Wcenter_1, Wcenter_2 );

  if (testbit)
    printf("MinRadius=%g, MaxRadius=%g, MinAngle=%g_deg, MaxAngle=%g_deg\n",
            MinRadius, MaxRadius, MinAngle*rad2deg, MaxAngle*rad2deg );

  /*
   * Loop over the output array.
   *
   * The outer loop goes over the radius, the inner one over the angle.
   * Radial values outside the requested range are skipped.
   *
   * Attention:
   * - all values of the output array must have been preset with the value 
   *   "I0Dummy"
   */

  for (i_1=0;i_1<I0Dim_1;i_1++) {
    Radius = WORLD(i_1,Off_10,Ps_10);

    if ( Radius > MaxRadius || Radius < MinRadius )
      continue;

    /* number of angular intervals for averaging */
    NAngle  = MAX2(1,(int) (DDAngle * Radius) + 1 ); 
    DAngle  = Ps_20/(float) NAngle;

    if (testbit>1)
      printf("%d: Radius=%g, NAngle=%d, DAngle=%g_deg\n",
        i_1,Radius,NAngle,DAngle*rad2deg);

    for (i_2=0;i_2<I0Dim_2;i_2++) {

      Angle = WORLD((LOWERBORDER+(float)i_2),Off_20,Ps_20) + DAngle*0.5;

      if ( Angle > MaxAngle || Angle < MinAngle ) {
        /* MinAngle and MaxAngle are normalized angles between [0..2*pi], 
           check also the normalized angle */
        NormAngle = Angle-floor(Angle/twopi)*twopi;
        if ( MinAngle <= MaxAngle ) {
          // check, whether the angle is outside [MinAngle..MaxAngle]
          if ( NormAngle > MaxAngle || NormAngle < MinAngle ) continue;
        } else {
          // check, whether the angle is inside [MaxAngle..MinAngle] 
          if ( NormAngle < MaxAngle && NormAngle > MinAngle ) continue;
        }
      }
       
      /* angular averaging */ 
      cnt = 0;
      varcnt = 0;
      I1CircleSum = 0.0; I1CircleSumWeight = 0.0;
      E1CircleSum = 0.0; E1CircleSumWeight = 0.0;
      for (iangle = 0; iangle<NAngle; iangle++) {

        W_1 = Radius * cos( Angle  ) + Wcenter_1;  
        W_2 = Radius * sin( Angle  ) + Wcenter_2;

        f_11 = INDEX(W_1,Off_11,Ps_11);
        f_21 = INDEX(W_2,Off_21,Ps_21);

        if ( E0Data ) {
          // V0 = V1
          if ( Isum2ldwE(I1Data,E1Data,I1Dim_1,I1Dim_2,I1Dummy,I1DDummy,
                 f_11-0.5, f_21-0.5, f_11+0.5, f_21+0.5,
                 &I1Sum, &I1Weight, &E1Sum, &E1Weight) ) {
            /* then do something with the data */
            I1CircleSum       += I1Sum;
            I1CircleSumWeight += I1Weight; 

            if ( E1Sum >= 0.0 ) {
              E1CircleSum       += E1Sum;
              E1CircleSumWeight += E1Weight; 
              varcnt++;
            }
            cnt++;
          } /* if Isum2ldwE ... */
        } else {
          if (Ipol2ldw (I1Data,I1Dim_1,I1Dim_2,I1Dummy,I1DDummy,
            f_11, f_21, &I1Sum, &I1Weight)) {
            /* then do something with the data */
            I1CircleSum       += I1Sum;
            I1CircleSumWeight += I1Weight;
            cnt++;
          } /* if Ipol2ld ... */
        }
        Angle += DAngle;
      } /* for */

      if (cnt>0) {
        pI0Data = ABSPTR(I0Data,I0Dim_1,I0Dim_2,i_1,i_2);
        pE0Data = E0Data-I0Data+pI0Data; 

        /* The following factor adjusts the size of a rectangular pixel with 
           the size Ps_11*Ps_21 to a circular pixel with height Ps_10
           and width Radius*DAngle */
        factor = (Radius*DAngle*Ps_10)/(Ps_11*Ps_21);

        I1CircleSum *= factor; I1CircleSumWeight *= factor;
        E1CircleSum *= factor; E1CircleSumWeight *= factor;

        I1Value = I1CircleSum; if (ave) I1Value /= I1CircleSumWeight;

        if ( E0Data && ( varcnt==cnt ) ) {
          E1Value = E1CircleSum; 
          if (ave) E1Value /= E1CircleSumWeight*E1CircleSumWeight;
          /* Take into account that the data was averaged in a sector */
        } else E1Value = -1.0;

        if (vsum) {
          /* Multiply with number of covered pixels */
          factor = I1CircleSumWeight;
          I1Value *= I1CircleSumWeight;
          if ( E1Value>=0 ) E1Value *= I1CircleSumWeight*I1CircleSumWeight;
        }

        UPDATE( *pI0Data, I1Value, I0Dummy, I0DDummy );

        if ( E0Data && ( E1Value>=0.0 ) ) {
          UPDATE( *pE0Data, E1Value, VarDummy, VarDDummy );
        }
      }

      /* end angular averaging */

    } /* for i_2 ... */

  } /* for i_1 ... */

} /* angle_sum */

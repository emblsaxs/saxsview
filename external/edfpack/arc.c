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

# define ARC_VERSION      "arc : V1.9 Peter Boesecke 2011-06-28"
/*+++------------------------------------------------------------------------
NAME

   arc --- routines for arc and ang averaging

SYNOPSIS

   # include arc.h

HISTORY
  2005-10-28 V1.0  Peter Boesecke
  2007-04-19 V1.2  PB -Wall compiler warnings resolved
  2009-10-02 V1.3  PB arc_lfactor moved to SaxsRoutines,
                      Success -> 0, SaxsArc -> arc
                      include only ipol.h and reference.h
  2010-03-05 V1.4  PB Problems averaging in a single step:
                      ang_sum, arc_sum: limits, steps etc. improved,
                      angular range limited to 360 degrees,
                      ang_sum: averaging in a single step possible
  2010-03-19 V1.5  PB ang_limits parameter changed to use waxs_Range.
                      ang_range, waxs included
  2010-03-20 V1.6  PB ang_limits: condition for min_angle corrected
  2010-05-31 V1.7  PB update for waxs.c V1.10
  2010-06-02 V1.8  PB maximum regrouping range 360_deg, splitted
                      in 3 ranges 
  2011-06-28 V1.9  PB waxs_get_transform and waxs_Transform updated

----------------------------------------------------------------------------*/
/******************************************************************************
* Include Files                                                               *
******************************************************************************/

# include "arc.h" 

/******************************************************************************
* Private Constants                                                           *
******************************************************************************/

static const double arc_rad2deg = 180.0/NUM_PI;
static const double arc_radius_eps = 1e-32;
static const double arc_angle_eps = 1e-32;
static const float  arc_twopi = 2.0*NUM_PI;

/******************************************************************************
* Private Definitions                                                         *
******************************************************************************/

# define CALC_DIST(A) sqrt((A[0])*(A[0])+(A[1])*(A[1]))

/******************************************************************************
* Routines                                                                    *
******************************************************************************/
/*+++------------------------------------------------------------------------
NAME

  ang_range

SYNOPSIS

  int ang_range( int rsys, int proin, int proout,
                 long  dim_1, long dim_2,
                 float off_1, float pix_1, float cen_1,
                 float off_2, float pix_2, float cen_2,
                 float dis, float wvl,
                 float detrot1, float detrot2, float detrot3,
                 WaxsCoord *Wmin, WaxsCoord *Wmax, int * pstatus);

DESCRIPTION
  Returns the maximum and minimum world coordinates of the cartesian
  input image in referecne system rsys. A transformation from
  Saxs to Waxs or vice versa is only done if the reference system
  rsys is IO_Saxs. In all other cases proin and proout are ignored.

  Wmin.s_1 : Wmin_1
  Wmin.s_2 : Wmin_2
  Wmax.s_1 : Wmax_1
  Wmax.s_2 : Wmax_2

RETURN VALUE
  -1: inverse projection transformation (WAXS->SAXS)
   0: no transformation
   1: normal projection transformation (SAXS->WAXS)
-------------------------------------------------------------------------+*/
int ang_range( int rsys, int proin, int proout,
               long  dim_1, long dim_2,
               float off_1, float pix_1, float cen_1,
               float off_2, float pix_2, float cen_2,
               float dis, float wvl,
               float detrot1, float detrot2, float detrot3,
               WaxsCoord *Wmin, WaxsCoord *Wmax, int * pstatus)
{                  
  float Off_11, Off_21;
  float Ps_11, Ps_21;
  float f10, f20, f11, f21;

  double K=1.0, rot1=0.0, rot2=0.0, rot3=0.0;
  WParams I1params, I0params;

  int transform=0; // no transformation

  int status=0;

  if (pstatus) *pstatus = status;

  /* Use waxs_Transform only if reference system is Saxs and if either the
    input or the output image is in Saxs projection, but not if both images 
    are in Saxs projection and the rotations are Zero. */
  if ( (rsys==IO_Saxs)&&((proin==IO_ProSaxs)||(proout==IO_ProSaxs)) 
     &&(!((proin==proout)&&(detrot1==0.0)&&(detrot2==0.0)&&(detrot3==0.0))) ) {

    K    = (double) WAVENUMBER(wvl);
    rot1 = (double) detrot1;
    rot2 = (double) detrot2;
    rot3 = (double) detrot3;

    waxs_Init ( &I1params, K, rot1, rot2, rot3 );
    waxs_Init ( &I0params, K, 0.0, 0.0, 0.0 );

    /* Get coordinate range */
    transform=waxs_Range( &I1params, &I0params, 
                          proin, proout,
                          dim_1, dim_2,
                          off_1, pix_1, cen_1,
                          off_2, pix_2, cen_2,
                          dis, wvl,
                          Wmin, Wmax, &status);
    if (status) goto ang_range_error;

  } else {

    /* Get world coordinate range of input image */
    switch (rsys) {
       case IO_Saxs:
         // SAXSREF(Off,Ps,O,P,C,S,W)
         SAXSREF(Off_11,Ps_11,off_1,pix_1,cen_1,dis,wvl);
         SAXSREF(Off_21,Ps_21,off_2,pix_2,cen_2,dis,wvl);
         break;
       case IO_Normal:
         NORMALREF(Off_11,Ps_11,off_1,pix_1,cen_1);
         NORMALREF(Off_21,Ps_21,off_2,pix_2,cen_2);
         break;
       default:
         status=-1;
         goto ang_range_error;
    }

    f10 = f20 = A2INDEX(ARRAYSTART+LOWERBORDER);
    f11 = A2INDEX(ARRAYSTART+LOWERBORDER+dim_1);
    f21 = A2INDEX(ARRAYSTART+LOWERBORDER+dim_2);

    if (Wmin) {
      Wmin->s_1 = WORLD(f10,Off_11,Ps_11); // W1min_1
      Wmin->s_2 = WORLD(f20,Off_21,Ps_21); // W1min_2
    }
    if (Wmax) {
      Wmax->s_1 = WORLD(f11,Off_11,Ps_11); // W1max_1
      Wmax->s_2 = WORLD(f21,Off_21,Ps_21); // W1max_2
    }
  }

  return ( transform );

ang_range_error:

  if (pstatus) *pstatus = status;
  return ( transform );

} // ang_range

/*+++------------------------------------------------------------------------
NAME
  
   ang_limits 

SYNOPSIS
  
  void ang_limits( WaxsCoord Wmin, WaxsCoord Wmax, 
                   WaxsCoord *Amin, WaxsCoord *Amax,
                   int * pstatus);

DESCRIPTION
  Returns the maximum and minimum distances of the output image.
  
  The returned distances are absolute values in world distances,
  the angles are returned in radian.
  
  Amin.s_1 : minimum radius
  Amax.s_1 : maximum radius
  Amin.s_2 : minimum angle
  Amax.s_2 : maximum angle

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
  void
-------------------------------------------------------------------------+*/
void ang_limits( WaxsCoord Wmin, WaxsCoord Wmax, 
                 WaxsCoord *Amin, WaxsCoord *Amax,
                 int * pstatus)
{
  float edge1[2], edge2[2], edge3[2], edge4[2];
  float dist1, dist2, dist3, dist4;
  float min_radius, max_radius, min_angle, max_angle;

  if (*pstatus) pstatus=0;

  edge1[0] = Wmin.s_1; // WORLD(i10,Off_11,Ps_11);
  edge1[1] = Wmin.s_2; // WORLD(i20,Off_21,Ps_21);

  edge2[0] = Wmax.s_1; // WORLD(i11,Off_11,Ps_11);
  edge2[1] = Wmin.s_2; // edge1[1];

  edge3[0] = Wmax.s_1; // edge2[0];
  edge3[1] = Wmax.s_2; // WORLD(i21,Off_21,Ps_21);

  edge4[0] = Wmin.s_1; // edge1[0];
  edge4[1] = Wmax.s_2; // edge3[1];

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
          max_angle = arc_twopi;
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

  // output range of angles is [0..2*pi]
  if ( max_angle<min_angle ) max_angle += arc_twopi;
  if ( (min_angle<-arc_angle_eps)||(max_angle<=0.0) ) {
    min_angle += arc_twopi; max_angle += arc_twopi; 
  }

  if (Amin) {
    Amin->s_1 = min_radius; // minimum radius
    Amin->s_2 = min_angle;  // minimum angle
  }
  if (Amax) {
    Amax->s_1 = max_radius; // maximum radius
    Amax->s_2 = max_angle;  // maximum angle
  }

  return;

} // ang_limits

/*---------------------------------------------------------------------------
NAME
 
  arc_sum --- Regrouping of an image from cartesian to radius/arc
  

SYNOPSIS

  void arc_sum ( int rsys, 
                 float * I0Data,   float * E0Data,           
                 int   I0Dim_1,    int   I0Dim_2, 
                 float I0Offset_1, float I0PSize_1, float I0Center_1,
                 float I0Offset_2, float I0PSize_2, float I0Center_2,
                 float I0SampleDistance, float I0WaveLength,
                 int I0Pro,
                 float I0Dummy,    float I0DDummy,
                 float * I1Data,   float * E1Data,
                 int   I1Dim_1,    int   I1Dim_2,
                 float I1Offset_1, float I1PSize_1, float I1Center_1,
                 float I1Offset_2, float I1PSize_2, float I1Center_2,
                 float I1SampleDistance, float I1WaveLength,
                 float I1DetRot1,  float I1DetRot2, float I1DetRot3,
                 int I1Pro,
                 float I1Dummy,    float I1DDummy,
                 float AngleMin,   float AngleMax,
                 float Shift_1,    float Shift_2,
                 int vsum, int ave, int testbit, int * pstatus );


PURPOSE

  Regrouping of an image radius/arc
  
DESCRIPTION

  The routine regroupes an input image with cartesian coordinates w_1 and w_2
  to an image with coordinates radius and arc. The input image is described 
  with Normal (Offset, Center, PSize) or Saxs coordinates (Offset, Center, 
  PSize, SampleDistance, WaveLength). Axis 1 of the regrouped image is the 
  radius, axis 2 the arc. The reference system of output axis 2 is always Normal.
  The regrouping is done in the sector between AngleMin and AngleMax.  If 
  AngleMin >= AngleMax nothing is done. AngleMax is limited to AngleMin+2*pi. 
  The part of the output image outside this range is left unchanged.

  (w_1, w_2) -> (radius,arc)

  with radius = sqrt(w_1^2+w_2^2), arc = radius*Angle and
  cos(Angle)=w_1/radius, sin(Angle)=w_2/radius.

ARGUMENTS
     int rsys      (i)       : reference system
  float * I0Data   (o)       : output image data
  float * E0Data   (o)       : output variance data (ignored if NULL)
  int   I0Dim_1,   (i)       : 
        I0Dim_2    (i)       : dimensions of output data arrays
  float I0Offset_1,
        I0PSize_1,
        I0Center_1,
        I0Offset_2,
        I0Center_2,
        I0PSize_2  (i) : output image parameters
        I0SampleDistance (i) :
        I0WaveLength (i)     :
        I0Pro ()             : projection type of input image
  float I0Dummy,
        I0DDummy   (i) : output image dummy definition
  float * I1Data   (i) : input image data
  float * E1Data   (i) : input variance data (ignored if NULL)
  int   I1Dim_1,   
        I1Dim_2    (i) : dimensions of input data arrays
  float I1Offset_1,
        I1PSize_1,
        I1Center_1,
        I1Offset_2,
        I1PSize_2,
        I1Center_2 (i) : input image parameters
        I1SampleDistance (i) :
        I1WaveLength (i)     :
        I1DetRot1, 
        I1DetRot1,
        I1DetRot1 (i) : detector rotations (in radian)
        I1Pro ()             : projection type of output image
  float AngleMin,
        AngleMax   (i) : angular range (in rad)
  float Shift_1,
        Shift_2    (i) : shift of the output image along axis 1 and 2.
                         The shift is usually 0.
  int   vsum       (i) : if 1, integrate output values
  int   ave        (i) : if 1, average input values
  int   testbit    (i) : if 1, write debug info
  int   *pstatus   (o) : returned status value

---------------------------------------------------------------------------*/
void arc_sum ( int rsys, 
               float * I0Data,   float * E0Data, 
               int   I0Dim_1,    int   I0Dim_2, 
               float I0Offset_1, float I0PSize_1, float I0Center_1,
               float I0Offset_2, float I0PSize_2, float I0Center_2,
               float I0SampleDistance, float I0WaveLength,
               int I0Pro,
               float I0Dummy,    float I0DDummy,
               float * I1Data,   float * E1Data,
               int   I1Dim_1,    int   I1Dim_2,
               float I1Offset_1, float I1PSize_1, float I1Center_1,
               float I1Offset_2, float I1PSize_2, float I1Center_2,
               float I1SampleDistance, float I1WaveLength,
               float I1DetRot1,  float I1DetRot2, float I1DetRot3,
               int I1Pro, 
               float I1Dummy,    float I1DDummy,
               float AngleMin,   float AngleMax,
               float Shift_1,    float Shift_2,
               int vsum, int ave, int testbit, int * pstatus )
{
  float *pI0Data, *pE0Data;

  float Off_10, Ps_10, Off_20, Ps_20;
  float Off_11, Ps_11, Off_21, Ps_21;

  float I1Value, I1Sum, I1Weight, I1ArcSum, I1ArcSumWeight;
  float E1Value, E1Sum, E1Weight, E1ArcSum, E1ArcSumWeight;

  double K=1.0, rot1=0.0, rot2=0.0, rot3=0.0;
  WParams I1params, I0params;

  float Angle, DAngle, AngleLower, AngleUpper;
  float MinArc, MaxArc;
  float AngleFst, AngleLst;

  float MinRadius, MaxRadius, MinAngle, MaxAngle;

  float W0_2Fst, W0_2Lst;
  float W0_2Min, W0_2Max;

  WaxsCoord W1min, W1max, A1min, A1max;
  WaxsCoord W0, W1;

  int transform=0;

  int i_1, i_2;
  int i_20, i_22;
  float f_11, f_21;

  float DDArc, DArc, Arc, Radius;
  int NArc, iarc;

  float VarDDummy=DDSET(VarDummy);

  float factor;

  int cnt, varcnt;

  int status=0;

  if (testbit) printf("arc_sum: AngleMin=% g_deg, AngleMax=% g_deg\n",
      AngleMin*arc_rad2deg,AngleMax*arc_rad2deg);

  /* Calculate pixel transformations */ 
  switch (rsys) {
    case IO_Normal:
      if (testbit) printf("arc_sum: The refrence system is NORMAL\n");
      NORMALREF(Off_11,Ps_11,I1Offset_1,I1PSize_1,I1Center_1);
      NORMALREF(Off_21,Ps_21,I1Offset_2,I1PSize_2,I1Center_2);
      NORMALREF(Off_10,Ps_10,I0Offset_1,I0PSize_1,I0Center_1);
      NORMALREF(Off_20,Ps_20,I0Offset_2,I0PSize_2,I0Center_2);
      break;
    case IO_Saxs:
      if (testbit) printf("arc_sum: The refrence system is SAXS\n");
      SAXSREF(Off_11,Ps_11,I1Offset_1,I1PSize_1,I1Center_1,I1SampleDistance,I1WaveLength); 
      SAXSREF(Off_21,Ps_21,I1Offset_2,I1PSize_2,I1Center_2,I1SampleDistance,I1WaveLength); 
      SAXSREF(Off_10,Ps_10,I0Offset_1,I0PSize_1,I0Center_1,I0SampleDistance,I0WaveLength); 
      SAXSREF(Off_20,Ps_20,I0Offset_2,I0PSize_2,I0Center_2,I0SampleDistance,I0WaveLength); 
      break;
    default: 
      fprintf(stderr,"ERROR: The refrence system is neither NORMAL nor SAXS (%d)\n",rsys);
      status=-1;
      goto arc_sum_error;
  }

  /* Subtract output shift for calculation */
  Off_10 = Off_10-Shift_1; Off_20 = Off_20-Shift_2;

  if (testbit) {
    printf("arc_sum: Off_10 = % f, Ps_10 = % f\n", Off_10,Ps_10);
    printf("arc_sum: Off_20 = % f, Ps_20 = % f\n", Off_20,Ps_20);
  }
  
  /* loop over I0Data */
  /*
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
   * DDArc is an intermediate variable, it corresponds to the number of input
   * image pixels at r = 1. It will be used to calculate NArc later.
   */

  DDArc = Ps_20/(MIN2(Ps_11,Ps_21));
  if (testbit) printf("arc_sum: DDArc   = % g_deg\n", DDArc * arc_rad2deg);

  /* 
   * Calculate the minimum and maximum arc values for the input image.
   * The dimensions of the output image should have been adjusted with
   * ang_limits
   */

  transform = ang_range( rsys, I1Pro, I0Pro,
                         I1Dim_1, I1Dim_2,
                         I1Offset_1,I1PSize_1,I1Center_1,
                         I1Offset_2,I1PSize_2,I1Center_2,
                         I1SampleDistance, I1WaveLength,
                         I1DetRot1, I1DetRot2, I1DetRot3,
                         &W1min, &W1max, &status);
  if (status) goto arc_sum_error;

  if (testbit) {
    printf("arc_sum: W1min_1=%g, W1max_1=%g\n", W1min.s_1,W1max.s_1);
    printf("arc_sum: W1min_2=%g, W1max_2=%g\n", W1min.s_2,W1max.s_2);
    printf("arc_sum: transform=%d\n",transform);
  }

  ang_limits( W1min, W1max, &A1min, &A1max, &status);
  if (status) goto arc_sum_error;

  if (testbit) {
    printf("arc_sum: A1min.s_1=%lg    , A1max.s_1=%lg\n",
      A1min.s_1,A1max.s_1);
    printf("arc_sum: A1min.s_2=%lg_deg, A1max.s_2=%lg_deg\n",
      A1min.s_2*arc_rad2deg,A1max.s_2*arc_rad2deg);
  }

  MinRadius =  A1min.s_1;
  MaxRadius =  A1max.s_1;
  MinAngle  =  A1min.s_2 + floor(AngleMin/arc_twopi)*arc_twopi;
  MaxAngle  =  A1max.s_2 + floor(AngleMin/arc_twopi)*arc_twopi;

  // Calculate coordinate range of Arc in output image
  W0_2Fst = WORLD(INDEXSTART+LOWERBORDER,Off_20,Ps_20); 
  W0_2Lst = WORLD(INDEXSTART+LOWERBORDER+I0Dim_2,Off_20,Ps_20); 

  if (W0_2Fst <= W0_2Lst) { 
    W0_2Min = W0_2Fst; W0_2Max = W0_2Lst; 
  } else {  
    W0_2Min = W0_2Lst; W0_2Max = W0_2Fst; 
  } 

  /* maximum range is AngleMin+2_pi*/
  AngleFst = MAX2(AngleMin,MinAngle);
  AngleLst = MIN2(AngleMax,MaxAngle);

  if (testbit) {
    printf("arc_sum: W0_2Min=%g_deg, W0_2Max=%g_deg\n",W0_2Min*arc_rad2deg,W0_2Max*arc_rad2deg);
    printf("arc_sum: MinRadius=%g, MaxRadius=%g, MinAngle=%g_deg, MaxAngle=%g_deg\n",
            MinRadius, MaxRadius, MinAngle*arc_rad2deg, MaxAngle*arc_rad2deg );
    printf("AngleFst=%g_deg, AngleLst=%g_deg\n",
            AngleFst*arc_rad2deg, AngleLst*arc_rad2deg );
  }

  K    = (double) WAVENUMBER(I1WaveLength);
  rot1 = (double) I1DetRot1;
  rot2 = (double) I1DetRot2;
  rot3 = (double) I1DetRot3; 

  waxs_Init ( &I1params, K, rot1, rot2, rot3 );
  waxs_Init ( &I0params, K, 0.0, 0.0, 0.0 );

  if (testbit) {
     printf("I0params\n");
     waxs_PrintParams ( stdout, I0params );
     printf("I1params\n");
     waxs_PrintParams ( stdout, I1params );
  }

  if (AngleFst < AngleLst) {
    /* MinArc and MaxArc depend on the actual radius, i.e. on i_1 */
    for (i_1=0;i_1<I0Dim_1;i_1++) {
      Radius = WORLD(i_1,Off_10,Ps_10);

      if ( Radius > MaxRadius || Radius < MinRadius ) continue;

      if (testbit) printf("arc_sum: AngleFst=%g_deg, AngleLst=%g_deg\n",
                AngleFst*arc_rad2deg, AngleLst*arc_rad2deg );

      if (Radius >= 0 ) {
        MinArc = MAX2(Radius*AngleFst,W0_2Min);
        MaxArc = MIN2(Radius*AngleLst,W0_2Max);
      } else {
        MinArc = MAX2(Radius*AngleFst,W0_2Min);
        MaxArc = MIN2(Radius*AngleLst,W0_2Max);
      }
 
      i_20=MAX2(0,floor(INDEX(MinArc,Off_20,Ps_20)-LOWERBORDER)); // first pixel in range
      i_22=MIN2(I0Dim_2,ceil(INDEX(MaxArc,Off_20,Ps_20)-LOWERBORDER)); // first pixel after range

      if (testbit>1) {
        printf("arc_sum: MinArc=%g, MaxArc=%g\n",MinArc*arc_rad2deg,MaxArc*arc_rad2deg);
        printf("arc_sum: i_20=%d, i_22=%d\n",i_20,i_22);
      }

      /* number of intervals on the arc for averaging */
      NArc  = MAX2(1,(int) DDArc + 1 ); 
      DArc  = Ps_20/(float) NArc;

      if (testbit>1)
        printf("arc_sum: %d: Radius=%g, NArc=%d, DArc=%g\n",
          i_1,Radius,NArc,DArc * arc_rad2deg);

      for (i_2=i_20;i_2<i_22;i_2++) {

        // Calculate center of first pixel
        Arc = WORLD(((float)i_2),Off_20,Ps_20);

        /* averaging on the arc */ 
        if ( Radius >= arc_radius_eps ) {
          DAngle = DArc/Radius;

          AngleLower = WORLD(((float)i_2+LOWERBORDER),Off_20,Ps_20)/Radius; // lower limit
          AngleUpper = WORLD(((float)i_2+1+LOWERBORDER),Off_20,Ps_20)/Radius; // upper limit

          // restrict integration range to [AngleFst..AngleLst];
          AngleLower = MAX2(AngleFst,AngleLower);
          AngleUpper = MIN2(AngleLst,AngleUpper);
        } else {
          DAngle = 0.0;
          AngleLower = AngleFst;
          AngleUpper = AngleLst;
        }

        if (testbit>2) {
          printf("arc_sum: %d,%d: Arc=%g, MinArc=%g_deg, MaxArc=%g_deg\n",
            i_1,i_2,Arc * arc_rad2deg,MinArc * arc_rad2deg,MaxArc * arc_rad2deg);
          printf("arc_sum: %d,%d: AngleLower=%g_deg, AngleUpper=%g_deg\n",
            i_1,i_2,AngleLower*arc_rad2deg,AngleUpper*arc_rad2deg);
        }

        cnt = 0;
        varcnt = 0;
        I1ArcSum = 0.0; I1ArcSumWeight = 0.0;
        E1ArcSum = 0.0; E1ArcSumWeight = 0.0;
        for (iarc = 0; iarc<NArc; iarc++) {

          if ( Radius >= arc_radius_eps ) {
            Angle = Arc/Radius;
            W0.s_1 = Radius * cos( Angle  );  // W_1
            W0.s_2 = Radius * sin( Angle  );  // W_2
          } else {
            Angle = 0.0;
            W0.s_1 = 0.0; // W_1
            W0.s_2 = 0.0; // W_2
          }

          /* transform saxs-coordinate of unrotated detector (I0params) or Waxs-
             projection to saxs-coordinate of rotated detector (I1params) */
//++++++++          W1 = waxs_Transform( &I1params, &I0params, transform, W0 );
          W1 = waxs_Transform( &I0params, &I1params, transform, W0 );

          if (!W1.status) {

            /* averaging range is [AngleFst..AngleLst] */
            if ( ( Angle < AngleLower ) || ( AngleUpper < Angle ) ) { 
              if (testbit>3)
                printf("arc_sum: iarc=%d: Angle=%g_deg not in [%g_deg..%g_deg] => continue\n",
                  iarc,Angle*arc_rad2deg,AngleLower*arc_rad2deg,AngleUpper*arc_rad2deg);
              Arc += DArc;
              continue;
            }
            f_11 = INDEX(W1.s_1,Off_11,Ps_11);
            f_21 = INDEX(W1.s_2,Off_21,Ps_21);
            if ( E0Data ) {
              // V0 = V1
              if ( Isum2ldwE(I1Data,E1Data,I1Dim_1,I1Dim_2,I1Dummy,I1DDummy,
                     f_11-0.5, f_21-0.5, f_11+0.5, f_21+0.5,
                     &I1Sum, &I1Weight, &E1Sum, &E1Weight) ) {
                /* then do something with the data */
                I1ArcSum       += I1Sum;
                I1ArcSumWeight += I1Weight; 
  
                if ( E1Sum >= 0.0 ) {
                  E1ArcSum       += E1Sum;
                  E1ArcSumWeight += E1Weight; 
                  varcnt++;
                }
                cnt++;
              } /* if Isum2ldwE ... */
            } else {
              if ( Isum2ldw(I1Data,I1Dim_1,I1Dim_2,I1Dummy,I1DDummy,
                     f_11-0.5, f_21-0.5, f_11+0.5, f_21+0.5,
                     &I1Sum, &I1Weight) ) {
  
                /* then do something with the data */
                I1ArcSum       += I1Sum;
                I1ArcSumWeight += I1Weight;
                cnt++;
              } /* if Isum2ldw ... */
            }

          } // if (!W1.status)

          Arc += DArc;

        } /* for iarc */

        if (cnt>0) {
          pI0Data = ABSPTR(I0Data,I0Dim_1,I0Dim_2,i_1,i_2);
          pE0Data = E0Data-I0Data+pI0Data; 

          /* The following factor adjusts the size of a rectangular pixel with 
             the size Ps_11*Ps_21 to a circular pixel with height Ps_10 and 
             width DArc */
          factor = (DArc*Ps_10)/(Ps_11*Ps_21);

          I1ArcSum *= factor; I1ArcSumWeight *= factor;
          E1ArcSum *= factor; E1ArcSumWeight *= factor;

          I1Value = I1ArcSum; if (ave) I1Value /= I1ArcSumWeight;

          if ( E0Data && ( varcnt==cnt ) ) {
            E1Value = E1ArcSum; 
            if (ave) E1Value /= E1ArcSumWeight*E1ArcSumWeight;
            /* Take into account that the data was averaged in a sector */
          } else E1Value = -1.0;

          if (vsum) {
            /* Multiply with number of covered pixels */
            factor = I1ArcSumWeight;
            I1Value *= I1ArcSumWeight;
            if ( E1Value>=0 ) E1Value *= I1ArcSumWeight*I1ArcSumWeight;
          }

          UPDATE( *pI0Data, I1Value, I0Dummy, I0DDummy );

          if ( E0Data && ( E1Value>=0.0 ) ) {
            UPDATE( *pE0Data, E1Value, VarDummy, VarDDummy );
          }
        }

        /* end angular averaging */

      } /* for i_2 ... */

    } /* for i_1 ... */

  } /* if (AngleFst < AngleLst) */

  if (pstatus) *pstatus = status;

  return;

arc_sum_error:

  if (pstatus) *pstatus = status;

  return;

} /* arc_sum */

/*---------------------------------------------------------------------------
NAME

  ang_sum --- Regrouping of an image from cartesian to polar coordinates
 
PURPOSE

  Regrouping of an image radius/arc
 
DESCRIPTION

  The routine regroupes an input image with cartesian coordinates w_1 and w_2
  to an image with polar coordinates. The input image is described with Normal 
  coordinates (Offset, Center, PSize) or Saxs coordinates (Offset, Center, 
  PSize, SampleDistance, WaveLength). Axis 1 of the regrouped image is the 
  radius, axis 2 the angle. The reference system of output axis 2 is always Normal.
  The regrouping is done in the sector between AngleMin and AngleMax.  If 
  AngleMin >= AngleMax nothing is done. AngleMax is limited to AngleMin+2*pi. 
  The part of the output image outside this range is left unchanged.

  (w_1, w_2) -> (radius,angle)

  with radius = sqrt(w_1^2+w_2^2) and
  cos(Angle)=w_1/radius, sin(Angle)=w_2/radius.

SYNOPSIS

  void ang_sum ( int rsys, 
                 float * I0Data,   float * E0Data,           
                 int   I0Dim_1,    int   I0Dim_2, 
                 float I0Offset_1, float I0PSize_1, float I0Center_1,
                 float I0Offset_2, float I0PSize_2, float I0Center_2,
                 float I0SampleDistance, float I0WaveLength,
                 int I0Pro,
                 float I0Dummy,    float I0DDummy,
                 float * I1Data,   float * E1Data,
                 int   I1Dim_1,    int   I1Dim_2,
                 float I1Offset_1, float I1PSize_1, float I1Center_1,
                 float I1Offset_2, float I1PSize_2, float I1Center_2,
                 float I1SampleDistance, float I1WaveLength,
                 float I1DetRot1,  float I1DetRot2, float I1DetRot3,
                 int I1Pro,
                 float I1Dummy,    float I1DDummy,
                 float AngleMin,   float AngleMax,
                 float Shift_1,    float Shift_2,
                 int vsum, int ave, int testbit, int * pstatus )


ARGUMENTS
     int rsys      (i)       : reference system
  float * I0Data   (o)       : output image data
  float * E0Data   (o)       : output variance data (ignored if NULL)
  int   I0Dim_1,   (i)       : 
        I0Dim_2    (i)       : dimensions of output data arrays
  float I0Offset_1,
        I0PSize_1,
        I0Center_1,
        I0Offset_2,
        I0Center_2,
        I0PSize_2  (i) : output image parameters
        I0SampleDistance (i) :
        I0WaveLength (i)     :
        I0Pro ()             : projection type of input image
  float I0Dummy,
        I0DDummy   (i) : output image dummy definition
  float * I1Data   (i) : input image data
  float * E1Data   (i) : input variance data (ignored if NULL)
  int   I1Dim_1,   
        I1Dim_2    (i) : dimensions of input data arrays
  float I1Offset_1,
        I1PSize_1,
        I1Center_1,
        I1Offset_2,
        I1PSize_2,
        I1Center_2 (i) : input image parameters
        I1SampleDistance (i) :
        I1WaveLength (i)     :
        I1DetRot1, 
        I1DetRot1,
        I1DetRot1 (i) : detector rotations (in radian)
        I1Pro ()             : projection type of output image
  float AngleMin,
        AngleMax   (i) : angular range (in rad)
  float Shift_1,
        Shift_2    (i) : shift of the output image along axis 1 and 2.
                         The shift is usually 0.
  int   vsum       (i) : if 1, integrate output values
  int   ave        (i) : if 1, average input values
  int   testbit    (i) : if 1, write debug info
  int   *pstatus   (o) : returned status value

---------------------------------------------------------------------------*/
void ang_sum ( int rsys, 
               float * I0Data,   float * E0Data,           
               int   I0Dim_1,    int   I0Dim_2, 
               float I0Offset_1, float I0PSize_1, float I0Center_1,
               float I0Offset_2, float I0PSize_2, float I0Center_2,
               float I0SampleDistance, float I0WaveLength,
               int I0Pro,
               float I0Dummy,    float I0DDummy,
               float * I1Data,   float * E1Data,
               int   I1Dim_1,    int   I1Dim_2,
               float I1Offset_1, float I1PSize_1, float I1Center_1,
               float I1Offset_2, float I1PSize_2, float I1Center_2,
               float I1SampleDistance, float I1WaveLength,
               float I1DetRot1,  float I1DetRot2, float I1DetRot3,
               int I1Pro,
               float I1Dummy,    float I1DDummy,
               float AngleMin,   float AngleMax,
               float Shift_1,    float Shift_2,
               int vsum, int ave, int testbit, int * pstatus )
{
  float *pI0Data, *pE0Data;

  float Off_10, Ps_10, Off_20, Ps_20;
  float Off_11, Ps_11, Off_21, Ps_21;

  float I1Value, I1Sum, I1Weight, I1CircleSum, I1CircleSumWeight;
  float E1Value, E1Sum, E1Weight, E1CircleSum, E1CircleSumWeight;

  double K=1.0, rot1=0.0, rot2=0.0, rot3=0.0;
  WParams I1params, I0params;

  int i_1, i_2;
  int i_10, i_11, i_20, i_22;
  float f_11, f_21;

  float DDAngle, DAngle, Angle, Radius;
  float AngleLower, AngleUpper;
  float angle, AngleFst, AngleLst;
  int NAngle, iangle, ianglefst, ianglelst;

# define N_RANGES 3
  float Fst[N_RANGES], Lst[N_RANGES];
  int range;

  float MinRadius, MaxRadius, MinAngle, MaxAngle;

  WaxsCoord W1min, W1max, A1min, A1max;
  WaxsCoord W0, W1;

  int transform=0;

  float VarDDummy=DDSET(VarDummy);

  float factor;

  int cnt, varcnt;

  int status=0;

  /* restrict range to 2 pi */
  AngleMax = MIN2(AngleMax,AngleMin+arc_twopi);

  if (testbit) {
     printf("ang_sum: AngleMin=% g_deg, AngleMax=% g_deg\n",
       AngleMin*arc_rad2deg,AngleMax*arc_rad2deg);
  }

  /* Calculate pixel transformations */ 
  switch (rsys) {
    case IO_Normal:
      if (testbit) printf("ang_sum: The refrence system is NORMAL\n");
      NORMALREF(Off_11,Ps_11,I1Offset_1,I1PSize_1,I1Center_1);
      NORMALREF(Off_21,Ps_21,I1Offset_2,I1PSize_2,I1Center_2);
      NORMALREF(Off_10,Ps_10,I0Offset_1,I0PSize_1,I0Center_1);
      NORMALREF(Off_20,Ps_20,I0Offset_2,I0PSize_2,I0Center_2);
      break;
    case IO_Saxs:
      if (testbit) printf("ang_sum: The radial refrence system is SAXS, the angular NORMAL\n");
      SAXSREF(Off_11,Ps_11,I1Offset_1,I1PSize_1,I1Center_1,I1SampleDistance,I1WaveLength); 
      SAXSREF(Off_21,Ps_21,I1Offset_2,I1PSize_2,I1Center_2,I1SampleDistance,I1WaveLength); 
      SAXSREF(Off_10,Ps_10,I0Offset_1,I0PSize_1,I0Center_1,I0SampleDistance,I0WaveLength); 
      NORMALREF(Off_20,Ps_20,I0Offset_2,I0PSize_2,I0Center_2); // Angle reference is NORMAL
      break;
    default:
      fprintf(stderr,"ERROR: The refrence system is neither NORMAL nor SAXS (%d)\n",rsys);
      status=-1;
      goto ang_sum_error;
  }

  /* Subtract output shift for calculation */
  Off_10 = Off_10-Shift_1; Off_20 = Off_20-Shift_2;

  if (testbit) {
    printf("ang_sum: Off_10 = % f, Ps_10 = % f\n", Off_10,Ps_10);
    printf("ang_sum: Off_20 = % f, Ps_20 = % f\n", Off_20,Ps_20);
  }

  /* 
   * Calculate the minimum and maximum arc values for the input image.
   * The dimensions of the output image should have been adjusted with
   * ang_limits
   */

  transform = ang_range( rsys, I1Pro, I0Pro,
                         I1Dim_1, I1Dim_2,
                         I1Offset_1,I1PSize_1,I1Center_1,
                         I1Offset_2,I1PSize_2,I1Center_2,
                         I1SampleDistance, I1WaveLength,
                         I1DetRot1, I1DetRot2, I1DetRot3,
                         &W1min, &W1max, &status);
  if (status) goto ang_sum_error;

  if (testbit) {
    printf("ang_sum: W1min_1=%g, W1max_1=%g\n", W1min.s_1,W1max.s_1);
    printf("ang_sum: W1min_2=%g, W1max_2=%g\n", W1min.s_2,W1max.s_2);
    printf("ang_sum: transform=%d\n",transform);
  }

  ang_limits( W1min, W1max, &A1min, &A1max, &status);
  if (status) goto ang_sum_error;

  if (testbit) {
    printf("ang_sum: A1min.s_1=%lg    , A1max.s_1=%lg\n",
      A1min.s_1,A1max.s_1);
    printf("ang_sum: A1min.s_2=%lg_deg, A1max.s_2=%lg_deg\n",
      A1min.s_2*arc_rad2deg,A1max.s_2*arc_rad2deg);
  }

  MinRadius =  A1min.s_1;
  MaxRadius =  A1max.s_1;
  MinAngle  =  A1min.s_2 + floor(AngleMin/arc_twopi)*arc_twopi;
  MaxAngle  =  A1max.s_2 + floor(AngleMin/arc_twopi)*arc_twopi;

  if (testbit) {
    printf("ang_sum: MinRadius=%g, MaxRadius=%g, MinAngle=%g_deg, MaxAngle=%g_deg\n",
            MinRadius, MaxRadius, MinAngle*arc_rad2deg, MaxAngle*arc_rad2deg );
  }

  /* loop over I0Data */
  DDAngle = Ps_20/(MIN2(Ps_11,Ps_21));

  if (testbit) printf("ang_sum: DDAngle   = % g_deg/m\n", 
    DDAngle * arc_rad2deg);

  K    = (double) WAVENUMBER(I1WaveLength);
  rot1 = (double) I1DetRot1;
  rot2 = (double) I1DetRot2;
  rot3 = (double) I1DetRot3;

  waxs_Init ( &I1params, K, rot1, rot2, rot3 );
  waxs_Init ( &I0params, K, 0.0, 0.0, 0.0 );

  if (testbit) {
     waxs_PrintParams ( stdout, I1params );
  }

  // REGROUPING BEGIN {

  /* Parameters

     Ranges:
       Output: AngleMin, AngleMax=MIN2(AngleMin+2_pi,AngleMax)
               [AngleMin..AngleMax]

       Range1: [AngleFst1=AngleMin..AngleLst1=MIN2(AngleMax,MaxAngle-2_pi)
       Range2: [AngleFst2=MAX2(AngleMin,MinAngle)..AngleLst2=MIN2(AngleMax,MaxAngle)]

   */

  /* maximum range is AngleMin .. AngleMax */
  Fst[0] = MAX2(AngleMin,MinAngle);
  Lst[0] = MIN2(AngleMax,MaxAngle);

  /* 2nd range, if MaxAngle > AngleMax  */
  Fst[1] = AngleMin;
  Lst[1] = MIN2(Fst[0],MaxAngle-arc_twopi);

  /* 3rd range, if MaxAngle > AngleMax  */
  Fst[2] = MAX2(Lst[0],MinAngle+arc_twopi);
  Lst[2] = AngleMax;

  for (range=0;range<N_RANGES;range++) {

    AngleFst = Fst[range];
    AngleLst = Lst[range];

    if (testbit) {
      printf("ang_sum: AngleFst=%g_deg, AngleLst=%g_deg\n",
              AngleFst*arc_rad2deg, AngleLst*arc_rad2deg );
    }

    if (AngleFst < AngleLst) {

      // calculate indices of pixel center from indices of lower and upper edges
      i_10=MAX2(0,floor(INDEX(MinRadius,Off_10,Ps_10)-LOWERBORDER+0.5));
      i_11=MIN2(I0Dim_1,ceil(INDEX(MaxRadius,Off_10,Ps_10)-LOWERBORDER+0.5));

      // calculate indices of pixel center from indices of lower and upper edges
      i_20=MAX2(0,floor(INDEX(AngleFst,Off_20,Ps_20)-LOWERBORDER)); // first pixel in range
      i_22=MIN2(I0Dim_2,ceil(INDEX(AngleLst,Off_20,Ps_20)-LOWERBORDER)); // first pixel after range

      if (testbit>1) {
        printf("ang_sum: i_10=%d, i_11=%d\n",i_10,i_11);
        printf("ang_sum: i_20=%d, i_22=%d\n",i_20,i_22);
      }

      for (i_1=i_10;i_1<i_11;i_1++) {
        Radius = WORLD(i_1,Off_10,Ps_10);

        if ( Radius > MaxRadius || Radius < MinRadius ) continue;

        /* number of angular intervals for averaging */
        NAngle  = MAX2(1,(int) (DDAngle * Radius) + 1 ); 
        DAngle  = Ps_20/(float) NAngle;

        if (testbit>1)
          printf("ang_sum: %d: Radius=%g, NAngle=%d, DAngle=%g_deg\n",
            i_1,Radius,NAngle,DAngle*arc_rad2deg);

        // the integration range is from [AngleFst..AngleLst];
        for (i_2=i_20;i_2<i_22;i_2++) {

          // use the center of the angular interval to get the pixel value
          Angle = WORLD(((float)i_2),Off_20,Ps_20); // output angle 
          AngleLower = WORLD(((float)i_2+LOWERBORDER),Off_20,Ps_20); // lower limit
          AngleUpper = WORLD(((float)i_2+1+LOWERBORDER),Off_20,Ps_20); // upper limit

          // restrict integration range to [AngleFst..AngleLst];
          AngleLower = MAX2(AngleFst,AngleLower);
          AngleUpper = MIN2(AngleLst,AngleUpper);

          if (testbit>2)
            printf("ang_sum:   %d,%d: Angle=%g_deg, AngleLower=%g_deg, AngleUpper=%g_deg\n",
              i_1,i_2,Angle*arc_rad2deg,AngleLower*arc_rad2deg,AngleUpper*arc_rad2deg);

          /* angular averaging */ 
          cnt = 0;
          varcnt = 0;
          I1CircleSum = 0.0; I1CircleSumWeight = 0.0;
          E1CircleSum = 0.0; E1CircleSumWeight = 0.0;
          // Angle .. Angle+NAngle*DAngle
          ianglefst = 0;
          ianglelst = floor( (AngleUpper - AngleLower) / DAngle + 0.5);

          // the reduced angular range is [AngleLower..AngleUpper]
          angle = AngleLower + DAngle*0.5;
          if (testbit>2)
            printf("ang_sum:     %d,%d: angle=%g_deg, ianglefst=%d, ianglelst=%d\n",
              i_1,i_2,angle*arc_rad2deg,ianglefst,ianglelst);
          // average only in range with valid pixels
          for (iangle = ianglefst; iangle<ianglelst; iangle++) {

            /* averaging range is [AngleFst..AngleLst] */
            if ( ( angle < AngleLower ) || ( AngleUpper < angle ) ) {
              if (testbit>3)
                printf("ang_sum:       iangle=%d: angle=%g_deg not in [%g_deg..%g_deg] => continue\n",
                  iangle,angle*arc_rad2deg,AngleLower*arc_rad2deg,AngleUpper*arc_rad2deg);

              angle += DAngle;

              continue;
            }

            W0.s_1 = Radius * cos( angle  );  
            W0.s_2 = Radius * sin( angle  );

            /* transform saxs-coordinate of unrotated detector (I0params) or Waxs-
               projection to saxs-coordinate of rotated detector (I1params) */
            W1 = waxs_Transform( &I0params, &I1params, transform, W0 );

            if (!W1.status) {

              f_11 = INDEX(W1.s_1,Off_11,Ps_11);
              f_21 = INDEX(W1.s_2,Off_21,Ps_21);
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
                if ( Isum2ldw(I1Data,I1Dim_1,I1Dim_2,I1Dummy,I1DDummy,
                       f_11-0.5, f_21-0.5, f_11+0.5, f_21+0.5,
                       &I1Sum, &I1Weight) ) {
                  /* then do something with the data */
                  I1CircleSum       += I1Sum;
                  I1CircleSumWeight += I1Weight;
                  cnt++;
                } /* if Isum2ldw ... */
              }
            } // if (!W1.status)

            angle += DAngle;

          } /* for iangle ... */

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
              /* Take into account that the data were averaged in a sector */
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

    } /* if ( AngleFst < AngleLst ) */

  } /* for range ... */

  // REGROUPING END }

  if (pstatus) *pstatus = status;

  return;
      
ang_sum_error:
  
  if (pstatus) *pstatus = status;
  
  return;

} /* ang_sum */

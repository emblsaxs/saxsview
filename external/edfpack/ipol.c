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

# define IPOL_VERSION      "ipol : V1.7 Peter Boesecke 2010-09-29"
/*+++------------------------------------------------------------------------
NAME

   ipol --- interpolation routines

SYNOPSIS

   # include ipol.h

DESCRIPTION

   The module does not used externally defined routines and does not need
   to be linked to external libraries. 

HISTORY
  2005-12-04 V1.0 Extracted from SaxsRoutines V1.38 
  2005-12-09 V1.1 Array and dummy definitions from SaxsDefinitions.
                  Isum2ldw and Isum2ldwE: IPOL_ANTIALIASED
  2006-05-16 V1.2 Isum2ldw and Isum2ldwE: in vicinity to dummies *weight 
                  can be zero if IPOL_ANTIALIASED is set, even if cnt is not
                  zero. cnt is now explicitely set to 0 if *weight is 0.
  2007-02-26 V1.3 IpolRebin2 added
  2007-04-19 V1.4 code corrected to avoid compiler warnings with -Wall
  2008-05-25 V1.5 Isum2ldwE (calculation of varsum): if VarDat is NULL 
                  pvarval is not incremented and points always to 0,
                  unused variable pvarstart has been removed.
                  Isum2ldwEw (weighted sum), Isum2ldwE renamed to Isum2ldwEe.
                  Isum2ldwE uses Isum2ldwEe or Isum2ldwEw depending on
                  the switch variable IPolWeight.
  2008-05-27 V1.6 IPolMin: minimum coverage ratio of all non-dummy input 
                  pixels to the output pixel. 
                  If IPolMin*the output pixel area is larger than the sum
                  of all contributing input pixel areas, the calculated 
                  value is rejected (cnt=0). 

                  To do: the weight mode should also be used for rebinning,
                         probably better: pixels covering regions fully 
                         inside the output pixel should not be rejected, i.e.
                         that are contributing with 100%.
  2010-09-29 V1.7 IDX redefined using floor function

DESCRIPTION

----------------------------------------------------------------------------*/
/******************************************************************************
* Include Files                                                               *
******************************************************************************/

# include "ipol.h"

/******************************************************************************
* Private Macros                                                              *
******************************************************************************/

/******************************************************************************
* Private Variables                                                           *
******************************************************************************/

PRIVATE int IPolMode   = IPOL_NORMAL;
PRIVATE int IPolWeight = IPOL_EQUAL;
PRIVATE float IPolMin  = 0.5; // Minimum ratio of output to input pixel size

/******************************************************************************
* Routines                                                                    *
******************************************************************************/

/*---------------------------------------------------------------------------
NAME

  Ipolmode --- set/return interpolation mode 

DESCRIPTION
  Sets the interpolation mode and returns its new value:

    IPOL_NORMAL: normal, IPOL_ANTIALIASED: anti-aliased  

  If the input value is 0 only the current mode is returned.

---------------------------------------------------------------------------*/
int Ipolmode ( int mode )
{ switch (mode ) {
    case      IPOL_NORMAL: IPolMode = IPOL_NORMAL; break;
    case IPOL_ANTIALIASED: IPolMode = IPOL_ANTIALIASED; break;
  }
  return ( IPolMode );
} /* Ipolmode */

const char *Ipolmode2str ( int mode )
{ switch (mode ) {
    case      IPOL_NORMAL: return("normal");
    case IPOL_ANTIALIASED: return("antialiased");
    default: return("undefined");
  }
} /* Ipolmode2str */

/*---------------------------------------------------------------------------
NAME

  Ipolweight --- set/return weight method

DESCRIPTION
  Sets the method of weighting of data points and returns the new value:

    IPOL_EQUAL: each data point has equal weight, 
    IPOL_WEIGHTED: each point is weighted with its inverse variance 

  If the input value is 0 only the current method is returned.

---------------------------------------------------------------------------*/
int Ipolweight ( int method )
{ switch (method ) {
    case IPOL_EQUAL:    IPolWeight = IPOL_EQUAL; break;
    case IPOL_WEIGHTED: IPolWeight = IPOL_WEIGHTED; break;
  }
  return ( IPolWeight );
} /* Ipolweight */

const char *Ipolweight2str ( int method )
{ switch (method ) {
    case IPOL_EQUAL:    return("equal");
    case IPOL_WEIGHTED: return("weighted");
    default: return("undefined");
  }
} /* Ipolweight2str */

/*---------------------------------------------------------------------------
NAME

  Ipolmin --- set/return minimum ratio between input and output pixel area 

DESCRIPTION
  Sets the minimum accepted ratio between input and output pixel area
  If not used the default setting is 0.5. 

  If the input value is negative only the actually used value is returned.

---------------------------------------------------------------------------*/
float Ipolmin ( float minimum )
{ if (minimum>=0.0) IPolMin = minimum;
  return ( IPolMin );
} /* Ipolmin */

/*---------------------------------------------------------------------------
NAME
 
  Isum2ldwEw --- Weighted pixel area integral with variance array
 
PURPOSE
  Like Isum2ldwEw but each point is additionally weighted with 1/variance.
 
DESCRIPTION
  The output value "sum" is the weighted area integral of all pixel values in 
  the region between (f1_1,f1_2) and (f3_1,f3_2). If one of the pixels lies 
  outside the range of the image or if it is a dummy it is ignored (wi=0):
  All pixels are weighted with 1/variance. 
  If at least one pixel of the sum has zero variance only the pixels with
  variance 0 will contribute to the sum according to the covered area.

  The output value "varsum" is the area integral of all variance values
  in the same region weighted with 1/variance. It is effectively the sum of
  all contributing pixels. If the variance value of at least one non-dummy 
  pixel in the region is negative "varsum" cannot be calculated and is set 
  to -1.0.
 
    area      = (f1_2-f1_1)*(f3_2,f3_1)
    sum       = vsign * Sum(Ii*wi/Ei)/Mean(1/Ei) 
                  (vsign * Sum(Ii*wi) if one of the Ei is 0)
    weight    = vsign * Sum(wi/Ei)/Mean(1/Ei) 
                  (vsign * Sum(wi) if one of the Ei is 0)
    varsum    = Sum(wi)/Mean(1/Ei) (Sum(Ei*wi) if one of the Ei is 0)
                  or -1.0 if it cannot be calculated
    varweight = Sum(wi/Ei)/Mean(1/Ei) (Sum(wi) if one of the Ei is 0)
    cnt       = number of contributing intensity values

           where Mean(1/Ei) = Sum(1/Ei)/cnt
 
    wi = overlap area of pixel i with [(f1_1,f1_2)..(f3_1,f3_2)]
 
  For non-zero weight, the averaged values are Sum(Ii*wi/Ei)/Sum(wi/Ei) and
  Sum(wi)/Sum(wi/Ei). vsign takes into account the direction of integration.
  If the integration along a single axis is done from positive to negative 
  coordinates vsign is negative, otherwise positive.

  The variance values are always positive, independent of the integration
  direction. If the variance sum varsum is negative it is invalid.
  If VarDat is NULL, sum, weight and varweight are calculated, varsum is 
  0.0 for all non-dummy pixels inside the array and negative for all pixels
  outside the array.

SYNOPSIS
 
  int Isum2ldwEw           ( float *Data, float *VarDat, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f1_1, float f1_2,  float f3_1, float f3_2,
                             float *sum, float *weight,
                             float *varsum, float *varweight);

  return value            (o)   : number of pixels contributing to the output
                                  value (0..4), if 0, no valid data point found.
  float Data[Dim_1,Dim_2]   (i) : input data array
  float VarDat[Dim_1,Dim_2] (i) : input variance array
  int Dim_1, Dim_2          (i) : dimension of array
  float Dummy, DDummy       (i) : dummy specification
  float f1_1, f1_2          (i) : index coordinate of lower left corner
  float f3_1, f3_2          (i) : index coordinate of upper right corner
  float *sum                (o) : area integral of data array (Data)
  float *weight             (o) : area of contributing pixels to sum
  float *varsum             (o) : area integral of variance array (VarDat)
                                  *varsum is set to -1.0 if one of the 
                                  contributing non-dummy pixels is negative.
  float *varweight          (o) : area of contributing pixels
 
---------------------------------------------------------------------------*/
int Isum2ldwEw             ( float *Data, float *VarDat, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f1_1, float f1_2,  float f3_1, float f3_2,
                             float *sum, float *weight, 
                             float *varsum, float *varweight)
{ // const float eps = IPOLEPS; // unused
  int cnt, varcnt;
  float *pval, *pstart;
  float *pvarval, varval, invvarval, wdvarval;
  float vsign=1.0, vvar=1.0;
  float w;
  float w1_1,w1_2,w3_1,w3_2;
  float null = 0.0;

  float weightv, sumv, varsumv;
  int cntv, varcntv;
  float suminvvar, meaninvvar;
  int varflag = 1; // set to 0 if one of the variances is 0
 
  int   i_1,  i_2;
  int   i1_1, i1_2, i3_1, i3_2;
  float Df_1, Df_2, Df_12;
  float Df_1A, Df_2A, Df_12A;
  float f_1A, f_2A;
  float tmp;
 
  Df_1  = f3_1-f1_1;
  Df_2  = f3_2-f1_2;
  Df_12 = Df_1*Df_2;
 
  /* sort range */
  if (Df_1<0) {tmp=f1_1;f1_1=f3_1;f3_1=tmp;vsign*=-1.0;Df_1=-Df_1;}
  if (Df_2<0) {tmp=f1_2;f1_2=f3_2;f3_2=tmp;vsign*=-1.0;Df_2=-Df_2;}
 
  *weight=0.0;
  *varweight=0.0;
  *sum=0.0;
  *varsum=-1.0;
  cnt=0;
  varcnt=0;

  weightv = 0.0;
  sumv = 0.0;
  varsumv = 0.0;
  cntv = 0;
  varcntv = 0;

  suminvvar = 0.0;
 
  if (!Data) return(cnt);    // return, if NULL pointer

  if (IPolMode == IPOL_ANTIALIASED) {
    /* Increase the integration range in both directions to 1. For
       renormalization of the results adjust vsign. */
    if (Df_1<=1.0) {
      Df_1A = 1.0;
      f_1A  = 0.5*(f1_1+f3_1);
      f1_1  =f_1A-0.5;f3_1=f_1A+0.5;
    } else Df_1A = Df_1;

    if (Df_2<=1.0) {
      Df_2A = 1.0;
      f_2A  = 0.5*(f1_2+f3_2);
      f1_2  =f_2A-0.5;f3_2=f_2A+0.5;
    } else Df_2A = Df_2;

    Df_12A = Df_1A*Df_2A;
    if (Df_12A == 0.0) return(cnt);

    tmp = Df_12/Df_12A;
    vvar  *= tmp;
    vsign *= tmp;
  }
 
  /* Add 0.5 to exclude negative array indices */
  f1_1+=0.5; f1_2+=0.5; f3_1+=0.5; f3_2+=0.5;
 
  // Lower left edge f1, w1_i is the overlap area of pixel p1
  // All w1_i are positive: 0<=w1_i<=1.0
  i1_1 = floor(f1_1);
  if (i1_1 >= 0 ) w1_1 = 1.0 - (f1_1 - i1_1);
    else {i1_1 = 0; w1_1 = 1.0; }
 
  i1_2 = floor(f1_2);
  if (i1_2 >= 0 ) w1_2 = 1.0 - (f1_2 - i1_2);
    else {i1_2 = 0; w1_2 = 1.0; }
 
  // upper right edge f3, w3_i is the overestimated area of pixel p3
  // All w3_i are negative or zero: -1.0<=w3_i<=0.0
  i3_1 = ceil(f3_1);
  if (i3_1<=Dim_1) w3_1 = f3_1 - i3_1;
    else { i3_1 = Dim_1; w3_1 = 0.0; }
 
  i3_2 = ceil(f3_2);
  if (i3_2<=Dim_2) w3_2 = f3_2 - i3_2;
    else { i3_2 = Dim_2; w3_2 = 0.0; }
 
  // Stop, if already the lower left pixel p1 lies outside the array.
  // or if upper right pixel lies outside the array.
  if ((i1_1>=Dim_1)||((i1_2>=Dim_2))||(i3_1<0)||(i3_2<0)) return(cnt);

  pvarval = &null; // if VarDat is NULL
  *varsum = 0.0;

  cntv = 0;
  suminvvar = 0.0;
  weightv = 0.0;
  sumv = 0.0;

  varcntv = 0;
  varsumv = 0.0; 

  varflag = 1; // set to 0 if one of the variances is 0
 
  // p1 (lower left pixel)
  pstart = pval = ABSPTR(Data,Dim_1,Dim_2,i1_1,i1_2);
  if (VarDat) pvarval = VarDat-Data+pval;
  w = w1_1*w1_2;
  if NODUMMY(*pval,Dummy,DDummy) {
    varval=*pvarval;
    if (varval==0.0) {
      cnt++; *weight+=w; 
      *sum+=*pval*w; 
      //*varsum+=varval*w;
      varcnt++; 
      varflag=0; 
    } else {
      if ((varflag)&&(varval>0)) {
        invvarval=1.0/varval;
        wdvarval=w*invvarval;
        cntv++; weightv+=wdvarval;
        sumv+=*pval*wdvarval; 
        suminvvar+=invvarval;
        varsumv+=w; varcntv++; 
      }
    }
  }

  // lower line between p1 and p2
  for (i_1=i1_1+1;i_1<i3_1;i_1++) {
    pval = NEXTCOL(pval,Dim_1,Dim_2);
    if (VarDat) pvarval = VarDat-Data+pval;
    w=w1_2;
    if NODUMMY(*pval,Dummy,DDummy) {
      varval=*pvarval;
      if (varval==0.0) {
        cnt++; *weight+=w;
        *sum+=*pval*w;
        //*varsum+=varval*w;
        varcnt++;
        varflag=0;
      } else {
        if ((varflag)&&(varval>0)) {
          invvarval=1.0/varval;
          wdvarval=w*invvarval;
          cntv++; weightv+=wdvarval;
          sumv+=*pval*wdvarval;
          suminvvar+=invvarval;
          varsumv+=w; varcntv++;
        }
      }
    }
  }
 
  // p2 (correct overestimation)
  w = w3_1*w1_2;
  if NODUMMY(*pval,Dummy,DDummy) {
    varval=*pvarval;
    if (varval==0.0) {
      cnt++; *weight+=w;
      *sum+=*pval*w;
      //*varsum+=varval*w;
      varcnt++;
      varflag=0;
    } else {
      if ((varflag)&&(varval>0)) {
        invvarval=1.0/varval;
        wdvarval=w*invvarval;
        cntv++; weightv+=wdvarval;
        sumv+=*pval*wdvarval;
        suminvvar+=invvarval;
        varsumv+=w; varcntv++;
      }
    }
  }

  for (i_2=i1_2+1;i_2<i3_2;i_2++) {
    // first column
    pstart = pval = NEXTROW(pstart,Dim_1,Dim_2);
    if (VarDat) pvarval = VarDat-Data+pval;
    w=w1_1;
    if NODUMMY(*pval,Dummy,DDummy) {
      varval=*pvarval;
      if (varval==0.0) {
        cnt++; *weight+=w;
        *sum+=*pval*w;
        //*varsum+=varval*w;
        varcnt++;
        varflag=0;
      } else {
        if ((varflag)&&(varval>0)) {
          invvarval=1.0/varval;
          wdvarval=w*invvarval;
          cntv++; weightv+=wdvarval;
          sumv+=*pval*wdvarval;
          suminvvar+=invvarval;
          varsumv+=w; varcntv++;
        }
      }
    }

    // line
    for (i_1=i1_1+1;i_1<i3_1;i_1++) {
      pval = NEXTCOL(pval,Dim_1,Dim_2);
      if (VarDat) pvarval = VarDat-Data+pval;
      // w=1.0
      if NODUMMY(*pval,Dummy,DDummy) {
        varval=*pvarval;
        if (varval==0.0) {
          cnt++; *weight+=1.0;
          *sum+=*pval;
          //*varsum+=varval;
          varcnt++;
          varflag=0;
        } else {
          if ((varflag)&&(varval>0)) {
            invvarval=1.0/varval;
            cntv++; weightv+=invvarval;
            sumv+=*pval*invvarval;
            suminvvar+=invvarval;
            varsumv+=1.0; varcntv++;
          }
        }
      }
    }
 
    // last column (correct overestimation)
    w=w3_1;
    if NODUMMY(*pval,Dummy,DDummy) {
      varval=*pvarval;
      if (varval==0.0) {
        cnt++; *weight+=w;
        *sum+=*pval*w;
        //*varsum+=varval*w;
        varcnt++;
        varflag=0;
      } else {
        if ((varflag)&&(varval>0)) {
          invvarval=1.0/varval;
          wdvarval=w*invvarval;
          cntv++; weightv+=wdvarval;
          sumv+=*pval*wdvarval;
          suminvvar+=invvarval;
          varsumv+=w; varcntv++;
        }
      }
    }
  }
 
  // p4 (correct overestimation)
  pval = pstart;
  if (VarDat) pvarval = VarDat-Data+pval;
  w = w1_1*w3_2;
  if NODUMMY(*pval,Dummy,DDummy) {
    varval=*pvarval;
    if (varval==0.0) {
      cnt++; *weight+=w;
      *sum+=*pval*w;
      //*varsum+=varval*w;
      varcnt++;
      varflag=0;
    } else {
      if ((varflag)&&(varval>0)) {
        invvarval=1.0/varval;
        wdvarval=w*invvarval;
        cntv++; weightv+=wdvarval;
        sumv+=*pval*wdvarval;
        suminvvar+=invvarval;
        varsumv+=w; varcntv++;
      }
    }
  }

  // upper line between p4 and p3 (correction)
  for (i_1=i1_1+1;i_1<i3_1;i_1++) {
    pval = NEXTCOL(pval,Dim_1,Dim_2);
    if (VarDat) pvarval = VarDat-Data+pval;
    w=w3_2;
    if NODUMMY(*pval,Dummy,DDummy) {
      varval=*pvarval;
      if (varval==0.0) {
        cnt++; *weight+=w;
        *sum+=*pval*w;
        //*varsum+=varval*w;
        varcnt++;
        varflag=0;
      } else {
        if ((varflag)&&(varval>0)) {  
          invvarval=1.0/varval;
          wdvarval=w*invvarval;
          cntv++; weightv+=wdvarval;
          sumv+=*pval*wdvarval;
          suminvvar+=invvarval;
          varsumv+=w; varcntv++;
        }
      }
    }
  }
 
  // p3 (correct underestimation)
  w = w3_1*w3_2;
  if NODUMMY(*pval,Dummy,DDummy) {
    varval=*pvarval;
    if (varval==0.0) {
      cnt++; *weight+=w;
      *sum+=*pval*w;
      //*varsum+=varval*w;
      varcnt++;
      varflag=0;
    } else {
      if ((varflag)&&(varval>0)) {  
        invvarval=1.0/varval;
        wdvarval=w*invvarval;
        cntv++; weightv+=wdvarval;
        sumv+=*pval*wdvarval;
        suminvvar+=invvarval;
        varsumv+=w; varcntv++;
      }
    }
  }

  if (varflag) {
    meaninvvar = suminvvar/cntv;
    cnt        = cntv;
    varcnt     = varcntv;
    *sum       = sumv/meaninvvar;
    *weight    = weightv/meaninvvar;
    *varsum    = varsumv/meaninvvar;
  }

  *sum      *= vsign;
  *weight   *= vsign;

  if (varcnt!=cnt) *varsum = -1.0; else *varsum *= vvar;

  *varweight = fabs(*weight);

  // *weight can apparently be zero when cnt is not if IPOL_ANTIALIASED is set
  if (fabs(*weight)<1e-32) cnt=0;
  else // reject pixels with less than IPolMin coverage
    if ( fabs(Df_12)*IPolMin > fabs(*weight) ) cnt=0;

  return(cnt);
 
} /* Isum2ldwEw */

/*---------------------------------------------------------------------------
NAME
 
  Isum2ldwEe--- Pixel area integral with variance array
 
PURPOSE
  Like Isum2ldw but additionally with calculation of variance array.
 
DESCRIPTION
  The output value "sum" is the area integral of all pixel values in the region
  between (f1_1,f1_2) and (f3_1,f3_2). If one of the pixels lies outside the
  range of the image or if it is a dummy it is ignored (wi=0):

  The output value "varsum" is the area integral of all variance values
  in the same region. If the variance value of at least one non-dummy pixel in
  the region is negative "varsum" cannot be calculated and is set to -1.0.
 
    area      = (f1_2-f1_1)*(f3_2,f3_1)
    sum       = vsign * Sum(Ii*wi)
    weight    = vsign * Sum(wi)
    varsum    = Sum(Ei*wi) or -1.0 if it cannot be calculated
    varweight = Sum(wi)
    cnt       = number of contributing intensity values
 
    wi = overlap area of pixel i with [(f1_1,f1_2)..(f3_1,f3_2)]
 
  For non-zero weight, the averaged values are Sum(Ii*wi)/Sum(wi) and
  Sum(Ei*wi)/Sum(wi). vsign takes into account the direction of integration.
  If the integration along a single axis is done from positive to negative 
  coordinates vsign is negative, otherwise positive.

  The variance values are always positive, independent of the integration
  direction. If the variance sum varsum is negative it is invalid.
  If VarDat is NULL, sum, weight and varweight are calculated, varsum is 
  0.0 for all non-dummy pixels inside the array and negative for all pixels
  outside the array.

SYNOPSIS
 
  int Isum2ldwEe           ( float *Data, float *VarDat, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f1_1, float f1_2,  float f3_1, float f3_2,
                             float *sum, float *weight,
                             float *varsum, float *varweight);

  return value            (o)   : number of pixels contributing to the output
                                  value (0..4), if 0, no valid data point found.
  float Data[Dim_1,Dim_2]   (i) : input data array
  float VarDat[Dim_1,Dim_2] (i) : input variance array
  int Dim_1, Dim_2          (i) : dimension of array
  float Dummy, DDummy       (i) : dummy specification
  float f1_1, f1_2          (i) : index coordinate of lower left corner
  float f3_1, f3_2          (i) : index coordinate of upper right corner
  float *sum                (o) : area integral of data array (Data)
  float *weight             (o) : area of contributing pixels to sum
  float *varsum             (o) : area integral of variance array (VarDat)
                                  *varsum is set to -1.0 if one of the 
                                  contributing non-dummy pixels is negative.
  float *varweight          (o) : area of contributing pixels
 
---------------------------------------------------------------------------*/
int Isum2ldwEe             ( float *Data, float *VarDat, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f1_1, float f1_2,  float f3_1, float f3_2,
                             float *sum, float *weight, 
                             float *varsum, float *varweight)
{ // const float eps = IPOLEPS; // unused
  int cnt, varcnt;
  float *pval, *pstart;
  float *pvarval;
  float vsign=1.0, vvar=1.0;
  float w;
  float w1_1,w1_2,w3_1,w3_2;
  float null = 0.0;
 
  int   i_1,  i_2;
  int   i1_1, i1_2, i3_1, i3_2;
  float Df_1, Df_2, Df_12;
  float Df_1A, Df_2A, Df_12A;
  float f_1A, f_2A;
  float tmp;
 
  Df_1  = f3_1-f1_1;
  Df_2  = f3_2-f1_2;
  Df_12 = Df_1*Df_2;
 
  /* sort range */
  if (Df_1<0) {tmp=f1_1;f1_1=f3_1;f3_1=tmp;vsign*=-1.0;Df_1=-Df_1;}
  if (Df_2<0) {tmp=f1_2;f1_2=f3_2;f3_2=tmp;vsign*=-1.0;Df_2=-Df_2;}
 
  *weight=0.0;
  *varweight=0.0;
  *sum=0.0;
  *varsum=-1.0;
  cnt=0;
  varcnt=0;
 
  if (!Data) return(cnt);    // return, if NULL pointer

  if (IPolMode == IPOL_ANTIALIASED) {
    /* Increase the integration range in both directions to 1. For
       renormalization of the results adjust vsign. */
    if (Df_1<=1.0) {
      Df_1A = 1.0;
      f_1A  = 0.5*(f1_1+f3_1);
      f1_1  =f_1A-0.5;f3_1=f_1A+0.5;
    } else Df_1A = Df_1;

    if (Df_2<=1.0) {
      Df_2A = 1.0;
      f_2A  = 0.5*(f1_2+f3_2);
      f1_2  =f_2A-0.5;f3_2=f_2A+0.5;
    } else Df_2A = Df_2;

    Df_12A = Df_1A*Df_2A;
    if (Df_12A == 0.0) return(cnt);

    tmp = Df_12/Df_12A;
    vvar  *= tmp;
    vsign *= tmp;
  }
 
  /* Add 0.5 to exclude negative array indices */
  f1_1+=0.5; f1_2+=0.5; f3_1+=0.5; f3_2+=0.5;
 
  // Lower left edge f1, w1_i is the overlap area of pixel p1
  // All w1_i are positive: 0<=w1_i<=1.0
  i1_1 = floor(f1_1);
  if (i1_1 >= 0 ) w1_1 = 1.0 - (f1_1 - i1_1);
    else {i1_1 = 0; w1_1 = 1.0; }
 
  i1_2 = floor(f1_2);
  if (i1_2 >= 0 ) w1_2 = 1.0 - (f1_2 - i1_2);
    else {i1_2 = 0; w1_2 = 1.0; }
 
  // upper right edge f3, w3_i is the overestimated area of pixel p3
  // All w3_i are negative or zero: -1.0<=w3_i<=0.0
  i3_1 = ceil(f3_1);
  if (i3_1<=Dim_1) w3_1 = f3_1 - i3_1;
    else { i3_1 = Dim_1; w3_1 = 0.0; }
 
  i3_2 = ceil(f3_2);
  if (i3_2<=Dim_2) w3_2 = f3_2 - i3_2;
    else { i3_2 = Dim_2; w3_2 = 0.0; }
 
  // Stop, if already the lower left pixel p1 lies outside the array.
  // or if upper right pixel lies outside the array.
  if ((i1_1>=Dim_1)||((i1_2>=Dim_2))||(i3_1<0)||(i3_2<0)) return(cnt);

  pvarval = &null; // if VarDat is NULL
  *varsum = 0.0;
 
  // p1 (lower left pixel)
  pstart = pval = ABSPTR(Data,Dim_1,Dim_2,i1_1,i1_2);
  if (VarDat) pvarval = VarDat-Data+pval;
  w = w1_1*w1_2;
  if NODUMMY(*pval,Dummy,DDummy) { 
    cnt++; *weight+=w; 
    *sum+=*pval*w; 
    if (*pvarval>=0) { *varsum+=*pvarval*w; varcnt++; }
  }
  // lower line between p1 and p2
  for (i_1=i1_1+1;i_1<i3_1;i_1++) {
    pval = NEXTCOL(pval,Dim_1,Dim_2);
    if (VarDat) pvarval = VarDat-Data+pval;
    if NODUMMY(*pval,Dummy,DDummy) { 
      cnt++; *weight+=w1_2; 
      *sum+=*pval*w1_2; 
      if (*pvarval>=0) { *varsum+=*pvarval*w1_2; varcnt++; } 
    }
  }
 
  // p2 (correct overestimation)
  w = w3_1*w1_2;
  if NODUMMY(*pval,Dummy,DDummy) { 
    cnt++; *weight+=w; 
    *sum+=*pval*w; 
    if (*pvarval>=0) { *varsum+=*pvarval*w; varcnt++; } 
  }
 
  for (i_2=i1_2+1;i_2<i3_2;i_2++) {
    // first column
    pstart = pval = NEXTROW(pstart,Dim_1,Dim_2);
    if (VarDat) pvarval = VarDat-Data+pval;
    if NODUMMY(*pval,Dummy,DDummy) { 
      cnt++; *weight+=w1_1; 
      *sum+=*pval*w1_1; 
      if (*pvarval>=0) { *varsum+=*pvarval*w1_1; varcnt++; } 
    }
 
    // line
    for (i_1=i1_1+1;i_1<i3_1;i_1++) {
      pval = NEXTCOL(pval,Dim_1,Dim_2);
      if (VarDat) pvarval = VarDat-Data+pval;
      if NODUMMY(*pval,Dummy,DDummy) { 
        cnt++; *weight+=1.0; 
        *sum+=*pval; 
        if (*pvarval>=0) { *varsum+=*pvarval; varcnt++; }
      }
    }
 
    // last column (correct overestimation)
    if NODUMMY(*pval,Dummy,DDummy) { 
      cnt++; *weight+=w3_1; 
      *sum+=*pval*w3_1; 
      if (*pvarval>=0) { *varsum+=*pvarval*w3_1; varcnt++; }
    }
  }
 
  // p4 (correct overestimation)
  pval = pstart;
  if (VarDat) pvarval = VarDat-Data+pval;
  w = w1_1*w3_2;
  if NODUMMY(*pval,Dummy,DDummy) { 
    cnt++; *weight+=w; 
    *sum+=*pval*w; 
    if (*pvarval>=0) { *varsum+=*pvarval*w; varcnt++; }
  }
 
  // upper line between p4 and p3 (correction)
  for (i_1=i1_1+1;i_1<i3_1;i_1++) {
    pval = NEXTCOL(pval,Dim_1,Dim_2);
    if (VarDat) pvarval = VarDat-Data+pval;
    if NODUMMY(*pval,Dummy,DDummy) { 
      cnt++; *weight+=w3_2; 
      *sum+=*pval*w3_2; 
      if (*pvarval>=0) { *varsum+=*pvarval*w3_2; varcnt++; }
    }
  }
 
  // p3 (correct underestimation)
  w = w3_1*w3_2;
  if NODUMMY(*pval,Dummy,DDummy) { 
    cnt++; *weight+=w; 
    *sum+=*pval*w; 
    if (*pvarval>=0) { *varsum+=*pvarval*w; varcnt++; }
  }

  *sum      *= vsign;
  *weight   *= vsign;

  if (varcnt!=cnt) *varsum = -1.0; else *varsum *= vvar;

  *varweight = fabs(*weight);

  // *weight can apparently be zero when cnt is not if IPOL_ANTIALIASED is set
  if (fabs(*weight)<1e-32) cnt=0;
  else // reject pixels with less than IPolMin coverage
    if ( fabs(Df_12)*IPolMin > fabs(*weight) ) cnt=0;

  return(cnt);
 
} /* Isum2ldwEe*/

/*---------------------------------------------------------------------------
NAME

  Isum2ldwE --- Pixel area integral with variance array
 
PURPOSE
  Use Isum2ldwEe or Isum2ldwEw depending on IPolWeight.
 
DESCRIPTION
  See Isum2ldwEe and Isum2ldwEw.
---------------------------------------------------------------------------*/
int Isum2ldwE              ( float *Data, float *VarDat, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f1_1, float f1_2,  float f3_1, float f3_2,
                             float *sum, float *weight,
                             float *varsum, float *varweight)
{ int cnt=0;
  switch (IPolWeight) {
    case IPOL_EQUAL:
      cnt = Isum2ldwEe   ( Data, VarDat, Dim_1, Dim_2,
                             Dummy, DDummy, f1_1, f1_2,  f3_1, f3_2,
                             sum, weight, varsum, varweight); break;
    case IPOL_WEIGHTED:
      cnt = Isum2ldwEw   ( Data, VarDat, Dim_1, Dim_2,
                             Dummy, DDummy, f1_1, f1_2,  f3_1, f3_2,
                             sum, weight, varsum, varweight); break;
  }
  return( cnt );
} // Isum2ldwE

/*---------------------------------------------------------------------------
NAME

  Isum2ldw --- Pixel area integral

PURPOSE
  Area integral of the array Data between index coordinate (f1_1,f1_2) and 
  index coordinate (f3_1,f3_2). Dummy values and array limits are checked. 
  The contributing area is returned in weight. The intensity value per
  pixel area is sum divided by weight. The return value of the function is the 
  number of contributing pixels. 
  A call with (f3_1,f3_2) = (f1_1+1,f1_2+1) corresponds to a call to Ipol2ldw
  with (f_1,f_2) = (f1_1+0.5,f1_2+0.5). See also Ipol2ldw.

  If the interpolation mode is IPOL_ANTIALIASED (default), the minimum side 
  length of an integration area is 1. Smaller side lengths are increased to 1 
  and the results are renomalized to match the original side lengths. 

DESCRIPTION
  The output value "sum" is the area integral of all pixel values in the region 
  between (f1_1,f1_2) and (f3_1,f3_2). If one of the pixels lies outside the 
  range of the image or if it is a dummy it is ignored (wi=0):
 
    sum    = Sum(Ii*wi)
    weight = Sum(wi)
    cnt    = number of contributing intensity values
 
    wi = overlap area of pixel i with [(f1_1,f1_2)..(f3_1,f3_2)] 
 
  For non-zero weight, the averaged value is Sum(Ii*wi)/Sum(wi).
 
              --------------------- --------------------- ---------------------
   f3_2     - |             ******| |*******************| |**************     |
              |             ******|.|*******************|.|**************     |
              |        p4- -*-*-*-|.|-*-*-*-*-*-*-*-*-*-|.|-*-*-*-*p3****     |
              |         |   ******|.|*******************|.|*********|****     |
              |             ******| |*******************| |**************     |
              --------------------- --------------------- ---------------------
                       ...                   ...                   ...
              --------------------- --------------------- ---------------------
              |             ******| |*******************| |*********|****     |
  f = ***     |         |   ******|.|*******************|.|**************     |
              |             ******|.|*******************|.|*********|****     |
              |         |   ******|.|*******************|.|**************     |
              |             ******| |*******************| |*********|****     |
              --------------------- --------------------- ---------------------
                       ...                   ...                   ...
              --------------------- --------------------- ---------------------
   f1_2     - |             ******| |*******************| |**************     |
              |         |         |.|                   |.|         |         |
              |        p1- - - - -|.|- - - - - - - - - -|.|- - - - p2         |
              |                   |.|                   |.|                   |
              |                   | |                   | |                   |
              --------------------- --------------------- ---------------------
                            |                                           |
                          f1_1                                        f3_1

 
SYNOPSIS
 
  int Isum2ldw ( float *Data, int Dim_1, int Dim_2, float Dummy, float DDummy,
                 float f1_1, float f1_2,  float f3_1, float f3_2,
                 float *sum, float *weight);
 
  return value            (o)   : number of pixels contributing to the output
                                  value (0..4), if 0, no valid data point found.
  float Data[Dim_1,Dim_2] (i)   : input array
  int Dim_1, Dim_2        (i)   : dimension of array
  float Dummy, DDummy     (i)   : dummy specification
  float f1_1, f1_2        (i)   : index coordinate of lower left corner
  float f3_1, f3_2        (i)   : index coordinate of upper right corner 
  float *sum              (o)   : area integral of Data array
  float *weight           (o)   : area of contributing pixels 
  
---------------------------------------------------------------------------*/
int Isum2ldw               ( float *Data, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f1_1, float f1_2,  float f3_1, float f3_2,
                             float *sum, float *weight)
{ // const float eps = IPOLEPS; // unused
  int cnt=0;
  float *pval, *pstart;
  float vsign=1.0;
  float w;
  float w1_1,w1_2,w3_1,w3_2;
 
  int   i_1,  i_2;
  int   i1_1, i1_2, i3_1, i3_2;
  float Df_1, Df_2, Df_12;
  float Df_1A, Df_2A, Df_12A;
  float f_1A, f_2A;
  float tmp;

  Df_1  = f3_1-f1_1;
  Df_2  = f3_2-f1_2;
  Df_12 = Df_1*Df_2;

  /* use faster routine Ipol2ldw, if area size is 1x1 */
  if ((Df_1==1.0)&&(Df_2==1.0)) 
   return(Ipol2ldw(Data,Dim_1,Dim_2,Dummy,DDummy,f1_1+0.5,f1_2+0.5,sum,weight));

  /* sort range */
  if (Df_1<0) {tmp=f1_1;f1_1=f3_1;f3_1=tmp;vsign*=-1.0;Df_1=-Df_1;}
  if (Df_2<0) {tmp=f1_2;f1_2=f3_2;f3_2=tmp;vsign*=-1.0;Df_2=-Df_2;}

  *weight=0.0;
  *sum=0.0;
  cnt=0;

  if (!Data) return(cnt); // return, if NULL pointer
 
  if (IPolMode == IPOL_ANTIALIASED) {
    /* Increase the integration range in both directions to 1. For 
       renormalization of the results adjust vsign. */ 
    if (Df_1<=1.0) {
      Df_1A = 1.0;
      f_1A  = 0.5*(f1_1+f3_1);
      f1_1  =f_1A-0.5;f3_1=f_1A+0.5;
    } else Df_1A = Df_1;

    if (Df_2<=1.0) {
      Df_2A = 1.0;
      f_2A  = 0.5*(f1_2+f3_2);
      f1_2  =f_2A-0.5;f3_2=f_2A+0.5;
    } else Df_2A = Df_2;

    Df_12A = Df_1A*Df_2A;

    if (Df_12A == 0.0) return(cnt);

    vsign*=Df_12/Df_12A;
  }

  /* Add 0.5 to exclude negative array indices */
  f1_1+=0.5; f1_2+=0.5; f3_1+=0.5; f3_2+=0.5;

  // Lower left edge f1, w1_i is the overlap area of pixel p1
  // All w1_i are positive: 0<=w1_i<=1.0
  i1_1 = floor(f1_1);
  if (i1_1 >= 0 ) w1_1 = 1.0 - (f1_1 - i1_1);
    else {i1_1 = 0; w1_1 = 1.0; } 

  i1_2 = floor(f1_2);
  if (i1_2 >= 0 ) w1_2 = 1.0 - (f1_2 - i1_2);
    else {i1_2 = 0; w1_2 = 1.0; }

  // upper right edge f3, w3_i is the overestimated area of pixel p3
  // All w3_i are negative or zero: -1.0<=w3_i<=0.0
  i3_1 = ceil(f3_1); 
  if (i3_1<=Dim_1) w3_1 = f3_1 - i3_1; 
    else { i3_1 = Dim_1; w3_1 = 0.0; }

  i3_2 = ceil(f3_2);
  if (i3_2<=Dim_2) w3_2 = f3_2 - i3_2;
    else { i3_2 = Dim_2; w3_2 = 0.0; }

  // Stop, if already the lower left pixel p1 lies outside the array.
  // or if upper right pixel lies outside the array.
  if ((i1_1>=Dim_1)||((i1_2>=Dim_2))||(i3_1<0)||(i3_2<0)) return(cnt);

  // p1 (lower left pixel)
  pstart = pval = ABSPTR(Data,Dim_1,Dim_2,i1_1,i1_2);
  w = w1_1*w1_2;
  if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=w; *sum+=*pval*w; }

  // lower line between p1 and p2
  for (i_1=i1_1+1;i_1<i3_1;i_1++) {
    pval = NEXTCOL(pval,Dim_1,Dim_2);
    if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=w1_2; *sum+=*pval*w1_2; }
  }

  // p2 (correct overestimation)
  w = w3_1*w1_2;
  if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=w; *sum+=*pval*w; }

  for (i_2=i1_2+1;i_2<i3_2;i_2++) {
    // first column
    pstart = pval = NEXTROW(pstart,Dim_1,Dim_2);
    if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=w1_1; *sum+=*pval*w1_1; }

    // line
    for (i_1=i1_1+1;i_1<i3_1;i_1++) {
      pval = NEXTCOL(pval,Dim_1,Dim_2);
      if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=1.0; *sum+=*pval; }
    }

    // last column (correct overestimation)
    if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=w3_1; *sum+=*pval*w3_1; }
  }

  // p4 (correct overestimation)
  pval = pstart;
  w = w1_1*w3_2;
  if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=w; *sum+=*pval*w; }

  // upper line between p4 and p3 (correction)
  for (i_1=i1_1+1;i_1<i3_1;i_1++) {
    pval = NEXTCOL(pval,Dim_1,Dim_2);
    if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=w3_2; *sum+=*pval*w3_2; }
  }

  // p3 (correct underestimation)
  w = w3_1*w3_2;
  if NODUMMY(*pval,Dummy,DDummy) { cnt++; *weight+=w; *sum+=*pval*w; }

  *sum    *= vsign;
  *weight *= vsign;

  // *weight can apparently be zero when cnt is not if IPOL_ANTIALIASED is set
  if (fabs(*weight)<1e-32) cnt=0;
  else // reject pixels with less than IPolMin coverage
    if ( fabs(Df_12)*IPolMin > fabs(*weight) ) cnt=0;

  return(cnt);

} /* Isum2ldw */

/*---------------------------------------------------------------------------
NAME

  Ipol2ldw --- Linear two dimensional interpolation

PURPOSE
  Linear two dimensional interpolation including dummies with limit checks
  Ipol2ldw calculates the weighted sum and the sum of the weights of the 
  contributing pixels. The array limits are checked. To calculate the 
  interpolated value, sum must be devided by weight. The function returns 
  the number of contributing pixels.  See also Ipol2ld.
  The result corresponds to Isum2ldw(...,f_1-0.5,f_2-0.5,f_1+0.5,f_2+0.5,...).

DESCRIPTION
  The output value "sum" is the weighted sum of the intensities of the 4 
  closest pixels p1 to p4 around pixel f. Pixel f is located at (f_1,f_2). 
  The values of the pixels p1=Data[i_1,i_2], p2=Data[i_1+1,i_2], 
  p3=Data[i_1+1,i_2+1] and p4=Data[I_1,i_2+1] are multiplied by the overlap 
  areas wi of each pixel with the pixel f and added. The overlap areas w1 
  to w4 are calculated from the displacement r_1 and r_2 of f from p1. 
  If one of the 4 pixels lies outside the range of the image or if it is 
  a dummy it is ignored (wi=0): 

    sum    = Sum(Ii*wi)
    weight = Sum(wi)
    cnt    = number of used intensity values

    w1 = (1-r_1)*(1-r_2)
    w2 = r_1*(1-r_2)
    w3 = r_1*r_2
    w4 = (1-r_1)*r_2

  with r_1 = f_1 - floor(f_1) and r_2 = f_2 - floor(f_2)

  For non-zero weight, the averaged value is Sum(Ii*wi)/Sum(wi).

                 -----------------------------------------
                 |             //////|**************     |
                 |             //////|**************     |
    w4 = ///     |        p4- -/-/-/-|-*-*-*-*p3****     |  w3 = ***
        f_2  ----|         |   //////|***f*****|****     |
             ^   |             //////|**************     |
             |   -----------------------------------------
    w1 = +++ |r_2|             ++++++|\\\\\\\\\\\\\\     |  w2 = \\\
             v   |         |         |         |         |
  floor(f_2)-----|        p1- - - - -|- - - - p2         |
                 |                   |                   |
                 |                   |                   |
                 -----------------------------------------
                           |      r_1    |
                           |<----------->|
                           |
                       floor(f_1)       f_1

SYNOPSIS

  int Ipol2ldw (float *Data, int Dim_1, int Dim_2, float Dummy, float DDummy,
                float f_1, float f_2, float *sum, float *weight)

  return value            (o)   : number of pixels contributing to the output
                                  value (0..4), if 0, no valid data point found.
  float Data[Dim_1,Dim_2] (i)   : input array
  int Dim_1, Dim_2        (i)   : dimension of array
  float Dummy, DDummy     (i)   : dummy specification
  float f_1, f_2          (i)   : program array indices (interpolation point)
  float *sum              (o)   : weighted sum of interpolated pixels 
  float *weight           (o)   : weight of sum 
                                  minimum 0.0: no valid pixel found
                                  maximum 1.0: interpolation between 1 to 4 
                                               distance weighted pixels

---------------------------------------------------------------------------*/
int Ipol2ldw (float *Data, int Dim_1, int Dim_2, float Dummy, float DDummy,
              float f_1, float f_2, float *sum, float *weight )
{ const float eps = IPOLEPS;
  int cnt;
  float *pval1, *pval2, *pval3, *pval4;
  float w1, w2, w3, w4;

  int   i_1, i_2;
  float r_1, r_2;

  /* calculate integer indices and rest */
  IDX(f_1,i_1,r_1);
  IDX(f_2,i_2,r_2);

  *weight=0.0;
  *sum=0.0;
  cnt=0;

  if (!Data) return(cnt); // return, if NULL pointer

  pval1 = ABSPTR(Data,Dim_1,Dim_2,i_1,i_2);

  /* General check */
  if ( (i_1>=0) && (i_2>=0) && (i_1<Dim_1-1) && (i_2<Dim_2-1) )
    /* all 4 points inside the array */
    if (r_1<eps) {
      if (r_2<eps) {
        if NODUMMY(*pval1,Dummy,DDummy) {
          cnt++; *weight += 1.0; *sum = *pval1; /* no interpolation */
          } /* if NODUMMY */
        } else {
        if NODUMMY(*pval1,Dummy,DDummy) {
          cnt++;w1=1.0-r_2; *weight+=w1; *sum+=*pval1*w1;
          } /* if NODUMMY */
        pval4 = NEXTROW(pval1,Dim_1,Dim_2);
        if NODUMMY(*pval4,Dummy,DDummy) {
          cnt++;w4=r_2; *weight+=w4; *sum+=*pval4*w4;
          } /* if NODUMMY */
        } /* if (r_2<eps) */
      } else {
      if (r_2<eps) {
        if NODUMMY(*pval1,Dummy,DDummy) {
          cnt++; w1=(1.0-r_1); *weight+=w1; *sum = *pval1*w1;
          } /* if NODUMMY */
        pval2 = NEXTCOL(pval1,Dim_1,Dim_2);
        if NODUMMY(*pval2,Dummy,DDummy) {
          cnt++; w2=r_1; *weight+=w2; *sum+=*pval2*w2;
          } /* if NODUMMY */
        } else {
        if NODUMMY(*pval1,Dummy,DDummy) {
          cnt++;w1=(1.0-r_1)*(1.0-r_2); *weight+=w1; *sum+=*pval1*w1;
          } /* if NODUMMY */
        pval2 = NEXTCOL(pval1,Dim_1,Dim_2);
        if NODUMMY(*pval2,Dummy,DDummy) {
          cnt++;w2=r_1*(1.0-r_2); *weight+=w2; *sum+=*pval2*w2;
          } /* if NODUMMY */
        pval4 = NEXTROW(pval1,Dim_1,Dim_2);
        if NODUMMY(*pval4,Dummy,DDummy) {
          cnt++;w4=(1.0-r_1)*r_2; *weight+=w4; *sum+=*pval4*w4;
          } /* if NODUMMY */
        pval3 = NEXTCOLROW(pval1,Dim_1,Dim_2);
        if NODUMMY(*pval3,Dummy,DDummy) {
          cnt++;w3=r_1*r_2; *weight+=w3; *sum+=*pval3*w3;
          } /* if NODUMMY */
        } /* (r_2<eps) */
      } /* if (r_1 ... */
   else if ( (i_1>=-1) && (i_2>=-1) && (i_1<Dim_1) && (i_2<Dim_2) ) {
    /* some of the 4 points are inside the array */
    if (r_1<eps) {
      if (r_2<eps) {
        if ( (i_1>=0) && (i_2>=0) ) /* pval1 OK? */
          if NODUMMY(*pval1,Dummy,DDummy) {
            cnt++; *weight += 1.0; *sum = *pval1; /* no interpolation */
            } /* if NODUMMY */

        } else {
        if ( (i_1>=0) && (i_2>=0) ) /* pval1 OK? */
          if NODUMMY(*pval1,Dummy,DDummy) {
            cnt++;w1=1.0-r_2; *weight+=w1; *sum+=*pval1*w1;
            } /* if NODUMMY */

        if ( (i_1>=0) && (i_2<Dim_2-1) ) { 
          pval4 = NEXTROW(pval1,Dim_1,Dim_2);
          if NODUMMY(*pval4,Dummy,DDummy) {
            cnt++;w4=r_2; *weight+=w4; *sum+=*pval4*w4;
            } /* if NODUMMY */
          } /* pval4 */

        } /* if (r_2<eps) */
      } else {
      if (r_2<eps) {
        if ( (i_1>=0) && (i_2>=0) ) /* pval1 OK? */
          if NODUMMY(*pval1,Dummy,DDummy) {
            cnt++; w1=(1.0-r_1); *weight+=w1; *sum = *pval1*w1;
            } /* if NODUMMY */

        if ( (i_2>=0) && (i_1<Dim_1-1) ) { 
          pval2 = NEXTCOL(pval1,Dim_1,Dim_2);
          if NODUMMY(*pval2,Dummy,DDummy) {
            cnt++; w2=r_1; *weight+=w2; *sum+=*pval2*w2;
            } /* if NODUMMY */
          } /* pval2 */

        } else {
        if ( (i_1>=0) && (i_2>=0) ) /* pval1 OK? */
          if NODUMMY(*pval1,Dummy,DDummy) {
            cnt++;w1=(1.0-r_1)*(1.0-r_2); *weight+=w1; *sum+=*pval1*w1;
            } /* if NODUMMY */

        if ( (i_2>=0) && (i_1<Dim_1-1) ) {
          pval2 = NEXTCOL(pval1,Dim_1,Dim_2);
          if NODUMMY(*pval2,Dummy,DDummy) {
            cnt++;w2=r_1*(1.0-r_2); *weight+=w2; *sum+=*pval2*w2;
            } /* if NODUMMY */
          } /* pval2 */

        if ( (i_1>=0) && (i_2<Dim_2-1) ) { 
          pval4 = NEXTROW(pval1,Dim_1,Dim_2);
          if NODUMMY(*pval4,Dummy,DDummy) {
            cnt++;w4=(1.0-r_1)*r_2; *weight+=w4; *sum+=*pval4*w4;
            } /* if NODUMMY */
          } /* pval4 */

        if ( (i_1<Dim_1-1) && (i_2<Dim_2-1) ) { 
          pval3 = NEXTCOLROW(pval1,Dim_1,Dim_2);
          if NODUMMY(*pval3,Dummy,DDummy) {
            cnt++;w3=r_1*r_2; *weight+=w3; *sum+=*pval3*w3;
            } /* if NODUMMY */
          } /* pval3 */

        } /* (r_2<eps) */
      } /* if (r_1 ... */
    }

  // reject pixels with less than IPolMin coverage
  if ( IPolMin > fabs(*weight) ) cnt=0;

  return(cnt);

} /* Ipol2ldw */

/*---------------------------------------------------------------------------
NAME

  Ipol2ld --- Interpolation including dummies with limit checks

PURPOSE
  Linear two dimensional interpolation including dummies with limit checks
  Ipol2ld returns the interpolated value. To calculate the sum and the weight
  of the interpolated pixels use Ipol2ldw.

METHOD
  See Ipol2ldw. Ipol2ld calculates only the interpolated value and returns
  the number of interpolated pixels.
---------------------------------------------------------------------------*/
int Ipol2ld (float *Data, int Dim_1, int Dim_2, float Dummy, float DDummy,
             float f_1, float f_2, float *value) 
{ float weight;
  int cnt;

  cnt = Ipol2ldw(Data, Dim_1, Dim_2, Dummy, DDummy,f_1, f_2, value, &weight);
  if (cnt) *value = *value/weight;

  return ( cnt );

} /* Ipol2ld */

int Ipol2d (float *Data, int Dim_1, int Dim_2, float Dummy, float DDummy,
            float f_1, float f_2, float *value)
{  return(Ipol2ld (Data, Dim_1, Dim_2, Dummy, DDummy,
            f_1, f_2, value));
} /* Ipol2d */

/*---------------------------------------------------------------------------
NAME

  Ipol2 --- Two dimensional interpolation, no limit checks, no dummy checks

SYNOPSIS
  void Ipol2 (float *Data, int Dim_1, int Dim_2, 
             float f_1, float f_2, float *value)

PURPOSE
  Linear two dimensional interpolation 
  no limit checks,
  no dummy checks

METHOD
  The point (f_1, f_2) is linearly interpolated between the 4 closest
  program array elements val1=Data[i_1,i_2], val2=Data[i_1+1,I_2],
  val3=Data[i_1+1,i_2+1] and val4=Data[I_1,i_2+1]. If a float index
  is an integer number 0, 1, 2, 3 etc. no interpolation is done in the
  corresponding direction.

ARGUMENTS

  return value            (o)   : number of pixels contributing to the output
                                  value (0..4), if 0, only dummies found.
  float Data[Dim_1,Dim_2] (i)   : input array
  int Dim_1, Dim_2        (i)   : dimension of array
  float f_1, f_2          (i)   : program array indices (interpolation point)
  float *value            (o)   : interpolated value

2 HISTORY
  28-Apr-1995 PB from Ipol2d

---------------------------------------------------------------------------*/
void Ipol2 (float *Data, int Dim_1, int Dim_2,
            float f_1, float f_2, float *value)
{ const float eps = IPOLEPS;
  int cnt;
  float *pval1, *pval2, *pval3, *pval4;
  float w1, w2, w3, w4;
  float weight;

  int   i_1, i_2;
  float r_1, r_2;

  /* calculate integer indices and rest */
  IDX(f_1,i_1,r_1);
  IDX(f_2,i_2,r_2);

  weight=0.0;
  *value=0.0;
  cnt=0;

  if (!Data) return; // return, if NULL pointer

  pval1 = ABSPTR(Data,Dim_1,Dim_2,i_1,i_2);

  if (r_1<eps) {
     if (r_2<eps) {
         cnt++; *value = *pval1;  /* no interpolation */
       } else {
         cnt++;w1=1.0-r_2; weight+=w1; *value+=*pval1*w1;
       pval4 = NEXTROW(pval1,Dim_1,Dim_2);
         cnt++;w4=r_2; weight+=w4; *value+=*pval4*w4;
       if (cnt) *value = *value/weight;
       } /* if (r_2<eps) */
     } else {
     if (r_2<eps) {
         cnt++; w1=(1-r_1); weight+=w1; *value = *pval1*w1;
       pval2 = NEXTCOL(pval1,Dim_1,Dim_2);
         cnt++; w2=r_1; weight+=w2; *value+=*pval2*w2;
       if (cnt) *value = *value/weight;
       } else {
         cnt++;w1=(1.0-r_1)*(1.0-r_2); weight+=w1; *value+=*pval1*w1;
       pval2 = NEXTCOL(pval1,Dim_1,Dim_2);
         cnt++;w2=r_1*(1.0-r_2); weight+=w2; *value+=*pval2*w2;
       pval4 = NEXTROW(pval1,Dim_1,Dim_2);
         cnt++;w4=(1.0-r_1)*r_2; weight+=w4; *value+=*pval4*w4;
       pval3 = NEXTCOLROW(pval1,Dim_1,Dim_2);
         cnt++;w3=r_1*r_2; weight+=w3; *value+=*pval3*w3;
       if (cnt) *value = *value/weight;
       } /* (r_2<eps) */
     } /* if (r_1 ... */

} /* Ipol2 */

/*---------------------------------------------------------------------------
NAME

  IpolRebin2 --- Two dimensional rebinning of a float array.

SYNOPSIS
  void IpolRebin2 ( float *Data, int Dim_1, int Dim_2,
                    float * DataOut, int * OutDim_1, int * OutDim_2,
                    float Dummy, float DDummy, int Bin_1, int Bin_2, int average )


PURPOSE
  Two dimensional rebinning of a float array. The result is written to the
  output array. The new dimensions are returned.

ARGUMENTS

  float Data[Dim_1,Dim_2]    (i) : input array with dimension Dim_1, Dim_2
  int Dim_1, Dim_2           (i) : dimensions of input array
  float *DataOut             (o) : output array (needs to be allocated!)
  int *pOutDim_1, *pOutDim_2 (o) : pointer to output dimensions 
                                   (need to be allocated!)
  float Dummy, DDummy        (i) : Dummy values of input and output arrays 
  int Bin_1, Bin_2           (i) : Binning factors (>=1) 
  int Average                (i) : flag: sum (0) or average (!0) pixel values

  Data and DataOut can be identical. If they are not identical, they must not
  overlap. The output variables *pOutDim_1 and *pOutDim_2 must have been 
  allocated. They can be identical to Dim_1 and Dim_2.

HISTORY
  2007-02-26 PB from RebinFloat2d

---------------------------------------------------------------------------*/
void IpolRebin2 ( float *Data, int Dim_1, int Dim_2,
                  float * DataOut, int * pOutDim_1, int * pOutDim_2,
                  float Dummy, float DDummy, int Bin_1, int Bin_2, int Average )
{ register long int j_1, j_2, i_1, i_2;
  float * pin, * pout;
  float value, sum, count;
  int bin_1, bin_2;
  int dim_1, dim_2;

  dim_1 = Dim_1;
  dim_2 = Dim_2;

  if (pOutDim_1!=NULL) *pOutDim_1=dim_1;
  if (pOutDim_2!=NULL) *pOutDim_2=dim_2;

  bin_1 = Bin_1>1?Bin_1:1; // max(1,bin_1)
  bin_2 = Bin_2>1?Bin_2:1; // max(1,bin_2)

  if ((bin_1>1) || (bin_2>1) || (DataOut!=Data)) {
    pout = DataOut;
    if (pout!=NULL) {
      for (j_2=0;j_2<=dim_2-bin_2;j_2+=bin_2)
        for (j_1=0;j_1<=dim_1-bin_1;j_1+=bin_1) {
          sum = 0.0; count = 0.0;
          for (i_2=j_2;i_2<j_2+bin_2;i_2++) {
            pin = ABSPTR(Data,dim_1,dim_2,j_1,i_2);
            for (i_1=j_1;i_1<j_1+bin_1;i_1++) {
              value = *(pin++);
              if NODUMMY(value,Dummy,DDummy)
                { sum += value; count += 1.0; }
            }
          }
          if (count>0.0) *(pout++) = Average?sum/count:sum;
          else *(pout++) = Dummy;
        }
      dim_1 /= bin_1; dim_2 /= bin_2;
      if (pOutDim_1!=NULL) *pOutDim_1=dim_1;
      if (pOutDim_2!=NULL) *pOutDim_2=dim_2;
    }
  }

} /* IpolRebin2 */


/*****************************************************************************/

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

   reference --- macros for transformations between reference systems

PURPOSE

  Definition of transformation macros and procedures between reference systems.
  All macros may contain several instructions separated by a semicolon. If
  necessary, e.g. in an if-statement, they MUST be put into parentheses:
  if (condition) {ARRAYREF(Off,Ps);}

HISTORY

  2009-10-02 PB V2.31 extracted from SaxsDefinitions.h  V2.30
  2010-03-18 PB V2.32 Reference systems and projections (from SaxsOptions.h)
  2011-04-07 PB V2.33 CSWAP2, OSWAP2, RASREG added.
  2011-05-14 PB V2.34 Axis types (from SaxsOptions.h)
  2011-07-24 PB V2.35 Reference system Tangens added (IO_Tangens)

DESCRIPTION

----------------------------------------------------------------------------*/

/****************************************************************************
* reference.h
****************************************************************************/

#ifndef _REFERENCE_
# define REFERENCE_VERSION "reference : V2.34 Peter Boesecke 2011-05-14"

/* ---------------------------------------------------------------------------
  General Constants
  - TRUE  1,  - FALSE 0
---------------------------------------------------------------------------*/
# ifndef FALSE
#   define FALSE 0
#   define TRUE  1
# endif

/*---------------------------------------------------------------------------
 The following macro calculates the absolute value of the argument
 Usage:
 absolut = ABS(-3.2);
---------------------------------------------------------------------------*/
# define ABS( x) ( ( x)> 0 ? ( x) : -( x) )

/*---------------------------------------------------------------------------
 The following macros calculate the maximum and minimum of the arguments
 Usage:
 minimum = MIN2(-3.2, 5.7);
 maximum = MAX2( 3.2, 5.9);
 minimum = MIN3( 2.2, 4.7, 0.3 );
 maximum = MAX3( 1.0, 3.0, -3.54 );
 minimum = MIN4( 2.2, 4.7, 0.3 , 4.5 );
 maximum = MAX4( 1.0, 3.0, -3.54 , 8.9 );
---------------------------------------------------------------------------*/
#define MAX2( x1, x2) ( ( x1)>( x2) ? ( x1) : ( x2) )
#define MIN2( n1, n2) ( ( n1)<( n2) ? ( n1) : ( n2) )
#define MAX3( y1, y2, y3) ( MAX2( MAX2( ( y1), ( y2) ), ( y3) ) )
#define MIN3( y1, y2, y3) ( MIN2( MIN2( ( y1), ( y2) ), ( y3) ) )
#define MAX4( z1, z2, z3, z4 ) MAX2( MAX2( ( z1), ( z2)), MAX2( ( z3), ( z4)) )
#define MIN4( z1, z2, z3, z4 ) MIN2( MIN2( ( z1), ( z2)), MIN2( ( z3), ( z4)) )

/*---------------------------------------------------------------------------
 Conventions
 -----------
  pixel index      : Machine dependent numbering of pixels inside an array.
                     The machine dependent numbering starts with INDEXSTART.
  pixel number     : Machine independent numbering of a pixel.
                     The machine indedpendend numbering starts with NUMBERSTART.
  pixel coordinate : Machine independent coordinate of a pixel array.
                     ARRAYSTART is the pixel coordinate of the
                     center of the pixel with the index INDEXSTART.
  world coordinate : Machine independent coordinate of a pixel array. The
                     actually used world coordinates are chosen with the
                     reference system. World coordinates are defined as
                     affine transformations of pixel coordinates:
                     world coordinate = (pixel coordinate + OFFSET) * Ps

  A pixel is covering the interval [pixel number - 0.5 .. pixel number + 0.5].
  Binning does not change world coordinates.

 Coordinate Systems
 ------------------
 The positioning of several pixel arrays to each other is chosen by the
 reference system. All basic lengths (pixel size, wave length,
 sample distance etc.) are given in meters. Four different systems are used:

 ARRAY   coordinate = pixel coordinate
 IMAGE   coordinate = array coordinate + offset
 CENTER  coordinate = image coordinate - center
 REAL    coordinate = image coordinate * pixel size
 NORMAL  coordinate = (image coordinate - center) * pixel size
 TANGENS coordinate = (image coordinate - center) *
                       (pixel size/sample distance)
 SAXS    coordinate = (image coordinate - center) *
                       (pixel size/sample distance) * (WaveLength0/wave length)

 The principal coordinate system is IMAGE.

 REFERENCE system : coordinate system that is used for calculations
 USER system      : coordinate system that is used for keyboard I/O by the
                    user.

 Description of macros
 ---------------------
 Only the following macros and constants should be used for transformations
 between the different coordinate systems.

 INDEXSTART  = lowest pixel index (machine dependent)
 NUMBERSTART = lowest pixel number (machine independent)
 ARRAYSTART  = pixel coordinate of center of pixel INDEXSTART
 LOWERBORDER = distance between pixel center and its lower border
 DAI         = ARRAYSTART - INDEXSTART = Difference (ARRAYSTART, INDEXSTART)
 WaveLength0 = reference wavelength in meters used in SAXS reference system

 WAVENUMBER
  Calculates the wavenumber as it is used for reference system coordinates

 Example
  Lower left corner has coordinates (0.5,0.5):
   INDEXSTART 0, ARRAYSTART 1.0, LOWERBORDER (-0.5), DAI 1.0
  Lower left corner has coordinates (0.0,0.0):
   INDEXSTART 0, ARRAYSTART 0.5, LOWERBORDER (-0.5), DAI 0.5 

 The world coordinates of the array boundaries are:
  lower boundary = WORLD(INDEXSTART+LOWERBORDER,Off,Ps);
  upper boundary = WORLD(INDEXSTART+DIM+LOWERBORDER,Off,Ps);

 The world coordinates of the first and the last column (row) of an array are:
  lower column = WORLD(INDEXSTART,Off,Ps);
  upper column = WORLD(INDEXSTART+DIM-1,Off,Ps);

 ARRAYREF, IMAGEREF, CENTERREF, REALREF, NORMALREF, SAXSREF, WORLD, INDEX
  The preprocessor macros ARRAYREF, IMAGEREF, CENTERREF, REALREF, NORMALREF,
  TANGENSREF and SAXSREF calculate offsets and pixel sizes to allow a direct 
  affin transformation between pixel indices and world coordinates. The 
  transformations are done with WORLD and INDEX. The calculated internal 
  offsets (Off) and pixel sizes (Ps) must not be confused with the basic 
  offsets and pixel sizes which are given in pixel coordinates and meters.

 Usage (transformation of coordinates) :
  float Off, Ps;
  float Offset, PSize, Center, SampleDistance, WaveLength;
  float IIndex;
  float WArray, WImage, WCenter, WReal, WNormal, WSaxs;
  {ARRAYREF(Off,Ps);}
  WArray = WORLD( IIndex, Off, Ps );
  IIndex = INDEX( WArray, Off, Ps );
  {IMAGEREF(Off,Ps,Offset);}
  WImage = WORLD( IIndex, Off, Ps );
  IIndex = INDEX( WImage, Off, Ps );
  {CENTERREF(Off,Ps,Offset,Center);}
  WCenter = WORLD( IIndex, Off, Ps );
  IIndex = INDEX( WCenter, Off, Ps );
  {REALREF(Off,Ps,Offset,PSize);}
  WReal = WORLD( IIndex, Off, Ps );
  IIndex = INDEX( WReal, Off, Ps );
  {NORMALREF(Off,Ps,Offset,PSize,Center);}
  WNormal = WORLD( IIndex, Off, Ps );
  IIndex = INDEX( WNormal, Off, Ps );
  {TANGENSREF(Off,Ps,Offset,PSize,Center,SampleDistance);}
  WTangens = WORLD( IIndex, Off, Ps );
  IIndex = INDEX( WSaxs, Off, Ps );
  {SAXSREF(Off,Ps,Offset,PSize,Center,SampleDistance,WaveLength);}
  WSaxs = WORLD( IIndex, Off, Ps );
  IIndex = INDEX( WSaxs, Off, Ps );

 A2INDEX, I2INDEX, C2INDEX, R2INDEX, N2INDEX, T2INDEX, S2INDEX, 
  INDEX2A, INDEX2I, INDEX2C, INDEX2R, INDEX2N, INDEX2T, INDEX2S
  The preprocessor macros A2INDEX, I2INDEX, C2INDEX, R2INDEX, N2INDEX, 
  T2INDEX and S2INDEX transform a world coordinate W directly into a pixel 
  index IIndex. INDEX2A, INDEX2I, INDEX2C, INDEX2R, INDEX2N, INDEX2T and 
  INDEX2S transform a pixel index directly into a world coordinate W.

 Usage (direct transformation of coordinates) :
  float Offset, PSize, Center, SampleDistance, WaveLength;
  float IIndex;
  float WArray, WImage, WCenter, WReal, WNormal, WSaxs;
  IIndex   = A2INDEX(WArray);
  IIndex   = I2INDEX(WImage,Offset);
  IIndex   = C2INDEX(WCenter,Offset,Center);
  IIndex   = R2INDEX(WReal,Offset,PSize);
  IIndex   = N2INDEX(WNormal,Offset,PSize,Center);
  IIndex   = T2INDEX(WSaxs,Offset,PSize,Center,SampleDistance);
  IIndex   = S2INDEX(WSaxs,Offset,PSize,Center,SampleDistance,WaveLength);
  WArray   = INDEX2A(IIndex);
  WImage   = INDEX2I(IIndex,Offset);
  WCenter  = INDEX2C(IIndex,Offset,Center);
  WReal    = INDEX2R(IIndex,Offset,PSize);
  WNormal  = INDEX2N(IIndex,Offset,PSize,Center);
  WTangens = INDEX2S(IIndex,Offset,PSize,Center,SampleDistance);
  WSaxs    = INDEX2S(IIndex,Offset,PSize,Center,SampleDistance,WaveLength);

 REF2USER, USER2REF
  Transformation between a user system coordinate and
  a reference system coordinate

 Usage (transformation between user system and reference system ) :
  float ROff, RPs, UOff, UPs;
  float Offset, PSize, Center, SampleDistance, WaveLength;
  float RW, UW;

  {IMAGEREF(UOff,UPs,Offset);}
  {SAXSREF(ROff,RPs,Offset,PSize,Center,SampleDistance,WaveLength);}
  UW = REF2USER(RW,ROff,RPs,UOff,UPs);
  RW = USER2REF(UW,ROff,RPs,UOff,UPs);

  etc.

 AREBIN
  Calculation of new offset, bin size, pixel size and center coordinate 
  after a binning by factor Bin

 Usage (binning) :
  AREBIN(Offset,BSize,PSize,Center,Bin)

 I2OFFSET, C2OFFSET, R2OFFSET, N2OFFSET, T2OFFSET, S2OFFSET
  Calculation of the offset value. The input value must be the coordinate 
  of the lower edge of the region or pixel (Image, Center, Real, 
  Normal and Saxs). An offset cannot be calculated for Array.
  
 Usage : 
  Offset = I2OFFSET(WImage);
  Offset = C2OFFSET(WNormal,Center);
  Offset = R2OFFSET(WReal,PSize);
  Offset = N2OFFSET(WNormal,PSize,Center);
  Offset = T2OFFSET(WSaxs,PSize,Center,SampleDistance);
  Offset = S2OFFSET(WSaxs,PSize,Center,SampleDistance,WaveLength);

 R2PSIZE, N2PSIZE, T2PSIZE, S2PSIZE
  Calculation of the pixel size from a distance in Real coordinates,
  Normal coordinates and Saxs coordinates.

 Usage :
  PSize = R2PSIZE(WRealDistance);
  PSize = N2PSIZE(WNormalDistance);
  PSize = T2PSIZE(WSaxsDistance,SampleDistance);
  PSize = S2PSIZE(WSaxsDistance,SampleDistance,WaveLength);

 R2CENTER
  Calculation of center from a Real coordinate

 Usage :
  Center = R2CENTER(WReal,PSize)

 RASREG 
  Calculation of the raster region. RasReg is the outer dimension 
  of a region, i.e. the dimension of a region with binning size 1 
  and offset 0 that covers the region exactly.

 Usage :
  RasReg = RASREG(BSize,Offset,Dim)

 OSWAP2
  Calculation of a new offset value after a swap of a data REGION.
  The new offset is calculated in such a way that REGIONs are swapped
  coherently. 

 Usage :
  Offset' = OSWAP2(RasReg,BSize,Offset,Dim)

 CSWAP2
  Calculation of a new center coordinate after a swap of the data ARRAY. 
  The new center coordinate is calculated in such a way that it points to 
  the same pixel as before the swap. PSize and Dim do not change
  when the array is swapped, the Offset can be changed.

 Usage :
  Center' =  CSWAP2(Offset',Center,Offset,Dim)

 CSWAP
  Like CSWAP2 but keeping the offset.

 Usage :
  Center' = CSWAP(Center,Offset,Dim);

 N2S
  Transformation of a Normal coordinate to a Saxs coordinate

 Usage :
  WSaxs = N2S(WNormal,SampleDistance,WaveLength);

----------------------------------------------------------------------------*/
/* reference systems */
# define IO_NoRSys 0
# define IO_Array 1
# define IO_Image 2
# define IO_Center 3
# define IO_Region 4
# define IO_Real 5
# define IO_Normal 6
# define IO_Tangens 7
# define IO_Saxs 8
/* projections */
# define IO_NoPro 0
# define IO_ProSaxs 1
# define IO_ProWaxs 2
/* axis types */
# define IO_NoAxisType 0
# define IO_AxisTypeDistance 1
# define IO_AxisTypeAngle 2
# define IO_AxisTypeNumerator 3
/* array specifications */
# define INDEXSTART  0
# define NUMBERSTART 1
// define ARRAYSTART  1.0 +++++++++++++++++++
# define ARRAYSTART  0.5 
# define LOWERBORDER (-0.5)
// define DAI         1.0 +++++++++++++++++++
# define DAI         0.5 
# define WaveLength0 1e-9
# define WAVENUMBER(W) (WaveLength0/(W)) 
/* transformation of coordinates*/
# define WORLD( I, Off, Ps) (((I)+(Off))*(Ps))
# define INDEX( W, Off, Ps) (((W)/(Ps))-(Off))
# define ARRAYREF(Off,Ps) Off=DAI; Ps=1.0
# define IMAGEREF(Off,Ps,O) Off=(O)+DAI; Ps=1.0
# define CENTERREF(Off,Ps,O,C) Off=(O)-(C)+DAI; Ps=1.0;
# define REALREF(Off,Ps,O,P) Off=(O)+DAI; Ps=(P)
# define NORMALREF(Off,Ps,O,P,C) Off=(O)-(C)+DAI; Ps=(P);
# define TANGENSREF(Off,Ps,O,P,C,S) NORMALREF(Off,Ps,(O),((P)/(S)),(C))
# define SAXSREF(Off,Ps,O,P,C,S,W) NORMALREF(Off,Ps,(O),((P)/(S))*WAVENUMBER(W),(C))
/* direct transformations */
# define A2INDEX(I) ((I)-DAI)
# define I2INDEX(I,O) ((I)-(O)-DAI)
# define C2INDEX(I,O,C) I2INDEX(I,(O)-(C))
# define R2INDEX(I,O,P) ((((I)/(P))-(O))-DAI)
# define N2INDEX(I,O,P,C) R2INDEX((I),(O)-(C),(P))
# define T2INDEX(I,O,P,C,S) N2INDEX((I),(O),((P)/(S)),(C))
# define S2INDEX(I,O,P,C,S,W) N2INDEX((I),(O),((P)/(S))*WAVENUMBER(W),(C))
# define INDEX2A(I) ((I)+DAI)
# define INDEX2I(I,O) ((I)+(O)+DAI)
# define INDEX2C(I,O,C) INDEX2I(I,(O)-(C))
# define INDEX2R(I,O,P) (((I)+(O)+DAI)*(P))
# define INDEX2N(I,O,P,C) INDEX2R((I),(O)-(C),(P))
# define INDEX2T(I,O,P,C,S) INDEX2N((I),(O),((P)/(S)),(C))
# define INDEX2S(I,O,P,C,S,W) INDEX2N((I),(O),((P)/(S))*WAVENUMBER(W),(C))
/* transformation of coordinates between user system and reference system */
# define REF2USER(RW,ROff,RPs,UOff,UPs) WORLD(INDEX(RW,ROff,RPs),UOff,UPs)
# define USER2REF(UW,ROff,RPs,UOff,UPs) WORLD(INDEX(UW,UOff,UPs),ROff,RPs)
/* transformation of distances between user system and reference system */
# define DREF2DUSER(DRW,RPs,UPs) ((DRW) * ((UPs)/(RPs)))
# define DUSER2DREF(DUW,RPs,UPs) ((DUW) * ((RPs)/(UPs)))
/* binning */
# define AREBIN(O,B,P,C,Bin) \
          O=( ((ARRAYSTART+LOWERBORDER)*(1.0-MAX2(1,Bin))+(O))/MAX2(1,Bin) ); \
          B=( (B)*MAX2(1,Bin) ); P=( (P)*MAX2(1,Bin) ); C=( (C)/MAX2(1,Bin) )
/* calculation of Offset */
# define I2OFFSET(I) ((I) - (ARRAYSTART + LOWERBORDER) ) 
# define C2OFFSET(I,C) ((I) + (C) - (ARRAYSTART + LOWERBORDER) )
# define R2OFFSET(I,P) ((I)/(P) - (ARRAYSTART + LOWERBORDER) )
# define N2OFFSET(I,P,C) (((I)/(P)) + (C) - (ARRAYSTART + LOWERBORDER) )
# define T2OFFSET(I,P,C,S) \
          ( (((I)/(P))*(S)) + (C) - (ARRAYSTART + LOWERBORDER) )
# define S2OFFSET(I,P,C,S,W) \
          ( (((I)/(P))*(S)*WAVENUMBER(W)) + (C) - (ARRAYSTART + LOWERBORDER) )
/* calculation of PSize */
# define R2PSIZE(D) (D)
# define N2PSIZE(D) R2PSIZE(D)
# define T2PSIZE(D,S) ( ((D) * (S)) )
# define S2PSIZE(D,S,W) ( ((D) * (S)) / WAVENUMBER(W) )
# define PSIZE2R(P) (P)
# define PSIZE2N(P) PSIZE2R(P)
# define PSIZE2T(P,S) ( ((P) / (S)) )
# define PSIZE2S(P,S,W) ( ((P) / (S)) * WAVENUMBER(W) )
/* calculation of Center */
# define R2CENTER(I,P) ((I)/(P))
# define CENTER2R(I,P) ((I)*(P))
/* calculation of the raster region from the full image without offset */
# define RASREG(B,D)  ( INDEX2R((INDEXSTART)+(LOWERBORDER)+(D),0,B) )
/* calculation of new offset and center after swapping the data array */
# define OSWAP2(R,B,O,D)   ( ( (R) / (B) ) - ( (O) + (D) ) )
# define CSWAP2(OO,C,O,D)  ( (OO) + (O) + (D) - (C) )
/* calculation of new center after swapping the data array and keeping offset */
# define CSWAP(C,O,D) ( CSWAP2(O,C,O,D) )
/* direct transformations between reference systems */
# define N2S(I,S,W) (((I)/(S))*WAVENUMBER(W))
# define S2N(I,S,W) (((I)*(S))/WAVENUMBER(W))

/*-------------------------------------------------------------------------
Center Value Definitions
SETCTRDEF ( Dim ) calculation of a default center point to dimension Dim
Usage:
Center_1 = SETCTRDEF( Dim[1] );
Center_2 = SETCTRDEF( Dim[2] );
---------------------------------------------------------------------------*/
//# define SETCTRDEF(D ) ( ( (float) (D ) + DAI ) * 0.5 )
# define SETCTRDEF(D ) ( ( (float) (D ) - 1 ) * 0.5 + ARRAYSTART )
/*-------------------------------------------------------------------------*/

# define _REFERENCE_ 
#endif /* _REFERENCE_ */

/****************************************************************************
*                                                                           *
****************************************************************************/


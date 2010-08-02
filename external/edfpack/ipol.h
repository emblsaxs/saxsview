/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 2001-2010 European Synchrotron Radiation Facility
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

/*+++***********************************************************************
NAME

    ipol.h

SYNOPSIS

    #include "ipol.h"

DESCRIPTION

    Header of the module "ipol.c"

    The header contains the following standard definitions:

    Array element access:    ABSPTR, NEXTCOL, NEXTROW, NEXTCOLROW
    Dummy value definitions: MinDDummy, AccDummy, VarDummy
    Dummy value macros:      DDSET, DUMMYDEFINED, NODUMMYDEFINED, 
                             DUMMY, NODUMMY, UPDATE 

    Functions for linear two dimensional interpolation/integration:

    Isum2ldwE  --- Area integral (+dummies, +limit checks, +weight, +variance) 
    Isum2ldw   --- Area integral (+dummies, +limit checks, +weight) 
    Ipol2ldw   --- Area interpolation (+dummies, +limit checks, +weight)
    Ipol2ld    --- Area interpolation (+dummies, +limit checks)
    Ipol2      --- Area interpolation (-dummies, -limit checks)

    Ipolmin --- set/return minimum accepted coverage ratio 
    Ipolmode --- set/return interpolation mode
    Ipolmode2str --- return interpolation mode string

    Ipolweight --- set/return weighting method
    Ipolweight2str --- return weighting method string

    IpolRebin2 --- rebinning of an array

***********************************************************************---*/
#ifndef _IPOL_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/

#ifndef PRIVATE
# define PRIVATE       static /* used to declare variables of private type */
# define PUBLIC                /* used to declare variables of public type */
#endif

# include <stdio.h>
# include <stdlib.h>
# include <limits.h>
# include <errno.h>
# include <stdio.h>
# include <fcntl.h>
# include <math.h>
# include <float.h>

/***************************************************************************
* MACROS                                                                   *
***************************************************************************/

/*---------------------------------------------------------------------------
 The constant IPOLEPS defines the interval around an array element in which
 no interpolation takes place.
---------------------------------------------------------------------------*/
# define IPOLEPS          1e-4

/*---------------------------------------------------------------------------
 The constants IPOL_NORMAL and IPOL_ANTIALIASED are used to set the 
 interpolation mode with the routine Ipolmod.
---------------------------------------------------------------------------*/
# define IPOL_NORMAL      1
# define IPOL_ANTIALIASED 2

/*---------------------------------------------------------------------------
 The constants IPOL_EQUAL and IPOL_WEIGHTED are used to set the
 weight mode with the routine Ipolweight
---------------------------------------------------------------------------*/
# define IPOL_EQUAL       4
# define IPOL_WEIGHTED    8

/*---------------------------------------------------------------------------
 Some internal macros
---------------------------------------------------------------------------*/
# define IPOL_ABS( x) ( ( x)> 0 ? ( x) : -( x) )
# define IPOL_MAX2( x1, x2) ( ( x1)>( x2) ? ( x1) : ( x2) )
# define IPOL_MIN2( n1, n2) ( ( n1)<( n2) ? ( n1) : ( n2) )

/*---------------------------------------------------------------------------
 IDX
  Calculation of the integer part and the rest of a float pixel index.
  IDX calculates to the float value f the closest integer number I less
  than f and the difference R = f - (float) I.
 Usage :
  float f; int I; float R;
  IDX( f, I, R );
---------------------------------------------------------------------------*/
#define IDX(f,I,R) ((f)>=0)?((I)=(int)(f)):((I)=((int)(f))-1);(R)=(f)-(float)(I)

/*---------------------------------------------------------------------------
 The following macros calcuate the pointers to array elements.
 - ABSPTR    calculates the pointer to the array element (column=i_1, row=i_2)
             from the base Data of the array.
 The next three macros are used for interpolation between neighbouring data
 point.
 -NEXTCOL    returns the pointer to the element in the next column
             of the same row
 -NEXTROW    returns the pointer to the element in the next row
             of the same column
 -NEXTCOLROW returns the pointer to the element in the next column
             of the next row.
 Usage:
 <dtype> Data[Dim_1,Dim_2] : <dtype> array of size Dim_1,Dim_2;
 int i_1,i_2 : array indices
 <dtype> *pdata, *pvalnxtcoli, *pvalnxtrow, *pvalnxtcolrow;
 pdata         = ABSPTR(Data,Dim_1,Dim_2,i_1,i_2);
 pvalnxtcol    = NEXTCOL(pdata,Dim_1,Dim_2);
 pvalnxtrow    = NEXTROW(pdata,Dim_1,Dim_2);
 pvalnxtcolrow = NEXTCOLROW(pdata,Dim_1,Dim_2);
---------------------------------------------------------------------------*/
#define ABSPTR(A,D1,D2,I1,I2) (A)+((I1)+((I2)*(D1)))
#define NEXTCOL(pA,D1,D2) (pA)+1
#define NEXTROW(pA,D1,D2) (pA)+(D1)
#define NEXTCOLROW(pA,D1,D2) (pA)+(1+(D1))

/*----------------------------------------------------------------------------
Dummy Value Definitions
MinDDummy minimum possible value for DDummy
AccDummy relative accuracy
VarDummy unchangeable dummy value of variance array
DDSET(Dummy) calculation of a good DDummy value to Dummy
DUMMY(Value, Dummy, DDummy) TRUE if value is a Dummy
NODUMMY(Value, Dummy, DDummy) TRUE if value is not a Dummy
UPDATE(Destination,Value,Dummy,DDummy) adds Value to Destination if not Dummy
Usage:
DDummy = DDSET(Dummy);
if DUMMYDEFINED (Dummy, DDummy) ...
if DUMMY(Value, Dummy, DDummy) { do something } else { do something else };
if NODUMMY(Value, Dummy, DDummy) { do something } else { do something else };
UPDATE( Destination, Value, Dummy, DDummy);
---------------------------------------------------------------------------*/
# define MinDDummy 0.1
# define AccDummy  1e-5
# define VarDummy  -1.0
# define DDSET(D) IPOL_MAX2(MinDDummy,IPOL_ABS(D)*AccDummy)
# define DUMMYDEFINED( D, DD) ((IPOL_ABS(D)>IPOL_MAX2(DD,MinDDummy)) ? 1 : 0)
# define NODUMMYDEFINED(D, DD) ((IPOL_ABS(D)>IPOL_MAX2(DD,MinDDummy)) ? 0 : 1)
# define DUMMY( V, D, DD) ((IPOL_ABS((V)-(D))<=DD)&&DUMMYDEFINED(D,DD))
# define NODUMMY( V, D, DD) ((IPOL_ABS((V)-(D))>DD)||NODUMMYDEFINED(D,DD))
# define UPDATE( DE, V, D, DD) if (DUMMY(DE,(D),(DD))) DE=(V); else DE+=(V)

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern void 
    Ipol2                  ( float *Data, int Dim_1, int Dim_2,
                             float f_1, float f_2, float *value),
    IpolRebin2             ( float *Data, int Dim_1, int Dim_2,
                             float *DataOut, int *pOutDim_1, int *pOutDim_2,
                             float Dummy, float DDummy,
                             int Bin_1, int Bin_2, int Average );
PUBLIC extern int 
    Isum2ldwE              ( float *Data, float *VarDat, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f1_1, float f1_2,  float f3_1, float f3_2,
                             float *sum, float *weight, 
                             float *varsum, float *varweight),
    Isum2ldw               ( float *Data, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f1_1, float f1_2,  float f3_1, float f3_2,
                             float *sum, float *weight),
    Ipol2ldw               ( float *Data, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f_1, float f_2, float *sum, float *weight),
    Ipol2ld                ( float *Data, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f_1, float f_2, float *value),
    Ipol2d                 ( float *Data, int Dim_1, int Dim_2,
                             float Dummy, float DDummy,
                             float f_1, float f_2, float *value);
PUBLIC extern float
    Ipolmin                ( float minimum );

PUBLIC extern int 
    Ipolmode               ( int mode ),
    Ipolweight             ( int method );

PUBLIC extern const char
    *Ipolmode2str          ( int mode ),
    *Ipolweight2str        ( int method );

# define _IPOL_
#endif /* _IPOL_ */

/****************************************************************************
*                                                                           *
****************************************************************************/

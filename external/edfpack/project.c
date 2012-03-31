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

/*+++***********************************************************************
NAME

   project.c --- Projection functions

SYNOPSIS

    void project_1         ( float *line, float *varline, int dim,
                             int imin, int imax, float initvalue, float factor,
                             float *data, float *vardat, int dim_1, int dim_2,
                             float f1_1, float f3_1, float Df_1,
                             float f1_2, float f3_2,
                             float dummy, float ddummy, int ave )

    void project_2         ( float *line, float *varline, int dim,
                             int imin, int imax, float initvalue, float factor,
                             float *data, float *vardat, int dim_1, int dim_2,
                             float f1_1, float f3_1,
                             float f1_2, float f3_2, float Df_2,
                             float dummy, float ddummy, int ave );

HISTORY
  2009-10-06 V1.00 PB extracted from SaxsRoutine (Projection_1, Projection_2)
                      without testbit
  2009-11-10 V1.01 PB unused variables removed

***********************************************************************---*/

/****************************************************************************
*  Include                                                                  *
****************************************************************************/

# include "project.h"

/***************************************************************************
* Functions                                                                *
***************************************************************************/

/*---------------------------------------------------------------------------
NAME
 
   project_1 --- Projection of image rows to a line (including variance)
 
SYNOPSIS
 
    void project_1      ( float *line, float *varline, int dim,
                             int imin, int imax, float initvalue, float factor,
                             float *data, float *vardat, int dim_1, int dim_2,
                             float f1_1, float f3_1, float Df_1,
                             float f1_2, float f3_2,
                             float dummy, float ddummy, int ave )
 
DESCRIPTION
Averaging/Integration of rows between f1_2 and f3_2 in the 2d array data
(float data[dim_1,dim_2]). The result is written to the 1d array line.
The output array line is initialized with initvalue. Each element i between
imin and imax of line is filled with the average/integral of the rectangle
between the edges with the index coordinates (fi_1, f1_2) and (fi_1+Df_1, f3_2)
in the input data array, where fi_1 starts with f1_1 for imin and is 
incremented by Df_1 for each element.
 
f1_1, f3_1, f1_2 and f3_2 are index coordinates of the input data array.

ARGUMENTS
float *line          : 1d output array float line[dim]
float *varline       : 1d output variance array float varline[dim]
long  dim            : dimensions of output arrays (dim=dim_1!)
long  imin, imax     : range of indices in output arrays
float initvalue      : all elements in line that could not be determined
                       are set to initvalue
                       (variance array values are set to VarDummy)
float factor         : multiplication factor for output values
float *data          : 2d input array float data[dim_1,dim_2]
float *vardat        : 2d input variance array float vardat[dim_1,dim_2]
long  dim_1, dim_2   : dimensions of the input array
float f1_1, f3_1     : lowest and highest index 1 in the input array
float Df_1           : increment of index coordinate 1
float f1_2, f3_2     : indices of integration limits
float dummy, ddummy  : dummy and ddummy of input array 
                       (negative values are dummy values of vardat)
int   ave            : 0: integration, 1: average
 
HISTORY
2005-09-17 Peter Boesecke
----------------------------------------------------------------------------*/
void project_1          ( float *line, float *varline, int dim,
                             int imin, int imax, float initvalue, float factor,
                             float *data, float *vardat, int dim_1, int dim_2,
                             float f1_1, float f3_1, float Df_1,
                             float f1_2, float f3_2,
                             float dummy, float ddummy, int ave )
{ const float eps = 1e-30;
  float sum, weight;
  float varsum, varweight;
  float f_1;
  long  i, i0, i1;    // loop
  int   cnt;
 
  i0 = MAX2(0,imin);
  i1 = MIN2(dim-1,imax);
 
  /* initialize line with initvalue */
  for (i=0;i<dim;i++) line[i] = initvalue;
  if (varline) for (i=0;i<dim;i++) varline[i] = VarDummy;
 
  f_1=f1_1; // start index in data
  for (i=i0; i<=i1; i++) {

    if ( varline ) {
      // line = project(data) * factor
      // varline = project(vardat) * factor^2
      cnt = Isum2ldwE(data,vardat,dim_1,dim_2,dummy,ddummy,
                      f_1, f1_2, f_1+Df_1, f3_2,
                      &sum, &weight, &varsum, &varweight);
      line[i] = sum*factor;
      varline[i] = varsum*factor*factor;

      if (ave) {
        if (fabs(weight)>eps) line[i] /= weight;
        else line[i] = initvalue;
        if (fabs(varweight)>eps) varline[i] /= varweight*varweight;
        else varline[i] = VarDummy;
      }

    } else {
      cnt = Isum2ldw ( data, dim_1, dim_2,
                       dummy, ddummy,
                       f_1, f1_2,  f_1+Df_1, f3_2,
                       &sum, &weight);
      line[i] = sum*factor;
      if (ave) {
        if (fabs(weight)>eps) line[i] /= weight;
        else line[i] = initvalue;
      }
    }
 
    f_1+=Df_1;
 
  } // for
 
} /* project_1 */

/*---------------------------------------------------------------------------
NAME
 
   project_2 --- Projection of image columns to a line (including variance)
 
SYNOPSIS

   void project_2       ( float *line, float *varline, int dim,
                             int imin, int imax, float initvalue, float factor,
                             float *data, float *vardat, int dim_1, int dim_2,
                             float f1_1, float f3_1,
                             float f1_2, float f3_2, float Df_2,
                             float dummy, float ddummy, int ave );
 
DESCRIPTION
Averaging/Integration of columns between f1_1 and f3_1 in the 2d array data
(float data[dim_1,dim_2]). The result is written to the 1d array line.
The output array line is initialized with initvalue. Each element i between
imin and imax of line is filled with the average/integral of the rectangle
between the edges with the index coordinates (f1_1, fi_2) and (f3_1, fi_2+Df_2)
in the input data array, where fi_2 starts with f1_2 for imin and is incremented
by Df_2 for each element.
 
f1_1, f3_1, f1_2 and f3_2 are index coordinates of the input data array.

ARGUMENTS
float *line          : 1d output array float line[dim]
float *varline       : 1d output variance array float varline[dim]
long  dim            : dimension of output array (dim=dim_1!)
long  imin, imax     : range of indices in output array
float initvalue      : all elements in line that could not be determined
                       are set to initvalue,
                       (variance array values are set to VarDummy)
float factor         : multiplication factor for output values
float *data          : 2d input array float data[dim_1,dim_2]
float *vardat        : 2d input variance array float vardat[dim_1,dim_2]
long  dim_1, dim_2   : dimensions of the input array
float f1_1, f3_1     : lowest and highest index 1 in the input array
float f1_2, f3_2     : indices of integration limits
float Df_2           : increment of index coordinate 2
float dummy, ddummy  : dummy and ddummy of input array 
                       (negative values are dummy values of vardat)
int   ave            : 0: integration, 1: average
 
HISTORY
2005-09-16 Peter Boesecke
----------------------------------------------------------------------------*/
void project_2          ( float *line, float *varline, int dim,
                             int imin, int imax, float initvalue, float factor,
                             float *data, float *vardat, int dim_1, int dim_2,
                             float f1_1, float f3_1,
                             float f1_2, float f3_2, float Df_2,
                             float dummy, float ddummy, int ave )
{ const float eps = 1e-30;
  float sum, weight;
  float varsum, varweight;
  float f_2;
  long  i, i0, i1;    // loop
  int   cnt;
 
  i0 = MAX2(0,imin);
  i1 = MIN2(dim-1,imax);
 
  /* initialize line with initvalue */
  for (i=0;i<dim;i++) line[i] = initvalue;
  if (varline) for (i=0;i<dim;i++) varline[i] = VarDummy;
 
  f_2=f1_2; // start index in data
  for (i=i0; i<=i1; i++) {

    if ( varline ) {
      // line = project(data) * factor
      // varline = project(vardat) * factor^2
      cnt = Isum2ldwE ( data, vardat, dim_1, dim_2, dummy, ddummy,
                        f1_1, f_2,  f3_1, f_2+Df_2,
                        &sum, &weight, &varsum, &varweight);
      line[i] = sum*factor;
      varline[i] = varsum*factor*factor;

      if (ave) {
        if (fabs(weight)>eps) line[i] /= weight;
        else line[i] = initvalue;
        if (fabs(varweight)>eps) varline[i] /= varweight*varweight;
        else varline[i] = VarDummy;
      } 

    } else { 
      cnt = Isum2ldw ( data, dim_1, dim_2, dummy, ddummy,
                       f1_1, f_2,  f3_1, f_2+Df_2,
                       &sum, &weight);
      line[i] = sum*factor;
      if (ave) {
        if (fabs(weight)>eps) line[i] /= weight;
        else line[i] = initvalue;
      }
    }
 
    f_2+=Df_2;
 
  } // for
 
} /* project_2 */


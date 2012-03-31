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

# define SX_VERSION      "sx : V1.09 Peter Boesecke 2011-12-14"
/*+++------------------------------------------------------------------------
NAME
  sx.c --- 3d orientation parameter transformation

INCLUDE FILES
  # include sx.h

PURPOSE
  Transformation of parameters during change of sx orientation.
  See PUBLIC functions for detail.

AUTHOR
  2011 Peter Boesecke (PB)

HISTORY
  2011-04-27 V1.0  PB
  2011-05-18 V1.01 PB axis types added
  2011-05-25 V1.02 PB sx_tf_img etc.
  2011-06-07 V1.03 PB void * params_in -> const void * params_in
                      sx_tf_img: array_ -> data_, variance_, Usage
                      sx_tf_img: initialize outp after copying input 
  2011-06-16 V1.04 PB double constants marked, e.g. 1->1.0
  2011-07-14 V1.05 PB calculation of bdis does neither require cen nor bcenr,
                      calculation of cen or bcen from bcen or cen requires
                      dis or bdis and rotations.
  2011-07-16 V1.06 PB sx_tf_params, sx_tf_img: parameter rot added,
                      use default rotations, when necessary
  2011-07-26 V1.07 PB sx_init function added
  2011-09-07 V1.08 PB SXPARAMS: space between # and pro removed to 
                      facilitate usage of awk, e.g.
                      sxparams  {<parameters>} |  awk '{print $5}' returns
                      on the first line the parameter name and on the second line
                      its calculated value.
  2011-12-14 V1.09 PB sx_tf_img: raster_order_normalization must be called
                      with data_dim_out and not with data_dim_in.
--------------------------------------------------------------------------*/

/****************************************************************************
*  Include                                                                  *
****************************************************************************/
# include "sx.h"

/****************************************************************************
* Definitions                                                               *
****************************************************************************/
# define SXPARAMS "#pro ori axis1 axis2 dim1 dim2 off1 off2 bis1 bis2 \
ras1 ras2 pix1 pix2 cen1 cen2 dis rot1 rot2 rot3 wvl \
bcen1 bcen2 bdis tilt1 tilt2 tilt3"

/****************************************************************************
* Static Variables                                                          *
****************************************************************************/

PRIVATE const double rad2deg = 180.0/NUM_PI;
PRIVATE const double SxEps = 1e-8;
PRIVATE int SxDebug = 0;
PRIVATE int SxLevel = 0;

PRIVATE char SX_Usage[SXBUFLEN];

/*--------------------------------------------------------------------------
NAME

  sx_version --- returns pointer to the version string

SYNOPSIS

  const char *sx_version ( void );

DESCRPTION

  Returns pointer to the version string.

--------------------------------------------------------------------------*/
PUBLIC const char *sx_version ( void )
{ 
  return ( SX_VERSION );
} /* sx_version */

/*--------------------------------------------------------------------------
NAME

  sx_usage2str --- return debug mode usage string

SYNOPSIS

  const char *sx_usage2str( void );

DESCRPTION

  Return debug mode usage string.

--------------------------------------------------------------------------*/
PUBLIC const char *sx_usage2str( void )
{ sprintf(SX_Usage,
    "verbose:0x%x,level:0x%x,showdata:0x%x,showtemp:0x%x,sxdebug:0x%x,sxraster:0x%x",
    SX_VERBOSE, SX_LEVEL, SX_SHOWDATA,
    SX_SHOWTEMP, SX_DEBUG, SX_RASTER_DEBUG);
  return(SX_Usage);
} // roca_usage2str

int fprint_debug( FILE *out )
{ fprintf(out,"debug      = 0x%x\n", SxDebug);
  fprintf(out,"verbose    = %d\n", SxDebug&SX_VERBOSE?1:0);
  fprintf(out,"level      = %d\n", SxLevel);
  fprintf(out,"showdata   = %d\n", SxDebug&SX_SHOWDATA?1:0);
  fprintf(out,"showtemp   = %d\n", SxDebug&SX_SHOWTEMP?1:0);
  fprintf(out,"sxdebug    = %d\n", SxDebug&SX_DEBUG?1:0);
  fprintf(out,"sxraster   = %d\n", SxDebug&SX_RASTER_DEBUG?1:0);
  return(0);
} // fprint_debug

/*--------------------------------------------------------------------------
NAME

  sx_debug_set --- set / reset module sx into debug mode

SYNOPSIS

  int sx_debug_set( int debug );

DESCRPTION

  Set / reset module sx into debug mode.

--------------------------------------------------------------------------*/
PUBLIC int sx_debug_set( int debug )
{ SxDebug = debug;
  SxLevel = (SxDebug&SX_LEVEL)>>1;

  raster_debug ( SxDebug&SX_RASTER_DEBUG?1:0 );

  if (SxDebug&SX_DEBUG) fprint_debug( stdout );
  return(0);

} // sx_debug_set

/*--------------------------------------------------------------------------
NAME

  sx_level --- return debug level 

SYNOPSIS

  int sx_level ( void );

--------------------------------------------------------------------------*/
PUBLIC int sx_level ( void )
{ return( SxLevel );
} // sx_Level

/*--------------------------------------------------------------------------
NAME

  sx_debug --- return debug value 

SYNOPSIS

  int sx_debug ( void );

--------------------------------------------------------------------------*/
PUBLIC int sx_debug ( void )

{ return( SxDebug );
} // sx_debug

/*--------------------------------------------------------------------------
NAME

  sx_init_params --- initializes sx parameters

SYNOPSIS

  SXParams * sx_init_params    ( SXParams * params );

DESCRPTION

  Initializes sx parameters with default values.

--------------------------------------------------------------------------*/
SXParams * sx_init_params    ( SXParams * params )
{
  if (SxDebug&SX_DEBUG) printf( "sx_init_params\n" );
  if ( params ) {
    // projection type as defined in reference.h
    params->pro.V = IO_ProSaxs;  params->pro.I = 0;
    // orientation number (1-16)
    params->ori.V = 1l;  params->ori.I = 0;
    // axis types as defined in reference.h
    params->axis1.V = IO_AxisTypeDistance;  params->axis1.I = 0;
    params->axis2.V = IO_AxisTypeDistance;  params->axis2.I = 0;
    // dimensions of 2d array
    params->dim1.V = 0l; params->dim1.I = 0;
    params->dim2.V = 1l; params->dim2.I = 0;
    // offsets of array coordinates
    params->off1.V = 0.0; params->off1.I = 0;
    params->off2.V = 0.0; params->off2.I = 0;
    // binning sizes
    params->bis1.V = 1.0; params->bis1.I = 0;
    params->bis2.V = 1.0; params->bis2.I = 0;
    // raster region of 2d array
    params->ras1.V = 0.0; params->ras1.I = 0;
    params->ras2.V = 0.0; params->ras2.I = 0;
    // pixel sizes [m]
    params->pix1.V = 1.0; params->pix1.I = 0;
    params->pix2.V = 1.0; params->pix2.I = 0;
    // PONI (point of normal incidence)
    params->cen1.V = 0.0; params->cen1.I = 0;
    params->cen2.V = 0.0; params->cen2.I = 0;
    // distance sample-PONI [m]
    params->dis.V = 1.0; params->dis.I = 0;
    // detector rotations [rad]
    params->rot1.V = 0.0; params->rot1.I = 0;
    params->rot2.V = 0.0; params->rot2.I = 0;
    params->rot3.V = 0.0; params->rot3.I = 0;
    // wavelength [m]
    params->wvl.V = 1.0; params->wvl.I = 0;
    // beam center (alt. cen1, cen2)
    params->bcen1.V = 0.0; params->bcen1.I = 0;
    params->bcen2.V = 0.0; params->bcen2.I = 0;
    // distance sample-bcen [m] (alt. dis)
    params->bdis.V = 1.0; params->bdis.I = 0;
    // detector rotations [rad]
    params->tilt1.V = 0.0; params->tilt1.I = 0;
    params->tilt2.V = 0.0; params->tilt2.I = 0;
    params->tilt3.V = 0.0; params->tilt3.I = 0;
  }

  if (SxDebug&SX_DEBUG) printf( "sx_init_params END\n" );
  return( params );
} // sx_init_params

/*--------------------------------------------------------------------------
NAME

  sx_cp_params --- copies input parameters to output

SYNOPSIS

  SXParams * sx_cp_params ( SXParams * params_out, 
                            const SXParams * params_in );

DESCRPTION

  Copies all params_in to params_out 

--------------------------------------------------------------------------*/
PUBLIC SXParams * sx_cp_params ( SXParams *params_out, 
                                 const SXParams *params_in )
{
  SXParams * out=NULL;

  if (SxDebug&SX_DEBUG) printf( "sx_cp_params\n" );

  if ( params_in && params_out ) {
    out = (SXParams *) memcpy( (void *) params_out, 
                               (const void *) params_in, sizeof( SXParams ));
  }

  if (SxDebug&SX_DEBUG) printf( "sx_cp_params END\n" );

  return( out );

} // sx_cp_params

/*--------------------------------------------------------------------------
NAME

  sx_new --- allocates and initializes new sx parameters

SYNOPSIS

  SXParams * sx_new    ( SXParams * params );

DESCRPTION

  Returns pointer to the successfully allocated and initialized
  parameters, otherwise NULL.

  If params is the NULL pointer, new memory is allocated and initialized,
  otherwise params is only initialized.

--------------------------------------------------------------------------*/
PUBLIC SXParams * sx_new    ( SXParams * params )
{ 
  SXParams * new = NULL;

  if (SxDebug&SX_DEBUG) printf( "sx_new\n" );

  if (!params) {
    if (!(new=malloc ( sizeof(SXParams) ))) 
      goto sx_new_error;
    params=new;
  }

  // initialize params
  if (!sx_init_params( params ))
    goto sx_new_error;

  if (SxDebug&SX_DEBUG) printf( "sx_new END\n" );

  return( params );

sx_new_error:

  if (new) free(new);

  if (SxDebug&SX_DEBUG) printf( "sx_new END (error)\n" );

  return(NULL);

} // sx_new

/*--------------------------------------------------------------------------
NAME

  sx_init --- initializes sx parameters

SYNOPSIS

  SXParams * sx_init    ( SXParams * params );

DESCRPTION

  Returns pointer to the successfully allocated and initialized
  parameters, otherwise NULL. Like sx_new, but without allocation of
  memory.

--------------------------------------------------------------------------*/
PUBLIC SXParams * sx_init    ( SXParams * params )
{
  if (SxDebug&SX_DEBUG) printf( "sx_init\n" );

  if (!params) goto sx_init_error;

  // initialize params
  if (!sx_init_params( params ))
    goto sx_init_error;

  if (SxDebug&SX_DEBUG) printf( "sx_init END\n" );

  return( params );

sx_init_error:

  if (SxDebug&SX_DEBUG) printf( "sx_init END (error)\n" );

  return(NULL);

} // sx_init

/*--------------------------------------------------------------------------
NAME

  sx_free --- releases the memory of sx parameters

SYNOPSIS

  SXParams * sx_free    ( SXParams * params );

DESCRPTION

  Returns NULL if successfully released, otherwise params.

--------------------------------------------------------------------------*/
PUBLIC SXParams * sx_free   ( SXParams * params )
{
  if (SxDebug&SX_DEBUG) printf( "sx_free\n" );
  if ( params ) free( params );
  if (SxDebug&SX_DEBUG) printf( "sx_free END\n" );

  return( NULL );

} // sx_free

/*--------------------------------------------------------------------------
NAME

  sx_pr_params --- print sx parameters

SYNOPSIS

  int sx_pr_params( FILE * out, const SXParams * params );

DESCRPTION

  Prints the value of the structure params.

RETURN VALUE

  0: success; -1: error

--------------------------------------------------------------------------*/
PUBLIC int sx_pr_params( FILE * out, const SXParams * params )
{
  if (SxDebug&SX_DEBUG) printf( "sx_pr_params\n" );

  if ((params)&&(out)) {

    fprintf(out,"  %s pro       = %10d : projection type (%d,%d)\n",
      params->pro.I?"X":" ",params->pro.V,IO_ProSaxs,IO_ProWaxs);
    fprintf(out,"  %s ori       = %10ld : orientation number (1-16)\n",
      params->ori.I?"X":" ",params->ori.V);
    fprintf(out,"  %s axis1     = %10d : type of axis 1 (%d,%d,%d)\n",
      params->axis1.I?"X":" ",params->axis1.V,IO_AxisTypeDistance,
      IO_AxisTypeAngle, IO_AxisTypeNumerator);
    fprintf(out,"  %s axis2     = %10d : type of axis 2 (%d,%d,%d)\n",
      params->axis2.I?"X":" ",params->axis2.V,IO_AxisTypeDistance, 
      IO_AxisTypeAngle, IO_AxisTypeNumerator);
    fprintf(out,"  %s dim1      = %10ld : dimension 1 of 2d array\n",
      params->dim1.I?"X":" ",params->dim1.V);
    fprintf(out,"  %s dim2      = %10ld : dimension 2 of 2d array\n",
      params->dim2.I?"X":" ",params->dim2.V);
    fprintf(out,"  %s off1      = %10lg : offset 1 of array coordinates\n",
      params->off1.I?"X":" ",params->off1.V);
    fprintf(out,"  %s off2      = %10lg : offset 2 of array coordinates\n",
      params->off2.I?"X":" ",params->off2.V);
    fprintf(out,"  %s bis1      = %10lg : binning size 1\n",
      params->bis1.I?"X":" ",params->bis1.V);
    fprintf(out,"  %s bis2      = %10lg : binning size 2\n",
      params->bis2.I?"X":" ",params->bis2.V);
    fprintf(out,"  %s ras1      = %10lg : raster region of axis 1\n",
      params->ras1.I?"X":" ",params->ras1.V);
    fprintf(out,"  %s ras2      = %10lg : raster region of axis 2\n",
      params->ras2.I?"X":" ",params->ras2.V);
    fprintf(out,"  %s pix1      = %10lg : pixel size 1 [m]\n",
      params->pix1.I?"X":" ",params->pix1.V);
    fprintf(out,"  %s pix2      = %10lg : pixel size 2 [m]\n",
      params->pix2.I?"X":" ",params->pix2.V);
    fprintf(out,"  %s cen1      = %10lg : PONI 1 (point of normal incidence)\n",
      params->cen1.I?"X":" ",params->cen1.V);
    fprintf(out,"  %s cen2      = %10lg : PONI 2 (point of normal incidence)\n",
      params->cen2.I?"X":" ",params->cen2.V);
    fprintf(out,"  %s dis       = %10lg : distance sample-PONI [m]\n",
      params->dis.I?"X":" ",params->dis.V);
    fprintf(out,"  %s rot1      = %10lg : detector rotation 1 [rad] (%10lg deg)\n",
      params->rot1.I?"X":" ",params->rot1.V,params->rot1.V*rad2deg);
    fprintf(out,"  %s rot2      = %10lg : detector rotation 2 [rad] (%10lg deg)\n",
      params->rot2.I?"X":" ",params->rot2.V,params->rot2.V*rad2deg);
    fprintf(out,"  %s rot3      = %10lg : detector rotation 3 [rad] (%10lg deg)\n",
      params->rot3.I?"X":" ",params->rot3.V,params->rot3.V*rad2deg);
    fprintf(out,"  %s wvl       = %10lg : wavelength [m]\n",
      params->wvl.I?"X":" ",params->wvl.V);
    fprintf(out,"  %s bcen1     = %10lg : beam center 1\n",
      params->bcen1.I?"X":" ",params->bcen1.V);
    fprintf(out,"  %s bcen2     = %10lg : beam center 2\n",
      params->bcen2.I?"X":" ",params->bcen2.V);
    fprintf(out,"  %s bdis      = %10lg : distance sample-bcen [m]\n",
      params->bdis.I?"X":" ",params->bdis.V);
    fprintf(out,"  %s tilt1      = %10lg : detector tilt 1 [rad] (%10lg deg)\n",
      params->tilt1.I?"X":" ",params->tilt1.V,params->tilt1.V*rad2deg);
    fprintf(out,"  %s tilt2      = %10lg : detector tilt 2 [rad] (%10lg deg)\n",
      params->tilt2.I?"X":" ",params->tilt2.V,params->tilt2.V*rad2deg);
    fprintf(out,"  %s tilt3      = %10lg : detector tilt 3 [rad] (%10lg deg)\n",
      params->tilt3.I?"X":" ",params->tilt3.V,params->tilt3.V*rad2deg);
  }

  if (SxDebug&SX_DEBUG) fprintf( stdout, "sx_pr_params END\n" );

  return(0);

} // sx_pr_params

/*---------------------------------------------------------------------------
NAME

  sx_pr_params_line --- print sx parameters in a single line

SYNOPSIS

  int sx_pr_params_line( FILE *out, const SXParams *params, int head );

DESCRIPTION

  Prints the value of the structure params. If head is > 0
  a commented head line with the name of all values is written on top.

  The parameters are

  SXI pro;                          // projection (IO_SaxsPro, IO_WaxsPro) 
  SXL ori;                          // orientation number (1-16)
  SXI axis1, axis2;                 // axis type (IO_AxisTypeDistance,
                                    // IO_AxisTypeAngle, IO_AxisTypeNumerator)
  SXL dim1;  SXL dim2;              // dimensions of 2d array 
  SXD off1;  SXD off2;              // offsets of array coordinates
  SXD bis1;  SXD bis2;              // binning sizes
  SXD ras1;  SXD ras2;              // raster region of 2d array 
  SXD pix1;  SXD pix2;              // pixel sizes [m]
  SXD cen1;  SXD cen2;              // PONI (point of normal incidence)
  SXD dis;                          // distance sample-PONI [m]
  SXD rot1;  SXD rot2; SXD rot3;    // detector rotations [rad]
  SXD wvl;                          // wavelength [m]
  SXD bcen1; SXD bcen2;             // beam center (alt. cen1, cen2)
  SXD bdis;                         // distance sample-bcen [m] (alt. dis) 
  SXD tilt1;  SXD tilt2; SXD tilt3; // detector tilts [rad]

RETURN VALUE

  0

---------------------------------------------------------------------------*/
PUBLIC int sx_pr_params_line( FILE *out, const SXParams *params, int head ) 
{
  if (SxDebug&SX_DEBUG) fprintf( stdout, "sx_pr_params_line\n" );

  if (head>0) fprintf(out,"%s\n",SXPARAMS);
  if (params->pro.I) fprintf(out,"%d ",params->pro.V); else fprintf(out,"- ");
  if (params->ori.I) fprintf(out,"%ld ",params->ori.V); else fprintf(out,"- ");
  if (params->axis1.I) fprintf(out,"%d ",params->axis1.V); else fprintf(out,"- ");
  if (params->axis2.I) fprintf(out,"%d ",params->axis2.V); else fprintf(out,"- ");
  if (params->dim1.I) fprintf(out,"%ld ",params->dim1.V); else fprintf(out,"- ");
  if (params->dim2.I) fprintf(out,"%ld ",params->dim2.V); else fprintf(out,"- ");
  if (params->off1.I) fprintf(out,"%lg ",params->off1.V); else fprintf(out,"- ");
  if (params->off2.I) fprintf(out,"%lg ",params->off2.V); else fprintf(out,"- ");
  if (params->bis1.I) fprintf(out,"%lg ",params->bis1.V); else fprintf(out,"- ");
  if (params->bis2.I) fprintf(out,"%lg ",params->bis2.V); else fprintf(out,"- ");
  if (params->ras1.I) fprintf(out,"%lg ",params->ras1.V); else fprintf(out,"- ");
  if (params->ras2.I) fprintf(out,"%lg ",params->ras2.V); else fprintf(out,"- ");
  if (params->pix1.I) fprintf(out,"%lg ",params->pix1.V); else fprintf(out,"- ");
  if (params->pix2.I) fprintf(out,"%lg ",params->pix2.V); else fprintf(out,"- ");
  if (params->cen1.I) fprintf(out,"%lg ",params->cen1.V); else fprintf(out,"- ");
  if (params->cen2.I) fprintf(out,"%lg ",params->cen2.V); else fprintf(out,"- ");
  if (params->dis.I) fprintf(out,"%lg ",params->dis.V); else fprintf(out,"- ");
  if (params->rot1.I) fprintf(out,"%lg ",params->rot1.V); else fprintf(out,"- ");
  if (params->rot2.I) fprintf(out,"%lg ",params->rot2.V); else fprintf(out,"- ");
  if (params->rot3.I) fprintf(out,"%lg ",params->rot3.V); else fprintf(out,"- ");
  if (params->wvl.I) fprintf(out,"%lg ",params->wvl.V); else fprintf(out,"- ");
  if (params->bcen1.I) fprintf(out,"%lg ",params->bcen1.V); else fprintf(out,"- ");
  if (params->bcen2.I) fprintf(out,"%lg ",params->bcen2.V); else fprintf(out,"- ");
  if (params->bdis.I) fprintf(out,"%lg ",params->bdis.V); else fprintf(out,"- ");
  if (params->tilt1.I) fprintf(out,"%lg ",params->tilt1.V); else fprintf(out,"- ");
  if (params->tilt2.I) fprintf(out,"%lg ",params->tilt2.V); else fprintf(out,"- ");
  if (params->tilt3.I) fprintf(out,"%lg ",params->tilt3.V); else fprintf(out,"- ");

  if (SxDebug&SX_DEBUG) fprintf( stdout, "sx_pr_params_line END\n" );

  return(0);

} // sx_pr_params_line

/*---------------------------------------------------------------------------
NAME

  sx_rd_params --- reads the parameters from the string array argv[]  

SYNOPSIS

  SXParams * sx_rd_params ( SXParams * params_out, char *argv[],
                              int * perrval );

DESCRIPTION

  The sx parameters are successively read from argv[0], argv[1], ...
  until the end of argv[], indicated by a NULL pointer, or until 
  all possible parameters have been read.

  If params_out is NULL a new parameter structure is allocated and
  needs, in case of success, to be released by the calling program.

  In case of success the pointer to the params_out or to the alloated
  parameter structure is returned.

RETURN VALUE

  In case of success the pointer to the parameter structure is returned,
  otherwise NULL.

---------------------------------------------------------------------------*/
PUBLIC SXParams * sx_rd_params ( SXParams * params_out, char *argv[], 
                                   int * perrval )

{
  char *nul = (char *) NULL;
  int errval=0;
  char **pargv=NULL;
  SXParams * params=NULL;
  long N=3;

  if (SxDebug&SX_DEBUG) printf( "sx_rd_params\n" );

  if (argv) pargv=&(argv[0]);
  else goto sx_rd_params_error;

  // initialize params_out (if NULL allocate and initialize)
  if ( !(params=sx_new ( params_out )) )
    goto sx_rd_params_error;

  if (*pargv==nul) goto sx_rd_params_end;

      // read params from argument list
      // projection (IO_ProSaxs, IO_ProWaxs) 
      if (sx_debug()&SX_DEBUG) 
        printf( "reading pro from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        strlib_tolower(*pargv);
        if ( (strcmp( *pargv,"saxs" ) == 0)||
             (strcmp( *pargv,"s" ) == 0)||
             (strncmp( *pargv,"no", 2 ) == 0) ) 
           params->pro.V = IO_ProSaxs; // no projection
        else if ( (strcmp( *pargv,"waxs" ) == 0)||
                  (strcmp( *pargv,"sp" ) == 0)||
                  (strncmp( *pargv,"ewa",3 ) == 0) ) 
           params->pro.V = IO_ProWaxs; // ewald sphere projection
        else {
          params->pro.V = (int) num_str2long ( *pargv, NULL, &errval );
          if (errval) goto sx_rd_params_error;
        }
        params->pro.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // ori orientation number (1-16)
      if (sx_debug()&SX_DEBUG) 
        printf( "reading ori from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->ori.V = raster_str2number( N, *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->ori.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // axis type (IO_AxisTypeDistance, IO_AxisTypeAngle, IO_AxisTypeNumerator)
      if (sx_debug()&SX_DEBUG)
        printf( "reading axis1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        strlib_tolower(*pargv);
        if ( (strcmp( *pargv,"angle" ) == 0) )
           params->axis1.V = IO_AxisTypeDistance; // distance 
        else if ( (strcmp( *pargv,"angle" ) == 0) )
           params->axis1.V = IO_AxisTypeAngle; // angle 
        else if ( (strcmp( *pargv,"numerator" ) == 0) )
           params->axis1.V = IO_AxisTypeNumerator; // numerator 
        else {
          params->axis1.V = (int) num_str2long ( *pargv, NULL, &errval );
          if (errval) goto sx_rd_params_error;
        }
        params->axis1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      if (sx_debug()&SX_DEBUG)
        printf( "reading axis2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        strlib_tolower(*pargv);
        if ( (strcmp( *pargv,"angle" ) == 0) )
           params->axis2.V = IO_AxisTypeDistance; // distance
        else if ( (strcmp( *pargv,"angle" ) == 0) )
           params->axis2.V = IO_AxisTypeAngle; // angle
        else if ( (strcmp( *pargv,"numerator" ) == 0) )
           params->axis2.V = IO_AxisTypeNumerator; // numerator
        else {
          params->axis2.V = (int) num_str2long ( *pargv, NULL, &errval );
          if (errval) goto sx_rd_params_error;
        }
        params->axis2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // dim1 dimensions of 2d array
      if (sx_debug()&SX_DEBUG) 
        printf( "reading dim1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->dim1.V = num_str2long ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->dim1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // dim2 dimensions of 2d array
      if (sx_debug()&SX_DEBUG) 
        printf( "reading dim2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->dim2.V = num_str2long ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->dim2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // off1 offsets of array coordinates
      if (sx_debug()&SX_DEBUG) 
        printf( "reading off1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->off1.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->off1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // off2 offsets of array coordinates
      if (sx_debug()&SX_DEBUG) 
        printf( "reading off2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->off2.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
      params->off2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // bis1 binning sizes
      if (sx_debug()&SX_DEBUG) 
        printf( "reading bis1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->bis1.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->bis1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // bis2 binning sizes
      if (sx_debug()&SX_DEBUG) 
        printf( "reading bis2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->bis2.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->bis2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // ras1 raster region of 2d array
      if (sx_debug()&SX_DEBUG) 
        printf( "reading ras1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->ras1.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->ras1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // ras2 raster region of 2d array
      if (sx_debug()&SX_DEBUG) 
        printf( "reading ras2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->ras2.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->ras2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // pix1 pixel sizes [m]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading pix1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->pix1.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->pix1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // pix2 pixel sizes [m]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading pix2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->pix2.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->pix2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // cen1 PONI (point of normal incidence)
      if (sx_debug()&SX_DEBUG) 
        printf( "reading cen1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->cen1.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->cen1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // cen2 PONI (point of normal incidence)
      if (sx_debug()&SX_DEBUG) 
        printf( "reading cen2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->cen2.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->cen2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // dis distance sample-PONI [m]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading dis from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->dis.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->dis.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // rot1 detector rotations [rad]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading rot1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->rot1.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->rot1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // rot2 detector rotations [rad]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading rot2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->rot2.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->rot2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // rot3 detector rotations [rad]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading rot3 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->rot3.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->rot3.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // wvl wavelength [m]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading wvl from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->wvl.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->wvl.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // bcen1 beam center (alt. cen1, cen2)
      if (sx_debug()&SX_DEBUG) 
        printf( "reading bcen1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->bcen1.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->bcen1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // bcen2 beam center (alt. cen1, cen2)
      if (sx_debug()&SX_DEBUG) 
        printf( "reading bcen2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->bcen2.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->bcen2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // bdis distance sample-bcen [m] (alt. dis)
      if (sx_debug()&SX_DEBUG) 
        printf( "reading bdis from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->bdis.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->bdis.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // tilt1 detector tilts [rad]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading tilt1 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->tilt1.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->tilt1.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // tilt2 detector tilt [rad]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading tilt2 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->tilt2.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->tilt2.I = 1;
      }

      if (*(++pargv)==nul) goto sx_rd_params_end;
      // tilt3 detector tilts [rad]
      if (sx_debug()&SX_DEBUG) 
        printf( "reading tilt3 from >>%s<<\n", *pargv );
      if (strlib_is_no_skip(*pargv)) {
        params->tilt3.V = num_str2double ( *pargv, NULL, &errval );
        if (errval) goto sx_rd_params_error;
        params->tilt3.I = 1;
      }

sx_rd_params_end:

  if (perrval) *perrval = errval;

  if (SxDebug&SX_DEBUG) printf( "sx_rd_params END\n" );

  return(params);

sx_rd_params_error:

  if ((!params_out)&&(params)) sx_free(params);

  if (perrval) *perrval = errval;

  if (SxDebug&SX_DEBUG) 
    printf( "sx_rd_params END (errval=%d)\n",errval );

  return(NULL);

} // sx_rd_params

/*--------------------------------------------------------------------------
NAME

  sx_tf_params --- transforms sx parameters to orientation ori

SYNOPSIS

  SXParams *sx_tf_params ( SXParams * params_out, const SXParams * params_in,
                           long ori, int rot, int *perrval );

DESCRPTION

  Returns a pointer to the successfully transformed sx input parameters, 
  otherwise NULL. If the output pointer out is NULL memory is allocated 
  and must be released by the calling program, otherwise out is used. 
  The input (*in) and output (*buffer) buffers can be identical.

  params_in->pro.V: input projection (if 0, IO_ProSaxs is used)
  params_in->ori.V: input orientation (if 0, orientation 1 is used)
  long ori: output orientation (if 0, in->ori.V is used)
  int rot: use default rotations, when necessary

--------------------------------------------------------------------------*/
PUBLIC SXParams *sx_tf_params ( SXParams * params_out, 
                                const SXParams * params_in,
                                long ori, int rot, int *perrval )
{ SXParams *new=NULL, *outp=NULL;
  SXParams in_buffer, *inp;

  long t_ori, inv_t_ori;
  long *t_order=NULL, *inv_t_order=NULL;
  double *T=NULL, *inv_T=NULL;
  long omod2, omod4, omod8, omod16;

  int N=3;
  double R[3][3], RT[3][3], TRT[3][3];
  double Angle[3], TAngle[3];
  double Tilt[3], TTilt[3];

  int tmpX;
  long tmpL;
  double tmpV;
  int tmpI;
  int RotI=0;

  int errval=0;

  if (SxDebug&SX_DEBUG) printf( "sx_tf_params BEGIN\n" );

  if (params_in) {

    // copy input parameters to an internal buffer
    if (!(inp=sx_cp_params ( &in_buffer, params_in ))) {
      errval=SX_COPY_ERROR;
      goto sx_tf_params_error;
    }

    // normalize input projection
    if ( inp->pro.V == 0 ) inp->pro.V = IO_ProSaxs;

    // normalize input orientation
    if ( inp->ori.V == 0l ) inp->ori.V=1l; // default
    else if ( inp->ori.V < 0l ) inp->ori.V = raster_inversion( -inp->ori.V );

    // normalize axis types
    if ( inp->axis1.V == 0 ) inp->axis1.V = IO_AxisTypeDistance;
    if ( inp->axis2.V == 0 ) inp->axis2.V = IO_AxisTypeDistance;

    if (SxDebug&SX_SHOWDATA) {
      printf( "  Input parameters\n");
      sx_pr_params( stdout, inp );
    }

    if ( !(( inp->pro.V == IO_ProSaxs )||( inp->pro.V == IO_ProWaxs )) ) {
      errval=SX_INVALID_PROJECTION;
      goto sx_tf_params_error;
    }

    if ( inp->ori.V > 16l ) {
      errval=SX_INVALID_ORIENTATION;
      goto sx_tf_params_error;
    }

    if ( !(( inp->axis1.V == IO_AxisTypeDistance )||\
           ( inp->axis1.V == IO_AxisTypeAngle )||\
           ( inp->axis1.V == IO_AxisTypeNumerator )) ) {
      errval=SX_INVALID_AXISTYPE;
      goto sx_tf_params_error;
    }

    if ( !(( inp->axis2.V == IO_AxisTypeDistance )||\
           ( inp->axis2.V == IO_AxisTypeAngle )||\
           ( inp->axis2.V == IO_AxisTypeNumerator )) ) {
      errval=SX_INVALID_AXISTYPE;
      goto sx_tf_params_error;
    }

    // set output orientation default
    if ( ori == 0l ) ori = inp->ori.V;
    else if ( ori < 0l ) ori = raster_inversion( -ori );

    if (SxDebug&SX_SHOWDATA)
      printf( "  Output orientation = %ld\n",ori);

    if ( ori > 16l ) {
      errval=SX_INVALID_ORIENTATION;
      goto sx_tf_params_error;
    }

    // calculate tilts from rotations
    Angle[0] = inp->rot1.V;
    Angle[1] = inp->rot2.V;
    Angle[2] = inp->rot3.V;

    if (rot3d_matrix(Angle, R)) {
      errval=SX_MATRIX_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }
    if (tilt3d_angles(Tilt, R)) {
      errval=SX_ANGLE_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }

    // update tilt1, tilt2, tilt3
    if (inp->tilt1.I) Tilt[0] = inp->tilt1.V;
    if (inp->tilt2.I) Tilt[1] = inp->tilt2.V;
    if (inp->tilt3.I) Tilt[2] = inp->tilt3.V;

    if (tilt3d_matrix(Tilt, R)) {
      errval=SX_MATRIX_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }
    RotI=((RotI)||(inp->tilt1.I)||(inp->tilt2.I)||(inp->tilt3.I))?1:0;

    // recalculate rot1, rot2, rot3
    if (rot3d_angles(Angle, R)) {
      errval=SX_ANGLE_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }

    // update rotations
    if (inp->rot1.I) Angle[0] = inp->rot1.V;
    if (inp->rot2.I) Angle[1] = inp->rot2.V;
    if (inp->rot3.I) Angle[2] = inp->rot3.V;
    if (SxDebug&SX_SHOWTEMP) // display Angle
      raster_fprint_matrix( stdout, N, 1, Angle, "Angle[3]" );

    // calculation rotation matrix
    if (rot3d_matrix(Angle, R)) {
      errval=SX_MATRIX_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }
    RotI=((RotI)||(inp->rot1.I)||(inp->rot2.I)||(inp->rot3.I))?1:0;

    if (SxDebug&SX_SHOWTEMP) // display R[3][3]
      raster_fprint_matrix( stdout, N, N, (double*) R, "R[3][3]" );

    // update beam parameters, if possible
    if (SxDebug&SX_SHOWTEMP) // fabs( R[2][2] )
      printf("R[2][2] = %lg, SxEps = %lg\n", R[2][2],SxEps);
    if ( fabs( R[2][2] ) > SxEps ) {
      // the detector normal is not perpendicular to the beam
      // update bcen and bdis if not set (to allow default values)
      //  SXD bcen1; SXD bcen2;       // beam center (alt. cen1, cen2)
      //  SXD bdis;                   // distance sample-bcen [m] (alt. dis) 
      if ( inp->pro.V==IO_ProSaxs ) {
        if (!inp->bdis.I) {
          if (r2t_bdis ( &(inp->bdis.V),inp->dis.V,R )) {
            errval=SX_BEAMDISTANCE_CALCULATION_ERROR;
            goto sx_tf_params_error;
          }
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->bdis.V = %lg (updated)\n", inp->bdis.V);
        }
        if (!inp->bcen1.I) {
          if (r2t_bcen1( &(inp->bcen1.V),inp->pix1.V,inp->cen1.V,inp->dis.V,R ) ) {
            errval=SX_BEAMCENTER_CALCULATION_ERROR;
            goto sx_tf_params_error;
          }
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->bcen1.V = %lg (updated)\n", inp->bcen1.V);
        }
        if (!inp->bcen2.I) {
          if (r2t_bcen2( &(inp->bcen2.V),inp->pix2.V,inp->cen2.V,inp->dis.V,R ) ) {
            errval=SX_BEAMCENTER_CALCULATION_ERROR;
            goto sx_tf_params_error;
          }
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->bcen2.V = %lg (updated)\n", inp->bcen2.V);
        }
      } else { // IO_ProWaxs
        if (!inp->bcen1.I) {
          inp->bcen1.V = inp->cen1.V;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->bcen1.V = %lg (updated)\n", inp->bcen1.V);
        }
        if (!inp->bcen2.I) {
          inp->bcen2.V = inp->cen2.V;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->bcen2.V = %lg (updated)\n", inp->bcen2.V);
        }
        if (!inp->bdis.I) {
          inp->bdis.V = inp->dis.V;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->bdis.V = %lg (updated)\n", inp->bdis.V);
        }
      }

      // update dis, cen1, cen2 if not set
      //  SXD bcen1; SXD bcen2;       // beam center (alt. cen1, cen2)
      //  SXD bdis;                    // distance sample-bcen [m] (alt. dis) 
      if ( inp->pro.V==IO_ProSaxs ) {
        if (rot) RotI=((inp->bdis.I))?1:RotI;
        if (!inp->dis.I) {
          if (r2t_dis  ( &(inp->dis.V),inp->bdis.V,R )) {
            errval=SX_DISTANCE_CALCULATION_ERROR;
            goto sx_tf_params_error;
          }
          inp->dis.I=((RotI)&&(inp->bdis.I))?1:0;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->dis.V = %lg (updated)\n", inp->dis.V);
        }
        if (rot) RotI=((inp->bcen1.I)&&(inp->pix1.I)&&
                      ((inp->bdis.I)||(inp->dis.I)))?1:RotI;
        if (!inp->cen1.I) {
          if (r2t_cen1 ( &(inp->cen1.V),inp->pix1.V,inp->bcen1.V,inp->bdis.V,R )) {
            errval=SX_CENTER_CALCULATION_ERROR;
            goto sx_tf_params_error;
          }
          inp->cen1.I=((RotI)&&(inp->bcen1.I)&&(inp->pix1.I)&&(inp->bdis.I))?1:0;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->cen1.V = %lg (updated)\n", inp->cen1.V);
        }
        if (rot) RotI=((inp->bcen2.I)&&(inp->pix2.I)&&
                      ((inp->bdis.I)||(inp->dis.I)))?1:RotI;
        if (!inp->cen2.I) {
          if (r2t_cen2 ( &(inp->cen2.V),inp->pix2.V,inp->bcen2.V,inp->bdis.V,R )) {
            errval=SX_CENTER_CALCULATION_ERROR;
            goto sx_tf_params_error;
          }
          inp->cen2.I=((RotI)&&(inp->bcen2.I)&&(inp->pix2.I)&&(inp->bdis.I))?1:0;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->cen2.V = %lg (updated)\n", inp->cen2.V);
        }
      } else { // IO_ProWaxs
        if (!inp->dis.I) {
          inp->dis.V=inp->bdis.V;
          inp->dis.I=(inp->bdis.I)?1:0;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->dis.V = %lg (updated)\n", inp->dis.V);
        }
        if (!inp->cen1.I) {
          inp->cen1.V=inp->bcen1.V;
          inp->cen1.I=(inp->bcen1.I)?1:0;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->cen1.V = %lg (updated)\n", inp->cen1.V);
        }
        if (!inp->cen2.I) {
          inp->cen2.V=inp->bcen2.V;
          inp->cen2.I=(inp->bcen1.I)?1:0;
          if (SxDebug&SX_SHOWTEMP)
            printf("inp->cen2.V = %lg (updated)\n", inp->cen2.V);
        }
      }
    } // if ( fabs( R[2][2] ) > SxEps ) 

    // initialize output buffer
    if (!params_out) {
      if (!(new=sx_new( NULL ))) {
        errval=SX_MEMORY_ALLOCATION_ERROR;
        goto sx_tf_params_error;
      }
      outp = new;
    } else if (!(outp=sx_new( params_out ))) {
      errval=SX_MEMORY_ALLOCATION_ERROR;
      goto sx_tf_params_error;
    }

    // copy input parameters to output parameters
    if (!sx_cp_params ( outp, inp )) {
      errval=SX_COPY_ERROR;
      goto sx_tf_params_error;
    }

    outp->ori.V = ori; outp->ori.I = 1;

    // calculate relative transformation t_ori from inp->ori.V to outp->ori.V
    if (SxDebug&SX_SHOWTEMP) // display outp->ori.V and inp->ori.V
      printf( "  outp->ori.V = %ld, inp->ori.V = %ld\n",outp->ori.V,inp->ori.V);
    t_ori = raster_multiplication( outp->ori.V, raster_inversion( inp->ori.V ) );
    if (SxDebug&SX_SHOWTEMP) // display t_ori
      printf( "  t_ori = %ld\n",t_ori);

    t_order = raster_number2order ( NULL, 0, 3, t_ori );
    if (!t_order) {
      errval=SX_ORDER_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }

    inv_t_ori = raster_inversion ( t_ori );
    if (SxDebug&SX_SHOWTEMP) // display inv_t_ori
      printf( "  inv_t_ori = %ld\n",inv_t_ori);

    inv_t_order = raster_number2order ( NULL, 0, 3, inv_t_ori );
    if (!inv_t_order) {
      errval=SX_ORDER_CALCULATION_ERROR; 
      goto sx_tf_params_error;
    }

    T = raster_order2matrix ( NULL, 0, t_order );
    if (!T) {
      errval=SX_MATRIX_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }
    if (SxDebug&SX_SHOWTEMP) // display T
      raster_fprint_matrix( stdout, N, N, T, "T[3][3]" );

    inv_T = raster_order2matrix ( NULL, 0, inv_t_order );
    if (!inv_T) {
      errval=SX_MATRIX_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }
    if (SxDebug&SX_SHOWTEMP) // display inv_T
      raster_fprint_matrix( stdout, N, N, inv_T, "inv_T[3][3]" );

    omod2  = (t_ori-1l) % 2l;
    omod4  = (t_ori-1l) % 4l;
    omod8  = (t_ori-1l) % 8l;
    omod16 = (t_ori-1l) % 16l;

    if ( omod2 >= 1l ) { // invert first coordinate
      if (SxDebug&SX_DEBUG)
        printf("  invert first coordinate\n");
      if (inp->ras1.I) {
        outp->off1.V = OSWAP2(inp->ras1.V,inp->bis1.V,inp->off1.V,inp->dim1.V);
        outp->off1.I = 1;
      }
      outp->cen1.V = CSWAP2(outp->off1.V,inp->cen1.V,inp->off1.V,inp->dim1.V);
      outp->cen1.I = inp->cen1.I;
    }

    if ( omod4 >= 2l ) { // invert second coordinate
      if (SxDebug&SX_DEBUG)
        printf("  invert second coordinate\n");
      if (inp->ras2.I) {
        outp->off2.V = OSWAP2(inp->ras2.V,inp->bis2.V,inp->off2.V,inp->dim2.V);
        outp->off2.I = 1;
      }
      outp->cen2.V = CSWAP2(outp->off2.V,inp->cen2.V,inp->off2.V,inp->dim2.V);
      outp->cen2.I = inp->cen2.I;
    }

    if ( omod8 >= 4l ) { // swap coordinates
      if (SxDebug&SX_DEBUG)
        printf("  swap first and second coordinates\n");

      tmpX = outp->axis1.V; tmpI = outp->dim1.I;
      outp->axis1.V = outp->axis2.V; outp->axis1.I = outp->axis2.I;
      outp->axis2.V = tmpX; outp->axis2.I = tmpI;

      tmpL = outp->dim1.V; tmpI = outp->dim1.I;
      outp->dim1.V = outp->dim2.V; outp->dim1.I = outp->dim2.I;
      outp->dim2.V = tmpL; outp->dim2.I = tmpI;

      tmpV = outp->off1.V; tmpI = outp->off1.I;
      outp->off1.V = outp->off2.V; outp->off1.I = outp->off2.I;
      outp->off2.V = tmpV; outp->off2.I = tmpI;

      tmpV = outp->cen1.V; tmpI = outp->cen1.I;
      outp->cen1.V = outp->cen2.V; outp->cen1.I = outp->cen2.I;
      outp->cen2.V = tmpV; outp->cen2.I = tmpI;

      tmpV = outp->bis1.V; tmpI = outp->bis1.I;
      outp->bis1.V = outp->bis2.V; outp->bis1.I = outp->bis2.I;
      outp->bis2.V = tmpV; outp->bis2.I = tmpI;

      tmpV = outp->pix1.V; tmpI = outp->pix1.I;
      outp->pix1.V = outp->pix2.V; outp->pix1.I = outp->pix2.I;
      outp->pix2.V = tmpV; outp->pix2.I = tmpI;

      tmpV = outp->ras1.V; tmpI = outp->ras1.I;
      outp->ras1.V = outp->ras2.V; outp->ras1.I = outp->ras2.I;
      outp->ras2.V = tmpV; outp->ras2.I = tmpI;
    }

    outp->dis.V = inp->dis.V; outp->dis.I = inp->dis.I;
    outp->wvl.V = inp->wvl.V; outp->wvl.I = inp->wvl.I;

//  SXD rot1;  SXD rot2; SXD rot3; // detector rotations [rad]

    raster_matrix_product ( (double*) RT, N*N, (double *) R, T, N, N, N );
    if (SxDebug&SX_SHOWTEMP)// display RT
      raster_fprint_matrix( stdout, N, N, (double*) RT, "RT[3][3]" );
    raster_matrix_product ( (double*) TRT, N*N, inv_T, (double *) RT, N, N, N );
    if (SxDebug&SX_SHOWTEMP)// display TRT
      raster_fprint_matrix( stdout, N, N, (double*) TRT, "TRT[3][3]" );
    if (rot3d_angles(TAngle, TRT)) {
      errval=SX_ANGLE_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }
    if (SxDebug&SX_SHOWTEMP)// display TAngle
      raster_fprint_matrix( stdout, N, 1, TAngle, "TAngle[3]" );

    outp->rot1.V = TAngle[0];
    outp->rot2.V = TAngle[1];
    outp->rot3.V = TAngle[2];

    outp->rot1.I = RotI;
    outp->rot2.I = RotI;
    outp->rot3.I = RotI;

    // update tilt1, tilt2, tilt3
    if (tilt3d_angles(TTilt, TRT)) {
      errval=SX_ANGLE_CALCULATION_ERROR;
      goto sx_tf_params_error;
    }
    if (SxDebug&SX_SHOWTEMP)// display TTilts
      raster_fprint_matrix( stdout, N, 1, TTilt, "TTilts[3]" );

    outp->tilt1.V = TTilt[0];
    outp->tilt2.V = TTilt[1];
    outp->tilt3.V = TTilt[2];

    outp->tilt1.I = RotI;
    outp->tilt2.I = RotI;
    outp->tilt3.I = RotI;

    // update beam parameters if detector plane is not perpendicular to beam
    if (SxDebug&SX_SHOWTEMP) // fabs( TRT[2][2] )
      printf("TRT[2][2] = %lg, SxEps = %lg\n", TRT[2][2],SxEps);
    if ( fabs( TRT[2][2] ) > SxEps ) {
      // update bcen and bdis
      //  SXD bcen1; SXD bcen2;      // beam center (alt. cen1, cen2)
      //  SXD bdis;                   // distance sample-bcen [m] (alt. dis) 
      if ( outp->pro.V==IO_ProSaxs ) {
        if (r2t_bdis ( &(outp->bdis.V),outp->dis.V,TRT )) {
          errval=SX_BEAMDISTANCE_CALCULATION_ERROR;
          goto sx_tf_params_error;
        }
        outp->bdis.I=((outp->dis.I)&&(RotI))?1:0;
        if (SxDebug&SX_SHOWTEMP)
          printf("outp->bdis.V = %lg (updated)\n", outp->bdis.V);
        if (r2t_bcen1( &(outp->bcen1.V),outp->pix1.V,outp->cen1.V,outp->dis.V,TRT )){
          errval=SX_BEAMCENTER_CALCULATION_ERROR;
          goto sx_tf_params_error;
        }
        outp->bcen1.I=((outp->cen1.I)&&(RotI)&&(outp->pix1.I))?1:0;
        if (SxDebug&SX_SHOWTEMP)
          printf("outp->bcen1.V = %lg (updated)\n", outp->bcen1.V);
        if (r2t_bcen2( &(outp->bcen2.V),outp->pix2.V,outp->cen2.V,outp->dis.V,TRT )){
          errval=SX_BEAMCENTER_CALCULATION_ERROR;
          goto sx_tf_params_error;
        }
        outp->bcen2.I=((outp->cen2.I)&&(RotI)&&(outp->pix2.I))?1:0;
        if (SxDebug&SX_SHOWTEMP)
          printf("outp->bcen2.V = %lg (updated)\n", outp->bcen2.V);
      } else { // IO_ProWaxs
        outp->bdis.V=outp->dis.V; 
        outp->bdis.I=((outp->dis.I)&&(RotI))?1:0;
        if (SxDebug&SX_SHOWTEMP)
          printf("outp->bdis.V = %lg (updated)\n", outp->bdis.V);
        outp->bcen1.V=outp->cen1.V; 
        outp->bcen1.I=((outp->cen1.I)&&(RotI))?1:0;
        if (SxDebug&SX_SHOWTEMP)
          printf("outp->bcen1.V = %lg (updated)\n", outp->bcen1.V);
        outp->bcen2.V=outp->cen2.V; 
        outp->bcen2.I=((outp->cen2.I)&&(RotI))?1:0;
        if (SxDebug&SX_SHOWTEMP)
          printf("outp->bcen2.V = %lg (updated)\n", outp->bcen2.V);
      }
    } // if ( fabs( TRT[2][2] ) > SxEps )

    if (inv_T) free(inv_T);
    if (T) free(T);
    if (inv_t_order) free(inv_t_order);
    if (t_order) free(t_order);

  }

  errval = SX_SUCCESS;

  if (SxDebug&SX_SHOWDATA) {
     printf("  Output parameters\n");
     sx_pr_params( stdout, outp );
  }

  if (perrval) *perrval=errval;

  if (SxDebug&SX_DEBUG) printf( "sx_tf_params END\n" );

  return( outp );

sx_tf_params_error:

  if (inv_T) free(inv_T);
  if (T) free(T);
  if (inv_t_order) free(inv_t_order);
  if (t_order) free(t_order);
  if (new) free(new);

  if (perrval) *perrval=errval;

  if (SxDebug&SX_DEBUG) 
    printf( "sx_tf_params END (error=%d)\n",errval );

  return( NULL );

} // sx_tf_params

/*--------------------------------------------------------------------------
NAME

  sx_tf_img --- transforms an image to a different orientation

SYNOPSIS

  int sx_tf_img ( SXParams *params_out,  
              void *data_out, void *variance_out, size_t item_number,
              const SXParams *params_in, 
              const void *data_in, const void *variance_in, size_t item_size,
              long ori, int rot, int *perrval );

ARGUMENTS

   SXParams *params_out (o) : output sx params (must be allocated)
   void *data_out       (o) : output data array (must be allocated)
   size_t item_number   (i) : allocated number of array elements (all arrays)
   const SXParams *params_in (i) : input sx params
   const void *data_in  (i) : input data array 
   size_t item_size     (i) : size of a single array element (both arrays)
   long ori             (i) : output raster orientation
   int rot              (i) : use default rotations, if they are not supplied
   int * perrval        (o) : error message

DESCRPTION

  params_in contains the image parameters, data_in the pixel data.
  ori is the orientation of the output image.
  The transformed parameters are written to params_out and the pixel data 
  to data_out.  params_out and data_out must be sufficiently large.
  If data_in or data_out is the NULL pointer, only the parameter are 
  converted. 
  Input (params_in, data_in) and output (params_out, data_out) parameters 
  can be identical.

RETURN VALUE

   0 in case of success
  -1 otherwise

--------------------------------------------------------------------------*/
PUBLIC int
  sx_tf_img ( SXParams *params_out, 
              void *data_out, void *variance_out, size_t item_number,
              const SXParams *params_in, 
              const void *data_in, const void *variance_in, size_t item_size,
              long ori, int rot, int *perrval )

{ int errval;
  long order_in[4], order_out[4], order_inv[4], order_tf[4];
  long data_dim_out[4];
  size_t used_number;
  SXParams params; // copy of *params_in

  if (SxDebug&SX_DEBUG) 
    printf( "sx_tf_img BEGIN\n" );

  if (!params_out) {
    errval=SX_NULL_POINTER;
    goto sx_tf_img_error;
  }

  /* make a copy of params_in */
  if ( !(sx_cp_params ( &params, params_in )) ) {
    errval=SX_COPY_ERROR;
    goto sx_tf_img_error;
  }

  if ( !(sx_tf_params (params_out, &params, ori, rot, &errval)) )
    goto sx_tf_img_error;

  /* reorder arrays */
  if ( ((data_in)&&(data_out))||((variance_in)&&(variance_out)) ) {

    if (SxDebug&SX_DEBUG)
      printf( "  reorder arrays: orientation %ld -> %ld\n",
        params.ori.V, params_out->ori.V );

    if ( !(raster_number2order ( order_in, 4, 3, params.ori.V )) ) {
      errval=SX_INVALID_ORIENTATION;
      goto sx_tf_img_error;
    }

    if (SxDebug&SX_DEBUG)
      printf("    order_in=%ld\n",raster_order2number(order_in));

    if ( (!raster_number2order ( order_out, 4, 3, params_out->ori.V )) ) {
      errval=SX_INVALID_ORIENTATION;
      goto sx_tf_img_error;
    }
    if (SxDebug&SX_DEBUG)
      printf("    order_out=%ld\n",raster_order2number(order_out));

    if ( (!raster_order_inversion ( order_inv, 4, order_out )) ) {
      errval=SX_INVALID_ORIENTATION;
      goto sx_tf_img_error;
    }
    if (SxDebug&SX_DEBUG)
      printf("    order_out_inv=%ld\n",raster_order2number(order_inv));

    if ( (!raster_order_multiplication(order_tf, 4, order_inv, order_in)) ) {
      errval=SX_INVALID_ORIENTATION;
      goto sx_tf_img_error;
    }
    if (SxDebug&SX_DEBUG)
      printf("    order_tf=%ld\n",raster_order2number(order_tf));

    data_dim_out[0]=3l;
    data_dim_out[1]=params_out->dim1.V;
    data_dim_out[2]=params_out->dim2.V;
    data_dim_out[3]=1l; // third dimension is 1

    /* check, that item_number is sufficiently large */
    used_number = (unsigned long) ( data_dim_out[1]*data_dim_out[2] );
    if (item_number<used_number) {
      errval=SX_ARRAY_TOOSMALL;
      goto sx_tf_img_error;
    }

    if ( (data_in)&&(data_out) ) {
      if (SxDebug&SX_DEBUG)
        printf( "  raster_order_normalization\n" );
      if ( raster_order_normalization ( data_out, data_in, data_dim_out, 
                                        order_tf, item_size, &errval) ) {
        errval+=SX_RASTER_ERROR;
        goto sx_tf_img_error;
      }
    }

    if ( (variance_in)&&(variance_out) ) {
      if ( raster_order_normalization ( variance_out, variance_in, data_dim_out,
                                        order_tf, item_size, &errval) ) {
        errval+=SX_RASTER_ERROR;
        goto sx_tf_img_error;
      }
    }
  }

  errval = SX_SUCCESS;

  if (perrval) *perrval=errval;

  if (SxDebug&SX_DEBUG) printf( "sx_tf_img END\n" );

  return(0);

sx_tf_img_error:

  if (perrval) *perrval=errval;

  if (SxDebug&SX_DEBUG)
    printf( "sx_tf_img END (error=%d)\n",errval );

  return( -1 );

} // sx_tf_img

/*--------------------------------------------------------------------------
NAME

  sx_errval2str --- writes error message into a buffer

SYNOPSIS

  char *sx_errval2str( char buffer[], size_t buflen, int errval );

ARGUMENTS

   char buffer[]        (i) : output buffer for message
   size_t buflen        (i) : length of buffer (including terminating NULL)
   int errval           (i) : error message to write
   return char *        (o) : pointer to buffer 

DESCRPTION

  Writes an error message into the buffer corresponding to errval.

  SX_INVALID_PROJECTION, SX_INVALID_ORIENTATION, 
  SX_MEMORY_ALLOCATION_ERROR, SX_ORDER_CALCULATION_ERROR, 
  SX_MATRIX_CALCULATION_ERROR, SX_ANGLE_CALCULATION_ERROR,
  SX_BEAMCENTER_CALCULATION_ERROR, SX_BEAMDISTANCE_CALCULATION_ERROR,
  SX_CENTER_CALCULATION_ERROR, SX_DISTANCE_CALCULATION_ERROR,
  SX_COPY_ERROR

--------------------------------------------------------------------------*/
PUBLIC char *sx_errval2str( char buffer[], size_t buflen, int errval )
{
  char * value;

  value=buffer;

  if ( (buffer)&&(buflen>0) ) {
    switch (errval) {
      case SX_SUCCESS:
             strncpy(buffer,"success",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_NULL_POINTER:
             strncpy(buffer,"NULL pointer",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_INVALID_PROJECTION:
             strncpy(buffer,"invalid projection",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_INVALID_ORIENTATION:
             strncpy(buffer,"invalid orientation",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_INVALID_AXISTYPE:
             strncpy(buffer,"invalid axis type",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_MEMORY_ALLOCATION_ERROR:
             strncpy(buffer,"memory allocation error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_ORDER_CALCULATION_ERROR:
             strncpy(buffer,"order calculation error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_MATRIX_CALCULATION_ERROR:
             strncpy(buffer,"matrix calculation error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_ANGLE_CALCULATION_ERROR:
             strncpy(buffer,"angle calculation error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_BEAMCENTER_CALCULATION_ERROR:
             strncpy(buffer,"beam center calculation error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_BEAMDISTANCE_CALCULATION_ERROR:
             strncpy(buffer,"beam distance calculation error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_CENTER_CALCULATION_ERROR:
             strncpy(buffer,"center calculation error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_DISTANCE_CALCULATION_ERROR:
             strncpy(buffer,"distance calculation error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_COPY_ERROR:
             strncpy(buffer,"copying error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_ARRAY_TOOSMALL:
             strncpy(buffer,"array size too small",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case SX_RASTER_ERROR:
             strncpy(buffer,"raster error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      default:
             strncpy(buffer,"error value",buflen-1);
             buffer[buflen-1]='\0';

    } // switch
  } else value=NULL;

  return( value );

} // sx_errval2str

/****************************************************************************/

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

/*--------------------------------------------------------------------------
NAME

   cmpr.c --- de/compression functions 

DESCRIPTION

   zlib compression/decompression of binary data
   - GzipCompression
   - ZCompression

AUTHOR
  2010-12-19 Peter Boesecke

HISTORY

  2011-01-07 V1.1 PB no gzip compression possible for zlib version<1.2.4, 
                     zlib version checked and error message given,
                     CMPR_DEF_MEM_LEVEL defined with respect to 
                     MAX_MEM_LEVEL, more debug added to cmpr_deflate and 
                     cmpr_inflate.
  2011-01-20 V1.2 PB only info in case of an error for zlib version<1.2.4
  2012-06-24 V1.3 PB cmpr_inflatefile and cmpr_deflatefile added
  2012-06-26 V1.4 PB CMPRUTILS added
  2012-12-29 V1.5 PB zlib defines z_strm.avail_in as uInt =>
                     print format changed from %lu to %u
--------------------------------------------------------------------------*/
# define CMPR_VERSION "cmpr : V1.5 Peter Boesecke 2012-12-29 -- zlib : " ZLIB_VERSION
/****************************************************************************
*  Include                                                                  *
****************************************************************************/
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <ctype.h>
# include <limits.h>
# include <errno.h>
# include <fcntl.h>
# include <math.h>
# include <float.h>

# include "cmpr.h"

/****************************************************************************
*  PRIVATE part                                                             *
****************************************************************************/

#if MAX_MEM_LEVEL >= 8
#  define CMPR_DEF_MEM_LEVEL 8
#else
#  define CMPR_DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif

# define CHUNK 16384

/****************************************************************************
* Static Variables                                                          *
****************************************************************************/
static int CMPR_debug = 0;

/*--------------------------------------------------------------------------
NAME

  cmpr_debug --- set / reset module cmpr into debug mode

SYNOPSIS

  void cmpr_debug ( int debug );

DESCRPTION

  Writes ´debug´ into CMPR_debug.

--------------------------------------------------------------------------*/
void cmpr_debug ( int debug )
{ CMPR_debug = debug;
} /* cmpr_debug */

/*--------------------------------------------------------------------------
NAME

  cmpr_deflate --- compress using zlib

SYNOPSIS

  int cmpr_deflate ( void * out, size_t outlen,
                     const void * inp, size_t inplen,
                     int cmpr_method, size_t * pconverted,
                     int * perrval );

DESCRIPTION

  Compress a maximum of inplen bytes in steps of CHUNK bytes from inp
  to out using zlib deflate routine. The size of the output buffer is
  outlen.

  cmpr_deflate returns 0 on success and -1 on error. The *perrval returns
  more specific information: 

  Z_OK on success,
  Z_MEM_ERROR if memory could not be allocated for processing, 
  Z_DATA_ERROR if the deflate data is invalid or incomplete, 
  Z_VERSION_ERROR if the version of zlib.h and the version of the library
    linked do not match, 
  Z_ERRNO  in case of an output error, e.g. if the output buffer is too short
           or the compression method is not defined.

ARGUMENTS
  void * out           pointer to output buffer
  size_t outlen        length of output buffer in bytes
  const void * inp     pointer to input buffer
  size_t inplen        length of input buffer in bytes
  int cmpr_method      compression method
  size_t * pconverted  number of valid bytes in output buffer
  int * perrval        zlib error value

AUTHOR
  Peter Boesecke 2010-12-12
-----------------------------------------------------------------------------*/
int cmpr_deflate ( void * out, size_t outlen,
                   const void * inp, size_t inplen,
                   int cmpr_method, size_t * pconverted,
                   int * perrval )
{
  int errval, flush;
  z_stream strm;

  unsigned char *pinp;
  unsigned char *pout;

  size_t rest_inp=(size_t) 0, rest_out=(size_t) 0;
  size_t avail_inp=(size_t) 0, avail_out=(size_t) 0;
  size_t incr_inp=(size_t) 0, incr_out=(size_t) 0;
  size_t converted=(size_t) 0;

  if (CMPR_debug) printf("\n cmpr_deflate BEGIN\n");

  if (!out) outlen=(size_t) 0;
  if (!inp) inplen=(size_t) 0;

  /* compress all input */
  errval = Z_STREAM_END;

  if (inplen>0) { /* if there is something to compress */

    /* Compresssion Levels:
       Z_NO_COMPRESSION
       Z_BEST_SPEED
       Z_BEST_COMPRESSION
       Z_DEFAULT_COMPRESSION */

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    switch ( cmpr_method ) {
      case GzipCompression:
        /* initialize gzip compression */
        if (CMPR_debug) {
          printf("   gzip: deflateInit2(strm,level,method,windowBits,memLevel,strategy)\n");
          printf("         deflateInit2(...,%d,%d,%d,%d,%d)\n",
            Z_DEFAULT_COMPRESSION, Z_DEFLATED, (MAX_WBITS+16), 
            CMPR_DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
        }
        errval = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, \
          Z_DEFLATED, (MAX_WBITS+16), \
          CMPR_DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY); // +16 for gzip
        if (errval != Z_OK) {
#if !defined(ZLIB_VERNUM) || (ZLIB_VERNUM < 0x1204)
        /* Versions of zlib < 1.2.4 do not support raw deflate or gzip */
          fprintf(stderr,"cmpr_deflate: zlib library %s < 1.2.4 does not support gzip.\n",
            ZLIB_VERSION);
#endif
          goto cmpr_deflate_error;
        }
        break;
      case ZCompression:
        if (CMPR_debug) {
          printf("   Z: deflateInit(strm,level)\n");
          printf("      deflateInit(...,%d)\n",
            Z_DEFAULT_COMPRESSION);
        }
        /* initialize Z compression */
        errval = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
        if (errval != Z_OK) goto cmpr_deflate_error;
        break;
      default:
        errval=Z_ERRNO;
        goto cmpr_deflate_error1;
    }

    /* compress all input */
    pinp = (unsigned char *) inp;
    rest_inp = inplen;

    pout = (unsigned char *) out;
    rest_out = outlen;

    do {

      /* read not more than CHUNK bytes */
      if ( rest_inp>CHUNK ) {
        /* more input follows */
        avail_inp = CHUNK;
        flush = Z_NO_FLUSH;
      } else { 
        /* input finished */
        avail_inp = rest_inp;
        flush = Z_FINISH;
      }

      strm.avail_in = avail_inp; /* maximum number of bytes to read */
      strm.next_in = pinp; /* pointer to input buffer */

      do {

        /* do not write more than CHUNK bytes */
        avail_out = (rest_out>CHUNK)?CHUNK:rest_out;

        strm.avail_out = avail_out;  /* maximum number of bytes to write */
        strm.next_out = pout; /* pointer to output buffer */

        errval = deflate(&strm, flush); /* no bad return value */
        if ( (errval!=Z_STREAM_END)&&(errval!=Z_OK) ) 
          goto cmpr_deflate_error;

        incr_out = avail_out-strm.avail_out; // number of used bytes 
        rest_out -= incr_out;
        pout += incr_out;
      } while ( (strm.avail_out == 0)&&(rest_out>0) );

      incr_inp = avail_inp-strm.avail_in; /* number of unread bytes */
      rest_inp -= incr_inp;
      pinp += incr_inp;
    } while ( (flush != Z_FINISH)&&(rest_out>0) );

    converted=pout-(unsigned char *) out;

    /* clean up */
    (void)deflateEnd(&strm);

  } // if (inplen>0)

  if ( (errval!=Z_STREAM_END) && (rest_inp>0) && (rest_out==0) ) {
    if (CMPR_debug) {
      printf(" cmpr_deflate: output buffer too short (%lu of %lu bytes used)\n",
        rest_out,outlen);
      printf("              %lu of %lu input bytes unprocessed.\n",
        rest_inp,inplen);
    }
    errval=Z_ERRNO;
    goto cmpr_deflate_error;
  }
  
  if (errval!=Z_STREAM_END) {
    fprintf(stderr, "deflate should report Z_STREAM_END\n");
    errval=Z_DATA_ERROR;
    goto cmpr_deflate_error;
  } else errval=Z_OK;

  if (CMPR_debug) 
    printf(" cmpr_deflate %lu bytes converted END\n",converted);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(0);

cmpr_deflate_error:

  /* clean up and return */
  (void)deflateEnd(&strm);

cmpr_deflate_error1:

  fprintf(stderr,"cmpr_deflate: %s (%s)\n",cmpr_errval2string(errval),
    cmpr_version());
  if (CMPR_debug) 
    printf(" cmpr_deflate END (errval=%d)\n",errval);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(-1);

} // cmpr_deflate

/*--------------------------------------------------------------------------
NAME

  cmpr_inflate --- uncompress using zlib

SYNOPSIS

  int cmpr_inflate ( void * out, size_t outlen,
                     const void * inp, size_t inplen,
                     int cmpr_method, size_t * pconverted,
                     int * perrval );

DESCRIPTION

  Decompress a maximum of inplen bytes in steps of CHUNK bytes from inp
  to out using zlib deflate routine. The size of the output buffer is
  outlen.

  cmpr_inflate returns 0 on success and -1 on error. The *perrval returns
  more specific information: 

  Z_OK on success,
  Z_MEM_ERROR if memory could not be allocated for processing, 
  Z_DATA_ERROR if the deflate data is invalid or incomplete, 
  Z_VERSION_ERROR if the version of zlib.h and the version of the library
    linked do not match, 
  Z_ERRNO  in case of an output error, e.g. if the output buffer is too short
           or the compression method is not defined.

ARGUMENTS
  void * out           pointer to output buffer
  size_t outlen        length of output buffer in bytes
  const void * inp     pointer to input buffer
  size_t inplen        length of input buffer in bytes
  int cmpr_method      compression method
  size_t * pconverted  number of valid bytes in output buffer
  int * perrval        zlib error value

AUTHOR
  Peter Boesecke 2010-12-12
-----------------------------------------------------------------------------*/
int cmpr_inflate ( void * out, size_t outlen,
                   const void * inp, size_t inplen,
                   int cmpr_method, size_t * pconverted,
                   int * perrval )
{
  int errval;
  z_stream strm;

  unsigned char *pinp;
  unsigned char *pout;

  size_t rest_inp=(size_t) 0, rest_out=(size_t) 0;
  size_t avail_inp=(size_t) 0, avail_out=(size_t) 0;
  size_t incr_inp=(size_t) 0, incr_out=(size_t) 0;
  size_t converted=(size_t) 0;

  if (CMPR_debug) printf("\n cmpr_inflate BEGIN\n");

  if (!out) outlen=(size_t) 0;
  if (!inp) inplen=(size_t) 0;

  /* decompress all input */
  errval = Z_STREAM_END;

  if (inplen>0) { /* if there is something to decompress */

    pinp = (unsigned char *) inp;
    rest_inp = inplen;

    pout = (unsigned char *) out;
    rest_out = outlen;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    switch ( cmpr_method ) {
      case GzipCompression:
        /* initialize gzip decompression */
        if (CMPR_debug) {
          printf("   gzip: inflateInit2(strm,windowBits)\n");
          printf("         inflateInit2(...,%d)\n",
            (MAX_WBITS+16));
        }
        errval = inflateInit2(&strm, (MAX_WBITS+16)); // +16 for gzip
        if (errval != Z_OK) {
#if !defined(ZLIB_VERNUM) || (ZLIB_VERNUM < 0x1204)
          /* Versions of zlib < 1.2.4 do not support raw deflate or gzip */
          fprintf(stderr,"cmpr_inflate: zlib library %s < 1.2.4 does not support gzip.\n",
            ZLIB_VERSION);
#endif
          goto cmpr_inflate_error;
        }
        break;
      case ZCompression:
        /* initialize Z decompression */
        if (CMPR_debug) {
          printf("   Z: inflateInit(strm)\n");
          printf("      inflateInit(...)\n");
        }
        errval = inflateInit(&strm);
        if (errval != Z_OK) goto cmpr_inflate_error;
        break;
      default:
        errval=Z_ERRNO;
        goto cmpr_inflate_error1;
    }

    do {
  
      avail_inp = (rest_inp>CHUNK)?CHUNK:rest_inp;

      strm.avail_in = avail_inp;
      strm.next_in = pinp;

      /* run inflate() on input until output buffer not full */
      do {
        avail_out = rest_out>CHUNK?CHUNK:rest_out;

        strm.avail_out = avail_out;
        strm.next_out = pout;

        errval = inflate(&strm, Z_NO_FLUSH);
        switch (errval) {
          case Z_STREAM_ERROR:
            goto cmpr_inflate_error;
          case Z_NEED_DICT:
            errval = Z_DATA_ERROR;  /* and fall through */
          case Z_DATA_ERROR:
          case Z_MEM_ERROR:
            goto cmpr_inflate_error;
        }

        incr_out = avail_out-strm.avail_out;
        rest_out -= incr_out;
        pout += incr_out;
      } while ( (strm.avail_out==0)&&(rest_out>0) );

      incr_inp = avail_inp-strm.avail_in;
      rest_inp -= incr_inp;
      pinp += incr_inp;
    } while ( (errval != Z_STREAM_END)&&(rest_out>0) );

    converted=pout-(unsigned char *) out;
            
    /* clean up */
    (void)inflateEnd(&strm);
  }

  if ( (errval!=Z_STREAM_END) && (rest_inp>0) && (rest_out==0) ) {
    if (CMPR_debug) {
      printf(" cmpr_inflate: output buffer too short (%lu of %lu bytes used)\n",
        rest_out,outlen);
      printf("              %lu of %lu input bytes unprocessed.\n",
        rest_inp,inplen);
    }
    errval=Z_ERRNO;
    goto cmpr_inflate_error;
  }

  if (errval!=Z_STREAM_END) {
    fprintf(stderr, "inflate should report Z_STREAM_END\n");
    errval=Z_DATA_ERROR;
    goto cmpr_inflate_error;
  } else errval=Z_OK;

  if (CMPR_debug)
    printf(" cmpr_inflate %lu bytes converted END\n",converted);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(0);

cmpr_inflate_error:

  /* clean up and return */
  (void)inflateEnd(&strm);

cmpr_inflate_error1:

  fprintf(stderr,"cmpr_inflate: %s (%s)\n",cmpr_errval2string(errval),
    cmpr_version());
  if (CMPR_debug) printf(" cmpr_inflate END (errval=%d)\n",errval);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(-1);

} // cmpr_inflate

/*--------------------------------------------------------------------------
NAME

  cmpr_frinflate --- read from file and uncompress using zlib

SYNOPSIS

  int cmpr_frinflate ( void * out, size_t outlen,
                       FILE * inp, size_t inplen,
                       int cmpr_method, size_t * pconverted,
                       int * perrval );


DESCRIPTION

  Decompress a maximum of inplen bytes in steps of CHUNK bytes from inp
  to out using zlib inflate routine. The size of the output buffer is
  outlen.

  cmpr_frinflate returns 0 on success and -1 on error. The *perrval returns
  more specific information: 

  Z_OK on success,
  Z_MEM_ERROR if memory could not be allocated for processing, 
  Z_DATA_ERROR if the deflate data is invalid or incomplete, 
  Z_VERSION_ERROR if the version of zlib.h and the version of the library
    linked do not match, 
  Z_ERRNO  in case of an output error, e.g. if the output buffer is too short
           or the compression method is not defined.

ARGUMENTS
  void * out           pointer to output buffer
  size_t outlen        length of output buffer in bytes
  FILE * inp           file channel opened for binary read
  size_t inplen        maximum number of bytes to read
  int cmpr_method      compression method
  size_t * pconverted  number of valid bytes in output buffer
  int * perrval        zlib error value

AUTHOR
  Peter Boesecke 2010-12-13
-----------------------------------------------------------------------------*/
int cmpr_frinflate ( void * out, size_t outlen,
                     FILE * inp, size_t inplen,
                     int cmpr_method, size_t * pconverted,
                     int * perrval )
{
  int errval;
  z_stream strm;

  unsigned char *pout;

  unsigned char in[CHUNK];

  size_t rest_inp=(size_t) 0, rest_out=(size_t) 0;
  size_t avail_inp=(size_t) 0, avail_out=(size_t) 0;
  size_t incr_inp=(size_t) 0, incr_out=(size_t) 0;
  size_t converted=(size_t) 0;

  if (CMPR_debug) printf("\n cmpr_frinflate BEGIN\n");

  if (!out) outlen=(size_t) 0;
  if (!inp) inplen=(size_t) 0;

  /* decompress all input */
  errval = Z_STREAM_END;

  if (inplen>0) { /* if there is something to decompress */

    rest_inp = inplen;

    pout = (unsigned char *) out;
    rest_out = outlen;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    switch ( cmpr_method ) {
      case GzipCompression:
        /* initialize gzip decompression */
        if (CMPR_debug) {
          printf("   gzip: inflateInit2(strm,windowBits)\n");
          printf("         inflateInit2(...,%d)\n",
            (MAX_WBITS+16));
        }
        errval = inflateInit2(&strm, (MAX_WBITS+16)); // +16 for gzip
        if (errval != Z_OK) {
#if !defined(ZLIB_VERNUM) || (ZLIB_VERNUM < 0x1204)
          /* Versions of zlib < 1.2.4 do not support raw deflate or gzip */
          fprintf(stderr,"cmpr_frinflate: zlib library %s < 1.2.4 does not support gzip.\n",
            ZLIB_VERSION);
#endif
          goto cmpr_frinflate_error;
        }
        break;
      case ZCompression:
        /* initialize Z decompression */
        if (CMPR_debug) {
          printf("   Z: inflateInit(strm)\n");
          printf("      inflateInit(...)\n");
        }
        errval = inflateInit(&strm);
        if (errval != Z_OK) goto cmpr_frinflate_error;
        break;
      default:
        errval=Z_ERRNO;
        goto cmpr_frinflate_error1;
    }

    do {
  
      avail_inp = (rest_inp>CHUNK)?CHUNK:rest_inp;
      incr_inp = fread(in, 1, avail_inp, inp);
      if (ferror(inp)) {
        errval=Z_ERRNO;
        goto cmpr_frinflate_error;
      }

      strm.avail_in = incr_inp;
      if (strm.avail_in == 0) break;

      strm.next_in = in;

      /* run inflate() on input until output buffer not full */
      do {
        avail_out = rest_out>CHUNK?CHUNK:rest_out;

        strm.avail_out = avail_out;
        strm.next_out = pout;

        errval = inflate(&strm, Z_NO_FLUSH);
        switch (errval) {
          case Z_STREAM_ERROR:
            goto cmpr_frinflate_error;
          case Z_NEED_DICT:
            errval = Z_DATA_ERROR;  /* and fall through */
          case Z_DATA_ERROR:
          case Z_MEM_ERROR:
            goto cmpr_frinflate_error;
        }

        incr_out = avail_out-strm.avail_out;
        rest_out -= incr_out;
        pout += incr_out;
      } while ( (strm.avail_out==0)&&(rest_out>0) );

      rest_inp -= incr_inp;
    } while ( (errval != Z_STREAM_END)&&(rest_out>0) );

    converted=pout-(unsigned char *) out;
            
    /* clean up */
    (void)inflateEnd(&strm);
  }

  if ( (errval!=Z_STREAM_END) && (rest_inp>0) && (rest_out==0) ) {
    if (CMPR_debug) {
      printf(" cmpr_inflate: output buffer too short (%lu of %lu bytes used)\n",
        rest_out,outlen);
      printf("              %lu of %lu input bytes unprocessed.\n",
        rest_inp,inplen);
    }
    errval=Z_ERRNO;
    goto cmpr_frinflate_error;
  }

  if (errval!=Z_STREAM_END) {
    fprintf(stderr, "inflate should report Z_STREAM_END\n");
    errval=Z_DATA_ERROR;
    goto cmpr_frinflate_error;
  } else errval=Z_OK;

  if (CMPR_debug)
    printf(" cmpr_frinflate %lu bytes converted END\n",converted);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(0);

cmpr_frinflate_error:

  /* clean up and return */
  (void)inflateEnd(&strm);

cmpr_frinflate_error1:

  fprintf(stderr,"cmpr_frinflate: %s (%s)\n",cmpr_errval2string(errval),
    cmpr_version());
  if (CMPR_debug) printf(" cmpr_frinflate END (errval=%d)\n",errval);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(-1);

} // cmpr_frinflate

/*--------------------------------------------------------------------------
NAME

  cmpr_inflatefile --- read/write from/to file and uncompress using zlib

SYNOPSIS

  int cmpr_inflatefile ( FILE * outp, FILE * inp,
                         int cmpr_method, size_t * pconverted,
                         int * perrval );


DESCRIPTION

  Decompress the input file inp in steps of CHUNK bytes using zlib inflate 
  routine and write the result to the output file outp.

  cmpr_inflatefile returns 0 on success and -1 on error. The *perrval returns
  more specific information: 

  Z_OK on success,
  Z_MEM_ERROR if memory could not be allocated for processing, 
  Z_DATA_ERROR if the deflate data is invalid or incomplete, 
  Z_VERSION_ERROR if the version of zlib.h and the version of the library
    linked do not match, 
  Z_ERRNO  in case of an output error, e.g. if the output buffer is too short
           or the compression method is not defined.

ARGUMENTS
  FILE * outp          pointer to output buffer
  FILE * inp           file channel opened for binary read
  int cmpr_method      compression method
  size_t * pconverted  number of bytes written to the output file  
  int * perrval        zlib error value

AUTHOR
  Peter Boesecke 2012-06-23
-----------------------------------------------------------------------------*/
int cmpr_inflatefile ( FILE * outp, FILE * inp,
                       int cmpr_method, size_t * pconverted,
                       int * perrval )
{
  int errval;
  z_stream strm;

  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  size_t incr_out=(size_t) 0;
  size_t converted=(size_t) 0;

  if (CMPR_debug) printf("\n cmpr_inflatefile BEGIN\n");

  /* decompress all input */
  errval = Z_STREAM_END;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    switch ( cmpr_method ) {
      case GzipCompression:
        /* initialize gzip decompression */
        if (CMPR_debug) {
          printf("   gzip: inflateInit2(strm,windowBits)\n");
          printf("         inflateInit2(...,%d)\n",
            (MAX_WBITS+16));
        }
        errval = inflateInit2(&strm, (MAX_WBITS+16)); // +16 for gzip
        if (errval != Z_OK) {
#if !defined(ZLIB_VERNUM) || (ZLIB_VERNUM < 0x1204)
          /* Versions of zlib < 1.2.4 do not support raw deflate or gzip */
          fprintf(stderr,"cmpr_inflatefile: zlib library %s < 1.2.4 does not support gzip.\n",
            ZLIB_VERSION);
#endif
          goto cmpr_inflatefile_error;
        }
        break;
      case ZCompression:
        /* initialize Z decompression */
        if (CMPR_debug) {
          printf("   Z: inflateInit(strm)\n");
          printf("      inflateInit(...)\n");
        }
        errval = inflateInit(&strm);
        if (errval != Z_OK) goto cmpr_inflatefile_error;
        break;
      default:
        errval=Z_ERRNO;
        goto cmpr_inflatefile_error1;
    }

    /* decompress until deflate stream ends or end of file */
    do {

      strm.avail_in = fread(in, 1, CHUNK, inp);
      if (ferror(inp)) {
        errval=Z_ERRNO;
        goto cmpr_inflatefile_error;
      }

      if (strm.avail_in == 0) break;

      strm.next_in = in;

      /* run inflate() on input until output buffer not full */
      do {

        strm.avail_out = CHUNK;
        strm.next_out = out;

        errval = inflate(&strm, Z_NO_FLUSH);
        switch (errval) {
          case Z_STREAM_ERROR:
            goto cmpr_inflatefile_error;
          case Z_NEED_DICT:
            errval = Z_DATA_ERROR;
          case Z_DATA_ERROR:
          case Z_MEM_ERROR:
            goto cmpr_inflatefile_error;
        }
        incr_out = CHUNK - strm.avail_out;
        if (fwrite(out, 1, incr_out, outp) != incr_out || ferror(outp)) {
          errval = Z_ERRNO;
          goto cmpr_inflatefile_error;
        }
        converted += incr_out;
      } while (strm.avail_out == 0);

    } while (errval != Z_STREAM_END);

  /* clean up */
  (void)inflateEnd(&strm);

  if (CMPR_debug)
    printf(" cmpr_inflatefile %lu bytes converted END\n",converted);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(0);

cmpr_inflatefile_error:

  /* clean up and return */
  (void)inflateEnd(&strm);

cmpr_inflatefile_error1:

  fprintf(stderr,"cmpr_inflatefile: %s (%s)\n",cmpr_errval2string(errval),
    cmpr_version());
  if (CMPR_debug) printf(" cmpr_inflatefile END (errval=%d)\n",errval);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(-1);

} // cmpr_inflatefile

/*--------------------------------------------------------------------------
NAME

  cmpr_deflatefile --- read/write from/to file and compress using zlib

SYNOPSIS

  int cmpr_deflatefile ( FILE * outp, FILE * inp,
                         int cmpr_method, size_t * pconverted,
                         int * perrval );

DESCRIPTION

  Decompress the input file inp in steps of CHUNK bytes using zlib deflate
  routine and write the result to the output file outp.

  cmpr_deflatefile returns 0 on success and -1 on error. The *perrval returns
  more specific information:

  Z_OK on success,
  Z_MEM_ERROR if memory could not be allocated for processing,
  Z_DATA_ERROR if the deflate data is invalid or incomplete,
  Z_VERSION_ERROR if the version of zlib.h and the version of the library
    linked do not match,
  Z_ERRNO  in case of an output error, e.g. if the output buffer is too short
           or the compression method is not defined.

ARGUMENTS
  FILE * outp          pointer to output buffer
  FILE * inp           file channel opened for binary read
  int cmpr_method      compression method
  size_t * pconverted  number of bytes written to the output file
  int * perrval        zlib error value

AUTHOR
  Peter Boesecke 2012-06-23
-----------------------------------------------------------------------------*/
int cmpr_deflatefile ( FILE * outp, FILE * inp,
                       int cmpr_method, size_t * pconverted,
                       int * perrval )
{

  int errval, flush;
  z_stream strm;

  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  size_t incr_out=(size_t) 0;
  size_t converted=(size_t) 0;

  if (CMPR_debug) printf("\n cmpr_deflatefile BEGIN\n");

  /* Compresssion Levels:
     Z_NO_COMPRESSION
     Z_BEST_SPEED
     Z_BEST_COMPRESSION
     Z_DEFAULT_COMPRESSION */

  /* allocate deflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  switch ( cmpr_method ) {
    case GzipCompression:
      /* initialize gzip compression */
      if (CMPR_debug) {
        printf("   gzip: deflateInit2(strm,level,method,windowBits,memLevel,strategy)\n");
        printf("         deflateInit2(...,%d,%d,%d,%d,%d)\n",
          Z_DEFAULT_COMPRESSION, Z_DEFLATED, (MAX_WBITS+16),
          CMPR_DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
      }
      errval = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, \
        Z_DEFLATED, (MAX_WBITS+16), \
        CMPR_DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY); // +16 for gzip
      if (errval != Z_OK) {
#if !defined(ZLIB_VERNUM) || (ZLIB_VERNUM < 0x1204)
      /* Versions of zlib < 1.2.4 do not support raw deflate or gzip */
        fprintf(stderr,"cmpr_deflate: zlib library %s < 1.2.4 does not support gzip.\n",
          ZLIB_VERSION);
#endif
        goto cmpr_deflatefile_error;
      }
      break;
    case ZCompression:
      if (CMPR_debug) {
        printf("   Z: deflateInit(strm,level)\n");
        printf("      deflateInit(...,%d)\n",
          Z_DEFAULT_COMPRESSION);
      }
      /* initialize Z compression */
      errval = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
      if (errval != Z_OK) goto cmpr_deflatefile_error;
      break;
    default:
      errval=Z_ERRNO;
      goto cmpr_deflatefile_error1;
  }

  /* compress until end of file */
  errval = Z_STREAM_END;
  do {
      strm.avail_in = fread(in, 1, CHUNK, inp);
      if (ferror(inp)) {
          errval = Z_ERRNO;
          goto cmpr_deflatefile_error;
      }
      flush = feof(inp) ? Z_FINISH : Z_NO_FLUSH;
      strm.next_in = in;

      /* run deflate() on input until output buffer not full, finish
         compression if all of inp has been read */
      do {
          strm.avail_out = CHUNK;
          strm.next_out = out;
          errval = deflate(&strm, flush);  /* no bad return value */
          if (errval==Z_STREAM_ERROR) 
            goto cmpr_deflatefile_error;

          incr_out = CHUNK - strm.avail_out;
          if (fwrite(out, 1, incr_out, outp) != incr_out || ferror(outp)) {
              (void)deflateEnd(&strm);
              errval = Z_ERRNO;
              goto cmpr_deflatefile_error;
          }
          converted += incr_out; 
      } while (strm.avail_out == 0);

      if (strm.avail_in!=0) {
        /* all input must be used */
        if (CMPR_debug) {
          printf(" cmpr_deflatefile: not all input deflated (%u bytes unprocessed)\n",
            strm.avail_in);
        }
        errval=Z_ERRNO;
        goto cmpr_deflatefile_error;
      }

      /* done when last data in file processed */
  } while (flush != Z_FINISH);

  /* clean up */
  (void)deflateEnd(&strm);

  if (errval!=Z_STREAM_END) {
    fprintf(stderr, "deflate should report Z_STREAM_END\n");
    errval=Z_DATA_ERROR;
    goto cmpr_deflatefile_error;
  } else errval=Z_OK;

  if (CMPR_debug)
    printf(" cmpr_deflatefile %lu bytes converted END\n",converted);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(0);

cmpr_deflatefile_error:

  /* clean up and return */
  (void)deflateEnd(&strm);

cmpr_deflatefile_error1:

  fprintf(stderr,"cmpr_deflatefile: %s (%s)\n",cmpr_errval2string(errval),
    cmpr_version());
  if (CMPR_debug) printf(" cmpr_deflatefile END (errval=%d)\n",errval);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(-1);

} // cmpr_deflatefile

/*--------------------------------------------------------------------------
NAME

  cmpr_errval2string --- convert zlib error value to string

SYNOPSIS

  const char * cmpr_errval2string(int errval);

DESCRIPTION

ARGUMENTS
  int errval zlib error value

RETURN VALUE

  const char * : error value converted to string

AUTHOR
  Peter Boesecke 2010-12-12
-----------------------------------------------------------------------------*/
const char * cmpr_errval2string(int errval)
{
    switch (errval) {
    case Z_ERRNO:
         return("error reading or writing data buffer");
        break;
    case Z_STREAM_ERROR:
        return("invalid compression level");
        break;
    case Z_DATA_ERROR:
        return("invalid or incomplete deflate data");
        break;
    case Z_MEM_ERROR:
        return("out of memory");
        break;
    case Z_VERSION_ERROR:
        return("zlib version mismatch!");
    }
  return("");
} // cmpr_errval2string

const char *cmpr_version ( void )
{ return(CMPR_VERSION); }

/****************************************************************************
*  CMPRUTILS                                                                *
****************************************************************************/
/****************************************************************************
*  Additional include files                                                 *
****************************************************************************/
# include <limits.h>
# include "strlib.h"
# include "filename.h"
# include "numio.h"

#ifdef PATH_MAX
# define CMPR_BUFLEN PATH_MAX
#else
# ifdef MAX_FNAME
#  define CMPR_BUFLEN MAX_FNAME
# else
#  define CMPR_BUFLEN 1024
# endif
#endif

// returns the compression type
int cmpr_checkextension(const char *extension)
{ int cmpr=UnCompressed;
  if (!strlib_ncasecmp(extension,"Z",1)) cmpr = ZCompression; // decompress
  else if (!strlib_ncasecmp(extension,"gz",2)) cmpr = GzipCompression; // decompress
  return(cmpr);
} // cmpr_checkextension

// returns the file compression type
int cmpr_checkfiletype(const char *filename)
{ int cmpr=0;
  char extension[CMPR_BUFLEN];
  filename_extension(extension,CMPR_BUFLEN,filename);
  cmpr = cmpr_checkextension(extension);
  return( cmpr );
} // cmpr_checkfiletype

// appends (or removes) the compression extension
char * cmpr_filename(char * buffer, size_t buflen, const char *filename, int cmpr)
{ char * pb;
  size_t blen;
  if ( cmpr_checkfiletype(filename) > 1 ) {
    // copy filename to buffer without extension 
    filename_body ( buffer, buflen, filename );
  } else {
    // copy filename to buffer
    strncpy( buffer, filename, buflen );
  }

  buffer[buflen-1] = '\0'; // force terminating NULL

  pb = buffer+strlen(buffer);
  blen = buflen-strlen(buffer);
  switch (cmpr) {
    case GzipCompression:
      strncpy( pb, ".gz", blen );
      break;
    case ZCompression:
      strncpy( pb, ".Z", blen );
      break;
  }

  buffer[buflen-1] = '\0'; // force terminating NULL

  return( buffer );

} // cmpr_filename

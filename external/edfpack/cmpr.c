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
--------------------------------------------------------------------------*/
# define CMPR_VERSION "cmpr : V1.2 Peter Boesecke 2011-01-20 -- zlib : " ZLIB_VERSION
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
        goto cmpr_deflate_error;
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
      printf(" cmpr_deflate: output buffer too short (%zu of %zu bytes used)\n",
        rest_out,outlen);
      printf("              %zu of %zu input bytes unprocessed.\n",
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

  if (CMPR_debug) printf(" cmpr_deflate END\n");

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(0);

cmpr_deflate_error:

  /* clean up and return */
  (void)deflateEnd(&strm);

  fprintf(stderr,"cmpr_deflate: %s (%s)\n",cmpr_errval2string(errval),
    cmpr_version());
  if (CMPR_debug) printf(" cmpr_deflate END (errval=%d)\n",errval);

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
        goto cmpr_inflate_error;
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
      printf(" cmpr_inflate: output buffer too short (%zu of %zu bytes used)\n",
        rest_out,outlen);
      printf("              %zu of %zu input bytes unprocessed.\n",
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

  if (CMPR_debug) printf(" cmpr_inflate END\n");

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(0);

cmpr_inflate_error:

  /* clean up and return */
  (void)inflateEnd(&strm);

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
  to out using zlib deflate routine. The size of the output buffer is
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
        goto cmpr_frinflate_error;
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
      printf(" cmpr_inflate: output buffer too short (%zu of %zu bytes used)\n",
        rest_out,outlen);
      printf("              %zu of %zu input bytes unprocessed.\n",
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

  if (CMPR_debug) printf(" cmpr_inflate END\n");

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(0);

cmpr_frinflate_error:

  /* clean up and return */
  (void)inflateEnd(&strm);

  fprintf(stderr,"cmpr_frinflate: %s (%s)\n",cmpr_errval2string(errval),
    cmpr_version());
  if (CMPR_debug) printf(" cmpr_frinflate END (errval=%d)\n",errval);

  if (pconverted) *pconverted=converted;
  if (perrval) *perrval=errval;

  return(-1);

} // cmpr_frinflate

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
  return(""); // to make compiler happy
} // cmpr_errval2string

const char *cmpr_version ( void )
{ return(CMPR_VERSION); }

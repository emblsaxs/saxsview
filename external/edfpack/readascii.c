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

# define READASCII_VERSION "readascii : V1.0 2007-09-07 Peter Boesecke"
/*+++***********************************************************************
NAME

    readascii.c

SYNOPSIS

    #include "readascii.h"

INCLUDE FILES
  numio.h 
  edfio.h 
  filename.h 

TO LINK WITH
  standard C libraries
  edfio, bslio, numio, gamma, filename

DESCRIPTION
    Header of the module "readascii.c"

PURPOSE
  Conversion of ascii tables to float
  See PUBLIC functions for detail.

PUBLIC ROUTINES

  // open, read and close a file
  RADescr *RA_openfile(const char *filename, int verbose, int *pstatus);
  long RA_readfile( RADescr * descr, float ** pdata, int * pstatus );
  void RA_closefile( RADescr * descr, int * pstatus );

  // set special parameters, defaults are used, if not set
  int RA_setbskp( RADescr * descr, long bskp );
  int RA_setlskp( RADescr * descr, long lskp );
  int RA_setcskp( RADescr * descr, long cskp );
  int RA_setskipcol( RADescr * descr, long skipcol );
  int RA_setskiprow( RADescr * descr, long skiprow );
  int RA_setdim1( RADescr * descr, long dim1 );
  int RA_setdim2( RADescr * descr, long dim2 );
  int RA_setori( RADescr * descr, long ori );
  int RA_setdummy( RADescr * descr, float dummy );
  int RA_setdelimiterset( RADescr * descr, const char *delimiterset );
  int RA_setcommentset( RADescr * descr, const char *commentset );

  // return version string
  const char * RA_version ( void );

AUTHOR
  2007 Peter Boesecke (PB)
  
HISTORY  
  2007-09-07 V1.0 PB 
***************************************************************************/

/***************************************************************************
* Private Include                                                          *
***************************************************************************/
# include "readascii.h"

/***************************************************************************
* Private Definitions                                                      *
***************************************************************************/
/* Elements longer than RA_MAXELEMENTWIDTH-1 are truncated */
# define RA_MAXELEMENTWIDTH 1024

/***************************************************************************
* PRIVATE Constants and Variables                                          *
***************************************************************************/
PRIVATE char RA_white_spaces[7] = { ' ', '\t', '\r', '\n', '\f', '\v', '\0' };
PRIVATE char RA_new_line[2] = { '\n', '\0' };
PRIVATE int RAInit = 0;
PRIVATE RADescr * RA_root = (RADescr *) NULL;

/***************************************************************************/

/****************************************************************************
* Routines                                                                  *
****************************************************************************/

/*+++------------------------------------------------------------------------
NAME

   RA_version --- return version string

SYNOPSIS

   const char * RA_version ( void );

DESCRIPTION

RETURN VALUE
   Pointer to a constant character string containing the version of
   this module.

----------------------------------------------------------------------------*/
PUBLIC const char * RA_version ( void )
{ return(READASCII_VERSION);
} // RA_version

char * RA_newstr( const char * string )
{ char * newstring;

  if (!(newstring = (char *) malloc(strlen(string)+1))) return((char *) NULL);
  (void) strcpy(newstring,string);

  return( newstring );

} /* RA_newstr */

/*---------------------------------------------------------------------------
NAME

    RA_is_member

SYNOPSIS

    int RA_is_member ( const char * chargroup, char c );

DESCRIPTION

Returns 1 if c is contained in the string chargroup, 0, if not.

HISTORY
18-Jan-1998 Peter Boesecke
---------------------------------------------------------------------------*/
int RA_is_member ( const char * chargroup, char c )
{
   if ( strchr( chargroup, (int) c ) )
     return(1); /* is member */ else return(0); /* is not member */

} /* RA_is_member */

/*---------------------------------------------------------------------------
NAME

   RA_init()

SYNOPSIS

DESCRIPTION

RETURN VALUES
Returns 0 in case of success and -1 if no success.
--------------------------------------------------------------------------*/
int RA_init( void )
{ RA_root = (RADescr *) NULL;
  RAInit  = 1;
  return(0);
} /* RA_init */

int RA_descr_new  ( RADescr ** proot, const char * key, RADescr ** pdescr )
{
  RADescr * newhlist, * previous, * next;
  int     notfound = 1;

  /* search for key */
  previous = (RADescr *) NULL;
  next = *proot;

  *pdescr = (RADescr *) NULL;

  /* search insertion point (insertion before next) */
  while( ( next!=(RADescr *) NULL ) && (notfound>0) ) {
    notfound = strcmp(next->Key,key);
    if (notfound>0) {previous = next; next = next->Next;}
  }

  if (notfound) {
    /* create new hlist */
    if (!(newhlist = (RADescr *) malloc(sizeof(RADescr)))) return(-1);
    newhlist->Key = RA_newstr(key); if (!newhlist->Key) return(-1);

    /* insert newhlist before next */
    if (next) next->Previous = newhlist;
    newhlist->Next=next;
    newhlist->Previous=previous;
    if (previous) previous->Next=newhlist;
      else *proot = newhlist;

    next = newhlist;
  }

  next->in=(FILE *) NULL;
  next->verbose=0;
  next->filename=RA_newstr(key);

  // -1 == "undetermined/not set"
  next->bskp=-1;
  next->lskp=-1;
  next->cskp=-1;
  next->skipcol=-1;
  next->skiprow=-1;
  next->dim1=-1;
  next->dim2=-1;
  next->cnt=-1;
  next->ori=1;
  next->dummy=0.0;
  next->delimiterset=RA_newstr(RA_white_spaces);
  next->commentset=RA_newstr("#");
  next->buffer=(float *) NULL;
  next->bufsiz=(size_t) NULL;

  *pdescr = next;

  return(0);

} /* RA_descr_new */

int RA_descr_remove  ( RADescr ** proot, RADescr * descr )
{
  RADescr *current, *previous, *next;

  next = descr;
  if ( next!=(RADescr *) NULL ) { 

    current  = next;
    previous = current->Previous;
    next     = current->Next;

    // unlink descr
    if (next) next->Previous = previous;
    if (previous) previous->Next = next;
      else *proot = next;

    // release memory of descr
    if (current->filename) free(current->filename);
    if (current->Key) free(current->Key);

    if (current->buffer) free( current->buffer );
    if (current->delimiterset) free( current->delimiterset );
    if (current->commentset) free( current->commentset );

    free ( current );

  }

  return(0);

} /* RA_descr_remove */

RADescr * RA_new_descr ( const char * key, int verbose )
{ RADescr * descr = (RADescr *) NULL;

  if (!RAInit) RA_init();

  RA_descr_new  ( &RA_root, key, &descr );
  if (!descr) return(descr);

  descr->verbose = verbose;

  return( descr );

} /* RA_new_descr */

int RA_rm_descr ( RADescr * descr )
{ 
  int status=0, verbose;

  if (!RAInit) RA_init();

  verbose = descr->verbose;

  if ( (status=RA_descr_remove( &RA_root, descr )) ) return(status);

  return(status);

} /* RA_rm_descr */

void _fprintset( FILE * out, const char * set )
{ const char *ps;
  ps=set;
  fprintf( out, ">>" );
  while (*ps) {
    if ((int) *ps<32) fprintf( out, "\'%02x\'",(int) *ps);
    else fprintf( out, "%c",*ps);
    ps++;
  }
  fprintf( out, "<<\n");
} // _fprintset

int RA_printdescr( FILE * out, RADescr * descr )
{
  fprintf(out,"            in=%p\n",descr->in);
  fprintf(out,"       verbose=%d\n",descr->verbose);
  if (descr->filename)
    fprintf(out,"    filename=>>%s<<\n",descr->filename);
  else fprintf(out,"    filename=(null)\n");
  fprintf(out,"        bskp=%ld\n",descr->bskp);
  fprintf(out,"        lskp=%ld\n",descr->lskp);
  fprintf(out,"        cskp=%ld\n",descr->cskp);
  fprintf(out,"     skipcol=%ld\n",descr->skipcol);
  fprintf(out,"     skiprow=%ld\n",descr->skiprow);
  fprintf(out,"        dim1=%ld\n",descr->dim1);
  fprintf(out,"        dim2=%ld\n",descr->dim2);
  fprintf(out,"         cnt=%ld\n",descr->cnt);
  fprintf(out,"         ori=%ld\n",descr->ori);
  fprintf(out,"       dummy=%g\n",descr->dummy);
  fprintf(out,"  commentset=");_fprintset( out,descr->commentset );
  fprintf(out,"delimiterset=");_fprintset( out,descr->delimiterset );
  fprintf(out,"      bufsiz=%lu\n",descr->bufsiz);
  fprintf(out,"      buffer=%p\n",descr->buffer);
  return(0);
} // RA_printdescr

/*+++------------------------------------------------------------------------
NAME
  RA_ReadElement --- copies the next element into buffer

SYNOPSIS

  char * RA_ReadElement  (  FILE * in, const char * delimiterset, 
                            const char * commentset,
                            char * buffer, size_t buflen, long *pnlcnt );

DESCRIPTION

  Leading and trailing delimiters are skipped. It reads the next element from 
  'in', copies it into buffer and positions the file pointer at the start of 
  the next element or at the first character after a trailing new line. If 
  buflen is 0, the next element is just skipped and buffer is not filled. 
  In this case buffer is not accessed. In both cases the pointer to buffer 
  is returned or NULL in case of an error.
  If a comment character is read the rest of the line is skipped.

RETURN VALUES

  NULL : eof or error
  pointer to buffer
  *pnlcnt : incremented by 1 if a new line follows the read element

----------------------------------------------------------------------------*/
char * RA_ReadElement  (  FILE * in, const char * delimiterset, 
                          const char * commentset,
                          char * buffer, size_t buflen, long *pnlcnt )
{ int c;
  long i=0;
  size_t csize;
  long newlinecount=0;

  csize = sizeof(char);

  if (buflen>=csize) buffer[0] = '\0';

  c = fgetc(in);
  if (ferror(in)) return((char*) NULL);
  if (feof(in)) return(buffer);

  while ((RA_is_member ( delimiterset, c ))||(RA_is_member ( commentset, c ))) {
    // skip all leading delimiters, do not count leading line feeds
    while (RA_is_member ( delimiterset, c )) {
      c = fgetc(in);
      if (ferror(in)) return((char*) NULL);
      if (feof(in)) return(buffer);
    }
    if (RA_is_member ( commentset, c )) {
      // skip rest of the line
      do {
        c = fgetc(in);
        if (ferror(in)) return((char*) NULL);
        if (feof(in)) return(buffer);
      } while (!RA_is_member ( RA_new_line, c ));
    }
  }

  // copy input to buffer until next delimiter or comment
  while (!(RA_is_member ( delimiterset, c )||(RA_is_member ( commentset, c )))) {
    if ((csize*i)<buflen) buffer[i++] = (char) c;
    c = fgetc(in);
    if (ferror(in)) return((char*) NULL);
    if (feof(in)) break;
  }
  if (buflen>(csize*i)) buffer[i] = '\0';
  else if ((buflen>=csize)&&(buflen>csize*(i-1))) buffer[i-1] = '\0';

  // skip all trailing delimiters and comments, stop after RA_new_line or at feof
  while (RA_is_member ( delimiterset, c )) {
    if (RA_is_member ( RA_new_line, c )) break;
    c = fgetc(in);
    if (ferror(in)) return((char*) NULL);
    if (feof(in)) break;
  }
  if (RA_is_member ( commentset, c )) {
    // skip rest of the line
    do {
      c = fgetc(in);
      if (ferror(in)) return((char*) NULL);
      if (feof(in)) break;
    } while (!RA_is_member ( RA_new_line, c ));
  }

  if ((RA_is_member ( RA_new_line, c ))||(feof(in))) newlinecount++;

  if (!RA_is_member ( delimiterset, c )) ungetc(c,in);
  if (ferror(in)) return((char*) NULL);

  if (pnlcnt) *pnlcnt+=newlinecount;

  return( buffer );

} // RA_ReadElement

/*+++------------------------------------------------------------------------
NAME
  RA_skipchar --- Skips character in delimiter skp times
  
SYNOPSIS

  int RA_skipchar  (  FILE * in, const char *delimiter, long skp );
  
DESCRIPTION

  Skips all file input until any of the characters in delimiter have been 
  read skp times. The file pointer is positioned after the last occurence 
  of this character.
  
RETURN VALUES

  0: OK, otherwise not found or other error

----------------------------------------------------------------------------*/
int RA_skipchar (  FILE * in, const char * delimiterset, long skp )
{ int c=(int) ' ';
  long i=0;

  if (skp>0) {
    for (i=0;i<skp;i++) {
      do { // skip all input until next delimiter
        c = fgetc(in);
        if (ferror(in)) return(-1);
        if (feof(in)) return(-1); 
      } while (!RA_is_member ( delimiterset, c ));
    }
    if (!RA_is_member ( delimiterset, c )) ungetc(c,in);
    if (ferror(in)) {
      perror("RA_skipchar");return(-1);
    }
  }

  return( 0 );

} // RA_skipchar 

/*+++------------------------------------------------------------------------
NAME

  RA_openfile --- Open an ASCII file
    
SYNOPSIS 

  RADescr * RA_openfile( const char * filename, int verbose, int * pstatus );

DESCRIPTION

  Opens a file to read and generates a file descriptor. 

RETURN VALUES

  In case of success the pointer to the generated file descriptor is
  returned, otherwise NULL.

----------------------------------------------------------------------------*/
PUBLIC RADescr * RA_openfile( const char * filename, int verbose, int * pstatus )
{ RADescr * descr = NULL;

  if (pstatus) *pstatus = -1;

  if (verbose>1) printf("RA_openfile BEGIN\n");

  if ( (!filename) || (strlen(filename)<=0) ) return(NULL);

  // set descr to default
  descr = RA_new_descr ( filename, verbose );
  if (!descr) return(NULL);

  descr->in = fopen(filename,"r");
  if (descr->in == (FILE *) NULL) {
    RA_rm_descr(descr);
    return(NULL);
  }

  if (pstatus) *pstatus = 0;

  if (verbose>1) printf("RA_openfile END\n");

  return(descr);

} // RA_openfile

/*+++------------------------------------------------------------------------
NAME

  RA_closefile --- Close the file 

SYNOPSIS 

  void RA_closefile( RADescr * descr, int * pstatus );
  
DESCRIPTION

  Closes the file and releases all allocated memory.

RETURN VALUE

  void

----------------------------------------------------------------------------*/
PUBLIC void RA_closefile( RADescr * descr, int * pstatus )
{
  if (pstatus) *pstatus = -1;
  if (descr->verbose>1) printf("RA_closefile BEGIN\n");
  if (descr) {
    fclose(descr->in);
    if (RA_rm_descr( descr )) return;
  }
  if (pstatus) *pstatus = 0;
  if (descr->verbose>1) printf("RA_closefile END\n");

} // RA_closefile

/*+++------------------------------------------------------------------------
NAME

  RA_readfile --- Read ascii table from file and convert to float 

SYNOPSIS

  long RA_readfile( RADescr * descr, float ** pdata, int * pstatus )

DESCRIPTION

  Reads data from the input file according to the file descriptor. An
  output data buffer *pdata is allocated and must be released with 
  RA_closefile.

RETURN VALUES
  In case of success the number of read elements is returned.

----------------------------------------------------------------------------*/
PUBLIC long RA_readfile( RADescr * descr, float ** pdata, int * pstatus )
{
  long nread=0, cnt=0, maxcnt=-1, newlinecnt=0, linecnt=0;
  long itemcnt=0, ipl=-1, maxipl=-1, minipl=-1;
  long rowcnt, colcnt;
  long startpos;
  float tmp;
  size_t bufsiz;
  int errval;

  char buffer1[RA_MAXELEMENTWIDTH];
  char * C1;

  if (pstatus) *pstatus = -1;

  if (descr->verbose>1) printf("RA_readfile BEGIN\n");

  if (descr->verbose>3) RA_printdescr( stdout, descr );

  if (!descr->in) return(nread);

  if (descr->ori<1) descr->ori=1;

  if (!descr->delimiterset) descr->delimiterset=RA_newstr(RA_white_spaces);
  if (!descr->commentset) descr->commentset=RA_newstr("#");

  if ((descr->dim1>0)&&(descr->dim2>0)) maxcnt = descr->dim1*descr->dim2;

  // two passes

  if (descr->verbose>2) printf("1st pass\n");

  // search start position, skip bskp bytes 
  if (descr->bskp<0) descr->bskp=0;
  if ( fseek( descr->in, descr->bskp, SEEK_SET) ) {
      perror("RA_readfile->fseek(SEEK_SET)"); return(nread); 
  }

  // skip lskp lines
  if (descr->lskp<0) descr->lskp=0;
  if (RA_skipchar (  descr->in, RA_new_line, descr->lskp )) return(nread);

  // skip cskp bytes
  if (descr->cskp<0) descr->cskp=0;
  if ( fseek( descr->in, descr->cskp, SEEK_CUR) ) {
      perror("RA_readfile->fseek(SEEK_CUR)"); return(nread);
  }

  // remember current position as start position
  startpos = ftell(descr->in);
  if (startpos == -1l) {
      perror("RA_readfile->ftell"); return(nread);
  }

  if (descr->skipcol<0) descr->skipcol=0;
  if (descr->skiprow<0) descr->skiprow=0;

  // 1st pass (if number of items is not specified in options)
  // count number of items (stop counting at eof)

  linecnt=newlinecnt;
  if (maxcnt<0) {
    // skip rows
    for (rowcnt=0;rowcnt<descr->skiprow;) {
      C1 = RA_ReadElement( descr->in, descr->delimiterset, descr->commentset,
                           buffer1, (size_t) 0, &rowcnt );
      if (!C1) return(nread);
      if (feof(descr->in)) break;
    }

    // count all elements until eof
    colcnt=0;
    for (maxcnt=0;;) {
      if (linecnt!=newlinecnt) {
        ipl=itemcnt; itemcnt=0;linecnt=newlinecnt;
        if (maxipl<0) maxipl=ipl; else if (ipl>maxipl) maxipl=ipl;
        if (minipl<0) minipl=ipl; else if (ipl<minipl) minipl=ipl;
        colcnt=0;
      }

      C1 = RA_ReadElement( descr->in, descr->delimiterset, descr->commentset,
                           buffer1, (size_t) 0, &newlinecnt );
      if (!C1) return(nread);   
      if (feof(descr->in)) break;
      // Do not count the the skipped columns
      if (colcnt>=descr->skipcol) {
        itemcnt++;
        maxcnt++;
      }
      colcnt++;
    }

    if ((descr->dim1<0)&&(descr->dim2<0)) {
      // guess dimensions
      if ((maxipl==minipl)&&(maxipl*newlinecnt==maxcnt)) {
        descr->dim1 = maxipl;
        descr->dim2 = newlinecnt;
      } else {
        descr->dim1 = 1;
        descr->dim2 = maxcnt;
      }
    } else {
      // calculate missing dimension
      if ((descr->dim1<0)&&(descr->dim2>0)) {
        if (maxcnt%descr->dim2)
          descr->dim1 = maxcnt/descr->dim2+1; 
        else descr->dim1 = maxcnt/descr->dim2; 
      } else if ((descr->dim2<0)&&(descr->dim1>0)) {
        if (maxcnt%descr->dim1)
          descr->dim2 = maxcnt/descr->dim1+1;
        else descr->dim2 = maxcnt/descr->dim1;
      } 
    }
    maxcnt = descr->dim1*descr->dim2;
  } // if (maxcnt<0) ...

  // 2nd pass (always)

  if (descr->verbose>2) printf("2nd pass\n");
  if (descr->verbose>3) RA_printdescr( stdout, descr );
  if (descr->verbose>0) printf("  Reading file \'%s\' dim=%ldx%ld\n",
    descr->filename,descr->dim1,descr->dim2);

  // search start position 
  if ( fseek( descr->in, startpos, SEEK_SET) ) {
      perror("RA_readfile->fseek(startpos)"); return(nread);
  }

  // allocate output buffer
  
  bufsiz = sizeof(float)*maxcnt;
  if (bufsiz) {
    descr->buffer = (float *) malloc( bufsiz );
    if (descr->buffer) {
      descr->bufsiz = bufsiz;
      if (pdata) *pdata=descr->buffer;
    } else return(nread);
  }

  // fill buffer with dummies
  for (cnt=0;cnt<maxcnt;cnt++) {
     (descr->buffer)[cnt] = descr->dummy;
  }

  // skip rows
  for (rowcnt=0;rowcnt<descr->skiprow;) {
    C1 = RA_ReadElement( descr->in, descr->delimiterset, descr->commentset,
                         buffer1, (size_t) 0, &rowcnt );
    if (!C1) return(nread);
    if (feof(descr->in)) break;
  }

  // read items and copy to output buffer 
  colcnt=0;rowcnt=0;
  for (cnt=0;cnt<maxcnt;) {
    if (rowcnt!=0) { colcnt=0; rowcnt=0; }

    C1 = RA_ReadElement( descr->in, descr->delimiterset, descr->commentset, 
                         buffer1, RA_MAXELEMENTWIDTH, &rowcnt );
    if (!C1) return(nread);
    if (feof(descr->in)) break;

    // Do not use the the skipped columns
    if (colcnt>=descr->skipcol) {
      if (descr->verbose>3) printf("C1=%s\n",C1);
      if (cnt<maxcnt) {
        if (descr->buffer) {
          tmp=num_str2double ( C1, NULL, &errval);
          if (!errval) {
            (descr->buffer)[cnt] = tmp;
          }
          nread++;
        }
      }
      cnt++;
    }
    colcnt++;

  }

  descr->cnt=cnt;

  if (descr->verbose>1) {
    printf("    Number of lines = %ld\n",newlinecnt); 
    printf("    Total number of items = %ld\n",cnt);
    printf("    Minimum/Maximum number of items per line = %ld/%ld\n",
      minipl,maxipl); 
    printf("    Dim_1=%ld, Dim_2=%ld\n",descr->dim1,descr->dim2);
    if (descr->verbose>2) RA_printdescr( stdout, descr );
  }

  if (pstatus) *pstatus = 0;
  if (descr->verbose>1) printf("RA_readfile END\n");

  return(nread);
  
} // RA_readfile

PUBLIC int RA_setbskp( RADescr * descr, long bskp )
{ if (bskp>=0) descr->bskp      = bskp;
  return(0);
} // RA_setbskp

PUBLIC int RA_setlskp( RADescr * descr, long lskp )
{ if (lskp>=0) descr->lskp      = lskp;
  return(0);
} // RA_setlskp

PUBLIC int RA_setcskp( RADescr * descr, long cskp )
{ if (cskp>=0) descr->cskp      = cskp;
  return(0);
} // RA_setcskp

PUBLIC int RA_setskiprow( RADescr * descr, long skiprow )
{ if (skiprow>=0) descr->skiprow      = skiprow;
  return(0);
} // RA_setskiprow

PUBLIC int RA_setskipcol( RADescr * descr, long skipcol )
{ if (skipcol>=0) descr->skipcol      = skipcol;
  return(0);
} // RA_setskipcol

PUBLIC int RA_setdim1( RADescr * descr, long dim1 )
{ if (dim1>=0) descr->dim1      = dim1;
  return(0);
} // RA_setdim1

PUBLIC int RA_setdim2( RADescr * descr, long dim2 )
{ if (dim2>=0) descr->dim2      = dim2;
  return(0);
} // RA_setdim2

PUBLIC int RA_setori( RADescr * descr, long ori )
{ if (ori>=1) descr->ori      = ori;
  return(0);
} // RA_setori

PUBLIC int RA_setdummy( RADescr * descr, float dummy )
{ if (dummy!=0.0) descr->dummy      = dummy;
  return(0);
} // RA_setdummy

PUBLIC int RA_setdelimiterset( RADescr * descr, const char *delimiterset )
{ if (delimiterset) {
    if (descr->delimiterset) free(descr->delimiterset);
    descr->delimiterset = RA_newstr(delimiterset);
    if (!(descr->delimiterset)) return(1);
  }
  return(0);
} // RA_setdelimiterset

PUBLIC int RA_setcommentset( RADescr * descr, const char *commentset )
{ if (commentset) {
    if (descr->commentset) free(descr->commentset);
    descr->commentset = RA_newstr(commentset);
    if (!(descr->commentset)) return(1);
  }
  return(0);
} // RA_setcommentset

/****************************************************************************/

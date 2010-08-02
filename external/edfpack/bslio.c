/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 1998-2010 European Synchrotron Radiation Facility
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

# define BSLIO_VERSION      "bslio : V0.64 Peter Boesecke 2007-11-23"
/*+++***********************************************************************
NAME

   bslio.c

SYNOPSIS

   #include "edfio.h"

   typedef struct Bsl_Data_Specification {
     char          *BinaryFileName;                 // defined in edfio 
     unsigned long BinaryFilePos;                   // defined in edfio 
     unsigned long BinaryFileLen;                   // defined in edfio
     void          *Data;                           // defined in edfio 
     long          *Dim;                            // defined in edfio 
     int           DataType;                        // defined in edfio
     int           ByteOrder;                       // defined in edfio
     long          RasterConfiguration;             // defined in edfio
     int           Compression;                     // defined in edfio
   } BslDataSpec;

   extern int
      open_bsl_file ( const char * FileName , const char * mode ),
      close_bsl_file ( int stream );
      bsl_memory_range( int stream, long * minmem, long *maxmem ); 
      bsl_frame_range( int stream, long memnum, long * minfra, long * maxfra );

   extern BslDataSpec
      *read_bsl_data_spec ( int stream, long memnum, long franum );

   extern void
      print_bsl_data_spec ( FILE * out, const BslDataSpec * data_spec ),
      print_bsl_filetable ( FILE * out, int level, int verbose ),
      read_bsl_file_headers ( int stream, char **first_header,
                                                 char **second_header );

   extern char
      char *bslio_version ( void );

DESCRIPTION

   Library for access to bsl and otoko files. Public routines are defined
   in "bslio.h"


***************************************************************************/

/***************************************************************************
* Private part                                                             *
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
# include <unistd.h>
#endif

#include <ctype.h>

#ifdef sun      /* sun specific stuff */
#  include <sys/time.h>
#  include <floatingpoint.h>
#else
#  include <time.h>
#endif

# include "bslio.h"
# include "numio.h"

/*****************************************************************************/
# define MaxBslFiles           20                /* maximum opened bsl files */
# define BslIndicNumber        10     /* minimum size of the indicator array */
# define BslBufferSize        512                  /* size of the I/O buffer */
# define MaxBslLineLen         80       /* maximum length per line in output */
/*****************************************************************************/

typedef struct Bsl_Frame {
  char        *FrameKey;
  BslDataSpec *DataSpec;
  struct Bsl_Frame  *Previous, *Next;            /* previous and next memory */
  struct Bsl_Memory *Memory;                            /* the owning memory */
} BslFrame;

typedef struct Bsl_Memory {
  char       *MemoryKey;
  char       *FileName;                           /* name of the memory file */
  char       *Indicator;
  BslFrame   *FrameList;
  struct Bsl_Memory *Previous, *Next;            /* previous and next memory */
  struct Bsl_File   *File;                                /* the owning file */
} BslMemory;

typedef struct Bsl_File {
  char       *Name;                                   /* file name with path */
  char       *Path;                                             /* path only */
  FILE       *Channel;                                        /* i/o channel */
  char       *Buffer;                                /* pointer to IO Buffer */
  char       *FirstHeader;
  char       *SecondHeader;
  BslMemory  *MemoryList;               /* the list of memories in this file */
} BslFile;

PRIVATE int        InitBslTable = 0;
PRIVATE BslFile    BslTable[MaxBslFiles];
PRIVATE char       *BslNew  = "new", *BslOld = "old", *BslAny = "any",
                   *BslRead = "read";

PRIVATE char       end_of_line[3] = { '\r', '\n', '\0' };

enum BslOpenMode { NewBslFile, OldBslFile, AnyBslFile };

enum BslSortMode { Bsl_CaseSensitive, Bsl_UpperCase, Bsl_Number };

enum IndicIndex  { index_dim_1=1, index_dim_2, index_dim_3, index4, index5, 
                   index6, index7, index8, index9, lastindex, IndicEnd };


/***************************************************************************
* Defines                                                                  *
***************************************************************************/

#ifdef sun
# include <sys/param.h>
# define GETCWD(x,y) getwd (x)
#else
# ifdef WIN32
#  include <direct.h>
#  define GETCWD(x,y) _getcwd(x, y)
# else
#  define GETCWD(x,y) getcwd (x, y)
# endif
#endif

/*****************************************************************************/

/*---------------------------------------------------------------------------
NAME

   bslio_version --- returns the current version of bslio

SYNOPSIS

   char *bslio_version ( void );

RETURN VALUE

char * version string
---------------------------------------------------------------------------*/
char *bslio_version ( void )
{ return ( BSLIO_VERSION );
} /* bslio_version */

/*---------------------------------------------------------------------------
NAME

   newstr_bsl --- allocate memory and copy a character string into it

SYNOPSIS

   char * newstr_bsl( const char * string );

DESCRIPTION
  Allocates strlen('string')+1 bytes of memory and copies 'string' into it.
  In case of success the pointer to the allocated memory is returned. The
  null pointer is returned in case of an error.

RETURN VALUE
  Returns the pointer to the allocated string or (char *) NULL in case
  of an error.
---------------------------------------------------------------------------*/
char * newstr_bsl( const char * string )
{ char * newstr_bsling;

  if (!(newstr_bsling = (char *) malloc(strlen(string)+1))) return((char *) NULL);
  (void) strcpy(newstr_bsling,string);

  return( newstr_bsling );

} /* newstr_bsl */

/*---------------------------------------------------------------------------
NAME

       trim_bsl --- remove leading and trailing white spaces

SYNOPSIS

       char * trim_bsl ( char * str );

DESCRIPTION
       Leading and trailing characters in 'str' that return true when checked
       with isspace are removed. The string 'str' is actually modified.
       The first non-isspace character is copied to the start of 'str' and 
       the character after the last non-isspace character is set to '\0'.
       removed. If 'str' is the NULL pointer the NULL pointer is returned.

HISTORY
       23-Mar-1998 Peter Boesecke
---------------------------------------------------------------------------*/
char * trim_bsl ( char * str )
{ char *ps1, *ps2;

  if ( !str ) return ( str );

  ps1 = ps2 = str;
  while ( (*ps1) && (isspace(*ps1)) ) ps1++;
  while (*ps1) *ps2++ = *ps1++;
  *ps2--='\0';
  while ( (ps2>=str) && (isspace(*ps2)) ) *ps2--='\0';

  return ( str );
 
} /* trim_bsl */

/*---------------------------------------------------------------------------
NAME

       getpath_bsl --- extract path from filename 

SYNOPSIS

       char *getpath_bsl ( char *buffer, size_t buflen, const char * filename );

DESCRIPTION
       Extract path from filename and copies it into buffer. If the buffer is
       too short or another error has occurred NULL is returned. If filename 
       has no path the current working directory is determined with 
       getcwd and returned.

HISTORY
       23-Mar-1998 Peter Boesecke
---------------------------------------------------------------------------*/
char * getpath_bsl ( char * buffer, size_t buflen, const char * filename )
{ size_t str_len;  
  char *ps;

  str_len = strlen(filename);
  if (str_len>=buflen) return( (char *) NULL );


  if ( (ps = strrchr( filename, (int) '/' )) ) {
    memcpy( buffer, filename, (ps-filename) );
    buffer[(ps-filename)]='\0'; }
  else GETCWD( buffer, buflen );

  str_len = strlen(buffer);
  if (str_len<buflen) { buffer[str_len]='/';buffer[str_len+1]='\0'; }
    else buffer[0] = '\0';

  return ( buffer );

} /* getpath_bsl */

/*---------------------------------------------------------------------------
NAME

   s2long_bsl --- Conversion of a string to long int

SYNOPSIS

   long int s2long_bsl( const char * string );

DESCRIPTION
   Conversion of a string to long int. 
   In case of an error the output value is 0.

RETURN VALUE
   long int   Converted string
---------------------------------------------------------------------------*/
long int s2long_bsl( const char * string )
{ long int value;

  if ( sscanf(string,"%ld", &value  ) < 1 ) return( 0l );

  return(value);

} /* s2long_bsl */

/*---------------------------------------------------------------------------
NAME

   compare_bsl_keys --- compare two strings 

SYNOPSIS

  enum BslSortMode { Bsl_CaseSensitive, Bsl_UpperCase, Bsl_Number };
  int compare_bsl_keys ( const char * s1, const char * s2, SortMode smode );


DESCRIPTION
  Compares the two strings s1 and s2.
  If s1 < s2 the return value is -1
  if s1 == s2 the return value is 0
  if s1 > s2 the return value is 1
  In SortMode SM_UpperCase the sorting is not case sensitive
  In SortMode SM_Number the strings are converted to numbers and the numbers
  are compared. If the conversion fails the comparison is case insensitive.

RETURN VALUE
  int -1, 0, +1 depending on the comparison
---------------------------------------------------------------------------*/
int compare_bsl_keys ( const char * s1, const char * s2, int smode )
{ int comp_result;
  long l1, l2;
  char * err1, *err2;

  switch ( smode ) {
  case      Bsl_CaseSensitive: 
               comp_result = strncmp ( s1, s2, MaxBslLineLen ); 
               break;
  case      Bsl_UpperCase: 
               comp_result = num_strncasecmp ( s1, s2, MaxBslLineLen );
               break;
  case      Bsl_Number:
               l1 = strtol(s1, &err1 ,10);
               l2 = strtol(s2, &err2 ,10);
               if (err1) if (strlen(err1)==0) err1 = (char *) NULL;
               if (err2) if (strlen(err2)==0) err2 = (char *) NULL;
               if (!((err1)||(err2))) {
                 if (l1<l2) comp_result = -1;
                 else if (l1==l2) comp_result = 0;
                 else comp_result = 1;
               } else comp_result = num_strncasecmp ( s1, s2, MaxBslLineLen );
               break;
  default:  printf("compare: Invalid smode\n"); exit(-1);
            break;
  } /* switch */

  return ( comp_result );

} /* compare_bsl_keys */

/*---------------------------------------------------------------------------
NAME

   rmeoln --- removes end of line characters from the end of the line 

SYNOPSIS

   char * rmeoln ( char * line );

DESCRIPTION
   Removes end of line characters from 'line' and replaces them with '\0'.
   Nothing is done when 'line' is the NULL pointer.

ARGUMENTS
   char * line (input and output) 

RETURN VALUE
   char * line (pointer to output)
---------------------------------------------------------------------------*/
char * rmeoln ( char * line )
{ char * pc = line;

  
  if (pc)
    while (*pc) {
      if (strchr( end_of_line, (int) *pc )) *pc='\0';
      pc++;
      }
  return( line );

} /* rmeoln */

/*---------------------------------------------------------------------------
NAME

  init_bsl_file --- initialization of the bsl file structure

---------------------------------------------------------------------------*/
void init_bsl_file ( BslFile * file )
{ file->Name          = (char *) NULL;
  file->Path          = (char *) NULL;
  file->Channel       = (FILE *) NULL;
  file->Buffer        = (char *) NULL;
  file->FirstHeader   = (char *) NULL;
  file->SecondHeader  = (char *) NULL;
  file->MemoryList    = (BslMemory *) NULL;
} /* init_bsl_file */

/*---------------------------------------------------------------------------
NAME

  init_bsl_file_table --- initialization of the bsl file table cells

---------------------------------------------------------------------------*/
void init_bsl_file_table ( BslFile ftb[] )
{ register int i;
  for (i=0;i<MaxBslFiles;i++) init_bsl_file ( &ftb[i] );
  InitBslTable = 1;
} /* init_bsl_file_table */

/*---------------------------------------------------------------------------
NAME

  search_free_bsl_stream --- search an unused file table cell 

DESCRIPTION
Returns the index of the first unused file table cell

RETURN VALUE
success: int stream >= 0 
error:              <0
---------------------------------------------------------------------------*/
int search_free_bsl_stream ( BslFile ftb[] )
{ register int i;
  for (i=0;i<MaxBslFiles;i++) {
    if (ftb[i].Channel==(FILE *) NULL) return(i);
    }
  return(-1);
} /* search_free_bsl_stream */

/*---------------------------------------------------------------------------
NAME

  init_bsl_data --- initialization of the bsl data structure

---------------------------------------------------------------------------*/
void init_bsl_data_spec ( BslDataSpec * data_spec )
{ 
  data_spec->BinaryFileName      = (char *) NULL;
  data_spec->BinaryFilePos       = 0U;
  data_spec->BinaryFileLen       = 0U;
  data_spec->Data                = (void *) NULL;
  data_spec->Dim                 = (long *) NULL;
  data_spec->DataType            = FloatIEEE32;
  data_spec->ByteOrder           = edf_byteorder ();
  data_spec->RasterConfiguration = 1;
  data_spec->Compression         = UnCompressed;

} /* init_bsl_data_spec */

/*---------------------------------------------------------------------------
NAME

  new_bsl_data --- creates a new bsl data structure and initializes it

RETURN VALUE
success:  BslDataSpec * pointer to new bsl data specification
error:    NULL pointer
---------------------------------------------------------------------------*/
BslDataSpec * new_bsl_data_spec ( void )
{ BslDataSpec * newdata_spec;

  if ( (newdata_spec = (BslDataSpec *) malloc( sizeof(BslDataSpec) )) )
     init_bsl_data_spec ( newdata_spec );
  return(newdata_spec);

} /* new_bsl_data_spec */

/*---------------------------------------------------------------------------
NAME

   free_bsl_data_spec --- deallocate 'data_spec' and all its contents

SYNOPSIS

   BslDataSpec * free_bsl_data ( BslDataSpec * data_spec )

DESCRIPTION
The allocated memory of the contents of 'data_spec' is removed and
(BslDataSpec *) NULL is returned. If 'data_spec' is NULL, nothing is done.

RETURN VALUE
(BslDataSpec *) NULL
---------------------------------------------------------------------------*/
BslDataSpec * free_bsl_data_spec ( BslDataSpec * data_spec )
{ 

  if (!data_spec) return ( data_spec );

  if (data_spec->BinaryFileName) free ( data_spec->BinaryFileName );
  if (data_spec->Data)           free ( data_spec->Data );
  if (data_spec->Dim)            free ( data_spec->Dim );
 
  free(data_spec);
 
  return( (BslDataSpec *) NULL );
 
} /* free_bsl_data_spec */

/*---------------------------------------------------------------------------
NAME

  init_bsl_frame --- initialization of the bsl frame structure

---------------------------------------------------------------------------*/
void init_bsl_frame ( BslFrame * frame )
{ frame->FrameKey       = (char *) NULL;
  frame->DataSpec       = (BslDataSpec *) NULL;
  frame->Previous       = (BslFrame *) NULL;
  frame->Next           = (BslFrame *) NULL;
  frame->Memory         = (BslMemory *) NULL;
} /* init_bsl_frame */

/*---------------------------------------------------------------------------
NAME

  new_bsl_frame --- creates a new bsl frame and initializes it 

RETURN VALUE
success:  BslFrame * pointer to new bsl frame
error:    NULL pointer
---------------------------------------------------------------------------*/
BslFrame * new_bsl_frame ( const char * FrameKey )
{ BslFrame * newframe;
 
  if ( (newframe = (BslFrame *) malloc( sizeof(BslFrame) )) ) 
     init_bsl_frame ( newframe );
  newframe->FrameKey = newstr_bsl(FrameKey);
  newframe->DataSpec = new_bsl_data_spec();
  return(newframe);

} /* new_bsl_frame */


/*---------------------------------------------------------------------------
NAME

  search_bsl_frame --- searches the bsl 'frame' in the frame list of 'memory'

SYNOPSIS

  BslFrame * search_bsl_frame ( BslMemory * memory, char * FrameKey )

DESCRIPTION

RETURN VALUE
success BslFrame * frame
error   NULL
---------------------------------------------------------------------------*/
BslFrame * search_bsl_frame ( BslMemory * memory, const char * FrameKey )
{ BslFrame * current = (BslFrame *) NULL;

  if (memory == (BslMemory *) NULL) return( current );

  /* search frame */
  current = memory->FrameList;
  if (current!=(BslFrame *) NULL )
  while( ( current!=(BslFrame *) NULL ) &&
         ( compare_bsl_keys(current->FrameKey,FrameKey,Bsl_Number)!=0 ) ) {
    current = current->Next;
    }
  return( current );

} /* search_bsl_frame */
 
/*---------------------------------------------------------------------------
NAME

  insert_bsl_frame --- insert a bsl frame into the frame list of a bsl memory

SYNOPSIS

  BslFrame  * insert_bsl_frame ( BslMemory * memory, const char * FrameKey )

DESCRIPTION

RETURN VALUE
success:            pointer to inserted frame 
error:              NULL pointer 
---------------------------------------------------------------------------*/
BslFrame  * insert_bsl_frame ( BslMemory * memory, const char * FrameKey )
{ BslFrame *frame, *previous, *next, *newframe;
  int notfound = -1;

  frame = (BslFrame *) NULL;
  previous = (BslFrame *) NULL;

  if ( memory == (BslMemory *) NULL ) return( frame );

  /* search insertion point (insertion before *pnext) */
  next = memory->FrameList;
  while( ( next!=(BslFrame *) NULL ) && (notfound<0) ) {
    notfound = compare_bsl_keys(next->FrameKey,FrameKey,Bsl_Number);
    if (notfound<0) { previous = next; next = next->Next; }
    }

  /* create new frame, if no frame found */
  if ( next==(BslFrame *) NULL  ) {
    /* create new frame */
    newframe = new_bsl_frame ( FrameKey );
    if ( newframe == (BslFrame *) NULL ) return( frame );

    /* insert new frame before *pnext */
    if (next) next->Previous = newframe;
    newframe->Next=next;
    newframe->Previous=previous;
    if (previous) previous->Next=newframe;
      else memory->FrameList=newframe;

    /* link to owning memory */
    newframe->Memory      = memory;

    next = newframe;
    }

  frame = next;

  return( frame );

} /* insert_bsl_frame */

/*---------------------------------------------------------------------------
NAME

   remove_bsl_frame --- removes 'frame' from the frame list

SYNOPSIS

   void remove_bsl_frame ( BslFrame * frame )


DESCRIPTION
The allocated memory of the contents of 'frame' is removed. The frame is 
removed from the frame list of the owning memory. If 'frame' was the only 
frame in 'frame->Memory->FrameList' 'frame->Memory->FrameList' is set to 
(BslFrame *) NULL. If 'frame' is NULL, nothing is done.
---------------------------------------------------------------------------*/
void remove_bsl_frame ( BslFrame * frame )
{
  BslFrame **proot, *previous, *next;

  if (frame==(BslFrame*) NULL) return;
 
  proot    = &(frame->Memory->FrameList);
  previous = frame->Previous;
  next     = frame->Next; 

  if (next!=(BslFrame*) NULL)     next->Previous = previous;
  if (previous!=(BslFrame*) NULL) previous->Next = next;
  if (*proot==frame) { *proot = ((BslFrame*) NULL); }


  if (frame->FrameKey)       free(frame->FrameKey); 
  free_bsl_data_spec( frame->DataSpec );

  free(frame); 

  return;

} /* remove_bsl_frame */

/*---------------------------------------------------------------------------
NAME

   remove_bsl_frame_list --- empty the frame list of 'memory' 

SYNOPSIS

   void remove_bsl_frame_list ( BslMemory * memory );

---------------------------------------------------------------------------*/
void remove_bsl_frame_list ( BslMemory * memory )
{
  BslFrame *frame, *next;

  if (memory==(BslMemory*) NULL) return;

  next = memory->FrameList;
  while ( next!=(BslFrame*) NULL ) {
    frame = next;
    next  = next->Next;
    remove_bsl_frame ( frame );
    }
  return;

} /* free_bsl_frame_list */

/*---------------------------------------------------------------------------
NAME

   print_bsl_data_spec --- prints 'data_spec'

SYNOPSIS

   void print_bsl_data_spec ( FILE * out, const BslDataSpec * data_spec );

---------------------------------------------------------------------------*/
void print_bsl_data_spec ( FILE * out, const BslDataSpec * data_spec )
{ const char * SeparationLine =
    "-       -       -       -       -       -       -       -";
      long i_dim;

      if (!data_spec) return;

      fprintf(out,"  %s\n",SeparationLine);

      fprintf(out,"    BinaryFileName      = ");
      if ( data_spec->BinaryFileName )
        fprintf(out,"\"%s\"\n",data_spec->BinaryFileName);
        else fprintf(out,"(no binary file name)\n");
      fprintf(out,"    BinaryFilePos       = %lu\n",data_spec->BinaryFilePos);
      fprintf(out,"    BinaryFileLen       = %lu\n",data_spec->BinaryFileLen);
      fprintf(out,"    Data                = %p\n",data_spec->Data);
      fprintf(out,"    Dim                 = %p\n",data_spec->Dim);
      if (data_spec->Dim) for (i_dim=0;i_dim<=data_spec->Dim[0];i_dim++)
        fprintf(out,"          Dim[%1lu]        = %ld\n", 
                    i_dim,(data_spec->Dim)[i_dim]);
      fprintf(out,"    DataType            = %u\n",data_spec->DataType);
      fprintf(out,"    ByteOrder           = %u\n",data_spec->ByteOrder);
      fprintf(out,"    RasterConfiguration = %lu\n",
                    data_spec->RasterConfiguration);
      fprintf(out,"    Compression         = %u\n",data_spec->Compression);

      fprintf(out,"  %s\n",SeparationLine);

} /* print_bsl_data_spec */

/*---------------------------------------------------------------------------
NAME

   print_bsl_frame_list --- prints frame list contents of 'memory'

SYNOPSIS

   void print_bsl_frame_list ( FILE * out, const BslMemory * memory,
                               int level, int verbose )

---------------------------------------------------------------------------*/
void print_bsl_frame_list ( FILE * out, const BslMemory * memory,
                            int level, int verbose )

{ const char * SeparationLine =
    "-   -   -   -   -   -   -   -   -   -   -   -   -   -   -";

  const BslFrame * frame;

  if (level<1) return;

  frame = memory->FrameList;

  while(frame!=(BslFrame*) NULL) {
    if (verbose) {
      fprintf(out,"  %s\n",SeparationLine);
      fprintf(out,"  FrameKey           = %s\n",frame->FrameKey);
      fprintf(out,"  DataSpec           = %p\n",frame->DataSpec);
      print_bsl_data_spec ( out, frame->DataSpec );

      fprintf(out,"  Previous FrameKey  = ");
      if ((frame->Previous)!=(BslFrame*) NULL)
        fprintf(out,"%s\n", frame->Previous->FrameKey);
        else fprintf(out,"(no previous frame)\n");
      fprintf(out,"  Next FrameKey      = ");
      if ((frame->Next)!=(BslFrame*) NULL)
        fprintf(out,"%s\n", frame->Next->FrameKey);
        else fprintf(out,"(no next frame)\n");
      fprintf(out,"  Owner Memory       = %s\n",frame->Memory->MemoryKey);
      fprintf(out,"  %s\n",SeparationLine); 
      } else {
      fprintf(out,"  FrameKey           = '%s'\n",frame->FrameKey);
      }
    frame=frame->Next;
    }

} /* print_bsl_frame_list */

/*---------------------------------------------------------------------------
NAME

  init_bsl_memory --- initialization of the bsl memory structure

---------------------------------------------------------------------------*/
void init_bsl_memory ( BslMemory * memory )
{ memory->MemoryKey  = (char *) NULL;
  memory->FileName   = (char *) NULL;
  memory->Indicator  = (char *) NULL;
  memory->FrameList  = (BslFrame *) NULL;
  memory->Previous   = (BslMemory *) NULL;
  memory->Next       = (BslMemory *) NULL;
  memory->File       = (BslFile *) NULL;

} /* init_bsl_memory */

/*---------------------------------------------------------------------------
NAME

  new_bsl_memory --- creates a new bsl memory with name MemoryKey 

RETURN VALUE
success:  BslMemory * pointer to new bsl memory 
error:    NULL pointer
---------------------------------------------------------------------------*/
BslMemory * new_bsl_memory ( const char * MemoryKey )
{ BslMemory * newmemory;

  if ( (newmemory = (BslMemory*) malloc( sizeof(BslMemory) )) )
     init_bsl_memory ( newmemory );
  newmemory->MemoryKey = newstr_bsl(MemoryKey);
  return(newmemory);

} /* new_bsl_memory */

/*---------------------------------------------------------------------------
NAME

  search_bsl_memory --- searches the bsl 'memory' in the memory list of 'file'

SYNOPSIS

  BslMemory * search_bsl_memory ( BslFile * file, char * MemoryKey )

DESCRIPTION

RETURN VALUE
success BslMemory * found memory 
error   NULL
---------------------------------------------------------------------------*/
BslMemory * search_bsl_memory ( BslFile * file, const char * MemoryKey )
{ BslMemory * current = (BslMemory *) NULL;

  if (file == (BslFile *) NULL) return( current );

  /* search memory */
  current = file->MemoryList;
  if (current!=(BslMemory *) NULL )
  while( ( current!=(BslMemory *) NULL ) &&
         ( compare_bsl_keys(current->MemoryKey,MemoryKey,Bsl_UpperCase)!=0 ) ) {
    current = current->Next;
    }
  return( current );

} /* search_bsl_memory */

/*---------------------------------------------------------------------------
NAME

  insert_bsl_memory --- insert a bsl memory into the memory list of a bsl file 

SYNOPSIS

  BslMemory  * insert_bsl_memory ( BslFile * file, const char * MemoryKey )

DESCRIPTION

RETURN VALUE
success:            pointer to inserted memory 
error:              NULL pointer
---------------------------------------------------------------------------*/
BslMemory  * insert_bsl_memory ( BslFile * file, const char * MemoryKey )
{ BslMemory *memory, *previous, *next, *newmemory;
  int notfound = -1;

  memory   = (BslMemory *) NULL;
  previous = (BslMemory *) NULL;

  if ( file == (BslFile *) NULL ) return( memory );

  /* search insertion point (insertion before *pnext) */
  next = file->MemoryList;
  while( ( next!=(BslMemory *) NULL ) && (notfound<0) ) {
    notfound = compare_bsl_keys(next->MemoryKey,MemoryKey,Bsl_UpperCase);
    if (notfound<0) { previous = next; next = next->Next; }
    }

  /* create new memory, if no memory found */
  if ( next==(BslMemory *) NULL  ) {
    /* create new memory */
    newmemory = new_bsl_memory ( MemoryKey );
    if ( newmemory == (BslMemory *) NULL ) return( memory );

    /* insert new memory before *pnext */
    if (next) next->Previous = newmemory;
    newmemory->Next=next;
    newmemory->Previous=previous; 
    if (previous) previous->Next=newmemory;
      else file->MemoryList=newmemory;

    /* link to owning file */
    newmemory->File      = file;

    next = newmemory;
    }

  memory = next;

  return( memory );

} /* insert_bsl_memory */

/*---------------------------------------------------------------------------
NAME

   remove_bsl_memory --- removes 'memory' from the memory list

SYNOPSIS

   void remove_bsl_memory ( BslMemory * memory );

DESCRIPTION
The allocated memory of the contents of 'memory' is removed. 'memory'
is removed from the memory list of the owning file. If 'memory' was the 
only memory in 'memory->File->MemoryList' 'memory->File->MemoryList' is 
set to (BslMemory *) NULL. If 'memory' is NULL, nothing is done. 
---------------------------------------------------------------------------*/
void remove_bsl_memory ( BslMemory * memory )
{
  BslMemory **proot, *previous, *next;

  if (memory==(BslMemory*) NULL) return; 

  proot      = &(memory->File->MemoryList);
  previous   = memory->Previous;
  next       = memory->Next; 

  remove_bsl_frame_list ( memory );

  if (next!=(BslMemory*) NULL)       next->Previous = previous;
  if (previous!=(BslMemory*) NULL)   previous->Next = next;
  if (*proot==memory) { *proot = ((BslMemory*) NULL); }

  if (memory->MemoryKey)  free(memory->MemoryKey);
  if (memory->FileName)   free(memory->FileName);
  if (memory->Indicator)  free(memory->Indicator);
  free(memory);

  return;

} /* remove_bsl_memory */

/*---------------------------------------------------------------------------
NAME

   remove_bsl_memory_list --- empty the memory list of 'file'

SYNOPSIS

   void remove_bsl_memory_list ( BslFile * file )

---------------------------------------------------------------------------*/
void remove_bsl_memory_list ( BslFile * file )
{
  BslMemory *memory, *next;

  if (file==(BslFile*) NULL) return;

  next = file->MemoryList;
  while ( next!=(BslMemory*) NULL ) {
    memory = next;
    next=next->Next;
    remove_bsl_memory ( memory );
    }
  return;

} /* remove_bsl_memory_list */

/*---------------------------------------------------------------------------
NAME

   print_bsl_memory_list --- prints bsl memory contents

SYNOPSIS

   void print_bsl_memory_list ( FILE * out, const BslFile * file,
                               int level, int verbose );

---------------------------------------------------------------------------*/
void print_bsl_memory_list ( FILE * out, const BslFile * file, 
                             int level, int verbose )
{ const char * SeparationLine =
    "- - - - - - - - - - - - - - - - - - - - - - - - - - - - -";

  BslMemory * memory;

  if (level<1) return;

  memory = file->MemoryList;

  while(memory!=(BslMemory*) NULL) {
    if (verbose) {
      fprintf(out," %s\n",SeparationLine);
      fprintf(out," MemoryKey           = %s\n",memory->MemoryKey);
      fprintf(out," FileName            = \"%s\"\n",memory->FileName);

      fprintf(out," Indicator string    = \n\"%s\"\n",memory->Indicator); 

      fprintf(out," Previous MemoryKey   = ");
      if ((memory->Previous)!=(BslMemory*) NULL)
        fprintf(out,"%s\n", memory->Previous->MemoryKey);
        else fprintf(out,"(no previous memory)\n");
      fprintf(out," Next MemoryKey       = ");
      if ((memory->Next)!=(BslMemory*) NULL)
        fprintf(out,"%s\n", memory->Next->MemoryKey);
        else fprintf(out,"(no next memory)\n");
//      fprintf(out," Owner file          = %#x\n",memory->File->Channel);
      fprintf(out," Owner file          = %p\n",memory->File->Channel);
      fprintf(out," FrameList           = %p\n",memory->FrameList);
      print_bsl_frame_list( out, memory, level-1, verbose );
      fprintf(out," %s\n",SeparationLine); 
      } else { /* short */
      fprintf(out," MemoryKey           = '%s'\n",memory->MemoryKey);
      print_bsl_frame_list( out, memory, level-1, verbose );

      }
    memory=memory->Next;
    }

} /* print_bsl_memory_list */

/*---------------------------------------------------------------------------
NAME

   print_bsl_filetable --- prints bsl file contents

SYNOPSIS

   void print_bsl_filetable ( FILE * out, int level, int verbose )

---------------------------------------------------------------------------*/
void print_bsl_filetable ( FILE * out, int level, int verbose )
{ 
  const char * SeparationLine =
    "=========================================================";

  int stream;

  if (level<1) return;
  if (!InitBslTable) { printf("Bsl file table is not initialized\n");return;}

  for (stream=0;stream<MaxBslFiles;stream++)
    if (BslTable[stream].Channel) {
      if (verbose) {
        fprintf(out,"%s\n",SeparationLine);
        fprintf(out,"Stream               = %d\n",stream);
        fprintf(out,"Name                 = \"%s\"\n",BslTable[stream].Name);
	fprintf(out,"Path                 = \"%s\"\n",BslTable[stream].Path);
//        fprintf(out,"Channel              = %#x\n",BslTable[stream].Channel);
        fprintf(out,"Channel              = %p\n",BslTable[stream].Channel);
        fprintf(out,"Buffer               = %p\n",BslTable[stream].Buffer);
        fprintf(out,"FirstHeader          = \n\"%s\"\n",
                                             BslTable[stream].FirstHeader);
        fprintf(out,"SecondHeader         = \n\"%s\"\n",
                                             BslTable[stream].SecondHeader);
        fprintf(out,"MemoryList           = %p\n",BslTable[stream].MemoryList);
        print_bsl_memory_list(out,&BslTable[stream],level-1,verbose);
        fprintf(out,"%s\n",SeparationLine); 
        } else {
        fprintf(out,"Stream               = '%d'\n",stream);
        print_bsl_memory_list(out,&BslTable[stream],level-1,verbose);
        }
      }

} /* print_bsl_filetable */

/*===file_io===============================================================*/

/*---------------------------------------------------------------------------
NAME

   strisspace --- checks, whether the string contains only white spaces. 

SYNOPSIS

   int strisspace ( const char * str );

DESCRIPTION
   Scans 'str' with isspace and returns 1 if all characters are white spaces.

ARGUMENTS
   const char * str     string to be scanned

RETURN VALUE
   int  1  'str' contains only white spaces (isspace) 
   int  0  'str' contains also other characters
---------------------------------------------------------------------------*/
int strisspace ( const char * str ) 
{ 
  while (*str) if (!isspace((int) *str++)) return(0);

  return(1);
  
} /* strisspace */

/*---------------------------------------------------------------------------
NAME

   indicator2table --- create indicator table from indicator string

SYNOPSIS

   long * indicator2table ( const char * indicator );

DESCRIPTION
   Creates a table of values from indicator string. The length of the 
   table is written in element 0. 

ARGUMENTS
 const char * indicator    string with indicator numbers
 
RETURN VALUE
 Success:         long * indic_table  pointer to the indicator table 
 error:           long * NULL pointer
---------------------------------------------------------------------------*/
long * indicator2table ( const char * indicator )
{ char *rest, *previous;
  long int index, *indic_table = (long int *) NULL;
  int i, table_len;

  /* get length of table */ 
  table_len = 0;
  rest = (char *) indicator; /*sorry!*/
  do { table_len++;
       previous = rest;
       index = strtol(previous, &rest, 10);
   } while (( *rest ) && ( previous<rest ));

  if (strisspace(previous)) table_len--;  

  /* allocate memory for indic_table */
  indic_table = (long int *) malloc ( sizeof( long int) * (table_len+1) );
  if (!indic_table) return(indic_table); 
  indic_table[0] = table_len;

  /* convert */
  rest = (char *) indicator;
  for (i=1;i<=table_len;i++) indic_table[i] = strtol(rest, &rest, 10);
     
  return( indic_table );

} /* indicator2table */

/*---------------------------------------------------------------------------
NAME

   get_bsl_dim --- extract dimension from indicators

SYNOPSIS

   long * get_bsl_dim ( const BslMemory * memory );

DESCRIPTION
   Reads the dimensions from the indicator string 'memory->Indicator'.

ARGUMENTS
 const BslMemory * memory  input memory

RETURN VALUE
  long *
  error   : (long *) NULL
  success : Dim
              Dim[0]  : 2 = number of dimensions
              Dim[1]  : number of elements in first dimension
              Dim[2]  : number of elements in nn-th dimension
---------------------------------------------------------------------------*/
long * get_bsl_dim ( const BslMemory * memory )
{ long * dim = (long *) NULL;
  long * indic;

  if (!memory) return( dim );
  indic = indicator2table ( memory->Indicator );
  if (!indic) return( dim );
  if (indic[0]<BslIndicNumber) { free(indic); return( dim ); }

  dim = (long *) malloc ( sizeof (long int) * 4 );
  if (!dim) { free(indic); return ( dim ); }

  dim[0] = 2;
  dim[1] = indic[1];
  dim[2] = indic[2];

  free ( indic );

  return ( dim );

} /* get_bsl_dim */

/*---------------------------------------------------------------------------
NAME

   get_bsl_number --- extract number of frames from indicators

SYNOPSIS

   long get_bsl_number ( const BslMemory * memory );

DESCRIPTION
   Reads the number of frames from the indicator string 'memory->Indicator'. 

ARGUMENTS
 const BslMemory * memory  input memory

RETURN VALUE
  error   : long int   <0 
  success : long int   number of frames
---------------------------------------------------------------------------*/
long get_bsl_number ( const BslMemory * memory )
{ 
  long * indic, number = -1l;

  if (!memory) return( number );
  indic = indicator2table ( memory->Indicator );
  if (!indic) return( number );
  if (indic[0]<BslIndicNumber) { free(indic); return( number ); }

  number = indic[3];
  free ( indic );

  return ( number ); 

} /* get_bsl_number*/

/*---------------------------------------------------------------------------
NAME

   indicator2frame_list --- create a frame list from the indicator string

SYNOPSIS

   int indicator2frame_list ( BslMemory * memory );

DESCRIPTION
 Creates to the indicator string of 'memory' a frame list. Eventually 
 previously allocated memory for 'memory->FrameList' is released.

ARGUMENTS
 BslMemory * memory  memory 

RETURN VALUE
 Success:         int 0  
 error:           int <0
---------------------------------------------------------------------------*/
int indicator2frame_list ( BslMemory * memory )
{ long        number;  /* number of frames */
  BslFrame    *frame;  /* inserted frame */
  char        frame_key[MaxBslLineLen+1]; 
  BslDataSpec *data_spec;   /* data specification */
  char        fullname[BslBufferSize];

  BslFile     *file;
  
  int        i;

  if (!memory) return(-1);
 
  file = memory->File;
 
  number = get_bsl_number ( memory );
  if (number<0) return (-1);

  for (i=1;i<=number;i++) {
    sprintf(frame_key,"%d",i);    
    frame = insert_bsl_frame ( memory, frame_key ); if (!frame) return(-1);
    data_spec = frame->DataSpec;
    data_spec->Dim = get_bsl_dim ( memory ); if (!data_spec->Dim) return(-1);
    sprintf( fullname, "%s%s", file->Path, memory->FileName );
      data_spec->BinaryFileName = newstr_bsl ( fullname );
      if (!data_spec->BinaryFileName) return(-1);
    data_spec->BinaryFileLen  = 
      (unsigned long) (sizeof(float)*data_spec->Dim[1]*data_spec->Dim[2]);
    data_spec->BinaryFilePos  = 
      (unsigned long) (sizeof(float)*data_spec->Dim[1]*data_spec->Dim[2]*(i-1));
    }

  return(0);

} /* indicator2frame_list */

/*---------------------------------------------------------------------------
NAME

   read_bsl_line --- read a single line of an bsl header

SYNOPSIS

   char * read_bsl_line ( BslFile * file, int * io_error );

DESCRIPTION
  Reads a single line from 'file', allocates memory for it and returns it.

ARGUMENTS
 BslFile * file   opened input file
 int * io_error   returned error value from ferror

RETURN VALUE
 Success: char *  allocated string
 error:           NULL pointer
---------------------------------------------------------------------------*/
char * read_bsl_line ( BslFile * file, int * io_error )
{ const int buflen = BslBufferSize;
  static char buffer[BslBufferSize];
  char * str = (char *) NULL;

  FILE * channel = file->Channel;

  fgets (buffer, buflen, channel);

  if (!feof( channel ))
    if (!(*io_error=ferror( channel ))) str = newstr_bsl(buffer);

  return( rmeoln(str) );

} /* read_bsl_line */

/*---------------------------------------------------------------------------
NAME

   read_bsl_header --- read a bsl header file

SYNOPSIS

   int read_bsl_header ( BslFile * file );

DESCRIPTION
  Reads the bsl header from 'file'. The bsl header has the following
  structure:

  *       line 1. Header with up to 80 alphanumeric characters.
  *       line 2. Header with up to 80 alphanumeric characters.
  *       line 3. Integer indicators for the first binary file

  indicator(1)    = number of pixels        (dim_1)
  indicator(2)    = number of rasters       (dim_2)
  indicator(3)    = number of time frames   (dim_3)
  indicator(4)    =
  indicator(5)    =
  indicator(6)    =
  indicator(7)    =
  indicator(8)    =
  indicator(9)    =
  indicator(10)   = 1, except for the last binary file,  when  it
                    is zero

  *       line 4. File name of the binary file

The lines 3 and 4 are repeated for subsequent binary files.
The panel below shows an example of a header file with two binary
files.


RETURN VALUE
 Success: int     0
 error:   int    <0
---------------------------------------------------------------------------*/
int read_bsl_header ( BslFile * file )
{ BslMemory * memory;
 
  char *indicator, *file_name; 
  char memory_key[MaxBslLineLen+1];
  long int memory_number=1;
  int io_error;

  if (!(file->FirstHeader =trim_bsl(read_bsl_line(file,&io_error)))) return(-1);
  if (!(file->SecondHeader=trim_bsl(read_bsl_line(file,&io_error)))) return(-1);

  do {
    if ( (indicator  = read_bsl_line( file, &io_error )) )
      if ( (file_name = read_bsl_line( file, &io_error )) ) {
        sprintf(memory_key,"%ld",memory_number++);
        memory = insert_bsl_memory ( file, memory_key );
        if (!(memory)) return(-1);
        memory->FileName  = trim_bsl(file_name);
        memory->Indicator = trim_bsl(indicator);

        if (indicator2frame_list ( memory )) return(-1);
      }
   } while (( memory_key ) && ( indicator ));
  if (io_error) return(-1);

  return(0);

} /* read_bsl_header */

/*---------------------------------------------------------------------------
NAME

   free_bsl_file --- deallocates and reinitializes a bsl file.

SYNOPSIS

   int free_bsl_file ( BslFile * file );

DESCRIPTION
Deallocates all memory of a bsl file. If it is already done nothing happens.

RETURN VALUE
success:   int 0
error:     int -1
---------------------------------------------------------------------------*/
int free_bsl_file ( BslFile * file )
{
  /* remove memory list */
  remove_bsl_memory_list( file );

  /* close file */
  if (fclose(file->Channel)) return(-1);
  file->Channel = (FILE *) NULL;

  /* release IO-buffer */
  free(file->Buffer);
  file->Buffer = (char *) NULL;

  /* release file name */
  free(file->Name);
  file->Name = (char *) NULL;

  /* release path */
  free(file->Path);
  file->Path = (char *) NULL;

  /* free first and second header */
  if (file->FirstHeader) free ( file->FirstHeader );
  if (file->SecondHeader) free ( file->SecondHeader );

  init_bsl_file ( file );

  return(0);

} /* free_bsl_file */

/*---------------------------------------------------------------------------
NAME

   open_bsl_file --- opens a bsl file 

SYNOPSIS

   int open_bsl_file ( const char * FileName , const char * mode );

DESCRIPTION
Opens a bsl header file and analyzes the structure. If the contents of 
the file does not contain in line 2+n*2+1 at least 'BslIndicNumber' 
long integer values separated by white space, the reading of the file 
is stopped, all memory is released and a negative value is returned.

ARGUMENTS
const char * FileName        name of the file that should be opened
const char * mode            opening mode: "old", "new", "any"

RETURN VALUE
success:  int      stream >= 0  file stream
error:    int      stream < 0   
---------------------------------------------------------------------------*/
int open_bsl_file ( const char * FileName , const char * mode )
{ int       stream;
  BslFile   *file;
  int       buflen = BslBufferSize; 
  char      buffer[BslBufferSize];

  if (!InitBslTable) init_bsl_file_table ( BslTable );
  stream =  search_free_bsl_stream ( BslTable );
  if (stream<0) return( stream ); 

  file = &BslTable[stream];

  init_bsl_file ( file );

  if (strcmp(mode,BslOld)==0) { /* open old file */
    file->Channel = fopen( FileName,"rb+"); } else
  if (strcmp(mode,BslNew)==0) { /* open new file */
    file->Channel = fopen( FileName,"wb+"); } else
  if (strcmp(mode,BslAny)==0) { /* open old file or create new file */
    if ( (file->Channel = fopen( FileName,"rb+")) ) {
      } else {
      file->Channel = fopen( FileName,"wb+"); } } else
  if (strcmp(mode,BslRead)==0) { /* open old file read-only */
    file->Channel = fopen( FileName,"rb"); } 

  if (file->Channel == (FILE *) NULL) { free_bsl_file( file ); return(-1); }

  /* allocate IO-buffer */
  file->Buffer = (char *) malloc(sizeof(char)*BslBufferSize);
  if ( file->Buffer == (char *) NULL ) { free_bsl_file( file ); return(-1); }
  if ( setvbuf(file->Channel, file->Buffer, _IOFBF, BslBufferSize) ) {
    free_bsl_file( file ); return(-1); }

  /* file structure parameters */
  file->Name = newstr_bsl( FileName );
  file->Path = newstr_bsl( getpath_bsl( buffer, buflen, FileName ));

  /* read header(s) and locate data */
  if (read_bsl_header ( file )) { free_bsl_file( file ); return(-1); } 

  return( stream );

} /* open_bsl_file */

/*---------------------------------------------------------------------------
NAME

   close_bsl_file --- closes a bsl file

SYNOPSIS

   int close_bsl_file ( int stream );

DESCRIPTION
Closes a bsl file. If it is already closed nothing happens.

RETURN VALUE
success:   int 0
error:     int -1
---------------------------------------------------------------------------*/
int close_bsl_file ( int stream )
{
  return(free_bsl_file(&BslTable[stream]));

} /* close_bsl_file */

/*---------------------------------------------------------------------------
NAME

   bsl_memory_range --- returns the number of memories in the bsl file 

SYNOPSIS

   int bsl_memory_range( int stream, long * minmem, long *maxmem );    

DESCRIPTION
Returns the minimum memory number (*minmem) and the maximum memory number
(*maxmem) of the bsl.

RETURN VALUE
success:   int 0
error:     int -1
---------------------------------------------------------------------------*/
int bsl_memory_range( int stream, long * minmem, long *maxmem )
{ BslFile * file;
  BslMemory * memory;
  long memnum;

  file = &BslTable[stream];
  if (!(file->Channel)) return(-1); /* file not opened */

  memory = file->MemoryList;
  if (!memory) return(-1); /* file is empty */

  *minmem = *maxmem = s2long_bsl( memory->MemoryKey );

  while (memory) {
    memnum = s2long_bsl( memory->MemoryKey );
    if (memnum<*minmem) *minmem = memnum;
    else if (memnum>*maxmem) *maxmem = memnum;  
    memory = memory->Next;
   } 

  return(0);

} /* bsl_memory_range */

/*---------------------------------------------------------------------------
NAME

   bsl_frame_range --- returns the number of frames in a bsl memory 

SYNOPSIS

   int bsl_frame_range( int stream, long memnum, long * minfra, long * maxfra );

DESCRIPTION
Returns the minimum frame number (*minfra) and the maximum memory number
(*maxfra) of the bsl memory with number 'memnum'.

RETURN VALUE
success:   int 0
error:     int -1
---------------------------------------------------------------------------*/
int bsl_frame_range( int stream, long memnum, long * minfra, long * maxfra )
{ BslFile * file;
  BslMemory * memory;
  BslFrame * frame;
  char memory_key[MaxBslLineLen+1];
  long franum;

  file = &BslTable[stream];
  if (!(file->Channel)) return(-1); /* file not opened */

  sprintf(memory_key,"%ld",memnum); 
  memory = search_bsl_memory ( file, memory_key ); 
  if (!memory) return(-1); /* file is empty */

  frame = memory->FrameList;
  if (!frame) return(-1); /* memory is empty */

  *minfra = *maxfra = s2long_bsl( frame->FrameKey );

  while (frame) {
    franum = s2long_bsl( frame->FrameKey );
    if (franum<*minfra) *minfra = franum;
    else if (franum>*maxfra) *maxfra = franum;
    frame = frame->Next;
   }

  return(0);

} /* bsl_frame_range */

/*---------------------------------------------------------------------------
NAME

   read_bsl_file_headers --- read the first and second header 

SYNOPSIS

   # include "edfio.h"
   void read_bsl_file_headers ( int stream, char **first_header, 
                                            char **second_header );

DESCRIPTION
Pointer to the first and second file header lines are returned.
These pointers remain valid until the bsl file is closed.
---------------------------------------------------------------------------*/
void read_bsl_file_headers ( int stream, char **first_header,  
                                         char **second_header )
{ BslFile   * file;

  *first_header  = (char *) NULL;
  *second_header = (char *) NULL;

  file = &(BslTable[stream]);
  if (!(file->Channel)) return; /* file not opened */

  *first_header  = file->FirstHeader;
  *second_header = file->SecondHeader;

} /* read_bsl_file_headers */

/*---------------------------------------------------------------------------
NAME

   read_bsl_data_spec --- read data spec of frame 'franum' in memory 'memnum'

SYNOPSIS

   # include "edfio.h"
   BslDataSpec * read_bsl_data_spec ( int stream, long memnum, long franum );

DESCRIPTION

RETURN VALUE
success BslDataSpec * pointer to data spec of frame 'franum' in memory 'memnum' 
error   NULL pointer
---------------------------------------------------------------------------*/
BslDataSpec * read_bsl_data_spec ( int stream, long memnum, long franum )
{ 
  BslFile   * file;
  BslMemory * memory;
  BslFrame  * frame;

  char      memory_key[MaxBslLineLen+1];
  char      frame_key[MaxBslLineLen+1];
  
  file = &(BslTable[stream]);
  if (!(file->Channel)) return((BslDataSpec *) NULL); /* file not opened */

  sprintf(memory_key,"%ld",memnum);
  memory = search_bsl_memory ( file, memory_key );
  if (!memory) return((BslDataSpec *) NULL); /* memory not found */

  sprintf(frame_key,"%ld",franum);
  frame = search_bsl_frame ( memory, frame_key );
  if (!frame) return((BslDataSpec *) NULL); /* frame not found */

  return( frame->DataSpec );

} /* read_bsl_data_spec */

/******************************************************************************/


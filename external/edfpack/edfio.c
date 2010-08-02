/*
 *   Project: The SPD Image correction and azimuthal regrouping
 *			http://forge.epn-campus.eu/projects/show/azimuthal
 *
 *   Copyright (C) 1996-2010 European Synchrotron Radiation Facility
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

# define EDFIO_VERSION      "edfio : V2.25 Peter Boesecke 2008-05-27"
/*+++------------------------------------------------------------------------
NAME
   edfio --- EDF data format specific file access routines

SYNOPSIS

   # include edfio.h

INCLUDE FILES
   edfio.h
   bslio.h
   numio.h

TO LINK WITH
   bslio.c

RESTRICTIONS
   The file size is limited by the long integer format. On machines with
   4 byte long integers additional blocks after 2^31 bytes = 2Gbytes cannot 
   be accessed. Therefore, if a file contains several data blocks the 
   absolute size of a single data file should not exceed 2GBytes.

DESCRIPTION
   EDF data format specific file access routines (read and write routines)
   The file format is described in 'SaxsKeywords.pdf'. The data files can
   contain several data blocks that can be accessed by data numbers and
   chain numbers. Chain numbers are positive integer numbers. A data block 
   consists of a text block ("ASCII header") followed by a binary block 
   ("binary data"). The block type is written at the top of the text block
   after the keyword "EDF_DataBlockID". The value "1.Image.Psd" marks 
   data block number 1 of DataChain 1. Data chain 1 contains always primary 
   (scientific) data, therefore the extension ".Psd". The value 
   "1.Image.2" marks data block number 1 of DataChain 2 with additional
   image data, e.g. error data.

   The following convention should be followed:

   primary image data : chain number 1;
   error image data   : chain number 2;

   Internally, a general chain is defined. It has chain number 0 and 
   data number 0. 

   Data blocks with different chain numbers but the same data numbers
   belong together (e.g. data array (chain number 1) and associated 
   error array (chain number 2)). 

   The input routines can also read bsl files. Bsl frame numbers (franum)
   and memory numbers (memnum) are in the following way internally
   converted to edf data numbers and chain numbers:

   memnum franum    ChainNumber DataNumber  chain key   block key  block id
   1      n         1           n           Image.Psd   n          n.Image.Psd
   m      n         m           n           Image.Psd.m n          n.Image.Psd.m

--------------------------------------------------------------------------*/

/***************************************************************************
* Private part                                                             *
***************************************************************************/

/***************************************************************************
 * Defines                                                                 *
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

/***************************************************************************
* Include files                                                            *
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

# include "edfio.h"
# include "bslio.h"
# include "numio.h"

/***************************************************************************
* Macros                                                                   *
***************************************************************************/

#ifdef sun      /* sun specific stuff */
#  define ATOF(s)   (float) atof (s)   /* Sun/gcc doesn't work with strtod */
#else
#  define ATOF(s)   (float) strtod (s, (char **) NULL)
#endif

# define STRLEN(s) (s == (char *) NULL ? 0 : strlen ( (char *) s))
# define STRCMP(s1,s2) strcmp ( (char *) s1, (char *) s2 )
# define STRCPY(s1,s2) strcpy ( (char *) s1, (char *) s2 )

# define ATOI(s) (int) strtol (s, (char **) NULL, 10)
# define BLANK(c) (c == ' ' || c == '\t' ? True : False)
# define NEWLINE(c) (c == '\n' || c == '\r' ? True : False)

/* CEILMOD: Round N up to the next full multiple of DIV */
# define CEILMOD(N,DIV) (((N)%(DIV))?((DIV)+(N)-(N)%(DIV)):(N))
# define MAX(A,B) ((A)>(B))?(A):(B)
# define MIN(A,B) ((A)<(B))?(A):(B)

/***************************************************************************
* History Line Structure Definition                                        *
***************************************************************************/
typedef struct History_Symbol_List {
 char * key;                         /* pointer to the history key string */
 char * line;/*Pointer to the allocated character string with size_t bytes*/
 size_t size;    /* Allocated number of bytes, including terminating zero */
 int    required;/*if set, the next argument sets shortlen to strlen(line)*/
 size_t shortlen;      /* short length, excluding not required parameters */
 struct History_Symbol_List *Previous,*Next;/*the previous and next symbols*/
} HSymb;
 
typedef struct History_Block {
 char  * key;                  /* pointer to the history block key string */
 HSymb * history_line_root;           /* previously created history lines */
 HSymb * history_argv_root;              /* actually created history line */
 char  * current_history_line_key;
 struct History_Block *Previous, *Next;    /* the previous and next block */
} HBlock;

/***************************************************************************
* Header List Structure Definition                                         *
***************************************************************************/
typedef struct Header_Element_Flags {
 unsigned short Read;                  /* header element was already read */
} HEFlags;

typedef struct Header_Element {
 char * Key;                          /* pointer to the header key string */
 char * Value;               /* Pointer to the allocated character string */
 HEFlags Flags;
 struct Header_Element *Previous,*Next; /* the previous and next elements */
} HElement;

typedef struct Header_List {
 char     * Key;                      /* pointer to the header key string */
 HElement * ElementRoot;     /* pointer to the allocated character string */
 short      KeyOrderNo;                       /* currently used order key */
 struct Header_List *Previous,*Next;    /* the previous and next elements */
} HList;

/***************************************************************************
* Internal Data Representation                                             *
***************************************************************************/

/*------------------------------------------------------------------------
Internal Data Representation

Data_Block :

'BlockKey' is the unique name of this data block (usually a number).
'BlockKeyLen' is the length of the BlockKey (without stop character '\0')
'TextPos' is the displacement of the header section relative to the
begin of the file.
'BinaryPos' is the displacement of the binary section relative to the
begin of the file.
'TextLen' is the actual length of the header section, including padded
white spaces. It must be a multiple of 'BlockBoundary'.
'BinaryLen' is the length of the binary data section of the data block on
disk. It must be a multiple of 'BlockBoundary'.
'Flags' are reserved for future use.
'PadLen' are the number of white-spaces that must be added to adjust
the header section to a multiple of 'BlockBoundary'.
'SymbolList' is the list of all keywords and values that are listed in
the header.
'KeyOrderNo' is a temporary variable
'DataLen' is the number of meaningful bytes that should be written to the 
file out of the data buffer or that have been read from a file into the 
data buffer.  It cannot be larger than 'BinaryLen'.
'Data' is a typeless pointer to the data buffer. After writing, the data
buffer is automatically discarded, before reading, a sufficiently large
data buffer is allocated, if the NULL pointer is supplied.
---------------------------------------------------------------------------*/

typedef struct Data_Format_Version {
unsigned short Major,                              /* major version number */ 
               Minor;                              /* minor version number */
} DFVersion;

typedef struct Data_File_Flags {
unsigned short ExistingFile,             /* The file exists and was not
                                               opened with 'new' or 'temp' */
               ReadOnlyFile,                     /* This file is read-only */
               TemporaryFile,            /* The file was opened with 'temp'*/
               NoGeneralHeader;          /* The file has no general header */
} DFFlags;

typedef struct Data_Block_Flags {
unsigned short InternalHeader,   /* Keep header, but do not write it to disk */
               InternalData,    /* Keep data internally, never write to disk */
               DiskBlockUsed,                          /* Disk block is used */
               DiskBlockFixed, /* Position and length of disk block is fixed */
               HeaderExternal, /* The header contents must be read from disk */
               HeaderChanged, /* The header contents must be written to disk */
               DataExternal,              /* The data must be read from disk */
               DataChanged,          /* The data needs to be written to disk */
               ExternalDimAlloc,   /* DataDim buffer is externally allocated */
               ExternalDataAlloc,     /* Data buffer is externally allocated */
               BadBlock;                          /* Do not write this block */
} DBFlags;

typedef struct Symbol_Element_Flags {
  unsigned short Read;                    /* header element was already read */
} SEFlags; 

typedef struct Symbol_Element {
char             *Key,                                 /* the keyword itself */
                 *Value;                                       /* it's value */
unsigned long    KeyPos,                      /* position of keyword in file */
                 ValPos;                        /* position of value in file */
unsigned short   KeyLen,                                /* length of keyword */
                 ValLen;                                  /* length of value */
char             *String;    /* string of all continuation keys, if base key */
unsigned short   StringLen;                       /* length of string buffer */
SEFlags          Flags;                                     /* various flags */
struct Symbol_Element *Previous, *Next;     /* the previous and next symbols */
struct Data_Block *Block;                                /* the owning block */
} SElement;

typedef struct Data_Block {
  char           *BlockKey;      /* the block key value of this data block   */
  unsigned short BlockKeyLen;               /* the length of the block key   */
  unsigned long  TextPos,     /* start position of this header in the file   */
                 BinaryPos, /* start position of binary section in the file  */
                 TextLen,    /* actual length of the ASCII header in bytes   */
                 BinaryLen,         /* actual length of the binary section   */
                 PadLen;           /* the number of white-spaces for padding */
  char           *BinaryFileName; /* name of the file with the binary data,
                                                        NULL means this file */
  unsigned long  BinaryFilePos, /* position of binary data in BinaryFileName */
                 BinaryFileLen;  /* actual length of the binary file section */
          /* specification of the actually read and converted data in memory */
  void           *Data;               /* pointer to the typeless data buffer */
  size_t         DataBufferLen;  /* allocated length of data buffer in bytes */
  size_t         DataLen;       /* meaningful length of data buffer in bytes */
  long           DataByteOrder;             /* actual byte order of the data */
  long           DataRasterConfiguration;  /* data raster configuration type */
  long           DataCompression;        /* the actual data compression type */
  long           DataType;                           /* the actual data type */
  long           DataValueOffset;  /* offset must be added to each data item */
  long           *DataDim;                          /* the actual dimensions */
  DBFlags        Flags;                                     /* various flags */
  unsigned long  SequenceNumber;       /* the sequential number of the block */
  SElement       *SymbolList;          /* the symbol list of this data block */
  short          KeyOrderNo;                     /* currently used order key */
  SElement       *KeyOrderNextSymbol;    /* points to next symbol to ordered */
  struct Data_Block *Previous, *Next;        /* previous and next data block */
  struct Data_Chain *Chain;                              /* the owning chain */
} DBlock;

typedef struct Data_Chain {
  char           *ChainKey;                  /* the key string of this chain */
  unsigned short  ChainKeyLen;                /* the length of the chain key */
  unsigned short  Flags;                                    /* various flags */
  DBlock  *BlockList;                        /* the block list of this chain */
  struct Data_Chain *Previous, *Next;        /* previous and next data block */
  struct Data_File  *File;                                /* the owning file */
} DChain;

typedef struct Data_File {                     /* Definition of a file table */
  int       Used;                                   /* 1 if in use, 0 if not */
  char     *Name;                                               /* file name */
  FILE     *Channel;                                          /* i/o channel */
  char     *Buffer;                                  /* pointer to IO Buffer */
  DFVersion Version;                                  /* data format version */
  unsigned long  BlockBoundary;                /* BlockBoundary of this file */
  DFFlags   Flags;                                          /* various flags */
  DBlock   *GeneralBlock;       /* pointer to the general block of this file */
  DBlock   *ActiveBlock;              /* pointer to the current active block */
  unsigned long NextSequenceNumber; /* sequence number of next 
                                                          written data block */
  DBlock   *LastBlockInFile;        /* pointer to the last block in the file */
  DChain   *ChainList;               /* the list of data chains in this file */

} DFile;

/***************************************************************************
* Restrictions                                                             *
***************************************************************************/
#define MaxFiles      EdfMaxFiles      /* maximum number of files streams */
#define MaxKeyLen     EdfMaxKeyLen          /* maximum length of keywords */
#define MaxValLen     EdfMaxValLen            /* maximum length of values */
#define MaxLinLen     EdfMaxLinLen                 /* maximum line length */
#define MaxConLen        MaxLinLen    /* maximum continuation line length,*/
                                      /* must be smaller than MaxValLen   */
#define MaxDataChains EdfMaxDataChains /* max. number of user data chains */
#define BufferSize    EdfBufferSize               /* size of input buffer */
#define MaxDimensions EdfMaxDimensions   /* limiting number of dimensions */
#define StartHeader            '{'  /* used to mark the start of a header */
#define EndHeader              '}'    /* used to mark the end of a header */
#define Separator              '=' /* used to separate keyword from value */
#define Terminator             ';'         /* marker for end of statement */
#define FirstHeader             1   /* Header Numbers start at this value,
                                       Header Numbers are incremented by one
                                       each time a data block is written. */
#define Comment                '#'  /* comment if first character in line */
#define Continuation  '~' /* separator between key and continuation number*/

/***************************************************************************
* General strings                                                          *
***************************************************************************/
# define NONE                     "None"
# define UNDETERMINED             "Undetermined"
# define INVALID                  "Invalid"

/***************************************************************************
* Version and general setup values                                         *
***************************************************************************/
# define DATA_FORMAT_NAME         "EDF"
# define DATA_FORMAT_VERSION      "2.40"
# define DATA_BLOCKS_DEFAULT      UNDETERMINED      /* undetermined value */ 
# define BLOCK_BOUNDARY           512                            /* bytes */
# define PAD_WIDTH                78   /* width of padded text +2-0 chars */
# define MAX_LINE_WIDTH           0ul      /* maximum width of text lines */ 

/***************************************************************************
* Header Keywords                                                          *
* All keywords that are relevant for the physical data structure start     *
* with DATA_FORMAT_PREFIX                                                  * 
***************************************************************************/
# define DATA_FORMAT_PREFIX       DATA_FORMAT_NAME"_"
# define DATA_FORMAT_VERSION_KEY  DATA_FORMAT_PREFIX"DataFormatVersion"
# define DATA_BLOCKS_KEY          DATA_FORMAT_PREFIX"DataBlocks"
# define BLOCK_BOUNDARY_KEY       DATA_FORMAT_PREFIX"BlockBoundary"
# define BLOCK_ID_KEY             DATA_FORMAT_PREFIX"DataBlockID"
# define BINARY_SIZE_KEY          DATA_FORMAT_PREFIX"BinarySize"
# define HEADER_SIZE_KEY          DATA_FORMAT_PREFIX"HeaderSize"
# define BINARY_FILE_NAME_KEY     DATA_FORMAT_PREFIX"BinaryFileName"
# define BINARY_FILE_POSITION_KEY DATA_FORMAT_PREFIX"BinaryFilePosition"
# define BINARY_FILE_SIZE_KEY     DATA_FORMAT_PREFIX"BinaryFileSize"

/* Version 1.xx keywords relevant for the physical data structure */
# define V1_HEADER_ID_KEY         "HeaderID"
# define V1_SIZE_KEY              "Size"
# define V1_IMAGE_KEY             "Image"
# define V1_SUPPRESS              ".Psd"

/* Version 2.xx keywords relevant for the data ordering */
# define GENERAL_CHAIN_KEY        "General"
# define GENERAL_BLOCK_KEY        "All"
//+++++++++++++ # define DEFAULT_BLOCK_KEY        "Default"
# define CHAIN_KEY_DEFINITION     DATA_FORMAT_PREFIX"DataKey-"
//+++++++++++++ # define CLASS_KEY                "Image"

/* keywords describing the binary data */ 
# define DIMENSION_KEY_PREFIX     "Dim_"   /* key prefix for array dimension */
# define BYTE_ORDER_KEY           "ByteOrder" /* byte order of a single item */ 
# define LOW_BYTE_FIRST           "LowByteFirst"         /* byte order value */
# define HIGH_BYTE_FIRST          "HighByteFirst"        /* byte order value */
# define FIXED_BYTE_ORDER         "FixedByteOrder"    /* byte order is fixed */
# define RASTER_CONFIGURATION_KEY "DataRasterConfiguration"
# define COMPRESSION_KEY          "Compression"
# define DATA_TYPE_KEY            "DataType"   /* data type of a single item */
# define DATA_VALUE_OFFSET_KEY    "DataValueOffset"  
# define FLOAT_IEEE32             "FloatValue"            /* data type value */

/* optional keywords */
# define HISTORY_KEY_PREFIX       "History-"

/* Version 1.xx keywords */
# define V1_HEADER_ID_KEY         "HeaderID"
# define V1_SIZE_KEY              "Size"
# define V1_IMAGE_KEY             "Image"

/***************************************************************************
* The following translation tables correspond to public enumerated         *
* constants. The enums start with 1. 0 to specify an invalid value.        *
* The string tables start with INVALID and end with (char *) NULL.         *
***************************************************************************/

/***************************************************************************
* Data Type Translation Tables                                             *
* The indices of the tables correspond to 'enum DType'.                    * 
***************************************************************************/

PRIVATE const char * DTypeStringsAliases[18] =
           { INVALID,
             "Unsigned8",   "Signed8",      "Unsigned16",    "Signed16",
             "Unsigned32",  "Signed32",     "Unsigned64",    "Signed64",
             "FloatIEEE32", "DoubleIEEE64", "UnAssigned",    "UnAssigned",
             "FloatVAX32",  "DoubleVAX64",  "FloatConvex32", "DoubleConvex64",
             (const char *) NULL };

PRIVATE const char * DTypeStringsAliases1[18] =
      { INVALID,
        "UnsignedByte",    "SignedByte",    "UnsignedShort", "SignedShort",
        "UnsignedLong",    "SignedLong",    "Unsigned64",    "Signed64",
        "Float",           "DoubleValue",   "UnAssigned",    "UnAssigned",
        "FloatVAX32",      "DoubleVAX64",   "FloatConvex32", "DoubleConvex64",
        (const char *) NULL };

PRIVATE const char * DTypeStrings[18] =
      { INVALID,
        "UnsignedByte",    "SignedByte",    "UnsignedShort", "SignedShort",
        "UnsignedInteger", "SignedInteger", "Unsigned64",    "Signed64", 
        "FloatValue",      "DoubleValue",   "UnAssigned",    "UnAssigned",
        "FloatVAX32",      "DoubleVAX64",   "FloatConvex32", "DoubleConvex64",
        (const char *) NULL };

PRIVATE const size_t DSize[17] = 
      { 0lu,  
        1lu,              1lu,              2lu,             2lu,
        4lu,              4lu,              8lu,             8lu,
        4lu,              8lu,              0lu,             0lu,
        4lu,              8lu,              4lu,             8lu };

/***************************************************************************
* Byte Order Translation Tables                                            *
* The constants correspond to 'enum BOrder'.                               *    
***************************************************************************/

PRIVATE const char * BOrderStrings[5] = { INVALID, 
                                  LOW_BYTE_FIRST, HIGH_BYTE_FIRST,
                                  FIXED_BYTE_ORDER,
                                  (const char *) NULL };

/***************************************************************************
* Data Compression Translation Tables                                      *
* The constants correspond to 'enum DCompression'.                         *    
***************************************************************************/

PRIVATE const char * DCompressionStrings[3] =        { INVALID, 
                                               NONE, 
                                               (const char *) NULL };
PRIVATE const char * DCompressionStringsAliases[3] = { INVALID, 
                                               "UnCompressed" , 
                                               (const char *) NULL };
PRIVATE const char * DCompressionStringsAliases1[3] = { INVALID, 
                                               "NoSpecificValue" , 
                                               (const char *) NULL };

/***************************************************************************
* Data block classes and instances                                         *
* The constants correspond to 'enum DBClass' and 'enum DBInstance'.        *
***************************************************************************/

PRIVATE const char * DBClassStrings[4] =             { INVALID,
                                              "General",
                                              "Image",
                                               (const char *) NULL };
PRIVATE const char * DBInstanceStrings[4] =          { INVALID,
                                               "Psd" ,
                                               "Error",
                                               (const char *) NULL };

/***************************************************************************
* Private constants and variables                                          *
***************************************************************************/

PRIVATE char *DataFormatVersion = DATA_FORMAT_VERSION;
PRIVATE char *New = "new", *Old = "old", *Any = "any", *Read = "read";
PRIVATE char *Temp = "temp";

PRIVATE char white_spaces[7] = { ' ', '\t', '\r', '\n', '\f', '\v', '\0' };
PRIVATE char quote_chars[2] = { '\"', '\0' };
PRIVATE char numbers[11] = { '0','1','2','3','4','5','6','7','8','9', '\0' };
PRIVATE char tobequoted[8] = { '(', ')', '[', ']', '{', '}', '*' , '\0' };

// PRIVATE char header_begin[5] = { '\n', StartHeader, '\r', '\n', '\0' };
PRIVATE char header_begin[4] = { StartHeader, '\r', '\n', '\0' };
PRIVATE char header_end[5] = { '\r', '\n', EndHeader, '\n', '\0' };
PRIVATE char symbol_separator[4] = { ' ', Separator, ' ', '\0' };
PRIVATE char symbol_terminator[5] = { ' ', Terminator, '\r', '\n', '\0' };
PRIVATE char line_feed[3] = { '\n', '\0', '\0' }; /* DOS { '\r', '\n', '\0' }*/

PRIVATE int      InitTable = 0;
PRIVATE DFile    FileTable[MaxFiles];
PRIVATE char     IDBuffer[3*MaxKeyLen+1];
PRIVATE int      EDFIO_debug = 0;
PRIVATE int      write_general_block = 0;  /* default without general header */
PRIVATE int      write_headersize = 1;          /* default: with header size */
PRIVATE unsigned long minimum_headersize_out = 8192l; /* minimum header size */ 
PRIVATE int      data_type_out       = InValidDType;/* no specific data type */
PRIVATE long     data_value_offset_out = 0l;         /* default is no offset */
PRIVATE int      bsl_input_byteorder = InValidBOrder;  /* default: not spec. */ 
PRIVATE unsigned long max_line_width = MAX_LINE_WIDTH; /* default line width */

enum DFType { InValidDFType, EdfType, BslType };        /* data format types */
enum SMode { CaseSensitiveSort, UpperCaseSort, NumberSort };

/***************************************************************************
* Prototypes                                                               *
***************************************************************************/
PRIVATE const char * ByteOrder2String( int byte_order );
PRIVATE const char * Compression2String( int data_compression );
PRIVATE SElement **  keyorder_ordersymbols( DBlock * block );
PRIVATE int byteorder ( void );
PRIVATE int history_line_new  ( HSymb ** proot,
                        const char * history_line_key, size_t history_size,
                        HSymb ** phline );
PRIVATE int keyorder_compare( const char *key, const char *criterion );

PRIVATE long *get_data_dim ( DBlock * block );
PRIVATE int get_data_type ( DBlock * block );

/***************************************************************************
* Routines that return internal parameters                                 *
***************************************************************************/

unsigned long edf_maxfiles( void )
{ return ( MaxFiles ); }

unsigned long edf_maxkeylen ( void )
{ return ( MaxKeyLen ); }

unsigned long edf_maxvallen ( void )
{ return ( MaxValLen ); }

unsigned long edf_maxlinlen ( void )
{ return ( MaxLinLen ); }

unsigned long edf_maxdatachains ( void )
{ return ( MaxDataChains ); }

unsigned long edf_buffersize ( void )
{ return ( BufferSize ); }

unsigned long edf_maxdimensions ( void )
{ return ( MaxDimensions ); }

int edf_byteorder ( void )
{ return (  byteorder ( ) ); }

/***************************************************************************
* Routines                                                                 *
***************************************************************************/
/*+++------------------------------------------------------------------------
NAME

   edfio_version

SYNOPSIS

   char *edfio_version         ( void )

DESCRIPTION
   Returns a pointer to the version string of the module edfio

RETURN VALUE
   Pointer to the version string
-------------------------------------------------------------------------+*/
char *edfio_version         ( void )                                 /*---*/
{ 
# ifdef EVERSION
#   define _EDFIO_VERSION EDFIO_VERSION" ("EVERSION")"
# else
#   define _EDFIO_VERSION EDFIO_VERSION
# endif
  return ( _EDFIO_VERSION );
} /* edfio_version */

/*+++------------------------------------------------------------------------
NAME

   byteorder 

SYNOPSIS

   int byteorder ( void )

DESCRIPTION
  Checks the byte order of the machine
  byteorder : HighByteFirst, big endian byte order
  byteorder : LowByteFirst, little endian byte order

RETURN VALUE
  byteorder : HighByteFirst | LowByteFirst
-------------------------------------------------------------------------+*/
PRIVATE int byteorder ( void ) 
{ short int one = 1;
  int value;
  switch ((int) *(char *) &one) {
    case 1: value = LowByteFirst; break;
    case 0: value = HighByteFirst; break;
   default: value = InValidBOrder;
  } 
  return( value );
} // byteorder

/*+++------------------------------------------------------------------------
NAME

  edf_general_block  --- write/don't write file with general header

SYNOPSIS

  int edf_general_block  ( int writetodisk );

DESCRIPTION
  writetodisk : 1, write general header
  writetodisk : 0, do not write general header (default)

RETURN VALUE
  SUCCESS: 0
---------------------------------------------------------------------------*/
int edf_general_block          ( int writetodisk )                    /*---*/
{ write_general_block = writetodisk; 
  return(0);
} /* edf_general_block */

/*+++------------------------------------------------------------------------
NAME

  edf_headersize  --- write/don't write header size into header

SYNOPSIS

  int edf_headersize  ( int writetodisk );

DESCRIPTION
  writetodisk : 1, write header size into header (default)
  writetodisk : 0, do not write header size

RETURN VALUE
  SUCCESS: 0
---------------------------------------------------------------------------*/
int edf_headersize             ( int writetodisk )                    /*---*/
{ write_headersize = writetodisk;
  return(0);
} /* edf_headersize */

/*+++------------------------------------------------------------------------
NAME

  edf_set_minimumheadersize  --- sets minimum header size

SYNOPSIS

  int edf_set_minimumheadersize( unsigned long minimumheadersize_out )

DESCRIPTION
  The actual output headersize is rounded to the next multiple of
  BLOCK_BOUNDARY that is larger or equal to the maximum of
  minimumheadersize_out and the minimum required size to write all
  header values.

RETURN VALUE
  SUCCESS: 0
---------------------------------------------------------------------------*/
int edf_set_minimumheadersize   ( unsigned long minimumheadersize_out )
{ minimum_headersize_out = minimumheadersize_out;
  return(0);
} /* edf_set_minimumheadersize */

/*+++------------------------------------------------------------------------
NAME

  edf_set_datatype --- set data type of all output files
 
SYNOPSIS
 
  int edf_set_datatype  ( int datatype_out );
 
DESCRIPTION
 
RETURN VALUE
  SUCCESS: 0
---------------------------------------------------------------------------*/
int edf_set_datatype         ( int datatype_out )                     /*---*/
{ data_type_out = datatype_out;
  return(0);
} /* edf_set_datatype */                                                                                                                     
/*+++------------------------------------------------------------------------
NAME
 
  edf_set_datavalueoffset --- set data value offset of all output files
 
SYNOPSIS
 
  int edf_set_datavalueoffset  ( long int datavalueoffset_out );
 
DESCRIPTION
 
RETURN VALUE
  SUCCESS: 0
---------------------------------------------------------------------------*/
int edf_set_datavalueoffset  ( long int datavalueoffset_out )         /*---*/
{ data_value_offset_out = datavalueoffset_out;
  return(0);
} /* edf_set_datavalueoffset */

/*+++------------------------------------------------------------------------
NAME

  edf_set_bsl_input_byteorder --- set the byteorder for all bsl input files

SYNOPSIS

  int edf_set_bsl_input_byteorder      ( int byteorder )

DESCRIPTION
  Changes the byte order of all bsl input files to byteorder
  byteorder : HighByteFirst, big endian byte order
  byteorder : LowByteFirst, little endian byte order
  (default byte order: INTERNAL_BYTEORDER)

RETURN VALUE
  0: success
--------------------------------------------------------------------------+*/
int edf_set_bsl_input_byteorder      ( int byteorder )                /*---*/
{ bsl_input_byteorder = byteorder;
  return(0);
} /* edf_set_bsl_input_byteorder */ 

/*+++------------------------------------------------------------------------
NAME

  edf_set_max_line_width  --- set maximum line width 

SYNOPSIS

  int edf_set_max_line_width  ( unsigned long width );

DESCRIPTION
  width = 0 : no limitation 
  width > 0 : length of header line limited to width (text lines only)

RETURN VALUE
  SUCCESS: 0

HISTORY
  2000-04-07 : Peter Boesecke
---------------------------------------------------------------------------*/
int edf_set_max_line_width  ( unsigned long width )                   /*---*/
{ max_line_width = width;
  return(0);
} /* edf_set_max_line_width */

/*---------------------------------------------------------------------------
NAME

       getpath_edf --- extract path from filename

SYNOPSIS

       char *getpath_edf ( char *buffer, size_t buflen, const char * filename );

DESCRIPTION
       Extract path from filename and copies it into buffer. If the buffer is
       too short or another error has occurred NULL is returned. If filename
       has no path the current working directory is determined with
       getcwd and returned.

HISTORY
       23-Mar-1998 Peter Boesecke
---------------------------------------------------------------------------*/
char * getpath_edf ( char * buffer, size_t buflen, const char * filename )
{ size_t str_len;
  char *ps;

  str_len = strlen(filename);
  if (str_len>=buflen) return( (char *) NULL );

  if ((ps = strrchr( filename, (int) '/' ))) {
    memcpy( buffer, filename, (ps-filename) );
    buffer[(ps-filename)]='\0'; }
  else GETCWD( buffer, buflen );

  str_len = strlen(buffer);
  if (str_len<buflen) { buffer[str_len]='/';buffer[str_len+1]='\0'; }
    else buffer[0] = '\0';

  return ( buffer );

} /* getpath_edf */

/*===string_access BEGIN===================================================*/

/*---------------------------------------------------------------------------
NAME

   catstr --- allocates memory and copies two character strings into it

SYNOPSIS

   char * catstr( const char * a, const char * b );

DESCRIPTION
  Allocates strlen('a')+strlen('b')+1 bytes of memory and copies 'a' and 'b'
  into it. In case of success the pointer to the allocated memory is returned. 
  The null pointer is returned in case of an error.
  If 'a' or 'b' are NULL pointers the NULL pointer is returned.

RETURN VALUE
  Returns the pointer to the allocated string or (char *) NULL in case
  of an error.
---------------------------------------------------------------------------*/
char * catstr( const char * a, const char * b )
{ char * value;
  size_t lena, lenb;

  if (!(a&&b)) return( (char *) NULL );
  lena = strlen(a);
  lenb = strlen(b);
  if (!(value = (char *) malloc(lena+lenb+1))) 
    return((char *) NULL);
  (void) STRCPY(value,a);
  (void) STRCPY((value+lena),b);

  return( value );

} /* catstr */

/*---------------------------------------------------------------------------
NAME

   newstr --- allocate memory and copy a character string into it

SYNOPSIS

   char * newstr( const char * string );

DESCRIPTION
  Allocates strlen('string')+1 bytes of memory and copies 'string' into it.
  In case of success the pointer to the allocated memory is returned. The
  null pointer is returned in case of an error.
  If 'string' is the NULL pointer the NULL pointer is returned.

RETURN VALUE
  Returns the pointer to the allocated string or (char *) NULL in case
  of an error.
---------------------------------------------------------------------------*/
char * newstr( const char * string )
{ char * value;

  if (!string) return( (char *) NULL );
  if (!(value = (char *) malloc(STRLEN(string)+1))) return((char *) NULL);
  (void) STRCPY(value,string);

  return( value );

} /* newstr */

/*---------------------------------------------------------------------------
NAME
 
   newstrn --- create new string with maximum length
 
SYNOPSIS
 
   char * newstr( const char * string, size_t maxlen );
 
DESCRIPTION
  Allocates MIN(maxlen,strlen('string'))+1 bytes of memory and copies not 
  more than maxlen characters of 'string' into it.
  In case of success the pointer to the allocated memory is returned. The
  null pointer is returned in case of an error.
  If 'string' is the NULL pointer the NULL pointer is returned.
 
RETURN VALUE
  Returns the pointer to the allocated string or (char *) NULL in case
  of an error.
---------------------------------------------------------------------------*/
char * newstrn( const char * string, size_t maxlen )
{ size_t newlen;
  char * value;
 
  if (!string) return( (char *) NULL );
  if (!maxlen) return( (char *) NULL );
  newlen = MIN(maxlen,STRLEN(string));
  if (!(value = (char *) malloc(newlen+1))) return((char *) NULL);
  (void) strncpy(value,string,newlen);
  value[newlen]='\0'; // terminating zero
 
  return( value );
 
} /* newstrn */ 

/*---------------------------------------------------------------------------
NAME

   newstring --- allocates and initializes memory for a string with stringlen 

SYNOPSIS

   char * newstring( size_t stringlen );

DESCRIPTION
  Allocates 'stringlen'+1 bytes of memory and initializes the first element
  with a terminating 0.
  In case of success the pointer to the allocated memory is returned. The
  null pointer is returned in case of an error.

RETURN VALUE
  Returns the pointer to the allocated string or (char *) NULL in case
  of an error.
---------------------------------------------------------------------------*/
char * newstring( size_t stringlen )
{ char * value;

  if (!(value = (char *) malloc(stringlen+1))) return((char *) NULL);
  value[0] = '\0';

  return( value );

} /* newstring */

/*---------------------------------------------------------------------------
NAME

   newdim --- allocate memory and copy the data dimension into it 

SYNOPSIS

   long * newdim( const long * dim );

DESCRIPTION
  Allocates a long array with dim[0]+2 elements and copies 'dim' into it.
  The array element dim[dim[0]+1] is not copied. 
  If 'dim' is the NULL pointer the NULL pointer is returned.

RETURN VALUE
  Returns the pointer to the allocated dimension array and (long *) NULL 
  in case of an error. In case of an error no memory is allocated.
---------------------------------------------------------------------------*/
long * newdim( const long * dim )
{ long * newdimension;
  long N, idim;

  if (!dim) return( (long *) NULL );
  N = dim[0];
  if (!(newdimension = (long *) malloc(sizeof(long)*(N+2l)) ))
    return((long *) NULL);
  /* copy dim to newdimension */
  for (idim=0;idim<=N;idim++) newdimension[idim] = dim[idim];
  newdimension[N+1l]=0l;

  return( newdimension );

} /* newdim */

/*---------------------------------------------------------------------------
NAME

   copydim --- copy the data dimension from dim to buffer

SYNOPSIS

   long * copydim( long buffer[], long N, const long dim[] )

DESCRIPTION
  Copies the elements of dim[] between dim[0] and dim[dim[0]] to buffer.
  N is the required output dimension number. At least 'N'+1 elements must have
  been allocated for 'buffer' ('long buffer[N+2]'). 'buffer[0] is set to 'N'.
  If 'N' is larger than 'dim[0]' the remaining elements of buffer are filled
  with 1. If 'N' is smaller than dim[0] the routine does generally stop with
  an error, except in the case where all elements dim[>N] are 1. 

'N' must be at least
  equal to dim[0],

RETURN VALUE
  Returns the pointer to buffer or (long *) NULL in case of an error.
---------------------------------------------------------------------------*/
long * copydim( long buffer[], long N, const long dim[] )
{
  long idim, idim_max;

  if ( (!dim) || (!buffer) ) return ( (long *) NULL );
  if (dim[0]<N) idim_max = dim[0]; else idim_max = N;
  /* number of output dimensions */
  buffer[0] = N;
  /* copy the input dims */
  for (idim=1l;idim<=idim_max;idim++) {
    buffer[idim] = dim[idim];
    }
  /* set the remaining output dims to 1 */
  for (idim=idim_max+1l;idim<=N;idim++) {
    buffer[idim] = 1l;
    }
  /* are the remaining input dims 1? */
  for (idim=N+1l;idim<=dim[0];idim++) {
    if (dim[idim]!=1l) return( (long *) NULL );
    }

  return ( buffer );

} /* copydim */

/*+++------------------------------------------------------------------------
NAME

  edf_dataformat_version --- return edf data format version string

SYNOPSIS

  char * edf_dataformat_version ( void )

DESCRIPTION
  Returns the edf data format version string.

RETURN VALUE
  Pointer to edf data format version string.
  -------------------------------------------------------------------------+*/
char * edf_dataformat_version ( void )                                 /*---*/
{ return( DataFormatVersion );
} /* edf_dataformat_version */

/*---------------------------------------------------------------------------
s2u_long( string )
Conversion of a string to unsigned long. In case of an error the return 
value is 0.
---------------------------------------------------------------------------*/
unsigned long s2u_long( const char * string )
{ unsigned long value;

  if ( sscanf(string,"%lu", &value  ) < 1 ) return( 0u );

  return(value);

} /* s2u_long */

/*---------------------------------------------------------------------------
u_long2s( buffer, value )
Conversion of an unsigned long integer to a string and output to buffer. The 
pointer to buffer is returned. In case of an error the null pointer is returned.
---------------------------------------------------------------------------*/
char * u_long2s( char buffer[], unsigned long value )
{
  if ( sprintf(buffer,"%lu", value ) < 1 ) return( (char *) NULL);

  return(buffer);
} /* u_long2s */

/*---------------------------------------------------------------------------
long2s( buffer, value )
Conversion of a long integer to a string and output to buffer. The pointer to
buffer is returned. In case of an error the null pointer is returned.
---------------------------------------------------------------------------*/
char * long2s( char buffer[], long int value )
{
  if ( sprintf(buffer,"%ld", value ) < 1 ) return( (char *) NULL);

  return(buffer);
} /* long2s */

/*---------------------------------------------------------------------------
float2s( buffer, value )
Conversion of float to string and output to buffer. The pointer to
buffer is returned. In case of an error the null pointer is returned.
---------------------------------------------------------------------------*/
char * float2s( char buffer[], float value )
{
  if ( sprintf(buffer,"%g", value ) < 1 ) return( (char *) NULL);

  return(buffer);

} /* float2s */

/*---------------------------------------------------------------------------
strtolower

DESCRIPTION
Converts the character string str into lower case.
---------------------------------------------------------------------------*/
char *strtolower ( char *str )
{ char *ps=str;
  while (*ps) { *ps=tolower(*ps); ps++; }
  return(str);
} /*strtolower*/

/*---------------------------------------------------------------------------
NAME

    has_extension

SYNOPSIS

    int has_extension ( const char * str );

DESCRIPTION

Returns 1 if str contains a '.', 0, if not.

HISTORY
30-Mar-2001 Peter Boesecke
---------------------------------------------------------------------------*/
int has_extension ( const char * str ) 
{  int dot = (int) '.';
   if ( strchr( str, dot ) )
     return(1); /* with extension */ else return(0); /* no extension */

} /* has_extension */

/*---------------------------------------------------------------------------
NAME

    is_white

SYNOPSIS

    int is_white ( char c );

DESCRIPTION

Returns 1 if c is a white space, 0, if not.

HISTORY
18-Jan-1998 Peter Boesecke
---------------------------------------------------------------------------*/
int is_white ( char c )                                               
{
   if ( strchr( white_spaces, (int) c ) )
     return(1); /* white space */ else return(0); /* no white space */

} /* is_white */

/*---------------------------------------------------------------------------
NAME

    is_quote

SYNOPSIS

    int is_quote ( char c );

DESCRIPTION

Returns 1 if c is a quotation character, 0, if not.

HISTORY
27-Feb-2005 Peter Boesecke
---------------------------------------------------------------------------*/
int is_quote ( char c )
{
   if ( strchr( quote_chars, (int) c ) )
     return(1); /* quote */ else return(0); /* no quote */

} /* is_quote */

/*---------------------------------------------------------------------------
NAME

    is_number

SYNOPSIS

    int is_number ( char c );

DESCRIPTION

Returns 1 if c is a number ('0'..'9'), 0 if not.

HISTORY
15-Feb-1998 Peter Boesecke
---------------------------------------------------------------------------*/
int is_number ( char c )
{
   if ( strchr( numbers, (int) c ) )
     return(1); /* number */ else return(0); /* no number */

} /* is_number */

/*---------------------------------------------------------------------------
NAME

    is_tobequoted 

SYNOPSIS

    int is_tobequoted ( char c );

DESCRIPTION

Returns 1 if c is a character that must be quoted on a command line, 0, if not.

HISTORY
1999-06-26 Peter Boesecke
---------------------------------------------------------------------------*/
int is_tobequoted ( char c )
{
   if ( strchr( tobequoted, (int) c ) )
     return(1); /* must be quoted */ else return(0); /* no quotes */

} /* is_tobequoted */

/*---------------------------------------------------------------------------
NAME

    white2space

SYNOPSIS

    char white2space ( const char c );

DESCRIPTION
If c is a white space it is replaced by space, otherwise c.

HISTORY
18-Jan-1998 Peter Boesecke
---------------------------------------------------------------------------*/
char white2space ( const char c )
{ if (is_white(c)) return(' '); else return(c);
} /* white2space */

/*---------------------------------------------------------------------------
strtrm

DESCRIPTION
Removes leading and trailing white spaces from input string str and copies
it to buffer. The buffer is filled as much as possible, buffer and str can 
be identical. If str is the NULL pointer it is handled like an empty string.
In case of success, the pointer to buffer is returned, otherwise NULL.
The routine fails if buffer is the NULL pointer or buflen <1.
---------------------------------------------------------------------------*/
char *strtrm ( char buffer[], unsigned long buflen, const char *str )
{ const char *ps, *pstart, *pend;
  char *pb;
  unsigned long len;
  long unsigned int i;

  if ((!buffer)||(buflen<1)) return ( (char*) NULL );

  buffer[0]='\0';
  if (!str) return( buffer );

  len=strlen(str);

  pstart=str;
  pend=str+len;

  // search start (first non-white character in str)
  while ( (pstart<pend) && is_white(*pstart) ) pstart++;

  // search end (last non-white character in str)
  pend--;
  while ( (pstart<pend) && is_white(*pend) ) pend--;

  ps=pstart;
  pb=&buffer[0];
  for (i=1;i<buflen;i++) {
    if (ps>pend) break;
    *pb++=*ps++;
  }
  *pb='\0';

  return( buffer );

} /* strtrm */

/*---------------------------------------------------------------------------
NAME

   replace_string -- replaces *pstring with string.

SYNOPSIS

   int replace_string( char ** pstring, const char * string );

DESCRIPTION
   The memory to which pstring points is released. pstring is replaced
   by a pointer to a new allocated memory with strlen(string)+1 bytes.
   'string' is copied into this memory.

CHANGED
   The memory at *pstring is released, new memory is allocated and filled
   with 'string', pstring is replaced by a pointer to this memory.

RETURN VALUE
    0: OK
   -1: error, probably memory could not be allocated.
---------------------------------------------------------------------------*/
int replace_string( char ** pstring, const char * string )
{ const char * nix = { '\0' };
  char * tmp;

  if (!(pstring)) return(-1); /* cannot be changed */
  if (!(string)) string = nix;

  tmp = newstr( string ); if (!tmp ) return(-1);
  if (*pstring) free(*pstring);
  *pstring = tmp;

  return(0);

} /* replace_string */

/*---------------------------------------------------------------------------
NAME
 
    breakval - break a value string 
 
SYNOPSIS
 
    char * breakval( char buffer[], unsigned long buflen, const char * string,
                     unsigned long pos1, unsigned long pos2 )
 
DESCRIPTION
Breaks a value string into several lines. A symbol break ("'\r''\n''\'")
is inserted after the last space in a line before pos1. If no space appears 
the string is broken at pos1. After the first inserted break the position 
is reset to 1 and the following lines are broken at pos2. The input string 
must be a result of str2val. '\r' and '\n' characters are ignored. This 
transformation should only be applied once.

If pos1 or pos2 are less or equal to 1 the line is not broken.
 
Appends a 
RETURN VALUES
pointer to the value 
 
HISTORY
2000-04-06 Peter Boesecke
---------------------------------------------------------------------------*/
char * breakval( char buffer[], unsigned long buflen, const char * string,
                 unsigned long pos1, unsigned long pos2 )
{ unsigned long i, is;
  unsigned long pos, poss;
  unsigned long breakpos = pos1;
  char *pb = buffer, *pbs;
  const char *ps = string, *pss;
  char c;

  /* initialize buffer */
  buffer[0] = '\0';
  if (!string) return( buffer );

  if (pos1<=1) pos1 = 0U; // minimum length 2 characters
  if (pos2<=1) pos2 = 0U; // minimum length 2 characters

  i = 0U;
  pbs = pb; pss = ps; 
  while ( (i<buflen-1) && (*ps) ) {
    pos = 0U;
    poss = pos; is = i;
    // break at last space
    while ( (i<buflen-1) && (*ps) && (pos<breakpos-1) ) {
      if ((*ps) == '\r') ps++; // ignore cr
      else if ((*ps) == '\n') ps++; // ignore lf
      else if ((*ps) == '\\') { // copy escape sequence ...
        c = *ps; ps++;
        if ((*ps) == ('\r')) { ps++; c = (char) 0; }
        if ((*ps) == ('\n')) { ps++; c = (char) 0; }
        if ((c) && (i<buflen-2)) { // but ignore '\'[cr]lf
          *pb = c; pb++; *pb = *ps; pb++; ps++;
          i+=2; pos+=2;
          }
        }
      else if (((*ps) == ' ') && (i<buflen-4)) { // copy space and remember
        if (i<buflen-1) {
          *pb = *ps; pb++; ps++; i++; pos++;
          }
        pbs = pb; pss = ps; poss = pos; is = i; // remember position
        } 
      else { *pb = *ps; pb++; ps++; i++; pos++; }
      }
    // break
    if ((poss!=0U) && (*ps)) { pb = pbs; ps = pss; pos = poss; i = is; }
    if (*ps) {
      if (ps[1]) {
        if (i<buflen-3) {
          *pb = '\\'; pb++; *pb = '\r'; pb++; *pb = '\n'; pb++;
          i+=3; pos=0U;
          pbs = pb; pss = ps; poss = pos; is = i; breakpos = pos2;
          } 
        } else { if (*ps!='\\') { *pb = *ps; pb++; ps++; i++; pos++; } }
      }
    }
  // copy rest
  while ( (i<buflen-1) && (*ps) ) {
    if ((*ps) == '\r') ps++;  // ignore cr
    else if ((*ps) == '\n') ps++; // ignore lf
    else if ((*ps) == '\\') { // copy escape sequence ...
      c = *ps; ps++;
      if ((*ps) == ('\r')) { ps++; c = (char) 0; }
      if ((*ps) == ('\n')) { ps++; c = (char) 0; }
      if ((c)&&(i<buflen-2)) { // but ignore '\'[cr]lf
        *pb = c; pb++; *pb = *ps; pb++; ps++;
        i+=2; pos+=2;
        }
      }
    else { *pb = *ps; pb++; ps++; i++; pos++; 
         }
    }
  // terminate string

  *pb = '\0'; pb++;
 
  return( buffer );

} /* breakval */

/*---------------------------------------------------------------------------
NAME

    val2str - transform a value string to a standard string

SYNOPSIS

    char *val2str( char buffer[], unsigned long buflen, const char * value );

DESCRIPTION
Transformation of a value string to a string. A value string contains
'\' escape sequence for some characters which must not appear in the header
section. The result is written to buffer, which has the size buflen. The
maximum length of the output string is buflen-1. The transformation starts
witht the first non white-space in value and stops with the last non
white-space of value. If buffer is too short the value is only partially
transformed. 

A NULL pointer (value) is handled like an empty string.

The following transformations are done:

- Removing leading and trailing white spaces.
- Removing encapsulation between double quotes.
- Transformation of escape sequences to ASCII characters:

'\''l'  => line_feed
'\''r'  => '\r'
'\''n'  => '\n'
'\''s'  => ' '
'\''t'  => '\t'
'\''v'  => '\v'
'\''f'  => '\f'
'\''('  => '{'
'\'')'  => '}'
'\'':'  => ';'
'\''\r' => skipped
'\''\n' => skipped
'\'<character> => <character>
'\r' and '\n' are skipped

A single '\' at the end of the string is skipped.

RETURN VALUES
pointer to string

HISTORY
25-Jan-1998 Peter Boesecke
06-Apr-2000 PB '\''\r' => skipped and '\''\n' => skipped
---------------------------------------------------------------------------*/
char *val2str( char buffer[], unsigned long buflen, const char * value )
{  unsigned long i;
   unsigned long last; /* points after the last non-white character */
   char *pb = buffer; 
   const char *pv = value;

   /* initialize buffer */
   *pb = '\0';

   /* return empty string if value is NULL */
   if (!value) return( buffer );

   /* skip everything before the first non-white character */
   while ( is_white(*pv) && (*pv) ) { pv++; }

   /* Remove leading double quote */
   if ( (*pv) == '"' ) pv++;
   
   /* Read value until end of value string or until buffer full */
   last = i   = 0U;
   while ( (i<buflen-strlen(line_feed)) && (*pv) ) {
     if ( ((*pv) == '\r') || ((*pv) == '\n') ) { pv++; } else
       if ((*pv) == '\\') {
         pv++;
         if (*pv) {
           if ((*pv) == ':') { *pb = ';'; pb++; i++; } 
             else if ((*pv) == ')') { *pb = '}'; pb++; i++; } 
             else if ((*pv) == '(') { *pb = '{'; pb++; i++; }
             else if ((*pv) == 's') { *pb = ' '; pb++; i++; }
             else if ((*pv) == 't') { *pb = '\t'; pb++; i++; }
             else if ((*pv) == 'f') { *pb = '\f'; pb++; i++; }
             else if ((*pv) == 'v') { *pb = '\v'; pb++; i++; }
             else if ((*pv) == 'n') { *pb = '\n'; pb++; i++; } 
             else if ((*pv) == 'r') { *pb = '\r'; pb++; i++; } 
             else if ((*pv) == 'l') { 
                    *pb='\0';strcat(pb,line_feed); 
                    pb+=strlen(line_feed);
                    i+=strlen(line_feed); }
             else if ( ((*pv) == '\r') || ((*pv) == '\n') ) { pv++; }
             else {*pb = *pv; pb++; i++; }
           pv++; last = i;
           } 
         } else { *pb = *pv; pv++; i++; if (!(is_white(*pb))) last = i; pb++; }
     } /* while */

   /* Remove all trailing white spaces */
   pb += (last-i); i = last;

   /* Remove trailing double quote */
   if ( (i>0) && ( buffer[i-1] == '"' ) ) { pb--; i--; } 

   /* String end marker */
   *pb = '\0'; pb++; i++;

   return(buffer);

} /* val2str */

/*---------------------------------------------------------------------------
NAME

    str2val - transformation of a string to a value string

SYNOPSIS

    char *str2val( char buffer[], unsigned long buflen, const char * string,
                   const char ** pnext );

DESCRIPTION
Transformation of a string to a value string which can be used in
a key/value statement. The transformed string is written to buffer, which
has the size buflen. The input string is converted until the buffer is filled.
A pointer to the unconverted part of the input string is returned in *pnext.
The syntax of the resulting string is in all cases correct and it can
always be used in a key/value statement. The returned pointer
does not necessarily point to the beginning of buffer.

To write the rest of the string to a subsequent value pnext can be used as
input string in a subsequent call of str2val.

The following transformations are done:

- Encapsulation of the buffer value between double quotes if the first or the
  last character of the buffer is a white space.
- Transformation to escape sequences:

'\r''\n'  => '\''l'
'\n'      => '\''l'
'{'       => '\''('
'}'       => '\'')'
';'       => '\'':'
'\'       => '\''\'

To reduce the length of a line '\r''\n' is added after a linefeed escape
symbol '\''l'.

ARGUMENTS
char buffer[]        calculation buffer, needs at least 3 elements
unsigned long buflen length of buffer
const char * string  input string
const char **pnext   pointer to the uncoverted rest of string,
                     unused if NULL

RETURN VALUES
pointer to output value (points to a position inside buffer, not necessarily
                         to its start)

HISTORY
1998-01-25 Peter Boesecke
2005-02-20 PB pnext added
---------------------------------------------------------------------------*/
char *str2val( char buffer[], unsigned long buflen, const char * string,
               const char ** pnext )
{  unsigned long i;
   char *pb = buffer;
   char *value;
   const char *ps = string;

   /* Read string until end of string or until buffer full */
   i = 0U;
   *pb = '"'; pb++; i++; // first character is reserved for double quote
   *pb = '\0';     // second character is initialized with terminating zero
   if (!string) return(pb); // return empty string if input pointer is NULL 
   while ( (i<buflen-2) && (*ps) ) {
     if ((*ps) == '\r') {
       *pb='\\'; pb++; *pb='l'; pb++; i+=2; ps++; if ((*ps) == '\n') ps++;
       if (i<buflen-4) { *pb='\r'; pb++; *pb='\n'; pb++; i+=2; } }
       else if ((*ps) == '\n') { *pb='\\'; pb++; *pb='l'; pb++; i+=2; ps++;
       if (i<buflen-4) { *pb='\r'; pb++; *pb='\n'; pb++; i+=2; } }
       else if ((*ps) == '{') { *pb='\\'; pb++; *pb='('; pb++; i+=2; ps++; }
       else if ((*ps) == '}') { *pb='\\'; pb++; *pb=')'; pb++; i+=2; ps++; }
       else if ((*ps) == ';') { *pb='\\'; pb++; *pb=':'; pb++; i+=2; ps++; }
       else if ((*ps) == '\\') { *pb='\\'; pb++; *pb='\\'; pb++; i+=2; ps++; }
       else { *pb = *ps; pb++; ps++; i++; }
   } /* while */

   if (pnext) *pnext = ps;

   // encapsulation
   if (is_white(buffer[1])||is_white(*(pb-1)) ||
       is_quote(buffer[1])||is_quote(*(pb-1))) {
     *pb = '"'; pb++; i++; value=buffer;
   } else value=&buffer[1];

   *pb = '\0'; pb++; i++;

   return(value);

} /* str2val */

/*---------------------------------------------------------------------------
NAME
       strnpad - copy a string to memory and pad

SYNOPSIS

       char *strnpad(char *dest, const char *src, size_t n, int pad);

DESCRIPTION

  Copies not more than n characters from src to dest. The copying 
  stops after n characters. In the case where the length of src is 
  less than n, the remainder of dest will be padded with the 
  byte 'pad'. Thus, if there is no null byte among the first n bytes 
  of src, the result will not be terminated by the byte 'pad'.

  If 'pad' is the null byte strnpad works exactly like strncpy.
 
RETURN VALUE
  The  strnpad() function returns a pointer to the destination 
  string dest.

AUTHOR
  Peter Boesecke 11-Jan-1998

  --------------------------------------------------------------------------*/
char *strnpad(char *dest, const char *src, size_t n, int pad)
{
  const char *ps=src;
  char *pd=dest;
  size_t i=0;

  while ((i<n)&&(*ps)) {
    *pd=*ps; pd++; ps++;
    i++;
    }

  while (i<n) {
    *pd=(char) pad; pd++;
    i++;
    }

  return(dest);

} /* strnpad */

/*---------------------------------------------------------------------------
NAME

   suppress_suffix

DESCRIPTION
If the character string 'key' ends with the character string 'suffix' only
the characters before 'suffix' are copied into 'buffer'. If 'key' does not
end with 'suffix' all characters of 'key' are copied into 'buffer'. The
pointer to 'buffer' is returned. If 'csens' is TRUE the comparison is case
sensitive. The length of 'buffer' must be sufficiently long.
---------------------------------------------------------------------------*/
char * suppress_suffix ( char buffer[], const char *key, 
                         const char * suffix, int csens )
{ const char *ps;
  char *ps_end, *ps_out;
  char sfxbuf[MaxKeyLen+1];

  /* copy strings into buffers */
  if (key   ==(char *) NULL) return((char *) NULL);
  if (buffer==(char *) NULL) return((char *) NULL); else STRCPY( buffer, key );
  if (suffix==(char *) NULL) sfxbuf[0]='\0'; else STRCPY( sfxbuf, suffix );

  /* keep case, if csens is set */
  if (!csens) { (void) strtolower(buffer); (void) strtolower(sfxbuf);}

  /* compare strings */
  ps_end = strstr(buffer,sfxbuf);
  if (!((ps_end) && ((ps_end+strlen(sfxbuf))==(buffer+strlen(buffer)))))
    ps_end = buffer+strlen(buffer);

  /* copy key to buffer */
  ps     = key;
  ps_out = buffer;

  while (ps_out<ps_end) {
     *ps_out = *ps; ps_out++; ps++;
  }
  *ps_out='\0'; ps_out++;

  return(buffer);

} /* suppress_suffix */

/*---------------------------------------------------------------------------
NAME

   is_prefix - returns 1 if key starts with prefix

SYNOPSIS

   int is_prefix ( const char *key, const char *prefix, int csens );

DESCRIPTION
If 'key' starts with 'prefix' 1 is returned, in all other cases 0.
(char *) NULL strings are handled like empty strings.
If 'prefix' is an empty string, the returned value is always 0.
If csens is 1 the comparison is case sensitive.

RETURN VALUE
if key start with prefix: 1
if key does not start with prefix or is an empty string: 0

---------------------------------------------------------------------------*/
int is_prefix ( const char *key, const char *prefix, int csens )
{ char keybuf[MaxKeyLen+1];
  char prebuf[MaxKeyLen+1];

  /* copy strings into buffers */
  if (key   ==(char*) NULL) keybuf[0]='\0'; 
    else { strncpy(keybuf,key,MaxKeyLen); keybuf[MaxKeyLen]='\0'; }
  if (prefix==(char*) NULL) prebuf[0]='\0'; 
    else { strncpy(prebuf,prefix,MaxKeyLen); prebuf[MaxKeyLen]='\0'; } 

  /* empty prefix? */
  if (strlen(prebuf)==0) return(0);

  /* convert to lower case */
  if (!csens) {
    strtolower(prebuf);
    strtolower(keybuf);
    }
  /* compare buffers */
  if (keybuf == strstr(keybuf,prebuf)) 
    return(1); else return(0);

} /* is_prefix */

/*---------------------------------------------------------------------------
continuation_key
returns a continuation key according to depth, e.g.
depth   key             continuation_key
0       History-1       History-1
1       History-1       History-1~1
2       History-2       History-1~2
etc.

The return value is NULL if the buffer is too short to create the 
continuation key or, in case that depth is larger than zero, if key
contains already a continuation marker. 
---------------------------------------------------------------------------*/
const char * continuation_key( char buffer[], unsigned long buflen,
                               const char *key, int depth )
{ const char *value;
  char newkey[MaxKeyLen+1];

  if (depth<1) value = key;
  else {
    strtrm( newkey, MaxKeyLen+1, key ); 
    if (    ( (strlen(newkey)+2+(int)log10(depth)) < buflen ) 
         && ( !(strchr(newkey,Continuation)) )    ) {
      sprintf(buffer,"%s%c%u",newkey,Continuation,depth);
      value = buffer;
    } else {
      value = (const char *) NULL;
    }
  }
  return ( value );
} // continuation_key

/*---------------------------------------------------------------------------
NAME

   continuation_depth - returns the continuation depth of key

SYNOPSIS

   long continuation_depth ( const char *key );

DESCRIPTION
If 'key' terminates with the continuation marker '~' followed by a positive
integer number the integer number is returned as continuation number. If 
'key' does not contain a continuation marker 0 is returned. A valid 
continuation key is defined as

   <key> := <keybase>['~'<continuation number>]

   <continuation number> := 1|2|...|n|n+1... (positive decimal number)

(char *) NULL strings are handled like empty strings. The value 0 is 
returned. If the number after a continuation marker is invalid -1 is returned.

RETURN VALUE
continuation number
---------------------------------------------------------------------------*/
long continuation_depth ( const char *key )
{ char keybuf[MaxKeyLen+1];
  const char *ps;
  long tmp, number=0l;
  int numc=0;
  char rest;

  /* copy strings into buffers */
  if (key ==(char*) NULL) keybuf[0]='\0';
  else strtrm(keybuf,MaxKeyLen+1,key);

  /* search for first occurence of continuation marker */
  ps=strchr(keybuf, (int) Continuation );
  if (ps) {
    numc=sscanf(++ps,"%ld%c",&tmp,&rest);
    if ((numc==1)&&(tmp>0)) number=tmp;
    else number=-1l;
  }

  return( number );

} /* continuation_depth */

/*---------------------------------------------------------------------------
NAME

  compare_keys --- compares two strings (<0, 0, >0)

SYNOPSIS

  enum SMode { CaseSensitiveSort, UpperCaseSort, NumberSort };
  SMode sortmode;
  int compare_keys( const char *key1, const char *key2, int sortmode );

DESCRIPTION
The comparison is done for printable characters only, except spaces. Other
characters are suppressed. The comparison is done for a maximum of MaxKeyLen
characters (including all characters, printable and non-printable).
In sort mode NumberSort all characters are converted to uppercase, a leading
'+' or '-' sign and multiple leading '0's are removed. 
The comparison is only done for the key base until the first occurrence of a
continuation marker. The part after the continuation marker is converted to
a continuation number. If the comparison of two key bases returns 0 the key
order is determined from the continuation numbers.
NULL pointers (key1 and key2) are handled like empty strings.

GCC AND G++
For compatibility between g++ and gcc the variable declaration
"SMode sortmode" has been changed back to "int sortmode". The g++ declaration
was:
  
  int compare_keys( const char *key1, const char *key2, SMode sortmode );

It could not be compiled with gcc

RETURN VALUES
The routine returnes as result:

   input          cont. number    output
   key1 < key2       any          -1
                  num1 < num2     -1
   key1 = key2    num1 = num2      0
                  num1 > num2     +1
   key1 > key2       any          +1

In the case that a key (marked with TRUE) starts with DATA_FORMAT_PREFIX
the following values are returned:

   TRUE  = key starts with DATA_FORMAT_PREFIX
   FALSE = key does not start with DATA_FORMAT_PREFIX

   key1   key2   result 
   FALSE  FALSE  normal comparison
   FALSE  TRUE   +1
   TRUE   TRUE   normal comparison
   TRUE   FALSE  -1

HISTORY
Peter Boesecke
---------------------------------------------------------------------------*/
int compare_keys( const char * key1, const char * key2, int sortmode )
{ register int i,j;
  char buf1[MaxKeyLen+1], buf2[MaxKeyLen+1];
  char *pc1, *pc1o, *pc2, *pc2o;
  const char *prefix = DATA_FORMAT_PREFIX;
  long num1, num2;
  char stop1='\0', stop2='\0';
  int l1,l2; 
  int vz1=1, vz2=1;
  int p1, p2;
  int comparison;
  
  /* check prefix */
  p1 = is_prefix ( key1, prefix, False );
  p2 = is_prefix ( key2, prefix, False );
  if ((!p1) && p2) return(+1); /* key2 has prefix */
  if (p1 && (!p2)) return(-1); /* key1 has prefix */
  
  num1 = continuation_depth ( key1 );
  if (num1>0) stop1=Continuation; // don't stop if depth invalid
  num2 = continuation_depth ( key2 );
  if (num2>0) stop2=Continuation; // don't stop if depth invalid

  /* compare key bases before first occurence of a stop marker */
  pc1 = &buf1[0]; pc2 = &buf2[0];
  
  switch ( sortmode ) {
    case CaseSensitiveSort :
      if (key1) for (i=0;(i<MaxKeyLen)&&(*key1)&&(*key1!=stop1);i++) {
        if (isgraph(*key1)) *pc1++=*key1++; else key1++;
        }
      *pc1='\0';

      if (key2) for (i=0;(i<MaxKeyLen)&&(*key2)&&(*key2!=stop2);i++) {
        if (isgraph(*key2)) *pc2++=*key2++; else key2++;
        }
      *pc2='\0';

      break;
    case UpperCaseSort :
      if (key1) for (i=0;(i<MaxKeyLen)&&(*key1)&&(*key1!=stop1);i++) {
        if (isgraph(*key1)) *pc1++=toupper(*key1++); else key1++;
        }
      *pc1='\0';

      if (key2) for (i=0;(i<MaxKeyLen)&&(*key2)&&(*key2!=stop2);i++) {
        if (isgraph(*key2)) *pc2++=toupper(*key2++); else key2++;
        }
      *pc2='\0';

      break;
    case NumberSort :
      if (key1) for (i=0;(i<MaxKeyLen)&&(*key1)&&(*key1!=stop1);i++) {
        if (isgraph(*key1)) *pc1++=toupper(*key1++); else key1++;
        }
      *pc1='\0';

      if (key2) for (i=0;(i<MaxKeyLen)&&(*key2)&&(*key2!=stop2);i++) {
        if (isgraph(*key2)) *pc2++=toupper(*key2++); else key2++;
        }
      *pc2='\0';

      /* remove a leading '+' or a leading '-' and
         remove afterwards multiple leading '0's */
      pc1o = pc1 = &buf1[0];
      if (*pc1=='+') pc1++; else if (*pc1=='-') {vz1=-1;pc1++;}

      pc2o = pc2 = &buf2[0];
      if (*pc2=='+') pc2++; else if (*pc2=='-') {vz2=-1;pc2++;}

      if (vz1<vz2) return(-1); else if (vz2<vz1) return(1);

      while (*pc1 == '0') pc1++; if (*pc1 == '\0') *pc1o++='0';
      while (*pc1) *pc1o++ = *pc1++; *pc1o='\0';

      while (*pc2 == '0') pc2++; if (*pc2 == '\0') *pc2o++='0';
      while (*pc2) *pc2o++ = *pc2++; *pc2o='\0';

      /* pad strings with leading '0's to same lengths */
      l1=STRLEN(buf1); l2=STRLEN(buf2);
      if (l1!=l2) {
        if ( l1<l2) {
          for (i=l2,j=l1;j>=0;i--,j--) buf1[i]=buf1[j];
          for (;i>=0;i--)              buf1[i]='0';
        } else {
          for (i=l1,j=l2;j>=0;i--,j--) buf2[i]=buf2[j];
          for (i=i;i>=0;i--)           buf2[i]='0';
        }
      }

      break;

    default : printf("ERROR: Unknown sortmode\n"); exit(-1); break;
  }

  comparison = vz1*STRCMP(buf1,buf2);

  /* continuation keys are placed after their base keys in the order
     of the continuation depth, keys with the same base keys but
     invalid depth are placed after all continuation keys */
  if (comparison==0) {
    if ((num1>=0)&&(num2>=0)) { // no change if both are equal
      if (num1<num2) comparison = -1;
      else if (num1>num2) comparison = 1;
    } else { // no change if both are negative 
      if ((num1<0)&&(num2>=0)) comparison = 1;
      else if ((num1>=0)&&(num2<0)) comparison = -1;
    }
  }

  return( comparison );

} /* compare_keys */

/*--------------------------------------------------------------------------
NAME

   str2version --- convert string to version

SYNOPSIS

   DFVersion str2version( const char * string )

DESCRIPTION
The version string must have the following format: Vmajor.minor. It must 
not contain white spaces.  V is a non-numeric character string. It is 
ignored and can be omitted. major and minor are numeric character strings 
of type "unsigned short".  major and minor are separated by '.'. If '.' 
or minor is missing the value of minor is set to 0. 
In case of an error the program is terminated.

RETURN VALUE
DFVersion version, the converted version
---------------------------------------------------------------------------*/
DFVersion str2version( const char * string )
{ const char * ConversionError = "SEVERE ERROR converting version string ";
  const char * vs = string;
  DFVersion version;
  unsigned long int major = 0u, minor = 0u;
  int npar;

  while ( (*vs) && (!(is_number(*vs))) ) vs++;
  npar = sscanf(vs, "%lu.%lu",&major, &minor);
  if ( npar < 1 ) {
    fprintf(stderr,"%s\"%s\"\n", ConversionError, string);
    exit(-1); 
    } 
    else if ( npar < 2 ) minor = 0u;

  version.Major = (unsigned short int) major;
  version.Minor = (unsigned short int) minor;

  return(version);

} /* str2version */

/*--------------------------------------------------------------------------
NAME

   version2str --- write version to string

SYNOPSIS

   char * version2str( char buffer[], unsigned long buflen, DFVersion version );

DESCRIPTION
The created version string has the following format: 

  'version.Major'.'version.Minor' . 

In case of an error -1 is returned.

RETURN VALUE
pointer to string 

---------------------------------------------------------------------------*/
char * version2str( char buffer[], unsigned long buflen, DFVersion version )
{ const unsigned long outlen = MaxValLen+1;
  char out[MaxValLen+1], *pi, *pd;
  unsigned long length;
  
  sprintf(out, "%u.%2u", version.Major, version.Minor);
  length = (outlen<buflen)?(outlen-1):(buflen-1);

  memset(buffer,'0',length);
  pd = &buffer[0]; pi = &out[0];
  while (*pi) { if (*pi!=' ') *pd=*pi; pi++; pd++; }
  *pd = '\0';
 
  return( &buffer[0] );
 
} /* version2str */

/*--------------------------------------------------------------------------
NAME

   compare_versions --- compare versions (v1<v2:-1,v1==v2:0,v1>v2:1)

SYNOPSIS

   typedef struct Data_Format_Version { 
     unsigned short Major, Minor; 
     } DFVersion;
   int compare_versions( DFVersion version1, DFVersion version2 );

DESCRIPTION

The comparison is done in the following way:

major1 = version1.Major, etc.

major1<major2                 -1
major1=major2, minor1<minor2  -1
major1=major2, minor1=minor2   0
major1=major2, minor1>minor2   1
major1>major2                  1

RETURN VALUE
-1, 0, 1 depending on the comparison
---------------------------------------------------------------------------*/
int compare_versions( DFVersion version1, DFVersion version2 )
{ int result;

  if ( version1.Major<version2.Major ) result = -1;
    else if ( version1.Major>version2.Major ) result = +1;
    else if ( version1.Minor<version2.Minor ) result = -1;
    else if ( version1.Minor>version2.Minor ) result = +1;
    else result = 0;

  return(result);

} /* compare_versions */

/*===string_access END==================================================---*/
/*===data_structure BEGIN==================================================*/

/*---------------------------------------------------------------------------
NAME
   init_data_file_flags --- initialize data file flags
SYNOPSIS
   init init_data_file_flags( DFFlags * Flags )
RETURN VALUE
   success: 0
   error:   not 0
---------------------------------------------------------------------------*/
int init_data_file_flags( DFFlags * Flags )
{
  Flags->ExistingFile    = (unsigned short) False;
  Flags->ReadOnlyFile    = (unsigned short) False;
  Flags->TemporaryFile   = (unsigned short) False; 
  if (write_general_block)
    Flags->NoGeneralHeader = (unsigned short) False; 
  else Flags->NoGeneralHeader = (unsigned short) True;

  return(0);

} /* init_data_file_flags */

/*---------------------------------------------------------------------------
print_data_file_flags (success:0, error:-1)
---------------------------------------------------------------------------*/
int print_data_file_flags ( FILE * out, const DFFlags * Flags )
{
   fprintf(out,"Flags\n");
   fprintf(out,"  ExistingFile       = %hu\n",Flags->ExistingFile);
   fprintf(out,"  ReadOnlyFile       = %hu\n",Flags->ReadOnlyFile);
   fprintf(out,"  TemporaryFile      = %hu\n",Flags->TemporaryFile);
   fprintf(out,"  NoGeneralHeader    = %hu\n",Flags->NoGeneralHeader);
  
   return(0);

} /* print_data_file_flags */

/*---------------------------------------------------------------------------
init_file
---------------------------------------------------------------------------*/
void init_file ( DFile * file )
{
    file->Used               = False;
    file->Name               = (char *) NULL;
    file->Channel            = (FILE *) NULL;
    file->Buffer             = (char *) NULL;
    file->Version            = str2version( DataFormatVersion );
    file->BlockBoundary      = (unsigned long) BLOCK_BOUNDARY;
    init_data_file_flags( &(file->Flags) );
    file->GeneralBlock       = (DBlock *) NULL;
    file->ActiveBlock        = (DBlock *) NULL;
    file->NextSequenceNumber = (unsigned long) FirstHeader; 
    file->LastBlockInFile    = (DBlock *) NULL;
    file->ChainList          = (DChain *) NULL;

} /* init_file */

/*---------------------------------------------------------------------------
init_file_table 
---------------------------------------------------------------------------*/
void init_file_table( DFile ftb[] )
/* Init file table, set InitTable to 1 */
{ register int i;
  for (i=0;i<MaxFiles;i++) init_file ( &ftb[i] ); 
  InitTable = 1;
} /* init_file_table */

/*---------------------------------------------------------------------------
search_free_stream (success:0, error:-1)
---------------------------------------------------------------------------*/
int search_free_stream( DFile *ftb )
/* Return the index of the first unused file table cell, return
  -1 if failed */
{ register int i;
  for (i=0;i<MaxFiles;i++) {
    if (!ftb[i].Used) return(i);
    }
  return(-1);
} /* search_free_stream */

/*---------------------------------------------------------------------------
search_stream (success:0, error:-1)
---------------------------------------------------------------------------*/
int search_stream( DFile *ftb, const char * fname )
/* Return the index of the first opened file called fname, 
   return -1 if failed */
{ register int i;
  for (i=0;i<MaxFiles;i++) {
    if (ftb[i].Used) {
      if (!strcmp(ftb[i].Name,fname)) return(i);
      }
    }
  return(-1);
} /* search_stream */

/*---------------------------------------------------------------------------
NAME
   init_symbol_element_flags --- initialize symbol element flags
SYNOPSIS
   init init_symbol_element_flags( SEFlags * Flags )
RETURN VALUE
   success: 0
   error:   not 0
---------------------------------------------------------------------------*/
int init_symbol_element_flags( SEFlags * Flags )
{
  Flags->Read              = (unsigned short) 0;
 
  return(0);
 
} /* init_symbol_element_flags */

/*---------------------------------------------------------------------------
print_symbol_element_flags (success:0, error:-1)
---------------------------------------------------------------------------*/
int print_symbol_element_flags ( FILE * out, const SEFlags * Flags )
{
   fprintf(out,"  Flags\n");
   fprintf(out,"    Read             = %hu\n",Flags->Read);
 
   return(0);
 
} /* print_symbol_element_flags */

/*---------------------------------------------------------------------------
NAME

   insert_symbol -- insert/update 'key' in symbol-list of 'block'

SYNOPSIS

   int insert_symbol( DBlock * block, const char * Key,
                      const char * Value, SElement ** symbol );

DESCRIPTION
If 'Key' already exists (success), its 'Value' is updated and the 'symbol'
pointer is returned in *symbol if symbol is not the NULL pointer. Data 
format keys are inserted before all others. 'String' is not updated.

RETURN VALUE
  'Key' successfully updated, symbol pointer to symbol returned
        in *symbol, return value 0
  'Key' updated failed: -1, *symbol set to NULL pointer
---------------------------------------------------------------------------*/
int insert_symbol( DBlock * block, const char * Key,
                   const char * Value, SElement ** symbol )
{
  SElement * newsymbol, * next, * previous; 
  char * tmp;
  int notfound = -1;

  /* initialize *symbol */
  if (symbol) *symbol = (SElement *) NULL;

  /* check Key */
  if ( Key == (const char *) NULL ) return(-1); // Key required

  /* check Value */
  if ( Value == (const char *) NULL ) return(-1); // Value required

  /* check block */
  if ( block == (DBlock *) NULL ) return(-1); // block cannot be NULL

  /* warning, if Key too long */
  if ( strlen(Key)>MaxKeyLen ) {
//    fprintf(stderr,"\nWARNING: The length %zu of %10s... exceeds %d\n",
    fprintf(stderr,"\nWARNING: The length %zu | %lu of %10s... exceeds %d\n",
             strlen(Key), strlen(Key), Key, MaxKeyLen ); }

  previous = (SElement *) NULL;
  next = block->SymbolList;

  /* search insertion point (insertion before next) */
  while( ( next!=(SElement *) NULL ) && (notfound<0) ) {
    notfound = compare_keys(next->Key,Key,UpperCaseSort);
    if (notfound<0) {previous = next; next = next->Next;}
    }

  /* create new symbol, if (notfound) */
  if ( notfound ) {
    /* create new symbol */
    if (!(newsymbol = (SElement *) malloc( sizeof(SElement) ) )) return(-1);

    newsymbol->Key         = newstrn( Key, MaxKeyLen ); 
    if (!newsymbol->Key) { free(newsymbol); return(-1); }
    newsymbol->Value       = (char *) NULL;
    newsymbol->KeyPos      = (unsigned long) 0;
    newsymbol->ValPos      = (unsigned long) 0;
    newsymbol->KeyLen      = (unsigned short) STRLEN(Key);
    newsymbol->ValLen      = (unsigned short) 0;
    newsymbol->String      = (char *) NULL;
    newsymbol->StringLen   = (unsigned short) 0;
    init_symbol_element_flags( &(newsymbol->Flags) );

    /* insert newsymbol before next */
    if (next) next->Previous = newsymbol;
    newsymbol->Next=next;
    newsymbol->Previous=previous;
    if (previous) previous->Next=newsymbol;
       else block->SymbolList = newsymbol;

    /* link to owning block */
    newsymbol->Block      = block;

    next = newsymbol;
    }

  /* update Value (Value and original next->Value can be identical!) */
  tmp  = newstr( Value ); if (!tmp) return(-1);
  if ( next->Value ) free ( next->Value );
  next->Value  = tmp;
  next->ValLen = (unsigned short) STRLEN(Value);

  if (symbol) *symbol = next;

  block->KeyOrderNextSymbol = block->SymbolList;
  block->KeyOrderNo = 0;

  return(0);

} /* insert_symbol */

/*---------------------------------------------------------------------------
NAME

   search_symbol -- search for 'Key' in symbol-list of 'block'

SYNOPSIS

   int search_symbol( DBlock * block, const char * Key, SElement ** symbol );

DESCRIPTION
Searches for 'Key' and returns a pointer to it in *symbol, if symbol is not
the NULL pointer. 'String' is not updated.

RETURN VALUE
  'Key' found, symbol pointer returned in **symbol, return value 0
  'Key' not found: -1, pointer (SElement *) NULL returned in **symbol
---------------------------------------------------------------------------*/
int search_symbol( DBlock * block, const char * Key, SElement ** symbol )
{
  SElement * current;

  /* initialize *symbol */
  if (symbol) *symbol = (SElement *) NULL;

  /* check Key */
  if ( Key == (const char *) NULL ) return(-1); // Key required 

  /* check block */
  if ( block == (DBlock *) NULL ) return(-1); // block cannot be NULL

  /* warning, if Key too long */
  if ( strlen(Key)>MaxKeyLen ) {
//    fprintf(stderr,"\nWARNING (search_symbol): The length %zu of %10s... exceeds %d\n",
    fprintf(stderr,"\nWARNING (search_symbol): The length %zu | %lu of %10s... exceeds %d\n",
             strlen(Key), strlen(Key), Key, MaxKeyLen ); }

  /* search symbol */
  current = block->SymbolList;

  if ( current!=(SElement *) NULL )
  while( ( current!=(SElement *) NULL ) &&
         ( compare_keys(current->Key,Key,UpperCaseSort)!=0 ) ) {
    current = current->Next;
    }
  if (symbol) *symbol = current;

  if (current==(SElement *) NULL) return(-1);

  return(0);

} /* search_symbol */

/*---------------------------------------------------------------------------
NAME

   delete_symbol -- deletes 'symbol' from symbol-list of 'block'

SYNOPSIS

   int delete_symbol( DBlock * block, SElement * symbol, SElement ** Next )

DESCRIPTION
The routine ends with success if the symbol 'symbol' is not any more present, 
even if it symbol is the NULL pointer.  A pointer to the element after the 
deleted element is returned (ignored if Next is the NULL pointer).

RETURN VALUE
  success:0, error:-1
---------------------------------------------------------------------------*/
int delete_symbol( DBlock * block, SElement * symbol, SElement ** Next )
{ SElement *previous, *next;

  /* initialize *Next */
  if (Next) *Next = (SElement *) NULL;

  /* check symbol */
  if (!symbol) return(0); // nothing to delete

  /* check block */
  if ( block == (DBlock *) NULL ) return(-1); // block cannot be NULL

  /* change links */
  previous = symbol->Previous;
  next = symbol->Next;

  if ( next != (SElement *) NULL ) next->Previous = previous;
  if ( previous != (SElement *) NULL ) previous->Next = next;
    else block->SymbolList = next;

  /* remove symbol */
  free(symbol->Key);
  free(symbol->Value);
  if (symbol->String) free(symbol->String);
  free(symbol);

  block->KeyOrderNextSymbol = block->SymbolList;
  block->KeyOrderNo = 0;

  if (Next) *Next = next;

  return(0);

} // delete_symbol

/*---------------------------------------------------------------------------
NAME

   remove_symbol -- removes 'key' from symbol-list of 'block'

SYNOPSIS

   int remove_symbol( DBlock * block, const char * Key, SElement ** Next );

DESCRIPTION
The routine ends with success if the symbol is not any more present, even
if it does not exist before the call of this function. A pointer to the
element after the removed element is returned (ignored if Next is the NULL
pointer).

RETURN VALUE
  success:0, error:-1
---------------------------------------------------------------------------*/
int remove_symbol( DBlock * block, const char * Key, SElement ** Next )
{
  SElement * current;

  /* initialize *Next */
  if (Next) *Next = (SElement *) NULL;

  /* check Key */
  if ( Key == (const char *) NULL ) return(0); // nothing to remove any more

  /* check block */
  if ( block == (DBlock *) NULL ) return(-1); // block cannot be NULL

  /* warning, if Key too long */
  if ( strlen(Key)>MaxKeyLen ) {
//    fprintf(stderr,"\nWARNING: The length %zu of %10s... exceeds %d\n",
    fprintf(stderr,"\nWARNING: The length %zu | %lu of %10s... exceeds %d\n",
             strlen(Key), strlen(Key), Key, MaxKeyLen ); }

  /* search symbol */
  current = block->SymbolList;

  if ( current!=(SElement *) NULL )
    while( ( current!=(SElement *) NULL ) &&
           ( compare_keys(current->Key,Key,UpperCaseSort)!=0 ) ) {
      current = current->Next;
      }

  /* stop, if no symbol found */
  if (current==(SElement *) NULL) return(0);

  /* remove current */
  if (delete_symbol( block, current, Next )) return(-1);

  return(0);

} /* remove_symbol */

/*---------------------------------------------------------------------------
insert_symbol_root (success:0, error:-1)
The symbol is inserted directly at the root of the list. A symbol with
the same name is removed before inserting.
---------------------------------------------------------------------------*/
int insert_symbol_root( DBlock * block, const char * Key,
                        const char * Value, SElement ** symbol )
{
  SElement * newsymbol, * next, * previous; 
  char * tmpKey, * tmpValue;

  /* warning, if Key too long */
  if ( strlen(Key)>MaxKeyLen ) {
//    fprintf(stderr,"\nWARNING: The length %zu of %10s... exceeds %d\n",
    fprintf(stderr,"\nWARNING: The length %zu | %lu of %10s... exceeds %d\n",
             strlen(Key), strlen(Key), Key, MaxKeyLen ); }

  if ( block == (DBlock *) NULL) return(-1);

  /* allocate memory and copy Key and Value to new variables 
     (Value/Key and original next->Value/Key can be identical!) */
  tmpKey         = newstrn( Key, MaxKeyLen ); if (!tmpKey) return(-1);
  tmpValue       = newstr( Value ); if (!tmpValue) {free(tmpKey);return(-1);}

  /* remove old symbol */
  if ( remove_symbol(block, Key, NULL) ) {free(tmpKey);free(tmpValue);return(-1);}

  *symbol = (SElement *) NULL;
  previous = (SElement *) NULL;
  next = block->SymbolList;

  /* create new symbol */
  if (!(newsymbol = (SElement *) malloc( sizeof(SElement) ) )) {
    free(tmpKey);free(tmpValue);return(-1);
  }

  newsymbol->Key         = tmpKey;
  newsymbol->Value       = tmpValue;
  newsymbol->KeyPos      = (unsigned long) 0;
  newsymbol->ValPos      = (unsigned long) 0;
  newsymbol->KeyLen      = (unsigned short) STRLEN(tmpKey);
  newsymbol->ValLen      = (unsigned short) STRLEN(tmpValue);
  newsymbol->String      = (char *) NULL;
  newsymbol->StringLen   = (unsigned short) 0;
  init_symbol_element_flags( &(newsymbol->Flags) );

  /* insert newsymbol before next */
  if (next) next->Previous = newsymbol;
  newsymbol->Next=next;
  newsymbol->Previous=previous;
  if (previous) previous->Next=newsymbol;
    else block->SymbolList=newsymbol;

  /* link to owning block */
  newsymbol->Block      = block;

  *symbol = newsymbol;

  block->KeyOrderNextSymbol = block->SymbolList;
  block->KeyOrderNo = 0;

  return(0);

} /* insert_symbol_root */

/*---------------------------------------------------------------------------
free_symbol_list (success:0, error:-1)
---------------------------------------------------------------------------*/
int free_symbol_list( DBlock * block )
{
  SElement * symbol, * next;

  next = block->SymbolList;
  block->SymbolList = (SElement *) NULL;

  while(next!=(SElement*) NULL) {
    symbol = next;
    next=next->Next;
    free(symbol->Value);
    free(symbol->Key);
    if (symbol->String) free(symbol->String);
    free(symbol);
    }

  block->KeyOrderNextSymbol = block->SymbolList;
  block->KeyOrderNo = 0;

  return(0);

} /* free_symbol_list */

/*---------------------------------------------------------------------------
netto_header_length (success:0, error:-1)
Calculation of the netto header length (without PadLen). It includes:
 length(header_begin) 
 + [ Symbol->KeyLen + length(symbol_separator)  
     + Symbol->ValLen + length(symbol_terminator) ] (for symbols)
 + length(header_end)
---------------------------------------------------------------------------*/
unsigned long netto_header_length( DBlock * block )
{ const unsigned long h_begin=(unsigned long) strlen(header_begin);
  const unsigned long s_syms =(unsigned long) (strlen(symbol_separator) +
                         strlen(symbol_terminator));
  const unsigned long h_end  =(unsigned long) strlen(header_end);
  unsigned long h_len; 
  SElement * symbol;

  symbol = block->SymbolList;

  h_len  = h_begin;
  while (symbol!=(SElement*) NULL) {
    h_len += (unsigned long) symbol->KeyLen + (unsigned long) symbol->ValLen + s_syms;
    symbol=symbol->Next;
    }

  h_len += h_end;

  return(h_len);

} /* netto_header_length */

/*---------------------------------------------------------------------------
print_symbol_list (success:0, error:-1)
---------------------------------------------------------------------------*/
int print_symbol_list( FILE * out, DBlock * block, int level, int verbose )
{ const char * SeparationLine =
    "-       -       -       -       -       -       -       -";

  SElement * symbol;

  if (level<1) return(0);

  symbol = block->SymbolList;

  while (symbol!=(SElement*) NULL) {
    if (verbose) {
      fprintf(out,"   %s\n",SeparationLine);
      fprintf(out,"   Key               = %s\n",symbol->Key);
      fprintf(out,"   Value             = %s\n",symbol->Value);
      fprintf(out,"   KeyPos            = %lu\n",symbol->KeyPos);
      fprintf(out,"   ValPos            = %lu\n",symbol->ValPos);
      fprintf(out,"   KeyLen            = %u\n",symbol->KeyLen);
      fprintf(out,"   ValLen            = %u\n",symbol->ValLen);
      fprintf(out,"   String            = %s\n",
        (symbol->String)?(symbol->String):"(not set)");
      fprintf(out,"   StringLen         = %u\n",symbol->StringLen);
      print_symbol_element_flags ( out, &(symbol->Flags) );
      fprintf(out,"   Previous Key      = ");
      if ((symbol->Previous)!=(SElement*) NULL)
        fprintf(out,"%s\n", symbol->Previous->Key);
        else fprintf(out,"(no previous symbol)\n");
      fprintf(out,"   Next Key          = ");
      if ((symbol->Next)!=(SElement*) NULL)
        fprintf(out,"%s\n", symbol->Next->Key);
        else fprintf(out,"(no next symbol)\n");
      fprintf(out,"   Owner Block       = %s\n",symbol->Block->BlockKey);
      } else {
      fprintf(out,"   '%s' = '%s'\n",symbol->Key,symbol->Value);
      }
    symbol=symbol->Next;
    }

  if (verbose) fprintf(out,"   %s\n",SeparationLine);

  return(0);

} /* print_symbol_list */

/*---------------------------------------------------------------------------
NAME
   init_data_block_flags --- initialize data block flags
SYNOPSIS
   init init_data_block_flags( DBFlags * Flags )
RETURN VALUE
   success: 0
   error:   not 0 
---------------------------------------------------------------------------*/
int init_data_block_flags( DBFlags * Flags )
{
  Flags->InternalHeader    = (unsigned short) 0;
  Flags->InternalData      = (unsigned short) 0;
  Flags->DiskBlockUsed     = (unsigned short) 0;
  Flags->DiskBlockFixed    = (unsigned short) 0;
  Flags->HeaderExternal    = (unsigned short) 0;
  Flags->HeaderChanged     = (unsigned short) 0;
  Flags->DataExternal      = (unsigned short) True;
  Flags->DataChanged       = (unsigned short) 0;
  Flags->ExternalDimAlloc  = (unsigned short) 0;
  Flags->ExternalDataAlloc = (unsigned short) 0;  
  Flags->BadBlock          = (unsigned short) 0;

  return(0);

} /* init_data_block_flags */

/*---------------------------------------------------------------------------
print_data_block_flags (success:0, error:-1)
---------------------------------------------------------------------------*/
int print_data_block_flags ( FILE * out, const DBFlags * Flags )
{
   fprintf(out,"  Flags\n");
   fprintf(out,"    InternalHeader    = %hu\n",Flags->InternalHeader);
   fprintf(out,"    InternalData      = %hu\n",Flags->InternalData);
   fprintf(out,"    DiskBlockUsed     = %hu\n",Flags->DiskBlockUsed);
   fprintf(out,"    DiskBlockFixed    = %hu\n",Flags->DiskBlockFixed);
   fprintf(out,"    HeaderExternal    = %hu\n",Flags->HeaderExternal);
   fprintf(out,"    HeaderChanged     = %hu\n",Flags->HeaderChanged);
   fprintf(out,"    DataExternal      = %hu\n",Flags->DataExternal);
   fprintf(out,"    DataChanged       = %hu\n",Flags->DataChanged);
   fprintf(out,"    ExternalDimAlloc  = %hu\n",Flags->ExternalDimAlloc);
   fprintf(out,"    ExternalDataAlloc = %hu\n",Flags->ExternalDataAlloc);
   fprintf(out,"    BadBlock          = %hu\n",Flags->BadBlock);

   return(0);

} /* print_data_block_flags */

/*---------------------------------------------------------------------------
insert_data_block (success:0, error:-1)
If 'BlockKey' already exists (success), only the 'block' pointer is returned.
---------------------------------------------------------------------------*/
int insert_data_block( DChain * chain, const char * BlockKey, DBlock ** block )
{
  DBlock * newblock, * next, * previous;
  int notfound = -1;

  /* warning, if BlockKey too long */
  if ( strlen(BlockKey)>MaxKeyLen ) {
//    fprintf(stderr,"\nWARNING: The length %zu of %10s... exceeds %d\n",
    fprintf(stderr,"\nWARNING: The length %zu | %lu of %10s... exceeds %d\n",
             strlen(BlockKey), strlen(BlockKey), BlockKey, MaxKeyLen ); }

  if ( chain == (DChain *) NULL ) return(-1);

  *block = (DBlock *) NULL;
  previous = (DBlock *) NULL;
  next = chain->BlockList;

  /* search insertion point (insertion before *pnext) */
  while( ( next!=(DBlock *) NULL ) && (notfound<0) ) {
    notfound = compare_keys(next->BlockKey,BlockKey,NumberSort);
    if (notfound<0) {previous = next; next = next->Next;}
    }

  /* create new block, if (notfound) */
  if ( notfound ) {
    /* create new block */
    if (!(newblock = (DBlock *) malloc( sizeof(DBlock) ) )) return(-1);

    newblock->BlockKey         = newstrn( BlockKey, MaxKeyLen ); 
      if (!newblock->BlockKey) {free(newblock);return(-1);}
    newblock->BlockKeyLen      = (unsigned short) STRLEN(BlockKey);
    newblock->TextPos          = (unsigned long) 0;
    newblock->BinaryPos        = (unsigned long) 0;
    newblock->TextLen          = (unsigned long) 0;
    newblock->BinaryLen        = (unsigned long) 0;
    newblock->PadLen           = (unsigned long) 0;
    newblock->BinaryFileName   = (char *) NULL;
    newblock->BinaryFilePos    = (unsigned long) 0;
    newblock->BinaryFileLen    = (unsigned long) 0;
    newblock->Data             = (void *) NULL;
    newblock->DataBufferLen    = (size_t) 0;
    newblock->DataLen          = (size_t) 0;
    newblock->DataType         = (long)   0;
    newblock->DataByteOrder    = (long)   0;
    newblock->DataRasterConfiguration = (long) 0;
    newblock->DataCompression  = (long)   0;
    newblock->DataValueOffset  = (long)   0;
    newblock->DataDim          = (long *) NULL;

    if ( init_data_block_flags( &(newblock->Flags) ) ) return(-1);
    
    newblock->SequenceNumber   = (unsigned long) 0;
    newblock->SymbolList       = (SElement *) NULL;
    newblock->KeyOrderNo       = 0;
    newblock->KeyOrderNextSymbol = (SElement *) NULL;

    /* insert newblock before *pnext */
    if (next) next->Previous = newblock;
    newblock->Next=next;
    newblock->Previous=previous;
    if (previous) previous->Next=newblock;
      else chain->BlockList=newblock;

    /* link to owning chain */
    newblock->Chain      = chain;

    next = newblock;
    }

  *block = next;

  return(0);

} /* insert_data_block */

/*---------------------------------------------------------------------------
search_data_block (success:0, error:-1)
---------------------------------------------------------------------------*/
int search_data_block( DChain * chain, const char * BlockKey, DBlock ** block )
{
  DBlock * current;

  /* warning, if BlockKey too long */
  if ( strlen(BlockKey)>MaxKeyLen ) {
//    fprintf(stderr,"\nWARNING: The length %zu of %10s... exceeds %d\n",
    fprintf(stderr,"\nWARNING: The length %zu | %lu of %10s... exceeds %d\n",
             strlen(BlockKey), strlen(BlockKey), BlockKey, MaxKeyLen ); }

  /* search block */
  current = chain->BlockList;
  if (current!=(DBlock *) NULL )
  while( ( current!=(DBlock *) NULL ) &&
         ( compare_keys(current->BlockKey,BlockKey,NumberSort)!=0 ) ) {
    current = current->Next;
    }
  *block = current;

  if (current==(DBlock *) NULL) return(-1);

  return(0);

} /* search_data_block */

/*---------------------------------------------------------------------------
free_data_block_list (success:0, error:-1)
---------------------------------------------------------------------------*/
int free_data_block_list( DChain * chain )
{
  DBlock * block, * next;

  next = chain->BlockList;
  chain->BlockList = (DBlock *) NULL;

  while(next!=(DBlock*) NULL) {
    block = next;
    next=next->Next;
    if ( free_symbol_list( block ) ) return(-1);
    if ( block->BinaryFileName ) free( block->BinaryFileName );
    if ( block->BlockKey ) free( block->BlockKey );
    if ( ( block->DataDim ) && ( !block->Flags.ExternalDimAlloc ) )
      free( block->DataDim ); 
    if ( ( block->Data ) && ( !block->Flags.ExternalDataAlloc ) ) 
      free ( block->Data );

    free(block);
    }
  return(0);

} /* free_data_block_list */

/*---------------------------------------------------------------------------
print_data_block_list (success:0, error:-1)
---------------------------------------------------------------------------*/
int print_data_block_list( FILE * out, DChain * chain, int level, int verbose )
{ const char * SeparationLine =
    "-   -   -   -   -   -   -   -   -   -   -   -   -   -   -";

  long i_dim;

  DBlock * block;

  if (level<1) return(0);

  block = chain->BlockList;

  while(block!=(DBlock*) NULL) {
    if (verbose) {
      fprintf(out,"  %s\n",SeparationLine);
      fprintf(out,"  BlockKey                = %s\n",block->BlockKey);
      fprintf(out,"  BlockKeyLen             = %u\n",block->BlockKeyLen);
      fprintf(out,"  TextPos                 = %lu\n",block->TextPos);
      fprintf(out,"  BinaryPos               = %lu\n",block->BinaryPos);
      fprintf(out,"  TextLen                 = %lu\n",block->TextLen);
      fprintf(out,"  BinaryLen               = %lu\n",block->BinaryLen);
      fprintf(out,"  PadLen                  = %lu\n",block->PadLen);
      if ( block->BinaryFileName )
        fprintf(out,"  BinaryFileName          = %s\n",block->BinaryFileName);
      else
        fprintf(out,"  BinaryFileName          = %s\n","(this file)");
      fprintf(out,"  BinaryFilePos           = %lu\n",block->BinaryFilePos);
      fprintf(out,"  BinaryFileLen           = %lu\n",block->BinaryFileLen);
      fprintf(out,"  Data                    = %p\n",block->Data);
//      fprintf(out,"  DataLen (DataBufferLen) = %zu (%zu)\n",
      fprintf(out,"  DataLen (DataBufferLen) = %zu | %lu (%lu)\n",
                          block->DataLen,block->DataLen,block->DataBufferLen);
      fprintf(out,"  DataType                = %ld\n",block->DataType);
      fprintf(out,"  DataByteOrder           = %s\n",
                                ByteOrder2String(block->DataByteOrder));
      fprintf(out,"  DataRasterConfiguration = %ld\n",
                                        block->DataRasterConfiguration);
      fprintf(out,"  DataCompression         = %s\n",
                            Compression2String(block->DataCompression));
      fprintf(out,"  DataValueOffset         = %ld\n",block->DataValueOffset);
      fprintf(out,"  DataDim                 = %p\n",block->DataDim);
      if (block->DataDim) for (i_dim=0;i_dim<=block->DataDim[0];i_dim++) 
        fprintf(out,"    DataDim[%1lu]            = %ld\n",
                                      i_dim,(block->DataDim)[i_dim]);

      print_data_block_flags( out, &(block->Flags) );
      fprintf(out,"  SequenceNumber          = %lu\n",block->SequenceNumber);
      print_symbol_list( out, block, level-1, verbose );
      fprintf(out,"   KeyOrderNo        = %d\n",block->KeyOrderNo); 
      if (block->KeyOrderNextSymbol == (SElement *) NULL ) {
        fprintf(out,"   KeyOrderNextSymbol= (no next symbol)\n");
      } else {
        fprintf(out,"   KeyOrderNextSymbol= %s\n",block->KeyOrderNextSymbol->Key);
      }
      fprintf(out,"  Previous BlockKey       = ");
      if ((block->Previous)!=(DBlock*) NULL)
        fprintf(out,"%s\n", block->Previous->BlockKey);
        else fprintf(out,"(no previous block)\n");
      fprintf(out,"  Next BlockKey           = ");
      if ((block->Next)!=(DBlock*) NULL)
        fprintf(out,"%s\n", block->Next->BlockKey);
        else fprintf(out,"(no next block)\n");
      fprintf(out,"  Owner Chain             = %s\n",block->Chain->ChainKey);
      } else {
      fprintf(out,"  BlockKey                = '%s'\n",block->BlockKey);
      print_symbol_list( out, block, level-1, verbose );
      }
    block=block->Next;
    }

  if (verbose) fprintf(out,"  %s\n",SeparationLine);

  return(0);

} /* print_data_block_list */

/*---------------------------------------------------------------------------
insert_data_chain (success:0, error:-1)
If 'ChainKey' already exists (success), only the 'chain' pointer is returned.
---------------------------------------------------------------------------*/
int insert_data_chain( DFile * file, const char * ChainKey, DChain ** chain )
{
  DChain * newchain, * next, * previous;
  int notfound = -1;

  /* warning, if ChainKey too long */
  if ( strlen(ChainKey)>MaxKeyLen ) {
//    fprintf(stderr,"\nWARNING: The length %zu of %10s... exceeds %d\n",
    fprintf(stderr,"\nWARNING: The length %zu | %lu of %10s... exceeds %d\n",
             strlen(ChainKey), strlen(ChainKey), ChainKey, MaxKeyLen ); }

  if ( file == (DFile *) NULL ) return(-1);

  *chain = (DChain *) NULL;
  previous = (DChain *) NULL;
  next = file->ChainList;

  /* search insertion point (insertion before *pnext) */
  while( ( next!=(DChain *) NULL ) && (notfound<0) ) {
    notfound = compare_keys(next->ChainKey,ChainKey,UpperCaseSort);
    if (notfound<0) { previous = next; next = next->Next; }
    }

  /* create new chain, if (notfound) */
  if ( notfound ) {
    /* create new chain */
    if (!(newchain = (DChain *) malloc( sizeof(DChain) ) )) return(-1);

    newchain->ChainKey       = newstrn( ChainKey, MaxKeyLen ); 
      if (!newchain->ChainKey) {free(newchain);return(-1);}
    newchain->ChainKeyLen    = (unsigned short) STRLEN(ChainKey);
    newchain->Flags          = (unsigned short) 0;
    newchain->BlockList      = (DBlock *) NULL;

    /* insert newchain before *pnext */
    if (next) next->Previous = newchain;
    newchain->Next=next;
    newchain->Previous=previous;
    if (previous) previous->Next = newchain;
      else file->ChainList   = newchain;

    /* link to owning file */
    newchain->File           = file;

    next = newchain;
    }

  *chain = next;

  return(0);

} /* insert_data_chain */

/*---------------------------------------------------------------------------
search_data_chain (success:0, error:-1)
---------------------------------------------------------------------------*/
int search_data_chain( DFile * file, const char * ChainKey, DChain ** chain )
{
  DChain * current;

  /* warning, if ChainKey too long */
  if ( strlen(ChainKey)>MaxKeyLen ) {
//    fprintf(stderr,"\nWARNING: The length %zu of %10s... exceeds %d\n",
    fprintf(stderr,"\nWARNING: The length %zu | %lu of %10s... exceeds %d\n",
             strlen(ChainKey), strlen(ChainKey), ChainKey, MaxKeyLen ); }

  /* search chain */
  current = file->ChainList;
  if ( current!=(DChain *) NULL )
  while( ( current!=(DChain *) NULL ) &&
         ( compare_keys(current->ChainKey,ChainKey,UpperCaseSort)!=0 ) ) {
    current = current->Next;
    }
  *chain = current;

  if (current==(DChain *) NULL) return(-1);

  return(0);

} /* search_data_chain */

/*---------------------------------------------------------------------------
NAME

   free_data_chain --- deletes a data chain and all of its contents

SYNOPSIS

   int free_data_chain( DChain * chain );

DESCRIPTION
The memory of all blocks and symbols of the data chain 'chain' is
removed. The data chain is removed from the chain list of the owning 
file. If 'chain' was the only chain in 'chain->File->ChainList' 
'chain->File->ChainList is set to (DChain *) NULL. If 'chain' is 
NULL, nothing is done and 0 (success) is returned.

RETURN VALUES
success: 0
error  : -1 
---------------------------------------------------------------------------*/
int free_data_chain( DChain * chain )
{
  DChain **proot, *previous, *next;

  if (chain!=(DChain*) NULL) { 
    proot = &(chain->File->ChainList); 
    previous = chain->Previous; 
    next = chain->Next; } 
   else return(0);

  if (next!=(DChain*) NULL) next->Previous = previous;
  if (previous!=(DChain*) NULL) previous->Next = next;
  if (*proot==chain) { *proot = ((DChain*) NULL); }

  if (free_data_block_list( chain )) return(-1);
  free(chain->ChainKey);
  free(chain);

  return(0);

} /* free_data_chain */

/*---------------------------------------------------------------------------
free_data_chain_list (success:0, error:-1)
---------------------------------------------------------------------------*/
int free_data_chain_list( DFile * file )
{
  DChain * chain, * next;

  next = file->ChainList;
  file->ChainList = (DChain *) NULL;

  while ( next!=(DChain*) NULL ) {
    chain = next;
    next=next->Next;
    if (free_data_block_list( chain )) return(-1);
    free(chain->ChainKey);
    free(chain);
    }
  return(0);

} /* free_data_chain_list */

/*---------------------------------------------------------------------------
print_data_chain_list (success:0, error:-1)
---------------------------------------------------------------------------*/
int print_data_chain_list( FILE * out, DFile * file, int level, int verbose )
{ const char * SeparationLine =
    "- - - - - - - - - - - - - - - - - - - - - - - - - - - - -";

  DChain * chain;

  if (level<1) return(0);

  chain = file->ChainList;

  while(chain!=(DChain*) NULL) {
    if (verbose) {
      fprintf(out," %s\n",SeparationLine);
      fprintf(out," ChainKey            = %s\n",chain->ChainKey);
      fprintf(out," ChainKeyLen         = %u\n",chain->ChainKeyLen);
      fprintf(out," Flags               = %#x\n", chain->Flags);
      print_data_block_list( out, chain, level-1, verbose );
      fprintf(out," Previous ChainKey   = ");
      if ((chain->Previous)!=(DChain*) NULL)
        fprintf(out,"%s\n", chain->Previous->ChainKey);
        else fprintf(out,"(no previous chain)\n");
      fprintf(out," Next ChainKey       = ");
      if ((chain->Next)!=(DChain*) NULL)
        fprintf(out,"%s\n", chain->Next->ChainKey);
        else fprintf(out,"(no next chain)\n");
//      fprintf(out," Owner file          = %#x\n",chain->File->Channel);
      fprintf(out," Owner file          = %p\n",chain->File->Channel);
      } else {
      fprintf(out," ChainKey            = '%s'\n",chain->ChainKey);
      print_data_block_list( out, chain, level-1, verbose );
      }
    chain=chain->Next;
    }

  if (verbose) fprintf(out," %s\n",SeparationLine);

  return(0);

} /* print_data_chain_list */

/*+++------------------------------------------------------------------------
NAME

   edf_print_filetable (success:0, error:-1)

SYNOPSIS

   int edf_print_filetable( FILE * out, int level, int verbose )

DESCRIPTION

   Prints the current filetable to the file 'out' 

RETURN VALUE
    0: success
   -1: failed
---------------------------------------------------------------------------*/
int edf_print_filetable( FILE * out, int level, int verbose )         /*---*/
{ const int buflen = MaxValLen+1;
  char buffer[MaxValLen+1];
  const char * SeparationLine =
    "=========================================================";

  int stream;

  if (level<1) return(0);
  if (!InitTable) return(-1);

  for (stream=0;stream<MaxFiles;stream++)
    if (FileTable[stream].Used) {
      if (verbose) {
        fprintf(out,"%s\n",SeparationLine);
        fprintf(out,"Stream               = %d\n",stream);
        fprintf(out,"Name                 = %s\n",FileTable[stream].Name);
//        fprintf(out,"Channel              = %#x\n",FileTable[stream].Channel);
        fprintf(out,"Channel              = %p\n",FileTable[stream].Channel);
        fprintf(out,"Version              = %s\n",
                        version2str(buffer,buflen,FileTable[stream].Version));
        fprintf(out,"BlockBoundary        = %lu\n",
                                             FileTable[stream].BlockBoundary);
        print_data_file_flags ( out, &(FileTable[stream].Flags) );
        fprintf(out,"GeneralBlock         = ");
        if (FileTable[stream].GeneralBlock) {
          if (FileTable[stream].GeneralBlock->BlockKey)
            fprintf(out,"%s\n",FileTable[stream].GeneralBlock->BlockKey);
            else fprintf(out,"(no block key)\n");
            } else fprintf(out,"(block not defined)\n");
        fprintf(out,"ActiveBlock          = ");
        if (FileTable[stream].ActiveBlock) {
          if (FileTable[stream].ActiveBlock->BlockKey)
            fprintf(out,"%s\n",FileTable[stream].ActiveBlock->BlockKey);
            else fprintf(out,"(no block key)\n");
            } else fprintf(out,"(block not defined)\n");
        fprintf(out,"NextSequenceNumber   = %lu\n",
                                      FileTable[stream].NextSequenceNumber);
        fprintf(out,"LastBlockInFile      = ");
        if (FileTable[stream].LastBlockInFile) {
          if (FileTable[stream].LastBlockInFile->BlockKey)
            fprintf(out,"%s\n",FileTable[stream].LastBlockInFile->BlockKey);
            else fprintf(out,"(no block key)\n");
            } else fprintf(out,"(no block written)\n");
        fprintf(out,"ChainList            = %p\n",FileTable[stream].ChainList);
        print_data_chain_list(out,&FileTable[stream],level-1,verbose);
        } else {
        fprintf(out,"Stream               = '%d'\n",stream);
        print_data_chain_list(out,&FileTable[stream],level-1,verbose);
        }
      }

  if (verbose) fprintf(out,"%s\n",SeparationLine);

  return(0);

} /* edf_print_filetable */

/*---------------------------------------------------------------------------
NAME

   delete_continuationkeys -- delete continuation keys starting at current

SYNOPSIS

   int delete_continuationkeys( DBlock * block, SElement *current );

DESCRIPTION
The routine removes any continuation key starting at current, independent
of its name and depth, until the first key with depth <1. If current is
the NULL pointer nothing is done and the return value is 0.

RETURN VALUE
  success:0, error:-1
---------------------------------------------------------------------------*/
int delete_continuationkeys( DBlock * block, SElement *current )
{
  while ( ( current ) && ( continuation_depth( current->Key ) > 0 ) )
    if ( delete_symbol( block, current, &current ) ) return(-1);

  return(0);

} // delete_continuationkeys

/*---------------------------------------------------------------------------
NAME

   update_string --- update symbol string with continuation key values

SYNOPSIS

   int update_string( SElement *base, SElement **next );


DESCRIPTION
Updates the base string with continuation key values

RETURN VALUE
   0: successful update
  -1: failure
---------------------------------------------------------------------------*/
int update_string( SElement *base, SElement **next )
{ SElement *current;

  long stringlen=0l, nxtlen;
  char *ps;
  unsigned long keybuflen = MaxKeyLen+1;
  char keybuffer[MaxKeyLen+1];
  const char *fkey;
  int depth=1;

  if (next) *next = (SElement *) NULL;

  /* Calculate required string length from accumalated value lengths
     The length of a string values is at least the length of the string. */
  depth = 0;
  current = base;
  stringlen = 0l;
  fkey = continuation_key( keybuffer, keybuflen, base->Key, depth++ );

  while ( (current) && (fkey) ) {
    if (compare_keys(current->Key,fkey,UpperCaseSort)==0) {
      stringlen += STRLEN( current->Value );
      fkey = continuation_key( keybuffer, keybuflen, base->Key, depth++ );
      current = current->Next;
    } else fkey = (char *) NULL; // stop
  }

  // allocate and update base String
  if ( (base->StringLen<stringlen)||(!(base->String)) ) {
    if (base->String) free(base->String);
    base->String = newstring( stringlen );
    base->StringLen = stringlen;
  } else {
    stringlen = base->StringLen;
  }
  (base->String)[0] = '\0';

  // copy continuation key values to base string
  depth = 0;
  current = base;
  fkey = continuation_key( keybuffer, keybuflen, base->Key, depth++ );

  // read base->string
  ps = base->String;
  while ( (current) && (fkey) ) {

    if (compare_keys(current->Key,fkey,UpperCaseSort)==0) {
      if (stringlen>0) {
        /* transform value to string and append it to base->String */
        val2str( ps, stringlen+1, current->Value );
        nxtlen=STRLEN(ps);
        ps=&ps[nxtlen];
        stringlen-=nxtlen;
      }
      fkey = continuation_key( keybuffer, keybuflen, base->Key, depth++ );
      current = current->Next; // increment only when Key==fkey
    } else fkey = (char *) NULL; // stop

  }

  if (next) *next = current;

  return(0);

} // update_string

/*---------------------------------------------------------------------------
NAME

   insert_string -- insert/update continuation 'key's in symbol-list of 'block'

SYNOPSIS

   int insert_string( DBlock * block, const char * BaseKey,
                      const char * String, SElement ** symbol );

DESCRIPTION
The value of 'BaseKey' and all its continuation keys are updated with 'String'.
'String' is copied into the string-symbol of symbol.
Unused continuation keys are removed. If 'BaseKey' does not exist
it is created. 'String' is splitted into values that are each smaller than
MaxConLen (MaxConLen+1 including terminating zero). The pointer to the
created/modified base symbol (with continuation depth 0) is returned.

RETURN VALUE
  All continuation keys of 'BaseKey' successfully inserted/updated, symbol 
  pointer to 'BaseKey' returned in **symbol, return value 0
  Insertion/update failed, *symbol set to NULL pointer, return value -1
---------------------------------------------------------------------------*/
int insert_string( DBlock * block, const char * BaseKey,
                   const char * String, SElement ** symbol )
{
  unsigned long buflen = MaxConLen+1;
  char buffer1[MaxConLen+1], buffer2[MaxConLen+1];
  unsigned long keybuflen = MaxKeyLen+1;
  char keybuffer[MaxKeyLen+1];

  SElement *current;

  const char *val, *bkval, *ps;
  const char *fkey;
  unsigned long pos1, pos2;
  int depth=0;
  size_t stringlen;

  if (symbol) *symbol = (SElement *) NULL;

  // split String
  ps = String;
  pos2 = max_line_width;
  do {
    fkey = continuation_key( keybuffer, keybuflen, BaseKey, depth );
    if (fkey) {

      pos1 = MAX(2, max_line_width - STRLEN(symbol_separator) - STRLEN(fkey));

      val = str2val( buffer1, buflen, ps, &ps );

      bkval = breakval ( buffer2, buflen, val, pos1, pos2 ); // break line

      if ( insert_symbol( block, fkey, bkval, &current ) ) return( -1 );

      // update base string with input string, allocate memory if necessary
      if (depth==0) {
        stringlen = STRLEN( String );
        if ( ( (current->StringLen) < stringlen ) || (!(current->String)) ) {
          if ( current->String ) {
            free( current->String );
          }
          current->String = newstr( String );
          current->StringLen = stringlen;
        } else {
          strcpy(current->String, String );
        }
      }

    }
    depth++;
  } while ( (*ps)&&(fkey) ); // stop if fkey could not be created

  if (symbol) *symbol = current;

  // delete unused continuation keys that follow immediately
  if ( delete_continuationkeys( block, current->Next ) ) return(-1);

  return(0);

} // insert_string

/*---------------------------------------------------------------------------
NAME

   search_string -- searches for 'BaseKey' in symbol-list of 'block'

SYNOPSIS

   int search_string( DBlock *block, const char *BaseKey, SElement **symbol );

DESCRIPTION
Searches for 'BaseKey' in symbol-list of 'block'. If it was found the 
symbol-string is created from the values of 'BaseKey' and of all its
continuation keys. The pointer to 'BaseKey' is returned in symbol if it
is not NULL.

RETURN VALUE
  'BaseKey' found, symbol pointer returned in *symbol, return value 0
  'BaseKey' not found, *symbol set to NULL pointer, return value -1
---------------------------------------------------------------------------*/
int search_string( DBlock *block, const char *BaseKey, SElement **symbol )
{ SElement *base;
  // initialize *symbol
  if (symbol) *symbol = (SElement *) NULL;

  // search BaseKey
  if (search_symbol( block, BaseKey, &base )) return(-1);

  // update *symbol
  if (symbol) *symbol = base;

  if ( update_string( base, NULL ) ) return(-1);

  return(0);

} // search_string

/*---------------------------------------------------------------------------
NAME

   remove_string -- removes continuation 'key's from symbol-list of 'block'

SYNOPSIS

   int remove_string( DBlock * block, const char * Key );

DESCRIPTION
The routine ends with success if neither 'Key' nor its continuation 'Key's
are any more present, even if no key was actually removed.

RETURN VALUE
  success:0, error:-1
---------------------------------------------------------------------------*/
int remove_string( DBlock * block, const char * Key )
{ SElement *next;

  if ( remove_symbol( block, Key, &next ) ) return(-1);

  // delete unused continuation keys that follow immediately
  if ( delete_continuationkeys( block, next ) ) return(-1);

  return(0);

} // remove_string

/*---------------------------------------------------------------------------
NAME

   search_general -- search for 'key' in 'block' and GeneralBlock

SYNOPSIS

   int search_general( DBlock * block, const char * Key, SElement ** symbol );

DESCRIPTION
   Searches for a 'key' in the symbol list of 'block'. If 'key' was not found 
   and if it is a user key, it searches afterwards in the general block. A user 
   key is a key that does not start with 'DATA_FORMAT_PREFIX'. If 'key' was 
   found the pointer to the 'symbol' element is returned.

RETURN VALUE
  'key' found, symbol pointer returned in **symbol, return value 0
  'key' not found: -1, pointer (SElement *) NULL returned in **symbol
---------------------------------------------------------------------------*/
int search_general( DBlock * block, const char * Key, SElement ** symbol )
{ int return_status;

  if ((return_status = search_string( block, Key, symbol ))) {
    if (!is_prefix(Key,DATA_FORMAT_PREFIX,UpperCaseSort)) {
      return_status = search_string( block->Chain->File->GeneralBlock,
                                     Key, symbol ); }
  }
     
  return( return_status );

} /* search_general */

/*===data_structure END=================================================---*/
/*===block_access BEGIN====================================================*/

/*---------------------------------------------------------------------------
NAME

  DBClass2String --- converts db_class to a string

SYNOPSIS

  DBClass dbclass;
  const char * DBClass2String( int db_class );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the variable
  with its enumerated type has been replaced by "int".

RETURN VALUE
  Pointer to a constant result string.

AUTHOR
18-Mar-2000 PB Specification
  -------------------------------------------------------------------------*/
const char * DBClass2String( int db_class )
{
   if ((db_class<0)||(db_class>=EndDBClass)) db_class = InValidDBClass;
   return( DBClassStrings[db_class] );

} /* DBClass2String */

/*---------------------------------------------------------------------------
NAME

  String2DBClass --- converts a string to a data block class

SYNOPSIS

  (DBClass) int String2DBClass( const char * string );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the return
  value with its enumerated type has been replaced by "int".

RETURN VALUE
   0 : error, e.g. cannot convert
  >0 : valid data block class value

AUTHOR
18-Mar-2000 PB Specification
  -------------------------------------------------------------------------*/
int String2DBClass( const char * string )
{ int  NE=True;
  long i = 0;

  while ( (NE && DBClassStrings[i]) )
    NE = compare_keys( string, DBClassStrings[i++], UpperCaseSort );

  i = MAX(0,i-1);

  if (NE) return( InValidDBClass );
    else return( i );

} /* String2DBClass */

/*---------------------------------------------------------------------------
NAME


  DBInstance2String --- converts db_instance to a string

SYNOPSIS

  DBInstance db_instance;
  const char * DBInstance2String( int db_instance );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the variable
  with its enumerated type has been replaced by "int".

RETURN VALUE
  Pointer to a constant result string.

AUTHOR
18-Mar-2000 PB Specification
  -------------------------------------------------------------------------*/
const char * DBInstance2String( int db_instance )
{
   if ((db_instance<0)||(db_instance>=EndDBInstance))
     db_instance = InValidDBInstance;
   return( DBInstanceStrings[db_instance] );

} /* DBInstance2String */

/*---------------------------------------------------------------------------
NAME

  String2DBInstance --- converts a string to a data block instance

SYNOPSIS

  (DBInstance) int String2DBInstance( const char * string );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the return
  value with its enumerated type has been replaced by "int".

RETURN VALUE
   0 : error, e.g. cannot convert
  >0 : valid data block instance value

AUTHOR
18-Mar-2000 PB Specification
  -------------------------------------------------------------------------*/
int String2DBInstance( const char * string )
{ int  NE=True;
  long i = 0;

  while ( (NE && DBInstanceStrings[i]) )
    NE = compare_keys( string, DBInstanceStrings[i++], UpperCaseSort );

  i = MAX(0,i-1);

  if (NE) return( InValidDBInstance );
    else return( i );

} /* String2DBInstance */

/*---------------------------------------------------------------------------
header_id
Example: HeaderID = EH:000001:000000:000000 ;
---------------------------------------------------------------------------*/
char * header_id( DBlock * block )
{ char KeyVal[MaxKeyLen+1]; 
  char *ps=KeyVal, *pd=IDBuffer;

  memset(IDBuffer,'0',MaxKeyLen+1);
  (void) sprintf(KeyVal,"EH:%6lu:%6u:%6u",block->SequenceNumber,0,0);

  while (*ps) {
   if (*ps!=' ') *pd=*ps; 
   ps++; pd++;
   }
  *pd = *ps;

  return(IDBuffer);

} /* header_id */

/*---------------------------------------------------------------------------
header_id_number
Returns the first header id number
Example: HeaderID = EH:000009:000000:000000 ; -> 9
---------------------------------------------------------------------------*/
long header_id_number( const char * header_id )
{ const char *ps;
  long number;

  ps=strchr(header_id, (int) ':');

  sscanf(++ps,"%ld",&number);

  return( number );

} /* header_id_number */

/*---------------------------------------------------------------------------
NAME

  block_id

SYNOPSIS

  char * block_id( DBlock * block )

DESCRIPTION

  If block exists, its block ID string is returned, otherwise NULL. 
  The block ID is written into IDBuffer and a pointer to IDBuffer is returned.

---------------------------------------------------------------------------*/
char * block_id( DBlock * block )
{
  if ( block )
    (void) sprintf(IDBuffer,"%s.%s",
       block->BlockKey,
       block->Chain->ChainKey );
   else return ( (char *) NULL );

  return(IDBuffer);
} /* block_id */

/*---------------------------------------------------------------------------
NAME

  default_chain_key

SYNOPSIS

  char *default_chain_key ( char buffer[], long memnum );

DESCRIPTION
The default chainkey for a given memory number (memnum) is written into
the buffer. The generated chainkey has the format: 

   Image.<DBInstance>.memnum, e.g. "Image.Error.3"

Positive memnum means primary data, negative memnum for variance data. Zero
memnum is not allowed. In case of abs(memnum)==1 the memnum number
is suppressed, e.g. "Image.Psd". 
Zero memnum is the general chain.

RETURN VALUE

 Pointer to buffer in case of success, otherwise NULL.
---------------------------------------------------------------------------*/
char *default_chain_key ( char buffer[], long memnum ) 
{ int db_class = DBImage;
  int db_instance;

  db_instance = InValidDBInstance;
  if (memnum<0) db_instance = DBError;
    else if (memnum>0) db_instance = DBPrimaryData;

  memnum = abs(memnum);

  if (memnum==0)
     strncpy(buffer,GENERAL_CHAIN_KEY,MaxKeyLen+1);
   else if (memnum!=1)
     sprintf(buffer,"%s.%s.%lu",
             DBClass2String( db_class ), DBInstance2String( db_instance ),
             memnum );
   else sprintf(buffer,"%s.%s",
             DBClass2String( db_class ),DBInstance2String( db_instance ));

    return ( buffer );

} /* default_chain_key */

/*---------------------------------------------------------------------------
NAME

   print_file_warning

SYNOPSIS

   void print_file_warning ( FILE * out, DFile * file, unsigned long position,
                             const char * warning );

DESCRIPTION
Prints a warning message to out.
---------------------------------------------------------------------------*/
void print_file_warning ( FILE * out, DFile * file, unsigned long position,
                          const char * warning )
{
        fprintf(out,"\n   %s FILE WARNING\n",DATA_FORMAT_NAME);
        if (warning) fprintf(out,"   %s\n",warning );
        fprintf(out,"   File name    : %s\n", file->Name); 
//        fprintf(out,"   File position: %lu (0x%lx)\n", position, position );
        fprintf(out,"   File position: %lu (%#lx)\n", position, position );
        if ( block_id(file->LastBlockInFile)) 
          fprintf(out,"   Last block ID: %s\n",block_id(file->LastBlockInFile));
         else fprintf(out,"   Last block ID: (no block read)\n");
        fprintf(out,"\n");

} /* print_file_warning */

/*---------------------------------------------------------------------------
NAME

   print_file_error

SYNOPSIS

   void print_file_error ( FILE * out, DFile * file, unsigned long position,
                           const char * error );

DESCRIPTION
Prints a error message to out.
---------------------------------------------------------------------------*/
void print_file_error ( FILE * out, DFile * file, unsigned long position,
                          const char * error )
{
        fprintf(out,"\n   %s FILE ERROR\n",DATA_FORMAT_NAME);
        if (error) fprintf(out,"   %s\n",error );
        fprintf(out,"   File name    : %s\n", file->Name);
//        fprintf(out,"   File position: %lu (0x%lx)\n", position, position );
        fprintf(out,"   File position: %lu (%#lx)\n", position, position );
        if ( block_id(file->LastBlockInFile))
          fprintf(out,"   Last block ID: %s\n",block_id(file->LastBlockInFile));
         else fprintf(out,"   Last block ID: (no block read)\n");
        fprintf(out,"\n");

} /* print_file_error */

/*---------------------------------------------------------------------------
NAME

   split_block_id --- split block_id into BlockKey and ChainKey

SYNOPSIS

  int split_block_id( char buffer[], const char * block_id,
                    char ** pBlockKey, char ** pChainKey  );
 
DESCRIPTION
'block_id' is copied into 'buffer'. 'buffer' is split into 'BlockKey' 
and 'ChainKey' by replacing the first '.' in 'buffer' with '\0'. The pointer
to the leading substring is returned in 'BlockKey', the pointer to the trailing 
substring is returned in 'ChainKey'. The minimum length of 'buffer' is
strlen(block_id)+1, the maximum length is MaxValLen+1.

RETURN VALUE
success: 0
---------------------------------------------------------------------------*/
int split_block_id( char buffer[], int buflen, const char * block_id, 
                    char ** pBlockKey, char ** pChainKey  )
{ int dot = (int) '.';
  char *pc;

  buflen = MIN((int) strlen(block_id),buflen);

  strncpy(buffer, block_id, buflen);
  buffer[buflen] = '\0';

  pc = strchr(buffer, dot);

  if (pc) { *pc='\0';pc++;*pChainKey=pc; } else { *pChainKey=&buffer[buflen]; }
  *pBlockKey = buffer;

  return(0);

} /* split_block_id */

/*---------------------------------------------------------------------------
NAME

  is_general_block ( block )

DESCRIPTION
Returns '1' if block is the general block. In all other cases '0'.
---------------------------------------------------------------------------*/
int is_general_block ( DBlock *block )
{
  if (block==block->Chain->File->GeneralBlock) return(1); else return(0);

} /* is_general_block */

/*---------------------------------------------------------------------------
block_boundary ( block )
Returns the block_boundary of the file
---------------------------------------------------------------------------*/
unsigned long block_boundary ( DBlock * block )
{  return ( block->Chain->File->BlockBoundary );
} /* block_boundary */

/*---------------------------------------------------------------------------
set_block_boundary ( block, bboundary )
Sets the block_boundary of the file
---------------------------------------------------------------------------*/
int set_block_boundary( DBlock * block, unsigned long bboundary )
{
  block->Chain->File->BlockBoundary = bboundary;

  return(0);
 
} /* set_block_boundary */

/*---------------------------------------------------------------------------
NAME

   set_data_format_version ( block, version )

SYNOPSIS

   int set_data_format_version ( DBlock * block, DFVersion version );

DESCRIPTION
Sets the datat_format_version of the file

RETURN VALUE
success: 0
error: -1
---------------------------------------------------------------------------*/
int set_data_format_version ( DBlock * block, DFVersion version )
{
  if (!(block->Chain->File->GeneralBlock->BinaryPos)) 
    block->Chain->File->Version = version;
    else return(-1); /* general block exist already on disk */
  return(0);

} /* set_data_format_version */

/*---------------------------------------------------------------------------
NAME

   set_no_general_block ( block )

SYNOPSIS

   int set_no_general_block ( DBlock * block );

DESCRIPTION
Sets the file flag NoGeneralHeader to TRUE.

RETURN VALUE
success: 0
error: -1
---------------------------------------------------------------------------*/
int set_no_general_block ( DBlock * block )
{
  block->Chain->File->Flags.NoGeneralHeader = True; 

  return(0);

} /* set_no_general_block */

/*---------------------------------------------------------------------------
int pad_spaces ( channel, padwidth, padlen ) (success:0, error:-1)
Writes exactly padlen bytes to channel. If padwith is positive,
two bytes "\r\n" are written after every padwidth number of spaces.
---------------------------------------------------------------------------*/
int pad_spaces ( FILE * channel, int padwidth, int padlen )
{ int i_pad, i_col;

  /* pad with white spaces, next line after padwidth characters */
  i_pad=0; i_col=0;
  if (padwidth>=0)
    while (i_pad<padlen-2) {
      if ( i_col<padwidth ) {
        if ( fputs ( " " , channel) < 0 ) return(-1);
        i_pad++;i_col++;
        } else {
        if ( fputs (  "\r\n" , channel) < 0 ) return(-1);
        i_pad+=2;i_col=0;
        }
      } /* while */

  while (i_pad<padlen) {
    if ( fputs ( " " , channel) < 0 ) return(-1);
    i_pad++;
    } /* while */

  return(0);

} /* pad_spaces */

/*---------------------------------------------------------------------------
NAME

   sync_data_block --- updates symbol list, calculates TextLen and BinaryLen

SYNOPSIS

   int sync_data_block ( block )

DESCRIPTION
Synchronizes the header information with the actual data. Writes the
data format specific keywords into the header. 
a) Calculates BinaryLen from DataLen
b) Updates the symbol list with the required keywords and values, for:
   the general block and for the normal data blocks. 
c) Recalculates the header length and the number of bytes that
   must be added to reach a multiple of block_boundary. The values
   are updated in block->TextLen and block->PadLen.

If ( block->Flags.DiskBlockFixed ) BinaryLen and TextLen are kept constant.
If this is not possible, an error is returned.

RETURN VALUE
Returns 0 in case of success.

HISTORY
2001-01-01 PB calculation of block->BinaryLen for DiskBlockFixed is now 
              done before keyword is written to header.
---------------------------------------------------------------------------*/
int sync_data_block ( DBlock * block )
{
  unsigned long nh_len, mh_len, dif_len=0;
  char          keybuf[MaxKeyLen+1];
  char          valbuf[MaxValLen+1];

  DChain * chain;  
  DFile *  file;  
  SElement * symbol;

  unsigned long  text_len   = block->TextLen;
  unsigned long  binary_len = block->BinaryLen;

  chain   = block->Chain;
  file    = chain->File;

  if (block == (DBlock *) NULL) return(-1);

  /* Round up binary length to the next full multiple of block_boundary */
  block->BinaryLen = 
      CEILMOD ( (unsigned long) block->DataLen, block_boundary(block));

  if ( block->Flags.DiskBlockFixed ) {
    /* keep lengths fixed */
    block->BinaryLen = MAX(block->BinaryLen,binary_len);
    if ( (block->BinaryLen) != binary_len ) return(-1);
    }

  /* write keywords */ 
  if ( is_general_block ( block ) ) {
    /* GeneralBlock */ 
    if (write_headersize) {
      /* write temporary value */
      if (insert_symbol_root(block,HEADER_SIZE_KEY, UNDETERMINED,&symbol))
        return(-1);
    }
    /* if block boundary defined in symbol list, use this value */
    if (search_symbol( block, BLOCK_BOUNDARY_KEY, &symbol ) ) {
      if (insert_symbol_root(block, BLOCK_BOUNDARY_KEY,
                u_long2s( valbuf, block_boundary(block) ),&symbol))
      return(-1); 
    } else {
      if ( set_block_boundary( block, s2u_long( symbol->Value ) ) ) {
        fprintf(stderr,"ERROR: Cannot change block boundary\n");
        return(-1);
      }
    }
 
    if (insert_symbol_root(block, DATA_BLOCKS_KEY,
                           DATA_BLOCKS_DEFAULT, &symbol))
      return(-1);
    if (insert_symbol_root(block, DATA_FORMAT_VERSION_KEY,
                           DATA_FORMAT_VERSION, &symbol))
      return(-1);

  } else {
    /* normal DataBlock */
    if (write_headersize) {
      /* write temporary value */
      if (insert_symbol_root(block,HEADER_SIZE_KEY, UNDETERMINED,&symbol))
        return(-1);
    }
    if (insert_symbol_root(block,BINARY_SIZE_KEY,
                           u_long2s( valbuf, block->BinaryLen),&symbol))
      return(-1);
    if (insert_symbol_root(block, BLOCK_ID_KEY,    block_id(block),&symbol))
      return(-1);

    /* +++ compatibility to old data format, 'Image = NN' */
    if (insert_string( block,
       suppress_suffix(keybuf, chain->ChainKey, V1_SUPPRESS, False),
       block->BlockKey, &symbol))
      return(-1);
    /* --- end old data format */
  } /* END normal DataBlock */

  /* +++ compatibility to old data format */
  if ( !(block->Flags.DiskBlockFixed) )
     block->SequenceNumber = file->NextSequenceNumber;
  if (insert_string(block, V1_HEADER_ID_KEY,    header_id(block),&symbol))
    return(-1);
  if (insert_string(block, V1_SIZE_KEY,
                           u_long2s( valbuf, block->BinaryLen),&symbol))
    return(-1);
  /* --- end old data format */

  /* Round up header length to the next full multiple of block_boundary */
  nh_len = netto_header_length( block );
  mh_len = MAX( nh_len, minimum_headersize_out );
  block->TextLen = CEILMOD ( mh_len, block_boundary(block));

  if ( block->Flags.DiskBlockFixed ) {
    /* keep lengths fixed */
    block->TextLen   = MAX(block->TextLen,text_len);
    if ( (block->TextLen)   != text_len ) return(-1);
  }

  if (write_headersize) {
    /* correct temporary header size value */
    if (search_symbol( block, HEADER_SIZE_KEY, &symbol ) ) 
      return(-1);
    dif_len = strlen(symbol->Value)-strlen(u_long2s(valbuf,block->TextLen));
    if ( insert_symbol( block, HEADER_SIZE_KEY, valbuf, &symbol ) )
      return(-1);
  }

  block->PadLen  = block->TextLen - nh_len + dif_len;

  return(0);

} /* sync_data_block */

/*---------------------------------------------------------------------------
NAME

   get_block_pos --- get start position of 'block' in file

SYNOPSIS
int get_block_pos( DBlock * block, unsigned long *ptext_pos );

DESCRIPTION
Calculates the start position of the text header section of 'block'
relative to the start of the file.

RETURN VALUE
success:0, error:-1
---------------------------------------------------------------------------*/
int get_block_pos( DBlock * block, unsigned long *ptext_pos )
{ 
  DBlock * LastBlockInFile = block->Chain->File->LastBlockInFile;

  if (block->Flags.DiskBlockFixed) *ptext_pos = block->TextPos; 
    else /* goto end of file */ 
    if (LastBlockInFile) {
      *ptext_pos = LastBlockInFile->TextPos
        + LastBlockInFile->TextLen + LastBlockInFile->BinaryLen; }
      else *ptext_pos = 0ul;

  return(0);

} /* get_block_pos */

/*---------------------------------------------------------------------------
NAME
  
  flush_data_block

SYNOPSIS

  int flush_data_block ( block );

DESCRIPTION
Writes header section and binary section, ordered, into the main EDF 
data file. Before calling this routine the data block must be synchronized 
with sync_data_block. The header section is written according to the
currently defined keyorder_KeyOrderTable (defined with edf_keyorder_set_table. 

Nothing is written if the flag block->Flags.BadBlock is set. 
The flags block->Flags.HeaderChanged and DataChanged are checked. 
The header is only written to disk if HeaderChanged or DataChanged is set. 
The header is never written to disk if InternalHeader is set.
The header symbol list is never released when InternalHeader is set.
The data is only written to disk if DataChanged is set.
The data is never written to disk if InternalData is set.
The data is never released if InternalData is set.

Data is only written to disk if the header could be written. When 
InternalHeader is set, data is never written to disk.

The following parameters are updated:
  TextPos, BinaryPos, KeyPos, ValPos, BinaryFilePos, BinaryFileLen
---------------------------------------------------------------------------*/
int flush_data_block ( DBlock *block )
{ 
  const unsigned long h_begin =(unsigned long) strlen(header_begin);
  const unsigned long s_sepa  =(unsigned long) strlen(symbol_separator);
  const unsigned long s_term  =(unsigned long) strlen(symbol_terminator);
  unsigned long cur_pos;

  DChain * chain;
  DFile *  file;
  FILE *   channel;
  SElement ** table, ** psymbol;

  chain   = block->Chain;
  file    = chain->File;
  channel = file->Channel;

  if (block)
    /* the data is only written if the header can be written */
    if ( ((block->Flags.HeaderChanged)||(block->Flags.DataChanged)) &&
          (!block->Flags.InternalHeader) ) {
      /* get block position */
      if (get_block_pos( block, &cur_pos )) {
         perror("flush_data_block->get_block_pos");return(-1); }
      block->TextPos = cur_pos;

      /* search block position */
      if ( fseek( channel, cur_pos, SEEK_SET) ) {
        perror("flush_data_block->fseek"); return(-1); }

      /* write header section */ 
      if ( fputs ( header_begin , channel ) < 0 ) {
         perror("flush_data_block->header_begin"); return(-1); }
      cur_pos += h_begin;

      /* get ordered table of symbols */
      table = keyorder_ordersymbols ( block );
      if (!table) {
         perror("flush_data_block ordersymbols"); return(-1); }

      psymbol = table;
      while (*psymbol!=(SElement*) NULL) {
        (*psymbol)->KeyPos = cur_pos;
        if ( fputs ( (*psymbol)->Key , channel ) < 0 ) {
          perror("flush_data_block->Key"); free(table); return(-1); }
        if ( fputs ( symbol_separator, channel ) < 0 ) {
          perror("flush_data_block->symbol_separator"); free(table);return(-1);}
        cur_pos += s_sepa + (unsigned long) (*psymbol)->KeyLen;
        (*psymbol)->ValPos = cur_pos;

        if ( fputs ( (*psymbol)->Value, channel ) < 0  ) {
          perror("flush_data_block->Value"); free(table); return(-1); }
        if ( fputs ( symbol_terminator, channel) < 0 ) {
          perror("flush_data_block->symbol_terminator");free(table);return(-1);}
        cur_pos += s_term + (unsigned long) (*psymbol)->ValLen;
        psymbol++;
        }

      /* free table */
      free(table);

      /* pad header block with white spaces */
      if (pad_spaces ( channel, PAD_WIDTH, block->PadLen )) {
           perror("flush_data_block->pad_spaces"); return(-1); }

      if ( fputs ( header_end , channel) < 0 ) {
           perror("flush_data_block->header_end"); return(-1); }

      block->Flags.HeaderChanged  = False;


      if ( (block->Flags.DataChanged) && (!block->Flags.InternalData) ) {

        /* Calculate BinaryPos */
        block->BinaryPos = block->TextPos + (unsigned long) block->TextLen; 

        /* stored in main EDF file, no BinaryFileName */
        if (block->BinaryFileName) free (block->BinaryFileName);
        block->BinaryFilePos = block->BinaryPos;
        block->BinaryFileLen = block->BinaryLen;

        /* write binary data section and fill rest with spaces */
        if ( fwrite(block->Data,1,block->DataLen,channel) < block->DataLen ) {
           perror("flush_data_block->fwrite"); return(-1); }
        if ( pad_spaces( channel,-1,
                         block->BinaryLen-(unsigned long)block->DataLen) ) {
           perror("flush_data_block->pad_spaces"); return(-1); }

        if ( fflush( channel ) ) {
             perror("flush_data_block->fflush"); return(-1); }

        block->Flags.DataChanged    = False;
        }

      /* data block is written, increment block number for next time */
      if (block->SequenceNumber == file->NextSequenceNumber) {
        file->LastBlockInFile=block;
        file->NextSequenceNumber++;
        }

      /* update data block flags */
      block->Flags.DiskBlockUsed  = True;
      block->Flags.DiskBlockFixed = True;
      }

  return(0);

} /* flush_data_block */

/*---------------------------------------------------------------------------
NAME

  update_symbol --- Replaces the value of 'Key' in 'block' with 'Value'

SYNOPSIS

  int update_symbol( DBlock * block, const char * Key,
                     const char * Value, SElement ** symbol )

DESCRIPTION
  The key value of the symbol 'Key' is replaced with 'Value'. The original
  value length is not changed. If 'Value' is shorter than the original key
  value, the remaining bytes are padded with spaces. If 'Value' is longer
  than the original key value (symbol->ValLen), the routine stops with an
  error. 

RETURN VALUE
success:           0, *symbol != NULL
error:            -1, *symbol == NULL
---------------------------------------------------------------------------*/
int update_symbol( DBlock * block, const char * Key,
                   const char * Value, SElement ** symbol )

{ 
  *symbol = (SElement *) NULL;

  if ( block == (DBlock *) NULL) return(-1);
  if ( Value == (const char *) NULL ) return(-1);

  if (search_symbol( block, Key, symbol )) {
    *symbol = (SElement *) NULL; return(-1); }

  /* replace key value with value */
  strnpad((*symbol)->Value, Value, (size_t) (*symbol)->ValLen, ' ');

  block->KeyOrderNextSymbol = block->SymbolList;
  block->KeyOrderNo = 0;

  return(0);

} /* update_symbol */

/*---------------------------------------------------------------------------
NAME
       rewrite_symbol - rewrite the value of a symbol in a header section 

SYNOPSIS
 
       int rewrite_symbol ( const SElement * symbol );

DESCRIPTION
  If the key value was already written to the file the value in the 
  file is replaced. If symbol->KeyPos is zero, it is assumed that the key 
  value was not already written and no output to the file is done. 

  The file pointer is either repositioned at the position of the key value
  or remains at the original position. Before continuing writing data to 
  the end of the file the file pointer must be repositioned with 
  fseek(channel, 0L, SEEK_END). 

RETURN VALUE
  In case of succes the return value is 0 otherwise -1.

---------------------------------------------------------------------------*/
int rewrite_symbol ( const SElement * symbol )
{
  FILE     * channel = symbol->Block->Chain->File->Channel;
  long     pos; 

  /* rewrite value in the file, if it was already written */
  pos = (long) symbol->ValPos;
  if ( pos > 0L ) {
    /* seek position of Value */
    if ( fseek( channel, pos, SEEK_SET) ) {
      perror("rewrite_symbol->fseek"); return(-1); }

    /* replace old value */
    if ( fputs(symbol->Value, channel) < 0 ) {
      perror("rewrite_symbol->fputs"); return(-1); }
    } 

  return(0);

} /* rewrite_symbol */

/*---------------------------------------------------------------------------
NAME
       update_general - updates the general header after writing a data block 

SYNOPSIS

       int update_general ( DBlock * block );

DESCRIPTION
  Updates the following file informations in the general block:
  Number of data blocks in the file (the general block is not counted)
  DataBlocks = NextSequenceNumber - FirstHeader;

  If GeneralBlock->Flags.InternalHeader is set, the value is only
  updated in memory but not in the file.

  If the file is not a temporary file, the file pointer is repositioned 
  at the end of the file.

RETURN VALUE
  In case of succes the return value is 0 otherwise -1.

---------------------------------------------------------------------------*/
int update_general ( DBlock * block )
{
  DBlock      * general = block->Chain->File->GeneralBlock;
  DFile       * file    = block->Chain->File;
  SElement    * symbol;
  char          KeyVal[MaxKeyLen+1];
  FILE        * channel = block->Chain->File->Channel;
  unsigned long data_blocks;

  /* calculate number of data blocks without counting general block */
  data_blocks = block->Chain->File->NextSequenceNumber-FirstHeader-1;
  if (file->Flags.NoGeneralHeader) data_blocks-= 1; /* subtract general block */

  /* rewrite 'DataBlocks' */
  if (!(u_long2s(KeyVal,data_blocks))) return(-1);

  update_symbol( general, DATA_BLOCKS_KEY, KeyVal, &symbol );
  if (!(general->Flags.InternalHeader))
    if ( symbol ) if ( rewrite_symbol ( symbol ) ) return(-1);
      
  /* seek end of file */
  if (!(file->Flags.TemporaryFile)) 
    if ( fseek( channel, 0L, SEEK_END) ) {
      perror("update_general->fseek"); return(-1); }

  block->KeyOrderNextSymbol = block->SymbolList;
  block->KeyOrderNo = 0;

  return(0);

} /* update_general */

/*---------------------------------------------------------------------------
NAME
   put_data_block (success:0, error:-1)

SYNOPSIS

   int put_data_block( DBlock * block );

DESCRIPTION

Writes header and binary data to the stream if they were changed 
(HeaderChanged, DataChanged). The format specific symbols are 
written into the header. 

'block' is only written physically to the file if the flag 
'file->Flags.NoGeneralHeader' is False.

RETURN VALUE
success: 0
otherwise: -1
---------------------------------------------------------------------------*/
int put_data_block( DBlock * block )
{
  if ( block )
    if ( !block->Flags.BadBlock )
      if ( (block->Flags.HeaderChanged)||(block->Flags.DataChanged) ) {
        /* Don't write general header if NoGeneralHeader is set */
        if ( !( block->Chain->File->Flags.NoGeneralHeader ) || 
             !( is_general_block ( block ) ) ) { 

          /* synchronize header, binary data and data file format */
          if (sync_data_block ( block )) {
             block->Flags.BadBlock = True; return(-1); }

          /* flush data block to file */
          if (flush_data_block( block )) {
             block->Flags.BadBlock = True; return(-1); }

          /* write block number into general block */
          if ( !block->Chain->File->Flags.NoGeneralHeader )
            if (update_general ( block )) {
             block->Flags.BadBlock = True; return(-1); }
        }
      }

  return(0);

} /* put_data_block */

/*---------------------------------------------------------------------------
NAME

   check_start

SYNOPSIS

   int check_start( FILE * channel );

DESCRIPTION

   Checks whether the first characters in the file contain either: 
          '\r' '\n' 'StartHeader', 
          '\n' 'StartHeader' or
          'StartHeader'.
   The file pointer is positioned at the first character after 'StartHeader'. 
   If the patterns do not match or if EOF was read the return value is -1. 
   The file cannot be processed further.
   The correct pattern should be: 
      '\n' 'StartHeader' immediately followed by '\r' '\n'.

RETURN VALUE
   Success, this is a header start: 0
   This is not a header start     : 1
   End of file                    : -1
   Any other error, e.g. char(0)  : -2
---------------------------------------------------------------------------*/
int check_start( FILE * channel )
{ char  c;
  int   ic; 

  ic = fgetc( channel ); c = (char) ic; 
  if (ic==EOF) return(-1); else if (!ic) return(-2);

  if (c=='\r') { ic = fgetc( channel ); c = (char) ic; }
  if (ic==EOF) return(-1); else if (!ic) return(-2);

  if (c=='\n') { ic = fgetc( channel ); c = (char) ic; }
  if (ic==EOF) return(-1); else if (!ic) return(-2);

  if (c!=StartHeader) return (1);

  return ( 0 );

} /* check_start */

/*---------------------------------------------------------------------------
NAME

   check_end

SYNOPSIS

   int check_end( FILE * channel );

DESCRIPTION

   Checks, whether the file pointer is positioned at the end of a header
   section. If EOF or '\0' is read the routine stops with an error 
   (return value is -1). The file cannot be processed further. If the 
   first read character is not the 'EndHeader' character the return value 
   is 1.  If one of the following characters do not match the end header 
   marker, this character is returned to channel. 

   The routine returns 1 if one of the following patterns were found: 
   
          { 'EndHeader', '\r', '\n } or
          { 'EndHeader', '\n' } 

   At least 1 character is read from channel. If it is not equal to
   EndHeader the routine stops with the return value 1. By this way the
   routine can be used to scan a header section for a header end marker.

RETURN VALUE
   In case of success
   end of header:      0, file pointer positioned after '\n'
   not end of header:  1, wrong character is returned to channel
   error:              negative number
   EOF or '\0' :      -1, end of file or char NULL
   fatal error:       -2

HISTORY
 01-Mar-1997 PB error if ungetc corrected
------------------------------------------------------------------------------*/
int check_end( FILE * channel )
{ char  c;
  int   ic;

  ic = fgetc( channel ); c = (char) ic; if ((ic==EOF)||(!ic)) return(-1);

  if (c!=EndHeader) return(1);
          else {ic=fgetc(channel);c=(char)ic; if ((ic==EOF)||(!ic)) return(-1);}
  if (c=='\r') {ic=fgetc(channel);c=(char)ic; if ((ic==EOF)||(!ic)) return(-1);}
  if (c!='\n') {if ( ic != ungetc ( ic , channel) ) return(-2); return(1);} 

  return ( 0 );

} /* check_end */

/*---------------------------------------------------------------------------
NAME

   search_end - search the end of a header section

SYNOPSIS

   int search_end( FILE * channel, 
                   unsigned long start, unsigned long block_boundary );

DESCRIPTION
If block_boundary is larger or equal to the the length of the header end
marker the full header end marker "\r\n}\n" is searched relative to 
'start' at the positions  (n=1, 2, 3, ...)

  start + n * blockboundary - strlen(header_end) 

If the block boundary is smaller than strlen(header_end) the file is 
checked byte for byte with check_end starting at 'start' for the 
EndHeader character. If no header end marker is found -1 is returned.

If the header end marker was found the file pointer is  positioned
immediately after the marker (immediately after the '\n').

RETURN VALUE
   success:      0
   no success    a negative number
   EOF or '\0'   -1
   fatal error:  -2
---------------------------------------------------------------------------*/
int search_end( FILE * channel, 
                unsigned long start, unsigned long block_boundary )
{ 
  unsigned long he_len = strlen(header_end); 
  int   ic, res;
  char  *pb, buffer[MaxKeyLen+1];
  long  pos;
  unsigned long  i;

  /* start position */
  pos = (long) start;

  buffer[0] = '\0';
  if ( block_boundary >= he_len ) {
    while ( strcmp(buffer, header_end ) ) {
      /* seek position */
      pos += (long) block_boundary ;
      if ( fseek( channel, pos - he_len, SEEK_SET) ) {
         perror("search_end->fseek"); return(-2); }

      pb = buffer; 
      for (i=0;i<he_len;i++) { 
        ic = fgetc( channel );
        *pb = (char) ic; pb++; if ((ic==EOF)||(!ic)) return(-1);
        }
      *pb = '\0'; pb++;
      } /* while */

    } else {

    if ( fseek( channel, pos, SEEK_SET) ) {
         perror("search_end->fseek"); return(-2); }

    res = 1;
    while ( res ) { 
      res = check_end( channel ); 
      if ( res < -1 ) return(-2); /* fatal error */
       else if ( res < 0 ) return(-1); /* end of file or char NULL */
      }
    }

   return ( 0 ); /* header end found */

} /* search_end */
 
/*---------------------------------------------------------------------------
NAME

   nextline

SYNOPSIS

   int nextline( FILE * channel )

DESCRIPTION

   Skip all characters until end of line or end of header. In case
   of success (not eof or '\0') the file pointer is positioned at first
   character after '\n'.

RETURN VALUE
   In case of success:  0
   end of header:       1
   in case of an error: a negative number

---------------------------------------------------------------------------*/
int nextline( FILE * channel )
{  char   c;
   int    ic;

   ic = (int) ' '; c = (char) ic;

   /* Skip everything until EOF or line end */
   while ( (ic!=EOF) && (ic) && (c!=EndHeader) && (c!='\r') && (c!='\n') ) {
     ic = fgetc( channel ); c = (char) ic;
    } /* while */

   if ((ic==EOF) || (!ic)) return (-1);

   /* Read '\n' character, if line was terminated by '\r' */
   if (c == '\r') { ic = fgetc( channel ); c = (char) ic; }

   /* unget last character if it is EndHeader and check end */
   if (c == EndHeader) {
     ungetc( (int) c, channel );
     if ( check_end( channel ) ) {
       long tmp; 
       tmp = ftell(channel);
       fprintf(stderr,"\nERROR: position %ld, end marker '%c' not followed by eol\n",
          tmp, c); return(-1); 
     } else return(1); 
   }

   return(0);

} /* nextline */

/*---------------------------------------------------------------------------
NAME

   skipcomment 

SYNOPSIS

   int skipcomment( FILE * channel )

DESCRIPTION

   Skip all characters until end of line, ignoring end of header marker. 
   In case of success (not eof or '\0') the file pointer is positioned 
   at first character after '\n'. This routine is only good for 
   skipping comment lines which start with 'Comment' character.

RETURN VALUE
   In case of success:  0
   in case of an error: a negative number

---------------------------------------------------------------------------*/
int skipcomment( FILE * channel )
{  char   c;
   int    ic;

   ic = (int) ' '; c = (char) ic;

   /* Skip everything until EOF or line end */
   while ( (ic!=EOF) && (ic) && (c!='\r') && (c!='\n') ) {
     ic = fgetc( channel ); c = (char) ic;
    } /* while */

   if ((ic==EOF) || (!ic)) return (-1);

   /* Read '\n' character, if line was terminated by '\r' */
   if (c == '\r') { ic = fgetc( channel ); c = (char) ic; }

   return(0);

} /* skipcomment */

/*---------------------------------------------------------------------------
NAME

    get_key

SYNOPSIS

    int get_key( char buffer[], unsigned long buflen, FILE * channel,
                 unsigned long * pkey_pos, unsigned long *pkey_len);

DESCRIPTION
Reads 'key' starting at the first non-white character and positions
the file pointer after 'Separator'. 

 {'white-space'} ('EndHeader' | ({'key'} ['white-space'] 
                                     ('Separator' | '\r' | '\n') ) )

Starts at the first non-white character after the current position.
'key' is returned in buffer with white spaces replaced by space. A white-space 
character immediately before 'Separator' is not returned in 'key'. If 
'Separator' was read the read pointer is positioned immediately after 
'Separator'. 

If the first non-white-character is the 'EndHeader' character the routine 
stops and returns this character to 'channel'.  If the Separator character
could not be read before {'\r', '\n'} or '\n', EOR or '\0' the routine
stops with an error and returns -1. 

'key' is returned as a string with not more than MIN(MaxKeyLen,'buflen'-1)
characters (including white spaces). The string is terminated with '\0' 
(buflen). buflen is the size of 'buffer'.

RETURN VALUES
In case of success:  0 
end of header:       1
in case of an error ('Separator' not read, EOF or '\0'): a negative number
The key is returned in buffer[].
*pkey_pos is the position of the first character of 'key' in the file.
*pkey_len is the length of key, not including '\0'.

HISTORY
21-Jan-1998 Peter Boesecke
---------------------------------------------------------------------------*/
int get_key( char buffer[], unsigned long buflen, FILE * channel,
             unsigned long * pkey_pos, unsigned long *pkey_len)
{  unsigned long i;
   char      *pb;
   char      prev_c;
   int       ic;

   /* determine minimum of MaxKeyLen+1 and buflen */
   if ((MaxKeyLen+1) < buflen) buflen = MaxKeyLen+1;
   ic = (int) ' ';

   do {
     pb = buffer; // start again
     *pb = (char) ic;

   /* skip everything before the first non-white character, skip empty lines,
      skip comments */
     while ((is_white(*pb)||(*pb==Terminator)) && (*pb!=EndHeader)) {

       prev_c  = *pb;
       ic = fgetc( channel ); *pb = (char) ic; 
       if ( (ic==EOF) || (!(ic)) ) return(-1);

       /* Skip comment line */
       if ( (prev_c=='\n') && (*pb==Comment) ) {
         if ( skipcomment( channel ) < 0 ) return(-1);
         *pb = '\n'; }
     
       } /* while */

     /* unget last character if it is EndHeader and check end */
     if (*pb == EndHeader) { ungetc( (int) *pb, channel );
       if ( check_end( channel ) ) {
         long tmp;
         tmp = ftell(channel);
         fprintf(stderr,"\nERROR: position %ld, end marker '%c' not followed by eol\n",
            tmp, *pb); return(-1); 
       } else return(1); 
     }

     *pkey_pos = (unsigned long) ftell( channel ) - 1;

     /* Read key until Separator or Terminator or until buffer full */
     i   = 0U;
     while ( (i<buflen-1) && (*pb!=Separator) && (ic!=EOF) && (ic) 
                          && (*pb!=EndHeader) && (*pb!='\r') && (*pb!='\n') 
                          && (*pb!=Terminator) ) {
       *pb  = white2space(*pb); pb++;
       ic = fgetc( channel ); *pb = (char) ic;
       i++;
       } /* while */

     /* Skip everything until Separator or Terminator */
     while ( (*pb!=Separator) && (*pb!=Terminator)
                              && (ic!=EOF) && (ic) && (*pb!=EndHeader)
                              && (*pb!='\r') && (*pb!='\n') ) {
       ic = fgetc( channel ); *pb = (char) ic;
       } /* while */

   } while ((*pb==Terminator)||(*pb=='\r')||(*pb=='\n'));

   /* stop, if separator not read */
   if ((ic==EOF) || (!ic) || (*pb != Separator)) {
     long tmp;
     tmp = ftell(channel);
     fprintf(stderr,"\nERROR: position %ld, '%c' read, separator '%c' not found\n",
       tmp, *pb, Separator); return (-1); 
   }

   /* Space character before Separator does not belong to key */
   if ( (*pb==Separator) && (i>0) && (is_white(buffer[i-1])) ) { i--; *pb--; }

   /* Replace terminating character with end of string */
   *pb = '\0';
   *pkey_len = (unsigned long) i;

   return(0);
      
} /* get_key */

/*---------------------------------------------------------------------------
NAME

   get_val

SYNOPSIS

   int get_val( char buffer[], unsigned long buflen, FILE * channel,
                unsigned long * pval_pos, unsigned long *pval_len);

DESCRIPTION
Reads 'value' starting at the first non-white character and positions the
file read pointer after 'Terminator'. The routine 'get_key' should be 
called beforehand. The first buflen-1 read characters are returned in 
buffer, all following until 'Terminator' are skipped.

 {'white-space'} ( {'value'} ['white-space']
                                     ('Terminator' | '\r' | '\n') )

Starts reading at the first non-white character after the current position,
which is normally after 'Separator'. 'value' is returned in buffer. A 
white-space character immediately before 'Terminator' is not returned 
in 'value'. If 'Terminator' was read the read pointer is positioned at the 
first character after 'Terminator'. The 'EndHeader' and '\r' or '\n'
characters are passed without change. The only terminating conditions are: 
'Terminator' read (success),  '\0' read (error) or EOF reached (error).
The routine returns -1 on error and 0 on success.

RETURN VALUES
The routine returns -1 on error (EOF, '\0', Terminator not found). 
The routine returns 0 on success.
The value is returned in buffer[].
*pval_pos is the position of the first character of 'value' in the file.
*pval_len is the length of value, not including '\0'.

HISTORY
20-Jan-1998 Peter Boesecke
---------------------------------------------------------------------------*/
int get_val( char buffer[], unsigned long buflen, FILE * channel,
             unsigned long * pval_pos, unsigned long *pval_len)
{  unsigned long i;
   char      *pb      = buffer;
   int       ic;

   /* skip everything before the first non-white character */
   i   = 0U;
   ic = (int) ' '; *pb = (char) ic;
   while ( is_white(*pb) && (ic!=EOF) && (ic) ) {
     ic = fgetc( channel ); *pb = (char) ic;
     i++;
    } /* while */

   *pval_pos = (unsigned long) ftell ( channel ) - 1;

   /* Read key until Terminator or until buffer full */
   i   = 0U;
   while ( (i<buflen-1) &&
           (*pb!=Terminator) && (ic!=EOF) && (ic) ) {
     ic = fgetc( channel ); pb++; *pb = (char) ic;
     i++;
     } /* while */

   /* Skip everything until Terminator */
   while ( (*pb!=Terminator) && (ic!=EOF) && (ic) ) {
     ic = fgetc( channel ); *pb = (char) ic;
     } /* while */

   /* Stop, if Terminator not read */
   if ((ic==EOF) || (!ic) || (*pb!=Terminator)) {
     long tmp;
     tmp = ftell(channel);
     fprintf(stderr,"\nERROR: position %ld,'%c' read, terminator '%c' not found\n",
       tmp, *pb, Terminator); return (-1); 
   }

   /* white space before Terminator does not belong to value */
   if ( (*pb==Terminator) && (i>0) && (is_white(buffer[i-1])) ) { i--; *pb--; }

   /* Replace terminating character with end of string */
   *pb = '\0';
   *pval_len = (unsigned long) i;

   return(0);

} /* get_val */

/*---------------------------------------------------------------------------
NAME

   get_symbol_list --- read all symbols, starting at current file position

SYNOPSIS

   int get_symbol_list( DBlock * block, int level );

DESCRIPTION
Starting at the current file position, all key/value pairs are read and 
inserted into the symbol list of 'block'. According to the 'level' argument 
only the data format keys (level=0) or all keys (level=1) are read.

level = 0 Only keys starting with DATA_FORMAT_PREFIX are read. The read
          stops at the first key not starting with DATA_FORMAT_PREFIX or
          after 'header_end'. To read all keys the file pointer has to be 
          repositioned after the 'header_start' marker and then read with 
          level=1. In case of success, the file pointer is positioned at 
          the beginning of the first key that does not start with 
          DATA_FORMAT_PREFIX or it is pointing to the 'EndHeader' character.

level = 1 All keys are read. The read stops at the end of the header. In
          case of success, the file pointer is pointing to the 'EndHeader' 
          character. 

RETURN VALUES
Returns 0 in case of success and -1 if no success.

HISTORY
30-Jan-1998 Peter Boesecke
---------------------------------------------------------------------------*/
int get_symbol_list( DBlock * block, int level )
{ const int   kblen = MaxKeyLen+1;
  char  kbuf[MaxKeyLen+1];
  const int   vblen = MaxValLen+1;
  char  vbuf[MaxValLen+1];

  unsigned long key_pos, val_pos;
  unsigned long key_len, val_len;

  FILE     * channel = block->Chain->File->Channel;
  SElement * symbol;

  int   stop;

  /* read keywords */
  stop = get_key(kbuf, kblen, channel, &key_pos, &key_len);
  if ( stop < 0 ) return(-1);

  /* stop on error */
  if ( !stop ) 
  while ( level ? 1 : (is_prefix ( kbuf, DATA_FORMAT_PREFIX, False )) ) {

    /* read value */
    stop=get_val( vbuf, vblen, channel, &val_pos, &val_len );
//    if (stop) if ( stop < 0 ) return(-1); else break;
    if (stop) { if ( stop < 0 ) return(-1); else break; }

    /* insert symbol */
    if ( insert_symbol( block, kbuf, vbuf, &symbol ) ) return(-1);
    symbol->KeyPos = key_pos;
    symbol->KeyLen = (unsigned short) key_len;
    symbol->ValPos = val_pos;
    symbol->ValLen = (unsigned short) val_len;
    /* read next key */
    stop=get_key( kbuf, kblen, channel, &key_pos, &key_len );
//    if (stop) if ( stop < 0 ) return(-1); else break;
    if (stop) { if ( stop < 0 ) return(-1); else break; }
/*  printf("Key = %s, KeyPos = %u, KeyLen = %u\n", kbuf, key_pos, key_len); */

    } /* while is_prefix */

  /* go back to previous key position */
  if (!stop) fseek( channel, (size_t) key_pos, SEEK_SET );

  return(0);

} /* get_symbol_list */

/*---------------------------------------------------------------------------
NAME
new_general_block

SYNOPSIS
int new_general_block( DFile * file );

DESCRIPTION
Creates a general block with default values and updates the file strucure.

RETURN VALUES
Returns 0 in case of success and -1 if no success.

HISTORY
18-Jan-1998 Peter Boesecke
16-Mar-2000 PB default_chain_key
---------------------------------------------------------------------------*/
int new_general_block( DFile * file )
{ char     ChainKeyDefinition[MaxKeyLen+1];
  char     ChainKey[MaxKeyLen+1];
  DChain * chain;
  DBlock * block;
  SElement * symbol;

  if (insert_data_chain( file, GENERAL_CHAIN_KEY, &chain) ) return(-1);
  if (insert_data_block( chain, GENERAL_BLOCK_KEY, &block) ) return(-1);

  if (!(file->GeneralBlock)) file->GeneralBlock = block;
    else return(-1); /* cannot create a second general block */

  /* Defaults */
  /* define default DataChain */
  if ( sprintf(ChainKeyDefinition,"%s%u",CHAIN_KEY_DEFINITION,1)<1 ) return(-1);

  if (!(default_chain_key(ChainKey,1))) return(-1);
  if (insert_string( block, ChainKeyDefinition, ChainKey, &symbol)) return(-1);

  file->ActiveBlock = block;
  /* don't write general block to disk, if NoGeneralHeader or TemporaryFile 
     are set */
  if ( (file->Flags.NoGeneralHeader)||(file->Flags.TemporaryFile) ) {
     block->Flags.InternalHeader = True; block->Flags.InternalData = True; }

  return(0);

} /* new_general_block */

/*---------------------------------------------------------------------------
NAME

   read_general_block

SYNOPSIS
   int read_general_block( DFile * file );

DESCRIPTION
Reads the general block from the file. Can only be called once after
'new_general_block'. The general block must be at the start of the file 
and MUST begin with 

   'header_begin' 'DATA_FORMAT_PREFIX'.

Otherwise it is assumed that this file has no general block. If the header 
contains a 'BLOCK_ID_KEY' the version number V2.0 is assumed with default 
block boundary. In the other case a block boudary of 1 byte is assumed.
In all successful cases the file pointer is positioned at the beginning of 
the first data block. 

RETURN VALUES
Returns 0 in case of success and -1 if no success.

HISTORY
18-Mar-1998 Peter Boesecke
11-Jul-2001 PB accepts empty files as edf-files without genereal header
---------------------------------------------------------------------------*/
int read_general_block( DFile * file )
{ static char errmsg[MaxValLen+1]; 
  int   stop=0;
  int   check_status;

  const size_t buflen = MaxValLen+1;
  char   fullname[MaxValLen+1], path[MaxValLen+1];

  unsigned long text_pos, binary_pos;
  unsigned long binary_len;

  FILE   * channel = file->Channel;
  DBlock * block;
  SElement * symbol;

  /* get block */
  block = file->GeneralBlock;

  /* block must be created */
  if (!(block)) return(-1);

  /* goto start of file */
  rewind( channel ); 

  /* remember start position */
  text_pos = (unsigned long) ftell (channel);

  /* check StartHeader */
  switch( check_start( channel ) ) {
    case  0: /* read ALL keywords with and without DATA_FORMAT_PREFIX */
             if ( get_symbol_list( block, 1 ) ) {
               sprintf(errmsg, "Error reading header values");
               print_file_error ( stderr, file, text_pos, errmsg );
             return(-1); }
             break; // OK
    case -1: stop=1; break; // EOF (empty file, without keywords)
    default: {
      sprintf(errmsg, "This file is not a %s file",DATA_FORMAT_NAME);
      print_file_error ( stderr, file, text_pos, errmsg );
      return(-1);
    }
  } // switch

  /* DATA_FORMAT_VERSION_KEY */
  if ( search_symbol( block, DATA_FORMAT_VERSION_KEY, &symbol ) ) {
    /* this file has no general block */
    set_no_general_block ( block );
    /* BLOCK_ID_KEY */
    if ( (stop)||(!search_symbol( block, BLOCK_ID_KEY, &symbol )) ) {
      /* this file is empty or has a BLOCK_ID_KEY. It must be 
         version 2.xx without header */
      set_data_format_version ( block, str2version( "2.00" ) ); }
     else {
      set_data_format_version ( block, str2version( "1.00" ) );
      set_block_boundary ( block, 1U ); /* block boundary 1 */
     }
    /* recreate general block, because it contains wrong symbols */
    free_data_block_list( block->Chain ); /* remove general block */
    file->GeneralBlock = (DBlock *) NULL;
    if ( new_general_block( file ) ) return(-1); 
    rewind( channel ); 

    return(0); 

  } /* DATA_FORMAT_VERSION_KEY */

  /* BLOCK_BOUNDARY_KEY (alt: internal default) */
  if (search_symbol( block, BLOCK_BOUNDARY_KEY, &symbol ) ) {
    ; /* set_block_boundary( block, 1U ); +++++++++++++++*/
    // fprintf(stderr,"WARNING: Key \"%s\" not found\n", BLOCK_BOUNDARY_KEY);
    } else {
    if ( set_block_boundary( block, s2u_long( symbol->Value ) )) {
      sprintf(errmsg,"ERROR: Cannot change block boundary.");
      print_file_error ( stderr, file, text_pos, errmsg );
      return(-1);
      }
    }

  /* BINARY_SIZE_KEY (alt: 0U) */
  if (search_symbol( block, BINARY_SIZE_KEY, &symbol ) ) {
    binary_len = 0U; } 
    else { binary_len = s2u_long( symbol->Value ); } 

  /* BINARY_FILE_NAME_KEY (alt: null pointer */
  if ( !search_symbol( block, BINARY_FILE_NAME_KEY, &symbol ) ) { 
    /* get full path of header file and apply to binary file */
    getpath_edf( path, buflen, file->Name );
    sprintf( fullname, "%s%s", path, symbol->Value );
    block->BinaryFileName = newstr( fullname ); 
    if (!block->BinaryFileName) return(-1); 
    /* BINARY_FILE_POSITION_KEY (alt: ERROR) */
    if ( !search_symbol( block, BINARY_FILE_POSITION_KEY, &symbol ) ) {
      block->BinaryFilePos = s2u_long( symbol->Value ); }
      else { fprintf(stderr,"ERROR: %s requires %s\n",
        BINARY_FILE_NAME_KEY,BINARY_FILE_POSITION_KEY); return(-1); }
    /* BINARY_FILE_SIZE_KEY (alt: 0U) */
    if ( !search_symbol( block, BINARY_FILE_SIZE_KEY, &symbol ) ) {
      block->BinaryFileLen = s2u_long( symbol->Value ); }
    else block->BinaryFileLen = 0U;

    } /* BINARY_FILE_NAME_KEY */

  /* HEADER END */
  if (stop) { if ( check_end ( channel ) < 0 ) return(-1); } 
    else { /* search end of this header section */
    check_status = search_end( channel,text_pos,block_boundary( block ));
    if ( check_status == -1) /* search againg in 1 byte steps */
      check_status = search_end( channel,text_pos,1);
    if ( check_status ) {
      sprintf(errmsg,"Header end marker not found"); 
      print_file_error ( stderr, file, text_pos, errmsg );
      return(-1); }
    }

  binary_pos = (unsigned long) ftell (channel);

  block->TextPos   = text_pos;
  block->TextLen   = binary_pos - text_pos;
  block->BinaryPos = binary_pos;
  block->BinaryLen = binary_len;

  /* check position relative to block boundary of end marker if */ 
  if ( binary_pos % block_boundary( block ) ) {
    /* Warning only if binary section exist (mixed text binary file)*/
    if ( binary_len ) {
      fprintf(stderr,
      "WARNING: The binary section of this block starts at position %lu.\n",
             binary_pos);
      fprintf(stderr,"         This is not a multiple of the block boundary %lu.\n",
             block_boundary( block )); 
      }

    fprintf(stderr,"\nINFO: Reading the file %s with block boundary 1.\n\n",
            block->Chain->File->Name);
    set_block_boundary( block, 1U );
    }

  /* update data block flags */
  block->Flags.DiskBlockUsed  = True;
  block->Flags.DiskBlockFixed = True;
  block->Flags.HeaderExternal = False;

  file->LastBlockInFile=block;

  /* search start position of next block */
  if ( fseek( channel, binary_pos+binary_len, SEEK_SET) ) {
     perror("read_general_block->fseek"); return(-1); }

  return(0);

} /* read_general_block */

/*---------------------------------------------------------------------------
NAME

   locate_block

SYNOPSIS
   int locate_block( DFile * file, DBlock ** pblock );

DESCRIPTION
Reads a block from the file, starting at the current position.  
The block must start with:

   'header_begin' 

In versions < 2.0 the full header is scanned, for version >= 2.0
only the format specific symbols are read and the end of the header
is searched at multiples of BlockBoundary positions from the start of 
the header. The file pointer is located after the binary section of
this block at the start of the next block.

For V2.xx the data block id is used, for V1.xx the V1_IMAGE_KEY or
alternatively, if this was not found, V1_HEADER_ID_KEY is used to 
get the image number.

RETURN VALUES
success:                    0, all temporary memory is released
end of file:               -1, all temporary memory is released.
                               previously read file OK
further read not possible: -2, all temporary memory is released, 
                               previously read file OK
severe error:              -3, temporary memory might not be released.
                               program should be terminated.

Pointer *pblock to the located block.

HISTORY
28-Feb-1998 Peter Boesecke
19-Mar-1998 Peter Boesecke, severe error -3, error, but file good until now: -2
---------------------------------------------------------------------------*/
int locate_block( DFile * file , DBlock **pblock )
{ int   stop=0;
  int   check_status;
  const size_t buflen = MaxValLen+1;
  char   fullname[MaxValLen+1], path[MaxValLen+1];

  unsigned long text_pos, binary_pos;
  unsigned long text_len, binary_len;

  FILE   * channel = file->Channel;
  DChain * chain, *chain_tmp;
  DBlock * block, *block_tmp;
  SElement * symbol;

  long   *data_dim;
  size_t  data_len;

  DFVersion  version = file->Version;
  char       BlockIDBuffer[MaxValLen+1];
  char       ChainKeyBuffer[MaxValLen+1];
  char      *BlockKey, *ChainKey;

//  const DFVersion  V1_0 = str2version("1.00");
  const DFVersion  V2_0 = str2version("2.00");

  /* remember start position */
  text_pos        = (unsigned long) ftell (channel);

  /* CHECK HEADER START  */
  check_status = check_start( channel );
  if ( check_status ) {
    if ( check_status == -1 ) return(-1);
      else {
        print_file_warning ( stderr, file, text_pos, 
                             "Expecting EOF or header start" );
        return(-2);
        }
    }

  /* create temporary structure */
  if ( insert_data_chain( file,  "_TEMP_", &chain_tmp ) ) return(-3);
  if ( insert_data_block( chain_tmp, "_TEMP_", &block_tmp ) ) return(-3);

  /* READ FORMAT SPECIFIC KEYWORDS (if Version<2.00 read all) */
  if ( compare_versions( version, V2_0 ) < 0 ) { /* VERSION < 2.00 */
    if ( get_symbol_list( block_tmp, 1 ) ) {
      if ( free_data_chain( chain_tmp ) ) return(-3);
      print_file_warning ( stderr, file, text_pos, 
                           "Error reading header values V1.xx");
      return(-2); } 
  } else { /* VERSION >= 2.00 */
    if ( get_symbol_list( block_tmp, 0 ) ) {
       if ( free_data_chain( chain_tmp ) ) return(-3);
       print_file_warning ( stderr, file, text_pos, 
                            "Error reading header values");
       return(-2); }
  }

  /* BLOCK_ID_KEY (alt: V1_IMAGE_KEY, alt2: HeaderID, alt3: SequenceNumber) */
  if ( !search_symbol( block_tmp, BLOCK_ID_KEY, &symbol ) ) {
    if ( split_block_id( BlockIDBuffer, MaxValLen+1, symbol->Value,
       &BlockKey, &ChainKey ) ) {
       if ( free_data_chain( chain_tmp ) ) return(-3);
       print_file_warning ( stderr, file, text_pos, 
                            "Error reading block id");
       return(-2);}
   }
  else if ( !search_symbol( block_tmp, V1_IMAGE_KEY, &symbol ) ) {
    // alt1: V1_IMAGE_KEY
    sprintf(BlockIDBuffer,"%s.%s",
      symbol->Value,default_chain_key(ChainKeyBuffer,1));
    if ( split_block_id( BlockIDBuffer, MaxValLen+1, BlockIDBuffer,
       &BlockKey, &ChainKey ) ) {
       if ( free_data_chain( chain_tmp ) ) return(-3);
       print_file_warning ( stderr, file, text_pos, 
                            "Error reading block id");
       return(-2);} 
   }
  else if ( !search_symbol( block_tmp, V1_HEADER_ID_KEY, &symbol ) ) {
   // alt2: HeaderID 
    sprintf(BlockIDBuffer,"%ld.%s",
      header_id_number(symbol->Value),default_chain_key(ChainKeyBuffer,1));
    if ( split_block_id( BlockIDBuffer, MaxValLen+1, BlockIDBuffer,
       &BlockKey, &ChainKey ) ) {
       if ( free_data_chain( chain_tmp ) ) return(-3);
       print_file_warning ( stderr, file, text_pos, 
                            "Error reading V1.xx header id");
       return(-2); }
   }
  else {
   // alt3: SequenceNumber 
    fprintf(stderr,"WARNING: Missing block ID, using sequence number\n");
    sprintf(BlockIDBuffer,"%ld.%s", 
      file->NextSequenceNumber-1,
      default_chain_key(ChainKeyBuffer,1));
    if ( split_block_id( BlockIDBuffer, MaxValLen+1, BlockIDBuffer,
       &BlockKey, &ChainKey ) ) {
       if ( free_data_chain( chain_tmp ) ) return(-3);
       print_file_warning ( stderr, file, text_pos,
                            "Failed using sequence number");
       return(-2); }
   }

  /* BINARY_SIZE_KEY (alt: V1_SIZE_KEY) */
  if ( !search_symbol( block_tmp, BINARY_SIZE_KEY, &symbol ) ) 
    binary_len = s2u_long( symbol->Value ); 
  else if ( !search_symbol( block_tmp, V1_SIZE_KEY, &symbol ) ) 
    binary_len = s2u_long( symbol->Value ); 
  else if ( (data_dim = get_data_dim ( block_tmp )) ) { 
     /* FIT2D KLORA FORMAT (V1_SIZE_KEY missing, but dimensions given) */
     data_len = edf_dim_product(data_dim)*edf_data_sizeof(get_data_type(block_tmp));
     if (EDFIO_debug) {
       printf ("FIT2D KLORA FORMAT dim[0]=%lu, dim[1]=%lu, dim[2]=%lu, data_len=%lu\n",\
         data_dim[0],data_dim[1],data_dim[2],data_len);
     }
     binary_len = data_len;
  } else binary_len = 0U; 

  /* BINARY_FILE_NAME_KEY (alt: null pointer */
  if ( !search_symbol( block_tmp, BINARY_FILE_NAME_KEY, &symbol ) ) {
    /* get full path of header file and apply to binary file */
    getpath_edf( path, buflen, file->Name );
    sprintf( fullname, "%s%s", path, symbol->Value );
    block_tmp->BinaryFileName = newstr( fullname );
    if (!block_tmp->BinaryFileName) {
       if ( free_data_chain( chain_tmp ) ) return(-3);
       print_file_warning ( stderr, file, text_pos, 
                            "Error reading binary file name");
       return(-2);
     }
    /* BINARY_FILE_POSITION_KEY (alt: ERROR) */
    if ( !search_symbol( block_tmp, BINARY_FILE_POSITION_KEY, &symbol ) ) {
      block_tmp->BinaryFilePos = s2u_long( symbol->Value ); }
    else { fprintf(stderr,"ERROR: The key \"%s\" requires the key \"%s\"\n",
      BINARY_FILE_NAME_KEY,BINARY_FILE_POSITION_KEY); 
      if ( free_data_chain( chain_tmp ) ) return(-3);
      print_file_warning ( stderr, file, text_pos, 
                           "Missing binary file position");
      return(-2); }
    if ( !search_symbol( block_tmp, BINARY_FILE_SIZE_KEY, &symbol ) ) {
      block_tmp->BinaryFileLen = s2u_long( symbol->Value ); }
    else block_tmp->BinaryFileLen = 0U;
    
    } /* BINARY_FILE_NAME_KEY */

  /* SEARCH HEADER END */
  if (stop) { if ( check_end ( channel ) < 0 ) return(-1); }
    else { /* search end of this header section */
    check_status = search_end( channel, text_pos, file->BlockBoundary );
    /* in case of EOF or '\0' search again */
    if ( check_status == -1 ) /* search again in 1 byte steps */
      check_status = search_end( channel, text_pos, 1 ); 
    if ( check_status ) {
      if ( free_data_chain( chain_tmp ) ) return(-3);
      print_file_warning ( stderr, file, text_pos, 
                           "Header end marker not found");
      return(-2); } 
    } /* if (stop) */

  binary_pos = (unsigned long) ftell (channel);

  text_len = binary_pos - text_pos;

  /* CHECK POSITION relative to block boundary of end marker */

  if ( binary_pos % file->BlockBoundary ) {
    if (binary_len) {
      fprintf(stderr,
      "WARNING: The binary section of this block starts at position %lu.\n",
             binary_pos);
      fprintf(stderr,"         This is not a multiple of the block boundary %lu\n",
             block_boundary( block_tmp ) );
      }

    fprintf(stderr,"\nINFO: Reading the file %s with block boundary 1.\n\n",
           block_tmp->Chain->File->Name);
    set_block_boundary( block_tmp, 1U );
    }

  if ( insert_data_chain( file, ChainKey, &chain ) ) return(-3);
  if ( insert_data_block( chain, BlockKey, &block ) ) return(-3);

  /* data block positions */
  block->TextPos         = text_pos;
  block->TextLen         = text_len;
  block->BinaryPos       = binary_pos;
  block->BinaryLen       = binary_len;

  block->BinaryFileName  = newstr(block_tmp->BinaryFileName);
  block->BinaryFilePos   = block_tmp->BinaryFilePos;
  block->BinaryFileLen   = block_tmp->BinaryFileLen;

  /* data block flags */
  block->Flags.DiskBlockUsed     = True;
  block->Flags.DiskBlockFixed    = True;
  block->Flags.HeaderExternal    = True;
  block->Flags.HeaderChanged     = False;
  block->Flags.DataExternal      = True;
  block->Flags.DataChanged       = False;
  block->Flags.ExternalDimAlloc  = False;
  block->Flags.ExternalDataAlloc = False;
  block->Flags.BadBlock          = False;

  /* remove temporary data_chain */
  if ( free_data_chain( chain_tmp ) ) return(-3);

  *pblock = block;

  file->LastBlockInFile=block;

  /* search start position of next block */
  if ( fseek( channel, binary_pos+binary_len, SEEK_SET) ) {
    print_file_warning ( stderr, file, binary_pos+binary_len, 
                       "Cannot find start of next block");
    return(-2);
   }

  return(0);

} /* locate_block */

/*---------------------------------------------------------------------------
NAME
   get_data_header --- read data block header from disk

SYNOPSIS

   int get_data_header( DBlock * block );

DESCRIPTION
Reads all data block symbols from the stream. 

RETURN VALUE
success: 0;
otherwise: -1;
---------------------------------------------------------------------------*/
int get_data_header( DBlock * block )
{ unsigned long cur_pos;
  FILE * channel = block->Chain->File->Channel;

  if (block)
    if (block->Flags.HeaderExternal) {
      /* get block position */

      cur_pos = block->TextPos;

      /* search block position */
      if ( fseek( channel, cur_pos, SEEK_SET) ) {
         perror("get_data_header->fseek"); return(-1); }

      /* check StartHeader */
      if ( check_start( channel ) ) {
         perror("get_data_header->check_start"); return(-1); }

      /* read symbols */
      if ( get_symbol_list( block, 1 ) ) {
         perror("get_data_header->get_symbol_list"); return(-1); }

      /* update data block flags */
      block->Flags.DiskBlockUsed  = True;
      block->Flags.DiskBlockFixed = True;
      block->Flags.HeaderExternal = False;
      block->Flags.HeaderChanged  = False;
  }
 
  return(0);

} /* get_data_header */

/*===block_access END===================================================---*/

/*===raster_conversion BEGIN=================================================*/

/* public interface of raster_conversion -------------------------------------

  int edf_raster_normalization ( void * dest, const void * src,
                                 const long data_dim[],
                                 long raster_configuration, size_t item )

  long edf_raster_multiplication ( long a, long x );

  long edf_raster_inversion ( long x );

  long raster_number  ( long n );

  long edf_order2raster   ( const long order[] );

  long * raster2order ( long n , long raster_configuration );

  long * order_multiplication ( const long a_order[] , const long x_order[] );

  long * order_inversion ( const long x_order[] );

  void rc_debug       ( int debug );
  
  ----------------------------- end of public interface of raster_conversion */

# define MAX_RASTER_CONFIG_DIMENSION 9

static int RN_debug = 0;
/*--------------------------------------------------------------------------
NAME

  rc_debug --- set / reset module raster_conversion into debug mode

SYNOPSIS

  void rc_debug ( int debug );

DESCRPTION

  Writes 'debug' into RN_debug.

--------------------------------------------------------------------------*/
void rc_debug ( int debug )
{ RN_debug = debug;
} /* rc_debug */

/*--------------------------------------------------------------------------
NAME

  raster_number --- number of raster configurations of an n-dimensional array
 
SYNOPSIS

  long raster_number ( long n );

DESCRIPTION
  Calculates the number A(n) of raster configurations of an n-dimensional 
  array:

      A(n) = 2^n * (n!)

  For n>9 the number of configuration are out of the value range of long int.
  In this case the returned value is 0.

ARGUMENTS
  long n  dimension  (number of coordinates)

RETURN VALUE
  long  A(n) = 2^n * (n!)  (for 0<n<=9, otherwise 0)

HISTORY
  11-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
long raster_number ( long n )
{ const long max_n = MAX_RASTER_CONFIG_DIMENSION;
  register long i;
  long A;

  if ((n<0) || (n>max_n)) return ( 0 );
  A=1; 
  for (i=1;i<=n;i++) A*=2*i; 
  return( A );

} /* raster_number */

/*--------------------------------------------------------------------------
NAME

  value2index --- return index of value in order

SYNOPSIS

  long value2index( const long order[], long n, long value );

DESCRIPTION
  Starting with 'index=1' at 'order[1]' ('order[0]' contains the length of the 
  array) it increments 'index' at each array element which absolute value
  is smaller than n until an array element with abs('order[i]') = 'value' 
  is found. The 'index' of this array element is returned, positive if 
  'value' is positive, negative otherwise.

ARGUMENTS
  const long order[0] == N dimension (number of coordinates)
  long       order[n+1]    raster configuration array
  long       value         value to be found 

RETURN VALUE
  long       index          

HISTORY
  12-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
long value2index( const long order[], long n, long value )
{ long i;
  long N, index;

  value = labs(value);
  if ( (!order) || (value<1) || (n<value) ) return(0);

  N = order[0];
  index=1;
  for (i=1;i<=N;i++) {
    if (labs(order[i])<=n) {
      if (order[i]==value) break; 
      else if (order[i]==-value) { index=-index; break; }
      index++;
      }
   } /* for */

  if (index>n) index=0;

  return(index);

} /* value2index */

/*--------------------------------------------------------------------------
NAME

  value2order --- insert value at n-th free index

SYNOPSIS

  void value2order( long order[], long n, long index, long value );

DESCRIPTION
  Starting with order[1] (order[0] contains the length of the array) it skips 
  all non-zero array elements and decrements 'index' at each zero array 
  element. It replaces the 'index'ed zero array element with 'value'. 

ARGUMENTS
  long      order[n+1]   raster configuration array
  long      n            dimension (number of coordinates)
  long      index        index of FREE cell at which the value will be written.
  long      value        value to be written 

RETURN VALUE
  void

HISTORY
  11-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
void value2order( long order[], long N, long index, long value )
{ long i;

  if (index==0) return;
  for (i=1;i<=N;i++) {
    if (order[i]==0) 
      if (--index==0) { order[i] = value; break; }
   } 

  return;

} /* value2order */

/*--------------------------------------------------------------------------
NAME

  raster2order --- returns the coordinate order array of a raster configuration

SYNOPSIS

  long * raster2order ( long n , long raster_configuration );

DESCRIPTION
  The n-dimensional configuration order array corresponding to 
  'raster_configuration' is returned. For n>9 not all configurations 
  can be calculated due to the limited value range of long int. 

ARGUMENTS
  long      n                            dimension (number of coordinates)
  long      raster_configuration         raster configuration number

  An array with n+1 elements is allocated and must be free'ed by the
  calling program.

RETURN VALUE
  long * raster_coordinate      (allocated array with n+1 elements
         raster_coordinate[0]   dimension of array
         raster_coordinate[1]   coordinate corresponding to fastest index (1)
            ...
         raster_coordinate[n]   coordinate corresponding to slowest index (n) 
   In case of an error NULL is returned.

HISTORY
  14-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
long * raster2order ( long n , long raster_configuration )
{ const long max_n = MAX_RASTER_CONFIG_DIMENSION;
  long AN, ANm1x2, ANm1;
  long D = raster_configuration;
  long N = n;

  long * order = (long *) NULL;

  long index;
  long value;

  long m;

  /* allocate array and clear it */
  if ( !(order = (long*) calloc( (N+2), sizeof(long) )) ) return(NULL);
  order[0] = N;

  /* special case for n>max_n */
  if ( (D <= raster_number( max_n )) && ( n>max_n ) ) {
    for (index=max_n+1;index<=n;index++) order[index]=index; n=max_n; }

  AN     = raster_number ( n );

  /* check parameters */
  if ( !((0l<D) && (D<=AN)) ) {
     free ( order ); return( NULL ); /* undefined */ }

  for (n=n;n>0;--n) {
    ANm1   = AN/(n+n); if (AN==0) AN=1;
    ANm1x2 = ANm1+ANm1;

    m = (D-1) / ANm1x2;   /* m = 0, ... ,n-1 */
    D = D - m * ANm1x2;
    index = n - m;        /* index = 1, ..., n */
    if (D<=ANm1) value = n; else { value = -n;  D=D-ANm1; }
    value2order( order, N , index, value ); 

    AN     = ANm1;

    } /* for */

  return( order );

} /* raster2order */

/*--------------------------------------------------------------------------
NAME

  edf_order2raster --- raster configuration_number of a configuration order array

SYNOPSIS

  long edf_order2raster ( long order[] );

DESCRIPTION
  The raster configuration number of the order[0]-dimensional configuration 
  array 'order' is returned. For order[0]>9 not all configuration numbers
  can be calculated due to the limited value range of long int. In this case
  a zero is returned.

ARGUMENTS
  long      order[0]  number of dimensions (long order[order[0]+1]) 
  long      order[n]  configuration order array 
 
The element 'order[0]' must contain the dimension. At least 'order[0]+1' 
elements must be allocated for 'order[]'. 

RETURN VALUE
  success   long    raster_configuration
  error     long    0 

HISTORY
  12-Mar-1998 Peter Boesecke
--------------------------------------------------------------------------*/
long edf_order2raster ( const long order[] )
{ long i, n;
  long ANm1, D;
  long index;

  if (!order) return(0);

  n = order[0];

  if (MAX_RASTER_CONFIG_DIMENSION < n) {
    for (i=MAX_RASTER_CONFIG_DIMENSION+1; i<=n; i++)
      if (order[i]!=i) return(0); 
    n = MAX_RASTER_CONFIG_DIMENSION;
    }

  D = 1;

  ANm1 = raster_number ( 0 );
  for (i=1;i<=n;i++) {
    index = value2index( order, i, i );
    if ( index == 0 ) return(0);
    if ( index > 0 ) D += (i-index)*2*ANm1; 
    else D += (i+index) * 2*ANm1 + ANm1;

    ANm1 = ANm1 * 2 * i;   /* raster_number ( i ); */
    }

  return( D );
 
} /* edf_order2raster */

/*--------------------------------------------------------------------------
NAME

  order2wrap --- calculates the wrap array from order and data_dim 

SYNOPSIS

  int order2wrap ( long wrap[], const long dim[], const long order[] );

DESCRIPTION
  Calculates the array 'wrap' that contains for each index the displacement
  in number of array elements when it is incremented by 1. For the fastest 
  index wrap[fast] is 1.

ARGUMENTS
  long wrap[N+1]     output array element, must be sufficiently large
  long dim[N+1]      data array dimensions, dim[0] is N
  long order[N+1]    input order of dimensions
  All three array us only index 1 to index N. The values for the n_th 
  dimension is found in array[n], array[0] is used for the number of 
  dimensions, array[n+1] is used for the total length of the data array.

RETURN VALUE
int status
success : 0
error   : -1

AUTHOR
  15-Mar-1998 Peter Boesecke

--------------------------------------------------------------------------*/
int order2wrap ( long wrap[], const long dim[], const long order[] )
{ long N = dim[0];
  long i,j; 

  wrap[0] = dim[0]; wrap[N+1] = dim[N+1];

  for (i=1;i<=N;i++) {
    wrap[i]=1l;
    for (j=1;j<labs(order[i]);j++) wrap[i] *= dim[j];
    }

  return( 0 );

} /* order2wrap */

/*--------------------------------------------------------------------------
NAME

  reorder_raster --- reorder recursively the array src and write to dest 

SYNOPSIS

  void reorder_raster ( long n, void * dest, const void ** pps, size_t item, 
                        long I[], const long IS[], const long ILoop[], 
                        const long IInc[], const long IWrap[] );

DESCRIPTION
  Recursive function for reordering of a n-dimensional array. 
  The source array is read element by element. The elements of the 
  destination array are written according to the numbers in IS, ILoop, 
  IInc and IWrap.

ARGUMENTS
      long n           dimension of the data arrays
      void * dest      destination array (must be allocated)
const void ** pps      pointer to source array of dimension n 
                       (must contain useful data)
      t_size item      length of a single array element
      long I[n+2]      array that contains the current indices 
                       (must be allocated, will be used by this routine)
const long IS[n+2]     array that contains the start value of each index 
const long ILoop[n+2]  array that contains the number of loops of each index
const long IInc[n+2]   array that contains the increment of each index
const long IWrap[n+2]  array that contains the distance between elements in
                       the destination array. For no reordering (type 1
                       to type 1) IWrap[1] would be equal to 1, IWrap[2]
                       would be equal to Dim[1], IWrap[3] would be equal
                       to Dim[1]*Dim[2] etc. IWrap[0] must contain the
                       number of dimensions.

AUTHOR
  15-Mar-1998 Peter Boesecke
  17-May-1998 PB calculation of pd corrected: I[nn]  -> (I[nn]-1l)
  26-Nov-1999 PB cc on dec alpha: statements of the form
                 (void *) pp + (size_t) item; are not allowed, therefore
                 changed to (char *) pp1 + (size_t) item; pp = (void *) pp1;
  19-Dec-1999 PB pps1 correctly defined as char **pps1
-----------------------------------------------------------------------------*/
void reorder_raster ( long n, void * dest, const void ** pps, size_t item,
                      long I[], const long IS[], const long ILoop[],
                      const long IInc[], const long IWrap[] );
void reorder_raster ( long n, void * dest, const void ** pps, size_t item, 
                      long I[], const long IS[], const long ILoop[], 
                      const long IInc[], const long IWrap[] )
{ long ii, nn;
  long N = IWrap[0];
  char *pd, **pps1;

  if (n>0) {
    for (ii=0,I[n]=IS[n]; ii<ILoop[n]; ii++,I[n]+=IInc[n])
      reorder_raster ( n-1, dest, pps, item, I, IS, ILoop, IInc, IWrap ); }
  else { 
    pd = (char *) dest;
    for (nn=1;nn<=N;nn++) pd += item * (I[nn]-1l) * IWrap[nn];

    memcpy ( (void *) pd, *pps, item );
    pps1 = (char **) pps;
    *pps1 += item; }

} /* reorder_raster */

/*+++--------------------------------------------------------------------------
NAME

  edf_raster_normalization --- conversion to raster configuration 1

SYNOPSIS

  int edf_raster_normalization ( void * dest, const void * src,
                                 const long data_dim[],
                                 long raster_configuration, size_t item )

DESCRIPTION
  Conversion of the multi-dimensional array src with raster configuration 
  number 'raster_configuration' to the n-dimensional array dest with 
  raster configuration number 1. The length n of data_dim is stored in 
  data_dim[0]. The total length of the array is data_dim[n+1]. data_dim[i] 
  is the length of coordinate i. Input and output array must be different. 
  Sufficient memory must have been allocated for both arrays. 
  The total number of elements in the array is specified in data_dim[n+1].

  The raster configuration specifies only the way how data is stored. It does
  not influence the number of dimensions. Therefore, the dimension array is
  not changed even if, apparently, horizontal and vertical axes were changed.

  To convert array src from any raster configuration 'a' to any other raster 
  configuration 'b' the 'raster_configuration' argument is given by 

  'raster_configuration' = 
      edf_raster_multiplication( edf_raster_inversion('b'), 'a' );

ARGUMENTS
  void * dest                  output array (must be allocated), must
                               be different from source array
  const void * src             source array
  const long data_dim[]        source dimensions
  long raster_configuaration   raster configuration number of source array
  size_t item                  size of an array element
  
GENERAL
  An n-dimensional array has 'N = 2^n * (n!)' different ways of storing its
  data in a regular raster. Each of the n axes can be stored in two different
  ways: up and down. This results in 2^n different possibilities of data
  storage. The n axes can be stored in any of the (n!) possible permutations.
  This results in N = 2^n * (n!) different ways of storing the data. A
  two dimensional array (n=2) can be stored in 2^2 * 2! = 8 different ways, a
  three dimensional array (n=3) can be stored in 2^3 * 3! = 8*6 = 48 different
  ways, and so on.

  The data elements are stored in an array with dim_1 * dim_2 * ... * dim_n
  cells with identical size. The number of dimension is n. The first
  index (i_1) is the fastest. A specific raster configuration is given by 
  the n-tupel (k_2, -k_3, -k_1), which means that the fastest index 
  i_1 corresponds to the coordinate k_2, the medium fast index i_2 
  corresponds to the invers k_3 direction and the slowest coordinate i_3 
  corresponds to the invers k_1 direction. 

RASTER CONFIGURATION 
  Arrays can be stored in different ways depending on the relationship
  between offsets and fast and slow array indices. The configuration of
  the indices is specified by a raster configuration number D.

  A unique raster configuration number for multi dimensional arrays is 
  used that is defined on the basis of the following demands. The raster
  configuration number is called D in the text:

  - Array indices are numbered from 1 to N. 

  - A one dimensional array can be stored from low array indices to high 
    array indices (D=1) or from high array indices to low array indices
    (D=2).
 
  - The array element Array(k_1, k_2, k_3, ... , k_N) is accessed with an
    offset I from the first element measured in element size

    I = (i_1-1) + (i_2-1) * Dim[1] + (i_3-1) * Dim[2] * Dim[1] + ...

      = Sum[ J=1,J<=N,J++ ]( i_J * Product[i=1,i<=J,i++](Dim[i]) )

      (Dim[0]==1)
   
    with i_1, i_2, i_3, ...  replaced by k_1, k_2, k_3, ... in a 
    special order that is specified by the raster configuration number D.

    The raster configuration is described by grouping the array indices 
    k_nn and the offset indices i_nn, e.g. for a 2-dimensional array where 
    the fast and slow indices are interchanged. 

    Example for N=2 and D=5: 

    i_n   1, 2 
    k_nn  2, 1 

  - The raster configuration number D is 1, when the array indices are ordered
    from "fast" (i_1) to "slow" (i_N), e.g. when i_1 = k_1, i_2 = k_2, etc. 
    
  - The raster configuration D of the n-dimensional sub-array 
    Array[Dim_1, Dim_2, ... Dim_n] is the same as for the (n+1) 
    dimensional array Array[Dim_1, Dim_2, ... Dim_n, Dim_(n+1)] if 
    Dim_(n+1) == 1.
    
       D ( Array[Dim_1, Dim_2, ... Dim_n, 1 ] ) 
        == D ( Array[Dim_1, Dim_2, ... Dim_n] )

  These demands give a unique description of multi-dimensional raster 
  conformations with a configuration number D.

  The raster configuration is defined as follows:

  For the definition it is necessary to distinguish strictly between the 
  array indices k_1, k_2, etc. and the offset indices i_1, i_2, etc.

  A offset index with a small number, e.g. i_1 runs faster than an index 
  with a higher number, e.g. i_3. The definition of 
  the raster orientation is based on a standard orthogonal coordinate system 
  with the first coordinate (x_1) horizontal and the second coordinate (x_2) 
  vertical with respect to the observer. The position of the observer must be 
  defined elsewhere. In standard scattering geometry the observer is located at
  the sample position and is looking against the detector. The origin of the 
  coordinate system is at the lower left corner of the detector, independently 
  of any detector pixel readout coordinate. The direction of the third 
  coordinate x_3 is found with the vector product x_1 X x_2 = x_3. It is 
  pointing against the observer.  This might be usedful for representing three 
  dimensional objects. It should be possible to find higher dimensional 
  coordinates accordingly. 

  An n-dimensional array has 'N = 2^n * (n!)' different ways of storing its 
  data in a regular raster. Each of the n axes can be stored in two different 
  ways: up and down. This results in 2^n different possibilities of data
  storage. The n axes can be stored in any of the (n!) possible permutations.
  This results in N = 2^n * (n!) different ways of storing the data. A
  two dimensional array (n=2) can be stored in 2^2 * 2! = 8 different ways, a
  three dimensional array (n=3) can be stored in 2^3 * 3! = 8*6 = 48 different
  ways, and so on.

  The data elements are stored in an array with dim_1 * dim_2 * ... * dim_n 
  cells with identical size. The number of data elements is n. The first 
  index is the fastest. A specific raster configuration is given by the n-tupel
  (x_2, -x_3, -x_1), which means that the fastest index i_1 corresponds to 
  the coordinate x_2, the medium fast index i_2 corresponds to the invers x_3 
  direction and the slowest coordinate i_3 corresponds to the invers x_1 
  direction. If the array has n_1, n_2 and n_3 elements in each direction the 
  data origin X0 = (0,0,0) in the real world corresponds to the array element
  IX0 = (0,n_2,n_3). 

CONFIGURATION NUMBERS
  In the standard configuration (D=1) all array indices k_nn are identical
  to the offset indices i_nn. 

  In the 1-dimensional case the standard configuration (numbered 1) is the 
  configuration in which the array index increases with the coordinate. The 
  second configuration is where the index is antiparallel to the coordinate 
  (numbered 2).

  n=1 has two configurations (A(2) = 2): 

  n  raster_configuration D         Configuration
  1    1                              1
  1    2                             -1

  If the number of configurations for n coordinates is A(n) = 2^n * (n!),
  the number of configurations for n+1 coordinates is given by

     2 * (n+1) * A(n) = 2^(n+1) * (n+1)!
  
  C(n) is the group of all possible configurations for n coordinates. If it is
  given, the configurations for n+1 coordinates can be built by inserting the
  configurations of the new coordinate (1 and -1) at each of the n+1 possible 
  positions (n before each coordinate and 1 after the last coordinate). To
  have a well defined ordering the new coordinate is first added non-inverted 
  after the end of all A(n) configurations and then inverted (A(n)*2). This is 
  repeated subsequently from the end to the start before all n remaining 
  positions.   

    (x1, x2, x3, ... , xn) -> (x1, x2, x3, ... , xn,  xn+1) 
                              (x1, x2, x3, ... , xn, -xn+1)          +2 

                              (x1, x2, x3, ... ,  xn+1, xn)
                              (x1, x2, x3, ... , -xn+1, xn)          +2

                                          ...

                              ( xn+1, x1, x2, x3, ... , xn) 
                              (-xn+1, x1, x2, x3, ... ,-xn)          +2

                                                                   (n+1)*2
                                       A(n) = 2^n * (n!)


  The total number of configuration for n+1 coordinates is then

           2 * (n+1) * A(n) = 2^(n+1) * (n+1)! ,

  which is equal to A(n+1).   

EXAMPLE

  n  raster_configuration D        Configuration

  1    1                              1
  1    2                             -1

  2    1                              1, 2
  2    2                             -1, 2
  2    3                              1,-2
  2    4                             -1,-2
  2    5                              2, 1
  2    6                              2,-1
  2    7                             -2, 1
  2    8                             -2,-1

  3    1                              1, 2, 3
  3    2                             -1, 2, 3
  3    3                              1,-2, 3
  3    4                             -1,-2, 3
  3    5                              2, 1, 3
  3    6                              2,-1, 3
  3    7                             -2, 1, 3
  3    8                             -2,-1, 3
  3    9                              1, 2,-3
  3   10                             -1, 2,-3
  3   11                              1,-2,-3
  3   12                             -1,-2,-3
  3   13                              2, 1,-3
  3   14                              2,-1,-3
  3   15                             -2, 1,-3
  3   16                             -2,-1,-3
  3   17                              1, 3, 2
              ...
  3   32                             -2,-3,-1
  3   33                              3, 1, 2
              ...
  3   48                             -3,-2,-1

  The raster configuration 13 for n=3 (2, 1,-3) means that the first offset 
  index of the array (which is the fastest) corresponds to the coordinate 
  k_2, the second index corresponds to the coordinate k_1 and the third index 
  to the inverted coordinate k_3.

  The largest D (D = A(n)) is always the conformation where the direction
  and order of all array indices are inverted. 

RETURN VALUE
int status
success : 0
error   : -1

AUTHOR
  13-Mar-1998 Peter Boesecke
  17-May-1998 PB calculation of IS for positive order corrected:
                 IS[n_dim] = 0 changed to IS[n_dim] = 1.
-----------------------------------------------------------------------------*/
int edf_raster_normalization ( void * dest, const void * src, 
                               const long data_dim[], 
                               long raster_configuration, size_t item )
{ static const char * RN_Error  = "ERROR: edf_raster_normalization:";
  static const char * RN_NoDim  = "no or zero dimension";
  static const char * RN_NoMem  = "cannot allocate memory";
//  static const char * RN_NoSup  = "n>4 not supported";
  static const char * RN_NoRas  = "bad raster configuration";
  static const char * RN_Wrap   = "bad wrapping";

  long * order;
  long * IS, * IInc, * ILoop, * IWrap;
  long * I;

  const void * ps;
  long   n, n_dim;

  if (RN_debug) printf("\n edf_raster_normalization BEGIN\n");

  /* check that input and output arrays are different */
  if (dest==src) {
    printf("SEVERE %s Source and destination arrays must be different!",
            RN_Error); exit(-1); }

  /* get dimension */
  if (data_dim) n = data_dim[0]; else n = 0;
  if (n<=0) { fprintf(stderr,"%s %s\n",RN_Error, RN_NoDim); return(-1); }

  /* get index order */
  if (!(order = raster2order ( n , raster_configuration )) ) {
    fprintf(stderr,"%s %s\n",RN_Error, RN_NoRas); return(-1);}

  /* calculate loop parameters */
  if (!(IS = (long *) malloc ( sizeof(long) * (n+2) )) ) {
    fprintf(stderr,"%s %s\n",RN_Error, RN_NoMem); return(-1);}
  if (!(IInc = (long *) malloc ( sizeof(long) * (n+2) )) ) {
    fprintf(stderr,"%s %s\n",RN_Error, RN_NoMem); return(-1);}
  if (!(ILoop = (long *) malloc ( sizeof(long) * (n+2) )) ) {
    fprintf(stderr,"%s %s\n",RN_Error, RN_NoMem); return(-1);}
  if (!(IWrap = (long *) malloc ( sizeof(long) * (n+2) )) ) {
    fprintf(stderr,"%s %s\n",RN_Error, RN_NoMem); return(-1);}
  if (!(I = (long *) malloc ( sizeof(long) * (n+2) )) ) {
    fprintf(stderr,"%s %s\n",RN_Error, RN_NoMem); return(-1);}

  /* calculate start, increment, loop count */
  for (n_dim=1;n_dim<=n;n_dim++)
    if (order[n_dim]<0) {
      IS[n_dim] = data_dim[-order[n_dim]]; 
      IInc[n_dim] = -1; ILoop[n_dim] = data_dim[-order[n_dim]]; }
    else { IS[n_dim] = 1;
      IInc[n_dim] = 1; ILoop[n_dim] = data_dim[order[n_dim]]; }

  if ( order2wrap ( IWrap, data_dim, order ) ) {
    fprintf(stderr,"%s %s\n",RN_Error, RN_Wrap); return(-1);}

  if (RN_debug)
    for (n_dim=1;n_dim<=n;n_dim++) {
      printf(" raster_configuration = %ld\n", raster_configuration);
      printf("         data_dim[%ld] = %ld\n", n_dim, data_dim[n_dim]);
      printf("            order[%ld] = %ld\n", n_dim, order[n_dim]);
      printf("            IWrap[%ld] = %ld\n", n_dim, IWrap[n_dim]);
      printf("               IS[%ld] = %ld\n", n_dim, IS[n_dim]);
      printf("             IInc[%ld] = %ld\n", n_dim, IInc[n_dim]);
      printf("            ILoop[%ld] = %ld\n", n_dim, ILoop[n_dim]);
    } 

  ps = src;
  reorder_raster ( n, dest, &ps, item, I, IS, ILoop, IInc, IWrap );

  free ( I ); free ( IWrap ); free ( ILoop ); free ( IInc ); free ( IS );

  free ( order ); 

  if (RN_debug) printf(" edf_raster_normalization END\n");

  return(0);

} /* edf_raster_normalization */

/*--------------------------------------------------------------------------
NAME

  order_multiplication --- raster transformation of a configuration order array

SYNOPSIS

  long * order_multiplication ( const long a_order[] , const long x_order[] );

DESCRIPTION

ARGUMENTS
  const long a_order[n]  raster transformation array
  const long x_order[n]  raster configuration array

The elements 'a_order[0]' and 'x_order[0]' must contain the dimensions. At
least 'a_order[0]+1' and 'x_order[0]+1 elements must be allocated for
each array. 

RETURN VALUE
  success   long    * x_order
  error     long    NULL

If the return value is not equal to NULL a pointer to allocated memory is 
returned that must be free'd by the calling routine.

HISTORY
  2000-07-29 Peter Boesecke
--------------------------------------------------------------------------*/
long * order_multiplication ( const long a_order[] , const long x_order[] )
{ register int i;
  long I;
  long * y_order = (long *) NULL;
  long N = 0l;

  if (!(a_order && x_order)) return( 0 );
  if (a_order[0]>x_order[0]) return( 0 );
  N = a_order[0];

  /* allocate array and clear it */
  if ( !(y_order = (long*) calloc( (N+2), sizeof(long) )) ) return(NULL);
  y_order[0] = N;

  for (i=1;i<=N;i++) {
    I = a_order[i];
    if ( labs(I)<=x_order[0] ) {
      if (I>0) y_order[i] = x_order[I];
        else y_order[i] = - x_order[-I];
      } else y_order[i] = 0l; // not defined
    } // for

  return( y_order );

} /* order_multiplication */

/*--------------------------------------------------------------------------
NAME

  edf_raster_multiplication --- raster number after a raster transformation

SYNOPSIS

  long edf_raster_multiplication ( long a, long x );

DESCRIPTION
  The resulting raster configuration number of the transformations a*x 
  is returned. x is the input raster configuration number, a is the 
  raster transformation which is applied to x. 

ARGUMENTS
  long      a         raster transformation
  long      x         input raster configuration

  a and x must be smaller or equal to A(n) = 2^n * (n!)

RETURN VALUE
  success   long    raster_configuration
  error     long    0

HISTORY
  2000-07-29 Peter Boesecke
--------------------------------------------------------------------------*/
long edf_raster_multiplication ( long a, long x )
{ long * a_order;
  long * x_order;
  long * y_order;
  long value;

  long n = MAX_RASTER_CONFIG_DIMENSION;

  a_order = raster2order ( n , a );
  x_order = raster2order ( n , x );

  y_order = order_multiplication ( a_order , x_order );
  
  value = edf_order2raster ( y_order );

  if (a_order) free ( a_order );
  if (x_order) free ( x_order );
  if (y_order) free ( y_order );

  return ( value );

} /* edf_raster_multiplication */

/*--------------------------------------------------------------------------
NAME

  order_inversion --- returns inverse raster order array 

SYNOPSIS

  long * order_inversion ( const long x_order[] );

DESCRIPTION
  Returns the inverted raster order array a_order = x_order^-1,
  with a_order[] * x_order[] = 1[];

ARGUMENTS
  const long x_order[n]  input raster order array

The element 'x_order[0]' must contain the dimension. At least 'x_order[0]+1 
elements must be allocated for the array. 

RETURN VALUE
  success   long    * a_order 
  error     long    NULL

If the return value is not equal to NULL a pointer to allocated memory is 
returned that must be free'd by the calling routine.

HISTORY
  2000-07-29 Peter Boesecke
--------------------------------------------------------------------------*/
long * order_inversion ( const long x_order[] )
{ register int i;
  long I;
  long * a_order = (long *) NULL;
  long N = 0l;

  if (!(x_order)) return( 0 );
  N = x_order[0];

  /* allocate array and clear it */
  if ( !(a_order = (long*) calloc( (N+2), sizeof(long) )) ) return(NULL);
  a_order[0] = N;

  for ( i=1;i<=N;i++ ) {
    I = x_order[i];
    if ( labs(I)<=x_order[0] ) {
      if (I>0) a_order[I] = i;
        else a_order[-I] = -i;
      } else a_order[i] = 0l; // not defined
    } // for

  return( a_order );

} /* order_inversion */

/*--------------------------------------------------------------------------
NAME

  edf_raster_inversion --- returns raster number of the inverse transformation

SYNOPSIS

  long edf_raster_inversion ( long x  );

DESCRIPTION
  The raster number of the transformation a = x^-1 is returned
  that transforms configuration x to 1: a * x = 1 

ARGUMENTS
  long      x         input raster configuration

RETURN VALUE
  success   long    inverse raster configuration
  error     long    0

HISTORY
  2000-07-29 Peter Boesecke
--------------------------------------------------------------------------*/
long edf_raster_inversion ( long x )
{ long * a_order;
  long * x_order;
  long value;

  long n = MAX_RASTER_CONFIG_DIMENSION;

  x_order = raster2order ( n , x );

  a_order = order_inversion ( x_order );

  value = edf_order2raster ( a_order );

  if (a_order) free ( a_order );
  if (x_order) free ( x_order );

  return ( value );

} /* edf_raster_inversion */

/*===raster_conversion END===================================================*/

/*---get_binary_array BEGIN================================================*/
/*---------------------------------------------------------------------------
  NAME

  edf_bswap - byte swapping inside an item

  SYNOPSIS

  void edf_bswap( void * dest, const void * src, size_t item, unsigned long n );

  DESCRIPTION
  Swaps all bytes inside an item of 'src' consisting of 'item' bytes and
  writes the result to the output buffer 'dest'. The output and input
  buffers can be identical. The minimum size of both buffers in bytes
  is 'item' * 'n'.

         byte     byte     byte     byte    byte     byte     byte     byte
  src  :    0   |    1   |    2   |   3   |  ...  | item-3 | item-2 | item-1 |
  dest : item-1 | item-2 | item-3 |  ...  |   3   |    2   |    1   |    0   |

  ARGUMENTS

  void     * dest    ouput buffer<br>
  void     * src     input buffer<br>
  size_t   item      size of a single item in bytes <br>
  unsigned long n    number of items<br>

  AUTHOR
   Peter Boesecke 14-Jan-1998

  HISTORY
   30-Apr-2000 PB renamed to edf_bswap
  --------------------------------------------------------------------------*/
void edf_bswap ( void * dest, const void * src, size_t item, unsigned long n )
{
  const unsigned char *in;
  const unsigned char *pi1, *pi2;
  unsigned char       *out;
  unsigned char       *po1, *po2, temp;
  unsigned long        i, j;
  size_t               step = item;
  unsigned long        jmax = (step + 1u) / 2u;

  pi1 = in  = (const unsigned char *) src;
  pi2 = in + step;
  po1 = out = (unsigned char *) dest;
  po2 = out + step;

  for (i=0;i<n;i++) {
    /* swap */
    for (j=0;j<jmax;j++) { 
      temp=*pi1;pi1++; 
      pi2--;*po1=*pi2;po1++; 
      po2--;*po2=temp; 
    }
    pi1 =  in += step; pi2 =  in + step;
    po1 = out += step; po2 = out + step;
    } /* for */

} /* edf_bswap */

/*------------------------------------------------------------------------
NAME

   strarrlen --- returns the length of a string array. it must stop with NULL 

SYNOPSIS

   long strarrlen( const char * strarr[] );

DESCRIPTION
 Returns the length of an array of strings. The string array must end with
 (char *) NULL.

RETURN VALUE

   long : length of the string array

AUTHOR
  ------------------------------------------------------------------------*/
long strarrlen( const char * strarr[] )
{ long i=0;

  while (strarr[i]) i++;

  return(i);
 
} /* strarrlen */

/*---------------------------------------------------------------------------
NAME

  ByteOrder2String --- converts byte order to a string 

SYNOPSIS

  BOrder byte_order;
  const char * ByteOrder2String( int byte_order );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the variable
  with its enumerated type has been replaced by "int".

RETURN VALUE
  Pointer to a constant result string.

AUTHOR
03-Mar-1998 PB Specification
  -------------------------------------------------------------------------*/
PRIVATE const char * ByteOrder2String( int byte_order )
{  
   if ((byte_order<0)||(byte_order>=EndBOrder)) byte_order = InValidBOrder; 
   return( BOrderStrings[byte_order] );

} /* ByteOrder2String */

/*---------------------------------------------------------------------------
NAME

  String2ByteOrder --- converts a string to a byte order value 

SYNOPSIS

  (BOrder) int String2ByteOrder( const char * string );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the return 
  value with its enumerated type has been replaced by "int".

RETURN VALUE
  BOrder == 0 : error, e.g. cannot convert
  BOrder >  0 : valid byte order value

AUTHOR
03-Mar-1998 PB Specification
  -------------------------------------------------------------------------*/
int String2ByteOrder( const char * string )
{ int  NE=True;
  long i = 0;

  while ( (NE && BOrderStrings[i]) ) 
    NE = compare_keys( string, BOrderStrings[i++], UpperCaseSort );

  i = MAX(0,i-1);

  if (NE) return( InValidBOrder ); 
    else return( i );
    
} /* String2ByteOrder */

/*---------------------------------------------------------------------------
NAME

  edf_datatype2string --- converts data type to a string

SYNOPSIS

  DType data_type;
  const char * edf_datatype2string( int data_type )

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the variable
  with its enumerated type has been replaced by "int".

RETURN VALUE
  Pointer to a constant result string.

AUTHOR
03-Mar-1998 PB Specification
  -------------------------------------------------------------------------*/
const char * edf_datatype2string( int data_type )
{  
   if ((data_type<0)||(data_type>=EndDType)) data_type = InValidDType;
   return( DTypeStrings[data_type] );

} /* edf_datatype2string */

/*---------------------------------------------------------------------------
NAME

  edf_string2datatype --- converts a string to DType 

SYNOPSIS

  (DType) int edf_string2datatype( const char * string );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the return
  value with its enumerated type has been replaced by "int".

RETURN VALUE
  DType == 0 : error, e.g. cannot convert
  DType >  0 : valid byte order value

AUTHOR
03-Mar-1998 PB Specification
  --------------------------------------------------------------------------*/
int edf_string2datatype( const char * string )
{ int  NE=True;
  long i = 0;

  while ( (NE && DTypeStrings[i]) )
    NE = compare_keys( string, DTypeStrings[i++], UpperCaseSort );

  /* aliases */
  if (NE) { i=0;
    while ( (NE && DTypeStringsAliases[i]) )
      NE = compare_keys( string, DTypeStringsAliases[i++], UpperCaseSort );
    }

  /* aliases */
  if (NE) { i=0;
    while ( (NE && DTypeStringsAliases1[i]) )
      NE = compare_keys( string, DTypeStringsAliases1[i++], UpperCaseSort );
    }

  i = MAX(0,i-1);

  if (NE) return( InValidDType );
    else return( i );

} /* edf_string2datatype */

/*---------------------------------------------------------------------------
NAME

  Compression2String --- converts compression to a string

SYNOPSIS

  DCompression data_compression;
  const char * Compression2String( int data_compression )

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the variable
  with its enumerated type has been replaced by "int".

RETURN VALUE
  Pointer to a constant result string.

AUTHOR
05-Mar-1998 PB 
  -------------------------------------------------------------------------*/
PRIVATE const char * Compression2String( int data_compression )
{  
   if ( (data_compression < 0) || (data_compression >= EndDCompression) )
     data_compression = InValidDCompression;

   return( DCompressionStrings[data_compression] );

} /* Compression2String */

/*---------------------------------------------------------------------------
NAME

  String2Compression --- converts a string to a byte order value

SYNOPSIS

  (DCompression) int String2Compression( const char * string );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the return
  value with its enumerated type has been replaced by "int".

RETURN VALUE
  DCompression == 0 : error, e.g. cannot convert
  DCompression >  0 : valid byte order value

AUTHOR
03-Mar-1998 PB Specification
  -------------------------------------------------------------------------*/
int String2Compression( const char * string )
{ int  NE=True;
  long i = 0;

  while ( (NE && DCompressionStrings[i]) )
    NE = compare_keys( string, DCompressionStrings[i++], UpperCaseSort );

  /* aliases */
  if (NE) { i=0;
    while ( (NE && DCompressionStringsAliases[i]) )
      NE = compare_keys(string,DCompressionStringsAliases[i++],UpperCaseSort);
    }

  /* aliases1 */
  if (NE) { i=0;
    while ( (NE && DCompressionStringsAliases1[i]) )
      NE = compare_keys(string,DCompressionStringsAliases1[i++],UpperCaseSort);
    }

  i = MAX(0,i-1);

  if (NE) return( InValidDCompression );
    else return( i );

} /* String2Compression */

/*---------------------------------------------------------------------------
NAME

  edf_dim_product --- returns the product of dim[] in all dimensions 

SYNOPSIS

  unsigned long edf_dim_product ( const long * dim );

DESCRIPTION

  The product of dim[nn] from nn=1 to dim[0] is returned as an unsigned
  long value. If one of the dim[nn] is zero or negative, 0 is returned.

RETURN VALUE
  unsigned long == 0 : error
  unsigned long  > 0 : product of dim[] of all dimensions 

AUTHOR
03-Mar-1998 PB Specification
  -------------------------------------------------------------------------*/
unsigned long edf_dim_product ( const long * dim )
{ unsigned long product;
  long   i;

  if ( dim ) {
    product = (unsigned long) 1;
    for (i=1;i<=dim[0];i++) {
      if (dim[i]<=0l) return(0lu);
      product *= (unsigned long) dim[i];
      }
    } else product = 0;
  return(product);

} /* edf_dim_product */

/*+++------------------------------------------------------------------------
NAME

  edf_data_sizeof --- returns the size of a data type element 

SYNOPSIS

  DType data_type;
  size_t edf_data_sizeof ( int data_type );

DESCRIPTION
  
  The size required for a data_type element is returned. Where data_type
  is the enum type DType.

RETURN VALUE
   NULL : error   (e.g. for invalid data type)
  >NULL : size of data_type

AUTHOR
03-Mar-1998 PB Specification
  --------------------------------------------------------------------------+*/
size_t edf_data_sizeof ( int data_type )                                /*---*/
{ 
  if (!((0<data_type) && (data_type<EndDType))) data_type = InValidDType;
  return( DSize[data_type] );

} /* edf_data_sizeof */

/*---------------------------------------------------------------------------
NAME

  get_data_type --- get data type from the symbol list of block

SYNOPSIS

  (DType) int get_data_type ( DBlock * block );

DESCRIPTION

  The data type is read from the symbol list of block and converted
  to the enumerated type DType, using the tables 'DTypeStrings' and
  'DTypeStringsAliases'.

  If data type is not found the default is FloatIEEE32.

RETURN VALUE
  long integer
  error   : 0
  DType   : positive number

AUTHOR
03-Mar-1998 PB Specification
  -------------------------------------------------------------------------*/
PRIVATE int get_data_type ( DBlock * block )
{ int data_type;
  SElement * symbol;

  if ( search_general( block, DATA_TYPE_KEY, &symbol ) )
    data_type = FloatIEEE32;
  else data_type = edf_string2datatype( symbol->String );

  return( data_type );

} /* get_data_type */

/*--------------------------------------------------------------------------
NAME

  get_data_dim --- get data dimension from the symbol list of block

SYNOPSIS

  long *get_data_dim ( DBlock * block );

DESCRIPTION

  The dimension values are read successively from the symbol list of block,
  starting with Dim_1 and stopping when Dim_nn does not exist. Memory for
  an array is allocated that contains { nn, Dim_1, Dim_2, ... , Dim_nn }.

  An array to store the dimensions is allocated. It must be freed explicitely.
  Attention, the result is not written to block.

  Keyword Dim_0 is ignored.

  If Dim_1 is not found an error is returned ((long *) NULL)

RETURN VALUE
  error   : (long *) NULL
  DIM[0]  : nn = number of dimensions
  DIM[1]  : number of elements in first dimension
  ...
  DIM[nn] : number of elements in nn-th dimension

AUTHOR
03-Mar-1998 PB Specification
15-May-2001 PB dim[n+1] is not longer used (as foreseen in edfio V1.37)
  -------------------------------------------------------------------------*/
PRIVATE long *get_data_dim ( DBlock * block )
{ char Key[MaxKeyLen+1];
  long dim, *data_dim, maxdim;
  long product;
  int errval;

  SElement *symbol;

  /* first run: determine dimension */
  dim = 1l;
  sprintf(Key,"%s%lu",DIMENSION_KEY_PREFIX,dim);
  while ( !search_general( block, Key, &symbol )) {
    dim++;
    sprintf(Key,"%s%lu",DIMENSION_KEY_PREFIX,dim);
  }
  dim--;

  data_dim = (long *) NULL;
  if ( dim > 0 ) {
    /* second run: store data_dim */    
    if ( !(data_dim = (long *) malloc( sizeof( long )*(dim+1) )) )
      return( (long *) NULL );
  
    product = 1l;
    data_dim[0] = maxdim = dim;
    for (dim = 1; dim<=maxdim; dim++) {
      sprintf(Key,"%s%lu",DIMENSION_KEY_PREFIX,dim);
      search_general( block, Key, &symbol );
      product *= data_dim[dim] = num_str2long( symbol->String, NULL, &errval ); 
      if (errval) {
        fprintf(stderr,"ERROR: Failed to read header value %s\n",Key); 
        free( data_dim ); return((long *) NULL);
      }
    }
  }

  return( data_dim ); 

} /* get_data_dim */

/*---------------------------------------------------------------------------
NAME

  get_byte_order --- get the byte order from the symbol list of block

SYNOPSIS

  (BOrder) int get_byte_order ( DBlock * block );

DESCRIPTION

  The byte order is read from the symbol list of block and converted
  to the enumerated type BOrder, using the table 'BOrderStrings'.

  If byte order is not found the default is HighByteFirst.

RETURN VALUE
  integer
  error   : 0
  BOrder  : positive number

AUTHOR
03-Mar-1998 PB Specification
  ---------------------------------------------------------------------------*/
int get_byte_order ( DBlock * block )
{ int byte_order;
  SElement * symbol;

  if ( search_general( block, BYTE_ORDER_KEY, &symbol ) )
    byte_order = HighByteFirst;
  else byte_order = String2ByteOrder( symbol->String );

  return( byte_order );

} /* get_byte_order */

/*---------------------------------------------------------------------------
NAME

  get_compression --- get the compression type from the symbol list of block

SYNOPSIS

  (DCompression) int get_compression ( DBlock * block );

DESCRIPTION

  The compression is read from the symbol list of block and converted
  to the enumerated type DCompression, using the table 'DCompressionStrings'
  and 'DCompressionStringsAliases'.

  If compresssion is not found the default is UnCompressed.

RETURN VALUE
  integer
  error        : 0
  DCompression : positive number

AUTHOR
03-Mar-1998 PB Specification
  --------------------------------------------------------------------------*/
int get_compression ( DBlock * block )
{ int compression;
  SElement * symbol;

  if ( search_general( block, COMPRESSION_KEY, &symbol ) )
    compression = UnCompressed;
  else compression = String2Compression( symbol->String );

  return( compression );

} /* get_compression */

/*---------------------------------------------------------------------------
NAME

  get_data_value_offset--- get data_value_offset from the symbol list of block

SYNOPSIS

  long int get_data_value_offset ( DBlock * block );

DESCRIPTION

  The data_value_offset is read from the symbol list of block and converted
  to long integer. 

  If data_value_offset is not found the default is 0l. 

RETURN VALUE
  long integer

AUTHOR
23-Jul-1999 PB Specification (EDF_DataFormatVersion 2.20)
  --------------------------------------------------------------------------*/
long int get_data_value_offset ( DBlock * block )
{ long int data_value_offset; 
  SElement * symbol;
  int errval;

  if ( search_general( block, DATA_VALUE_OFFSET_KEY, &symbol ) )
    data_value_offset = 0l;
  else {
    data_value_offset = num_str2long( symbol->String, NULL, &errval );
    if (errval) {
      fprintf(stderr,"WARNING: Failed to convert header value %s = %s\n",
             DATA_VALUE_OFFSET_KEY, symbol->String );
      fprintf(stderr,"         Using %s = 0\n",DATA_VALUE_OFFSET_KEY);
      data_value_offset = 0l;
    }
  }

  return( data_value_offset );

} /* get_data_value_offset */

/*---------------------------------------------------------------------------
NAME

  get_raster_configuration --- get the raster configuration number

SYNOPSIS

  long get_raster_configuration ( DBlock * block );

DESCRIPTION

  The raster_configuration is read from the symbol list of block and 
  returned as a long integer value.

  If raster_configuration is not found the default is 1.

RETURN VALUE
  long integer
  error        : 0
  DCompression : positive number

AUTHOR
14-Mar-1998 PB Specification
  ---------------------------------------------------------------------------*/
long get_raster_configuration ( DBlock * block )
{ long raster_configuration;
  SElement * symbol;
  int errval;

  if ( search_general( block, RASTER_CONFIGURATION_KEY, &symbol ) )
    raster_configuration = 1l;
  else {
    raster_configuration = num_str2long( symbol->String, NULL, &errval );
    if (errval) {
      fprintf(stderr,"WARNING: Failed to convert header value %s = %s\n",
             RASTER_CONFIGURATION_KEY, symbol->String); 
      fprintf(stderr,"         Using %s = 1\n",RASTER_CONFIGURATION_KEY);
      raster_configuration = 1l;
    }
  }

  return( raster_configuration );

} /* get_raster_configuration */
 
/*---------------------------------------------------------------------------
NAME

  get_binary_array --- read data array (decompressed, with endian correction)

SYNOPSIS

  int get_binary_array ( DBlock * block, void * buffer, size_t buflen, 
                         int must_use_buffer );


DESCRIPTION
  The binary data of 'block' is read from disk. 'block' must have been 
  opened with 'open_read_block' and the header must be in memory.  

  If buffer is the NULL-pointer a memory section is internally allocated
  for the data. It has the size

     DataLen = PRODUCT('DataDim_nn')[nn=1,NN]*sizeof('DataType') bytes

  Otherwise, if 'buffer' is not the NULL-pointer and 'must_use_buffer' 
  is set (True) the read data will be written to 'buffer'.
  If the buffer length (buflen) is too short to read all data the routine 
  exits with an error (return value -1).
  If 'buffer' is not the NULL-pointer and 'must_use_buffer' is not set 
  (False) the read data will be written to 'buffer' only if the buffer
  length (buflen) is sufficiently large to read all data, otherwise
  a memory section is allocated like in the case where 'buffer' is the 
  NULL-pointer.

  'DataDim_nn' stands for the nn-th dimension and NN for the 
  dimensionality of the array. PRODUCT indicates the product over all 
  array dimensions Dim_nn from nn=1 to NN. 

  The data are read like they are on stored. No transformation is applied.

  The following key words are read

  Dim_1, Dim_2, ... (default 1, 0, ...)
  DataType (default FloatIEEE32)
  ByteOrder (default HighByteFirst)
  Compression (default None)
  DataValueOffset (default 0)

  The following parameter are updated in 'block':
  block->Data = pointer to the allocated data buffer
  block->DataBufferLen = length of the data buffer in bytes
  block->DataLen = length of the meaningful data in bytes
  block->DataType = DataType of a single data element in the data buffer
  block->DataByteOrder = ByteOrder of a single data element in the data buffer
  block->DataRasterConfiguration = RasterConfiguration of the data array
  block->DataCompression = Compression type of the data in the data array 
  block->DataValueOffset = offset to be added to each data item
 *block->DataDim = pointer to the allocated dimension array
                   with DataDim[0] = NN.

DATA SOURCE 
  According to 'block->BinaryFileName' the binary data is read from the channel
  of the main file ('block->Chain->File->Channel') or from an external file.
  The file pointer is in both cases positioned at 'block->BinaryFilePos' of
  the data file.

  block->BinaryFileName   input channel

  (char *) NULL           block->Chain->File->Channel 
  <filename>              open <filename>

RETURN VALUE
  success :  0
  error   : -1

AUTHOR
03-Mar-1998 PB Specification
29-Dec-2000 PB buffer, buflen, test block->Flags.DataExternal
30-Mar-2001 PB alt_binaryname
  ---------------------------------------------------------------------------*/
int get_binary_array ( DBlock * block, void * buffer, size_t buflen, 
                       int must_use_buffer )
{ static const char * GBA_Error = "ERROR: get_binary_array:" ;

  int           i;
  int    intern = True;

  char         *alt_binaryname;

  long         *data_dim;
  int           data_type;
  int           byte_order;
  int           compression;
  long          data_value_offset;
  long          raster_configuration;

  size_t   data_size, data_len, data_buffer_len, data_read_len;
  unsigned long   data_pos, data_number;

  FILE     *channel; 

  /* check whether 'block' exists */
  if (!block) return(-1); 

  /* get binary data description */
  data_dim    = get_data_dim ( block ); /* is free'ed when block is free'ed */
  if (!data_dim) return(-1); // MissingArrayDimensions
  data_type   = get_data_type ( block );
  byte_order  = get_byte_order ( block );
  compression = get_compression ( block );
  data_value_offset = get_data_value_offset ( block );
  raster_configuration = get_raster_configuration ( block );

  data_size   = edf_data_sizeof ( data_type );
  data_number = edf_dim_product ( data_dim ) ;
  data_len    = data_size * data_number ;
  data_read_len = data_len;
  if (block->BinaryFileName) {
    data_pos = block->BinaryFilePos;
    // data_read_len <= BinaryFileLen
    if (block->BinaryFileLen) // only if BinaryFileLen is set
      data_read_len = MIN(data_read_len,block->BinaryFileLen);
  } else {
    data_pos = block->BinaryPos; 
    // data_read_len <= BinaryLen
    data_read_len = MIN(data_read_len,block->BinaryLen);
  }

  if (EDFIO_debug) {
    printf(" ---- get_binary_array BEGIN\n");
    printf(" block                = %p\n", block);
    printf(" buffer               = %p\n", buffer);
//    printf(" buflen               = %zu\n", buflen);
    printf(" buflen               = %zu | %lu\n", buflen, buflen);
    printf(" data_dim[0]          = %ld\n", data_dim[0]);
    for (i=1;i<=data_dim[0];i++) 
      printf("  data_dim[%d]         = %ld\n", i, data_dim[i]);
    printf(" data_type            = %d\n", data_type);
    printf(" byte_order           = %d\n", byte_order);
    printf(" compression          = %d\n", compression);
    printf(" data_value_offset    = %ld\n", data_value_offset);
    printf(" raster_configuration = %ld\n", raster_configuration);
//    printf(" data_size            = %zu\n", data_size);
    printf(" data_size            = %zu | %lu\n", data_size, data_size);
    printf(" data_number          = %ld\n", data_number);
//    printf(" data_len             = %zu\n", data_len);
    printf(" data_len             = %zu | %lu\n", data_len, data_len);
//    printf(" data_read_len        = %zu\n", data_read_len);
    printf(" data_read_len        = %zu | %lu\n", data_read_len, data_read_len);
    printf(" data_pos             = %ld\n", data_pos);
  } /* EDFIO_debug */

  /* update DataDim array in block */
  if (block->DataDim){ // copy dimensions into existing array, release data_dim
    if ( !(copydim( block->DataDim, block->DataDim[0], data_dim ) ) ) {
       free(data_dim); fprintf(stderr,"%s copydim\n",GBA_Error); return(-1); }
    free(data_dim);
  } else { // use allocated buffer
    block->DataDim  = data_dim;
  }

  if (block->BinaryFileName) {
    /* open external source read only */
    if (EDFIO_debug) printf(" BinaryFileName = %s\n",block->BinaryFileName);
    if (!(channel = fopen( block->BinaryFileName,"rb"))) {
      if (!has_extension ( block->BinaryFileName )) {
        /* add an empty extension */
        if (!(alt_binaryname = catstr( block->BinaryFileName, "." ))) {
          fprintf(stderr,"%s fopen 1\n",GBA_Error); return(-1); }
        channel = fopen( alt_binaryname,"rb"); 
        free( alt_binaryname );
        if (!(channel)) { fprintf(stderr,"%s fopen 2\n",GBA_Error); return(-1);}
      } else { fprintf(stderr,"%s fopen\n",GBA_Error); return(-1); }
    }
    intern = False;
  } else channel = block->Chain->File->Channel;

  /* search data start position */
  if ( fseek( channel, data_pos, SEEK_SET) ) {
      fprintf(stderr,"%s fseek\n",GBA_Error); return(-1); }

  // do not use buffer if it is equal to block->Data
  if ( (block->Data == buffer) && (!must_use_buffer) ) {
    buffer = (void *) NULL; buflen = (size_t) 0; }

  // unlink externally allocated memory or release internally allocated memory
  if (block->Flags.ExternalDataAlloc) {
    block->Data = (void *) NULL;
    block->DataBufferLen = (unsigned long) 0;
    block->DataLen = (unsigned long) 0;
    block->Flags.ExternalDataAlloc = False;
    } else {
    if ( ( (block->Data) && (block->DataBufferLen<data_len) ) ||
         (must_use_buffer) ) {
      free(block->Data); block->Data = (void *) NULL;
      block->DataBufferLen = (unsigned long) 0;
      block->DataLen = (unsigned long) 0;
      }
    }                                                                                                                                       

  if ( (buffer == (void *) NULL) || 
       ((buflen<data_len)&&(!must_use_buffer)) ) {
    /* allocate memory, if necessary */
    if (EDFIO_debug) printf(" Allocate new memory, if necessary\n");
    if (block->DataBufferLen<data_len)
      if (!(block->Data = malloc( data_len ))) {
        fprintf(stderr,"%s malloc\n",GBA_Error); return(-1); }
    data_buffer_len = data_len;
    block->Flags.ExternalDataAlloc = False;
    } else {
    if (EDFIO_debug) printf(" Use supplied buffer\n");
    if (buflen<data_len) {
//      fprintf(stderr,"%s supplied buffer < %zu bytes\n",GBA_Error,data_len); 
      fprintf(stderr,"%s supplied buffer < %zu | %lu bytes\n",GBA_Error,data_len,data_len); 
      return(-1); }
    block->Flags.ExternalDataAlloc = True;
    block->Data = buffer;
    data_buffer_len = buflen;
    }

  /* read data */
  if ( fread(block->Data,1,data_read_len,channel) < data_read_len ) {
    fprintf(stderr,"%s fread\n",GBA_Error); return(-1); }

  if (data_len-data_read_len>0) {
//    fprintf(stderr,"%s binary block/file size %zu smaller than array size %zu\n",
    fprintf(stderr,"%s binary block/file size %zu | %lu smaller than array size %zu | %lu\n",
      GBA_Error, data_read_len, data_read_len, data_len, data_len ); return(-1); }
    
  /* close external file */
  if ( !intern )
    if (fclose( channel )) {
      fprintf(stderr,"%s fclose\n",GBA_Error); return(-1); }

  /* update block */
  block->DataLen  = data_len; 
  block->DataBufferLen = data_buffer_len;
  block->DataType = data_type;
  block->DataByteOrder = byte_order;
  block->DataRasterConfiguration = raster_configuration;
  block->DataCompression = compression;
  block->DataValueOffset = data_value_offset;

  block->Flags.DataExternal = False; /* data are read from file */

  if (EDFIO_debug) printf(" ---- get_binary_array END\n");

  return(0);

} /*  get_binary_array */

/*===get_binary_array END================================================---*/

/****************************************************************************/
/*---------------------------------------------------------------------------
NAME

  find_chainkey --- returns ChainKey for DataChain

SYNOPSIS

  int find_chainkey ( int stream, int DataChain, char ChainKey[] )

DESCRIPTON
Searches a chain key definition for DataChain and copies it to ChainKey.
ChainKey must be sufficiently long (MaxKeyLen+1). First, it searches a 
chain key definition in the general block, then it takes the default
chain key. DataChain must be zero or a positive number.

RETURN VALUE
  0 : success 
 -1 : failed
---------------------------------------------------------------------------*/
int find_chainkey ( int stream, int DataChain, char ChainKey[] )
{
  SElement *symbol;
  DFile    *file = &FileTable[stream];

  char ChainKeyDefinition[MaxKeyLen+1];

  if (DataChain==0) {
    strncpy(ChainKey,GENERAL_CHAIN_KEY,MaxKeyLen+1);
    } else {
    /* test, if general block exists */
    if (file->GeneralBlock == (DBlock *) NULL) return(-1);
    /* search chainkey in general block */
    sprintf(ChainKeyDefinition,"%s%u",CHAIN_KEY_DEFINITION,DataChain);
    if (!( search_symbol( file->GeneralBlock,
                        ChainKeyDefinition, &symbol ) ) ) { 
      strncpy(ChainKey,symbol->Value,MaxKeyLen+1);
      } else { if (!(default_chain_key(ChainKey, DataChain))) return(-1); }
    }

  return(0);

} /* find_chainkey */

/*---------------------------------------------------------------------------
find_blockkey (found : 0, failed : -1)
BlockKey must be sufficiently long (MaxKeyLen+1)
---------------------------------------------------------------------------*/
int find_blockkey ( int DataChain, int DataNumber, char BlockKey[] )
{
  if ( DataChain==0 ) strncpy(BlockKey,GENERAL_BLOCK_KEY,MaxKeyLen+1);
    else long2s(BlockKey,DataNumber);
  return(0);

} /* find_blockkey */

/*---------------------------------------------------------------------------
NAME

  disk_write_block

SYNOPSIS

  int disk_write_block ( DBlock * block, int * pErrorValue, int * pstatus );

DESCRIPTION
  If 'block' exists header and binary data are written to disk.
  The symbol list and the link to the data are removed. DataLen 
  is set to 0. The data block flags DiskBlockUsed, DiskBlockFixed,
  HeaderExternal and DataExternal are set to True, HeaderChanged 
  and DataChanged are set to False. Before reusing this data block 
  it must be read back from the file.

INPUT VALUES
  DBlock * block;

OUTPUT VALUES
  *pErrorValue, *pstatus

MODIFIED
  Block is written to the output file and afterwards the symbol list 
  is removed from block. The data specification is cleared and the
  dynamic arrays (Data and DataDim) are released if they were not 
  externally allocated. Externally allocated arrays must be released
  separately.

RETURN VALUE
  In case of success the returned value is 1 otherwise 0.
  The return value should not be used to check for an error, the returned
  value of status should be used. In case of success the *pstatus is 0,
  otherwise not. A more precise description of the error is returned in
  *pErrorValue.
  --------------------------------------------------------------------------*/
int disk_write_block ( DBlock * block, int * pErrorValue, int * pstatus )
{
  DFile  * file;

  *pstatus = status_error;

  if ( block ) {
    file = block->Chain->File;
    if ( put_data_block( block ) ) {
      *pErrorValue = WriteDataError; return(0);
      }

    /* free symbol list only if InternalHeader is not set 
       and if block is not the general block */
    if (( !block->Flags.InternalHeader ) && (block != file->GeneralBlock)) {
      if ( free_symbol_list( block ) ) {
        *pErrorValue = CouldNotFreeHeaders; return(0);
        }
      /* update data block flags for header*/
      block->Flags.HeaderExternal = True;
      }

    /* unlink data only if InternalData is not set
       and if block is not the general block */
    if (( !block->Flags.InternalData ) && (block != file->GeneralBlock)) {

      /* clear data specification and release dynamic arrays */
      if ( (!block->Flags.ExternalDataAlloc) && (block->Data) ) 
        free( block->Data );
      block->Data            = NULL;
      block->DataLen         = (size_t) 0;
      block->DataType        = InValidDType;
      block->DataValueOffset = 0l;
      block->DataByteOrder   = InValidBOrder;
      block->DataRasterConfiguration = 0l;
      block->DataCompression = InValidDCompression;
      if ( (!block->Flags.ExternalDimAlloc) && (block->DataDim) ) 
        free( block->DataDim );

      block->DataDim = (long*) NULL;

      /* update data block flags for data*/
      block->Flags.DataExternal      = True;
      block->Flags.ExternalDataAlloc = False;
      block->Flags.ExternalDimAlloc  = False;
      }

    } /* if (block) ... */

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* disk_write_block */

/*---------------------------------------------------------------------------
NAME

  disk_read_header

SYNOPSIS

  int disk_read_header ( DBlock * block, int * pErrorValue, int * pstatus );

DESCRIPTION
  The header of 'block' is read from disk. 

INPUT VALUES
  DBlock * block;

OUTPUT VALUES
  *pErrorValue, *pstatus

RETURN VALUE
  success: 1
  otherwise: 0
  The return value should not be used to check for an error, the returned
  value of status should be used. In case of success *pstatus is 0,
  otherwise not. A more precise description of the error is returned in
  *pErrorValue.
  --------------------------------------------------------------------------*/
int disk_read_header ( DBlock * block, int * pErrorValue, int * pstatus )
{
  *pErrorValue = RoutineSucceeded;
  *pstatus = status_error;

  if ( get_data_header( block ) ) { *pErrorValue = ReadDataError; return(0); }

  *pstatus = status_success;

  return(1);

} /* disk_read_header */

/*---------------------------------------------------------------------------
NAME
      open_write_block - open a single write to a data block

SYNOPSIS

      int open_write_block( int stream, long int DataNumber, int DataChain,
                      DBlock ** pblock, int * pErrorValue, int * pstatus );


DESCRIPTION
  Opens a single write to the data_block file.
  Searches for header 'DataNumber' in 'DataChain' and returns pblock.
  If it does not exists, it is created. 'DataChain' zero is the
  general block. It must be at the beginning of the file.

  In case of success the return value is 1, otherwise 0.

RETURN VALUE
  In case of success the returned value is 1 otherwise 0.
  The return value should not be used to check for an error, instead
  the returned value of status should be used. In case of success 
  the *pstatus is 0, otherwise not. A more precise description of the 
  error is returned in *pErrorValue.

STATUS
  OK
  --------------------------------------------------------------------------*/
int open_write_block( int stream, long int DataNumber, int DataChain,
                      DBlock ** pblock, int * pErrorValue, int * pstatus )
{
  DFile    * file = &FileTable[stream];
  DChain   * chain;
  DBlock   * block;

  char BlockKey[MaxKeyLen+1];
  char ChainKey[MaxKeyLen+1];

  *pstatus = status_error;

  /* GeneralBlock can only be modified when active */ 
  if (DataChain==0)
    if (file->GeneralBlock!=file->ActiveBlock) {
    *pErrorValue=GeneralBlockNotFirst; return(0);
    }

  /* get Channel */
  if ( !file->Used ) {
    *pErrorValue = NoFileOpen; return(0);
    }

  /* find 'ChainKey' */
  if ( find_chainkey ( stream, DataChain, ChainKey ) ) {
    *pErrorValue = MissingKeyDefinition; return(0);
    }

  /* find 'BlockKey' */
  if ( find_blockkey ( DataChain, DataNumber, BlockKey ) ) {
    *pErrorValue = MissingKeyDefinition; return(0);
    }

  /* insert header */
  if (insert_data_chain( file, ChainKey, &chain) ) {
    *pErrorValue = CouldNotInsertChain; return(0); }
  if (insert_data_block( chain, BlockKey, &block) ) {
    *pErrorValue = CouldNotInsertBlock; return(0); }

  /* update data block flags */
  if ( file->Flags.TemporaryFile ) {
    block->Flags.InternalHeader = True;
    block->Flags.InternalData   = True;
    block->Flags.HeaderExternal = False;
    block->Flags.DataExternal   = False;
    }

  /* flush previously active block */
  if ( file->ActiveBlock != block) {
    /* flush ActiveBlock */
    disk_write_block ( block->Chain->File->ActiveBlock, pErrorValue, pstatus );
    if ( *pstatus != status_success )  return(0);
    /* update ActiveBlock */
    file->ActiveBlock = block;
    }

  *pblock = block;

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* open_write_block */

/*---------------------------------------------------------------------------
NAME
      close_write_block - close a single write to a data block 

SYNOPSIS

      int close_write_block(DBlock * block, int * pErrorValue, int * pstatus);

DESCRIPTION
  Closes a single write to the data_block, currently a dummy routine.
  
RETURN VALUE
  In case of success the returned value is 1 otherwise 0. 
  The return value should not be used to check for an error, the returned
  value of status should be used. In case of success the *pstatus is 0,
  otherwise not. A more precise description of the error is returned in
  *pErrorValue. 
  --------------------------------------------------------------------------*/
int close_write_block( DBlock * block, int * pErrorValue, int * pstatus )
{
  DFile  * file;
  DChain * chain;

  // int  GeneralBlock = is_general_block ( block ); // unused

  chain   = block->Chain;
  file    = chain->File;

  *pstatus = status_error;

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* close_write_block */

/*---------------------------------------------------------------------------
NAME
      open_read_block - open a single read from a data block

SYNOPSIS

      int open_read_block(int stream, long int DataNumber, int DataChain,
                      DBlock ** pblock, int * pErrorValue, int * pstatus );

DESCRIPTION
  Opens a single read from the data_block
  file. In case of success the return value is 1, otherwise 0.

RETURN VALUE
  In case of success the returned value is 1 otherwise 0.
  The return value should not be used to check for an error, the returned
  value of status should be used. In case of success the *pstatus is 0,
  otherwise not. A more precise description of the error is returned in
  *pErrorValue.

STATUS
  return value FALSE  : 'DataNumber' or 'DataChain' not found and no other 
                         error
                         *pstatus = status_error;
                         *pErrorValue=(CouldNotFindHeader, RoutineSucceeded);
                TRUE  : data header found or error,
                         *pstatus = Success or status_error;
                         *pErrorValue = <any>
  --------------------------------------------------------------------------*/
int open_read_block( int stream, long int DataNumber, int DataChain,
                      DBlock ** pblock, int * pErrorValue, int * pstatus )
{
  DFile    * file;
  DChain   * chain;
  DBlock   * block;

  char BlockKey[MaxKeyLen+1];
  char ChainKey[MaxKeyLen+1];

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  /* check stream */
  if ((stream<0)||(stream>=MaxFiles)) {
    *pErrorValue = InvalidStream; return(1);
    }
  /* get file */
  file = &FileTable[stream];
  if ( !file->Used ) {
    *pErrorValue = NoFileOpen; return(1);
    }

  /* find 'ChainKey' */

  if ( find_chainkey ( stream, DataChain, ChainKey ) ) {
    *pErrorValue = MissingKeyDefinition; return(1);
    }

  /* find 'BlockKey' */
  if ( find_blockkey ( DataChain, DataNumber, BlockKey ) ) {
    *pErrorValue = MissingKeyDefinition; return(1);
    }

  /* search header */
  if (search_data_chain( file, ChainKey, &chain) ) {
    *pErrorValue = CouldNotFindHeader; return(0); }
  if (search_data_block( chain, BlockKey, &block) ) {
    *pErrorValue = CouldNotFindHeader; return(0); }

  /* update data block flags */
  if ( file->Flags.TemporaryFile ) {
    block->Flags.InternalHeader = True;
    block->Flags.InternalData   = True; 
    block->Flags.HeaderExternal = False;
    block->Flags.DataExternal   = False;
    }

  /* flush previously active block and get block from disk */
    if ( file->ActiveBlock != block) {
      /* flush ActiveBlock */
      disk_write_block( file->ActiveBlock, pErrorValue, pstatus );
      if ( *pstatus != status_success )  return(1);
      /* read block from disk */
      disk_read_header ( block, pErrorValue, pstatus );
      if ( *pstatus != status_success ) return(1);
      /* update ActiveBlock */
      file->ActiveBlock = block;
      }

  *pblock = block;

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* open_read_block */

/*---------------------------------------------------------------------------
NAME
      close_read_block - close a single read from a data block

SYNOPSIS

      int close_read_block(DBlock * block, int * pErrorValue, int * pstatus);

DESCRIPTION
  Closes a single read from a data block file. In case of success the 
  return value is 1.

RETURN VALUE
  In case of success the returned value is 1 otherwise 0.
  The return value should not be used to check for an error, the returned
  value of status should be used. In case of success  *pstatus is 0,
  otherwise not. A more precise description of the error is returned in
  *pErrorValue.

STATUS
  In preparation
  --------------------------------------------------------------------------*/
int close_read_block( DBlock * block, int * pErrorValue, int * pstatus )
{
  DFile  * file;
  DChain * chain;

  // int  GeneralBlock = is_general_block ( block ); // unused

  chain   = block->Chain;
  file    = chain->File;

  *pstatus = status_error;

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* close_read_block */

/*---------------------------------------------------------------------------
NAME

  read_header_string --- read a string from the header 

DESCRIPTION
  Searches for 'keyword' in the header 'DataNumber' in 'DataChain'.
  If the header or the keyword does not exists, the return value is 0 and a
  specific error value is returned. This error is not fatal and can be used
  as a test for the existence of the keyword or the header. 
  The returned argument *pString is a pointer to a location that contains
  the string of the element 'keyword'. 
  This string is not accesible any more when the file is closed or if
  the file header is not kept in memory when another block is accessed.

RETURN VALUE
  return value 0 if not found and no other error
  return( int ) 0 : data header not found or keyword not found
                         *pstatus = status_error;
                         *pErrorValue= ( CouldNotFindHeader, 
                                         CouldNotFindSymbol,
                                         RoutineSucceeded );
                1  : data header found or error,
                         *pstatus = Success or status_error;
                         *pErrorValue = <any>
  --------------------------------------------------------------------------+*/
int read_header_string ( int stream, long int DataNumber, int DataChain,
                        const char * keyword, const char ** pString,
                        int * pErrorValue, int * pstatus )              /*---*/
{
  DBlock * block;
  SElement * symbol;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("read_header_string");

  if ( !open_read_block (stream, DataNumber, DataChain,
                     &block, pErrorValue, pstatus) ) {
    /* data header not found */
    if (EDFIO_debug) printf("\n"); return(0); }
  if ( *pstatus != status_success )  return(1);

  /* read keyword value from symbol list */
  if (search_general( block, keyword, &symbol) ) {
    if (EDFIO_debug) printf(" %s missing;\n",keyword); 
    *pstatus     = status_error;
    *pErrorValue = CouldNotFindSymbol; return(0); }

  close_read_block( block, pErrorValue, pstatus );
  if ( *pstatus != status_success )  return(1);

  *pString = symbol->String;

  if (EDFIO_debug) printf(" %s = \"%s\";\n",keyword,*pString);

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* read_header_string */

/*---------------------------------------------------------------------------
NAME

  write_header_string --- write character string into header 

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  If it does not exists, it is created. It writes keyword and value with 
  insert_string into the header. This routine writes, if necessary, the 
  input string with several continuation keys into the header.

RETURN VALUE
  In case of success the return value is 1, otherwise 0.
  --------------------------------------------------------------------------+*/
int write_header_string ( int stream, long int DataNumber, int DataChain,
                        const char * keyword, const char *String,
                        int * pErrorValue, int * pstatus )              /*---*/
{
  DBlock * block;
  SElement * symbol;

  if (EDFIO_debug) printf("write_header_string %s = \"%s\";\n",keyword,String);

  open_write_block (stream, DataNumber, DataChain,
                    &block, pErrorValue, pstatus);
  if ( *pstatus != status_success )  return(0);

  /* create/replace keyword and value in header */
  if (insert_string( block, keyword, String, &symbol) ) {
    *pstatus = status_error;
    *pErrorValue = CouldNotInsertSymbol; return(0); }

  close_write_block( block, pErrorValue, pstatus );
  if ( *pstatus != status_success )  return(0);

  /* The header was changed and must be written to disk */
  block->Flags.HeaderChanged = True;

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* write_header_string */

/*===machinetype BEGIN====================================================*/

/*+++ public interface of machinetypes------------------------------------

 DESCRIPTION
 The function edf_datatype2machinetype returns a machine type for data type.
 If a machine type does not exist to a given data type the return value
 is zero.
 If MachineTypeInit is set, MachineType contains a translation table from
 DType to MType, e.g mtype = MachineType[dtype]. 

 PUBLIC enum MType; // defined in edfio.h
 PUBLIC const char * MachineType2String ( int mtype );
 PUBLIC int String2MachineType( const char * string );
 PUBLIC int edf_datatype2machinetype ( int dtype );
 PUBLIC int edf_machinetype2datatype ( int mtype );
 PUBLIC size_t edf_machine_sizeof( int mtype );
 PUBLIC void edf_showdatatypes ( int full );
 PUBLIC void edf_showmachinetypes ( int full );

  ----------------------------------------------- end of machinetypes ---*/

# define Unsigned8Max    255UL
# define Unsigned8Min    0UL
# define Unsigned16Max   65535UL
# define Unsigned16Min   0UL
# define Unsigned32Max   4294967295UL
# define Unsigned32Min   0UL
# if __WORDSIZE == 64
#   define Unsigned64Max 18446744073709551615UL
# else
#   define Unsigned64Max 0UL
# endif
# define Unsigned64Min 0UL

# define Signed8Max      127L
# define Signed8Min      ( -Signed8Max -1L )
# define Signed16Max     32767L
# define Signed16Min     ( -Signed16Max -1L )
# define Signed32Max     2147483647L
# define Signed32Min     ( -Signed32Max -1L )
# if __WORDSIZE == 64
#   define Signed64Max   9223372036854775807L
# else
#   define Signed64Max   0UL
# endif
# define Signed64Min     (-Signed64Max - 1L)

PRIVATE unsigned long UCharMin, UCharMax, UShortMin, UShortMax;
PRIVATE unsigned long UIntMin,  UIntMax,  ULongMin,  ULongMax;
PRIVATE long          CharMin,  CharMax,  ShortMin,  ShortMax;
PRIVATE long          IntMin,   IntMax,   LongMin,   LongMax;

PRIVATE int MachineType[EndDType];
PRIVATE int DataType[EndMType];

PRIVATE int MachineTypeInit = 0;    // not initialized

const char * MTypeStrings[EndMType+1] =
           { "Invalid",
             "unsigned char",    "char",      "unsigned short",   "short", 
             "unsigned int",     "int",       "unsigned long",    "long",
             "float",            "double",
             (const char *) NULL };

/*+++------------------------------------------------------------------------
NAME

  edf_machine_sizeof --- returns the size of a machine data type element

SYNOPSIS
  MType mtype;
  size_t edf_machine_sizeof( int mtype )

DESCRIPTION
  The size required for a MType element is returned. 

RETURN VALUE
   NULL : error   (e.g. for invalid mtype)
  >NULL : size of mtype

AUTHOR
30-Apr-2000 PB 
  --------------------------------------------------------------------------+*/
size_t edf_machine_sizeof( int mtype )
{ switch ( mtype ) {
  case MUnsignedChar    : return ( sizeof( unsigned char ) );
  case MChar            : return ( sizeof( char ) );
  case MUnsignedShort   : return ( sizeof( unsigned short ) );
  case MShort           : return ( sizeof( short ) );
  case MUnsignedInteger : return ( sizeof( unsigned int ) );
  case MInteger         : return ( sizeof( int ) );
  case MUnsignedLong    : return ( sizeof( unsigned long ) );
  case MLong            : return ( sizeof( long ) );
  case MFloat           : return ( sizeof( float ) );
  case MDouble          : return ( sizeof( double ) );
  }; 
  return ( (size_t) 0 );
} /* edf_machine_sizeof */

/*---------------------------------------------------------------------------
NAME
  MachineType2String ---  returns mtype as string

SYNOPSIS
  const char * MachineType2String ( int mtype )

DESCRIPTION
  shows internal data types
---------------------------------------------------------------------------*/
const char * MachineType2String ( int mtype )
{ if ((mtype<0)||(mtype>=EndMType)) mtype = InValidMType;
  return( MTypeStrings[mtype] );
} /* MachineType2String */

/*---------------------------------------------------------------------------
NAME

  String2MachineType --- converts a string to MType 

SYNOPSIS

  (MType) int String2MachineType( const char * string );

DESCRIPTION

GCC AND G++
  For compatibility between gcc and g++ the declaration of the return
  value with its enumerated type has been replaced by "int".

RETURN VALUE
  MType == 0 : error, e.g. cannot convert
  MType >  0 : valid machine type value

AUTHOR
30-Apr-2000 PB from edf_string2datatype 
  --------------------------------------------------------------------------*/
int String2MachineType( const char * string )
{ int  NE=True;
  long i = 0;

  while ( (NE && MTypeStrings[i]) )
    NE = compare_keys( string, MTypeStrings[i++], UpperCaseSort );

  i = MAX(0,i-1);

  if (NE) return( InValidMType );
    else return( i );

} /* String2MachineType */

/*---------------------------------------------------------------------------
NAME
   initmachinetypes

SYNOPSIS
   void initmachinetypes ( void )

DESCRIPTION
  inits machine type array
---------------------------------------------------------------------------*/
void initmachinetypes ( void )
{ int i;
  int * machinetype = MachineType;
  int * datatype    = DataType;

  for (i=0;i<EndDType;i++) machinetype[i] = InValidMType;
  for (i=0;i<EndMType;i++) datatype[i]    = InValidDType;

  // unsigned char
  switch (sizeof( unsigned char ) ) {
     case 1 : machinetype[Unsigned8]      = MUnsignedChar;
              datatype[MUnsignedChar]     = Unsigned8;
              UCharMin                    = Unsigned8Min;
              UCharMax                    = Unsigned8Max;
              break;
     case 2 : machinetype[Unsigned16]     = MUnsignedChar;
              datatype[MUnsignedChar]     = Unsigned16;
              UCharMin                    = Unsigned16Min;
              UCharMax                    = Unsigned16Max;
              break;
     }

  // unsigned short int
  switch (sizeof( unsigned short int ) ) {
     case 1 : machinetype[Unsigned8]      = MUnsignedShort;
              datatype[MUnsignedShort]    = Unsigned8;
              UShortMin                   = Unsigned8Min;
              UShortMax                   = Unsigned8Max;
              break;
     case 2 : machinetype[Unsigned16]     = MUnsignedShort;
              datatype[MUnsignedShort]    = Unsigned16;
              UShortMin                   = Unsigned16Min;
              UShortMax                   = Unsigned16Max;
              break;
     case 4 : machinetype[Unsigned32]     = MUnsignedShort;
              datatype[MUnsignedShort]    = Unsigned32;
              UShortMin                   = Unsigned32Min;
              UShortMax                   = Unsigned32Max;
              break;
     }

  // unsigned int
  switch (sizeof( unsigned int ) ) {
     case 1 : machinetype[Unsigned8]      = MUnsignedInteger;
              datatype[MUnsignedInteger]  = Unsigned8;
              UIntMin                     = Unsigned8Min;
              UIntMax                     = Unsigned8Max;
              break;
     case 2 : machinetype[Unsigned16]     = MUnsignedInteger;
              datatype[MUnsignedInteger]  = Unsigned16;
              UIntMin                     = Unsigned16Min;
              UIntMax                     = Unsigned16Max;
              break;
     case 4 : machinetype[Unsigned32]     = MUnsignedInteger;
              datatype[MUnsignedInteger]  = Unsigned32;
              UIntMin                     = Unsigned32Min;
              UIntMax                     = Unsigned32Max;
              break;
     case 8 : machinetype[Unsigned64]     = MUnsignedInteger;
              datatype[MInteger]          = Unsigned64;
              UIntMin                     = Unsigned64Min;
              UIntMax                     = Unsigned64Max;
              break;
     }

  // unsigned long
  switch (sizeof( unsigned long ) ) {
     case 1 : machinetype[Unsigned8]      = MUnsignedLong;
              datatype[MUnsignedLong]     = Unsigned8;
              ULongMin                    = Unsigned8Min;
              ULongMax                    = Unsigned8Max;
              break;
     case 2 : machinetype[Unsigned16]     = MUnsignedLong;
              datatype[MUnsignedLong]     = Unsigned16;
              ULongMin                    = Unsigned16Min;
              ULongMax                    = Unsigned16Max;
              break;
     case 4 : machinetype[Unsigned32]     = MUnsignedLong;
              datatype[MUnsignedLong]     = Unsigned32;
              ULongMin                    = Unsigned32Min;
              ULongMax                    = Unsigned32Max;
              break;
     case 8 : machinetype[Unsigned64]     = MUnsignedLong;
              datatype[MUnsignedLong]     = Unsigned64;
              ULongMin                    = Unsigned64Min;
              ULongMax                    = Unsigned64Max;
              break;
     }

  // char
  switch (sizeof( char ) ) {
     case 1 : machinetype[Signed8]        = MChar;
              datatype[MChar]             = Signed8;
              CharMin                     = Signed8Min;
              CharMax                     = Signed8Max;
              break;
     case 2 : machinetype[Signed16]       = MChar;
              datatype[MChar]             = Signed16;
              CharMin                     = Signed16Min;
              CharMax                     = Signed16Max;
              break;
     }

  // short int
  switch (sizeof( short int ) ) {
     case 1 : machinetype[Signed8]        = MShort;
              datatype[MShort]            = Signed8;
              ShortMin                    = Signed8Min;
              ShortMax                    = Signed8Max;
              break;
     case 2 : machinetype[Signed16]       = MShort;
              datatype[MShort]            = Signed16;
              ShortMin                    = Signed16Min;
              ShortMax                    = Signed16Max;
              break;
     case 4 : machinetype[Signed32]       = MShort;
              datatype[MShort]            = Signed32;
              ShortMin                    = Signed32Min;
              ShortMax                    = Signed32Max;
              break;
     }

  // int
  switch (sizeof( int ) ) {
     case 1 : machinetype[Signed8]        = MInteger;
              datatype[MInteger]          = Signed8;
              IntMin                      = Signed8Min;
              IntMax                      = Signed8Max;
              break;
     case 2 : machinetype[Signed16]       = MInteger;
              datatype[MInteger]          = Signed16;
              IntMin                      = Signed16Min;
              IntMax                      = Signed16Max;
              break;
     case 4 : machinetype[Signed32]       = MInteger;
              datatype[MInteger]          = Signed32;
              IntMin                      = Signed32Min;
              IntMax                      = Signed32Max;
              break;
     case 8 : machinetype[Signed64]       = MInteger;
              datatype[MInteger]          = Signed64;
              IntMin                      = Signed64Min;
              IntMax                      = Signed64Max;
              break;
     }

  // long
  switch (sizeof( long ) ) {
     case 1 : machinetype[Signed8]        = MLong;
              datatype[MLong]             = Signed8;
              LongMin                     = Signed8Min;
              LongMax                     = Signed8Max;
              break;
     case 2 : machinetype[Signed16]       = MLong;
              datatype[MLong]             = Signed16;
              LongMin                     = Signed16Min;
              LongMax                     = Signed16Max;
              break;
     case 4 : machinetype[Signed32]       = MLong;
              datatype[MLong]             = Signed32;
              LongMin                     = Signed32Min;
              LongMax                     = Signed32Max;
              break;
     case 8 : machinetype[Signed64]       = MLong;
              datatype[MLong]             = Signed64;
              LongMin                     = Signed64Min;
              LongMax                     = Signed64Max;
              break;
     }

  // float
  switch (sizeof( float ) ) {
     case 4 : machinetype[FloatIEEE32]    = MFloat;
              datatype[MFloat]            = FloatIEEE32;
              break;
     case 8 : machinetype[DoubleIEEE64]   = MFloat;
              datatype[MFloat]            = DoubleIEEE64;
              break;
     }

  // double
  switch (sizeof( double ) ) {
     case 4 : machinetype[FloatIEEE32]    = MDouble;
              datatype[MDouble]           = FloatIEEE32;
              break;
     case 8 : machinetype[DoubleIEEE64]   = MDouble;
              datatype[MDouble]           = DoubleIEEE64;
              break;
     }

  MachineTypeInit = 1;

} /* initmachinetype */

/*---------------------------------------------------------------------------
NAME
  edf_datatype2machinetype ---  returns the machine data type of a data type

SYNOPSIS
  int edf_datatype2machinetype ( int dtype )

---------------------------------------------------------------------------*/
int edf_datatype2machinetype ( int dtype )
{ if (!MachineTypeInit) initmachinetypes();
  if ((dtype<0)||(dtype>=EndDType)) dtype = InValidDType;
  return( MachineType[dtype] );
} /* edf_datatype2machinetype */

/*---------------------------------------------------------------------------
NAME
  edf_machinetype2datatype ---  returns the data type of a machine type

SYNOPSIS
  int edf_machinetype2datatype ( int mtype )

---------------------------------------------------------------------------*/
int edf_machinetype2datatype ( int mtype )
{ if (!MachineTypeInit) initmachinetypes();
  if ((mtype<0)||(mtype>=EndMType)) mtype = InValidMType;
  return( DataType[mtype] );
} /* edf_machinetype2datatype */

/*---------------------------------------------------------------------------
NAME
   edf_showdatatypes

SYNOPSIS
   void edf_showdatatypes ( int full )

DESCRIPTION
  Shows data types and the corresponding machine data types.
  If full is 0 only data types are shown that can be converted.
---------------------------------------------------------------------------*/
void edf_showdatatypes ( int full )
{ int i;
  if (full)
    printf(" %15s               = %15s\n",  "DataType", "MachineType");
  for (i=1; i<EndDType; i++) {
    if ( (full) || (edf_datatype2machinetype(i)) ) { 
      printf(" %15s (%5lu bytes) = %15s (%5lu bytes)\n",
        edf_datatype2string(i), edf_data_sizeof(i),
        MachineType2String(edf_datatype2machinetype(i)),
        edf_machine_sizeof(edf_datatype2machinetype(i)) );
      }
    }

} /* edf_showdatatypes */

/*---------------------------------------------------------------------------
NAME
   edf_showmachinetypes

SYNOPSIS
   void edf_showmachinetypes ( int full )

DESCRIPTION
  Shows machine data types and the corresponding data types.
  If full is 0 only machine types are shown that can be converted
---------------------------------------------------------------------------*/
void edf_showmachinetypes ( int full )
{ int i;
  if (full)
    printf(" %15s               = %15s\n", "MachineType", "DataType");
  for (i=1; i<EndMType; i++) {
    if ( (full) || (edf_machinetype2datatype(i)) ) {
      printf(" %15s (%5lu bytes) = %15s (%5lu bytes)\n",
        MachineType2String(i), edf_machine_sizeof(i),
        edf_datatype2string(edf_machinetype2datatype(i)),
        edf_data_sizeof(edf_machinetype2datatype(i)) );
      }
    }

} /* edf_showmachinetypes */

/*===machinetype END=======================================================*/

/*===data_conversion BEGIN===============================================---*/

//add UOFF and clip at 0 and UMAX (signed in, unsigned out)
# define ACLIP_S2U( X, UOFF, UMAX) (( UOFF)<-( X))?0UL:((( UMAX)-( UOFF))<( X))?( UMAX):( ( UOFF) + ( X))
//subtract UOFF and clip at 0 and UMAX (signed in, unsigned out)
# define SCLIP_S2U( X, UOFF, UMAX) ((UOFF)>( X))?0UL:(( UMAX)<( ( X) - ( UOFF)))?( UMAX):( ( X) - ( UOFF) )
//add UOFF and clip at 0 and UMAX (unsigned in, unsigned out)
# define ACLIP_U2U( UX, UOFF, UMAX)  ((UMAX)-( UOFF))<( UX)?( UMAX):( ( UOFF) + ( UX) )
//subtract UOFF and clip at 0 and UMAX (unsigned in, unsigned out)
# define SCLIP_U2U( UX, UOFF, UMAX)  (( UOFF)>( UX))?0UL:(( UMAX)<(( UX)-( UOFF)))?UMAX:(( UX)-( UOFF))
//clip at MIN and MAX (float in)
# define CLIP_FLOAT(X, MIN, MAX) (( X)<( MIN))?( MIN):( (( X)>( MAX))?( MAX):( X) )

/*---------------------------------------------------------------------------
NAME
 
  Convert2UnsignedShort
 
SYNOPSIS
 
  DType DataTypeIn;
  int Convert2UnsignedShort( unsigned short * Out,
                             const void * In, long ValueOffset,
                             int MachineTypeIn, unsigned long DataCount);
 
DESCRIPTION
 
  Conversion of MachineTypeIn array In to unsigned short (MUnsignedShort) 
  array Out. The input data buffer 'In' with elements of the type 
  'MachineTypeIn' is copied to the output data buffer 'Out' with elements 
  of the type unsigned short. Both buffers contain 'DataCount' elements.
 
  The allocated memory for the output buffer must be sufficiently large.
  If the required memory for the output buffer is smaller or equal to the
  input buffer, the same buffer can be used for input and output.
 
RETURN VALUE
  success:  0
  error:   -1
 
AUTHOR
  Peter Boesecke 
 
HISTORY
  2001-09-02 PB from Convert2Float
  --------------------------------------------------------------------------*/
int Convert2UnsignedShort( unsigned short * Out,
                           const void * In, long ValueOffset,
                           int MachineTypeIn, unsigned long DataCount)
{ register unsigned long i;
 
  const unsigned char  * pinuchar;
  const char           * pinchar;
  const unsigned short * pinushort;
  const short          * pinshort;
  const unsigned int   * pinuint;
  const int            * pinint;
  const unsigned long  * pinulong;
  const long           * pinlong;
  const float          * pinfloat;
  const double         * pindouble;
 
  unsigned short       * pout;
  long                   dvo = ValueOffset;
  unsigned long          advo = (ValueOffset<0)?-ValueOffset:ValueOffset;
  double                 tmp;

  size_t t_in, t_out = edf_machine_sizeof ( MUnsignedShort );

  if (EDFIO_debug) {
    printf("Convert2UnsignedShort\n");
  } // EDFIO_debug
 
  switch ( MachineTypeIn ) {
    // conversion to long, add dvo, conversion to unsigned short
    case MUnsignedChar : t_in = edf_machine_sizeof( MUnsignedChar );
                         if (t_in<t_out) {
                           pinuchar = (unsigned char *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pinuchar;
                             if (dvo<0) 
                               *(--pout) = SCLIP_U2U(*pinuchar,advo,UShortMax);
                              else 
                               *(--pout) = ACLIP_U2U(*pinuchar,advo,UShortMax);
                             }
                          } else {
                           pinuchar = (unsigned char *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             if (dvo<0)
                               *pout = SCLIP_U2U(*pinuchar,advo,UShortMax);
                              else
                               *pout = ACLIP_U2U(*pinuchar,advo,UShortMax);
                             pout++; pinuchar++;
                             }
                           }
                         break;
     case MChar :         t_in = edf_machine_sizeof( MChar );
                         if (t_in<t_out) {
                           pinchar = (char *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pinchar;--pout;
                             if (dvo<0)
                               *pout = SCLIP_S2U(*pinchar,advo,UShortMax);
                              else
                               *pout = ACLIP_S2U(*pinchar,advo,UShortMax);
                             }
                          } else {
                           pinchar = (char *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             if (dvo<0)
                               *pout = SCLIP_S2U(*pinchar,advo,UShortMax);
                              else
                               *pout = ACLIP_S2U(*pinchar,advo,UShortMax);
                             pout++; pinchar++;
                             }
                           }
                         break;
    case MUnsignedShort: t_in = edf_machine_sizeof( MUnsignedShort );
                         if ((In!=Out)||(ValueOffset!=0)) {
                           pinushort = (unsigned short *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pinushort;
                             if (dvo<0)
                               *(--pout) = SCLIP_U2U(*pinushort,advo,UShortMax);
                              else
                               *(--pout) = ACLIP_U2U(*pinushort,advo,UShortMax);
                             }
                           }
                         break;
     case MShort :       t_in = edf_machine_sizeof( MShort );
                         if (t_in<t_out) {
                           pinshort = (short *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pinshort;
                             if (dvo<0)
                               *(--pout) = SCLIP_S2U(*pinshort,advo,UShortMax);
                              else
                               *(--pout) = ACLIP_S2U(*pinshort,advo,UShortMax);
                             }
                          } else {
                           pinshort = (short *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             if (dvo<0)
                               *pout = SCLIP_S2U(*pinshort,advo,UShortMax);
                              else
                               *pout = ACLIP_S2U(*pinshort,advo,UShortMax);
                             pout++; pinshort++;
                             }
                          }
                         break;
     case MUnsignedInteger: t_in = edf_machine_sizeof( MUnsignedInteger );
                         if (t_in<t_out) {
                           pinuint = (unsigned int *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pinuint; --pout;
                             if (dvo<0)
                               *pout = SCLIP_U2U(*pinuint,advo,UShortMax);
                              else
                               *pout = ACLIP_U2U(*pinuint,advo,UShortMax);
                             }
                          } else {
                           pinuint = (unsigned int *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             if (dvo<0)
                               *pout = SCLIP_U2U(*pinuint,advo,UShortMax);
                              else
                               *pout = ACLIP_U2U(*pinuint,advo,UShortMax);
                             pout++; pinuint++;
                             }
                          }
                         break;
     case MInteger :     t_in = edf_machine_sizeof( MInteger );
                         if (t_in<t_out) {
                           pinint = (int *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pinint;--pout;
                             if (dvo<0)
                               *pout = SCLIP_S2U(*pinint,advo,UShortMax);
                              else
                               *pout = ACLIP_S2U(*pinint,advo,UShortMax);
                             }
                          } else {                                                                                                                                       pinint = (int *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             if (dvo<0)
                               *pout = SCLIP_S2U(*pinint,advo,UShortMax);
                              else
                               *pout = ACLIP_S2U(*pinint,advo,UShortMax);
                             pout++; pinint++;
                             }
                          }
                         break;
    case MUnsignedLong:  t_in = edf_machine_sizeof( MUnsignedLong );
                         if (t_in<t_out) {
                           pinulong = (unsigned long *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pinulong; --pout;
                             if (dvo<0)
                               *pout = SCLIP_U2U(*pinulong,advo,UShortMax);
                              else
                               *pout = ACLIP_U2U(*pinulong,advo,UShortMax);
                             }
                          } else {
                           pinulong = (unsigned long *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             if (dvo<0)
                               *pout = SCLIP_U2U(*pinulong,advo,UShortMax);
                              else
                               *pout = ACLIP_U2U(*pinulong,advo,UShortMax);
                             pout++; pinulong++;
                             }
                          }
                         break;
    case MLong :         t_in = edf_machine_sizeof( MLong );
                         if (t_in<t_out) {
                           pinlong = (long *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pinlong; --pout;
                             if (dvo<0)
                               *pout = SCLIP_S2U(*pinlong,advo,UShortMax);
                              else
                               *pout = ACLIP_S2U(*pinlong,advo,UShortMax);
                             }
                          } else {
                           pinlong = (long *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             if (dvo<0)
                               *pout = SCLIP_S2U(*pinlong,advo,UShortMax);
                              else
                               *pout = ACLIP_S2U(*pinlong,advo,UShortMax);
                             pout++; pinlong++; 
                             }
                          }
                         break;
     case MFloat:        t_in = edf_machine_sizeof( MFloat );
                         if (t_in<t_out) {
                           pinfloat = (float *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) { // round
                             --pinfloat;
                             tmp = floor( *pinfloat + 0.5 + dvo );
                             pout--;
                             *pout = CLIP_FLOAT(tmp, UShortMin, UShortMax);
                             }
                          } else {
                           pinfloat = (float *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) { // round
                             tmp = floor( *pinfloat + 0.5 + dvo );
                             pinfloat++;
                             *pout = CLIP_FLOAT(tmp, UShortMin, UShortMax);
                             pout++;
                             }
                          }
                         break;
    case MDouble :       t_in = edf_machine_sizeof( MDouble );
                         if (t_in<t_out) {
                           pindouble = (double *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pindouble;
                             tmp = floor( *pindouble + 0.5 + dvo );
                             --pout;
                             *pout = CLIP_FLOAT(tmp, UShortMin, UShortMax);
                             }
                          } else {
                           pindouble = (double *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             tmp = floor( *pindouble + 0.5 + dvo );
                             pindouble++;
                             *pout = CLIP_FLOAT(tmp, UShortMin, UShortMax);
                             pout++;
                             }
                          }
                         break;
    default :            return(-1);
                         break;
    } /* switch */
 
  return(0);
 
} /* Convert2UnsignedShort */

/*---------------------------------------------------------------------------
NAME

  Convert2Float

SYNOPSIS

  DType DataTypeIn;
  int Convert2Float( float * Out, 
                     const void * In, long ValueOffset, 
                     int MachineTypeIn, unsigned long DataCount);

DESCRIPTION

  Conversion of MachineTypeIn array In to float (MFloat) array Out.
  The input data buffer 'In' with elements of the type 'MachineTypeIn'
  is copied to the output data buffer 'Out' with elements of the
  type float. Both buffers contain 'DataCount' elements.

  The allocated memory for the output buffer must be sufficiently large. 
  If the required memory for the output buffer is smaller or equal to the
  input buffer, the same buffer can be used for input and output.

RETURN VALUE
  success:  0
  error:   -1

AUTHOR
  Peter Boesecke 08-Mar-1998
  
HISTORY
  1999-07-23 PB DataValueOffset
  1999-11-26 PB t_out initialized with sizeof(float)
  2000-04-30 PB name changed to Data2Float, MType for output
  2001-09-02 PB name changed to Convert2Float: 
                DataTypeIn -> MachineTypeIn
                DataValueOffset -> ValueOffset
  --------------------------------------------------------------------------*/
int Convert2Float( float * Out, const void * In, long ValueOffset,
                   int MachineTypeIn, unsigned long DataCount)
{ register unsigned long i;

  const unsigned char  * pinuchar;
  const char           * pinchar;
  const unsigned short * pinushort;
  const short          * pinshort;
  const unsigned int   * pinuint;
  const int            * pinint;
  const unsigned long  * pinulong;
  const long           * pinlong;
  const float          * pinfloat;
  const double         * pindouble;

  float * pout;
  float dvo = (float) ValueOffset;

  size_t t_in, t_out = edf_machine_sizeof ( MFloat );

  if (EDFIO_debug) {
    printf("Convert2Float\n");
  } // EDFIO_debug

  switch ( MachineTypeIn ) {
    case MUnsignedChar : t_in = edf_machine_sizeof( MUnsignedChar );
                         if (t_in<t_out) {
                           pinuchar = (unsigned char *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pinuchar;
                             *pout = dvo + (float) *pinuchar;
                           }
                         } else {
                           pinuchar = (unsigned char *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pinuchar;
                             pout++; pinuchar++;
                           }
                         }
                         break;
     case MChar :         t_in = edf_machine_sizeof( MChar );
                         if (t_in<t_out) {
                           pinchar = (char *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pinchar;
                             *pout = dvo + (float) *pinchar;
                           }
                         } else {
                           pinchar = (char *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pinchar;
                             pout++; pinchar++;
                           }
                         }
                         break;
     case MUnsignedShort: t_in = edf_machine_sizeof( MUnsignedShort );
                         if (t_in<t_out) {
                           pinushort = (unsigned short *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pinushort;
                             *pout = dvo + (float) *pinushort;
                           }
                         } else {
                           pinushort = (unsigned short *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pinushort;
                             pout++; pinushort++;
                           }
                         }
                         break;
     case MShort :        t_in = edf_machine_sizeof( MShort );
                         if (t_in<t_out) {
                           pinshort = (short *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pinshort;
                             *pout = dvo + (float) *pinshort;
                           }
                         } else {
                           pinshort = (short *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pinshort;
                             pout++; pinshort++;
                           }
                         }
                         break;
     case MUnsignedInteger: t_in = edf_machine_sizeof( MUnsignedInteger );
                         if (t_in<t_out) {
                           pinuint = (unsigned int *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pinuint;
                             *pout = dvo + (float) *pinuint;
                           }
                         } else {
                           pinuint = (unsigned int *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pinuint;
                             pout++; pinuint++;
                           }
                         }
                         break;
     case MInteger :     t_in = edf_machine_sizeof( MInteger );
                         if (t_in<t_out) {
                           pinint = (int *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pinint;
                             *pout = dvo + (float) *pinint;
                           }
                         } else {
                           pinint = (int *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pinint;
                             pout++; pinint++;
                           }
                         }
                         break;
    case MUnsignedLong:  t_in = edf_machine_sizeof( MUnsignedLong );
                         if (t_in<t_out) {
                           pinulong = (unsigned long *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pinulong;
                             *pout = dvo + (float) *pinulong;
                           }
                         } else {
                           pinulong = (unsigned long *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pinulong;
                             pout++; pinulong++;
                           }
                         }
                         break;
    case MLong :         t_in = edf_machine_sizeof( MLong );
                         if (t_in<t_out) {
                           pinlong = (long *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pinlong;
                             *pout = dvo + (float) *pinlong;
                           }
                         } else {
                           pinlong = (long *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pinlong;
                             pout++; pinlong++;
                           }
                         }
                         break;
    case MFloat :        t_in = edf_machine_sizeof( MFloat );
                         if ((In!=Out)||(ValueOffset!=0)) {
                           pinfloat = (float *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i)
                             *(--pout) = dvo + *(--pinfloat);
                           }
                         break;
    case MDouble :       t_in = edf_machine_sizeof( MDouble );
                         if (t_in<t_out) {
                           pindouble = (double *) In + DataCount;
                           pout = Out + DataCount;
                           for (i=DataCount;i>0;--i) {
                             --pout; --pindouble;
                             *pout = dvo + (float) *pindouble;
                           }
                         } else {
                           pindouble = (double *) In;
                           pout = Out;
                           for (i=0;i<DataCount;i++) {
                             *pout = dvo + (float) *pindouble;
                             pout++; pindouble++;
                           }
                         }
                         break;
    default :            return(-1);
                         break; 
    } /* switch */

  return(0);

} /* Convert2Float */

/*---------------------------------------------------------------------------
NAME
  
  edf_machine2machine

SYNOPSIS

  MType mtype_src; 
  MType mtype_dest;
  int edf_machine2machine ( void * dest, int mtype_dest,
                            const void * src, long data_value_offset,
                            int mtype_src, unsigned long n)

DESCRIPTION

  The input data buffer 'src' with element of the type 'mtype_src' 
  is copied to the output data buffer 'dest' with elements of the 
  type 'mtype_dest'. Both buffers consists of 'n' elements.

  The data types are listed in 'MType'.

  The allocated memory for the output buffer must be sufficiently large. 
  If the required memory for the output buffer is smaller or equal to the
  length of the input buffer both buffers can be identical.

RETURN VALUE
  success: 0
  error:   -1
  
AUTHOR
  Peter Boesecke 08-Mar-1998

HISTORY
  23-Jul-1999 DataValueOffset (_DataVersion 2.20)
  30-Apr-2000 Name changed to edf_data2machine and MachineType introduced.
  02-Sep-2001 dtype_src changed to mtype_src, 
              name changed to edf_machine2machine 
  --------------------------------------------------------------------------*/
int edf_machine2machine ( void * dest, int mtype_dest,
                          const void * src, long value_offset,
                          int mtype_src, unsigned long n)
{ int failed = True;
  void * out = dest;
  const void *in = src;

  if (EDFIO_debug) {
    printf("edf_machine2machine\n");
    edf_showmachinetypes( True );
    printf("Conversion of %s to %s\n",
            MachineType2String( mtype_src ), 
            MachineType2String( mtype_dest ) );
    } // EDFIO_debug

  if (!mtype_src) return(-1);
  switch ( mtype_dest ) {
    case MUnsignedChar   : break;
    case MChar           : break;
    case MUnsignedShort  : failed = Convert2UnsignedShort( 
                           (unsigned short *) out, in,
                           value_offset, mtype_src, n ); break; 
    case MShort          : break;
    case MUnsignedInteger: break;
    case MInteger        : break;
    case MUnsignedLong   : break;
    case MLong           : break;
    case MFloat          : failed = Convert2Float( 
                           (float *) out, in, 
                           value_offset, mtype_src, n ); break;
    case MDouble         : break;
    default              : break;  
    } /* switch */

  if (failed) return(-1);

  return(0);

} /* edf_machine2machine */

/*===data_conversion END=================================================---*/

/*===history BEGIN========================================================*/

/*+++ public interface of history -------------------------------------------
 
  DESCRIPTION
  The routines of this module are used to read and write history lines.
  'edf_history_new' must be called first. It clears and initializes the
  internal history list and argument list.
  'edf_history_skip' marks the next argument as not required
  'edf_history_take' marks the next argument as required (default)
  'edf_history_argv' is used to store the arguments of the call in an
  internal list. It can be called several times to pass all arguments.
  'edf_read_header_history' initializes the history list and reads the
  history strings from the file header.
  'edf_write_header_history' writes the history strings into the output
  file header.
  'edf_history_free' releases all internal lists. 'edf_history_new' must
  be called before using one of the routines again.
  'hist_debug' sets the module into debug mode.
 
  The length of a history line is limited to MaxHistoryLineSize-1.
 
  HISTORY
  1999-06-24  V1.0 Peter Boesecke
  1999-11-08  V1.1 PB
  2001-09-13       PB history_key
  2001-09-15  V1.2 PB history_root -> history_line_root
                      history_root used for HBlock
 
PUBLIC extern int
  edf_history_new          ( const char * history_key ),
  edf_history_skip         ( const char * history_key ),
  edf_history_take         ( const char * history_key ),
  edf_history_argv         ( const char * history_key,
                             const char * substring ),
  edf_read_header_history  ( int stream, long int DataNumber,
                             int DataChain, const char * history_key,
                             int * pErrorValue, int * pstatus ),
  edf_write_header_history ( int stream, long int DataNumber,
                             int DataChain, const char * history_key,
                             int * pErrorValue, int * pstatus ),
  edf_history_read_header  ( const char * header_key,
                             const char * history_key,
                             int * pErrorValue, int * pstatus ),
  edf_history_write_header ( const char * header_key,
                             const char * history_key,
                             int * pErrorValue, int * pstatus ),
  edf_history_copy         ( const char * history_copy,
                             const char * history_key ),
  edf_history_free         ( const char * history_key );
 
PUBLIC extern void
  hist_debug               ( int debug );
 
  ---------------------------------- end of public interface of history ---*/                                                                 
/****************************************************************************
* Constants                                                                 *
****************************************************************************/
//# define MaxHistoryLineSize        (MaxLinLen+1) no restriction 2005-02-27
# define MaxHistoryLineSize        (2048+1)
# define MaxHistoryKeySize         (MaxKeyLen+1)
# define HistoryKeyBufferSize      1024
 
/****************************************************************************
* Static Variables                                                          *
****************************************************************************/
static int          HIST_debug = 0;
static int          InitHistory = 0;
static HBlock     * history_root = (HBlock *) NULL;
static const char * argv_key = "_argv_key" ;
static const char * default_history_line_key = HISTORY_KEY_PREFIX"1" ;

/*---------------------------------------------------------------------------
NAME
 
   needquotes
 
SYNOPSIS
 
  int needquotes( const char * string );
 
DESCRIPTION
Tests, whether the string must be encapsulated between double quotes.
Conditions:
  string starts with '\"'      -> return value 0
  string contains white spaces or characters that must be quoted
                               -> return value >0 , otherwise 0
  string has zero length       -> return value 1
 
RETURN VALUES
Returns a positive number when the string must be encapsulated, otherwise 0
---------------------------------------------------------------------------*/
int needquotes( const char * string )
{ int encapsulation = 0;
  const char * ps = string;
 
   if (string[0]!='\"') {
     while ( (*ps) && (!(encapsulation)) ) {
       encapsulation = is_white(*ps) + is_tobequoted ( *ps );
       ps++;
     }
   } else encapsulation = 0;

  if ( strlen(string)==0 ) encapsulation = 1;
 
  return ( encapsulation );
 
} /* needquotes  */

/*--------------------------------------------------------------------------
NAME
 
  hist_debug --- set / reset module history into debug mode
 
SYNOPSIS
 
  void hist_debug ( int debug );
 
DESCRPTION
 
  Writes 'debug' into HIST_debug.
 
-----------------------------------------------------------------------------*/
void hist_debug ( int debug )
{ HIST_debug = debug;
} /* hist_debug */

/*---------------------------------------------------------------------------
print_history_list (success:0, error:-1)
---------------------------------------------------------------------------*/
int print_history_list( FILE * out, int level, int verbose, HSymb * root )
{ const char * SeparationLine =
    "-       -       -       -       -       -       -       -";
 
  HSymb * hline = root;
 
  if (level<1) return(0);
 
  while (hline!=(HSymb*) NULL) {
    if (verbose) {
      fprintf(out,"   %s\n",SeparationLine);
      fprintf(out,"   key               = %s\n",hline->key);
      fprintf(out,"   line              = %s\n",hline->line);
//      fprintf(out,"   size              = %zu\n",hline->size);
      fprintf(out,"   size              = %zu | %lu\n",
        hline->size,hline->size);
      fprintf(out,"   required          = %d\n",hline->required);
//      fprintf(out,"   shortlen          = %zu\n",hline->shortlen);
      fprintf(out,"   shortlen          = %zu | %lu\n",
        hline->shortlen, hline->shortlen);
      fprintf(out,"   Previous key      = ");
      if ((hline->Previous)!=(HSymb*) NULL)
        fprintf(out,"%s\n", hline->Previous->key);
        else fprintf(out,"(no previous history line)\n");
      fprintf(out,"   Next key          = ");
      if ((hline->Next)!=(HSymb*) NULL)
        fprintf(out,"%s\n", hline->Next->key);
        else fprintf(out,"(no next history line)\n");
      } else {
      fprintf(out,"   '%s' = '%s'\n",hline->key,hline->line);
      }
    hline=hline->Next;
    }
    if (verbose) fprintf(out,"   %s\n",SeparationLine);
 
  return(0);
 
} /* print_history_list */

/*---------------------------------------------------------------------------
NAME
 
   init_history
 
SYNOPSIS

   int init_history( void )
 
DESCRIPTION
   Inits history module
 
RETURN VALUES
Returns 0 in case of success and -1 if no success.
---------------------------------------------------------------------------*/
int init_history( void )
{ history_root = (HBlock *) NULL; 
  InitHistory  = 1;
  return(0);
} /* init_history */

/*---------------------------------------------------------------------------
NAME
 
  copy_history_list
 
SYNOPSIS

  int copy_history_list(HSymb **proot, HSymb *src);
 
DESCRIPTION
Copies all history lines from src to *proot
 
RETURN VALUES
In case of success the return value is 0, otherwise -1.
 
---------------------------------------------------------------------------*/
int copy_history_list(HSymb **proot, HSymb *src)
{ HSymb * hline;
 
  while (src!=(HSymb*) NULL) {

    if (history_line_new  ( proot, src->key, src->size, &hline )) return(-1);

    hline->line     = strncpy(hline->line,src->line,src->size);
    hline->required = src->required;
    hline->shortlen = src->shortlen;

    src=src->Next;
  }

  return(0);

} /* copy_history_list */

/*---------------------------------------------------------------------------
NAME
 
  history_free
 
SYNOPSIS
 
DESCRIPTION
Removes all history lines.
 
RETURN VALUES
In case of success the return value is 0, otherwise -1.
 
---------------------------------------------------------------------------*/
int history_free         ( HSymb **proot )
{ HSymb * hline, * next;
 
  next = *proot;
 
  while (next) {
    hline = next;
    next = hline->Next;
    if (hline->key) free(hline->key);
    if (hline->line) free(hline->line);
    if (hline) free(hline);
    }
 
  *proot = (HSymb *) NULL;
 
  return(0);
 
} /* history_free */

/*+++------------------------------------------------------------------------
NAME
 
   clear_header_history
 
SYNOPSIS
 
   int clear_header_history ( HBlock * hblock )
 
DESCRIPTION
Releases all allocated history lines of 'hblock'
 
RETURN VALUES
In case of success the return value is 0, otherwise -1.
 
---------------------------------------------------------------------------*/
int clear_header_history ( HBlock * hblock )                          /*---*/
{
  if (!InitHistory) return(-1);
 
  if (history_free( &hblock->history_line_root ) ) return(-1);
  if (hblock->current_history_line_key ) {
    free( hblock->current_history_line_key );
    hblock->current_history_line_key = (char *) NULL;
    }
 
  return(0);
 
} /* clear_header_history */

/*+++------------------------------------------------------------------------
NAME
 
   insert_history_block
 
SYNOPSIS
 
   int insert_history_block( const char * history_key, HBlock **hblock );
 
DESCRIPTION
Searches for a history block with 'history_key' and clears it or inserts
an empty history block with 'history_key'. The pointer to the history
block is returned in *hblock.
 
RETURN VALUES
In case of success the return value is 0, otherwise -1.
---------------------------------------------------------------------------*/
int insert_history_block( const char * history_key, HBlock **hblock ) /*---*/
{
  HBlock * newhblock, * previous, * next;
  int     notfound = 1;
 
  /* search for history_key */
  *hblock = (HBlock *) NULL;
  previous = (HBlock *) NULL;
  next = history_root;
 
  /* search insertion point (insertion before next) */
  while( ( next!=(HBlock *) NULL ) && (notfound>0) ) {
    notfound = compare_keys(next->key,history_key,UpperCaseSort);
    if (notfound>0) {previous = next; next = next->Next;}
    }
 
  if (notfound) {
    /* create new hblock */
    if (!(newhblock = (HBlock *) malloc(sizeof(HBlock)))) return(-1);
    newhblock->key = newstr(history_key); 
    if (!newhblock->key) {free(newhblock);return(-1);}
    newhblock->history_argv_root = (HSymb *) NULL;
    newhblock->history_line_root = (HSymb *) NULL;
    newhblock->current_history_line_key = (char *) NULL;
 
    /* insert newhblock before next */
    if (next) next->Previous = newhblock;
    newhblock->Next=next;
    newhblock->Previous=previous;
    if (previous) previous->Next=newhblock;
      else history_root = newhblock;
 
    next = newhblock;
    }
 
  /* clear block (next) */
  if (history_free( &next->history_argv_root )) return(-1);
  if (clear_header_history( next )) return(-1);
 
  *hblock = next;
 
  return(0);
 
} /* insert_history_block */

/*+++------------------------------------------------------------------------
NAME
 
   copy_history_block
 
SYNOPSIS
 
   int copy_history_block( const char * history_key, HBlock **hblock,
                           HBlock *src )
 
DESCRIPTION
Searches for a history block with 'history_key' and clears it or inserts
an empty history block with 'history_key'. The contents of src is copied 
into this block. The pointer to the history block is returned in *hblock.
 
RETURN VALUES
In case of success the return value is 0, otherwise -1.
---------------------------------------------------------------------------*/
int copy_history_block( const char * history_key, HBlock **hblock,
                        HBlock *src ) /*---*/
{ HBlock * dest;

  if (insert_history_block( history_key, hblock )) return(-1);

  // already done if src is identical to *hblock
  if (src == *hblock) return(0); 

  // copy src to *hbloc
  dest = *hblock; 
  if (copy_history_list(&(dest->history_line_root),src->history_line_root))
    return(-1);
  if (copy_history_list(&(dest->history_argv_root),src->history_argv_root))
    return(-1);
  if (!(dest->current_history_line_key=newstr(src->current_history_line_key)))
    return(-1);

  return(0);

} /* copy_history_block */

/*+++------------------------------------------------------------------------
NAME
 
   search_history_block
 
SYNOPSIS
 
   int search_history_block( const char * history_key, HBlock ** hblock );
 
DESCRIPTION
Searches for history block 'history_key' and returns it in *hblock
 
RETURN VALUES
In case of success the return value is 0, otherwise -1.
---------------------------------------------------------------------------*/
int search_history_block( const char * history_key, HBlock ** hblock )
{ HBlock * previous, * next;
  int      notfound = 1;
 
  /* search for history_key */
  *hblock = (HBlock *) NULL;
  previous = (HBlock *) NULL;
  next = history_root;
 
  /* search history_key */
  while( ( next!=(HBlock *) NULL ) && (notfound) ) {
    notfound = compare_keys(next->key,history_key,UpperCaseSort);
    if (notfound) {previous = next; next = next->Next;}
    }
 
  *hblock = next;
 
  if (*hblock==(HBlock *) NULL) return(-1); else return(0);
 
} /* search_history_block */

/*+++------------------------------------------------------------------------
NAME
 
   remove_history_block
 
SYNOPSIS
 
   int remove_history_block( HBlock * hblock );
 
DESCRIPTION
Removes history block hblock from history_root and releases its contents
 
RETURN VALUES
In case of success the return value is 0, otherwise -1.
---------------------------------------------------------------------------*/
int remove_history_block( HBlock * hblock )
{ HBlock * previous, *next;
 
  /* stop, if no symbol found */
  if (hblock==(HBlock *) NULL) return(-1);
 
  /* change links */
  previous = hblock->Previous;
  next = hblock->Next;
 
  if ( next != (HBlock *) NULL ) next->Previous = previous;
  if ( previous != (HBlock *) NULL ) previous->Next = next;
    else history_root = next;
 
  /* free history block and its elements */
  if (history_free( &hblock->history_argv_root ) ) return(-1);
  if (clear_header_history ( hblock ) ) return(-1);
  if (hblock->key) free(hblock->key);
  free(hblock);
 
  return(0);
 
} /* remove_history_block */

/*---------------------------------------------------------------------------
NAME
 
  history_line_new
 
SYNOPSIS
 
  int history_line_new  ( HSymb ** proot,
                          const char * history_line_key, size_t history_size,
                          HSymb ** phline )
 
DESCRIPTION
Adds a new history line with the key 'history_line_key' to the list and 
allocates 'history_size' bytes for the line. If the key already exists, only 
the line is cleared and reallocated with 'history_size' bytes. The key list is 
ordered in reverse alphabetical order. If phline is not NULL a pointer to the 
history line is returned.
 
RETURN VALUES
Returns 0 in case of success and -1 if no success.
---------------------------------------------------------------------------*/
PRIVATE int history_line_new  ( HSymb ** proot,
                        const char * history_line_key, size_t history_size,
                        HSymb ** phline )
{
  HSymb * newhline, * previous, * next;
  // HSymb * hline; // unused
  int     notfound = 1;
 
  /* search for history_line_key */
  if (phline!=(HSymb **)NULL) *phline = (HSymb *) NULL;
  previous = (HSymb *) NULL;
  next = *proot;
 
  /* search insertion point (insertion before next) */
  while( ( next!=(HSymb *) NULL ) && (notfound>0) ) {
    notfound = compare_keys(next->key,history_line_key,UpperCaseSort);
    if (notfound>0) {previous = next; next = next->Next;}
    }
 
  if (notfound) {
    /* create new hline */
    if (!(newhline = (HSymb *) malloc(sizeof(HSymb)))) return(-1);
    newhline->key = newstr(history_line_key); 
    if (!newhline->key) {free(newhline); return(-1);}
    newhline->line = (char *) NULL;
    newhline->size = (size_t) 0;
    newhline->required = 1;
    newhline->shortlen = 0;
 
    /* insert newhline before next */
    if (next) next->Previous = newhline;
    newhline->Next=next;
    newhline->Previous=previous;
    if (previous) previous->Next=newhline;
      else *proot = newhline;
 
    next = newhline;
    }
 
  /* update next->line */
  if (phline!=(HSymb **)NULL) *phline = next;
  if ( next->line ) {
    free ( next->line );
    next->line = (char *) NULL;
    next->size = (size_t) 0;
    next->required = 1;
    next->shortlen = 0;
    }                          
  if (history_size<1) return(-1);
  if (!(next->line = (char *) malloc(history_size))) return(-1);
  next->line[0] = '\0';
  next->size = history_size;
  next->required = 1;
  next->shortlen = strlen(next->line);
 
  return(0);
 
} /* history_line_new */

/*---------------------------------------------------------------------------
NAME
 
  history_line_add
 
SYNOPSIS
 
  int history_line_add  ( HSymb ** proot, const char * history_line_key,
                        const char * substring )
 
DESCRIPTION
Appends a space and a new item to the end of the history line.
If the history line becomes longer than size bytes (including trailing zero)
the rest is skipped.
 
RETURN VALUES
Returns 0 in case of success and -1 if no success.
 
---------------------------------------------------------------------------*/
int history_line_add  ( HSymb ** proot, const char * history_line_key,
                        const char * substring )
{ HSymb * next = *proot;
  int notfound = 1;
  size_t n, linelen;
 
  /* search history line (next) */
  while( ( next!=(HSymb *) NULL ) && (notfound>0) ) {
    notfound = compare_keys(next->key,history_line_key,UpperCaseSort);
    if (notfound>0) {next = next->Next;}
    }
 
  /* add arguments */
  if ( next!=(HSymb *) NULL ) {
 
    /* append space if string not empty */
    linelen = STRLEN(next->line);
    if ((linelen>0) && (linelen<next->size-1)) strcat(next->line, " ");
 
    n = (size_t) MAX(0l,MIN((long) next->size - (long) STRLEN(next->line) - 1l,
                            (long) STRLEN(substring)));
 
    if (n>0) {
      strncat(next->line, substring, n );
      strcat(next->line, "\0");
      }
 
    /* update shortlen if argument is required */
    if (next->required) next->shortlen = strlen(next->line);
    next->required = 1;
    }
 
  return(0);
 
} /* history_line_add */

/*+++------------------------------------------------------------------------
NAME
 
   edf_history_new
 
SYNOPSIS
   int edf_history_new ( const char * history_key )
 
DESCRIPTION
This routines must be called first. If not already initialized, it
initializes all history lists. Existing history lines are
removed and a new empty history line with size MaxHistoryLineSize is
created. 'edf_history_argv' adds parameters to this line.
'edf_write_header_history' appends it with a new key to the
history lines that were read with 'edf_read_header_history'.
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
 
--------------------------------------------------------------------------+*/
int edf_history_new          ( const char * history_key )             /*---*/
{  HBlock * hblock;

   if (HIST_debug) printf("\n edf_history_new BEGIN\n");
 
   /* init history module */
   if (!InitHistory) init_history();
   /* create an empty history block */
   if (insert_history_block( history_key, &hblock )) return(0);
   /* create empty history line */ 
   if (history_line_new  ( &hblock->history_argv_root, argv_key, 
                           MaxHistoryLineSize, (HSymb **) NULL ) ) return(0);
 
   /* default history key */
   if (hblock->current_history_line_key) 
     free(hblock->current_history_line_key);
   hblock->current_history_line_key = newstr(default_history_line_key);
 
   if (HIST_debug) printf("\n edf_history_new END\n");
 
   return(1);
 
} /* edf_history_new */

/*---------------------------------------------------------------------------
NAME
 
  history_line_required
 
SYNOPSIS
 
  int history_line_required ( HSymb ** proot, const char * history_line_key,
                              int required )
 
DESCRIPTION
Sets the parameter 'required' of 'history_line_key' to required.
If 'required' is 1,
  the next call to history_line_add will update the parameter 'shortlen',
if 'required' is 0,
  the next call to history_line_add will not update the parameter 'shortlen'
 
RETURN VALUES
Returns 0 in case of success and -1 if no success.
---------------------------------------------------------------------------*/
int history_line_required  ( HSymb ** proot, const char * history_line_key,
                             int required )
{ HSymb * next = *proot;
  int notfound = 1;
 
  /* search history line (next) */
  while( ( next!=(HSymb *) NULL ) && (notfound>0) ) {
    notfound = compare_keys(next->key,history_line_key,UpperCaseSort);
    if (notfound>0) {next = next->Next;}
    }
 
  if ( next!=(HSymb *) NULL ) {
    next->required = required;
    }
 
  return(0);
 
} /* history_line_required */

/*+++------------------------------------------------------------------------
NAME
 
   edf_history_skip
 
SYNOPSIS
 
   int edf_history_skip ( const char * history_key )
 
DESCRIPTION
This routines marks the next parameter that is passed to edf_history_argv as
not required.
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
--------------------------------------------------------------------------+*/
int edf_history_skip (  const char * history_key  )                   /*---*/
{ HBlock * hblock;

  if (HIST_debug) printf("\n edf_history_skip BEGIN\n");
  if (!InitHistory) init_history();
 
  /* search block 'history_key' */
  if (search_history_block( history_key, &hblock )) return(0);

  if (history_line_required ( &hblock->history_argv_root, argv_key, 0 )) 
    return(0);
 
  if (HIST_debug) {
    print_history_list( stdout, 1, 1, hblock->history_argv_root );
    }
  if (HIST_debug) printf("\n edf_history_skip END\n");
 
  return(1);
 
} /* edf_history_skip */

/*+++------------------------------------------------------------------------
NAME
 
   edf_history_take
 
SYNOPSIS
 
   int edf_history_take ( const char * history_key )
 
DESCRIPTION
This routines marks the next parameter that is passed to edf_history_argv as
required (opposite of edf_history_skip)
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
--------------------------------------------------------------------------+*/
int edf_history_take ( const char * history_key )                     /*---*/
{ HBlock *hblock;

  if (HIST_debug) printf("\n edf_history_take BEGIN\n");
  if (!InitHistory) init_history();

  /* search block 'history_key' */
  if (search_history_block( history_key, &hblock )) return(0);
 
  if (history_line_required ( &hblock->history_argv_root, argv_key, 1 )) 
    return(0);
 
  if (HIST_debug) {
    print_history_list( stdout, 1, 1, hblock->history_argv_root );
    }
  if (HIST_debug) printf("\n edf_history_take END\n");
 
  return(1);
 
} /* edf_history_take */

/*+++------------------------------------------------------------------------
NAME
 
  edf_history_free
 
SYNOPSIS
 
  int edf_history_free ( const char * history_key )
 
DESCRIPTION
Removes all history lines from history_key.
If called with history_key == (char *) NULL, all history blocks are removed
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
 
--------------------------------------------------------------------------+*/
int edf_history_free         ( const char * history_key )             /*---*/
{ HBlock * next, * hblock;

  if (HIST_debug) printf("\n edf_history_free BEGIN\n");
 
  if (!InitHistory) init_history();

  if ( history_key != (char *) NULL ) {
    /* search block 'history_key' */
    if (search_history_block( history_key, &hblock )) return(0);
    /* clear and remove history block */
    if (remove_history_block( hblock )) return(0);
   } else {
    next = history_root;
    while ( next != (HBlock *) NULL ) {
      hblock = next;
      next = hblock->Next;
      /* clear and remove history block */
      if (remove_history_block( hblock )) return(0);
      }
   }

  if (HIST_debug) printf("\n edf_history_free END\n");
 
  return( 1 );
 
} /* edf_history_free */

/*+++------------------------------------------------------------------------
NAME
 
   edf_read_header_history
 
SYNOPSIS
 
   int edf_read_header_history  (int stream, long int DataNumber,
                                 int DataChain, const char * history_key,
                                 int * pErrorValue, int * pstatus )
 
DESCRIPTION
Reads all history lines from the date file header and copies them to
'hline->key's. History lines have the keyword HISTORY_KEY_PREFIX'u',
where 'u' is an unsigned positive integer. A new history line with the
key HISTORY_KEY_PREFIX'last+1' and MaxHistoryLineSize bytes is created.
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
 
--------------------------------------------------------------------------+*/
int edf_read_header_history  (int stream, long int DataNumber,
                              int DataChain, const char * history_key,
                              int * pErrorValue, int * pstatus )      /*---*/
{  HBlock *hblock;
 
   char history_line[MaxHistoryLineSize];
   char history_line_key_buffer[HistoryKeyBufferSize];
   long depth=1;

   const char *String;
 
   if (HIST_debug) printf("\n edf_read_header_history BEGIN\n");
 
   *pErrorValue = RoutineSucceeded;
   *pstatus     = status_error;
 
   if (!InitHistory) init_history();
 
   /* search block 'history_key' */
   if (search_history_block( history_key, &hblock )) return(0);
 
   if (clear_header_history ( hblock ) ) return(0);
 
   sprintf(history_line_key_buffer,"%s%lu",HISTORY_KEY_PREFIX,depth++);
 
   while ( edf_read_header_string ( stream, DataNumber, DataChain,
                                  history_line_key_buffer, &String,
                                  pErrorValue, pstatus )) {
     if (*pstatus != status_success) return(0);
     /* Copy string to history line */
     if (String) {
       strncpy(history_line,String,MaxHistoryLineSize-1);
       history_line[MaxHistoryLineSize-1]='\0';
     } else history_line[0]='\0';
     if (history_line_new( &hblock->history_line_root, history_line_key_buffer,
                          STRLEN(history_line)+1, (HSymb **) NULL )) return(0);
     if (history_line_add( &hblock->history_line_root, history_line_key_buffer,
                            history_line)) return(0);
     sprintf(history_line_key_buffer,"%s%lu",HISTORY_KEY_PREFIX,depth++);
     }
 
  /* save current history key */
  if (hblock->current_history_line_key) free(hblock->current_history_line_key);
  hblock->current_history_line_key = newstr(history_line_key_buffer);
 
  if (HIST_debug) {
    printf(" history block key        = %s\n",hblock->key);
    printf(" current_history_line_key = %s\n",
           hblock->current_history_line_key);
    print_history_list( stdout, 1, 1, hblock->history_argv_root );
    print_history_list( stdout, 1, 1, hblock->history_line_root );
    }
 
  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_success;
 
  if (HIST_debug) printf("\n edf_read_header_history END\n");
 
  return(1);
 
} /* edf_read_header_history */

/*+++------------------------------------------------------------------------
NAME
 
   edf_write_header_history
 
SYNOPSIS
   int edf_write_header_history   ( int stream, long int DataNumber,
                                    int DataChain, const char * history_key,
                                    int * pErrorValue, int * pstatus )
 
DESCRIPTION
Writes the history strings in 'history_line_root' into the date file header
using the 'hline->key's as keywords. Writes the history string in
'history_argv_root' into the date file header using 'current_history_line_key'
as keyword.
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
 
--------------------------------------------------------------------------+*/
int edf_write_header_history   ( int stream, long int DataNumber,
                                 int DataChain, const char * history_key,
                                 int * pErrorValue, int * pstatus )   /*---*/
{ HBlock * hblock;
  HSymb * next;
  int retval = 1;
 
  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_error;
 
  if (HIST_debug) printf("\n edf_write_header_history BEGIN\n");
 
  if (!InitHistory) init_history();
 
  /* search block 'history_key' */
  if (search_history_block( history_key, &hblock )) return(0);
 
  if (HIST_debug) {
    printf(" history block key        = %s\n",hblock->key);
    printf(" current_history_line_key = %s\n",
           hblock->current_history_line_key);
    print_history_list( stdout, 1, 1, hblock->history_argv_root );
    print_history_list( stdout, 1, 1, hblock->history_line_root );
    }
 
  /* history_argv_root */
  if (( hblock->history_argv_root != (HSymb *) NULL ) &&
      ( hblock->current_history_line_key != (char *) NULL ) &&
      ( hblock->history_argv_root->line != (char *) NULL ) ) {
    /* write only non-empty history lines */
    if (strlen(hblock->history_argv_root->line)>0) {
      /* remove not required arguments */
      hblock->history_argv_root->line[hblock->history_argv_root->shortlen]='\0';
      if (!(edf_write_header_string ( stream, DataNumber, DataChain,
                                    hblock->current_history_line_key,
                                    hblock->history_argv_root->line,
                                    pErrorValue, pstatus ) ) ) return( retval );
      }
    }
 
  /* history_line_root */
  next = hblock->history_line_root;
  while ( next!=(HSymb *) NULL ) {
    if (!(edf_write_header_string ( stream, DataNumber, DataChain,
                                  next->key, next->line,
                                  pErrorValue, pstatus ) ) ) return( retval );
    next = next->Next;
    }
 
  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_success;
 
  if (HIST_debug) printf("\n edf_write_header_history END\n");
 
  return( retval );
 
} /* edf_write_header_history */

/*+++------------------------------------------------------------------------
NAME
 
   edf_history_read_header
 
SYNOPSIS
 
   int edf_history_read_header  ( const char * header_key,
                                  const char * history_key,
                                  int * pErrorValue, int * pstatus);
 
DESCRIPTION
Reads all history lines from the header header_key and copies them to
'hline->key's. History lines have the keyword HISTORY_KEY_PREFIX'u',
where 'u' is an unsigned positive integer. A new history line with the
key HISTORY_KEY_PREFIX'last+1' and MaxHistoryLineSize bytes is created.
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
 
--------------------------------------------------------------------------+*/
int edf_history_read_header  ( const char * header_key,
                               const char * history_key,
                               int * pErrorValue, int * pstatus)      /*---*/
{  HBlock *hblock;

   const char * history_line;
   char history_line_key_buffer[HistoryKeyBufferSize];
   long depth=1;
 
   if (HIST_debug) printf("\n edf_history_read_header BEGIN\n");

   *pErrorValue = RoutineSucceeded;
   *pstatus     = status_error;
 
   if (!InitHistory) init_history();

   /* search block 'history_key' */
   if (search_history_block( history_key, &hblock )) return(0);
 
   if (clear_header_history ( hblock ) ) return(0);
 
   sprintf(history_line_key_buffer,"%s%lu",HISTORY_KEY_PREFIX,depth++);
 
   while ( edf_search_header_element( header_key,
                                      history_line_key_buffer, &history_line,
                                      pErrorValue, pstatus )) {
     if (*pstatus != status_success) return(0);
     if (history_line_new( &hblock->history_line_root, history_line_key_buffer,
                          STRLEN(history_line)+1, (HSymb **) NULL )) return(0);
     if (history_line_add( &hblock->history_line_root, history_line_key_buffer,
                            history_line)) return(0);
     sprintf(history_line_key_buffer,"%s%lu",HISTORY_KEY_PREFIX,depth++);
     }
 
  /* save current history key */
  if (hblock->current_history_line_key) free(hblock->current_history_line_key);
  hblock->current_history_line_key = newstr(history_line_key_buffer);
 
  if (HIST_debug) {
    printf(" history block key        = %s\n",hblock->key);
    printf(" current_history_line_key = %s\n",
           hblock->current_history_line_key);
    print_history_list( stdout, 1, 1, hblock->history_argv_root );
    print_history_list( stdout, 1, 1, hblock->history_line_root );
    }
 
  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_success;

  if (HIST_debug) printf("\n edf_history_read_header END\n");
 
  return(1);
 
} /* edf_history_read_header */

/*+++------------------------------------------------------------------------
NAME
 
   edf_history_write_header
 
SYNOPSIS
   int edf_history_write_header ( const char * header_key,
                                  const char * history_key,
                                  int * pErrorValue, int * pstatus );
 
DESCRIPTION
Writes the history strings in 'history_line_root' into the date file header
using the 'hline->key's as keywords. Writes the history string in
'history_argv_root' into the date file header using 'current_history_line_key'
as keyword.
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
 
--------------------------------------------------------------------------+*/
int edf_history_write_header ( const char * header_key,
                               const char * history_key,
                               int * pErrorValue, int * pstatus)      /*---*/
{ HBlock * hblock;
  HSymb * next;
  int retval = 1;
 
  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_error;
 
  if (HIST_debug) printf("\n edf_history_write_header BEGIN\n");
 
  if (!InitHistory) init_history();

  /* search block 'history_key' */
  if (search_history_block( history_key, &hblock )) return(0);
 
  if (HIST_debug) {
    printf(" history block key        = %s\n",hblock->key);
    printf(" current_history_line_key = %s\n",
           hblock->current_history_line_key);
    print_history_list( stdout, 1, 1, hblock->history_argv_root );
    print_history_list( stdout, 1, 1, hblock->history_line_root );
    }
 
  /* history_argv_root */
  if (( hblock->history_argv_root != (HSymb *) NULL ) &&
      ( hblock->current_history_line_key != (char *) NULL ) &&
      ( hblock->history_argv_root->line != (char *) NULL ) ) {
    /* write only non-empty history lines */
    if (strlen(hblock->history_argv_root->line)>0) {
      /* remove not required arguments */
      hblock->history_argv_root->line[hblock->history_argv_root->shortlen]='\0';
      if (!( edf_add_header_element ( header_key,
                                      hblock->current_history_line_key, 
                                      hblock->history_argv_root->line,
                                      pErrorValue, pstatus ) ) ) return(retval);
      }
    }
 
  /* history_line_root */
  next = hblock->history_line_root;
  while ( next!=(HSymb *) NULL ) {
    if (!( edf_add_header_element ( header_key, next->key, next->line,
                                    pErrorValue, pstatus ) ) ) return( retval );
    next = next->Next;
    }
 
  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_success;
 
  if (HIST_debug) printf("\n edf_history_write_header END\n");
 
  return( retval );
 
} /* edf_history_write_header */

/*+++------------------------------------------------------------------------
NAME
 
  edf_history_copy --- copies a history block
 
SYNOPSIS

  int edf_history_copy ( const char * history_key_copy,
                         const char * history_key );
 
DESCRIPTION
  Copies the history block history_key to a new or existing history block
with the name history_key_copy. The existing contents of history_key_copy
will be lost.
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
 
--------------------------------------------------------------------------+*/
int edf_history_copy ( const char * history_key_copy,
                       const char * history_key )                     /*---*/
{ HBlock *hcopy, *hblock;

  if (HIST_debug) printf("\n edf_history_copy BEGIN\n");
 
  if (!InitHistory) init_history();
 
  /* search block 'history_key' */
  if (search_history_block( history_key, &hblock )) return(0);

  /* copy hblock to history_key_copy */
  if (copy_history_block( history_key_copy, &hcopy, hblock )) return(0);

  if (HIST_debug) {
    printf(" copied history block\n");
    printf(" history block key        = %s\n",hcopy->key);
    printf(" current_history_line_key = %s\n",
           hcopy->current_history_line_key);
    print_history_list( stdout, 1, 1, hcopy->history_argv_root );
    print_history_list( stdout, 1, 1, hcopy->history_line_root );
    }
 
  if (HIST_debug) printf("\n edf_history_copy END\n");

  return(1);

} /* edf_history_copy */
 
/*+++------------------------------------------------------------------------
NAME
 
  edf_history_argv --- appends an argument to history line
 
SYNOPSIS
  int edf_history_argv ( const char * history_key, const char * argument )
 
DESCRIPTION
Appends argument to history line
 
RETURN VALUES
In case of success the return value is 1, otherwise 0.
 
--------------------------------------------------------------------------+*/
int edf_history_argv ( const char * history_key,
                       const char * argument )                        /*---*/
{ HBlock *hblock;
  char * buffer = (char *) NULL;
 
  if (HIST_debug) printf("\n edf_history_argv BEGIN\n");
 
  if (!InitHistory) init_history();

  /* search block 'history_key' */
  if (search_history_block( history_key, &hblock )) return(0);
 
  if (needquotes(argument)) {
    /* add double quotes */
    if (!(buffer = (char *) malloc ( strlen(argument)+3 ))) return(0);
    sprintf(buffer,"\"%s\"",argument);
    if (history_line_add  ( &hblock->history_argv_root, argv_key,
                            buffer )) return(0);
    free(buffer);
    } else {
    /* do not add quotes */
    if (history_line_add  ( &hblock->history_argv_root, argv_key,
                            argument )) return(0);
    }
 
  if (HIST_debug) {
    print_history_list( stdout, 1, 1, hblock->history_argv_root );
    }
 
  if (HIST_debug) printf("\n edf_history_argv END\n");
 
  return(1);
 
} /* edf_history_argv */

/*+++------------------------------------------------------------------------
NAME
 
   edf_history_print
 
SYNOPSIS
 
   int edf_history_print       ( FILE * out, const char * history_key,
                                 int level, int verbose )
 
DESCRIPTION
 
   Prints contents of history 'history_key' to 'out', or contents of
   all histories, if 'history_key' is NULL.
 
RETURN VALUE
    1: success
    0: failed (specified history not found)
---------------------------------------------------------------------------*/
int edf_history_print       ( FILE * out, const char * history_key,
                              int level, int verbose )
{ const char * SeparationLine =
    "-   -   -   -   -   -   -   -   -   -   -   -   -   -   -";
 
  HBlock * hblock, * stop = (HBlock*) NULL;
 
  if (HIST_debug) printf("edf_history_print\n");
 
  if (level<1) return(0);
 
  if (!InitHistory) init_history();
 
  if ( history_key != (const char *) NULL ) {
    // list a single history block 
    if (search_history_block( history_key, &hblock )) return(0);
    if ( hblock == (HBlock *) NULL ) return(0);
    stop = hblock->Next;
  } else hblock = history_root; // list all history blocks
 
  while ( hblock != stop ) {
    if (verbose) {
      fprintf(out,"  %s\n",SeparationLine);
      fprintf(out,"  History block key  = %s\n",hblock->key);
      fprintf(out,"  Previous Key       = ");
      if ((hblock->Previous)!=(HBlock*) NULL)
        fprintf(out,"%s\n", hblock->Previous->key);
        else fprintf(out,"(no previous history block)\n");
      fprintf(out,"  Next Key           = ");
      if ((hblock->Next)!=(HBlock*) NULL)
        fprintf(out,"%s\n", hblock->Next->key);
        else fprintf(out,"(no next history block)\n");
      print_history_list( out, level-1, verbose, hblock->history_line_root );
      print_history_list( out, level-1, verbose, hblock->history_argv_root );
      printf(" current_history_line_key = %s\n",
           hblock->current_history_line_key);
      } else {
      fprintf(out,"  History block key  = %s\n",hblock->key);
      print_history_list( out, level-1, verbose, hblock->history_line_root );
      print_history_list( out, level-1, verbose, hblock->history_argv_root );
      printf(" current_history_line_key = %s\n",
           hblock->current_history_line_key);
      }
    hblock=hblock->Next;
    }
 
  if (verbose) fprintf(out,"   %s\n",SeparationLine);

  if (HIST_debug) printf("\n edf_history_print END\n"); 
 
  return(1); 

} /* edf_history_print */
 
/*===history END=============================================================*/

/****************************************************************************/

/*---------------------------------------------------------------------------
NAME

   renorm_data_array --- Convert data into a machine readable form

SYNOPSIS

   MType mtype;
   int renorm_data_array ( DBlock * block, void * buffer, size_t buflen,
                           int mtype, int * pErrorValue, int * pstatus );

DESCRIPTION
  Convert the data in 'block' into a machine readable form of type 'mtype'.
  - endian correction
  - binary decompression (not implemented)
  - raster order normalization 
  - conversion of data to mtype

If 'buffer' is not the NULL-pointer the converted data are always written to
'buffer' and  block->Flags.ExternalDataAlloc is set. 'buflen' is the available 
memory in bytes. If the input data buffer was internally allocated it is 
released. 
If block->Flags.ExternalDataAlloc is set and 'buffer' is NULL the converted 
data are written back to the input data buffer. No additional memory is 
allocated. 

The routine exits with an error if the buffer length is too small 
(DataBufferTooSmall). 

An endian correction is applied to the data if ByteOrder is different from 
the InternalByteOrder of the machine. The data is decompressed, raster
order is normalized and the data are converted into a machine readable
form ('mtype').

ERROR VALUES
  int * pErrorValue    returned ErrorValue,
  int * pstatus        return status

HISTORY
  01-Jan-2001 PB       extracted from read_data_array
  --------------------------------------------------------------------------+*/
int renorm_data_array ( DBlock * block, void * buffer, size_t buflen,
                        int mtype, int * pErrorValue, int * pstatus )
{ static const char * RDA_Error = "ERROR: renorm_data_array:";
  int    internal_byte_order; 

  void    *als[4] = { (void *) NULL };
  int      i, alc = 0;

  long    *data_dim;
  long     data_value_offset;
  long     raster_configuration;

  size_t   data_size, data_len;
  unsigned long   data_number;
  size_t   data_buffer_len, data_out_len;

  void    *current = (void*) NULL, *dest = (void*) NULL, *tmp;
  size_t   current_len = (size_t) 0, dest_len = (size_t) 0, tmp_len; 

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  internal_byte_order = byteorder();

  /* check whether 'block' exists */
  if (!block) return(-1);

  /* check, if data available */
  if ( block->Flags.DataExternal ) { // data must be read first
      *pErrorValue = ExternalData; return(-1); }

  /* do not use buffer if it is identical to block->Data */
  if (block->Data == buffer) { // do not use buffer
    buffer = (void *) NULL; buflen = (size_t) NULL;
    }

  /* get binary data description */
  data_value_offset = block->DataValueOffset;
  raster_configuration = block->DataRasterConfiguration;

  if (!block->DataDim) {
    *pErrorValue = MissingArrayDimensions; return(-1); }
  data_dim = block->DataDim;

  data_size   = edf_data_sizeof ( block->DataType );
  data_number = edf_dim_product ( data_dim );
  data_len    = data_size * data_number ;
  data_buffer_len = block->DataBufferLen;
  data_out_len = edf_machine_sizeof(mtype)*data_number;

  /* test output buffer size */
  if ( buffer == (void *) NULL ) {
    if (block->Flags.ExternalDataAlloc) { // use input data buffer for output
      if (data_buffer_len<data_out_len) {
//        fprintf(stderr,"%s allocated buffer < %zu bytes\n",
        fprintf(stderr,"%s allocated buffer < %zu | %lu bytes\n",
               RDA_Error,data_out_len,data_out_len);return(-1);
        }
      } 
    } else { // use supplied buffer for output
    if (buflen<data_out_len) {
//      fprintf(stderr,"%s allocated buffer < %zu bytes\n",
      fprintf(stderr,"%s allocated buffer < %zu | %lu bytes\n",
             RDA_Error,data_out_len,data_out_len);return(-1);
      }
    }

  /* toggle data between current and dest */
  current     = block->Data;
  current_len = data_buffer_len;

  /* --- endian correction */
  if ( block->DataByteOrder != internal_byte_order ) 
    edf_bswap ( current, current, data_size, data_number );
  block->DataByteOrder = internal_byte_order;

  /* --- binary decompression */
  if ( block->DataCompression != UnCompressed ) {
    fprintf(stderr,"%s Compression = %s is not implemented\n", RDA_Error,
            Compression2String( block->DataCompression )); return(-1); }

  /* --- raster order normalization */
  if (block->DataRasterConfiguration!=1l) {
    /* input and output memory must be different, allocate new memory */
    dest_len = data_len;
    if (!(dest = malloc( dest_len ))) {
      *pErrorValue = CouldNotMallocMemory;
//      fprintf(stderr,"%s malloc of %zu bytes failed\n",RDA_Error,dest_len);return(-1); }
      fprintf(stderr,"%s malloc of %zu | %lu bytes failed\n",
        RDA_Error,dest_len,dest_len);return(-1); }
    als[alc++] = dest; // add allocated memory

    if (edf_raster_normalization ( dest, current, data_dim,
                            raster_configuration, data_size)) {
      for ( i=alc; i>0; --i ) if (als[i]) free(als[i]); return(-1); } 
    tmp = current; current = dest; dest = tmp;
    tmp_len = current_len; current_len = dest_len; dest_len = tmp_len;
    } /* if (raster_configuration!=1) */
  block->DataRasterConfiguration = 1l;

  /* last step, if 'buffer' is set destination should be buffer, otherwise
     destination should be 'block->Data', source is 'current' */
  if ( buffer==(void *) NULL ) {
    dest = block->Data; 
    dest_len = data_buffer_len; 
    } else {
    dest = buffer;
    dest_len = buflen;
    if (dest_len<data_out_len) {
//      fprintf(stderr,"%s supplied buffer < %zu bytes\n",RDA_Error,data_out_len);
      fprintf(stderr,"%s supplied buffer < %zu | %lu bytes\n",
        RDA_Error,data_out_len,data_out_len);
      return(-1); }
    }

  /* --- data conversion to mtype*/
  if (dest_len<data_out_len) { /* allocate new memory for output */
    dest_len = data_out_len;
    if (!(dest = malloc ( dest_len ))) {
      for ( i=alc; i>0; --i ) if (als[i]) free(als[i]);       
      *pErrorValue = CouldNotMallocMemory;
//      fprintf(stderr,"%s malloc of %zu bytes failed\n",RDA_Error,dest_len);return(-1); 
      fprintf(stderr,"%s malloc of %zu | %lu bytes failed\n",
        RDA_Error,dest_len,dest_len);
      return(-1); 
    }
    als[alc++] = dest; // add allocated memory
    } /* if (dest_len ... */

  if ( edf_machine2machine ( dest, mtype,
                             current, data_value_offset,
                             edf_datatype2machinetype(block->DataType), 
                             data_number ) ) {
     for ( i=alc; i>0; --i ) if (als[i]) free(als[i]);        
     *pErrorValue = DataConversionFailed; return(-1); }

  if ( dest!=block->Data ) { // relink new memory
    // release only internally allocated data buffer
    if (!block->Flags.ExternalDataAlloc) free(block->Data); 
    block->Data = dest; 
    block->DataBufferLen = dest_len;
    if (dest==buffer) block->Flags.ExternalDataAlloc = True;
      else block->Flags.ExternalDataAlloc = False;
    /* remove from allocated memory list */
    for ( i=alc; i>0; --i ) if (als[i]==dest) als[i]=(void *) NULL;
    }

  /* update data block description */
  block->DataLen = data_out_len;
  block->DataValueOffset = 0l;
  block->DataType = InValidDType;
  block->Flags.DataExternal = True;

  /* release temporary memory */
  for ( i=alc; i>0; --i ) if (als[i]) free(als[i]); 

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(0);

} /* renorm_data_array */

/*---------------------------------------------------------------------------
NAME

   read_data_array --- read data array (raw, without conversion)

SYNOPSIS

  int read_data_array ( DBlock * block, void *buffer, size_t buflen, 
                        int must_use_buffer, long dim[],
                        int * pErrorValue, int * pstatus )
 
DESCRIPTION
  Reads a data array from the block 'DataNumber' in 'DataChain'.
  If ( block->DataExternal ) the data is read from the file, if not, it is 
  taken directy from 'block'.

  buffer is an optional data buffer. If buffer!=NULL buflen describes the
  available memory in bytes and 'must_use_buffer' is a flag that indicates
  whether this buffer must be used or whether it is optional. The data are 
  written to buffer. If its size is too short and the flag 'must_use_buffer'
  is set the routine stops with an error, otherwise an internal buffer with
  correct length is allocated. If buffer is NULL internal memory is always
  allocated. Internally allocated memory is released automatically after 
  the next write, when a different data block is accessed. or when the 
  stream is closed.

  If dim!=NULL the dimension array is copied to dim. The first element 
  dim[0] must contain the maximum image dimension, e.g. 2 for a 2d array. 
  If dim is too short the routine exits with an error. In all other cases 
  a correct dimension array is allocated and available in block->DataDim. 

PARAMETERS
  DBlock * block       Current data block
  void *buffer         Data buffer 
                       input: buffer == NULL 
                              -> memory allocated and pointer returned
                              buffer != NULL
                              -> up to buflen bytes are written to *buffer
                                 (buflen is data buffer size in bytes)
                                 If 'must_use_buffer' is False memory is
                                 allocated if external buffer is too short.
  size_t buflen        Size of input data buffer in bytes.
  int must_use_buffer  True: use always the supplied buffer
                       Fallse: use supplied buffer only if it is big enough
  long dim[]           Optional dimension array for output
                       dim[0]                length of the dimension array
                       dim[1]                length of dimension 1
                        ...
                       dim[dim[0]]   length of last dimension
                       input: dim == NULL
                       -> not used
                              dim != NULL
                       -> data array dimension is copied to dim.
                          dim[0] must contain the dimensions. dim[0] 
                          dimensions. Unused dimensions are set to 1, error 
                          if too short.
 
ERROR VALUES
  int * pErrorValue    returned ErrorValue, 
                       only used when *pstatus!=status_success. 
                       CouldNotGetBinaryArray: routine get_binary_array failed.
  int * pstatus        return status

AUTHOR
  05-Mar-1998 Peter Boesecke 
  30-Apr-1998 PB       no conversion for ConversionType==0, 
                       parameter pDataType added
  23-Jul-1999 PB       data_value_offset_in added
  30-Apr-2000 PB       Parameter ConversionType renamed to mtype
  12-Nov-2000 PB       DataDim is not unlinked any more
  12-Nov-2000 PB       DBlock * block instead of DataChain etc.
  01-Jan-2001 PB       totally redesigned, no conversion any more
  --------------------------------------------------------------------------+*/
int read_data_array ( DBlock * block, void *buffer, size_t buflen, 
                      int must_use_buffer, long dim[],
                      int * pErrorValue, int * pstatus )
{ 
  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  /* check whether 'block' exists */
  if (!block) return(-1);

  /* get data array */
  if ( block->Flags.DataExternal ) {
    if ( get_binary_array ( block, buffer, buflen, must_use_buffer ) ) {
      *pErrorValue = CouldNotGetBinaryArray; return(-1); }
    } 

  /* copy dimensions to dim */
  if (dim) { // copy dimensions to dim 
    if ( !(copydim( dim, dim[0], block->DataDim ) ) ) {
      *pErrorValue = NotNdData; return(-1);
      } 
    } 

  *pstatus     = status_success;
  *pErrorValue = RoutineSucceeded;

  return(0);

} /* read_data_array */

/****************************************************************************/

/*---------------------------------------------------------------------------
NAME

   free_data_file --- deallocates and reinitializes an edf file.

SYNOPSIS

   int free_data_file ( DFile * file );

DESCRIPTION
Deallocates all memory of a DFile. If it is already done nothing happens.

RETURN VALUE
success:   int 0
error:     int -1
---------------------------------------------------------------------------*/
int free_data_file ( DFile * file )
{
  /* remove chainlist */
  if (free_data_chain_list( file )) return(-1);

  /* close file */
  if (file->Channel!=NULL)
    if (fclose(file->Channel)) return(-1);
  file->Channel = (FILE *) NULL;

  /* release IO-buffer */
  if (file->Buffer!=NULL)
    free(file->Buffer);
  file->Buffer = (char *) NULL;

  /* release file name */
  if (file->Name!=NULL)
    free(file->Name);
  file->Name = (char *) NULL;

  init_file ( file );

  return(0);

} /* free_data_file */

/*---------------------------------------------------------------------------
NAME

   open_as_bsl_file --- reads the file as a bsl file 

SYNOPSIS

   int open_as_bsl_file (  const char *fname, const char * mode,
                           int *pErrorValue, int *pstatus );

DESCRIPTION
Opens 'fname' as a BSL file and, if successful, creates an edf file structure 
with chains corresponding to bsl memories and data blocks corresponding to
bsl frames. The translation is done in the following way:

memnum franum    chain key   block key  block id
1      n         image.psd   n          n.image.psd
m      n         image.m     n          n.image.m

The data block headers exist only in memory, they are not read a second time
from the file. The only allowed opening mode is 'Old'. 

ERROR VALUES
*pErrorValue
*pstatus

RETURN VALUE
success:   int  stream (Channel not opened)
error:     int  -1

HISTORY
22-Mar-1998 Peter Boesecke
17-Mar-2000 chain key changed from image.psd.m to image.m
---------------------------------------------------------------------------*/
int open_as_bsl_file (  const char *fname, const char * mode,
                        int *pErrorValue, int *pstatus )
{ int stream=-1, bsl_stream;
  long memnum, minmem, maxmem;
  long franum, minfra, maxfra;
  BslDataSpec *data_spec;
  char        *first_header, *second_header;

  char   chain_key[MaxKeyLen+1];
  char   block_key[MaxKeyLen+1];
  char   symbol_key[MaxKeyLen+1];
  char   symbol_value[MaxValLen+1];

  DFile * file;
  DChain * chain;
  DBlock * block;
  SElement * symbol;

  int i;

  *pstatus = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("open_as_bsl_file\n");

  if ( (STRCMP(mode,Old)!=0) && (STRCMP(mode,Read)!=0) ) {
    *pErrorValue = CannotOpenAsBslFile; return ( stream );
    }

  if (!InitTable) init_file_table( FileTable );
  if ( ( stream=search_free_stream( FileTable ) ) < 0 ) {
    *pErrorValue = NoMoreStreamsAvailable; return(-1);
    }

  if (EDFIO_debug) printf("\"%s\" : stream = %d (%s)\n",fname,stream,mode);

  file = &FileTable[stream];

  init_file ( file );
  file->Used = True;

  if ( !(file->Name = newstr( fname ) ) ) return(-1);

  /* set general flags */
  file->Flags.NoGeneralHeader = True;

  /* create general header */
  if ( new_general_block( file ) ) {
      *pErrorValue = ErrorCreatingGeneralBlock; return(-1);
      }

  bsl_stream = open_bsl_file ( fname , "read" );
  if (bsl_stream<0) { *pErrorValue = CannotOpenAsBslFile; return( stream ); }

  /* create edf file structure */

  read_bsl_file_headers( bsl_stream, &first_header, &second_header );

  if (EDFIO_debug) print_bsl_filetable ( stdout, 4, True );

  insert_string ( file->GeneralBlock, "Title", first_header,  &symbol );
  insert_string ( file->GeneralBlock, "SubTitle", second_header, &symbol );

  if (bsl_memory_range( bsl_stream, &minmem, &maxmem )) return(-1);

  for (memnum = minmem; memnum<=maxmem; memnum++) {
    if (bsl_frame_range( bsl_stream, memnum, &minfra, &maxfra )) return(-1);
 
    if (!(default_chain_key(chain_key, memnum))) return(-1);
    if ( insert_data_chain( file, chain_key, &chain ) ) return(-1);
 
    for (franum = minfra; franum<=maxfra; franum++) {
      data_spec = read_bsl_data_spec ( bsl_stream,  memnum, franum );
      if (!data_spec) return(-1);

      sprintf( block_key, "%ld", franum);
      if ( insert_data_block( chain, block_key, &block ) ) return(-1);

      block->Flags.InternalHeader = True;
      block->Flags.DataExternal   = True;
      block->BinaryFileName       = newstr(data_spec->BinaryFileName);
      block->BinaryFilePos        = data_spec->BinaryFilePos;
      block->BinaryFileLen        = data_spec->BinaryFileLen;
  
      for (i=1;i<=data_spec->Dim[0];i++) { 
        sprintf(symbol_key,"%s%1u",DIMENSION_KEY_PREFIX,i);  
        long2s(symbol_value,data_spec->Dim[i]);

        if (insert_string ( block, symbol_key, symbol_value, &symbol ))
           return(-1); 
        }

      if (insert_string ( block, DATA_TYPE_KEY, 
          edf_datatype2string( data_spec->DataType ), &symbol )) return(-1);

      if ( bsl_input_byteorder == InValidBOrder ) {
        if (insert_string ( block, BYTE_ORDER_KEY,
            ByteOrder2String( data_spec->ByteOrder ), &symbol )) return(-1); }
       else { 
        if (insert_string ( block, BYTE_ORDER_KEY,
            ByteOrder2String( bsl_input_byteorder ), &symbol )) return(-1); }

      if (insert_string ( block, COMPRESSION_KEY,
          Compression2String( data_spec->Compression ), &symbol )) return(-1);

      if (data_spec->RasterConfiguration!=1l) {
        sprintf(symbol_value,"%ld",data_spec->RasterConfiguration);
        if (insert_string ( block, RASTER_CONFIGURATION_KEY,
                            symbol_value, &symbol ) ) return(-1);
        }

    } /* for franum */
  } /* for memnum */

  if (close_bsl_file ( bsl_stream )) return(-1); 

  /* open existing file read only */
  file->Channel = fopen( fname,"rb");
  file->Flags.ExistingFile = True; 
  file->Flags.ReadOnlyFile = True;

  *pstatus = status_success;
  *pErrorValue = RoutineSucceeded;

  return (stream);

} /* open_as_bsl_file */

/*---------------------------------------------------------------------------
  open_as_edf_file
  Open the file 'fname' with mode "new", "old", "any", "read" and "temp"
  return a stream (success: 0 .. MaxFiles-1, error: -1);
  --------------------------------------------------------------------------+*/
int open_as_edf_file ( const char *fname, const char * mode,
                       int *pErrorValue, int *pstatus )                 /*---*/
{ int stream;
  int locate_status;

  DFile * file;
  DBlock * block;

  *pstatus = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("open_as_edf_file\n");

  if (!InitTable) init_file_table( FileTable );
  if ( ( stream=search_free_stream( FileTable ) ) < 0 ) {
    *pErrorValue = NoMoreStreamsAvailable; return(-1);
    }

  if (EDFIO_debug) printf("\"%s\" : stream = %d (%s)\n",fname,stream,mode);

  file = &FileTable[stream];

  init_file ( file );
  file->Used = True;

  if ( !(file->Name = newstr( fname ) ) ) return(-1); 

  if (STRCMP(mode,Old)==0) { /* open old file */ 
    file->Channel = fopen( fname,"rb+");
    file->Flags.ExistingFile = True; 
    file->Flags.ReadOnlyFile = False; } else
  if (STRCMP(mode,New)==0) { /* open new file */ 
    file->Channel = fopen( fname,"wb+");
    file->Flags.ExistingFile = False;
    file->Flags.ReadOnlyFile = False; } else
  if (STRCMP(mode,Any)==0) { /* open old file or create new file */
    if ( (file->Channel = fopen( fname,"rb+")) ) {
      file->Flags.ExistingFile = True; 
      file->Flags.ReadOnlyFile = False;
      } else {
      file->Channel = fopen( fname,"wb+");
      file->Flags.ExistingFile = False; 
      file->Flags.ReadOnlyFile = False; } } else
  if (STRCMP(mode,Read)==0) { /* open old file */
    file->Channel = fopen( fname,"rb");
    file->Flags.ExistingFile = True; 
    file->Flags.ReadOnlyFile = True; } 
  if (STRCMP(mode,Temp)==0) { /* open internal temporary file */
    file->Flags.TemporaryFile = True;
    file->Channel = (FILE *) NULL;
    } else {
    // open i/o-channel of non-temporary files
    if (file->Channel == (FILE *) NULL) {
      *pErrorValue = CouldNotOpenFile; return(-1);
      }
    /* allocate IO-buffer */
    file->Buffer = (char *) malloc(BufferSize*sizeof(char));
    if ( file->Buffer == (char *) NULL ) {
      *pErrorValue = CouldNotMallocMemory; return(-1);
      }
    if ( setvbuf(file->Channel, file->Buffer, _IOFBF, BufferSize) ) {
      *pErrorValue = CouldNotSetBuffer; return(-1); }
    }

  /* read general header */
  if ( new_general_block( file ) ) {
      *pErrorValue = ErrorCreatingGeneralBlock; return(-1);
      }

  if ( file->Flags.ExistingFile ) {
    if ( !read_general_block( file ) ) {

      /* This must be an EDF file */
      if ( !file->Flags.NoGeneralHeader )
        file->LastBlockInFile              = file->GeneralBlock;
      /* Update sequence number of general block */
      if (!file->Flags.NoGeneralHeader)
        file->GeneralBlock->SequenceNumber = file->NextSequenceNumber++;

      /* locate data blocks */
      locate_status = locate_block( file , &block );
      while ( !locate_status ) {
        file->LastBlockInFile = block;
        block->SequenceNumber = file->NextSequenceNumber++;
        locate_status = locate_block( file , &block );
        } 
      /* verify that end of file is reached */
      if ( locate_status<-2 ) {
        /* severe error */
        *pErrorValue = ErrorLocatingBlocks; return(-1);
        }
     } else { 
      if (free_data_file ( file )) {
        *pErrorValue = CouldNotCloseFile; return(-1); };
      *pErrorValue = ErrorReadingGeneralBlock;
      return(-1); 
     } /* if read_general_block */ 

    } else file->GeneralBlock->Flags.HeaderChanged = True;

  /* check, if something is in the file */
  if (STRCMP(mode,Old)==0) /* open old file */
    if (!file->LastBlockInFile) { 
      *pstatus = status_error;
      *pErrorValue = CouldNotFindHeader; 
      return(-1); }

  *pstatus = status_success;

  return(stream);

} /* open_as_edf_file */

/*---------------------------------------------------------------------------
NAME

   data_file_type --- returns the data file type 

SYNOPSIS

   int data_file_type ( const char *fname );

DESCRIPTION
  Checks file 'fname' and checks its start bytes with check_header
  for a header start of an edf file. If it fails, the file name
  is checked for a bsl type file. 

  If the file starts with the following sequence, it is opened as an edf
  type file.   

   '\r' '\n' 'StartHeader'
   '\n' 'StartHeader'
   'StartHeader'

REMARK
  All V2.0 Edf files must start with the following sequence
  '\n', StartHeader, '\r', '\n', "DATA_FORMAT_PREFIX". The 
  first 8 bytes for a version 2.0 edf data file look today 
  explicitely (22-Mar-1998):

  1    2    3    4    5    6    7    8
  '\n' '{'  '\r' '\n' 'E'  'D'  'F'  '_'

ARGUMENT
const char * fname      filename
int        * pErrorValue error value on exit
int        * pstatus     status on exit

RETURN VALUE
success:   int  stream
error:     int  -1

HISTORY
22-Mar-1998 Peter Boesecke
11-Jul-2001 PB accepts empty files as edf files
  --------------------------------------------------------------------------+*/
int data_file_type ( const char * fname, int *pErrorValue, int *pstatus )
{ int dftype = InValidDFType;
  FILE * channel;

  *pstatus = status_error;

  if (!(channel = fopen( fname, "rb" ))) {
      *pErrorValue = CouldNotOpenFile; return(dftype); }

  /* check for edf file */
  switch ( check_start( channel ) ) {
    case  0: dftype = EdfType; break;
    case -1: dftype = EdfType; break; // empty edf file
    default: dftype = BslType; // check for bsl file name
  }

  fclose ( channel );

  *pstatus     = status_success;
  *pErrorValue = RoutineSucceeded;

  return( dftype );

} /* data_file_type */

/*+++------------------------------------------------------------------------
NAME

   edf_open_data_file --- open file 'fname' with new, old, any, read or temp

SYNOPSIS

   int edf_open_data_file (  const char *fname, const char * mode,
                               int *pErrorValue, int *pstatus );

DESCRIPTION
  Opens the file 'fname' with mode "new", "old", "any" or "read" and
  return a stream (success: 0 .. MaxFiles-1, error: -1).
  "new": open a new file for read/write, an existing file with the same 
         name is overwritten
  "old": open an existing file for read/write and check file format
  "any": open either an existing file and check its file format or 
         open a new file for read/write 
 "read": open an existing file for read and check its file format
 "temp": open a new temporary file
  
  If an existing file is opened  with "old", "any" or "read" its file 
  start marker is checked and, according to it, it is opened as an edf file 
  or a bsl file.
  A temporary file is newer written to disk or read from disk. It can 
  only be accessed internally and it disappears after edf_close. 
  Attention, a temporary file is entirely kept in memory until it is closed.

RETURN VALUE
success:   int  stream 
error:     int  -1

HISTORY
22-Mar-1998 Peter Boesecke
05-Aug-2001 PB temporary file 
  --------------------------------------------------------------------------+*/
int edf_open_data_file ( const char *fname, const char * mode,
                     int *pErrorValue, int *pstatus )                   /*---*/
{ int stream;

  if (EDFIO_debug) printf("edf_open_data_file BEGIN\n");

  if ( (STRCMP(mode,Old)==0) || (STRCMP(mode,Read)==0) ) { /* open old file */

    switch (data_file_type ( fname, pErrorValue, pstatus )) {
      case EdfType: 
           stream = open_as_edf_file ( fname, mode, pErrorValue, pstatus );
           break;
      case BslType: 
           stream = open_as_bsl_file ( fname, mode, pErrorValue, pstatus );
           break;
      default: 
           /* unknown data file type */
           stream = -1;
           *pstatus = status_error;
      }; 
   } else stream = open_as_edf_file ( fname, mode, pErrorValue, pstatus );
 
  if (EDFIO_debug) printf("edf_open_data_file END\n");

  if (EDFIO_debug) edf_print_filetable( stdout, 4, True );
 
  return( stream );

} /* edf_open_data_file */

/*+++------------------------------------------------------------------------
NAME

   check_mode --- check, whether opening mode mode is compatible with stream

SYNOPSIS

  int check_mode( int stream, int mode );

DESCRIPTION

RETURN VALUE
compatible  :      0
incompatible:     -1

HISTORY
06-Jul-2001 Peter Boesecke
  --------------------------------------------------------------------------+*/
int check_mode( int stream, const char * mode )
{ int value=-1;
  unsigned short ef, ro;

  if ( (stream<0)||(stream>=MaxFiles) ) return(value);

  ef = FileTable[stream].Flags.ExistingFile;
  ro = FileTable[stream].Flags.ReadOnlyFile;

  if (!STRCMP(mode,Read)) {
    value = 0;                     // any file is readable 
  } else if (!STRCMP(mode,Old)) {
    if ( ef && (!ro) ) value=0;    // file must exist and not be write protected
  } else if (!STRCMP(mode,New)) {
    if ((!ef) && (!ro)) value = 0; // must have been opened with new 
  } else if (!STRCMP(mode,Any)) {
    if (!ro) value = 0;            // must not be write protected
  }

  return(value);

} // check_mode

/*+++------------------------------------------------------------------------
NAME

   edf_search_stream --- check, whether data file is already opened 

SYNOPSIS

   int edf_search_stream (  const char *fname, const char * mode,
                            int *pErrorValue, int *pstatus );

DESCRIPTION
  If a file called fname is already opened the function returns its stream, 
  otherwise -1. If a file is already opened with an incompatible opening
  mode the error IncompatibleOpeningMode is returned together with the
  stream.

RETURN VALUE
success:   int  stream
error:     int  -1

HISTORY
05-Jul-2001 Peter Boesecke
  --------------------------------------------------------------------------+*/
int edf_search_stream ( const char *fname, const char * mode,
                        int *pErrorValue, int *pstatus )

{ int stream;

  if (EDFIO_debug) printf("edf_search_data_file BEGIN\n");

  if (!InitTable) init_file_table( FileTable );
  stream=search_stream( FileTable, fname ); 

  *pstatus = status_success;
  *pErrorValue = RoutineSucceeded;

  if (stream>=0)
    if (check_mode( stream, mode )) {
      *pstatus     = status_error;
      *pErrorValue = IncompatibleOpeningModes;
    }

  if (EDFIO_debug) printf("edf_search_stream END\n");

  if (EDFIO_debug) edf_print_filetable( stdout, 4, True );

  return( stream );

} /* edf_search_stream */

/*+++------------------------------------------------------------------------
NAME

  edf_close_data_file --- close an edf data file
  
SYNOPSIS
 
  void edf_close_data_file ( int stream, int *pErrorValue, int *pstatus );

DESCRIPTION 
  Closes the edf data file stream

  --------------------------------------------------------------------------+*/
void edf_close_data_file ( int stream, int *pErrorValue, int *pstatus ) /*---*/
{ 
  DFile    *file = &FileTable[stream];
  *pstatus = status_error;

  if (EDFIO_debug) printf("edf_close_data_file\n");

  /* flush unsaved data block */
  disk_write_block( file->ActiveBlock, pErrorValue, pstatus ); 
  if (*pstatus) return;

  /* update active block */
  file->ActiveBlock = (DBlock *) NULL;

  if (free_data_file ( file )) {
     *pErrorValue = CouldNotCloseFile; return;
    }; 

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

} /* edf_close_data_file */

/*+++------------------------------------------------------------------------
NAME

  edf_free_data_file --- close all edf data files

SYNOPSIS

  int edf_free_data_file ( void );

DESCRIPTION
  Closes all edf data files and releases all allocated memory buffers.

  --------------------------------------------------------------------------+*/
int edf_free_data_file ( void )                                         /*---*/
{ int i;

  for (i=0;i<MaxFiles;i++) free_data_file ( &FileTable[i] ); 

  return(1);

} /* edf_free_data_file */

/*+++------------------------------------------------------------------------
NAME

  edf_write_header_line

SYNOPSIS
  int edf_write_header_line ( int stream, long int DataNumber, int DataChain,
                              const char * keyword, const char * Value,
                              int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  If it does not exists, it is created. It writes keyword and value
  into the header. The value is a character string. This function should
  be used when the value should be written to a single line. But the result
  can be clipped at MaxLinLen.
  Currently, it is identical to edf_write_header_string. 

RETURN VALUE
  In case of success the return value is 1, otherwise 0.
  --------------------------------------------------------------------------+*/
int edf_write_header_line ( int stream, long int DataNumber, int DataChain,
                        const char * keyword, const char * Value,
                        int * pErrorValue, int * pstatus )              /*---*/
{ 
  return ( write_header_string ( stream, DataNumber, DataChain,
            keyword, Value, pErrorValue, pstatus ) );
 
} /* edf_write_header_line */

/*+++------------------------------------------------------------------------
NAME

  edf_write_header_string

SYNOPSIS
  int edf_write_header_string ( int stream, long int DataNumber, int DataChain,
                                const char * keyword, const char * Value,
                                int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  If it does not exists, it is created. It writes keyword and value
  into the header. The value is a character string of any length.
  This function should be used if the value should be written to continuation
  lines, if necessary.

RETURN VALUE
  In case of success the return value is 1, otherwise 0.
  --------------------------------------------------------------------------+*/
int edf_write_header_string ( int stream, long int DataNumber, int DataChain,
                              const char * keyword, const char * Value,
                              int * pErrorValue, int * pstatus )        /*---*/
{
  return ( write_header_string ( stream, DataNumber, DataChain,
            keyword, Value, pErrorValue, pstatus ) );

} /* edf_write_header_line */

/*+++------------------------------------------------------------------------
NAME

  edf_write_header_float

SYNOPSIS
  int edf_write_header_float ( int stream, long int DataNumber, int DataChain,
                               const char * keyword, float Value,
                               int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  If it does not exists, it is created. It writes keyword and value as
  text into the header. The value is a float value.

RETURN VALUE
  In case of success the return value is 1, otherwise 0.
  --------------------------------------------------------------------------+*/
int edf_write_header_float ( int stream, long int DataNumber, int DataChain,
                         const char * keyword, float Value,
                         int * pErrorValue, int * pstatus )             /*---*/
{ char KeyBuf[MaxKeyLen+1];
  int errval;

  if (!num_double2str( KeyBuf, MaxKeyLen+1, (double) Value, "",
                       EdfNDigitsFloat, &errval )) {
    *pstatus     = status_error;
    *pErrorValue = NumberConversionFailed; return(0); }
  return( write_header_string ( stream, DataNumber, DataChain,
                        keyword, KeyBuf, pErrorValue, pstatus ) );

} /* edf_write_header_float */

/*+++------------------------------------------------------------------------
NAME

  edf_write_header_unit

SYNOPSIS
  int edf_write_header_unit  ( int stream, long DataNumber,
                               int DataChain, const char * keyword,
                               float Value, const char * unit,
                               int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  If it does not exists, it is created. It writes keyword and value as
  text into the header. The value is a float value.

RETURN VALUE
  In case of success the return value is 1, otherwise 0.
  --------------------------------------------------------------------------+*/
int edf_write_header_unit   ( int stream, long DataNumber,
                              int DataChain, const char * keyword,
                              float Value, const char * unit,
                              int * pErrorValue, int * pstatus )        /*---*/
{ char KeyBuf[MaxKeyLen+1];
  int errval;

  if (!num_double2str( KeyBuf, MaxKeyLen+1, (double) Value, unit,
                       EdfNDigitsFloat, &errval )) {
    *pstatus     = status_error;
    *pErrorValue = NumberConversionFailed; return(0); }
  return( write_header_string ( stream, DataNumber, DataChain,
                        keyword, KeyBuf, pErrorValue, pstatus ) );

} /* edf_write_header_unit */

/*+++------------------------------------------------------------------------
NAME

  edf_write_header_long

SYNOPSIS

  int edf_write_header_long ( int stream, long int DataNumber, int DataChain,
                              const char * keyword, long int Value,
                              int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  If it does not exists, it is created. It writes keyword and value as
  text into the header. The value is a long integer value.

RETURN VALUE
  In case of success the return value is 1, otherwise 0.
  --------------------------------------------------------------------------+*/
int edf_write_header_long ( int stream, long int DataNumber, int DataChain,
                        const char * keyword, long int Value,
                        int * pErrorValue, int * pstatus )              /*---*/
{ char KeyBuf[MaxKeyLen+1];

  if (!long2s(KeyBuf,Value)) {
    *pErrorValue = NumberConversionFailed; return(0); }
  return( write_header_string ( stream, DataNumber, DataChain,
                        keyword, KeyBuf, pErrorValue, pstatus ) );

} /* edf_write_header_long */

/*+++------------------------------------------------------------------------
NAME

  edf_write_data_raw --- Write n-dimensional raw data array without conversion

SYNOPSIS

  void edf_write_data_raw ( int stream, long int DataNumber,
                            int DataChain, const long Dim[],
                            void *pData, int DataType,
                            long DataValueOffset, int ByteOrder,
                            long RasterConfiguration,
                            int * pErrorValue, int *pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'. If it does not exists,
  it is created.
  Writes an n-dimensional DataType data array without any conversion as 
  binary data after the end of the header. The data array is specified by Dim, 
  DataValueOffset, ByteOrder, RasterConfiguration and DataType. All 
  informations are written into the header. 
  To write internal data the function edf_write_data should be used.

ARGUMENTS
    void * pData             (i) pointer to the start of the data array
    int DataType             (i) data type
    long DataValueOffset     (i)
    int ByteOrder            (i)
    long RasterConfiguration (i)
    const long  Dim[0]       (i) N, number of dimensions
                Dim[1]       (i) dimension 1
                Dim[2]       (i) dimension 2
                ...
                Dim[N]       (i) dimension N

HISTORY
  23-Jul-1999 DataValueOffset added
  10-Nov-2000 changed for Dim[0] dimensions
  06-Aug-2001 specifications updated
  --------------------------------------------------------------------------+*/
void edf_write_data_raw ( int stream, long DataNumber,
                          int DataChain, const long Dim[], 
                          void *pData, int DataType, 
                          long DataValueOffset, int ByteOrder,
                          long RasterConfiguration,
                          int * pErrorValue, int *pstatus )             /*---*/
{
  char  KeyBuf[MaxKeyLen+1];
  char  ValBuf[MaxValLen+1];

  DBlock   * block;
  SElement * symbol;

  long  idim;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("edf_write_data_raw\n");

  /* open output to block */
  open_write_block (stream, DataNumber, DataChain,
                    &block, pErrorValue, pstatus);
  if ( *pstatus != status_success )  return;

  /* link raw data and Dim with block and write specification */
  block->Data                    = pData;
  block->Flags.ExternalDataAlloc = True;
  block->DataLen = edf_data_sizeof(DataType) * edf_dim_product(Dim);
  block->DataBufferLen           = block->DataLen;
  block->DataType                = DataType;
  block->DataValueOffset         = DataValueOffset;
  block->DataByteOrder           = ByteOrder; 
  block->DataRasterConfiguration = RasterConfiguration;
  block->DataCompression         = UnCompressed; 
  block->DataDim                 = newdim(Dim);

  /* write compulsary keywords */

  /* --- Array Dimensions */
  for (idim = 1l; idim <= Dim[0]; idim++) {
    /* write DIM_idim */
    sprintf(KeyBuf,"%s%lu",DIMENSION_KEY_PREFIX,idim);
    long2s(ValBuf,Dim[idim]);
    if (insert_string( block, KeyBuf, ValBuf, &symbol) ) {
      *pErrorValue = CouldNotWriteDimension; return; }
    }

  /* --- DataType */
  if (insert_string( block, DATA_TYPE_KEY, edf_datatype2string( DataType ), 
                     &symbol) ) {
    *pErrorValue = CouldNotWriteBinary; return; }

  /* --- DataValueOffset */ 
  if (DataValueOffset!=0l) { /* write only if not zero, otherwise remove */
    if (insert_string( block, DATA_VALUE_OFFSET_KEY, 
                       long2s(ValBuf,DataValueOffset), &symbol) ) {
      *pErrorValue = CouldNotWriteBinary; return; }
   } else { /* remove if zero */
    if (remove_symbol( block, DATA_VALUE_OFFSET_KEY , NULL )) {
      *pErrorValue = CouldNotWriteBinary; return; }
   }

  /* --- ByteOrder */
  if (insert_string( block, BYTE_ORDER_KEY, ByteOrder2String( ByteOrder ), 
                     &symbol) ) {
    *pErrorValue = CouldNotWriteBinary; return; }

  /* --- Compression  */
  if (insert_string( block, COMPRESSION_KEY, NONE, &symbol) ) {
    *pErrorValue = CouldNotWriteBinary; return; }

  /* --- RasterConfiguration  */
  if (RasterConfiguration!=1l) { // only compulsary for RasterConfiguration!=1
    if (insert_string( block, RASTER_CONFIGURATION_KEY,
                       long2s(ValBuf,RasterConfiguration), &symbol) ) {
      *pErrorValue = CouldNotWriteBinary; return; }
    } else { /* remove if 1 */
    if (remove_symbol( block, RASTER_CONFIGURATION_KEY, NULL )) {
      *pErrorValue = CouldNotWriteBinary; return; }
    }

  /* end of compulsary keywords */

  /* close output to block */
  close_write_block( block, pErrorValue, pstatus );
  if ( *pstatus != status_success )  return;

  block->Flags.HeaderChanged = True;
  block->Flags.DataChanged   = True;

  /* write active data block to disk (this block) immediately */
  disk_write_block ( block->Chain->File->ActiveBlock, pErrorValue, pstatus );
  if ( *pstatus != status_success )  return;

  /* update ActiveBlock */
  block->Chain->File->ActiveBlock = (DBlock *) NULL;

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return;

} /* edf_write_data_raw */

/*+++------------------------------------------------------------------------
NAME

  edf_write_data --- Write n-dimensional machine data array

SYNOPSIS

  void edf_write_data             ( int stream, long DataNumber,
                              int DataChain, const long Dim[],
                              void *pData, int MachineType,
                              int * pErrorValue, int *pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'. If it does not exists,
  it is created.
  Writes an n-dimensional MachineType data array as binary data after the 
  end of the header. MachineType is translated to the corresponding data type. 
  If this is not possible, the data is converted into another data type. The 
  data array is specified by Dim and MachineType. Default values are used for 
  DataValueOffset (0), ByteOrder (internal order) and RasterConfiguration (1). 
  All (non-default) informations are written into the header. 

  Parameters:
    void *pData        (i)   pointer to the start of the data array
    const long  Dim[0] (i)   N, number of dimensions                
                Dim[1] (i)   dimension 1
                Dim[2] (i)   dimension 2
                ...
                Dim[N] (i)   dimension N

HISTORY

  --------------------------------------------------------------------------+*/
void edf_write_data        ( int stream, long DataNumber,
                             int DataChain, const long Dim[],
                             void *pData, int MachineType,
                             int * pErrorValue, int *pstatus )          /*---*/
{
  int DataTypeOut;
  int MachineTypeOut; 
  long DataValueOffsetOut  = data_value_offset_out; 
  int ByteOrder            = 0;
  long RasterConfiguration = 1l;
  void *DataOut            = pData; // default
  long DataCount;
  void *tmp                = (void *) NULL;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("edf_write_data\n");

  ByteOrder = byteorder();

  /* get output data type */
  if (data_type_out!=InValidDType) {
    /* convert */
    DataTypeOut = data_type_out;
    MachineTypeOut = edf_datatype2machinetype( DataTypeOut );
    } else {
    /* do not convert */
    DataTypeOut = edf_machinetype2datatype( MachineType );
    MachineTypeOut = MachineType;
    }
  /* get machine type of output data type */
  DataCount = edf_dim_product ( Dim );

  if (EDFIO_debug) {
    edf_showmachinetypes( True );
    printf("Save %s as %s (%s)\n",
            MachineType2String( MachineType ),
            edf_datatype2string( DataTypeOut ),
            MachineType2String( MachineTypeOut) );
    } // EDFIO_debug

  if ( (MachineTypeOut!=MachineType) || (DataValueOffsetOut!=0l) ) {
    /* allocate new memory */
    if (!(tmp = malloc( edf_machine_sizeof(MachineTypeOut) * DataCount ) )) {
      *pErrorValue = CouldNotMallocMemory;
      *pstatus     = status_error;
      return; 
      } 
    DataOut = tmp;
    /* convert to MachineTypeOut */
    if ( edf_machine2machine ( DataOut, MachineTypeOut,
                               pData, -DataValueOffsetOut,
                               MachineType, DataCount ) ) {
       *pErrorValue = DataConversionFailed; return; }
   } // if MachineTypeOut!=MachineType

  edf_write_data_raw ( stream, DataNumber,
                       DataChain, Dim, DataOut, DataTypeOut,
                       DataValueOffsetOut, ByteOrder, RasterConfiguration,
                       pErrorValue, pstatus );
  /* free tmp array */
  if (tmp) { free(tmp); tmp=(void *) NULL; }
  if ( *pstatus != status_success )  return;

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return;

} /* edf_write_data */

/*+++------------------------------------------------------------------------
NAME

  edf_read_header_line --- reads a line 

SYNOPSIS
int edf_read_header_line ( int stream, long int DataNumber, int DataChain,
                       const char * keyword, char * Value,
                       int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for 'keyword' in the header 'DataNumber' in 'DataChain'.
  If the header or the keyword does not exists, the return value is 0 and a 
  specific error value is returned. This error is not fatal and can be used 
  as a test for the existence of the keyword or the header. The 'Value' 
  string specified by 'keyword' is copied after the location pointed to by 
  Value. The minimum allocated size for Value must be MaxValLen+1.
  The function has not changed Value when the return value is 0.

RETURN VALUE
  return value FALSE if not found and no other error
  return( int ) FALSE : data header not found,
                         *pstatus = status_error;
                         *pErrorValue=(CouldNotFindHeader, RoutineSucceeded);
                TRUE  : data header found or error,
                         *pstatus = Success or status_error;
                         *pErrorValue = <any>
  --------------------------------------------------------------------------+*/
int edf_read_header_line ( int stream, long int DataNumber, int DataChain,
                           const char * keyword, char * Value,
                           int * pErrorValue, int * pstatus )           /*---*/
{ const char *hstring;

  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_error;

  if (!(read_header_string ( stream, DataNumber, DataChain,
                  keyword, &hstring, pErrorValue, pstatus ))) return(0);
  if (*pstatus != status_success) return(1);

  /* copy header value to output line value */
  strncpy( Value, hstring, MaxLinLen );
  Value[MaxLinLen] = '\0';

  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_success;

  return(1);

} /* edf_read_header_line */

/*+++------------------------------------------------------------------------
NAME

  edf_read_header_string --- reads a string from the header 

SYNOPSIS
  int edf_read_header_string ( int stream, long int DataNumber, int DataChain,
                               const char * keyword, const char ** pValue,
                               int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for 'keyword' in the header 'DataNumber' in 'DataChain'.
  If the header or the keyword does not exists, the return value is 0 and a
  specific error value is returned. The returned String pointer is NULL.
  This error is not fatal and can be used as a test for the existence of 
  the keyword or the header. In case of success *pValue points to an 
  internal character string that contains the concatenated string of all 
  continuation lines. Because the length of *pValue depends on the sum of 
  the lengths of all continuation lines its size is theoretically unlimited.

RETURN VALUE
  In case of a failure the returned *pValue pointer can be NULL.
  return value FALSE if not found and no other error,
  return( int ) FALSE : data header not found,
                         *pstatus = status_error;
                         *pErrorValue=(CouldNotFindHeader, RoutineSucceeded);
                TRUE  : data header found or error,
                         *pstatus = Success or status_error;
                         *pErrorValue = <any>
  --------------------------------------------------------------------------+*/
int edf_read_header_string ( int stream, long int DataNumber, int DataChain,
                             const char * keyword, const char ** String,
                             int * pErrorValue, int * pstatus )        /*---*/
{ 
  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_error;

  if (!(read_header_string ( stream, DataNumber, DataChain,
                  keyword, String, pErrorValue, pstatus ))) return(0);
  if (*pstatus != status_success) return(1);

  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_success;

  return(1);

} /* edf_read_header_string */

/*+++------------------------------------------------------------------------
NAME

  edf_read_header_float --- reads a float value

SYNOPSIS
  int edf_read_header_float ( int stream, long int DataNumber, int DataChain,
                              const char * keyword, float * Value,
                              int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  In this headers it searches for 'keyword'. If the header or the keyword
  do not exists, the return value is 0 and a specific error value
  is returned. This error is not fatal and can be used as a test for the
  existence of the keyword or the header.
  A pointer to the float value specified by 'keyword' is returned
  in 'Value'.
  The function has not changed Value when the return value is 0.

RETURN VALUE
  return value FALSE if not found and no other error
  return( int ) FALSE : data header not found,
                         *pstatus = status_error;
                         *pErrorValue=(CouldNotFindHeader, RoutineSucceeded);
                TRUE  : data header found or error,
                         *pstatus = Success or status_error;
                         *pErrorValue = <any>
  --------------------------------------------------------------------------+*/
int edf_read_header_float ( int stream, long int DataNumber, int DataChain,
                        const char * keyword, float * Value,
                        int * pErrorValue, int * pstatus )              /*---*/
{ const char *hstring;
  int errval;

  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_error;

  if (!(read_header_string ( stream, DataNumber, DataChain,
                  keyword, &hstring, pErrorValue, pstatus ))) return(0);
  if (*pstatus != status_success) return(1);

  /* copy header value to output float value */
  *Value = ( float ) num_str2double( hstring, NULL, &errval );

  if (errval) {
    // num_errval2str( errbuf, errbuflen, errval ); ++++++++++ get details
    *pErrorValue = NumberConversionFailed; *pstatus = status_error;
  } else { 
    *pErrorValue = RoutineSucceeded; *pstatus = status_success; 
  }

  return(1);

} /* edf_read_header_float */

/*+++------------------------------------------------------------------------
NAME

  edf_read_header_long --- reads a long integer value

SYNOPSIS

  int edf_read_header_long ( int stream, long int DataNumber, int DataChain,
                             const char * keyword, long int * Value,
                             int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  In this headers it searches for 'keyword'. If the header or the keyword
  do not exists, the return value is 0 and a specific error value
  is returned. This error is not fatal and can be used as a test for the
  existence of the keyword or the header.
  A pointer to the long int value specified by 'keyword' is returned
  in 'Value'.
  The function has not changed Value when the return value is 0.

RETURN VALUE
  return value FALSE if not found and no other error
  return( int ) FALSE : data header not found,
                         *pstatus = status_error;
                         *pErrorValue=(CouldNotFindHeader, RoutineSucceeded);
                TRUE  : data header found or error,
                         *pstatus = Success or status_error;
                         *pErrorValue = <any>
  --------------------------------------------------------------------------+*/
int edf_read_header_long ( int stream, long int DataNumber, int DataChain,
                       const char * keyword, long int * Value,
                       int * pErrorValue, int * pstatus )               /*---*/
{ const char *hstring;
  int errval;

  *pErrorValue = RoutineSucceeded;
  *pstatus     = status_error;

  if (!(read_header_string ( stream, DataNumber, DataChain,
                  keyword, &hstring, pErrorValue, pstatus ))) return(0);
  if (*pstatus != status_success) return(1);

  /* copy header value to long value */
  *Value = num_str2long( hstring, NULL, &errval );

  if (errval) {
    // num_errval2str( errbuf, errbuflen, errval ); ++++++++++ get details
    *pErrorValue = NumberConversionFailed; *pstatus = status_error;
  } else { 
    *pErrorValue = RoutineSucceeded; *pstatus = status_success; 
  }

  return(1);

} /* edf_read_header_long */

/*+++------------------------------------------------------------------------
NAME

  edf_read_data_raw --- reads 2d data without type conversion

SYNOPSIS
  void edf_read_data_raw ( int stream, long int DataNumber,
                           int DataChain, long **pDim,
                           size_t * pDataArraySize, void **ppData,
                           int * pDataType, long * pDataValueOffset,
                           int * pByteOrder, long * pRasterConfiguration,
                           int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  If it does not exist the routine stops with an error. A data array 
  with the dimension Data[(*pDim)[1],(*pDim)[2],...] is read from the 
  file. The pointer &&Data[0,0] is returned in ppData. The data type, 
  the data value offset, the byte order and the raster configuration 
  of the array are returned.

  The returned array has a length of *pDataArraySize bytes.
  The data buffer is allocated and must be released explicitly.

ARGUMENTS
    (If memory allocated for the dimension array otherwise *pDim=0)

    long   (*pDim)[0]         (i) number of dimensions N 
           (*pDim)[1]         (o) dimension 1
           (*pDim)[2]         (o) dimension 2
           ...
           (*pDim)[N]         (o) dimension N

    If the supplied *pDim is the NULL pointer a pointer to an allocated 
    dimension array is returned, otherwise the dimensions are copied 
    to the array (*pDim)[]. (*pDim)[0] must contain the dimension, e.g. 2 
    for a 2d data array. The dimension array must have N+1 elements, e.g.
    3 elements for a 2d-array. The first element of the dimension array 
    defines always the number of dimensions and is not changed. Remaining
    dimensions are set to 1. If the supplied dimension array is too
    short an error is returned (NotNdData).
    The internally allocated memory for (*pDim) is released automatically 
    during the next data-i/o or when the file is closed. 

    size_t   * pDataArraySize (o) size of the data array in bytes
    void    ** pData          (o) pointer to the pointer of the data array
    long     * pDataValueOffset (o) data value offset of the array elements
    int      * pDataType (o) data type of the array elements (DType) 

  HISTORY
  23-Jul-1999 PB pDataValueOffset, pByteOrder and pRasterConfiguration added 
  29-Dec-2000 PB total redesign
  --------------------------------------------------------------------------+*/
void edf_read_data_raw ( int stream, long int DataNumber, 
                         int DataChain, long **pDim,
                         size_t * pDataArraySize, void **ppData,
                         int * pDataType, long * pDataValueOffset,
                         int * pByteOrder, long * pRasterConfiguration,
                         int * pErrorValue, int * pstatus )          /*---*/
{ 
  DBlock * block;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("edf_read_data_raw\n");

  open_read_block(stream, DataNumber, DataChain, &block, pErrorValue, pstatus);
  if ( *pstatus != status_success )  return;

  // read without conversion
  read_data_array ( block, *ppData, *pDataArraySize, True, *pDim,
                    pErrorValue, pstatus );
  if ( *pstatus != status_success )  return;

  close_read_block( block, pErrorValue, pstatus );
  if ( *pstatus != status_success )  return;

  *pDataType              = block->DataType;
  *pDataValueOffset       = block->DataValueOffset;
  *pByteOrder             = block->DataByteOrder;
  *pRasterConfiguration   = block->DataRasterConfiguration;

                  *ppData = block->Data;
  if (!(*pDim))     *pDim = block->DataDim;
          *pDataArraySize = block->DataBufferLen;

  *pstatus     = status_success;
  *pErrorValue = RoutineSucceeded;

  return;

} /* edf_read_data_raw */

/*+++------------------------------------------------------------------------
NAME

  edf_read_data --- read data array and convert to MachineType 

SYNOPSIS

  void edf_read_data       ( int stream, long DataNumber, int DataChain, 
                             long **pDim, size_t * pDataArraySize,
                             void **ppData, int MachineType,
                             int * pErrorValue, int * pstatus );

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'.
  If it does not exist the routine stops with an error.
  A data array is read from the file and converted into a float array of
  the type float Data[Dim[1],Dim[2]]. The data array is allocated.
  The pointer &&Data[0,0] is returned in ppData.
  If the stored array is only 1 dimensional, the data is read with
  Dim[2] set to 1. 

  The data are read with the specification given in the header.
  All read data values are converted into float and the returned data
  array has a length of *pDataArraySize bytes.
  The data buffer is allocated and must be released explicitly.

ARGUMENTS
    (If memory allocated for the dimension array otherwise *pDim=0)

    long   (*pDim)[0]         (i) number of dimensions N
           (*pDim)[1]         (o) dimension 1
           (*pDim)[2]         (o) dimension 2
           ...
           (*pDim)[N]         (o) dimension N

    If the supplied *pDim is the NULL pointer a pointer to an allocated
    dimension array is returned, otherwise the dimensions are copied
    to the array (*pDim)[]. (*pDim)[0] must contain the dimension, e.g. 2
    for a 2d data array. The dimension array must have N+2 elements, e.g.
    4 elements for a 2d-array. The first element of the dimension array
    defines always the number of dimensions and is not changed. Remaining
    dimensions are set to 1. If the the supplied dimension array is too
    short an error is returned (NotNdData).
    The internally allocated memory for (*pDim) is released automatically
    during the next data-i/o or when the file is closed.

    size_t   * pDataArraySize (o) size of the data array in bytes
    float   ** pData          (o) pointer to the pointer of the data array

HISTORY
  29-Dec-2000 PB total redesign
  --------------------------------------------------------------------------+*/
void edf_read_data       ( int stream, long DataNumber, int DataChain, 
                           long **pDim, size_t * pDataArraySize,
                           void **ppData, int MachineType,
                           int * pErrorValue, int * pstatus )           /*---*/
{
  DBlock * block;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("edf_read_data\n");

  open_read_block(stream, DataNumber, DataChain, &block, pErrorValue, pstatus);
  if ( *pstatus != status_success )  return;

  // read without conversion
  read_data_array ( block, *ppData, *pDataArraySize, False, *pDim,
                    pErrorValue, pstatus );
  if ( *pstatus != status_success )  return;

  // conversion to MachineType
  renorm_data_array ( block, *ppData, *pDataArraySize, MachineType, 
                      pErrorValue, pstatus );
  if ( *pstatus != status_success )  return;

                  *ppData = block->Data;
  if (!(*pDim))     *pDim = block->DataDim;
          *pDataArraySize = block->DataBufferLen;

  close_read_block( block, pErrorValue, pstatus );
  if ( *pstatus != status_success )  return;

  *pstatus     = status_success;
  *pErrorValue = RoutineSucceeded;

  return;

} /* edf_read_data */

/*+++------------------------------------------------------------------------
NAME

  edf_search_minmax_number

SYNOPSIS

  int edf_search_minmax_number( int stream, int DataChain,
                                long int *pMinNumber, long int * pMaxNumber,
                                int *pErrorValue, int *pstatus)

DESCRIPTION
  Reads the headers in 'DataChain'
  and searches for the minimum and maximum data number.

  int DataChain   (i)           : data chain (0: general, 1: key_1, 2: key_2)
  long int * pMinNumber (o)     : minimum data number
  long int * pMaxNumber (o)     : maximum data number
  int *pErrorValue (o)          : error value
  int *pstatus (o)              : SAXS status

RETURN VALUE
  TRUE    : successful
  FALSE   : failed 

--------------------------------------------------------------------------+*/
int edf_search_minmax_number( int stream, int DataChain,
                          long int *pMinNumber, long int * pMaxNumber,
                          int *pErrorValue, int *pstatus)             /*---*/
{ DBlock * block;
  DChain * chain;
  DFile  * file;

  char   *min = (char *) NULL, *max = (char *) NULL;
  char ChainKey[MaxKeyLen+1];

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("edf_search_minmax_number\n");

  *pMinNumber = -1l; *pMaxNumber = -2l;

  /* get file */
  file = &FileTable[stream];
  if ( !file->Used ) {
    *pErrorValue = NoFileOpen; return(0);
    }

  /* find 'ChainKey' */
  if ( find_chainkey ( stream, DataChain, ChainKey ) ) {
    *pErrorValue = MissingKeyDefinition; return(0);
    }

  /* search chain */
  if (search_data_chain( file, ChainKey, &chain) ) {
    *pErrorValue = CouldNotFindHeader; return(0); }

  /* loop over all blocks */
  block = chain->BlockList;
  if ( block ) {
    min = max = block->BlockKey;
    block = block->Next;
    while ( block ) {
      if ( compare_keys( block->BlockKey, min , NumberSort ) < 0 ) {
        min = block->BlockKey; } 
      else if ( compare_keys( block->BlockKey, max , NumberSort ) > 0 ) {
        max = block->BlockKey; }
      block = block->Next;
      } /* while */
    *pMinNumber = (long int) s2u_long( min );
    *pMaxNumber = (long int) s2u_long( max ); }
  else { *pErrorValue = NoDataBlocksFound; return(0); }

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* edf_search_minmax_number */

/*+++------------------------------------------------------------------------
NAME
  edf_read_data_dimension

SYNOPSIS

  int edf_read_data_dimension ( int stream, long int DataNumber,
                                int DataChain, long ** pDim,
                                size_t * pDataArraySize,
                                int * pErrorValue, int * pstatus )   
DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'
  and inquires for the dimension of the two dimensional data array.

  If the header or the dimension of the data array does not exists,
  the return value is FALSE and a specific error value is returned.
  This error is not fatal and can be used as a test for the
  existence of a header.

ARGUMENTS

  long int DataNumber   (i)     : data number
  int DataChain   (i)           : data chain (0: general, 1: key_1, 2: key_2)

  If memory is allocated for the dimension array otherwise *pDim=0

  long   (*pDim)[0]         (i) : number of dimensions N
         (*pDim)[1]         (o) : dimension 1
         (*pDim)[2]         (o) : dimension 2
         ...
         (*pDim)[N]         (o) : dimension N

    If the supplied *pDim is the NULL pointer a pointer to an allocated
    dimension array is returned, otherwise the dimensions are copied
    to the array (*pDim)[]. (*pDim)[0] must contain the dimension, e.g. 2
    for a 2d data array. The dimension array must have N+2 elements, e.g.
    4 elements for a 2d-array. The first element of the dimension array
    defines always the number of dimensions and is not changed. Remaining
    dimensions are set to 1. If the supplied dimension array is too
    short an error is returned (NotNdData).
    The internally allocated memory for (*pDim) is released automatically
    during the next data-i/o or when the file is closed. Attention, it is
    also released when the actual data is read.

    size_t   * pDataArraySize (o) : size of the raw data array in bytes

    int *pErrorValue (o)          : error value
    int *pstatus (o)		  : SAXS status


RETURN VALUE
  TRUE    : data header found, if *pstatus == Success
  FALSE   : data header not found,
            *pstatus == status_error;
            *pErrorValue == CouldNotFindHeader;

HISTORY

  --------------------------------------------------------------------------+*/
int edf_read_data_dimension ( int stream, long int DataNumber,
                              int DataChain, long ** pDim,
                              size_t * pDataArraySize,
                              int * pErrorValue, int * pstatus )        /*---*/
{ 
  DBlock      * block;

  long        * data_dim;
  size_t        data_len;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("edf_read_data_dimension\n");

  if (!open_read_block (stream, DataNumber, DataChain,
                    &block, pErrorValue, pstatus)) return(0);
  if ( *pstatus != status_success )  return(1);

  /* get dimension */
  if ( ! (data_dim = get_data_dim ( block )) ) {
      *pErrorValue = CouldNotReadDimension; return(1); }
  data_len = edf_dim_product(data_dim)*edf_data_sizeof(get_data_type(block));

  *pDataArraySize = data_len;

  /* copy data_dim to *pDim */
  if (*pDim) { // use supplied buffer
    if ( !(copydim( *pDim, (*pDim)[0], data_dim ) ) ) {
      *pErrorValue = NotNdData; return(1);
      } 
    } else { // use allocated buffer
    *pDim = data_dim;
    }

  /* update DataDim array */
  if (block->DataDim) { // copy dimensions into existing array 
    if ( !(copydim( block->DataDim, block->DataDim[0], data_dim ) ) ) {
       *pErrorValue = NotNdData; return(1); } 
    } else { // use allocated buffer
    block->DataDim  = data_dim;
    }

  close_read_block( block, pErrorValue, pstatus );
  if ( *pstatus != status_success ) return(1);

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* edf_read_data_dimension */

/*+++------------------------------------------------------------------------
NAME

   edf_test_header  --- test, whether a header exists

SYNOPSIS

   int edf_test_header      ( int stream, long DataNumber,
                              int DataChain,  
                              int *pErrorValue, int *pstatus ); 

DESCRIPTION
  Searches for header 'DataNumber' in 'DataChain'. Returns only 1
  if it was found, otherwise it returns 0.

  --------------------------------------------------------------------------+*/
int edf_test_header         ( int stream, long DataNumber,
                              int DataChain, 
                              int *pErrorValue, int *pstatus )          /*---*/
{ 
  DFile    * file;
  DChain   * chain;
  DBlock   * block;

  char BlockKey[MaxKeyLen+1];
  char ChainKey[MaxKeyLen+1];

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (EDFIO_debug) printf("edf_test_header\n");

  /* get file */
  file = &FileTable[stream];
  if ( !file->Used ) {   
    *pErrorValue = NoFileOpen; return(0);
    }

  /* find 'ChainKey' */
  if ( find_chainkey ( stream, DataChain, ChainKey ) ) {
    *pErrorValue = MissingKeyDefinition; return(0);
    }

  /* find 'BlockKey' */
  if ( find_blockkey ( DataChain, DataNumber, BlockKey ) ) {
    *pErrorValue = MissingKeyDefinition; return(0);
    }

  *pstatus     = status_success;

  /* search header */
  if (search_data_chain( file, ChainKey, &chain) ) return(0); 
  if (search_data_block( chain, BlockKey, &block) ) return(0); 

  return(1);

} /* edf_test_header */

/*===keyorder BEGIN=======================================================*/
 
/*+++ public interface of keyorder -----------------------------------------
  HISTORY
  2001-11-25  V1.0 Peter Boesecke

PUBLIC extern void
  edf_keyorder_set_table        ( const char * table[] ),
  edf_keyorder_print_table      ( FILE * out );

  The following routines are only used in edfio.c

PRIVATE SElement ** 
  keyorder_ordersymbols         ( DBlock * block );

PRIVATE int 
  keyorder_first_header_element ( HList * header, HElement ** element ),
  keyorder_next_header_element  ( HList * header, HElement ** element );

---------------------------------- end of public interface of keyorder ---*/

/****************************************************************************
* Static Variables                                                          *
****************************************************************************/

static int keyorder_Debug = 0;
//static const char * keyorder_DefaultTable[] = { (const char *) NULL };

static const char * keyorder_DefaultTable[] = { "ByteOrder",
  "DataType", "DataRasterConfiguration", "Dim_*", "Title", "Time",
  "Intensity*", "ExposureTime", "Dummy", "DDummy", "Offset_*",
  "Center_*", "BSize_*", "PSize_*", "Sample*", "WaveLength",
  "DetectorRotation_*", "Projection*", "Raster*", "AxisType_*",
  "Norm*", "Attenuator*", "Detector*", "Experiment*", "Gestion*", 
  "Machine*", "Monitor*", "Msensi*", "Optics*", "Proposal*", "Psic*", 
  "Session*", "Pslit*", "Slit*", "Station*", "History-*", 
  "HS32*", "H*",
  (const char *) NULL };

static const char ** keyorder_KeyOrderTable = keyorder_DefaultTable;

/*--------------------------------------------------------------------------
NAME
 
  keyorder_setkeyordertable --- defines a key order table 
 
SYNOPSIS
 
  void keyorder_setkeyordertable( const char * keyordertable[] )
 
DESCRPTION
  Redefines the internal order table by keyordertable. The table must be a list 
  of key words and the last element of the table must be a (char *) NULL. The 
  key words can be terminated by the wild card character '*'. In this case, 
  all keywords starting with the characters before the wild card will be 
  ordered in alphabetical order.
 
-----------------------------------------------------------------------------*/
void keyorder_setkeyordertable( const char * keyordertable[] )
{ keyorder_KeyOrderTable =  keyordertable;
} // keyorder_setkeyordertable

/*--------------------------------------------------------------------------
NAME
 
  keyorder_debug --- set / reset module keyorder into debug mode
 
SYNOPSIS
 
  void keyorder_debug ( int debug );
 
DESCRPTION
 
  Writes 'debug' into keyorder_debug.
 
-----------------------------------------------------------------------------*/
void keyorder_debug ( int debug )
{ keyorder_Debug = debug;
} // keyorder_debug 
 
/*--------------------------------------------------------------------------
NAME
 
  void keyorder_print_keyordertable ( FILE * out ) ---  print order key table
 
SYNOPSIS
 
  void keyorder_print_keyordertable ( FILE * out );
 
DESCRPTION
 
  Prints the internal order key table.
 
-----------------------------------------------------------------------------*/
void keyorder_print_keyordertable ( FILE * out )
{ const char **pkeyorder=keyorder_KeyOrderTable;
  int i;

  if (!pkeyorder) pkeyorder = keyorder_DefaultTable;
  i=0;
  while (*pkeyorder) {
    printf("keyorder_DefaultTable[%u] = %s\n", i++,*pkeyorder);
    pkeyorder++;
  }
  return;
  
} // keyorder_debug                                                                                                                          

/*---------------------------------------------------------------------------
NAME
 
   keyorder_next_header_element_wild -- searches for next 'keyorder' in header
 
SYNOPSIS
 
   int keyorder_next_header_element_wild( HList * header,  
                                           const char * keyorder,
                                           HElement ** element )

DESCRIPTION
   Searches the first header element matching 'keyorder'. 'keyorder' can 
   terminate with '*' as a wild card.
 
RETURN VALUE
  'keyorder' found, element pointer returned in **element
  'keyorder' pointer (HElement *) NULL returned in **element
  return value 0: OK, -1 : ERROR
---------------------------------------------------------------------------*/
int keyorder_next_header_element_wild( HList * header,  
                                       const char * keyorder, 
                                       HElement ** element )
{ HElement * current;

  /* return NULL in case that header was not found */
  *element = (HElement *) NULL;
 
  /* stop, if header not defined */
  if ( !header ) return(-1);
 
  /* search element */
  current = header->ElementRoot;
 
  while ( current!=(HElement *) NULL ) {
    if ( !(current->Flags.Read) ) {
      if (!keyorder) {
        current->Flags.Read = True; break;
      }
      if ( ( keyorder_compare( current->Key, keyorder ) ) == 0 ) {
        current->Flags.Read = True; break;
      }
    }
    current = current->Next;
  }
  *element = current;
 
  return( 0 );
 
} // keyorder_next_header_element_wild

/*---------------------------------------------------------------------------
NAME
 
   keyorder_clear_header_element_readflag -- clears all read flags 
 
SYNOPSIS
 
   int keyorder_clear_header_element_readflag( HList * header );
 
DESCRIPTION
   Clears the read flags of all header elements of header
 
RETURN VALUE
   0, if OK
   -1 if not OK
---------------------------------------------------------------------------*/
int keyorder_clear_header_element_readflags( HList * header )
{
  HElement * current;
 
  /* stop, if header not defined */
  if ( !header ) return(-1);
 
  current = header->ElementRoot;
 
  while ( current!=(HElement *) NULL ) {
    current->Flags.Read = False;
    current = current->Next;
  }
 
  return(0);                                                                                                                                  

} // keyorder_clear_header_element_readflag

/*---------------------------------------------------------------------------
NAME
 
   keyorder_first_header_element -- searches first element in header list
 
SYNOPSIS
 
   int keyorder_first_header_element( HList * header, HElement ** element )
 
DESCRIPTION
   Returns the first header element according to KeyOrderTable. The following 
   header elements must be read with keyorder_next_header_element

   The function clears the Read flags of all header elements and resets
   the KeyOrderNo of header.
 
RETURN VALUE
  first element found, element pointer returned in **element
  no element found, (HElement *) NULL returned in **element
--------------------------------------------------------------------------*/
int keyorder_first_header_element( HList * header, HElement ** element )
{ 
  /* return NULL in case that header was not found */
  *element = (HElement *) NULL;                                                                                                               

  /* stop, if header not defined */
  if ( !header ) return(-1);

  keyorder_clear_header_element_readflags( header );

  /* return NULL in case that header was not found */
  *element = (HElement *) NULL; 
 
  header->KeyOrderNo = 0;
  do {
    keyorder_next_header_element_wild( header,
      keyorder_KeyOrderTable[header->KeyOrderNo], element );
    if ( (*element!=(HElement *) NULL) ) break;
  } while ( (keyorder_KeyOrderTable[header->KeyOrderNo++]) );

  return(0);
 
} // keyorder_first_header_element

/*---------------------------------------------------------------------------
NAME
 
   keyorder_next_header_element -- returns next element in header list
 
SYNOPSIS
 
   int keyorder_next_header_element( HList * header, HElement ** element )
 
DESCRIPTION
   Returns the next header element according to KeyOrderTable.
   The first element must be read with keyorder_first_header_element.
 
RETURN VALUE
  first element found, element pointer returned in **element
  no element found, (HElement *) NULL returned in **element 
---------------------------------------------------------------------------*/
int keyorder_next_header_element( HList * header, HElement ** element )
{ 
  /* return NULL in case that header was not found */
  *element = (HElement *) NULL;
 
  /* stop, if header not defined */
  if ( !header ) return(-1);
 
  do {
    keyorder_next_header_element_wild( header,
      keyorder_KeyOrderTable[header->KeyOrderNo], element );
    if ( (*element!=(HElement *) NULL) ) break;
  } while ( (keyorder_KeyOrderTable[header->KeyOrderNo++]) );
 
  return(0);
 
} // keyorder_next_header_element

/*---------------------------------------------------------------------------
NAME
 
  keyorder_strcolup --- converts a string to upper case and removes spaces
 
SYNOPSIS
 
  char * keyorder_strcolup( char buffer[], unsigned long buflen,
                             const char * string )
 
DESCRIPTION
Copies all printable characters except space to buffer and converts them
to upper case.
 
RETURN VALUES
Pointer to  buffer
 
HISTORY
Peter Boesecke
---------------------------------------------------------------------------*/
char * keyorder_strcolup( char buffer[], unsigned long buflen,
                           const char * string )
{ register unsigned int i;
  char *pc;
 
  pc = &buffer[0];
  if (string) 
    for (i=0;(i<(buflen-1))&&(*string);i++) {
      if (isgraph(*string)) { *pc=toupper(*string); pc++; }
      string++;
    }
  *pc=(char) 0;
 
  return( buffer );
 
} // keyorder_strcolup

/*---------------------------------------------------------------------------
NAME
 
  keyorder_wildcmp --- compares a key with a wild card key
 
SYNOPSIS
 
  int keyorder_wildcmp( const char *key, const char *wild );
 
DESCRIPTION
  The routines compares the characters of wild with the characters of key.
  If '*' is found in wild the comparison of the remaining characters in
  key is skipped until the first character after '*' is found in key. There,
  the comparison is continued. However, the function may not return the 
  expected result if '*' is not the last character in wild and if key
  contains several times the character after '*', e.g.

  key="abcabc", wild = "abcabc" -> 0
  key="abcabc", wild = "abc*"   -> 0
  key="abcabc", wild = "*abc"   -> 1 comparison is stopped after the first 
                                     occurrence of abc
  key="abcabc", wild = "*abc*"  -> 0
  key="abcabc", wild = "*cabc"  -> 0
 
RETURN VALUES
The routine returnes as result:
 
   input             output
   key < wild     -1
   key = wild      0
   key > wild     +1
 
HISTORY
Peter Boesecke
---------------------------------------------------------------------------*/
int keyorder_wildcmp( const char *key1, const char *key2 )
{ const char *pc1, *pc2;
  int value=0;

  if ((!key1) || (!key2) ) {
    if (key2) return(-1); else { if (key1) return(1); else return(0); }
  }

  pc1 = key1; pc2 = key2; 

  value=0;

  do {
    if ((*pc2)=='*') { // wild card
      do pc2++; while ((*pc2)=='*'); // get first character after wild cards
      while ((*pc1)!=(*pc2)) {
        if (*pc1) pc1++;
        else break;
      }
    }
    if (*pc2) {
      if ( (*pc1)!=(*pc2) ) {
        if ( (*pc1)<(*pc2) ) value=-1; else value=1;
      }
    } else {
      if (*pc1) value=1;
    }
  } while ( (*pc1++) && (*pc2++) );
 
  return( value );
 
} // keyorder_wildcmp

/*---------------------------------------------------------------------------
NAME
 
  keyorder_compare --- compares a key with a criterion card key
 
SYNOPSIS
 
  int keyorder_compare( const char *key, const char *criterion );
 
DESCRIPTION
The comparison is done for printable characters only, except spaces. Other
characters are suppressed. The comparison is done for a maximum of MaxKeyLen
characters (including all characters, printable and non-printable). The 
comparison is not case sensitive and is done after conversion of all 
characters to upper-case.  If the criterion key contains a '*' the 
comparison is stopped a this position
 
RETURN VALUES
The routine returnes as result:
 
   input             output
   key < criterion     -1
   key = criterion      0
   key > criterion     +1
 
HISTORY
Peter Boesecke
---------------------------------------------------------------------------*/
PRIVATE int keyorder_compare( const char *key, const char *criterion )
{ char buf1[MaxKeyLen+1], buf2[MaxKeyLen+1];
 
  /* convert to upper case */
  keyorder_strcolup( buf1, MaxKeyLen+1, key );
  keyorder_strcolup( buf2, MaxKeyLen+1, criterion );

  /* compare */

  return(keyorder_wildcmp( buf1, buf2 ));
 
} /* keyorder_compare */

//==============================================================================

/*---------------------------------------------------------------------------
NAME
 
   keyorder_clear_symbol_readflag -- clears all symbol read flags
 
SYNOPSIS
 
   int keyorder_clear_symbol_readflag( DBlock * block,
                                        unsigned long * ptablelen );
 
DESCRIPTION
   Clears the read flags of all symbols in block and returns the 
   total number of keys in *ptablelen.

   Must be called before using keyorder_copy_formatsymbols, 
   keyorder_next_symbol_wild and keyorder_next_symbol
 
RETURN VALUE
   0, if OK
   -1 if not OK
---------------------------------------------------------------------------*/
int keyorder_clear_symbol_readflags( DBlock * block,
                                       unsigned long * ptablelen )
{
  SElement * current;
 
  *ptablelen = 0l;
  if ( !block ) return(-1);
 
  current = block->SymbolList;
 
  while ( current!=(SElement *) NULL ) {
    current->Flags.Read = False;
    current = current->Next;
    (*ptablelen)++;
  }

  block->KeyOrderNextSymbol = block->SymbolList;
  block->KeyOrderNo = 0;
 
  return(0);
 
} // keyorder_clear_symbol_readflags 

/*---------------------------------------------------------------------------
NAME
 
   keyorder_copy_formatsymbols -- copies all data format symbols 
 
SYNOPSIS
 
   int keyorder_copy_formatsymbols( DBlock * block,
                                     SElement *** ptable );
 
DESCRIPTION
   Copies all data format specific symbol pointers of block to *ptable[i]. 
   After return *ptable points after the copied format symbols. 
 
RETURN VALUE
   0, if OK
   -1 if not OK
---------------------------------------------------------------------------*/
int keyorder_copy_formatsymbols( DBlock * block, SElement *** ptable )
{
  SElement * current;
 
  if ( !block ) return(-1);
 
  current = block->KeyOrderNextSymbol;

  while ( ( current!=(SElement *) NULL ) && 
          (is_prefix(current->Key,DATA_FORMAT_PREFIX,UpperCaseSort)) ) {
    *(*ptable) = current; (*ptable)++; 
    current->Flags.Read = True;
    current = current->Next;
  }

  block->KeyOrderNextSymbol = current;
 
  return(0);
 
} // keyorder_copy_formatsymbols

/*---------------------------------------------------------------------------
NAME
 
   keyorder_next_symbol_wild -- searches for next 'keyorder' in block 
 
SYNOPSIS
 
   int keyorder_next_symbol_wild( DBlock * block,
                                  const char * keyorder,
                                  SElement ** symbol )
 
DESCRIPTION
   Searches the first symbol matching 'keyorder'. 'keyorder' can terminate
   with '*' as a wild card.

   To increase the speed in long lists the search starts at 
   block->KeyOrderNextSymbol which is the first symbol that has not already 
   been read. First, block->KeyOrderNextSymbol is updated. Then the search 
   is started. The first symbol that matches keyorder and that has not 
   already been read.
 
RETURN VALUE
  'keyorder' found, symbol pointer returned in **symbol
  'keyorder' pointer (HElement *) NULL returned in **symbol
  return value 0: OK, -1 : ERROR
---------------------------------------------------------------------------*/
int keyorder_next_symbol_wild( DBlock * block,
                               const char * keyorder,
                               SElement ** symbol )
{ SElement * current;
 
  /* reset symbol */
  *symbol = (SElement *) NULL;
 
  /* stop, if block not defined */
  if ( !block ) return(-1);
 
  /* get start point */
  current = block->KeyOrderNextSymbol;

  /* skip already read symbols */
  while ( ( current!=(SElement *) NULL ) && (current->Flags.Read) ) {
    current = current->Next;
  }

  /* set new start point */
  block->KeyOrderNextSymbol = current;
 
  /* search symbol */
  while ( current!=(SElement *) NULL ) {
    if ( !(current->Flags.Read) ) {
      if (!keyorder) {
        current->Flags.Read = True; break;
      }
      if ( ( keyorder_compare( current->Key, keyorder ) ) == 0 ) {
        current->Flags.Read = True; break;
      }
    }
    current = current->Next;
  }
  *symbol = current;
 
  return( 0 );
 
} // keyorder_next_symbol_wild 

/*---------------------------------------------------------------------------
NAME
 
   keyorder_next_symbol -- returns next symbol 
 
SYNOPSIS
 
   int keyorder_next_symbol( DBlock * block, SElement ** symbol )
 
DESCRIPTION
   Returns the next symbol according to KeyOrderTable.
   The first symbol must be read with keyorder_first_symbol.
 
RETURN VALUE
  next symbol found, symbol pointer returned in **symbol
  no symbol found, (SElement *) NULL returned in **symbol
---------------------------------------------------------------------------*/
int keyorder_next_symbol( DBlock * block, SElement ** symbol ) 
{
  /* reset symbol  */
  *symbol = (SElement *) NULL;
 
  /* stop, if block not defined */
  if ( !block ) return(-1);
 
  do {
    keyorder_next_symbol_wild( block,
      keyorder_KeyOrderTable[block->KeyOrderNo], symbol );
    if ( (*symbol!=(SElement *) NULL) ) break;
  } while ( (keyorder_KeyOrderTable[block->KeyOrderNo++]) );
 
  return(0);
 
} // keyorder_next_symbol

/*---------------------------------------------------------------------------
NAME
 
  keyorder_ordersymbols --- return a table with symbols in order 
 
SYNOPSIS
 
  SElement ** keyorder_ordersymbols( DBlock * block )
 
DESCRIPTION
  Returns a table that lists pointers to the user symbols of block according to
  keyorder_KeyOrderTable. The table is terminated with an empty element 
  (SElement*) NULL. The table must be released after use.
 
RETURN VALUES
The routine returnes as result:
  (SElement **) NULL: error otherwise, pointer to allocated ordered symbol table  
 
HISTORY
Peter Boesecke
---------------------------------------------------------------------------*/
PRIVATE SElement ** keyorder_ordersymbols( DBlock * block )
{ SElement * symbol, ** table = (SElement **) NULL, ** ptable;
  unsigned long tablelen;

  if ( keyorder_clear_symbol_readflags( block, &tablelen ) )
    return( table );

  table = (SElement **) malloc ( (tablelen+1) * sizeof(SElement **) );
  if (!table) return( table );

  ptable = table;

  // copy all format symbols into table
  keyorder_copy_formatsymbols( block, &ptable );

  // search first user symbol
  keyorder_next_symbol( block, &symbol ); 

  while ( symbol ) {
    // save symbol pointer in table  
    *ptable = symbol; ptable++; 

    // search next symbol
    keyorder_next_symbol( block, &symbol );
  }

  *ptable = (SElement *) NULL;

  return( table );
  
} /* keyorder_ordersymbols */

/*+++------------------------------------------------------------------------
NAME
 
  edf_keyorder_set_table --- sets a new order key table 
 
SYNOPSIS
 
  void edf_keyorder_set_table( const char * table[] )
 
DESCRIPTION
  This function defines a new order key table. The table contains
  all keys in the order in what they should be written. The table must be 
  terminated with (char *) NULL. Keys that are not found in the table are
  listed after the keys that have been found. An asterisk at the end of a 
  key in the table is a wild card. 
 
HISTORY
2001-11-24 Peter Boesecke
  --------------------------------------------------------------------------+*/
void  edf_keyorder_set_table( const char * table[] )
{ keyorder_setkeyordertable( table );
} /* edf_keyorder_set_table */
 
/*+++------------------------------------------------------------------------
NAME
 
  edf_keyorder_print_table --- prints the order key table
 
SYNOPSIS
 
  void edf_keyorder_print_table( FILE * out ) 
 
DESCRIPTION
  The order key table is printed to out.
 
HISTORY
2001-11-24 Peter Boesecke
--------------------------------------------------------------------------+*/
void edf_keyorder_print_table( FILE * out )
{ keyorder_print_keyordertable ( out );
} /* edf_keyorder_print_table */

/*===keyorder END=======================================================---*/

/*===header_list BEGIN=====================================================*/

/*+++ public interface of header_list ---------------------------------------

  DESCRIPTION
  The routines of this module are used to read and write header lines.
  'edf_new_header' must be called first. Opens a header with header_key.
  'edf_read_header'  reads the header from the input image
  'edf_write_header' writes the header to the output image
  'edf_first_header_element' returns key and value of the first header element
  'edf_next_header_element' returns key and value of the next header element
  'edf_search_header_element' returns value of header element
  'edf_add_header_element' adds or replaces a header element  
  'edf_delete_header_element' deletes a header element
  'edf_free_header' releases a specific or all header keys. 
  'header_debug' sets the module into debug mode.

  The values of header_list do not contain escape sequences.

  HISTORY
  2000-12-31 PB
  2001-11-25 PB edf_first_header_element, edf_next_header_element

PUBLIC extern int
  edf_new_header           ( const char * header_key ),
  edf_read_header          ( int stream, long int DataNumber,
                             int DataChain, const char * header_key, 
                             int * pErrorValue, int * pstatus ),
  edf_write_header         ( int stream, long int DataNumber,
                             int DataChain, const char * header_key,
                             int * pErrorValue, int * pstatus ),
  edf_first_header_element ( const char * header_key,
                             const char ** pkey, const char ** pvalue,
                             int * pErrorValue, int * pstatus ),
  edf_next_header_element  ( const char * header_key,
                             const char ** pkey, const char ** pvalue,
                             int * pErrorValue, int * pstatus ),
  edf_search_header_element( const char * header_key,
                             const char * key, const char ** pvalue,
                             int * pErrorValue, int * pstatus ),
  edf_add_header_element   ( const char * header_key,
                             const char * key, const char * value,
                           int * pErrorValue, int * pstatus ),
  edf_delete_header_element( const char * header_key,
                             const char * key,
                             int * pErrorValue, int * pstatus ),
  edf_free_header          ( const char * header_key ),
  edf_print_header         ( FILE * out, const char * header_key,
                             int level, int verbose );

PUBLIC extern void
  header_debug            ( int debug );

  ------------------------------ end of public interface of header_list ---*/

/****************************************************************************
* Static Variables                                                          *
****************************************************************************/
static int InitHeader = 0;
static HList * header_root = (HList *) NULL;
static int HEADER_debug = 0;

/*---------------------------------------------------------------------------
NAME

   init_header()

SYNOPSIS

DESCRIPTION

RETURN VALUES
Returns 0 in case of success and -1 if no success.
---------------------------------------------------------------------------*/
int init_header( void )
{ header_root = (HList *) NULL;
  InitHeader  = 1;
  return(0);
} /* init_header */

/*+++------------------------------------------------------------------------
NAME

   free_header_list (success:0, error:-1)

SYNOPSIS

   int free_header_list( HList * header )

DESCRIPTION

   Frees the element list of header.

RETURN VALUE
    0: success
   -1: failed
---------------------------------------------------------------------------*/
int free_header_list( HList * header )
{
  HElement * element, * next;

  if (header==(HList *)NULL) return(-1);
  next = header->ElementRoot;

  while(next!=(HElement*) NULL) {
    element = next;
    next=next->Next;
    free(element->Value);
    free(element->Key);
    free(element);
    }

  header->ElementRoot = (HElement *) NULL;

  return(0);

} /* free_header_list */

/*---------------------------------------------------------------------------
NAME

  search_header_list

SYNOPSIS

  int search_header_list  ( HList * root, const char * header_key,
                            HList ** pheader )

DESCRIPTION
Searches the key 'header_key' in the header list and returns the pointer
to the header (*pheader) or NULL if not found. 

RETURN VALUES
Returns 0 if found and -1 if no success.
---------------------------------------------------------------------------*/
int search_header_list  ( HList * root, const char * header_key,
                          HList ** pheader )
{
  HList * previous, * next;
  int     notfound = 1;

  *pheader = (HList *) NULL;

  /* search for header_key */
  previous = (HList *) NULL;
  next = root;

  if (root == (HList*) NULL) return(-1);

  /* search insertion point (insertion before next) */
  while( ( next!=(HList *) NULL ) && (notfound>0) ) {
    notfound = compare_keys(next->Key,header_key,UpperCaseSort);
    if (notfound>0) {previous = next; next = next->Next;}
    }

  if (!notfound) *pheader = next;

  return(0);

} /* search_header_list */

/*---------------------------------------------------------------------------
NAME

  header_list_new 

SYNOPSIS

  int header_list_new  ( HList ** proot, const char * header_key,
                         HList ** pheader );

DESCRIPTION
Adds a new header list with the key 'header_key' to the header list 
If the key already exists, the existing elements are removed. 
The key list is ordered in reverse alphabetical order.

RETURN VALUES
Returns 0 in case of success and -1 if no success.
---------------------------------------------------------------------------*/
int header_list_new  ( HList ** proot, const char * header_key,
                       HList ** pheader )
{
  HList * newhlist, * previous, * next;
  int     notfound = 1;

  /* search for header_key */
  previous = (HList *) NULL;
  next = *proot;

  *pheader = (HList *) NULL;

  /* search insertion point (insertion before next) */
  while( ( next!=(HList *) NULL ) && (notfound>0) ) {
    notfound = compare_keys(next->Key,header_key,UpperCaseSort);
    if (notfound>0) {previous = next; next = next->Next;}
    }

  if (notfound) {
    /* create new hlist */
    if (!(newhlist = (HList *) malloc(sizeof(HList)))) return(-1);
    newhlist->Key = newstr(header_key); if (!newhlist->Key) return(-1);
    newhlist->ElementRoot = (HElement *) NULL;
    newhlist->KeyOrderNo = 0;

    /* insert newhlist before next */
    if (next) next->Previous = newhlist;
    newhlist->Next=next;
    newhlist->Previous=previous;
    if (previous) previous->Next=newhlist;
      else *proot = newhlist;

    next = newhlist;
    }

  /* free next->ElementRoot */
  if ( next->ElementRoot ) {
    free_header_list( next );
    }

  *pheader = next;

  return(0);

} /* header_list_new */

/*---------------------------------------------------------------------------
NAME

  header_list_free

SYNOPSIS

  int header_list_free  ( HList ** proot );

DESCRIPTION
Removes all header lists from proot.

RETURN VALUES
Returns 0 in case of success and -1 if no success.
---------------------------------------------------------------------------*/
int header_list_free  ( HList ** proot )
{
  HList *current, * next;

  /* search for header_key */
  next = *proot;

  while( next!=(HList *) NULL ) {
    current = next;
    next = next->Next;
    free_header_list( current );
    free( current );
    }

  *proot = next;

  return(0);

} /* header_list_free */

/*---------------------------------------------------------------------------
NAME

  header_list_remove

SYNOPSIS

  int header_list_remove  ( HList ** proot, const char * header_key );

DESCRIPTION
Removes header_key from proot.

RETURN VALUES
Returns 0 in case of success and -1 if no success.
---------------------------------------------------------------------------*/
int header_list_remove  ( HList ** proot, const char * header_key )
{
  HList *current, *previous, *next;

  /* search for header_key */
  if (search_header_list  ( *proot, header_key, &next )) return(-1);
  if ( next!=(HList *) NULL ) { // remove current 

    current  = next;
    previous = current->Previous;
    next     = current->Next; 

    if (next) next->Previous = previous;
    if (previous) previous->Next = next;
      else *proot = next;

    free_header_list( current );
    free ( current );

    }

  return(0);

} /* header_list_remove */

/*---------------------------------------------------------------------------
NAME
   init_header_element_flags --- initialize header element flags
SYNOPSIS
   init init_header_element_flags( HEFlags * Flags )
RETURN VALUE
   success: 0
   error:   not 0
---------------------------------------------------------------------------*/
int init_header_element_flags( HEFlags * Flags )
{
  Flags->Read              = (unsigned short) 0;
 
  return(0);
 
} /* init_header_element_flags */
 
/*---------------------------------------------------------------------------
print_header_element_flags (success:0, error:-1)
---------------------------------------------------------------------------*/
int print_header_element_flags ( FILE * out, const HEFlags * Flags )
{
   fprintf(out,"   Flags\n");
   fprintf(out,"     Read            = %hu\n",Flags->Read);
 
   return(0);
 
} /* print_header_element_flags */

/*---------------------------------------------------------------------------
insert_header_element (success:0, error:-1)
If 'Key' already exists, its 'Value' is updated, otherwise it is created.
In case of success the pointer to the new element is returned.
---------------------------------------------------------------------------*/
int insert_header_element( HList * header, const char * Key,
                           const char * Value , HElement ** element )
{
  HElement * newelement, * next, * previous; 
  char * tmp;
  int notfound = -1;

  if ( header == (HList *) NULL) return(-1);
  if ( Value == (const char *) NULL ) return(-1);

  *element = (HElement *) NULL;
  previous = (HElement *) NULL;
  next = header->ElementRoot;

  /* search insertion point (insertion before next) */
  while( ( next!=(HElement *) NULL ) && (notfound<0) ) {
    notfound = compare_keys(next->Key,Key,UpperCaseSort);
    if (notfound<0) {previous = next; next = next->Next;}
    }

  /* create new element, if (notfound) */
  if ( notfound ) {
    /* create new element */
    if (!(newelement = (HElement *) malloc( sizeof(HElement) ) )) return(-1);

    newelement->Key   = newstr( Key ); if (!newelement->Key) return(-1);
    newelement->Value = (char *) NULL;
    init_header_element_flags( &(newelement->Flags) );

    /* insert newelement before next */
    if (next) next->Previous = newelement;
    newelement->Next=next;
    newelement->Previous=previous;
    if (previous) previous->Next=newelement;
       else header->ElementRoot = newelement;

    next = newelement; 
    }

  /* update Value */
  tmp  = newstr( Value ); if (!tmp) return(-1);
  if ( next->Value ) free ( next->Value );
  next->Value  = tmp;

  *element = next;

  return(0);

} /* insert_header_element */

/*---------------------------------------------------------------------------
search_header_element (success:0, error:-1)
In case of success the pointer to the element is returned.
---------------------------------------------------------------------------*/
int search_header_element( HList * header, const char * Key,
                           HElement ** element )
{
  HElement * current;

  /* return NULL in case that element was not found */
  *element = (HElement *) NULL;

  /* stop, if header not defined */
  if ( !header ) return(-1);

  /* search element */
  current = header->ElementRoot;

  if ( current!=(HElement *) NULL )
  while( ( current!=(HElement *) NULL ) &&
         ( compare_keys(current->Key,Key,UpperCaseSort)!=0 ) ) {
    current = current->Next;
    }
  *element = current;

  if (current==(HElement *) NULL) return(-1);

  return(0);

} /* search_header_element */

/*---------------------------------------------------------------------------
remove_header_element (success:0, error:-1)
The routine ends with success if the element is not any more present, even
if it does not exist before the call of this function.
---------------------------------------------------------------------------*/
int remove_header_element( HList * header, const char * Key )
{
  HElement * current, * previous, *next;
 
  /* search element */
  current = header->ElementRoot;
 
  if ( current!=(HElement *) NULL )
    while( ( current!=(HElement *) NULL ) &&
           ( compare_keys(current->Key,Key,UpperCaseSort)!=0 ) ) {
      current = current->Next;
      }
 
  /* stop, if no element found */
  if (current==(HElement *) NULL) return(0);
 
  /* change links */
  previous = current->Previous;
  next = current->Next;
 
  if ( next != (HElement *) NULL ) next->Previous = previous;
  if ( previous != (HElement *) NULL ) previous->Next = next;
    else header->ElementRoot = next;
 
  /* remove element */
  free(current->Key);
  free(current->Value);
  free(current);

  return(0);
 
} /* remove_header_element */

/*---------------------------------------------------------------------------
read_header_list (success:0, error:-1)
Add all header user values of block to a list and convert them to string.
DBlock * block    : input block
HList * header    : output header 
---------------------------------------------------------------------------*/
int read_header_list( DBlock * block, HList * header )
{ SElement * symbol, * next;
  HElement * element;

  if ( (block == (DBlock *) NULL) || (header==(HList*) NULL) ) return(-1);
  symbol = block->SymbolList;
 
  while (symbol!=(SElement*) NULL) {

    // update symbol string with values of continuation keys
    if ( update_string( symbol, &next ) ) {
      free_header_list(header); return(-1);
    }

    if (HEADER_debug) printf("   '%s' = '%s'\n",symbol->Key,symbol->String);

    if (!is_prefix(symbol->Key,DATA_FORMAT_PREFIX,UpperCaseSort)) {
      if (insert_header_element(header,symbol->Key,symbol->String,&element)) {
        free_header_list(header); return(-1); 
      }
    }

    symbol=next; // skip continuation keys
  }

  return(0);

} /* read_header_list */

/*---------------------------------------------------------------------------
write_header_list (success:0, error:-1)
Write list into header of block
DBlock * block  : output block
HList * header : input header
---------------------------------------------------------------------------*/
int write_header_list( DBlock * block, HList * header )
{
  HElement * element;

  if ( (block == (DBlock *) NULL) || (header==(HList*) NULL) ) return(-1);
  element = header->ElementRoot;
 
  while (element!=(HElement*) NULL) {
    if (HEADER_debug) printf("   '%s' = '%s'\n",element->Key,element->Value);

    /* create/replace keyword and value in header */
    if (insert_string(block,element->Key,element->Value,NULL)) return(-1);

    /* The header was changed and must be written to disk */
    block->Flags.HeaderChanged = True;

    element=element->Next;
  }

  return(0);

} /* write_header_list */

/*+++------------------------------------------------------------------------
NAME

   print_header_list (success:0, error:-1)

SYNOPSIS

   int print_header_list( FILE * out, HList * header, 
                          int level, int verbose );

DESCRIPTION

   Prints the header list to the file 'out'

RETURN VALUE
    0: success
   -1: failed
---------------------------------------------------------------------------*/
int print_header_list( FILE * out, HList * header, int level, int verbose )
{ const char * SeparationLine =
    "-       -       -       -       -       -       -       -";

  HElement * element;

  if (!header) return(-1);
  if (level<1) return(0);

  element = header->ElementRoot;

  while (element!=(HElement*) NULL) {
    if (verbose) {
      fprintf(out,"   %s\n",SeparationLine);
      fprintf(out,"   Key               = %s\n",element->Key);
      fprintf(out,"   Value             = %s\n",element->Value);
      print_header_element_flags ( out, &(element->Flags) );
      fprintf(out,"   Previous Key      = ");
      if ((element->Previous)!=(HElement*) NULL)
        fprintf(out,"%s\n", element->Previous->Key);
        else fprintf(out,"(no previous element)\n");
      fprintf(out,"   Next Key          = ");
      if ((element->Next)!=(HElement*) NULL)
        fprintf(out,"%s\n", element->Next->Key);
        else fprintf(out,"(no next element)\n");
      } else {
      fprintf(out,"   '%s' = '%s'\n",element->Key,element->Value);
      }
    element=element->Next;
    }

  if (verbose) fprintf(out,"   %s\n",SeparationLine);

  return(0);

} /* print_header_list */

int edf_dump_format ( FILE * out, DBlock * block, int format, int newchain, 
                      char * keyword[] )
{
  SElement * symbol;
  char **pkey;

      switch (format) {
        case 0: // standard format
        case 10000:
        case 10001:
            fprintf(out," chain = %s\r\n", block->Chain->ChainKey);
            fprintf(out,"  block = %s\r\n", block->BlockKey);
            if (format>=10000) {
              fprintf(out,
                "   HeaderBlockPos = %lu (%#lx), Length = %lu (%#lx)\r\n", 
                block->TextPos, block->TextPos, block->TextLen, block->TextLen);
              fprintf(out,
                "   BinaryBlockPos = %lu (%#lx), Length = %lu (%#lx)\r\n", 
                block->BinaryPos, block->BinaryPos, 
                block->BinaryLen, block->BinaryLen);
            }
            if ( (keyword) && (keyword[0]!=(char*) NULL) )
              // load data header
              if (!get_data_header( block )) {
                for (pkey=keyword;*pkey!=(char*) NULL;pkey++) {
                  if (!search_general( block, *pkey, &symbol )) {
                    fprintf(out,"   %s = %s\r\n",*pkey, symbol->String );
                    if (format==10001) {
                      fprintf(out,
                        "    KeyPos = %lu (%#lx), Length = %u (%#x)\r\n", 
                        symbol->KeyPos, symbol->KeyPos, 
                        symbol->KeyLen, symbol->KeyLen);
                      fprintf(out,
                        "    ValPos = %lu (%#lx), Length = %u (%#x)\r\n", 
                        symbol->ValPos, symbol->ValPos,
                        symbol->ValLen, symbol->ValLen);
                    }
                  }
                }
              } // get_data_header
            break; // case 0

        case 1: // short format
            fprintf(out,"%s",block->Chain->File->Name);
            fprintf(out,"\t%s",block->Chain->ChainKey);
            fprintf(out,"\t%s",block->BlockKey);
            if ( (keyword) && (keyword[0]!=(char*) NULL) ) {
              // load data header
              if (!get_data_header( block )) {
                for (pkey=keyword;*pkey!=(char*) NULL;pkey++) {
                  if (!search_general( block, *pkey, &symbol )) {
                    fprintf(out,"\t%s", symbol->String );
                  }
                }
              } // get_data_header
              fprintf(out,"\r\n");
            } // keyword
            break; // case 1

        case 2: // very short format
            if ( (keyword) && (keyword[0]!=(char*) NULL) ) {
              // load data header
              if (!get_data_header( block )) {
                pkey=keyword;
                if (!search_general( block, *pkey, &symbol ))
                  fprintf(out,"%s", symbol->String );
                pkey++;
                for (;*pkey!=(char*) NULL;pkey++) {
                  if (!search_general( block, *pkey, &symbol )) {
                    fprintf(out,"\t%s", symbol->String );
                  }
                }
              } // get_data_header
              fprintf(out,"\r\n");
            } // keyword
            break; // case 2

        case 101: // short format, no keywords, one line per chain key
            if (newchain) {
              fprintf(out,"%s",block->Chain->File->Name);
              fprintf(out,"\t%s",block->Chain->ChainKey);
            }
            fprintf(out,"\t%s",block->BlockKey);
            break; // case 101


        default : break;

      } // switch

  return(0);

} // edf_dump_format

/*+++------------------------------------------------------------------------
NAME


SYNOPSIS

  int edf_dump( FILE * out, int stream, int format, 
                char * chainkey[], char * blockkey[], char * keyword[] );

DESCRIPTION

  Prints the current structure of the file. When the keyword table is not 
  empty the contents of all headers are read.

  format 0   : simple, all on separate lines

         1   : short, one line per block key with keywords:
               <filename> <chainkey> <blockkey> <keywords...>
         101 : or one line per chain key without keywords:
               <filename> <chainkey> <blockkeys...> 
         2   : very short, one line per block key, keywords only
               <keywords...>

PARAMETERS
 
  FILE * out       : output file
  int stream       : stream of opened edf file
  int format       : output format
  char *chainkey[] : chainkey table (use all chainkeys, if empty)
  char *blockkey[] : blockkey table (use all blockkeys, if empty)
  char *keyword[]  : keyword table (no keyword, if empty)
  The tables chainkey, blockkey and keyword must be terminated with
  (char *) NULL.

RETURN VALUE
    1: success
    0: failed
---------------------------------------------------------------------------*/
int edf_dump( FILE * out, int stream, int format,                   
              char * chainkey[], char * blockkey[], char * keyword[] )
{ 
  DFile    * file;
  DChain   * chain;
  DBlock   * block;

  char **pckey, **pbkey;
  int newchain, newline=0;

  /* get file */
  file = &FileTable[stream];
  if ( !file->Used ) return(0);

  if ( (format==1) && ((!keyword)||(!keyword[0])) ) format=101;

  if ((format==0)||(format==10000)||(format==10001))
    fprintf(out,"file = %s\r\n",file->Name);

  if ( chainkey && chainkey[0] ) {
    for (pckey=chainkey;*pckey!=(char*) NULL;pckey++) {
      newchain = True;
      if (!search_data_chain( file, * pckey, &chain )) {
        if ( blockkey && blockkey[0] ) {
          for (pbkey=blockkey;*pbkey!=(char*) NULL;pbkey++) {
            if (!search_data_block( chain, * pbkey, &block )) {
              if ( !(is_general_block( block )) ) {
                edf_dump_format ( out, block, format, newchain, keyword );
                newline=1;
                }
              newchain = False;
              }
            } // for block
          } else {
          block = chain->BlockList;
          while (block!=(DBlock*) NULL) {
            if ( !(is_general_block( block )) ) {
              edf_dump_format ( out, block, format, newchain, keyword );
              newline=1;
              }
            block = block->Next;
            newchain = False;
            } // while block
          } // if (blockkey ..
        if ( (10<=format) && (newline) ) fprintf(out,"\r\n");
        } 
      } // for chain
    } else {

    chain = file->ChainList;
    while(chain!=(DChain*) NULL) {
      newchain = True;

      if ( blockkey && blockkey[0] ) {
        for (pbkey=blockkey;*pbkey!=(char*) NULL;pbkey++) {
          if (!search_data_block( chain, * pbkey, &block )) {
            if ( !(is_general_block( block )) ) {
              edf_dump_format ( out, block, format, newchain, keyword );
              newline=1;
              }
            newchain = False;
            }
          } // for block
        } else {
        block = chain->BlockList;
        while (block!=(DBlock*) NULL) {
          if ( !(is_general_block( block )) ) {
            edf_dump_format ( out, block, format, newchain, keyword );
            newline=1;
            }
          block = block->Next;
          newchain = False;
          } // while block
        } // if (blockkey ..
      if ( (10<=format) && (newline) ) fprintf(out,"\r\n");

      chain = chain->Next;
      } // wihle chain

    } // if ( chainkey ..

 return(1);

} // edf_dump

/*+++------------------------------------------------------------------------
NAME

   edf_print_header

SYNOPSIS

   int edf_print_header( FILE * out, const char * header_key,
                         int level, int verbose );

DESCRIPTION

   Prints contents of header 'header_key' to 'out', or contents of
   all headers, if 'header_key' is NULL.

RETURN VALUE
    1: success
    0: failed (specified header not found)
---------------------------------------------------------------------------*/
int edf_print_header( FILE * out, const char * header_key,
                      int level, int verbose )
{ const char * SeparationLine =
    "-   -   -   -   -   -   -   -   -   -   -   -   -   -   -";

  HList * header, * stop = (HList*) NULL;

  if (HEADER_debug) printf("edf_print_header\n");

  if (level<1) return(0);

  if (!InitHeader) init_header();

  if ( header_key != (const char *) NULL ) {
    // list a single header
    search_header_list  ( header_root, header_key, &header );
    if ( header == (HList *) NULL ) return(0);
    stop = header->Next;
  } else header = header_root; // list all headers

  while ( header != stop ) {
    if (verbose) {
      fprintf(out,"  %s\n",SeparationLine);
      fprintf(out,"  Header key         = %s\n",header->Key);
      fprintf(out,"  Previous Key       = ");
      if ((header->Previous)!=(HList*) NULL)
        fprintf(out,"%s\n", header->Previous->Key);
        else fprintf(out,"(no previous header)\n");
      fprintf(out,"  Next Key           = ");
      if ((header->Next)!=(HList*) NULL)
        fprintf(out,"%s\n", header->Next->Key);
        else fprintf(out,"(no next header)\n");
      print_header_list( out, header, level-1, verbose );
      fprintf(out,"  KeyOrderNo         = %d\n",header->KeyOrderNo);
      } else {
      fprintf(out,"  Header key         = %s\n",header->Key);
      print_header_list( out, header, level-1, verbose );
      }
    header=header->Next;
    }

  if (verbose) fprintf(out,"   %s\n",SeparationLine);

  return(1);

} /* edf_print_header */

/*+++------------------------------------------------------------------------
NAME

  edf_read_header --- reads all user keys and values from a header

SYNOPSIS
  int edf_read_header  ( int stream, long DataNumber,
                         int DataChain, const char * header_key,
                         int * pErrorValue, int * pstatus);

DESCRIPTION
  Reads all user keys and values from the data header and the general header.
  It writes them to the element list with the name 'header_key'. 
  The header can be released with edf_free_header( 'header_key' );

RETURN VALUE
  return value FALSE if not found and no other error
  return( int ) FALSE : data header not found,
                         *pstatus = status_error;
                         *pErrorValue=(CouldNotFindHeader, RoutineSucceeded);
                TRUE  : data header found or error,
                         *pstatus = Success or status_error;
                         *pErrorValue = <any>

HISTORY
2000-12-31 Peter Boesecke
  --------------------------------------------------------------------------+*/
int edf_read_header  ( int stream, long DataNumber,
                       int DataChain, const char * header_key,
                       int * pErrorValue, int * pstatus)
{
  DBlock * block;
  HList * header;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (HEADER_debug) printf("edf_read_header\n");

  if (!InitHeader) init_header();

  search_header_list  ( header_root, header_key, &header );
  if ( header == (HList*)NULL ) // try to create it
     header_list_new  ( &header_root, header_key, &header ); 
  if ( header == (HList*)NULL ) return(1); // cannot create 

  if ( !open_read_block (stream, DataNumber, DataChain,
                     &block, pErrorValue, pstatus) ) {
    /* data header not found */ return(0); }
  if ( *pstatus != status_success )  return(1);

  /* read keyword list from general block and copy it to element list */
  if ( read_header_list( block->Chain->File->GeneralBlock, header ) )
     return(1);

  /* read keyword list from block and copy it to element list */
  if ( read_header_list( block, header ) ) return(1);

  close_read_block( block, pErrorValue, pstatus );
  if ( *pstatus != status_success )  return(1);

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* edf_read_header */

/*+++------------------------------------------------------------------------
NAME

  edf_write_header --- writes a header symbol list into the header

SYNOPSIS

  int edf_write_header ( int stream, long DataNumber,
                         int DataChain, const char * header_key,
                         int * pErrorValue, int * pstatus )

DESCRIPTION
  Writes the elementlist with the name 'header_key' into the header 
  of the specified image.

RETURN VALUE
  In case of success the return value is 1, otherwise 0.

HISTORY
2000-12-31 Peter Boesecke
  --------------------------------------------------------------------------+*/
int edf_write_header ( int stream, long DataNumber,
                       int DataChain, const char * header_key,
                       int * pErrorValue, int * pstatus )
{
  DBlock * block;
  HList * header;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;

  if (HEADER_debug) printf("edf_write_header\n");

  if (!InitHeader) init_header();

  search_header_list  ( header_root, header_key, &header );
  if ( header == (HList*) NULL ) {
     *pErrorValue=CouldNotFindHeaderKey; return(0); } // header key not found

  open_write_block (stream, DataNumber, DataChain,
                    &block, pErrorValue, pstatus);
  if ( *pstatus != status_success )  return(0);

  /* create/replace keyword and value in header */
  if ( write_header_list( block, header )) {
    *pErrorValue = CouldNotInsertSymbol; return(0); }

  close_write_block( block, pErrorValue, pstatus );
  if ( *pstatus != status_success )  return(0);

  /* The header was changed and must be written to disk */
  block->Flags.HeaderChanged = True;

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(1);

} /* edf_write_header */

/*+++------------------------------------------------------------------------
NAME
 
  edf_first_header_element --- return first element of header 'header_key'
 
SYNOPSIS
 
  int edf_first_header_element( const char * header_key,
                                const char ** pkey, const char ** pvalue,
                                int * pErrorValue, int * pstatus )
 
DESCRIPTION
  The first element is read from the header list with the name 'header_key'.
  A pointer to the key of this element is returned in *pkey and a pointer
  to the value of this key is returned in *pvalue.
  If the end of the header list is reached or in case of an error
  NULL-pointers are returned.
 
RETURN VALUE
 
  In case of success the return value is 1, otherwise 0.
 
HISTORY
2001-11-24 Peter Boesecke
  --------------------------------------------------------------------------+*/
int edf_first_header_element( const char * header_key,
                              const char ** pkey, const char ** pvalue,
                              int * pErrorValue, int * pstatus )
{ HList * header;
  HElement * element;
 
  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;
  *pkey        = (const char *) NULL;
  *pvalue      = (const char *) NULL;
 
  if (HEADER_debug) printf("edf_first_header_element\n");
 
  if (!InitHeader) init_header();
 
  search_header_list  ( header_root, header_key, &header );
  if ( header == (HList*) NULL ) {
     *pErrorValue=CouldNotFindHeaderKey; return(0); } // header key not found
 
  keyorder_first_header_element( header, &element );
  if ( element != (HElement *) NULL ) {
    *pkey = element->Key;
    *pvalue = element->Value;
  }
 
  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;
 
  return(1);
 
} /* edf_first_header_element */

/*+++------------------------------------------------------------------------
NAME
 
  edf_next_header_element --- return next element from header 'header_key'
 
SYNOPSIS
 
  int edf_next_header_element( const char * header_key,
                               const char ** pkey, const char ** pvalue,
                               int * pErrorValue, int * pstatus )
 
DESCRIPTION
  The next element is read from the header list with the name 'header_key'.
  A pointer to the key of this element is returned in *pkey and a pointer
  to the value of this key is returned in *pvalue.
  If the end of the header list is reached or in case of an error
  NULL-pointers are returned.
  Before using this function edf_first_header_element must have been called.
 
RETURN VALUE
 
  In case of success the return value is 1, otherwise 0.
 
HISTORY
2001-11-24 Peter Boesecke
  --------------------------------------------------------------------------+*/
int edf_next_header_element( const char * header_key,
                             const char ** pkey, const char ** pvalue,
                             int * pErrorValue, int * pstatus )
{ HList * header;
  HElement * element;
 
  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;
  *pkey        = (const char *) NULL;
  *pvalue      = (const char *) NULL;
 
  if (HEADER_debug) printf("edf_next_header_element\n");
 
  if (!InitHeader) init_header();
 
  search_header_list  ( header_root, header_key, &header );
  if ( header == (HList*) NULL ) {
     *pErrorValue=CouldNotFindHeaderKey; return(0); } // header key not found
 
  keyorder_next_header_element( header, &element );
  if ( element != (HElement *) NULL ) {
    *pkey = element->Key;
    *pvalue = element->Value;
  }
 
  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;
 
  return(1);
 
} /* edf_next_header_element */

/*+++------------------------------------------------------------------------
NAME

  edf_search_header_element --- return value of header element 

SYNOPSIS

  int edf_search_header_element( const char * header_key,
                                 const char * key, const char ** pvalue,
                                 int * pErrorValue, int * pstatus )

DESCRIPTION
  The value of the element with name 'key' is read from the header list 
  with the name 'header_key'. A pointer to the value of this key is 
  returned in *pvalue. If the key was not found or in case of an error
  NULL-pointers are returned.

RETURN VALUE

  In case of success (found) the return value is 1, otherwise 0.

HISTORY
2002-01-08 Peter Boesecke
  --------------------------------------------------------------------------+*/
int edf_search_header_element( const char * header_key,
                               const char * key, const char ** pvalue,
                               int * pErrorValue, int * pstatus )
{ HList * header;
  HElement * element;
  int retval=0;

  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;
  *pvalue      = (const char *) NULL;

  if (HEADER_debug) printf("edf_search_header_element\n");

  if (!InitHeader) init_header();

  search_header_list  ( header_root, header_key, &header );
  if ( header == (HList*) NULL ) {
     *pErrorValue=CouldNotFindHeaderKey; return(retval);} //header key not found

  search_header_element( header, key, &element );
  if (element) {
    *pvalue = element->Value; 
    retval = 1;
  }

  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;

  return(retval);

} /* edf_search_header_element */

/*+++------------------------------------------------------------------------
NAME
 
  edf_add_header_element --- add an element to header 'header_key'
 
SYNOPSIS
 
  int edf_add_header_element ( const char * header_key,
                               const char * key, const char * value,
                               int * pErrorValue, int * pstatus )
 
DESCRIPTION
  An element with key and value is inserted into the header list with the
  name 'header_key'. If a key with the same name already exists its value
  is replaced by the new value.
  Before using this function a header must have been created with 
  edf_new_header.
 
RETURN VALUE
 
  In case of success the return value is 1, otherwise 0.
 
HISTORY
2001-12-08 Peter Boesecke
  --------------------------------------------------------------------------+*/
int edf_add_header_element ( const char * header_key,
                             const char * key, const char * value,
                             int * pErrorValue, int * pstatus )
{ HList * header;
  HElement * element;
 
  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;
 
  if (HEADER_debug) printf("edf_add_header_element\n");
 
  if (!InitHeader) init_header();
 
  search_header_list  ( header_root, header_key, &header );
  if ( header == (HList*) NULL ) {
     *pErrorValue=CouldNotFindHeaderKey; return(0); } // header key not found

  insert_header_element( header, key, value, &element );
  if ( element == (HElement*) NULL) {
     *pErrorValue=CouldNotInsertElement; return(0);} // header not updated
 
  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;
 
  return(1);
 
} /* edf_add_header_element */

/*+++------------------------------------------------------------------------
NAME
 
  edf_delete_header_element --- remove an element from header 'header_key'
 
SYNOPSIS
 
  int edf_delete_header_element( const char * header_key,
                                 const char * key,
                                 int * pErrorValue, int * pstatus );
 
DESCRIPTION
  If an element with the name 'key' is found in the header with the name
  'header_key' it is removed otherwise nothing is done. In both cases the
  function returns with success.

  Before using this function a header must have been created with
  edf_new_header.
 
RETURN VALUE
 
  In case of success the return value is 1, otherwise 0.
 
HISTORY
2001-12-08 Peter Boesecke
  --------------------------------------------------------------------------+*/
int edf_delete_header_element( const char * header_key,
                               const char * key,
                               int * pErrorValue, int * pstatus )
{ HList * header;
 
  *pstatus     = status_error;
  *pErrorValue = RoutineSucceeded;
 
  if (HEADER_debug) printf("edf_add_header_element\n");
 
  if (!InitHeader) init_header();
 
  search_header_list  ( header_root, header_key, &header );
  if ( header == (HList*) NULL ) {
     *pErrorValue=CouldNotFindHeaderKey; return(0); } // header key not found
 
  remove_header_element( header, key ); 
 
  *pErrorValue = RoutineSucceeded;
  *pstatus = status_success;
 
  return(1);
 
} /* edf_add_header_element */

/*+++------------------------------------------------------------------------
NAME

   edf_new_header 

SYNOPSIS

   int edf_new_header  ( const char * name );

DESCRIPTION
This routines must be called first. If not already initialized, it
initializes the header module and creates a header that can be accessed
with name. 

RETURN VALUES
In case of success the return value is 1, otherwise 0.

--------------------------------------------------------------------------+*/
int edf_new_header  ( const char * header_key )
{ HList * header;

   if (HEADER_debug) printf("\n edf_new_header BEGIN\n");

   if (!InitHeader) init_header();

   if (header_list_new  ( &header_root, header_key, &header ) ) return(0);

   if (HEADER_debug) printf("\n edf_new_header END\n");

   return(1);

} /* edf_new_header */

/*+++------------------------------------------------------------------------
NAME

  edf_free_header 

SYNOPSIS

   int edf_free_header ( const char * header_key );

DESCRIPTION
This routine removes all header lists with its contents. 
If not already initialized, it initializes the header module.

RETURN VALUES
In case of success the return value is 1, otherwise 0.

--------------------------------------------------------------------------+*/
int edf_free_header ( const char * header_key )
{ int retval;

   if (HEADER_debug) printf("\n edf_free_header BEGIN\n");

   retval = 1;

   if (!InitHeader) init_header();

   if ( header_key == (const char *) NULL ) { // remove all keys
     if (header_list_free  ( &header_root ) ) retval = 0;
   } else { // remove header_key
     if (header_list_remove ( &header_root, header_key ) ) retval = 0;
   }

   if (HEADER_debug) printf("\n edf_free_header END\n");

   return(retval);

} /* edf_free_header */

/*--------------------------------------------------------------------------
NAME

  header_debug --- set / reset module history into debug mode

SYNOPSIS

  void header_debug ( int debug );

DESCRPTION

  Writes 'debug' into HEADER_debug.

-----------------------------------------------------------------------------*/
void header_debug ( int debug )
{ HEADER_debug = debug;
} /* header_debug */

/*===header_list END====================================================---*/

/*+++------------------------------------------------------------------------
NAME

  edf_free_all --- remove all allocated memory 

SYNOPSIS

  int edf_free_all ( void );

DESCRIPTION
  Closes everything and releases all allocated memory buffers.
  --------------------------------------------------------------------------+*/
int edf_free_all ( void )                                               /*---*/
{ 
  edf_history_free( (const char *) NULL ); 
  edf_free_header( (const char *) NULL );
  edf_free_data_file();

  return(1);
} /* edf_free_all */

/*+++------------------------------------------------------------------------
NAME

  edf_report_data_error --- returns the error message of ErrorValue

SYNOPSIS

  char * edf_report_data_error ( int ErrorValue );

DESCRIPTION
  Allocates a buffer and copies the error message corresponding to 
  'ErrorValue'. It returns a pointer to the allocated buffer.

HISTORY
  --------------------------------------------------------------------------+*/
// PRIVATE const char *NDFE = "NoDataFormatError"; //unused
PRIVATE const char *UEV  = "UnknownErrorValue";
PRIVATE const char *RF   = "RoutineFailed";
PRIVATE const char *RS   = "RoutineSucceeded";
PRIVATE const char *CNMM = "CouldNotMallocMemory";
PRIVATE const char *CNFH = "CouldNotFreeHeaders";
PRIVATE const char *CNGA = "CouldNotGetBinaryArray";
PRIVATE const char *NMSA = "NoMoreStreamsAvailable";
PRIVATE const char *INVS = "InvalidStream";
PRIVATE const char *CNOF = "CouldNotOpenFile";
PRIVATE const char *EOFD = "EndOfFileDetected";
PRIVATE const char *CNFI = "CouldNotFindHeader";
PRIVATE const char *CNFS = "CouldNotFindSymbol";
// PRIVATE const char *BSD  = "BadSizeDefinition"; // unused
PRIVATE const char *BDB  = "BadDataBlock";
PRIVATE const char *CNFK = "CouldNotFindKeyword";
PRIVATE const char *WDE  = "WriteDataError";
PRIVATE const char *RDE  = "ReadDataError";
PRIVATE const char *NFO  = "NoFileOpen";
PRIVATE const char *NEDF = "NotESRFDataFile";
PRIVATE const char *NDBF = "NoDataBlocksFound";
PRIVATE const char *ED   = "ExternalData";
PRIVATE const char *FINW = "FileIsNotWritable";
PRIVATE const char *FINO = "FileIsNotOpened";
PRIVATE const char *IOM  = "IncompatibleOpeningModes";
PRIVATE const char *CNCF = "CouldNotCloseFile";
PRIVATE const char *CNIC = "CouldNotInsertChain";
PRIVATE const char *CNIB = "CouldNotInsertBlock";
PRIVATE const char *CNIS = "CouldNotInsertSymbol";
PRIVATE const char *MKD  = "MissingKeyDefinition";
PRIVATE const char *GBNF = "GeneralBlockNotFirst";
PRIVATE const char *ECGB = "ErrorCreatingGeneralBlock";
PRIVATE const char *ERGB = "ErrorReadingGeneralBlock";
PRIVATE const char *ELB  = "ErrorLocatingBlocks";
PRIVATE const char *CNSB = "CouldNotSetBuffer";
PRIVATE const char *NCVF = "NumberConversionFailed";
PRIVATE const char *DCVF = "DataConversionFailed";
PRIVATE const char *DBTS = "DataBufferTooSmall";
PRIVATE const char *MAD  = "MissingArrayDimensions";
PRIVATE const char *NND  = "NotNdData";
PRIVATE const char *N2D  = "Not2dData";
PRIVATE const char *CNWD = "CouldNotWriteDimension";
PRIVATE const char *CNRD = "CouldNotReadDimension";
PRIVATE const char *CNWB = "CouldNotWriteBinary";
PRIVATE const char *CROG = "CannotReOpenGeneralBlock";
PRIVATE const char *COAB = "CannotOpenAsBslFile";
PRIVATE const char *CNIE = "CouldNotInsertElement";
PRIVATE const char *CNFY = "CouldNotFindHeaderKey";

char * edf_report_data_error ( int ErrorValue )                      /*---*/
{ char * errmsg;
  errmsg = (char *) malloc(BufferSize);
    switch (ErrorValue) {
      case RoutineFailed            : sprintf(errmsg,"\n%s\n",RF);   break;
      case RoutineSucceeded         : sprintf(errmsg,"\n%s\n",RS);   break;
      case CouldNotMallocMemory     : sprintf(errmsg,"\n%s\n",CNMM); break;
      case CouldNotFreeHeaders      : sprintf(errmsg,"\n%s\n",CNFH); break;
      case NoMoreStreamsAvailable   : sprintf(errmsg,"\n%s\n",NMSA); break;
      case InvalidStream            : sprintf(errmsg,"\n%s\n",INVS); break;
      case CouldNotOpenFile         : sprintf(errmsg,"\n%s\n",CNOF); break;
      case EndOfFileDetected        : sprintf(errmsg,"\n%s\n",EOFD); break;
      case CouldNotFindHeader       : sprintf(errmsg,"\n%s\n",CNFI); break;
      case CouldNotFindSymbol       : sprintf(errmsg,"\n%s\n",CNFS); break;
      case CouldNotGetBinaryArray   : sprintf(errmsg,"\n%s\n",CNGA); break;
      case BadDataBlock             : sprintf(errmsg,"\n%s\n",BDB);  break;
      case CouldNotFindKeyword      : sprintf(errmsg,"\n%s\n",CNFK); break;
      case WriteDataError           : sprintf(errmsg,"\n%s\n",WDE);  break;
      case ReadDataError            : sprintf(errmsg,"\n%s\n",RDE);  break;
      case NoFileOpen               : sprintf(errmsg,"\n%s\n",NFO);  break;
      case NotESRFDataFile          : sprintf(errmsg,"\n%s\n",NEDF); break;
      case NoDataBlocksFound        : sprintf(errmsg,"\n%s\n",NDBF); break;
      case ExternalData             : sprintf(errmsg,"\n%s\n",ED);  break;
      case FileIsNotWritable        : sprintf(errmsg,"\n%s\n",FINW); break;
      case FileIsNotOpened          : sprintf(errmsg,"\n%s\n",FINO); break;
      case IncompatibleOpeningModes : sprintf(errmsg,"\n%s\n",IOM);  break;
      case CouldNotCloseFile        : sprintf(errmsg,"\n%s\n",CNCF); break;
      case CouldNotInsertChain      : sprintf(errmsg,"\n%s\n",CNIC); break;
      case CouldNotInsertBlock      : sprintf(errmsg,"\n%s\n",CNIB); break;
      case CouldNotInsertSymbol     : sprintf(errmsg,"\n%s\n",CNIS); break;
      case MissingKeyDefinition     : sprintf(errmsg,"\n%s\n",MKD);  break;
      case GeneralBlockNotFirst     : sprintf(errmsg,"\n%s\n",GBNF); break;
      case ErrorCreatingGeneralBlock: sprintf(errmsg,"\n%s\n",ECGB); break;
      case ErrorReadingGeneralBlock : sprintf(errmsg,"\n%s\n",ERGB); break;
      case ErrorLocatingBlocks      : sprintf(errmsg,"\n%s\n",ELB);  break;
      case CouldNotSetBuffer        : sprintf(errmsg,"\n%s\n",CNSB); break;
      case NumberConversionFailed   : sprintf(errmsg,"\n%s\n",NCVF); break;
      case DataConversionFailed     : sprintf(errmsg,"\n%s\n",DCVF); break;
      case DataBufferTooSmall       : sprintf(errmsg,"\n%s\n",DBTS); break;
      case MissingArrayDimensions   : sprintf(errmsg,"\n%s\n",MAD);  break;
      case NotNdData                : sprintf(errmsg,"\n%s\n",NND);  break;
      case Not2dData                : sprintf(errmsg,"\n%s\n",N2D);  break;
      case CouldNotWriteDimension   : sprintf(errmsg,"\n%s\n",CNWD); break;
      case CouldNotReadDimension    : sprintf(errmsg,"\n%s\n",CNRD); break;
      case CouldNotWriteBinary      : sprintf(errmsg,"\n%s\n",CNWB); break;
      case CannotReOpenGeneralBlock : sprintf(errmsg,"\n%s\n",CROG); break;
      case CannotOpenAsBslFile      : sprintf(errmsg,"\n%s\n",COAB); break; 
      case CouldNotInsertElement    : sprintf(errmsg,"\n%s\n",CNIE); break;
      case CouldNotFindHeaderKey    : sprintf(errmsg,"\n%s\n",CNFY); break;
 
      default : sprintf(errmsg,"\n%s : %d\n",UEV,ErrorValue);
      } 

  return(errmsg);
} /* edf_report_data_error */

/*****************************************************************************/

/*+++--------------------------------------------------------------------------
NAME

  edfio_debug --- set / reset module into debug mode

SYNOPSIS

  void edfio_debug ( int debug );

DESCRPTION

  Sets/resets all sub-modules into debug mode

----------------------------------------------------------------------------+*/
void edfio_debug ( int debug )                                          /*---*/
{ EDFIO_debug = debug;
  rc_debug ( EDFIO_debug );
  hist_debug ( EDFIO_debug );
  header_debug ( EDFIO_debug );
  keyorder_debug ( EDFIO_debug );
  numio_debug ( EDFIO_debug );
} /* edfio_debug */
/*****************************************************************************/

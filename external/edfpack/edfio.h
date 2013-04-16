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

    edfio.h 

SYNOPSIS

    #include "edfio.h"

DESCRIPTION
    Header of the module "edfio.c"

HISTORY 
    17-Mar-1998 Peter Boesecke
    16-May-1998 PB INTERNAL_BYTE_ORDER, data_sizeof
    01-Jun-1998 PB set_bsl_input_byteorder
    18-Apr-1999 PB all public routines start with edf_ or edfio_
    30-Apr-1999 PB new routine edf_read_data_2d_raw
    23-Jun-1999 PB Restrictions (PUBLIC)
    24-Jun-1999 PB module HISTORY added, restriction again only private
    26-Jun-1999 PB History_Line changed to Header_Symbol_List (HSymb)
    23-Jul-1999 PB call to edf_read_data_2d_raw and edf_write_data_2d_raw
                   changed (DataValueOffset)
    08-Nov-1999 PB edf_raster_normalization added
    08-Nov-1999 PB in HSymb: shortlen and required added, 
                   new functions: edf_history_skip(), edf_history_take()
                   INTERNAL_BYTE_ORDER redefined according to bytesex.h
    18-Nov-1999 PB default of INTERNAL_BYTE_ORDER: HighByteFirst
    25-Nov-1999 PB dec alpha (__alpha)
    16-Mar-2000 PB comments revised
    18-Mar-2000 PB DBClass and DBInstance
    07-Apr-2000 PB V1.19 set_max_line_width
    30-Apr-2000 PB V1.20 MType
    01-Mai-2000 PB V1.21
    30-Jul-2000 PB V1.24 edf_raster_multiplication, edf_raster_inversion
    10-Nov-2000 PB V1.30 edf_write_data_2d_raw -> edf_write_data_raw
                         edf_write_data_2d_float -> edf_write_data_float
    11-Nov-2000 PB V1.31
    12-Nov-2000 PB V1.32 edf_write_data_float -> edf_write_data(...,MFloat,...)
                   V1.33 long * pDataArraySize -> size_t * pDataArraySize
                   V1.34 edf_read_data_...(... , long Dim[], ...)
                         ---> edf_read_data_...(... , long **pDim, ...)
                         edf_read_data_2d_... -> edf_read_data_...
                         edf_read_data_float --> edf_read_data(...,MFloat,...)
    30-Dec-2000 PB V1.36 
    01-Jan-2001 PB V1.37
    19-May-2001 PB V1.47 edf_dump
    01-Jun-2001 PB V1.49 edf_dump, new parameter list
    07-Jun-2001 PB V1.50 internal parameter functions,
                         the parameter list of edf_dump is not finished,
                         do not use generally 
    06-Jul-2001 PB V1.53 edf_search_stream
    08-Jul-2001 PB V1.55 EdfMaxKeyLen etc., edf_byteorder
    02-Sep-2001 PB V1.59 edf_set_datatype 
    13-Sep-2001 PB V1.62 history routines with history_key
    25-Nov-2001 PB V1.70 keyorder routines
    08-Dec-2001 PB V1.75 edf_add/delete_header_element 
    08-Jan-2002 PB V1.76 edf_search_header_element
    09-Jan-2002 PB V1.77 argument list of edf_print_header 
    12-Feb-2002 PB V1.79 edf_datatype2machinetype, edf_machine_sizeof
    25-Apr-2002 PB V1.80 edf_history_read_header, edf_history_write_header,
                         edf_history_copy , edf_history_print
    24-Mar-2004 PB V1.88 new parameter: EdfNDigitsFloat
    12-Apr-2004 PB V1.93 edf_headersize
    17-Jul-2004 PB V1.95 edf_set_minimumheadersize
    24-Feb-2005 PB V2.01 INTERNAL_BYTE_ORDER replaced by edf_byteorder()
    27-Feb-2005 PB V2.02 edf_write_header_string, edf_read_header_string
    19-Dec-2010 PB V2.32 DCompression is now defined in cmpr.h
    13-Dec-2011 PB V2.39 edf_byteorder2string, edf_string2byteorder
***************************************************************************/

#ifndef EDFIO

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/
#ifndef PRIVATE
# define PRIVATE       static /* used to declare variables of private type */
# define PUBLIC                /* used to declare variables of public type */
#endif

#ifndef True
# define True           1 
# define False          0
#endif

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "cmpr.h"

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern void
edf_write_data_raw          ( int stream, long DataNumber,
                              int DataChain, const long Dim[],
                              void *pData, int DataType, 
                              long DataValueOffset, int ByteOrder, 
                              long RasterConfiguration,
                              int * pErrorValue, int *pstatus ), 
edf_write_data              ( int stream, long DataNumber,
                              int DataChain, const long Dim[],
                              void *pData, int MachineType,
                              int * pErrorValue, int *pstatus ),
edf_read_data_raw           ( int stream, long DataNumber,
                              int DataChain, long **pDim,
                              size_t * pDataArraySize, void **ppData,
                              int * pDataType, long * pDataValueOffset,
                              int * pByteOrder, long * pRasterConfiguration,
                              int * pErrorValue, int * pstatus ), 
edf_read_data               ( int stream, long DataNumber,
                              int DataChain, long **pDim, size_t * pDataArraySize, 
                              void **ppData, int MachineType,
                              int * pErrorValue, int * pstatus ),
edf_close_data_file         ( int stream, int *pErrorValue, int *pstatus );

PUBLIC extern int
edf_open_data_file          ( const char *fname, const char * mode,
                              int *pErrorValue, int *pstatus ),
edf_search_stream           ( const char *fname,  const char * mode,
                              int *pErrorValue, int *pstatus ),
edf_read_data_dimension     ( int stream, long DataNumber,
                              int DataChain, long **pDim,
                              size_t * pDataArraySize,
                              int * pErrorValue, int * pstatus ),
edf_search_minmax_number    ( int stream, int DataChain,
                              long *pMinNumber, long * pMaxNumber,
                              int *pErrorValue, int *pstatus),
edf_read_header_long        ( int stream, long DataNumber,
                              int DataChain, const char * keyword,
                              long * Value, int * pErrorValue, int * pstatus),
edf_read_header_float       ( int stream, long DataNumber,
                              int DataChain, const char * keyword,
		              float * Value, int * pErrorValue, int * pstatus ),
edf_read_header_line        ( int stream, long DataNumber,
                              int DataChain, const char * keyword,
		              char * Value, int * pErrorValue, int * pstatus ),
edf_read_header_string      ( int stream, long int DataNumber, int DataChain,
                              const char * keyword, const char ** pString,
                              int * pErrorValue, int * pstatus ),
edf_write_header_long       ( int stream, long DataNumber,
		              int DataChain, const char * keyword,
		              long Value, int * pErrorValue, int * pstatus ),
edf_write_header_float      ( int stream, long DataNumber,
                              int DataChain, const char * keyword,
                              float Value, int * pErrorValue, int * pstatus ),
edf_write_header_line       ( int stream, long DataNumber,
	           	      int DataChain, const char * keyword,
           		      const char * Value,
		              int * pErrorValue, int * pstatus ),
edf_write_header_string     ( int stream, long DataNumber,
	           	      int DataChain, const char * keyword,
           		      const char * Value,
		              int * pErrorValue, int * pstatus ),
edf_delete_key              ( int stream, long int DataNumber, int DataChain,
                              const char * keyword,
                              int * pErrorValue, int * pstatus ),
edf_write_header_unit       ( int stream, long DataNumber,
                              int DataChain, const char * keyword,
                              float Value, const char * unit,
                              int * pErrorValue, int * pstatus ),
edf_test_header             ( int stream, long DataNumber,
                              int DataChain,
                              int *pErrorValue, int *pstatus ),
edf_general_block           ( int writetodisk ), 
edf_headersize              ( int writetodisk ),
edf_set_minimumheadersize   ( unsigned long minimumheadersize_out ),
edf_set_datatype            ( int datatype_out ),
edf_set_datavalueoffset     ( long int datavalueoffset_out ),
edf_set_datacompression     ( int datacompression_out ),
edf_set_bsl_input_byteorder ( int byteorder ),
edf_set_max_line_width      ( unsigned long width ),
edf_free_data_file          ( void ),
edf_free_all                ( void );

PUBLIC extern char
*edf_report_data_error      ( int ErrorValue ),
*edf_dataformat_version     ( void );

/***************************************************************************
* Data conversion routines                                                 *
***************************************************************************/
PUBLIC extern size_t 
edf_data_sizeof             ( int data_type ),
edf_machine_sizeof          ( int mtype );

PUBLIC extern unsigned long
edf_dim_product             ( const long * dim );

PUBLIC extern int
edf_datatype2machinetype    ( int dtype ),
edf_machinetype2datatype    ( int mtype ),
edf_string2datatype         ( const char * string ),
edf_string2byteorder        ( const char * string ),
edf_string2compression      ( const char * string ),
edf_raster_normalization    ( void * dest, const void * src,
                              const long data_dim[],
                              long raster_configuration, size_t item ),
edf_machine2machine         ( void * dest, int mtype_dest,
                              const void * src, long value_offset,
                              int mtype_src, unsigned long n );

PUBLIC extern long
edf_raster_multiplication   ( long a, long x ),
edf_raster_inversion        ( long x ),
edf_raster_order2number     ( const long order[] );

PUBLIC extern const char
* edf_datatype2string       ( int data_type ),
* edf_byteorder2string      ( int byte_order ),
* edf_compression2string    ( int data_compression );

PUBLIC extern void
edf_bswap                   ( void * dest, const void * src, size_t item,
                              unsigned long n ),
edf_showdatatypes           ( int full ),
edf_showmachinetypes        ( int full );

/***************************************************************************
* Debug routines                                                           *
***************************************************************************/
PUBLIC extern void
edfio_debug                 ( int debug );

PUBLIC extern int
edf_print_filetable         ( FILE * out, int level, int verbose ),
edf_dump                    ( FILE * out, int stream, int format, 
                              char * chainkey[], char * blockkey[],
                              char * keyword[] ); 

PUBLIC extern char
*edfio_version              ( void );

/***************************************************************************
* History routines                                                         *
***************************************************************************/
PUBLIC extern int
edf_history_new             ( const char * history_key ),
edf_history_skip            ( const char * history_key ),
edf_history_take            ( const char * history_key ),
edf_history_argv            ( const char * history_key,
                              const char * substring ),
edf_read_header_history     ( int stream, long DataNumber,
                              int DataChain, const char * history_key,
                              int * pErrorValue, int * pstatus ),
edf_write_header_history    ( int stream, long DataNumber,
                              int DataChain, const char * history_key,
                              int * pErrorValue, int * pstatus ), 
edf_history_read_header     ( const char * header_key,
                              const char * history_key,
                              int * pErrorValue, int * pstatus ),
edf_history_write_header    ( const char * header_key,
                              const char * history_key,
                              int * pErrorValue, int * pstatus ),
edf_history_copy            ( const char * history_key_copy,
                              const char * history_key ),
edf_history_free            ( const char * history_key ),
edf_history_print           ( FILE * out, const char * history_key,
                              int level, int verbose );

/***************************************************************************
* Key Order routines                                                       *
***************************************************************************/
PUBLIC extern void
edf_keyorder_set_table      ( const char * table[] ),
edf_keyorder_print_table    ( FILE * out );

/***************************************************************************
* Header routines                                                          *
***************************************************************************/
PUBLIC extern int
edf_new_header              ( const char * header_key ),
edf_read_header             ( int stream, long DataNumber,
                              int DataChain, const char * header_key,
                              int * pErrorValue, int * pstatus ),
edf_write_header            ( int stream, long DataNumber,
                              int DataChain, const char * header_key,
                              int * pErrorValue, int * pstatus ),
edf_first_header_element    ( const char * header_key,
                              const char ** pkey, const char ** pvalue,
                              int * pErrorValue, int * pstatus ),
edf_next_header_element     ( const char * header_key,
                              const char ** pkey, const char ** pvalue,
                              int * pErrorValue, int * pstatus ),
edf_search_header_element   ( const char * header_key,
                              const char * key, const char ** pvalue,
                              int * pErrorValue, int * pstatus ),
edf_add_header_element      ( const char * header_key,
                              const char * key, const char * value,
                              int * pErrorValue, int * pstatus ),
edf_delete_header_element   ( const char * header_key,
                              const char * key,
                              int * pErrorValue, int * pstatus ), 
edf_free_header             ( const char * header_key ),
edf_print_header            ( FILE * out, const char * header_key,
                              int level, int verbose );

/***************************************************************************
* Internal Parameter Definitions                                           *
***************************************************************************/

#define EdfMaxFiles                 20  /* maximum number of file streams */
#define EdfMaxKeyLen                64      /* maximum length of keywords */
#define EdfMaxValLen              2048        /* maximum length of values */
#define EdfMaxLinLen              1023             /* maximum line length */
#define EdfMaxDataChains            20 /* max. number of user data chains */
#define EdfBufferSize              512            /* size of input buffer */
#define EdfMaxDimensions EdfBufferSize    /* maximum number of dimensions */
#define EdfNDigitsFloat           7 /* number of significant float digits */

/***************************************************************************
* Internal Parameter Functions (return the above defined parameters)       *
***************************************************************************/

PUBLIC extern unsigned long
edf_maxfiles             ( void ),
edf_maxkeylen            ( void ),
edf_maxvallen            ( void ),
edf_maxlinlen            ( void ),
edf_maxdatachains        ( void ),
edf_buffersize           ( void ),
edf_maxdimensions        ( void );

PUBLIC extern int
edf_byteorder            ( void );           /* return internal byteorder */

/***************************************************************************
* Symbolic Names of Error Values                                           *
***************************************************************************/

# define status_error           100
# define status_success         0

/***************************************************************************
* Error Values                                                             *
***************************************************************************/

enum ErrorValues {
RoutineFailed = -1,
RoutineSucceeded,
CouldNotMallocMemory,
CouldNotFreeHeaders,
CouldNotGetBinaryArray,
NoMoreStreamsAvailable,
InvalidStream,
CouldNotOpenFile,
EndOfFileDetected,
CompressionError,
CouldNotFindHeader,
CouldNotFindSymbol,
BadSizeDefinition,
BadDataBlock,
CouldNotFindKeyword,
WriteDataError,
ReadDataError,
NoFileOpen,
NotESRFDataFile,
NoDataBlocksFound,
ExternalData,
FileIsNotWritable,
FileIsNotOpened,
IncompatibleOpeningModes,
CouldNotCloseFile,
CouldNotCloseBlock,
CouldNotInsertChain,
CouldNotInsertBlock,
CouldNotInsertSymbol,
MissingKeyDefinition,
GeneralBlockNotFirst,
ErrorCreatingGeneralBlock,
ErrorReadingGeneralBlock,
ErrorLocatingBlocks,
CouldNotSetBuffer,
NumberConversionFailed,
DataConversionFailed,
DataBufferTooSmall,
MissingArrayDimensions,
NotNdData,
Not2dData,
CouldNotWriteDimension,
CouldNotReadDimension,
CouldNotWriteBinary,
CannotReOpenGeneralBlock,
CannotOpenAsBslFile,
CouldNotInsertElement,
CouldNotDeleteString,
CouldNotFindHeaderKey,
CouldNotCreateHeader,
CouldNotReadGeneralHeader,
CouldNotReadHeader
};

/* The following enums start with 1. 0 is used to specify an invalid value.
   The string tables start with "InValid" and end with (char *) NULL. */

/***************************************************************************
* Data Type Translation Tables                                             *
***************************************************************************/

enum DType { InValidDType,
             Unsigned8=1, Signed8,      Unsigned16,    Signed16,
             Unsigned32,  Signed32,     Unsigned64,    Signed64,
             FloatIEEE32, DoubleIEEE64, Unused11,      Unused12,
             FloatVAX32,  DoubleVAX64,  FloatConvex32, DoubleConvex64,
             EndDType };

enum MType { InValidMType,
             MUnsignedChar=1,    MChar,       MUnsignedShort, MShort,
             MUnsignedInteger,   MInteger,    MUnsignedLong,  MLong,
             MFloat,             MDouble,
             EndMType };

/***************************************************************************
* Byte Order Translation Tables                                            *
***************************************************************************/

enum BOrder { InValidBOrder, 
              LowByteFirst=1, HighByteFirst, 
              EndBOrder };

/***************************************************************************
* Data block classes and instances                                         *
***************************************************************************/

enum DBClass      { InValidDBClass, 
                    DBGeneral, 
                    DBImage, 
                    EndDBClass };
enum DBInstance   { InValidDBInstance, 
                    DBPrimaryData, 
                    DBError, 
                    EndDBInstance };

#endif
#ifndef EDFIO 
#  define EDFIO 
#endif
/************************************************************************---*/

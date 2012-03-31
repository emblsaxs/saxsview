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

# define NUMIO_VERSION      "numio : V1.34 Peter Boesecke 2011-06-16"
/*+++------------------------------------------------------------------------
NAME
  numio.c --- number expressions

SYNOPSIS
  # include numio.h

INCLUDE FILES
  numio.h
  gamma.h

TO LINK WITH
  gamma.c

PURPOSE
  Reading of double and long integer expressions from strings.
  See PUBLIC functions for detail.

CALL
  long   num_str2long(const char *str, const char **tail, int *perrval);
  double num_str2double(const char *str, const char **tail, int *perrval);

AUTHOR
  1995 Peter Boesecke (PB)

HISTORY
  11-Oct-1996 PB extracted from input.c
  13-Oct-1996 PB dpconstant : physical constants and units,
                 units are preceeded by an underscore '_'
  02-Aug-2000 PB dpterm : case '%' added
                 function doubleexpr added
  28-Nov-2000 PB dpfunction : GAMMA_ added
                 dpconstant : km3, ..., m3 added
  04-Dec-2000 PB ->numio.c, .h
  04-Feb-2003 PB Inf
  03-Aug-2003 PB longexpr, floatexpr, doubleexpr: char * ->  const char *
  19-Feb-2004 PB dpfunction : min, max
  02-Mar-2004 PB If a factor is followed by the underscore operator '_' 
                 it is immediately multiplied with the factor following the
                 underscore. Parentheses around both factors are not necessary
                 in this case. This simplifies the use of units, e.g. 
                 "1/1_nm" is identical to "1/(1*nm)". 
                 It was necessar to change the following functions: 
                 dpconstant: unit identifier '_' removed,
                             no distinction between units and names any more
                             names and units ordered by length
                 dpfactor:   new multiplicator '_'
                 Because a unit does not start any more with an underscore 
                 all macros using units must be adapted. 
  16-Mar-2004 PB SaxsExpression -> numio V1.00
  24-Mar-2004 PB STRNCASECMP -> num_strncasecmp
  30-Mar-2004 PB error corrected: 
                 dpexpression and lvexpression stop at white space or comma
  02-Apr-2004 PB parameter tail added to argument list of num_str2...
                 num_str2... stops at a white space, a comma or a semicolon.
                 If the evaluated expression is not complete or faulty, an 
                 error is returned.
  03-Apr-2004 PB new units kg, J, W, pixel, photon
  13-Jun-2004 PB double constant list (pixel and photon no longer defined)
  19-Jun-2004 PB list rearranged, numio_debug created,
                 name of electron charge changed from e to ec
                 new units and constants: erg, dyn, cal, Pa, bar, 
                 N, V, A, C, lb, in, ft, lbf, psi, gN, ga
  20-Jun-2004 PB debug mode 0|1|2, consistency check of units
  07-Jul-2004 PB show quantity in debug mode,
                 barn symbol is b instead of barn,
                 new units Ar (a), poundal (pdl)
  08-Jul-2004 PB new units Neugrad (gon), knots (kn), Kelvin (K)
  05-Feb-2005 PB physical constants from CODATA 2002, 
                 SI prefixes extended: Peta - Yotta, zepto - yocto
                 previous definition of Exa corrected to Peta
                 Units from PTB 2004
                 mi -> mile after PTB 2004, unit Hz, constant mn
                 inconsistent definition of amu in 2002 and 2004, using 2002,
                 unit pond without prefixes, because mp hides proton rest
                 mass mp
  16-May-2005 PB Loop in dpfactor removed and dpfactor splitted into 
                 dpfactor1 and dpfactor2
  21-May-2005 PB isvariable added
  28-Jun-2005 PB test version 
                 NumProgramError, NumNoVariable, NumVar, dpprogram_run
                 num_str2long calculates dp values with temporary program.
                 next step: remove return value from all dp routines
  29-Jun-2005 PB The file numprog.h has been included into numio.h.
                 The file numprog.c is included into the code of numio.c.
                 num_str2double creates a temporary program that is 
                 executed to calculate the value, new functions
                 num_str2prog4, num_runprog4, num_rmprog.
  30-Jun-2005 PB num_str2prog4 and num_runprog4 replaced by 
                 num_str2prog and num_runprog using a variable argument
                 list (stdarg.h), numprog.c and numprog.h copied into
                 numio.c and numio.h
  06-Aug-2005 PB num_chkvar added
  15-Sep-2005 PB dpvariable: Used is incremented and not Value
                 num_chkvar corrected
  11-Dec-2005 PB print routines declared in numio.h
                 length routines: num_prog_variables, num_prog_accumulators,
                 num_prog_instructions, num_prog_variable_size,
                 num_prog_accumulator_size, num_prog_instruction_size
                 num_prog_size, num_prog_size_all
                 numprog_up_accumulator: If CurrentAccumulator is NULL,
                 next is initialized with next = program->AccumulatorList
                 and not with NULL, to force the reuse of already allocated
                 accumulators. Otherwise each call to dpprogram_run would
                 allocate an accumulator with number 1. 
  15-Mar-2006 PB dpconstant_print: printf argument mismatch corrected
  13-Jun-2006 PB units added: liter, minute, hour, day,
                       POW calculation corrected
  19-Apr-2007 PB code corrected to avoid compiler warnings with -Wall
  18-Jun-2007 V1.21 PB num_str2double, num_str2num: If str is the null pointer
                       it is handled like an empty string.
  19-Jul-2007 V1.22 PB units degK, degC, degF added, functions degC2K, 
                       degF2K, degK2K, K2degC, K2degF, K2degK added,
                       isfunction: extended to capital characters 'A'-'Z'
  08-Feb-2008 V1.23 PB CEIL: cosh corrected to ceil
  21-May-2008 V1.24 PB binary constants added
  22-May-2008 V1.25 PB num_double2hex added 
  23-May-2008 V1.26 PB num_double2hex
  20-Mar-2009 V1.27 PB char **tail -> const char **tail
  21-Mar-2009 V1.28 PB logical operators added:
                       NOT, EQU, NEQ, LE, LT, GE, GT, AND, OR, IF
  06-Oct-2009 V1.29 PB pi constant NUM_PI defined in numio.h
                       (not used internally)
  30-Jan-2011 V1.30 PB %g -> %lg
                       double2s shortened
  31-Jan-2011 V1.31 PB lcc does not like %ld and %lg on the same line,
                       splitted to make compiler happy
  09-Mar-2011 V1.32 PB double2s: format corrected to avoid leading spaces,
                       in all public functions: perrval can be NULL
  01-Jun-2011 V1.33 PB dpprogram_step: unique error exit
  16-Jun-2011 V1.34 PB double constants marked, e.g. 1->1.0 

--------------------------------------------------------------------------*/

/****************************************************************************
*  Include                                                                  *
****************************************************************************/
# include "numio.h"

/****************************************************************************
* Static Variables                                                          *
****************************************************************************/
static int NUMIO_debug = 0;

/****************************************************************************
*  Enum (Basic Instructions)                                                *
*  New functions must be added to these tables. Each function needs an      *
*  entry in dpprogram_step.                                                 *
*  ATTENTION: InValidNumCommand must have the value 0                       *
*             Do not change PUSHVAL, PUSHADDR                               *
****************************************************************************/
 
enum NumCommand {
  InValidNumCommand, PUSHVAL, PUSHADDR, NEG, MUL,
  NOT, EQU, NEQ, LE, LT, 
  GE, GT, AND, OR, IF,
  DIV, REST, ADD, SUB, RAD,
  DEG, PI, SIN, COS, TAN,
  ASIN, ACOS, ATAN, ATAN2, SINH,
  COSH, TANH, FLOOR, CEIL, FABS,
  EXP, LOG, LOG10, POW, SQRT,
  ROUND, GAMMA, FMIN, FMAX, DEGC2K, 
  K2DEGC, DEGF2K, K2DEGF, DEGK2K, K2DEGK,
  DEGF2DEGC, DEGC2DEGF,
  EndNumCommand
};
 
static const char * NumCommandStrings[] = {
  "Invalid", "PUSHVAL", "PUSHADDR", "NEG", "MUL",
  "NOT", "EQU", "NEQ", "LE", "LT", 
  "GE", "GT", "AND", "OR", "IF",
  "DIV", "REST", "ADD", "SUB", "RAD",
  "DEG", "PI", "SIN", "COS", "TAN",
  "ASIN", "ACOS", "ATAN", "ATAN2", "SINH",
  "COSH", "TANH", "FLOOR", "CEIL", "FABS",
  "EXP", "LOG", "LOG10", "POW", "SQRT",
  "ROUND", "GAMMA", "FMIN", "FMAX", "DEGC2K", 
  "K2DEGC", "DEGF2K", "K2DEGF", "DEGK2K", "K2DEGK",
  "DEGF2DEGC", "DEGC2DEGF",
  (const char *) NULL
};

/****************************************************************************
* Number Program Routines                                                   *
****************************************************************************/

/****************************************************************************
*  Defines                                                                  *
****************************************************************************/
# define NumBUFLEN 128

/***************************************************************************
* Number Program Static Variables                                          *
***************************************************************************/
static int NUMPROG_init = 0;                                 /* init flag */
static NumProg * NumProgRoot = (NumProg *) NULL;          /* program root */

/***************************************************************************
* Number Program Functions                                                 *
***************************************************************************/

int numprog_init                   ( void );

NumProg * numprog_new              ( const char * Name );

int numprog_insert                 ( const char * Name,
                                     NumProg **pprogram );
int numprog_remove                 ( const char * Name );
int numprog_search                 ( const char * Name, 
                                     NumProg **pprogram );
int numprog_free                   ( NumProg *program );
 
int numprog_append_variable        ( NumProg *program, const char * Key,
                                     double InitValue,
                                     NumVar **pvariable );
int numprog_search_variable        ( NumProg *program, const char *Key,
                                     NumVar **pvariable, int mode );
int numprog_free_variable_list     ( NumProg *program );
long num_prog_variables            ( NumProg *program );
size_t num_prog_variable_size       ( NumProg *program );
int num_prog_print_variable_list   ( FILE * out, NumProg *program, 
                                     int level, int verbose );

NumAccu *numprog_up_accumulator    ( NumProg *program, double Value );
NumAccu *numprog_down_accumulator  ( NumProg *program );
int numprog_free_accumulator_list  ( NumProg *program );
long num_prog_accumulators         ( NumProg *program );
size_t num_prog_accumulator_size    ( NumProg *program );
int num_prog_print_accumulator_list( FILE * out, NumProg *program, 
                                     int level, int verbose );

int numprog_append_instruction     ( NumProg *program, int mode,
                                     int Command, int Nargs,
                                     double Value, double *Address,
                                     NumInstr **pinstruction );
 
int numprog_free_instruction_list  ( NumProg *program, int mode );
long num_prog_instructions         ( NumProg *program, int mode );
size_t num_prog_instruction_size   ( NumProg *program, int mode );
int num_prog_print_instruction_list( FILE * out, NumProg *program, int mode,
                                     int level, int verbose );

int num_prog_print_list            ( FILE * out, NumProg * program,
                                     int level, int verbose );
 
/*---------------------------------------------------------------------------
NAME
 
   numprog_newstr --- allocate memory and copy a character string into it
 
SYNOPSIS
 
   char * numprog_newstr( const char * string );
 
DESCRIPTION
  Allocates strlen(´string´)+1 bytes of memory and copies ´string´ into it.
  In case of success the pointer to the allocated memory is returned. The
  null pointer is returned in case of an error.
  If ´string´ is the NULL pointer the NULL pointer is returned.
 
RETURN VALUE
  Returns the pointer to the allocated string or (char *) NULL in case
  of an error.
---------------------------------------------------------------------------*/
char * numprog_newstr( const char * string )
{ char * newstring;
 
  if (!string) return( (char *) NULL );
  if (!(newstring = (char *) malloc(strlen(string)+1))) return((char *) NULL);
  (void) strcpy(newstring,string);
 
  return( newstring );
 
} /* numprog_newstr */

/*---------------------------------------------------------------------------
NAME
 
   numprog_checkvar --- check whether variable name contains invalid chars
 
SYNOPSIS
 
   int numprog_checkvar( const char * string );
 
DESCRIPTION

   Checks, whether the input string contains only allowed characters.

   first character: ['a'..'z'], ['A'..'Z'] (isalpha)
   all other characters: ['0'..'9'], ['a'..'z'], ['A'..'Z'] (isalnum)
 
RETURN VALUE
  0: success, -1: error
---------------------------------------------------------------------------*/
int numprog_checkvar( const char * string )
{ 
  const char * ps;

  ps=string;

  if (!ps) return(-1);

  if (*ps) {
    if (isalpha((int) *ps)==0) return(-1);
    ps++;
  }

  while (*ps) {
    if (isalnum((int) *ps)==0) return(-1);
    ps++;
  }
 
  return( 0 );
 
} /* numprog_checkvar */
 
/*---------------------------------------------------------------------------
NAME

  numprog_cmd2str --- converts NumCommand to a string

SYNOPSIS

  NumCommand numprog_cmd;
  const char * numprog_cmd2str( int numprog_cmd )

RETURN VALUE
  Pointer to a constant result string.
  -------------------------------------------------------------------------*/
const char * numprog_cmd2str( int numprog_cmd )
{
   if ((numprog_cmd<0)||(numprog_cmd>=EndNumCommand)) 
     numprog_cmd = InValidNumCommand;
   return( NumCommandStrings[numprog_cmd] );

} /* numprog_cmd2str */

/*---------------------------------------------------------------------------
NAME

  numprog_ins2str --- converts NumInstr to a string

SYNOPSIS

  NumCommand numprog_cmd;
  const char * numprog_ins2str( char buffer[], size_t buflen, NumInstr * ins )

RETURN VALUE
  success: Pointer to a constant result string.
  -------------------------------------------------------------------------*/
const char * numprog_ins2str ( char buffer[], size_t buflen, NumInstr * ins )
{ 
  char * pb;

  buffer[0] = (char) 0;

  pb = buffer;
  if (buflen<50) return(pb);

  switch (ins->Command) {
    case PUSHVAL: /* value parameter */
      sprintf(pb,"%10s  %10g",
        numprog_cmd2str(ins->Command), ins->Value);
      break;
    case PUSHADDR: /* address parameter */
      sprintf(pb,"%10s *%10p",
        numprog_cmd2str(ins->Command), ins->Address);
      break;
    default: /* no parameter */
      sprintf(pb,"%10s  %10s (%d args)", 
        numprog_cmd2str(ins->Command), "", ins->Nargs);
      break;
  }
  
  return( pb );

} /* numprog_ins2str */

/*---------------------------------------------------------------------------
NAME

  numprog_init --- Initializes the module

SYNOPSIS

  int numprog_init ( void );

DESCRPTION

  Initializes the program lists. It should be called by routines
  that access directly NumProgRoot, if NUMPROG_init is not set.
  Currently, only the initialization flag is set to 1.

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int numprog_init ( void )
{
  NUMPROG_init = 1;

  return(0);

} /* numprog_init */

/*---------------------------------------------------------------------------
NAME

  numprog_new --- Create a program

SYNOPSIS

  NumProg * numprog_new        ( const char * Name );

DESCRPTION

  Creates new program with Name and returns a pointer to it.
  An existing program is removed and reallocated with a different pointer.

RETURN VALUE

  success: POINTER, error: NULL

---------------------------------------------------------------------------*/
PUBLIC NumProg * numprog_new            ( const char * Name )
{ NumProg * program;

  if (numprog_search ( Name, &program )) 
    return ( (NumProg *) NULL ); 

  if (numprog_free( program ))
    return ( (NumProg *) NULL ); 

  if (numprog_insert ( Name, &program )) 
    return ( (NumProg *) NULL ); 

  return( program );
   
} /* numprog_new */

/*---------------------------------------------------------------------------
NAME

  numprog_insert --- Insert a program

SYNOPSIS

  int numprog_insert          ( const char * Name,
                                  NumProg **pprogram );

DESCRPTION

  Insert a program with Name and update *pprogram with the pointer to 
  program. If 'Name' already exists only *pprogram is updated.
  In case of success the pointer to program is returned in *pprogram, 
  otherwise NULL.

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int numprog_insert          ( const char * Name,
                              NumProg **pprogram )
{
  NumProg * newprogram, * next, * previous;
  int notfound = 1;

  if (!NUMPROG_init) numprog_init();

  if (pprogram) *pprogram = (NumProg *) NULL;

  if (!Name) return(-1);

  previous = (NumProg *) NULL;
  next = NumProgRoot;

  /* search insertion point (insertion before next) */
  while ( ( next!=(NumProg *) NULL ) && (notfound>0) ) {
    notfound = strcmp(next->Name,Name);
    if (notfound>0) {previous = next; next = next->Next;}
    }

  /* create new program Name, if notfound */
  if ( notfound ) {
    /* create new program Name */
    if (!(newprogram = (NumProg*) malloc(sizeof(NumProg)))) return(-1);

    newprogram->Name            = numprog_newstr( Name );
    if (!newprogram->Name) return(-1);

    newprogram->VariableList    = (NumVar *) NULL;
    newprogram->AccumulatorList = (NumAccu *) NULL;
    newprogram->CurrentAccumulator = (NumAccu *) NULL;
    newprogram->InstructionList = (NumInstr *) NULL;
    newprogram->CompiledList    = (NumInstr *) NULL;

    /* insert newprogram before next */
    if (next) next->Previous = newprogram;
    newprogram->Next=next;
    newprogram->Previous=previous;
    if (previous) previous->Next=newprogram;
       else NumProgRoot = newprogram;

    next = newprogram;
  }

  if (pprogram) *pprogram = next;

  return(0);

} /* numprog_insert */

/*---------------------------------------------------------------------------
NAME

  numprog_search --- search program

SYNOPSIS

  int numprog_search ( const char * Name, NumProg **pprogram )

DESCRPTION

  Search program Name in the program list. If found, the pointer to program 
  is returned, otherwise NULL. All characters of Name are compared.
  Name is searched from the beginning of the list which is inversely
  lexicographically ordered.

RETURN VALUE

  Attention: The return value indicates errors only. It does not indicate 
             whether the program was found.

  success:0, error:-1

---------------------------------------------------------------------------*/
int numprog_search ( const char * Name, NumProg **pprogram )
{
  NumProg * current;

  if (!NUMPROG_init) numprog_init();

  if (pprogram) *pprogram = (NumProg *) NULL;

  if (!Name) return(-1);

  /* search Name */
  current = NumProgRoot;

  if ( current!=(NumProg *) NULL )
    while( ( current!=(NumProg *) NULL ) &&
           ( strcmp(current->Name,Name)!=0) ) {
      current = current->Next;
    }

  if (pprogram) *pprogram = current;

  return(0);

} /* numprog_search */

/*---------------------------------------------------------------------------
NAME

   numprog_free

SYNOPSIS

   int numprog_free( NumProg * program );

DESCRIPTION

Removes program from NumProgRoot and releases its contents

RETURN VALUE

  success:0, error:-1
---------------------------------------------------------------------------*/
PUBLIC int numprog_free( NumProg * program )
{ NumProg * previous, *next;

  if (!NUMPROG_init) numprog_init();

  /* nothing to do, if NULL pointer is given */
  if (program==(NumProg *) NULL) return(0);

  /* check, whether program is in NumProgRoot list */
  next=NumProgRoot;
  while ((next)&&(program!=next)) next=next->Next;
  if (next==(NumProg *) NULL) return(-1);

  /* change links */
  previous = program->Previous;
  next = program->Next;

  if ( next != (NumProg *) NULL ) next->Previous = previous;
  if ( previous != (NumProg *) NULL ) previous->Next = next;
    else NumProgRoot = next;

  /* free program and its elements */
  if (numprog_free_variable_list ( program )) return(-1);
  if (numprog_free_accumulator_list ( program )) return(-1);
  if (numprog_free_instruction_list ( program, 0 )) return(-1);
  if (numprog_free_instruction_list ( program, 1 )) return(-1);
  if (program->Name) free(program->Name);
  free(program);

  return(0);

} /* numprog_free */

/*---------------------------------------------------------------------------
NAME

  numprog_remove --- remove program(s)

SYNOPSIS

  int numprog_remove ( const char * Name );

DESCRIPTION
Removes program Name and its contents.
If called with Name == (char *) NULL, all programs are removed

RETURN VALUE

  success:0, error:-1

--------------------------------------------------------------------------+*/
int numprog_remove ( const char * Name )
{ NumProg * next, * current;

  if (!NUMPROG_init) numprog_init();

  if ( Name != (char *) NULL ) {
    /* search program ´Name´ */
    if (numprog_search( Name, &current )) return(-1);
    /* remove current program */
    if (numprog_free( current )) return(-1);
  } else {
    next = NumProgRoot;
    while ( next != (NumProg *) NULL ) {
      current = next;
      next = current->Next;
      /* remove current program */
      if (numprog_free( current )) return(-1);
    }
  }

  return( 0 );

} /* numprog_remove */

/*---------------------------------------------------------------------------
NAME

  num_prog_size --- return program size

SYNOPSIS

  size_t num_prog_size( NumProg * program );

DESCRIPTION

  Returns size of program or 0 if program is NULL.

RETURN VALUE

  program size in bytes

---------------------------------------------------------------------------*/
PUBLIC size_t num_prog_size( NumProg * program )
{
  size_t prog_size=0;

  if (!NUMPROG_init) numprog_init();

  if (program!=(NumProg *) NULL) {
    prog_size += sizeof( NumProg );
    prog_size += strlen( program->Name )+1;
    prog_size += num_prog_variable_size(program);
    prog_size += num_prog_accumulator_size(program);
    prog_size += num_prog_instruction_size(program,0);
    prog_size += num_prog_instruction_size(program,1);
  }

  return(prog_size);

} /* num_prog_size */

/*---------------------------------------------------------------------------
NAME

  num_prog_size_all --- return size of all programs

SYNOPSIS

  size_t num_prog_size_all( void );

DESCRIPTION

  Returns size of all programs.

RETURN VALUE

  program size in bytes

---------------------------------------------------------------------------*/
PUBLIC extern size_t num_prog_size_all ( void )
{
  NumProg * program;
  size_t prog_size=0;

  if (!NUMPROG_init) numprog_init();

  program = NumProgRoot;

  while(program!=(NumProg *) NULL)
    prog_size += num_prog_size ( program );

  return(prog_size);

} /* num_prog_size_all */

/*---------------------------------------------------------------------------
NAME
    
  num_prog_print_list --- print program(s)
    
SYNOPSIS 
    
  int num_prog_print_list( FILE * out, NumProg * program, 
                           int level, int verbose );

DESCRIPTION
Prints program list to out.

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
PUBLIC int num_prog_print_list( FILE * out, NumProg * program, 
                          int level, int verbose )
{ const char * SeparationLine =
    "- - - - - - - - - - - - - - - - - - - - - - - - - - - - -";

  NumProg * currentprogram;

  if (!NUMPROG_init) numprog_init();

  if (level<1) return(0);

  if (program) currentprogram = program;
  else currentprogram = NumProgRoot;

  while(currentprogram!=(NumProg *) NULL) {
    if (verbose) {
      fprintf(out," %s\n",SeparationLine);
      fprintf(out," Name                = %s\n",currentprogram->Name);

      num_prog_print_variable_list(out,currentprogram,level-1,verbose);
      num_prog_print_accumulator_list(out,currentprogram,level-1,verbose);
      num_prog_print_instruction_list(out,currentprogram,0,level-1,verbose);
//      num_prog_print_instruction_list(out,currentprogram,1,level-1,verbose);

      fprintf(out," Previous program = ");
      if ((currentprogram->Previous)!=(NumProg *) NULL)
        fprintf(out,"%s\n", currentprogram->Previous->Name);
        else fprintf(out,"(no previous program)\n");
      fprintf(out," Next program     = ");
      if ((currentprogram->Next)!=(NumProg *) NULL)
        fprintf(out,"%s\n", currentprogram->Next->Name);
        else fprintf(out,"(no next program)\n");
      if (currentprogram->CurrentAccumulator) {
        // fprintf splitted to make lcc happy
        fprintf(out," CurrentAccumulator  = #%ld", 
          currentprogram->CurrentAccumulator->Number);
        fprintf(out," (Value = %lg)\n",
          currentprogram->CurrentAccumulator->Value);
      } else fprintf(out," CurrentAccumulator  = (no current accumulator)\n");
    } else {
      fprintf(out," Program             = '%s'\n",currentprogram->Name);
      num_prog_print_variable_list(out,currentprogram,level-1,verbose );
      num_prog_print_instruction_list(out,currentprogram,0,level-1,verbose );
//      num_prog_print_instruction_list(out,currentprogram,1,level-1,verbose );
      num_prog_print_accumulator_list(out,currentprogram,level-1,verbose );
    }
    if (program) currentprogram = (NumProg *) NULL;
    else currentprogram=currentprogram->Next;
  }

  if (verbose) fprintf(out," %s\n",SeparationLine);

  return(0);

} /* num_prog_print_list */

/*---------------------------------------------------------------------------
NAME
 
  numprog_append_variable --- Appends a variable to the end of the list
 
SYNOPSIS
 
  int numprog_append_variable  ( NumProg *program, const char * Key,
                                 double InitValue, NumVar **pvariable );
 
DESCRPTION
 
  Appends a variable with name Key to the variable list of program and
  updates *pvariable with the pointer to the variable. If 'Key' already
  exists only *pvariable is updated and an error is returned. 
  If 'Key' does not exist the new variable is appended to the end of the list.
  The pointer to the variable is returned in *pvariable.
 
RETURN VALUE
 
  success:0, error:-1
 
---------------------------------------------------------------------------*/
PUBLIC int numprog_append_variable  ( NumProg *program, const char * Key,
                                      double InitValue, NumVar **pvariable )
{ NumVar * newvariable, * next, * previous;
  int notfound = 1;
 
  if (pvariable) *pvariable = (NumVar *) NULL;
 
  if (!Key) return(-1);

  /* Check, whether Key contains only allowed characters */
  if ( numprog_checkvar( Key ) ) return(-1); 
 
  /* Nothing to do if program pointer is NULL */
  if ( !program ) return(0);
 
  previous = (NumVar *) NULL;
  next = program->VariableList;
 
  /* search insertion point (insertion before next) */
  while ( ( next!=(NumVar *) NULL ) && (notfound!=0) ) {
    notfound = strcmp(next->Key,Key);
    if (notfound!=0) {previous = next; next = next->Next;}
    }
 
  /* create new variable Key, if notfound */
  if ( notfound ) {
    /* create new variable Key */
    if (!(newvariable = (NumVar*) malloc(sizeof(NumVar)))) return(-1);
 
    newvariable->Key      = numprog_newstr( Key );
    if (!newvariable->Key) return(-1);
 
    newvariable->Value    = InitValue;
    newvariable->Used     = 0;
 
    /* insert newvariable before next */
    if (next) next->Previous = newvariable;
    newvariable->Next=next;
    newvariable->Previous=previous;
    if (previous) previous->Next=newvariable;
       else program->VariableList = newvariable;
 
    next = newvariable;
  }
 
  if (pvariable) *pvariable = next;

  if (!notfound) return(-1);
 
  return(0);
 
} /* numprog_append_variable */

/*---------------------------------------------------------------------------
NAME

  numprog_search_variable --- search variable 

SYNOPSIS

  int numprog_search_variable        ( NumProg *program, const char *Key,
                                         NumVar **pvariable, int mode );

DESCRPTION

  Searches variable in the variable list of program. In case of success 
  the pointer to variable is returned in *pvariable, otherwise NULL.
  If mode==0, all characters of Key are compared,
  if mode==1, the comparison stops if a variable is found where all
              characters match the first characters of Key.
  Key is searched from the beginning of the list which is inversely
  lexicographically ordered.

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int numprog_search_variable        ( NumProg *program, const char *Key,
                                       NumVar **pvariable, int mode )
{ 
  NumVar * current;

  if (pvariable) *pvariable = (NumVar *) NULL;

  if (!Key) return(-1); 

  /* Nothing to search if program pointer is NULL */
  if ( !(NumProg * ) program ) return(-1);

  /* search Key */
  current = program->VariableList;

  if ( current!=(NumVar *) NULL ) {
    if (mode==0) {
      while( ( current!=(NumVar *) NULL ) &&
             ( strcmp(current->Key,Key)!=0) ) {
        current = current->Next;
      }
    } else {
      while( ( current!=(NumVar *) NULL ) &&
             ( strncmp(current->Key,Key,strlen(current->Key))!=0) ) {
        current = current->Next;
      }
    }
  }
  if (pvariable) *pvariable = current;

  if (current==(NumVar *) NULL) return(-1);

  return(0);

} /* numprog_search_variable */

/*---------------------------------------------------------------------------
NAME
    
  numprog_free_variable_list --- remove variable list 
    
SYNOPSIS 
    
  int numprog_free_variable_list ( NumProg * program );

DESCRIPTION

Removes variable list from program 
    
RETURN VALUE

  success:0, error:-1

--------------------------------------------------------------------------+*/
int numprog_free_variable_list ( NumProg * program )
{ NumVar * variable, * next;

  /* Nothing to do if no program pointer is given */
  if (!program) return(0);

  next = program->VariableList;
  program->VariableList = (NumVar *) NULL;

  while(next!=(NumVar*) NULL) {
    variable = next;
    next=next->Next;
    free(variable->Key);
    free(variable);
    }

  return(0);

} /* numprog_free_variable_list */

/*---------------------------------------------------------------------------
NAME

  num_prog_variables --- return number of variables 

SYNOPSIS

  long num_prog_variables ( NumProg *program );

DESCRIPTION

Return number of variables.

RETURN VALUE

  number of variables

---------------------------------------------------------------------------*/
PUBLIC long num_prog_variables ( NumProg *program )
{ NumVar * variable, * next;
  long variable_length=0l;

  /* Nothing to do if no program pointer is given */
  if (!program) return(variable_length);

  next = program->VariableList;

  while(next!=(NumVar*) NULL) {
    variable = next;
    next=next->Next;
    variable_length++;
    }

  return(variable_length);

} /* num_prog_variables */

/*---------------------------------------------------------------------------
NAME

  num_prog_variable_size --- return allocated memory size of variable list 

SYNOPSIS

  size_t num_prog_variable_size ( NumProg *program );

DESCRIPTION

  Return allocated memory size of variable list.

RETURN VALUE

  allocated memory size

---------------------------------------------------------------------------*/
PUBLIC size_t num_prog_variable_size       ( NumProg *program )
{ NumVar * variable, * next;
  size_t variable_size=0;

  /* Nothing to do if no program pointer is given */
  if (!program) return(variable_size);

  next = program->VariableList;

  while(next!=(NumVar*) NULL) {
    variable = next;
    next=next->Next;
    variable_size += sizeof(NumVar);
    }

  return(variable_size);

} /* num_prog_variable_size */

/*---------------------------------------------------------------------------
NAME
    
  num_prog_print_variable_list --- print variable list of program
   
SYNOPSIS
   
  int num_prog_print_variable_list ( FILE * out, NumProg *program,
                                      int level, int verbose );

DESCRIPTION
Prints program variable list to out.

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
PUBLIC int num_prog_print_variable_list ( FILE * out, NumProg *program,
                                    int level, int verbose )
{ const char * SeparationLine =
    "-   -   -   -   -   -   -   -   -   -   -   -   -   -   -";

  NumVar * variable;

  if (level<1) return(0);

  /* Nothing to print if program pointer is NULL */
  if ( !(NumProg * ) program ) return(0);

  variable = program->VariableList;

  while(variable!=(NumVar *) NULL) {
    if (verbose) {
      fprintf(out,"  %s\n",SeparationLine);
      fprintf(out,"  Variable            = %s\n",variable->Key);
      fprintf(out,"  Value               = %lg\n",variable->Value);
      fprintf(out,"  Used                = %d\n",variable->Used);

      fprintf(out,"  Previous variable = ");
      if ((variable->Previous)!=(NumVar *) NULL)
        fprintf(out,"%s\n", variable->Previous->Key);
        else fprintf(out,"(no previous variable)\n");
      fprintf(out,"  Next variable     = ");
      if ((variable->Next)!=(NumVar *) NULL)
        fprintf(out,"%s\n", variable->Next->Key);
        else fprintf(out,"(no next variable)\n");
    } else {
      fprintf(out,"  '%s' = %lg = *%p\n",
        variable->Key,variable->Value,&(variable->Value));
    }
    variable=variable->Next;
  }

  if (verbose) fprintf(out,"  %s\n",SeparationLine);

  return(0);

} /* num_prog_print_variable_list */

/*---------------------------------------------------------------------------
NAME

  numprog_up_accumulator --- increments program->CurrentAccumulator

SYNOPSIS

  NumAccu *numprog_up_accumulator( NumProg *program, double Value );

DESCRPTION

  Increments program->CurrentAccumulator to next Accumulator. If it does not 
  exist it is appended to the accumulator list of program. The Accumulator is
  initialized with Value. The pointer to the Accumulator is returned.

RETURN VALUE

  success: POINTER , error: NULL

---------------------------------------------------------------------------*/
NumAccu *numprog_up_accumulator( NumProg *program, double Value )
{
  NumAccu * newaccumulator, * next, * current;
  long current_number;

  /* Nothing to do if program pointer is NULL */
  if ( !program ) return( (NumAccu *) NULL );

  current = program->CurrentAccumulator;
  if (current) next = current->Next;
  else next = program->AccumulatorList; // use existing accumulator

  if (current) current_number=current->Number;
  else current_number=0;

  if (!next) {
  /* append new accumulator */
    if (!(newaccumulator = (NumAccu*) malloc(sizeof(NumAccu))))
      return( (NumAccu *) NULL );

    newaccumulator->Number   = current_number+1;

    /* insert newaccumulator before next */

    if (next) next->Previous = newaccumulator;

    newaccumulator->Next=next;
    newaccumulator->Previous=current;
    if (current) current->Next=newaccumulator;
       else program->AccumulatorList = newaccumulator;

    next = newaccumulator;
  }

  next->Value = Value;
  program->CurrentAccumulator = next;

  return( next );

} /* numprog_up_accumulator */

/*---------------------------------------------------------------------------
NAME

  numprog_down_accumulator --- decrements program->CurrentAccumulator 

SYNOPSIS

  NumAccu *numprog_down_accumulator( NumProg *program, double Value );

DESCRPTION

  Decrements program->CurrentAccumulator to previous Accumulator. 
  The pointer to the Accumulator is returned.

RETURN VALUE

  success: POINTER , error: NULL

---------------------------------------------------------------------------*/
NumAccu *numprog_down_accumulator ( NumProg *program )
{
  NumAccu * current, * previous;
  
  /* Nothing to do if program pointer is NULL */
  if ( !program ) return( (NumAccu *) NULL );
  
  current = program->CurrentAccumulator;
  if (current) previous = current->Previous;
  else previous = (NumAccu *) NULL;
  
  program->CurrentAccumulator = previous;

  return( previous );

} /* numprog_down_accumulator */

/*---------------------------------------------------------------------------
NAME

  numprog_free_accumulator_list --- remove accumulator list

SYNOPSIS

  int numprog_free_accumulator_list ( NumProg * program );

DESCRIPTION

Removes accumulator list from program

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int numprog_free_accumulator_list ( NumProg *program )
{ NumAccu * accumulator, * next;

  /* Nothing to do if no program pointer is given */
  if (!program) return(0);

  next = program->AccumulatorList;
  program->AccumulatorList = (NumAccu *) NULL;

  while(next!=(NumAccu*) NULL) {
    accumulator = next;
    next=next->Next;
    free(accumulator);
    }

  return(0);

} /* numprog_free_accumulator_list */

/*---------------------------------------------------------------------------
NAME

  num_prog_accumulators --- return number of available accumulators 

SYNOPSIS

  long num_prog_accumulators ( NumProg *program ):

DESCRIPTION

Return number of available accumulators.

RETURN VALUE

  number of accumulators

---------------------------------------------------------------------------*/
PUBLIC long num_prog_accumulators ( NumProg *program )
{ NumAccu * accumulator, * next;
  long accu_length=0l;

  /* Nothing to do if no program pointer is given */
  if (!program) return(accu_length);

  next = program->AccumulatorList;

  while(next!=(NumAccu*) NULL) {
    accumulator = next;
    next=next->Next;
    accu_length++;
  }

  return(accu_length);

} /* num_prog_accumulators */

/*---------------------------------------------------------------------------
NAME

  num_prog_accumulator_size --- return alloc. memory size of accumulator table 

SYNOPSIS

  size_t num_prog_accumulator_size    ( NumProg *program );

DESCRIPTION

  Return allocated memory size of accumulator table 

RETURN VALUE

  allocated memory size

---------------------------------------------------------------------------*/
PUBLIC size_t num_prog_accumulator_size    ( NumProg *program )
{ NumAccu * accumulator, * next;
  size_t accu_size=0;

  /* Nothing to do if no program pointer is given */
  if (!program) return(accu_size);

  next = program->AccumulatorList;

  while(next!=(NumAccu*) NULL) {
    accumulator = next;
    next=next->Next;
    accu_size += sizeof(NumAccu);
  }

  return(accu_size);

} /* num_prog_accumulator_size */

/*---------------------------------------------------------------------------
NAME
    
  num_prog_print_accumulator_list --- print accumulator list of program
   
SYNOPSIS
   
  int num_prog_print_accumulator_list ( FILE * out, NumProg *program,
                                         int level, int verbose );

DESCRIPTION
Prints program accumulator list to out.

RETURN VALUE

  success:0, error:-1
  
---------------------------------------------------------------------------*/
PUBLIC int num_prog_print_accumulator_list ( FILE * out, NumProg *program,
                                       int level, int verbose )
{ const char * SeparationLine =
    "-   -   -   -   -   -   -   -   -   -   -   -   -   -   -";

  NumAccu * accumulator;

  if (level<1) return(0);

  /* Nothing to print if program pointer is NULL */
  if ( !(NumProg * ) program ) return(0);

  accumulator = program->AccumulatorList;

  while(accumulator!=(NumAccu *) NULL) {
    if (verbose) {
      fprintf(out,"  %s\n",SeparationLine);
      fprintf(out,"  Accumulator          = #%ld\n",accumulator->Number);
      fprintf(out,"  Value                = %lg\n",accumulator->Value);

      fprintf(out,"  Previous accumulator = ");
      if ((accumulator->Previous)!=(NumAccu *) NULL)
        fprintf(out,"#%ld\n", accumulator->Previous->Number);
        else fprintf(out,"(no previous accumulator)\n");
      fprintf(out,"  Next accumulator     = ");
      if ((accumulator->Next)!=(NumAccu *) NULL)
        fprintf(out,"#%ld\n", accumulator->Next->Number);
        else fprintf(out,"(no next accumulator)\n");
    } else {
      // fprintf splitted to make lcc happy
      fprintf(out,"  Accumulator #%ld =", accumulator->Number);
      fprintf(out," %lg\n", accumulator->Value);
    }
    accumulator=accumulator->Next;
  }

  if (verbose) fprintf(out,"  %s\n",SeparationLine);

  return(0);

} /* num_prog_print_accumulator_list */

/*---------------------------------------------------------------------------
NAME

  numprog_append_instruction --- Appends a program instruction 

SYNOPSIS

  int numprog_append_instruction ( NumProg *program, int mode, 
                                     int Command, int Nargs,
                                     double Value, double *Address,
                                     NumInstr **pinstruction );

DESCRIPTION

  mode 0:
  Appends a new instruction to the end of the instruction list that starts at
  program->InstructionList.

  mode 1: 
  Appends a new instruction to the end of the instruction list that starts at
  program->CompiledList.

  Nargs is the number of arguments required by the command, e.g.

  Nargs = 0 : no argument, accumulator number is increased by 1 
  Nargs = 1 : 1 argument, accumulator number is not changed 
  Nargs = N : N arguments, accumulator number is decreased by N-1 

  This information is only used by numprog_optimize

  It updates *pinstruction with the pointer to the new instruction. 
  In case of success the pointer to the instruction is returned in
  *pinstruction, otherwise NULL.

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
PUBLIC int numprog_append_instruction ( NumProg *program, int mode, 
                                   int Command, int Nargs,
                                   double Value, double *Address,
                                   NumInstr **pinstruction )
{
  NumInstr * newinstruction, * next, * previous, **proot;

  if (pinstruction) *pinstruction = (NumInstr *) NULL;

  /* Nothing to append if program pointer is NULL */
  if ( !program ) return(0);

  previous = (NumInstr *) NULL;
  switch (mode) {
    case 0: 
      proot = &(program->InstructionList);
      break;
    case 1: 
      proot = &(program->CompiledList);
      break;
    default:
      /* Error */
      return(-1);
  }
  next = *proot;

  /* search insertion point (insertion before next) */
  while ( ( next!=(NumInstr *) NULL ) ) {
    previous = next; next = next->Next;
    }

  /* create new instruction */
  if (!next) {
    /* create new instruction */
    if (!(newinstruction = (NumInstr*) malloc(sizeof(NumInstr))))
      return(-1);

    newinstruction->Command  = Command;
    newinstruction->Nargs    = Nargs;
    newinstruction->Value    = Value;
    newinstruction->Address  = Address;

    /* insert newinstruction before next */
    if (next) next->Previous = newinstruction;
    newinstruction->Next=next;
    newinstruction->Previous=previous;
    if (previous) previous->Next=newinstruction;
       else *proot = newinstruction;

    next = newinstruction;
  }

  if (pinstruction) *pinstruction = next;

  return(0);

} /* numprog_append_instruction */

/*---------------------------------------------------------------------------
NAME

  numprog_free_instruction_list --- remove instruction list
    
SYNOPSIS
    
  int numprog_free_instruction_list ( NumProg * program, int mode );
    
DESCRIPTION

  mode 0:
  Removes the list starting at program->InstructionList.

  mode 1:
  Removes the list starting at program->CompiledList.

RETURN VALUE

  success:0, error:-1

--------------------------------------------------------------------------+*/
int numprog_free_instruction_list ( NumProg *program, int mode )
{ NumInstr * instruction, * next, **proot;

  /* Nothing to do if program pointer is NULL */
  if (!program) return(0);

  switch (mode) {
    case 0: 
      proot = &(program->InstructionList);
      break;
    case 1: 
      proot = &(program->CompiledList);
      break;
    default:
      /* Error */
      return(-1);
  }
  next = *proot;
  *proot = (NumInstr *) NULL;

  while(next!=(NumInstr*) NULL) {
    instruction = next;
    next=next->Next;
    free(instruction);
    }

  return(0);

} /* numprog_free_instruction_list */

/*---------------------------------------------------------------------------
NAME

  num_prog_instructions --- return number of instructions 

SYNOPSIS

  long num_prog_instructions ( NumProg *program, int mode );

DESCRIPTION

  Return number of instructions.

  mode 0:
  InstructionList.

  mode 1:
  CompiledList.

  mode 2:
  If CompiledList exist returns its length, otherwise 
  return the length of InstructionList 

RETURN VALUE

  number of instructions

---------------------------------------------------------------------------*/
PUBLIC long num_prog_instructions ( NumProg *program, int mode )
{ NumInstr * current;
  long instruction_length=0l;

  /* Nothing to do if program pointer is NULL */
  if (!program) return(instruction_length);

  switch (mode) {
    case 0:
      current = program->InstructionList;
      break;
    case 1:
      current = program->CompiledList;
      break;
    case 2:
      if (program->CompiledList)
        current = program->CompiledList;
      else current = program->InstructionList;
      break;
    default:
      /* Error */
      return(instruction_length);
  }

  while(current!=(NumInstr*) NULL) {
    instruction_length++;
    current=current->Next;
    }

  return(instruction_length);

} /* num_prog_instructions */

/*---------------------------------------------------------------------------
NAME

  num_prog_instruction_size --- return alloc. memory size of instruction list

SYNOPSIS

  size_t num_prog_instruction_size    ( NumProg *program, int mode );

DESCRIPTION

  Return allocated memory size of instruction list

  mode 0:
  InstructionList.

  mode 1:
  CompiledList.

RETURN VALUE

  allocated memory size

---------------------------------------------------------------------------*/
PUBLIC size_t num_prog_instruction_size    ( NumProg *program, int mode )
{ NumInstr * instruction, * next, **proot;
  size_t instruction_size=0;

  /* Nothing to do if program pointer is NULL */
  if (!program) return(instruction_size);

  switch (mode) {
    case 0:
      proot = &(program->InstructionList);
      break;
    case 1:
      proot = &(program->CompiledList);
      break;
    default:
      /* Error */
      return(0);
  }
  next = *proot;

  while(next!=(NumInstr*) NULL) {
    instruction = next;
    next=next->Next;
    instruction_size += sizeof(NumInstr);
    }

  return(instruction_size);

} /* num_prog_instruction_size */

/*---------------------------------------------------------------------------
NAME
    
  num_prog_print_instruction_list --- print instruction list of program
   
SYNOPSIS
   
  int num_prog_print_instruction_list ( FILE * out, NumProg *program,
                                         int mode, int level, int verbose );

DESCRIPTION

  mode 0:
  Prints the list starting at program->InstructionList to out.
  
  mode 1:
  Prints the list starting at program->CompiledList to out.


RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
PUBLIC int num_prog_print_instruction_list ( FILE * out, NumProg *program, 
                                             int mode, int level, int verbose )
{ const char * SeparationLine =
    "-   -   -   -   -   -   -   -   -   -   -   -   -   -   -";
  char buffer[NumBUFLEN];
  size_t buflen=NumBUFLEN;

  NumInstr * instruction, **proot;

  if (level<1) return(0);

  /* Nothing to print if program pointer is NULL */
  if ( !program ) return(0);

  switch (mode) {
    case 0: 
      fprintf(out,"  %s\n","InstructionList");
      proot = &(program->InstructionList);
      break;
    case 1: 
      fprintf(out,"  %s\n","CompiledList");
      proot = &(program->CompiledList);
      break;
    default:
      /* Error */
      return(-1);
  } 
  instruction = *proot;

  while(instruction!=(NumInstr *) NULL) {
    if (verbose) {
      fprintf(out,"  %s\n",SeparationLine);

      fprintf(out,"  Command              = %s\n",
        numprog_ins2str ( buffer, buflen, instruction ) );

      fprintf(out,"  Previous instruction = ");
      if ((instruction->Previous)!=(NumInstr *) NULL)
        fprintf(out,"%s\n", numprog_ins2str ( buffer,buflen,instruction->Previous ));
        else fprintf(out,"(no previous instruction)\n");
      fprintf(out,"  Next instruction     = ");
      if ((instruction->Next)!=(NumInstr *) NULL)
        fprintf(out,"%d\n", instruction->Next->Command);
        else fprintf(out,"(no next instruction)\n");
    } else {
      fprintf(out,"  %s\n", numprog_ins2str ( buffer, buflen, instruction ) );
    }
    instruction=instruction->Next;
  }

  if (verbose) fprintf(out,"  %s\n",SeparationLine);

  return(0);

} /* num_prog_print_instruction_list */

/***************************************************************************
* Double Constant List Structure Definition                                *
***************************************************************************/
typedef struct Double_Constant {
 char * Key;                                     /* pointer to key string */
 double Value;                                   /* Value of the constant */
 char * Quantity;                        /* pointer to description string */
 char * Unit;                                   /* pointer to unit string */
 struct Double_Constant *Previous,*Next;   /* previous and next constants */
} DPConstant;

/****************************************************************************
* Double Constant Static Variables                                          *
****************************************************************************/
static int DPConstantInit = 0;                                /* init flag */
static DPConstant * DPConstantRoot = (DPConstant *) NULL;     /* list root */

static const double Yotta = 1e+24, Yotta2 = 1e+48, Yotta3 = 1e+72;
static const double Zetta = 1e+21, Zetta2 = 1e+42, Zetta3 = 1e+63;
static const double Exa   = 1e+18,   Exa2 = 1e+36,   Exa3 = 1e+54;
static const double Peta  = 1e+15,  Peta2 = 1e+30,  Peta3 = 1e+45;
static const double Tera  = 1e+12,  Tera2 = 1e+24,  Tera3 = 1e+36;
static const double Giga  = 1e+09,  Giga2 = 1e+18,  Giga3 = 1e+27;
static const double Mega  = 1e+06,  Mega2 = 1e+12,  Mega3 = 1e+18;
static const double Kilo  = 1e+03,  Kilo2 = 1e+06,  Kilo3 = 1e+09;
static const double Hekto = 1e+02, Hekto2 = 1e+04, Hekto3 = 1e+06;
// static const double Deka  = 1e+01,  Deka2 = 1e+02,  Deka3 = 1e+03; //unused
static const double deci  = 1e-01,  deci2 = 1e-02,  deci3 = 1e-03;
static const double centi = 1e-02, centi2 = 1e-04, centi3 = 1e-06;
static const double milli = 1e-03, milli2 = 1e-06, milli3 = 1e-09;
static const double micro = 1e-06, micro2 = 1e-12, micro3 = 1e-18;
static const double nano  = 1e-09,  nano2 = 1e-18,  nano3 = 1e-27;
static const double pico  = 1e-12,  pico2 = 1e-24,  pico3 = 1e-36;
static const double femto = 1e-15, femto2 = 1e-30, femto3 = 1e-45;
static const double atto  = 1e-18,  atto2 = 1e-36,  atto3 = 1e-54;
// static const double zepto = 1e-21, zepto2 = 1e-42, zepto3 = 1e-63; // unused
static const double zepto = 1e-21, zepto3 = 1e-63;
static const double yocto = 1e-24, yocto2 = 1e-48, yocto3 = 1e-72;

/****************************************************************************
*  Internal Functions                                                       *
****************************************************************************/
/*---------------------------------------------------------------------------
NAME

   num_newstr --- allocate memory and copy a character string into it

SYNOPSIS

   char * num_newstr( const char * string );

DESCRIPTION
  Allocates strlen(´string´)+1 bytes of memory and copies ´string´ into it.
  In case of success the pointer to the allocated memory is returned. The
  null pointer is returned in case of an error.
  If ´string´ is the NULL pointer the NULL pointer is returned.

RETURN VALUE
  Returns the pointer to the allocated string or (char *) NULL in case
  of an error.
---------------------------------------------------------------------------*/
char * num_newstr( const char * string )
{ char * newstring;

  if (!string) return( (char *) NULL );
  if (!(newstring = (char *) malloc(strlen(string)+1))) return((char *) NULL);
  (void) strcpy(newstring,string);

  return( newstring );

} /* num_newstr */

/****************************************************************************
*  Program Functions                                                        *
****************************************************************************/

/*---------------------------------------------------------------------------
NAME

  dpprogram_step --- Step through a single instruction 

SYNOPSIS

  int dpprogram_step ( NumProg * program, NumInstr * instruction,
                       int * perrval );

DESCRPTION

Executes a single instruction. It uses program->CurrentAccumulator as current
accumulator. In case of success this pointer is updated. 

At entry accumulator->Value must contain argumentN, 
accumulator->Previous->Value must contain argumentN-1 
and so on until argument1. 

The instruction is executed with these input arguments and the result is 
written to the accumulator that contained originally argument1. 

Instructions without input arguments (PUSHVAL, PUSHADDR, PI) write the 
output value to accumulator->Next->Value. If necessary, memory for this 
accumulator is allocated.

The values of the variables that are defined in program->VariableList
are used for calculation.

ERROR VALUES

   *perrval

   NumSuccess       : success

   program errors: (in this case the returned address is NULL)
   NumNoAccumulator : not enough accumulator cells available 
   NumNoInstruction : unknown instruction

   calculation errors:
   NumDivByZero     : division by zero
   NumDomainError   : some of the input arguments are outside

RETURN VALUE

  success: 0 in case of success or a calculation error (DivByZero, DomainError)
  error  :-1 in case of a program error (NumNoAccumulator, NumNoInstruction)

  In case of return value 0 the calculations can be repeated with changed
  variable values, in case of return value -1 the program is corrupted and 
  cannot be used.

---------------------------------------------------------------------------*/
int dpprogram_step ( NumProg * program, NumInstr * instruction,
                     int * perrval )
{
  int errval;

  const double pi = 3.1415926535897932384626;
  const double degtorad = pi/180.0;
  const double radtodeg = 180.0/pi;

  double argument1, argument2, argument3;
  NumAccu * accumulator;

  errval = NumSuccess;

  if (!program) {
    errval = NumProgramError;
    goto dpprogram_step_error;
  }

  if (!instruction) {
    errval = NumNoInstruction;
    goto dpprogram_step_error;
  }

  accumulator = program->CurrentAccumulator;

  switch (instruction->Command) {

    case PUSHVAL:

      accumulator = numprog_up_accumulator( program, instruction->Value );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      break;

    case PUSHADDR:

      accumulator = numprog_up_accumulator( program,*(instruction->Address) );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      break;

    case NEG:

      argument1 = accumulator->Value;
      accumulator->Value = -argument1;

      break;

    case MUL:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = argument1 * argument2;

      break;

    case NOT:

      argument1 = accumulator->Value;
      accumulator->Value = argument1?0.0:1.0;

      break;

    case EQU:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = (argument1 == argument2)?1.0:0.0;

      break;

    case NEQ:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = (argument1 != argument2)?1.0:0.0;

      break;

    case LE:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = (argument1 <= argument2)?1.0:0.0;

      break;

    case LT:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = (argument1 < argument2)?1.0:0.0;

      break;

    case GE:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = (argument1 >= argument2)?1.0:0.0;

      break;

    case GT:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = (argument1 > argument2)?1.0:0.0;

      break;

    case AND:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = (argument1 && argument2)?1.0:0.0;

      break;

    case OR:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = (argument1 || argument2)?1.0:0.0;

      break;

    case IF:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }
      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      argument3 = accumulator->Next->Next->Value;
      accumulator->Value = argument1 ? argument2 : argument3;

      break;

    case DIV:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      if ( argument2 != 0.0 ) {
        accumulator->Value = argument1 / argument2;
      } else errval = NumDivByZero;

      break;

    case REST:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      if ( argument2 != 0.0 ) {
        accumulator->Value =
          (double) ((long)floor(argument1+0.5)%(long)floor(argument2+0.5));
      } else errval = NumDivByZero;

      break;

    case ADD:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = argument1 + argument2;

      break;

    case SUB:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = argument1 - argument2;

      break;

    case RAD:

      argument1 = accumulator->Value;
      accumulator->Value = degtorad * argument1;
 
      break;

    case DEG:

      argument1 = accumulator->Value;
      accumulator->Value = radtodeg * argument1;

      break;

    case PI:

      accumulator = numprog_up_accumulator( program, pi );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      break;

    case SIN:

      argument1 = accumulator->Value;
      accumulator->Value = sin( argument1 );

      break;

    case COS:

      argument1 = accumulator->Value;
      accumulator->Value = cos( argument1 );

      break;

    case TAN:

      argument1 = accumulator->Value;
      accumulator->Value = tan( argument1 );

      break;

    case ASIN:

      argument1 = accumulator->Value;
      if (fabs(argument1)<=1.0) {
        accumulator->Value = asin( argument1 );
      } else errval=NumDomainError;

      break;

    case ACOS:

      argument1 = accumulator->Value;
      if (fabs(argument1)<=1.0) {
        accumulator->Value = acos( argument1 );
      } else errval=NumDomainError;

      break;

    case ATAN:

      argument1 = accumulator->Value;
      accumulator->Value = atan( argument1 );

      break;

    case ATAN2:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value = atan2( argument1 , argument2 );

      break;

    case SINH:

      argument1 = accumulator->Value;
      accumulator->Value = sinh( argument1 );

      break;

    case COSH:

      argument1 = accumulator->Value;
      accumulator->Value = cosh( argument1 );

      break;

    case TANH:

      argument1 = accumulator->Value;
      accumulator->Value = tanh( argument1 ); 

      break;

    case FLOOR:

      argument1 = accumulator->Value;
      accumulator->Value = floor( argument1 );

      break;

    case CEIL:

      argument1 = accumulator->Value;
      accumulator->Value = ceil( argument1 );

      break;

    case FABS:

      argument1 = accumulator->Value;
      accumulator->Value = fabs( argument1 );

      break;

    case EXP:

      argument1 = accumulator->Value;
      accumulator->Value = exp( argument1 );

      break;

    case LOG:

      argument1 = accumulator->Value;
      if (argument1>0.0) {
        accumulator->Value = log( argument1 );
      } else errval=NumDomainError;

      break;

    case LOG10:

      argument1 = accumulator->Value;
      if (argument1>0.0) {
        accumulator->Value = log10( argument1 );
      } else errval=NumDomainError; 

      break;

    case POW:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;

      if ( argument2 == 0.0 ) {
        // argument1 is 0 -> result is defined as 1.0 
        accumulator->Value = 1.0;
      } else {
        if ( argument1 > 0.0 ) {
          // positive argument1 -> solutions for all argument2
          if ( argument2 > 0.0 ) {
            accumulator->Value = pow( argument1 , argument2 );
          } else {
            accumulator->Value = 1.0/pow( argument1 , -argument2 );
          }
        } else {
          // argument1 is negative or zero
          if ( argument1 == 0.0 ) {
            // argument1 is zero
            if ( argument2 > 0.0 ) {
              accumulator->Value = 0.0;
            } else errval=NumDomainError; 
          } else {
            // argument1 is negative
            if ( (floor(argument2+0.5)-argument2)==0.0 ) {
              // only solutions for integer values of argument2
              if ( argument2 > 0.0 ) {
                accumulator->Value = pow( argument1 , argument2 );
              } else {
                accumulator->Value = 1.0/pow( argument1 , -argument2 );
              }
            } else errval=NumDomainError;
          }
        }
      }

      break;

    case SQRT:

      argument1 = accumulator->Value;
      if (argument1>=0.0) {
        accumulator->Value = sqrt( argument1 );
      } else errval=NumDomainError;

      break;

    case ROUND:

      argument1 = accumulator->Value;
      accumulator->Value = floor( argument1 + 0.5 );

      break;

    case GAMMA:

      argument1 = accumulator->Value;
      accumulator->Value = gamma( argument1 );

      break;

    case FMIN:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value =
        (argument1<argument2)?argument1:argument2;

      break;

    case FMAX:

      accumulator = numprog_down_accumulator( program );
      if (!accumulator) {
        errval = NumNoAccumulator;
        goto dpprogram_step_error;
      }

      argument1 = accumulator->Value;
      argument2 = accumulator->Next->Value;
      accumulator->Value =
        (argument1>argument2)?argument1:argument2;

      break;

    case DEGC2K:

      argument1 = accumulator->Value;
      accumulator->Value = argument1 + 273.15;

      break;

    case K2DEGC:

      argument1 = accumulator->Value;
      accumulator->Value = argument1 - 273.15;

      break;

    case DEGF2K:

      argument1 = accumulator->Value;
      accumulator->Value = (5.0/9.0) * (argument1 - 32.0) + 273.15;

      break;

    case K2DEGF:

      argument1 = accumulator->Value;
      accumulator->Value = ((argument1 - 273.15) * (9.0/5.0)) + 32.0;

      break;

    case DEGK2K:

      argument1 = accumulator->Value;
      accumulator->Value = argument1;

      break;

    case K2DEGK:

      argument1 = accumulator->Value;
      accumulator->Value = argument1;

      break;

    case DEGF2DEGC:

      argument1 = accumulator->Value;
      accumulator->Value = (5.0/9.0) * (argument1 - 32.0);

      break;

    case DEGC2DEGF:

      argument1 = accumulator->Value;
      accumulator->Value = (argument1 * (9.0/5.0)) + 32.0;

      break;

    default: 
      errval = NumNoInstruction; 
      goto dpprogram_step_error;

  } /* cmd */

  if (perrval) *perrval=errval;
  return( 0 );

dpprogram_step_error:

  if (perrval) *perrval=errval;
  return( -1 );

} /* dpprogram_step */

/*---------------------------------------------------------------------------
NAME

  dpprogram_run --- Executes all instructions of the program

SYNOPSIS

  int dpprogram_run ( NumProg * program, int * perrval );

DESCRPTION

Executes all instructions starting with the first instruction of
InstructionList and with the first accumulator of AccumulatorList. 
At entry, the pointer program->CurrentAccumulator is reset to NULL.
The program is executed by subsequent calls to dpprogram_step. In 
case of success, program->CurrentAccumulator->Value points to the result.

ERROR VALUES

   *perrval

   NumSuccess       : success

   program errors   : (in this case the returned value is -1)
   NumNoAccumulator : not enough accumulator cells available
   NumNoInstruction : unknown instruction

   calculation errors:
   NumDivByZero     : division by zero
   NumDomainError   : some of the input arguments are outside

RETURN VALUE

  success: 0 in case of success or a calculation error (DivByZero, DomainError)
  error  :-1 in case of a program error (NumNoAccumulator, NumNoInstruction)

  In case of return value 0 the calculations can be repeated with changed
  variable values, in case of return value -1 the program is corrupted and 
  cannot be used.

---------------------------------------------------------------------------*/
PUBLIC int dpprogram_run ( NumProg * program, int * perrval )
{ 
  int status=-1;

  NumInstr * instruction;

  if (!program) { 
    *perrval = NumProgramError; return( status );
  }

  instruction = program->InstructionList;

  if (!instruction) {
    *perrval = NumNoInstruction; return( status );
  }

  program->CurrentAccumulator = ( NumAccu * ) NULL;

  while ( instruction ) {

    status = dpprogram_step( program, instruction, perrval ); 
    if ( *perrval ) return( status );

    instruction = instruction->Next;

  }

  return( status );

} /* dpprogram_run */

/*---------------------------------------------------------------------------
NAME

  dpprogram_optimize --- contract instructions where possible 

SYNOPSIS

  NumInstr * dpprogram_optimize ( NumProg * program, 
                                  NumInstr *  firstinstruction, 
                                  int * perrval );

DESCRIPTION

The routine executes the input instruction list and uses
program->CurrentAccumulator as first accumulator. The instruction list
is executed either until the command PUSHADDR is given or until 
program->CurrentAccumulator->Number reaches the same number that it
had at entry.

If PUSHADDR is found, the values of all accumulators between first
accumulator->Next and program->CurrentAccumulator are copied with PUSHVAL
to program->CompiledList.

In the second case, the values of all accumulators between first 
accumulator->Next and program->CurrentAccumulator are copied with PUSHVAL 
to program->CompiledList. 

If existing, the last instruction that would 
use or modify the value of the first accumulator is appended to 
program->CompiledList, because it cannot be executed immediately.

The last copied instruction is returned.

ERROR VALUES

   *perrval

   NumSuccess       : success

   program errors   : (in this case the returned value is NULL)
   NumNoAccumulator : not enough accumulator cells available
   NumNoInstruction : unknown instruction

   calculation errors (only, if no variables are involved):
   NumDivByZero     : division by zero
   NumDomainError   : some of the input arguments are outside

RETURN VALUE

   pointer to the last used instruction or NULL if end of list is reached
   NULL, together with an error value,  is also returned in case of a 
   program error.

---------------------------------------------------------------------------*/
NumInstr * dpprogram_optimize ( NumProg * program,
                                NumInstr *  firstinstruction,
                                int * perrval )
{ 
  NumInstr *instruction, *lastinstruction;
  NumAccu *accumulator, *firstaccumulator, *lastaccumulator;
  int accunum, firstaccunum, lastaccunum;

  if (!program) {
    *perrval = NumProgramError; return( (NumInstr *) NULL ); 
  }

  if (!firstinstruction) {
    *perrval = NumNoInstruction; return( (NumInstr *) NULL );
  }

  firstaccumulator = program->CurrentAccumulator;
  if ( firstaccumulator ) firstaccunum = firstaccumulator->Number; 
  else firstaccunum = 0; 
  
  instruction = firstinstruction;
  accumulator = firstaccumulator;
  accunum = firstaccunum;
  while ( ( instruction ) &&
          ( instruction->Command != PUSHADDR ) && 
          ( accunum+1-instruction->Nargs > firstaccunum ) ) {

    dpprogram_step( program, instruction, perrval );
    if ( *perrval ) return( instruction );

    accumulator = program->CurrentAccumulator;
    accunum = accumulator->Number;

    instruction = instruction->Next;

  }
  lastinstruction = instruction;
  lastaccumulator = accumulator;
  lastaccunum = accunum;

  /* copy (lastaccunum-firstaccunum) accumulator values starting at
     firstaccumulator->Next to lastaccumulator (inclusive) */

  if ( firstaccumulator ) accumulator=firstaccumulator->Next;
  else accumulator = program->AccumulatorList;

  for (accunum=firstaccunum+1;accunum<=lastaccunum;accunum++) {
    /* write accumulator values with PUSHVAL into CompiledList */
    if (numprog_append_instruction ( program, 1, PUSHVAL, 0, 
                                   accumulator->Value, NULL, 
                                   NULL ) ) return( (NumInstr *) NULL );
    accumulator=accumulator->Next;
  }

  if ( lastinstruction ) {
    /* copy last instruction */

    instruction=lastinstruction;
    if (numprog_append_instruction ( program, 1,
                             instruction->Command, instruction->Nargs,
                             instruction->Value, instruction->Address,
                             NULL ) ) return( (NumInstr *) NULL );
  }

  if ( lastinstruction ) {
    /* the last instruction was not executed */
    if (lastinstruction->Nargs==0) {
        numprog_up_accumulator ( program, *(lastinstruction->Address) );
    } else {
      for (accunum=lastaccunum;accunum>lastaccunum+1-lastinstruction->Nargs;
           accunum--)
        numprog_down_accumulator ( program );
    }
  }

  return( lastinstruction );

} /* dpprogram_optimize */

/*---------------------------------------------------------------------------
NAME
 
  dpprogram_compile --- Optimizes the program
 
SYNOPSIS
 
  int dpprogram_compile ( NumProg * program, int * perrval );
 
DESCRIPTION
 
The program is optimized by calculating parts of the program that do not
contain variables.
 
ERROR VALUES
 
   *perrval
 
   NumSuccess       : success
 
   program errors   : (in this case the returned value is -1)
   NumNoAccumulator : not enough accumulator cells available
   NumNoInstruction : unknown instruction
 
   calculation errors:
   NumDivByZero     : division by zero
   NumDomainError   : some of the input arguments are outside
 
RETURN VALUE
 
  success: 0 in case of success or a calculation error (DivByZero, DomainError)
  error  :-1 in case of a program error (NumNoAccumulator, NumNoInstruction)
 
  In case of return value 0 the calculation has caused an error, e.g.
  DivByZero, DomainError. If the program is run it will also return a
  calculation error. In case of return value -1 the program is corrupted and
  cannot be used.
 
---------------------------------------------------------------------------*/
PUBLIC int dpprogram_compile ( NumProg * program, int * perrval )
{
  NumInstr * instruction;

  if (!program) {
    *perrval = NumProgramError; return( -1 );
  }
 
  instruction = program->InstructionList;
 
  if (!instruction) {
    *perrval = NumNoInstruction; return( -1 );
  }
 
  program->CurrentAccumulator = ( NumAccu * ) NULL;
 
  while ( instruction ) {
 
    instruction = dpprogram_optimize( program, instruction, perrval );
    if ( *perrval )  return( instruction?0:-1 );
 
    if (instruction ) instruction = instruction->Next;
 
  }
 
  /* Replace InstructionList by CompiledList */
  if ( numprog_free_instruction_list  ( program, 0 ) ) {
    *perrval = NumProgramError; return( -1 );
  }
  program->InstructionList = program->CompiledList;
  program->CompiledList = ( NumInstr * ) NULL;
 
 
  return( 0 );
 
} /* dpprogram_compile */

/****************************************************************************
*  Number Functions                                                         *
****************************************************************************/
# define EXPRESSION dpcondition
void   dpcondition(    NumProg * program, const char **ps,
                       int level, int * perrval);
void   dplogicsum(     NumProg * program, const char **ps,
                       int level, int * perrval);
void   dplogicproduct( NumProg * program, const char **ps,
                       int level, int * perrval);
void   dpequality(     NumProg * program, const char **ps, 
                       int level, int * perrval);
void   dpcomparison(   NumProg * program, const char **ps, 
                       int level, int * perrval);
void   dpexpression(   NumProg * program, const char **ps, 
                       int level, int *perrval);
void   dpterm(         NumProg * program, const char **ps, 
                       int level, int *perrval);
void   dpfactor0(      NumProg * program, const char **ps, 
                       int level, int *perrval);
void   dpfactor1(      NumProg * program, const char **ps, 
                       int level, int *perrval);
void   dpfactor2(      NumProg * program, const char **ps, 
                       int level, int *perrval);
void   dpconstant(     NumProg * program, const char **ps, 
                       int *perrval);
void   dpvariable(     NumProg * program, const char **ps,
                       int *perrval);
void   dpfunction(     NumProg * program, const char **ps, 
                       int level, int *perrval);
long int lvexpression( const char **ps, int level, int *perrval);
long int lvfactor( const char **ps, int level, int *perrval);
long int lvterm( const char **ps, int level, int *perrval);

int isfunction( const char * s )
/* A function name starts with a character and contains characters and
   numbers. It ends with a parenthesis '('. This function returns 1
   if the string s starts with a function name */
{
   if (('0'<=*s) && (*s<'9')) return ( 0 ); /* no function */
   while ( (('0'<=*s) && (*s<'9')) || (('a'<=*s) && (*s<'z')) ||
           (('A'<=*s) && (*s<'Z')) ) s++;
   if (*s=='(') return ( 1 ); else return ( 0 );
} /* isfunction */

int isvariable( const char * s )
/* In an expression, a variable name is preceded by an underscore. This 
   function returns 1 if the string s starts with a variable name */
{
   if ('_'!=*s) return ( 0 ); /* no variable */
   return ( 1 );
} /* isvariable */

void print_spaces( FILE * out, int n )
{ for (n=n;n>0;n--) fprintf(out," ");
} /* print_spaces */

/***************************************************************************
* Double Constant Functions                                                *
***************************************************************************/

int dpconstant_insert         ( const char * Quantity, const char * Unit,
                                const char * Key, double value,
                                DPConstant ** pdpconstant );
int dpconstant_search         ( const char * Key, DPConstant ** pdpconstant,
                                int mode );
int dpconstant_free           ( void );
int dpconstant_print          ( FILE * out, int level, int verbose );
int dpconstant_init           ( void );

/*---------------------------------------------------------------------------
NAME

  dpconstant_insert --- Define Value of Key

SYNOPSIS

  int dpconstant_insert ( const char * Quantity, const char * Unit,
                          const char * Key, double Value , 
                          DPConstant ** pdpconstant )

DESCRPTION

  Insert or update the constant Key with Value, return pointer to dpconstant.
  If 'Key' already exists, its 'Value', 'Quantity' and 'Unit' are updated, 
  otherwise 'Key' is created.
  In case of success the pointer to the new dpconstant is returned, otherwise 
  NULL.
  'Quantity' and 'Unit' are optional strings. They are only used if 
  NUMIO_debug is > 0. If 'Quantity' or 'Unit' is NULL the corresponding value
  is ignored.

  The key list is inversely lexicographically ordered, i.e. longer keys are
  preceeding shorter keys (nnn->nn->n->mmm->mm>m).

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int dpconstant_insert ( const char * Quantity, const char * Unit,
                        const char * Key, double Value , 
                        DPConstant ** pdpconstant )
{
  DPConstant * newdpconstant, * next, * previous;
  int notfound = 1;

  if (pdpconstant) *pdpconstant = (DPConstant *) NULL;
  previous = (DPConstant *) NULL;
  next = DPConstantRoot;

  /* search insertion point (insertion before next) */
  while ( ( next!=(DPConstant *) NULL ) && (notfound>0) ) {
    notfound = strcmp(next->Key,Key);
    if (notfound>0) {previous = next; next = next->Next;}
    }

  /* create new constant Key, if notfound */
  if ( notfound ) {
    /* create new constant Key */
    if (!(newdpconstant = (DPConstant*) malloc(sizeof(DPConstant)))) return(-1);

    newdpconstant->Key         = num_newstr( Key ); 
    if (!newdpconstant->Key) return(-1);

    newdpconstant->Quantity    = (char *) NULL;
    newdpconstant->Unit        = (char *) NULL;

    /* insert newdpconstant before next */
    if (next) next->Previous = newdpconstant;
    newdpconstant->Next=next;
    newdpconstant->Previous=previous;
    if (previous) previous->Next=newdpconstant;
       else DPConstantRoot = newdpconstant;

    next = newdpconstant;
    }

  /* update Value */
  next->Value  = Value;

  /* update Quantity and Unit for debugging */
  if (NUMIO_debug > 0) {
    if (next->Quantity) free( next->Quantity );
    next->Quantity = num_newstr( Quantity ); 
    /* Ignore error if newstr returns NULL, because Quantity can be NULL */
    if (next->Unit) free( next->Unit );
    next->Unit = num_newstr( Unit );
    /* Ignore error if newstr returns NULL, because Unit can be NULL */
  }

  if (pdpconstant) *pdpconstant = next;

  return(0);

} /* dpconstant_insert */

/*---------------------------------------------------------------------------
NAME

  dpconstant_search --- search Key

SYNOPSIS
  
  int dpconstant_search  ( const char * Key, DPConstant ** pdpconstant )

DESCRPTION

  Search the Key. In case of success the pointer to dpconstant is returned, 
  otherwise NULL. 
  If mode==0, all characters of Key are compared, 
  if mode==1, the comparison stops if a dpconstant is found where all 
              characters match the first characters of Key. 
  Key is searched from the beginning of the list which is inversely 
  lexicographically ordered.

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int dpconstant_search  ( const char * Key, DPConstant ** pdpconstant, int mode )
{
  DPConstant * current;

  /* return NULL in case that Key was not found */
  if (pdpconstant) *pdpconstant = (DPConstant *) NULL;

  /* search Key */
  current = DPConstantRoot;

  if ( current!=(DPConstant *) NULL ) {
    if (mode==0) {
      while( ( current!=(DPConstant *) NULL ) &&
             ( strcmp(current->Key,Key)!=0) ) {
        current = current->Next;
      }
    } else {
      while( ( current!=(DPConstant *) NULL ) &&
             ( strncmp(current->Key,Key,strlen(current->Key))!=0) ) {
        current = current->Next;
      }
    }
  }
  if (pdpconstant) *pdpconstant = current;

  if (current==(DPConstant *) NULL) return(-1);

  if (NUMIO_debug > 0) {
    if (current->Unit) {
      if (current->Quantity) {
        printf("  %s = %.15g %s (%s)\n",
          current->Key,current->Value,current->Unit,current->Quantity);
      } else {
        printf("  %s = %.15g %s\n",current->Key,current->Value,current->Unit);
      }
    } else {
      if (current->Quantity) {
        printf("  %s = %.15g (%s)\n",
          current->Key,current->Value,current->Quantity);
      } else {
        printf("  %s = %.15g\n",current->Key,current->Value);
      }
    }
  }

  return(0);

} /* dpconstant_search */

/*+++------------------------------------------------------------------------
NAME

   dpconstant_free --- free list of constants 

SYNOPSIS

   int dpconstant_free( void )

DESCRIPTION

   Releases all Keys

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int dpconstant_free( void )
{
  DPConstant * current, * next;

  next = DPConstantRoot;

  while(next!=(DPConstant*) NULL) {
    current = next;
    next=next->Next;
    if (current->Quantity) free(current->Quantity);
    if (current->Unit) free(current->Unit);
    free(current->Key);
    free(current);
    }

  DPConstantRoot = (DPConstant *) NULL;

  return(0);

} /* dpconstant_free */

/*+++------------------------------------------------------------------------
NAME

   dpconstant_print --- print all constants

SYNOPSIS

   int dpconstant_print( FILE * out, int level, int verbose )


DESCRIPTION

   Prints all constants to the file ´out´

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int dpconstant_print( FILE * out, int level, int verbose )
{ const char * SeparationLine =
    "-       -       -       -       -       -       -       -";

  DPConstant * current;

  if (level<1) return(0);

  current = DPConstantRoot;

  while (current!=(DPConstant*) NULL) {
    if (verbose) {
      fprintf(out,"   %s\n",SeparationLine);
      fprintf(out,"   Key               = %s\n",current->Key);
      fprintf(out,"   Value             = %lg\n",current->Value);
      if (current->Quantity)
        fprintf(out,"   Quantity       = %s\n",current->Quantity);
      if (current->Unit)
        fprintf(out,"   Unit           = %s\n",current->Unit);
      fprintf(out,"   Previous Key      = ");
      if ((current->Previous)!=(DPConstant*) NULL)
        fprintf(out,"%s\n", current->Previous->Key);
        else fprintf(out,"(no previous dpconstant)\n");
      fprintf(out,"   Next Key          = ");
      if ((current->Next)!=(DPConstant*) NULL)
        fprintf(out,"%s\n", current->Next->Key);
        else fprintf(out,"(no next dpconstant)\n");
    } else {
      if (current->Quantity)
        if (current->Unit)
          fprintf(out,"   '%s' = %lg %s (%s)\n",
            current->Key,current->Value,current->Unit, current->Quantity);
        else
          fprintf(out,"   '%s' = %lg (%s)\n",
            current->Key,current->Value,current->Quantity);
      else
        if (current->Unit)
          fprintf(out,"   '%s' = %lg %s\n",
            current->Key,current->Value,current->Unit);
        else
          fprintf(out,"   '%s' = %lg\n",current->Key,current->Value);
    }
    current=current->Next;
  }

  if (verbose) fprintf(out,"   %s\n",SeparationLine);

  return(0);

} /* dpconstant_print */

/*---------------------------------------------------------------------------
NAME
    
  dpconstant_insert_unit --- Insert full range of unit from atto to Exa 
    
SYNOPSIS
  
  int dpconstant_insert_unit ( const char * Quantity, const char * Unit,
                               const char * Baseunit, double Basevalue );
    
DESCRPTION

  Baseunit preceeded by prefixes from atto to Exa is inserted into the list.

RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int dpconstant_insert_unit ( const char * Quantity, const char * Unit,
                             const char * Baseunit, double Basevalue )
{
  char unit[128];
  DPConstant * element;

  if (strlen(Baseunit)>64) return(-1);

  /* Yotta */
  if (           sprintf(     unit, "Y%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Yotta*Basevalue, &element )) return(-1);
  /* Zetta */
  if (           sprintf(     unit, "Z%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Zetta*Basevalue, &element )) return(-1);
  /* Exa */
  if (           sprintf(     unit, "E%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Exa*Basevalue, &element ))  return(-1);
  /* Peta */
  if (           sprintf(     unit, "P%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Peta*Basevalue, &element )) return(-1);
  /* Tera */
  if (           sprintf(     unit, "T%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Tera*Basevalue, &element )) return(-1);
  /* Giga */
  if (           sprintf(     unit, "G%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Giga*Basevalue, &element )) return(-1);
  /* Mega */
  if (           sprintf(     unit, "M%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Mega*Basevalue, &element )) return(-1);
  /* Kilo */
  if (           sprintf(     unit, "k%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Kilo*Basevalue, &element )) return(-1);
  /* Hekto */
  if (           sprintf(     unit, "h%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Hekto*Basevalue, &element )) return(-1);
  /* Base unit */
  if (dpconstant_insert ( Quantity, Unit,
                          Baseunit,       Basevalue, &element )) return(-1);
  /* deci */
  if (           sprintf(     unit, "d%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  deci*Basevalue, &element )) return(-1);
  /* centi */
  if (           sprintf(     unit, "c%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                               unit, centi*Basevalue, &element )) return(-1);
  /* milli */
  if (           sprintf(     unit, "m%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, milli*Basevalue, &element )) return(-1);
  /* micro */
  if (           sprintf(     unit, "u%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, micro*Basevalue, &element )) return(-1);
  /* nano */
  if (           sprintf(     unit, "n%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  nano*Basevalue, &element )) return(-1);
  /* pico */
  if (           sprintf(     unit, "p%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  pico*Basevalue, &element )) return(-1);
  /* femto */
  if (           sprintf(     unit, "f%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, femto*Basevalue, &element )) return(-1);
  /* atto */
  if (           sprintf(     unit, "a%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  atto*Basevalue, &element )) return(-1);
  /* zepto */
  if (           sprintf(     unit, "z%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, zepto*Basevalue, &element )) return(-1);
  /* yocto */
  if (           sprintf(     unit, "y%s", Baseunit) < 1 )       return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, yocto*Basevalue, &element )) return(-1);

  return(0);

} /* dpconstant_insert_unit */

/*---------------------------------------------------------------------------
NAME 
     
  dpconstant_insert_unit2 --- Insert full range of unit for square 
     
SYNOPSIS
  
  int dpconstant_insert_unit2 ( const char * Quantity, const char * Unit,
                                const char * Baseunit, double Basevalue );
     
DESCRPTION
  
  The square of the Baseunit preceeded by prefixes from atto to Exa and 
  followed by 2 is inserted into the list, e.g. km2 = Kilo*Kilo*m*m

  Baseunit and Basevalue are not squared!

  Unit is the unit symbol of Quantity, e.g. Unit="m^2", Quantity="area"
  
RETURN VALUE
  
  success:0, error:-1
  
---------------------------------------------------------------------------*/
int dpconstant_insert_unit2 ( const char * Quantity, const char * Unit,
                              const char * Baseunit, double Basevalue )
{
  double Basevalue2;
  char unit[128];
  DPConstant * element;

  if (strlen(Baseunit)>63) return(-1);

  Basevalue2 = Basevalue*Basevalue;

  /* Yotta2 */
  if (           sprintf(     unit, "Y%s2",Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Yotta2*Basevalue2, &element )) return(-1);
  /* Zetta2 */
  if (           sprintf(     unit, "Z%s2",Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Zetta2*Basevalue2, &element )) return(-1);
  /* Exa2 */
  if (           sprintf(     unit, "E%s2",Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Exa2*Basevalue2, &element ))  return(-1);
  /* Peta2 */
  if (           sprintf(     unit, "P%s2",Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Peta2*Basevalue2, &element )) return(-1);
  /* Tera2 */
  if (           sprintf(     unit, "T%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Tera2*Basevalue2, &element )) return(-1);
  /* Giga2 */
  if (           sprintf(     unit, "G%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Giga2*Basevalue2, &element )) return(-1);
  /* Mega2 */
  if (           sprintf(     unit, "M%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Mega2*Basevalue2, &element )) return(-1);
  /* Kilo2 */
  if (           sprintf(     unit, "k%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Kilo2*Basevalue2, &element )) return(-1);
  /* Hekto2 */
  if (           sprintf(     unit, "h%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Hekto2*Basevalue2, &element )) return(-1);
  /* Base unit ^2 */
  if (           sprintf(     unit, "%s2", Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,       Basevalue2, &element ))  return(-1);
  /* deci2 */
  if (           sprintf(     unit, "d%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  deci2*Basevalue2, &element )) return(-1);
  /* centi2 */
  if (           sprintf(     unit, "c%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, centi2*Basevalue2, &element )) return(-1);
  /* milli2 */
  if (           sprintf(     unit, "m%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, milli2*Basevalue2, &element )) return(-1);
  /* micro2 */
  if (           sprintf(     unit, "u%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, micro2*Basevalue2, &element )) return(-1);
  /* nano2 */
  if (           sprintf(     unit, "n%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  nano2*Basevalue2, &element )) return(-1);
  /* pico2 */
  if (           sprintf(     unit, "p%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  pico2*Basevalue2, &element )) return(-1);
  /* femto2 */
  if (           sprintf(     unit, "f%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, femto2*Basevalue2, &element )) return(-1);
  /* atto2 */
  if (           sprintf(     unit, "a%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  atto2*Basevalue2, &element )) return(-1);
  /* yocto2 */
  if (           sprintf(     unit, "y%s2", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, yocto2*Basevalue2, &element )) return(-1);

  return(0);

} /* dpconstant_insert_unit2 */

/*---------------------------------------------------------------------------
NAME

  dpconstant_insert_unit3 --- Insert full range of unit for unit^3 

SYNOPSIS

  int dpconstant_insert_unit3 ( const char * Quantity, const char * Unit,
                                const char * Baseunit, double Basevalue );

DESCRPTION

  The cube of the Baseunit preceeded by prefixes from atto to Exa and
  followed by 3 is inserted into the list, e.g. km2 = Kilo*Kilo*m*m

  Baseunit and Basevalue are the linear values, they are not cubed!

  Unit is the unit symbol of Quantity, e.g. Unit="m^3", Quantity="volume"


RETURN VALUE

  success:0, error:-1

---------------------------------------------------------------------------*/
int dpconstant_insert_unit3 ( const char * Quantity, const char * Unit,
                              const char * Baseunit, double Basevalue )
{
  double Basevalue3;
  char unit[128];
  DPConstant * element;

  if (strlen(Baseunit)>63) return(-1);

  Basevalue3 = Basevalue*Basevalue;

  /* Yotta3 */
  if (           sprintf(     unit, "Y%s3",Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Yotta3*Basevalue3, &element )) return(-1);
  /* Zetta3 */
  if (           sprintf(     unit, "Z%s3",Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Zetta3*Basevalue3, &element )) return(-1);
  /* Exa3 */
  if (           sprintf(     unit, "E%s3",Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Exa3*Basevalue3, &element ))  return(-1);
  /* Peta3 */
  if (           sprintf(     unit, "P%s3",Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Peta3*Basevalue3, &element )) return(-1);
  /* Tera3 */
  if (           sprintf(     unit, "T%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Tera3*Basevalue3, &element )) return(-1);
  /* Giga3 */
  if (           sprintf(     unit, "G%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Giga3*Basevalue3, &element )) return(-1);
  /* Mega3 */
  if (           sprintf(     unit, "M%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Mega3*Basevalue3, &element )) return(-1);
  /* Kilo3 */
  if (           sprintf(     unit, "k%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  Kilo3*Basevalue3, &element )) return(-1);
  /* Hekto3 */
  if (           sprintf(     unit, "h%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, Hekto3*Basevalue3, &element )) return(-1);
  /* Base unit ^3 */
  if (           sprintf(     unit, "%s3", Baseunit) < 1 )         return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,       Basevalue3, &element ))  return(-1);
  /* deci3 */
  if (           sprintf(     unit, "d%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  deci3*Basevalue3, &element )) return(-1);
  /* centi3 */
  if (           sprintf(     unit, "c%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, centi3*Basevalue3, &element )) return(-1);
  /* milli3 */
  if (           sprintf(     unit, "m%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, milli3*Basevalue3, &element )) return(-1);
  /* micro3 */
  if (           sprintf(     unit, "u%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, micro3*Basevalue3, &element )) return(-1);
  /* nano3 */
  if (           sprintf(     unit, "n%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  nano3*Basevalue3, &element )) return(-1);
  /* pico3 */
  if (           sprintf(     unit, "p%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  pico3*Basevalue3, &element )) return(-1);
  /* femto3 */
  if (           sprintf(     unit, "f%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, femto3*Basevalue3, &element )) return(-1);
  /* atto3 */
  if (           sprintf(     unit, "a%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit,  atto3*Basevalue3, &element )) return(-1);
  /* zepto3 */
  if (           sprintf(     unit, "z%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, zepto3*Basevalue3, &element )) return(-1);
  /* yocto3 */
  if (           sprintf(     unit, "y%s3", Baseunit) < 1 )        return(-1);
  if (dpconstant_insert ( Quantity, Unit,
                              unit, yocto3*Basevalue3, &element )) return(-1);

  return(0);

} /* dpconstant_insert_unit3 */

/*+++------------------------------------------------------------------------
NAME

   dpconstant_init --- Inits constants

SYNOPSIS

   int dpconstant_init( void )


DESCRIPTION

   Inits the constants list

RETURN VALUE

   success:0, error:-1

---------------------------------------------------------------------------*/
int dpconstant_init ( void )
/* Physical and mathematical constants are taken from: 
  [86] Lawrence Berkeley Laboratory
       University of California
       Berkeley, California 94720
       X-Ray data booklet, second printing, with corrections, April 1986 
  Constants that are not contained in this edition are taken from
  [91] Kuchling, Taschenbuch der Physik, Verlag Harri Deutsch 1991
  There are inconsistencies between in some definitions, e.g. amu, ga and p.
  86+ and 91+ means that the values were calculated from values in [86] or [91].
  If possible, all physical constants replaced by values from
  [2002] Peter J. Mohr and Barry N. Taylor, 
         CODATA Recommended Values of the Fundamental Physical Constants: 2002
         taken from http://physics.nist.gov/constants
  All units are preferably taken from
  [2004] PTB Die gesetzlichen Einheiten in Deutschland
         (download from http://www.ptb.de/  20/0104)
  Inconsistency for amu_ in 2002 and 2004
*/
{
  const double pi      = 3.1415926535897932384626; /* pi number    86 */
  const double gamma   = 0.577215664901532861;     /* gamma number 86 */
  const double e       = 2.718281828459045235;     /* Euler number 86 */
  const double rad_    = 1.0;                      /* angle (rad)  */
  const double deg_    = pi/180.0;                 /* angle (rad)  */
  const double gon_    = pi/200.0;                 /* angle (rad)  */
  const double arcmin_ = pi/180.0/60.0;            /* angle (rad)  */
  const double arcsec_ = pi/180.0/3600.0;          /* angle (rad)  */

  const double inf     = DBL_MAX;                  /* "infinity" (workaround) */

//  const double k     = 1.380662e-23;    /* Boltzmann constant (J/K) 86 */
  const double k     = 1.3806505e-23;    /* Boltzmann constant (J/K) 2002 */
//  const double me    = 9.109534e-31;    /* electron rest mass (kg) 86 */
  const double me    = 9.1093826e-31;    /* electron rest mass (kg) 2002 */
//  const double mp    = 1.6726485e-27;   /* proton rest mass (kg) 86 */
  const double mp    = 1.67262171e-27;   /* proton rest mass (kg) 2002 */
//  const double md    = 3.3436369e-27;   /* deuteron rest mass (kg) 86+ */
  const double md    = 3.34358335e-27;    /* deuteron rest mass (kg) 2002 */
  const double mn    = 1.67492728e-27;    /* neutron rest mass (kg) 2002 */
//const double NA    = 6.022045e23;     /* Avogadro number (1/mol) 86 */
//  const double NA    = 6.0221367e23;    /* Avogadro number (1/mol) 91 */
  const double NA    = 6.0221415e23;    /* Avogadro number (1/mol) 2002 */
//  const double re    = 2.8179380e-15;   /* classical electron radius (m) 86 */
  const double re    = 2.817940325e-15; /* classical electron radius (m) 2002 */
  const double c     = 2.99792458e8;    /* velocity of light (m/s) 2002 */
//  const double ec    = 1.6021892e-19;   /* electron charge magnitude (C) 86 */
  const double ec    = 1.60217653e-19;  /* elementary charge (C) 2002 */
//  const double h     = 6.626176e-34;    /* Planck's number (J*s) 86 */
  const double h     = 6.6260693e-34;     /* Planck constant (J*s) 2002 */
//  const double gN    = 6.6720e-11;  /* gravitational constant (m3/kg/s2) 86 */
  const double gN    = 6.6742e-11;       /* Newtonian constant of gravitation 
                                           (m3/kg/s2) 2002 */
//const double ga    = 9.8062;          /* gravitational acceleration at sea
//                                         level at 45 deg latitude (m/s2) 86 */
  const double ga    = 9.80665;    /* gravitational acceleration (m/s2) 2004 */
  const double u0    = 4e-7*pi;           /* permeability of vacuum (H/m) 86 */
  const double e0    = 1.0/(u0*c*c);      /* permittivity of vacuum (F/m) 86 */

  const double m_     = 1.0;              /* length (m) meter */
  const double in_    = 2.54e-2;          /* length (m) inch 86 2004 */
  const double ft_    = 0.3048;           /* length (m) foot 91 2004 */
  const double yd_    = 0.9144;           /* length (m) yard 91 2004 */
  const double mile_  = 1609.344;         /* length (m) mile 91 2004 */
  const double sm_    = 1852;             /* length (m) nautical mile 91 2004 */

  const double barn_  = 1e-28;            /* area (m2) barn 86 2004 */
  const double a_     = 1e2;              /* area (m2) Ar 91 2004 */

  const double sec_   = 1.0;              /* time (s) second */
  const double min_   = 60.0*sec_;        /* time (s) minute */
  const double hr_    = 60.0*min_;        /* time (s) hour */
  const double d_     = 24.0*hr_;         /* time (s) day */

  const double Hz_    = 1.0;              /* frequency (1/s) Herz */

  const double kn_    = sm_/3600.0/sec_;  /* speed (m/s) knots 91 2004 */

  const double l_     = 1e-3;             /* volume (m3) liter */

  const double g_     = 1e-3;             /* mass (kg) gram */
//  const double amu_   = 1.6605402e-27;    /* mass (kg) atomic mass unit 91 */
//  const double amu_   = 1.6605655e-27;    /* mass (kg) atomic mass unit 86 2004 */
  const double amu_   = 1.66053886e-27;   /* 0.00000028e-27 
                                             mass (kg) atomic mass constant 2002*/
  const double lb_    = 0.45359237;       /* mass (kg) pound 91 2004 */
//  const double oz_    = 0.02834952;       /* mass (kg) ounze 91 */
  const double oz_    = 0.0283495;        /* mass (kg) ounze (avoirdupois) 2004 */

  const double J_     = 1.0;            /* energy (J) Joule */
//  const double cal_   = 4.184;          /* energy (J) calorie 86 */
  const double cal_   = 4.1868;           /* energy (J) calorie 2004 */
  const double erg_   = 1e-7;             /* energy (J) erg 91 2004 */

  const double W_     = 1.0;              /* power (W) Watt */

  const double K_     = 1.0;              /* temperature (K) Kelvin */

  const double degK_  = K_;
  const double degC_  = K_;
  const double degF_  = (5.0/9.0)*K_;

  const double N_     = 1.0;              /* force (N) Newton */
  const double p_     = 9.80665e-3;       /* force (N) pond 91 2004 */
  const double dyn_   = 1e-5;             /* force (N) dyn 91 2004 */
//const double lbf_   = 4.44822;          /* force (N) pound force 91 */
  const double lbf_   = lb_*ga;           /* force (N) pound force */
  const double pdl_   = 0.138255;         /* force (N) poundal 91 */

  const double Pa_    = 1.0;              /* pressure (Pa) Pascal */
  const double bar_   = 1e5;              /* pressure (Pa) bar 91 2004 */
  const double Torr_  = 133.3224;         /* pressure (Pa) Torr 91 2004 */
  const double atm_   = 1.01325e5;        /* pressure (Pa) 
                                             physical atmosphere 2004 */
  const double at_    = 0.980665e5;       /* pressure (Pa) 
                                             technical atmosphere 2004 */
// const double psi_   = 6894.76;         /* pressure (Pa) lbf per square in 91 */
  const double psi_   = lbf_/in_/in_;     /* pressure (Pa) lbf per square in */

  const double V_     = 1.0;              /* voltage (V) Volt */

  const double A_     = 1.0;              /* electric current (A) Ampere */

  const double C_     = 1.0;              /* electric charge (C) Coulomb */

  const double F_     = 1.0;              /* electric capacity (F=A*s/V) Farad */

  const double Ohm_   = 1.0;              /* electric resistance (Ohm=V/A) */
  const double S_     = 1.0;              /* electric conductivity (S=A/V) Siemens */

  const double T_     = 1.0;              /* magnetic induction (T=V*s/m2) Tesla */

  const double Wb_    = 1.0;              /* magnetic flux (Wb=V*s) Weber */

  const double H_     = 1.0;              /* magnetic inductivity (H=V*s/A) Henry */

  const double mol_   = 1.0;              /* molecular amount */

  const double Byte_  = 1.0;              /* binary, B means Bel and
                                             cannot be used here */

  DPConstant * element;

  /* --- mathematical constants and units */
  /* Infinity */
  if (dpconstant_insert (          "infinity", NULL,
                            "inf",       inf, &element )) return(-1);
  /* Gamma */
  if (dpconstant_insert (          "gamma number", NULL,
                            "gamma",     gamma, &element )) return(-1);
  /* pi */
  if (dpconstant_insert (          "pi number", NULL,
                            "pi",        pi, &element )) return(-1);

  /* e */
  if (dpconstant_insert (          "Euler number", NULL,
                             "e",         e, &element )) return(-1);


 /* --- angle */
  /* rad (rad)*/
  if (dpconstant_insert (          "angle", "rad",
                           "rad",       rad_, &element )) return(-1);
  /* mrad (rad) */
  if (dpconstant_insert (          "angle", "rad",
                          "mrad", milli*rad_, &element )) return(-1);
  /* urad (rad) */
  if (dpconstant_insert (          "angle", "rad",
                          "urad", micro*rad_, &element )) return(-1);

  /* deg (rad)*/
  if (dpconstant_insert (          "angle", "rad",
                           "deg",       deg_, &element )) return(-1);
  /* mdeg (rad) */
  if (dpconstant_insert (          "angle", "rad",
                           "mdeg", milli*deg_, &element )) return(-1);
  /* udeg (rad) */
  if (dpconstant_insert (          "angle", "rad",
                           "udeg", micro*deg_, &element )) return(-1);

  /* gon (rad)*/
  if (dpconstant_insert (          "angle", "rad",
                           "gon",       gon_, &element )) return(-1);
  /* mgon (rad) */
  if (dpconstant_insert (          "angle", "rad",
                           "mgon", milli*gon_, &element )) return(-1);
  /* ugon (rad) */
  if (dpconstant_insert (          "angle", "rad",
                           "ugon", micro*gon_, &element )) return(-1);

  /* arcmin (rad)*/
  if (dpconstant_insert (          "angle", "rad",
                         "arcmin",  arcmin_, &element )) return(-1);
  /* arcsec (rad)*/
  if (dpconstant_insert (          "angle", "rad",
                         "arcsec",  arcsec_, &element )) return(-1);

 /* --- spherical angle (sr) */
  if (dpconstant_insert (          "spherical angle", "sr",
                           "sr",  rad_*rad_, &element )) return(-1);


 /* --- physical constants and units */
  /* k Boltzmann constant (J/K) */
  if (dpconstant_insert (          "Boltzmann constant", "J/K",
                             "k", k, &element )) return(-1);
  /* me electron rest mass (kg) */
  if (dpconstant_insert (          "electron rest mass", "kg",
                            "me", me, &element )) return(-1);
  /* mp proton rest mass (kg) */
  if (dpconstant_insert (          "proton rest mass", "kg",
                            "mp", mp, &element )) return(-1);
  /* mp deuteron rest mass (kg) */
  if (dpconstant_insert (          "deuteron rest mass", "kg",
                            "md", md, &element )) return(-1);
  /* mn neutron rest mass (kg) */
  if (dpconstant_insert (          "neutron rest mass", "kg",
                            "mn", mn, &element )) return(-1);
  /* NA Avogadro number (1/mol) */
  if (dpconstant_insert (          "Avogadro number", "1/mol",
                            "NA", NA, &element )) return(-1);
  /* re classical electron radius (m) */
  if (dpconstant_insert (          "classical electron radius", "m",
                            "re", re, &element )) return(-1);
  /* c velocity of light */
  if (dpconstant_insert (          "velocity of light", "m/s",
                             "c", c, &element )) return(-1);
  /* ec electron charge */
  if (dpconstant_insert (          "electron charge", "C",
                             "ec", ec, &element )) return(-1);
  /* h Planck's number (J*s) */
  if (dpconstant_insert (          "Planck constant", "J*s",
                             "h", h, &element )) return(-1);
  /* gN gravitational constant (m3/kg/s2) */
  if (dpconstant_insert (          "gravitational constant", "m3/kg/s2",
                            "gN", gN, &element )) return(-1);
  /* ga gravitational acceleration (m/s2) */
  if (dpconstant_insert (       "gravitational acceleration", "m/s2",
                            "ga", ga, &element )) return(-1);
  /* u0 permeability of vacuum = 4e-7*pi H/m */
  if ( dpconstant_insert       ( "permeability of vacuum", "H/m", 
                                 "u0", u0, &element ) ) return(-1);
  /* e0 permittivity of vacuum = 1/u0/c2 (F/m) */
  if ( dpconstant_insert       ( "permittivity of vacuum", "F/m", 
                                 "e0", e0, &element ) ) return(-1);

 /* Charge Coulomb (C) */
  if ( dpconstant_insert_unit  ( "electric charge", "C", "C", C_ ) ) return(-1);

 /* Voltage Volt (V) */
  if ( dpconstant_insert_unit  ( "voltage", "V", "V", V_ ) ) return(-1);

 /* Electric capacity Farad (F=As/V) */
  if ( dpconstant_insert_unit  ( "electric capacity", "F", 
                                 "F", F_ ) ) return(-1);
 /* Resistance (Ohm=V/A) */
  if ( dpconstant_insert_unit  ( "electric resistance", "Ohm", 
                                 "Ohm", Ohm_ ) ) return(-1);

 /* Conductivity Siemens (S=A/V) */
  if ( dpconstant_insert_unit  ( "electric conductivity", "S", 
                                 "S", S_ ) ) return(-1);

 /* Magnetic induction Tesla (T) */
  if ( dpconstant_insert_unit  ( "magnetic induction", "T", 
                                 "T", T_ ) ) return(-1);
  if ( dpconstant_insert_unit  ( "magnetic induction", "T", 
                                 "G", 1e-4*T_ ) ) return(-1);

 /* Magnetic flux Weber (Wb=Vs) */
  if ( dpconstant_insert_unit  ( "magnetic flux", "Wb", 
                                 "Wb", Wb_ ) ) return(-1);

 /* Magnetic inductivity Henry (H=Vs/A) */
  if ( dpconstant_insert_unit  ( "magnetic inductivity", "H", 
                                 "H", H_ ) ) return(-1);

 /* Electric current Ampere (A) */
  if ( dpconstant_insert_unit  ( "electric current", "A",
                                 "A", A_ ) ) return(-1);

 /* Time (s) */
  if ( dpconstant_insert_unit  ( "time", "s", "s", sec_ ) ) return(-1);
  if ( dpconstant_insert       ( "time", "s", 
                                 "min", min_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "time", "s",
                                 "hr", hr_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "time", "s",
                                 "d", d_, &element ) ) return(-1);

 /* Time^2 (s^2) */
  if ( dpconstant_insert_unit2 ( "time^2", "s2", "s", sec_ ) ) return(-1);

 /* Frequency (1/s) */
  if ( dpconstant_insert_unit  ( "frequency", "1/s", "Hz", Hz_ ) ) return(-1);

 /* Length (m) */
  if ( dpconstant_insert_unit  ( "length", "m", "m",   m_ ) ) return(-1);
  if ( dpconstant_insert       ( "length", "m", 
                                 "in", in_, &element )) return(-1);
  if ( dpconstant_insert       ( "length", "m", 
                                 "ft", ft_, &element )) return(-1);
  if ( dpconstant_insert       ( "length", "m", 
                                 "yd", yd_, &element )) return(-1);
  if ( dpconstant_insert       ( "length", "m", 
                                 "mile", mile_, &element )) return(-1);
  if ( dpconstant_insert       ( "length", "m", 
                                 "sm", sm_, &element )) return(-1);

 /* Speed (m/s) */
  if ( dpconstant_insert       ( "speed", "m/s",
                                 "kn", kn_, &element )) return(-1);

 /* Area (m^2) */
  if ( dpconstant_insert_unit2 ( "area", "m2", "m",   m_ ) ) return(-1);
  if ( dpconstant_insert       ( "area", "m2",
                                 "b", barn_, &element )) return(-1);
  if ( dpconstant_insert       ( "area", "m2",
                                 "a", a_, &element )) return(-1);
  if ( dpconstant_insert       ( "area", "m2",
                                 "ha", Hekto*a_, &element )) return(-1);

 /* Volume (m^3) */
  if ( dpconstant_insert_unit3 ( "volume", "m3", "m",   m_ ) ) return(-1);
  if ( dpconstant_insert_unit  ( "volume", "m3", "l",   l_ ) ) return(-1);

 /* Mass (kg) */
  if ( dpconstant_insert_unit  ( "mass", "kg", "g",   g_ ) ) return(-1);
  if ( dpconstant_insert       ( "mass", "kg", 
                                 "lb", lb_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "mass", "kg", 
                                 "oz", oz_, &element ) ) return(-1);
  /* amu atomic mass unit (kg) */
  if (dpconstant_insert (          "mass", "kg",
                           "amu", amu_, &element )) return(-1);

 /* Molecular amount (mol) */
  if ( dpconstant_insert_unit  ( "molecular amount", "mol", 
                                 "mol", mol_ ) ) return(-1);

 /* Energy Joule (J) */
  if ( dpconstant_insert_unit  ( "energy", "J", "J",   J_ ) ) return(-1);
  if ( dpconstant_insert_unit  ( "energy", "J", "eV",  ec*V_ ) ) return(-1);
  if ( dpconstant_insert_unit  ( "energy", "J", "cal", cal_  ) ) return(-1);
  if ( dpconstant_insert       ( "energy", "J", 
                                 "erg", erg_, &element ) ) return(-1);

 /* Temperature Kelvin (K) */
  if ( dpconstant_insert       ( "temperature", "K", 
                                 "K", K_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "temperature", "K", 
                                 "mK", milli*K_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "temperature", "K", 
                                 "uK", micro*K_, &element ) ) return(-1);

 /* Temperature degrees Kelvin, Celsius, Fahrenheit (K) */ 
  if ( dpconstant_insert       ( "temperature", "K", 
                                 "degK", degK_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "temperature", "K", 
                                 "degC", degC_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "temperature", "K", 
                                 "degF", degF_, &element ) ) return(-1);

 /* Power Watt (W) */
  if ( dpconstant_insert_unit  ( "power", "W", "W",   W_ ) ) return(-1);

 /* Force Newton (N) */
  if ( dpconstant_insert_unit  ( "force", "N", "N", N_ ) ) return(-1);
/*  if ( dpconstant_insert_unit  ( "force", "N", "p", p_ ) ) return(-1); */
  if ( dpconstant_insert       ( "force", "N", "p", p_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "force", "N", 
                                 "lbf", lbf_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "force", "N", 
                                 "pdl", pdl_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "force", "N", 
                                 "dyn", dyn_, &element ) ) return(-1);

 /* Pressure Pascal (Pa) */
  if ( dpconstant_insert_unit  ( "pressure", "Pa", "Pa", Pa_ ) ) return(-1);
  if ( dpconstant_insert_unit  ( "pressure", "Pa", "bar", bar_ ) ) return(-1);
  if ( dpconstant_insert       ( "physical atmospheric pressure", "Pa", 
                                 "atm", atm_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "technical atmospheric pressure", "Pa", 
                                 "at", at_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "pressure", "Pa", 
                                 "psi", psi_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "pressure", "Pa", "Torr", Torr_,
                                 &element ) ) return(-1);
 /* Binary constants (Byte) */
  if ( dpconstant_insert       ( "Byte", "Byte", "Byte",
                                  Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "KiloByte", "Byte", "kByte",
                                  Kilo*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "MegaByte", "Byte", "MByte",
                                  Mega*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "GigaByte", "Byte", "GByte",
                                  Giga*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "TeraByte", "Byte", "TByte",
                                  Tera*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "PetaByte", "Byte", "PByte",
                                  Peta*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "ExaByte", "Byte", "EByte",
                                  Exa*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "ZettaByte", "Byte", "ZByte",
                                  Zetta*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "YottaByte", "Byte", "YByte",
                                  Yotta*Byte_, &element ) ) return(-1);

  if ( dpconstant_insert       ( "KibiByte", "Byte", "KiByte", 
                                  pow(1024,1)*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "MebiByte", "Byte", "MiByte", 
                                  pow(1024,2)*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "GibiByte", "Byte", "GiByte", 
                                  pow(1024,3)*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "TebiByte", "Byte", "TiByte", 
                                  pow(1024,4)*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "PebiByte", "Byte", "PiByte", 
                                  pow(1024,5)*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "ExbiByte", "Byte", "EiByte", 
                                  pow(1024,6)*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "ZebiByte", "Byte", "ZiByte", 
                                  pow(1024,7)*Byte_, &element ) ) return(-1);
  if ( dpconstant_insert       ( "YobiByte", "Byte", "YiByte", 
                                  pow(1024,8)*Byte_, &element ) ) return(-1);


  DPConstantInit = 1;

  if ( NUMIO_debug > 1 ) dpconstant_print ( stdout, 1, 0 );

  return( 0 );

} /* dpconstant_init */

/***************************************************************************
* Number Functions                                                         *
***************************************************************************/
void dpconstant( NumProg * program, const char **ps, int *perrval)
{
  double value;
  DPConstant * constant;

  *perrval = NumSuccess;
  value = 1.0;

  if (!DPConstantInit) dpconstant_init();

  if ( !dpconstant_search( *ps, &constant, 1 ) ) {
    value=constant->Value; *ps=*ps+strlen(constant->Key);

    if (numprog_append_instruction ( program, 0, PUSHVAL, 0, value, 
      NULL, NULL)) { *perrval = NumProgramError; return; }

  } else {
    /* --- no float constant */
    *perrval = NumNoFloatNumber;
  }

  return;

} /* dpconstant */

void dpvariable( NumProg * program, const char **ps, int *perrval)
{
  double *addr;
  NumVar *variable;

  *perrval = NumSuccess;

  if ( !numprog_search_variable ( program, *ps, &variable, 1 ) ) {
    addr=&(variable->Value); *ps=*ps+strlen(variable->Key);
    variable->Used++;

    if (numprog_append_instruction ( program, 0, PUSHADDR, 0, *addr, 
      addr, NULL )) { *perrval = NumProgramError; return; }

  } else {
    /* --- undefined variable */
    *perrval = NumNoVariable;
  }
  
  return;

} /* dpvariable */

/*---------------------------------------------------------------------------
NAME

   dpfunction --- Append a function call to program.

SYNOPSI

   void dpfunction( NumProg * program, const char **ps,
                    int level, int *perrval);

DESCRIPTION

   Append a function call to program.

   To add a new function FX the following steps needs to be done
  
   file numprog.h: Add FX to NumCommand and NumCommandStrings
   dpprogram_step: Add an entry of FX to dpprogram_step. All functions
                   needs to be defined there.
       dpfunction: Add the string FX_ to dpfunction. It must be terminated
                   with '(', e.g. const char *FX_="fx(".
                   Append the instruction with numprog_append_instruction,
                   as it is done for the other functions. NArgs must be
                   exactly the number of arguments that are needed to
                   calculate the function value.

----------------------------------------------------------------------------*/
void dpfunction( NumProg * program, const char **ps, 
                 int level, int *perrval)
{
   const double pi = 3.1415926535897932384626;
//   const double degtorad = pi/180.0; // unused
//   const double radtodeg = 180.0/pi; // unused

   char * RAD_="rad("; char * DEG_="deg("; char * PI_ ="pi(";
   char * SIN_="sin("; char * COS_="cos("; char * TAN_="tan(";
   char * ASIN_="asin("; char * ACOS_="acos("; char * ATAN_="atan(";
   char * ATAN2_="atan2("; char * SINH_="sinh("; char * COSH_="cosh(";
   char * TANH_="tanh("; char * FLOOR_="floor("; char * CEIL_="ceil(";
   char * FABS_="abs("; char * EXP_="exp("; char * LOG_="log(";
   char * LOG10_="log10("; char * POW_="pow("; char * SQRT_="sqrt(";
   char * ROUND_="round("; char * GAMMA_="gamma(";
   char * FMIN_="min("; char * FMAX_="max(";
   char * DEGC2K_="degC2K("; char * K2DEGC_="K2degC(";
   char * DEGF2K_="degF2K("; char * K2DEGF_="K2degF(";
   char * DEGK2K_="degK2K("; char * K2DEGK_="K2degK(";
   char * DEGF2DEGC_="degF2degC("; char * DEGC2DEGF_="degC2degF(";

   *perrval = NumSuccess;

         /* rad-function */
              if (!strncmp(*ps,RAD_,strlen(RAD_))) { *ps+=strlen(RAD_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, RAD, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* rad_ */

         /* deg-function */
         else if (!strncmp(*ps,DEG_,strlen(DEG_))) { *ps+=strlen(DEG_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, DEG, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* DEG_ */

         /* pi-function */
         else if (!strncmp(*ps,PI_,strlen(PI_))) { *ps+=strlen(PI_);

             if (numprog_append_instruction ( program, 0, PUSHVAL, 0, pi, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* PI_ */

         /* sin-function */
         else if (!strncmp(*ps,SIN_,strlen(SIN_))) { *ps+=strlen(SIN_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, SIN, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* SIN_ */

         /* cos-function */
         else if (!strncmp(*ps,COS_,strlen(COS_))) { *ps+=strlen(COS_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, COS, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* COS_ */

         /* tan-function */
         else if (!strncmp(*ps,TAN_,strlen(TAN_))) { *ps+=strlen(TAN_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, TAN, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* TAN_ */

         /* asin-function */
         else if (!strncmp(*ps,ASIN_,strlen(ASIN_))) { *ps+=strlen(ASIN_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, ASIN, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* ASIN_ */

         /* acos-function */
         else if (!strncmp(*ps,ACOS_,strlen(ACOS_))) { *ps+=strlen(ACOS_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, ACOS, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* ACOS_ */

         /* atan-function */
         else if (!strncmp(*ps,ATAN_,strlen(ATAN_))) { *ps+=strlen(ATAN_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, ATAN, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* ATAN_ */

         /* atan2-function */
         else if (!strncmp(*ps,ATAN2_,strlen(ATAN2_))) { *ps+=strlen(ATAN2_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;
             if ((**ps)!=',') *perrval = NumCommaExpected; else (*ps)++;
             if (*perrval != NumSuccess) return;
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, ATAN2, 2, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* ATAN2_ */

         /* sinh-function */
         else if (!strncmp(*ps,SINH_,strlen(SINH_))) { *ps+=strlen(SINH_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, SINH, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* SINH_ */

         /* cosh-function */
         else if (!strncmp(*ps,COSH_,strlen(COSH_))) { *ps+=strlen(COSH_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, COSH, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* COSH_ */

         /* tanh-function */
         else if (!strncmp(*ps,TANH_,strlen(TANH_))) { *ps+=strlen(TANH_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, TANH, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* TANH_ */

         /* floor-function */
         else if (!strncmp(*ps,FLOOR_,strlen(FLOOR_))) { *ps+=strlen(FLOOR_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, FLOOR, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* FLOOR_ */

         /* ceil-function */
         else if (!strncmp(*ps,CEIL_,strlen(CEIL_))) { *ps+=strlen(CEIL_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, CEIL, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* FLOOR_ */

         /* fabs-function */
         else if (!strncmp(*ps,FABS_,strlen(FABS_))) { *ps+=strlen(FABS_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, FABS, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* FABS_ */

         /* exp-function */
         else if (!strncmp(*ps,EXP_,strlen(EXP_))) { *ps+=strlen(EXP_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, EXP, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* EXP_ */

         /* log-function */
         else if (!strncmp(*ps,LOG_,strlen(LOG_))) { *ps+=strlen(LOG_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, LOG, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* LOG_ */

         /* log10-function */
         else if (!strncmp(*ps,LOG10_,strlen(LOG10_))) { *ps+=strlen(LOG10_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, LOG10, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* LOG10_ */

         /* pow-function */
         else if (!strncmp(*ps,POW_,strlen(POW_))) { *ps+=strlen(POW_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;
             if ((**ps)!=',') *perrval = NumCommaExpected; else (*ps)++;
             if (*perrval != NumSuccess) return;
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, POW, 2, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* POW_ */

         /* sqrt-function */
         else if (!strncmp(*ps,SQRT_,strlen(SQRT_))) { *ps+=strlen(SQRT_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, SQRT, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* SQRT_ */

         /* round-function */
         else if (!strncmp(*ps,ROUND_,strlen(ROUND_))) { *ps+=strlen(ROUND_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, ROUND, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* ROUND_ */

         /* gamma-function */
         else if (!strncmp(*ps,GAMMA_,strlen(GAMMA_))) { *ps+=strlen(GAMMA_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, GAMMA, 1, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* GAMMA_ */

         /* min-function */
         else if (!strncmp(*ps,FMIN_,strlen(FMIN_))) { *ps+=strlen(FMIN_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;
             if ((**ps)!=',') *perrval = NumCommaExpected; else (*ps)++;
             if (*perrval != NumSuccess) return;
             EXPRESSION(program,ps,level+1,perrval);

             if (numprog_append_instruction ( program, 0, FMIN, 2, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* FMIN_ */

         /* max-function */
         else if (!strncmp(*ps,FMAX_,strlen(FMAX_))) { *ps+=strlen(FMAX_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;
             if ((**ps)!=',') *perrval = NumCommaExpected; else (*ps)++;
             if (*perrval != NumSuccess) return;
             EXPRESSION(program,ps,level+1,perrval);

             if (numprog_append_instruction ( program, 0, FMAX, 2, 0, 
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* FMAX_ */

         /* degC2K-function */
         else if (!strncmp(*ps,DEGC2K_,strlen(DEGC2K_))) { *ps+=strlen(DEGC2K_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, DEGC2K, 1, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* DEGC2K_ */

         /* K2degC-function */
         else if (!strncmp(*ps,K2DEGC_,strlen(K2DEGC_))) { *ps+=strlen(K2DEGC_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, K2DEGC, 1, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* K2DEGC_ */

         /* degF2K-function */
         else if (!strncmp(*ps,DEGF2K_,strlen(DEGF2K_))) { *ps+=strlen(DEGF2K_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, DEGF2K, 1, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* DEGF2K_ */

         /* K2degF-function */
         else if (!strncmp(*ps,K2DEGF_,strlen(K2DEGF_))) { *ps+=strlen(K2DEGF_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, K2DEGF, 1, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* K2DEGF_ */

         /* degK2K-function */
         else if (!strncmp(*ps,DEGK2K_,strlen(DEGK2K_))) { *ps+=strlen(DEGK2K_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, DEGK2K, 1, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* DEGK2K_ */

         /* K2degK-function */
         else if (!strncmp(*ps,K2DEGK_,strlen(K2DEGK_))) { *ps+=strlen(K2DEGK_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, K2DEGK, 1, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* K2DEGK_ */

         /* degF2degC-function */
         else if (!strncmp(*ps,DEGF2DEGC_,strlen(DEGF2DEGC_))) { 
             *ps+=strlen(DEGF2DEGC_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, DEGF2DEGC, 1, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* DEGF2DEGC_ */

         /* degC2degF-function */
         else if (!strncmp(*ps,DEGC2DEGF_,strlen(DEGC2DEGF_))) { 
             *ps+=strlen(DEGC2DEGF_);
             EXPRESSION(program,ps,level+1,perrval);
             if (*perrval != NumSuccess) return;

             if (numprog_append_instruction ( program, 0, DEGC2DEGF, 1, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

             } /* DEGC2DEGF_ */

         else { /* unknown function */
             *perrval = NumNoFloatFunction; return;
             }

         if ((**ps)!=')') *perrval = NumBadParenthesis; else (*ps)++;

         return;

} /* dpfunction */

void dpterm( NumProg * program, const char **ps, int level, int * perrval)
{
   *perrval = NumSuccess;

   dpfactor0(program,ps,level,perrval);
   if (*perrval!=NumSuccess) return;

   while (**ps) {
     switch (**ps) {
       case '*' :
         (*ps)++; dpfactor0(program,ps,level,perrval); 
         if (*perrval==NumSuccess) {

           if (numprog_append_instruction ( program, 0, MUL, 2, 0, 
             NULL, NULL )) { *perrval = NumProgramError; return; }

         }

         break;
       case '/' :
         (*ps)++; dpfactor0(program,ps,level,perrval);
         if (*perrval==NumSuccess) {

           if (numprog_append_instruction ( program, 0, DIV, 2, 0, 
             NULL, NULL )) { *perrval = NumProgramError; return; }

         }
         break;
       case '%' :
         (*ps)++; dpfactor0(program,ps,level,perrval);
         if (*perrval==NumSuccess) {

           if (numprog_append_instruction ( program, 0, REST, 2, 0, 
             NULL, NULL )) { *perrval = NumProgramError; return; }

         }
         break;
       default  :
         return;
     } /* switch */
     if (*perrval!=NumSuccess) return;
   } /* while */

   return;

} /* dpterm */

// Unary operator
void dpfactor0( NumProg * program, const char **ps,
                int level, int * perrval)
{
   *perrval = NumSuccess;

     switch (**ps) {
       case '!' : 
         (*ps)++; dpfactor1(program,ps,level,perrval);
         if (*perrval==NumSuccess) {

           if (numprog_append_instruction ( program, 0, NOT, 1, 0, 
             NULL, NULL )) { *perrval = NumProgramError; return; }

         }
         break;
       default  :
         dpfactor1(program,ps,level,perrval);
     } /* switch */
     if (*perrval!=NumSuccess) return;

   return;

} /* dpfactor0 */

// Unit multiplicator 
void dpfactor1( NumProg * program, const char **ps, 
                int level, int * perrval)
{
   *perrval = NumSuccess;

   dpfactor2(program,ps,level,perrval);
   if (*perrval!=NumSuccess) return;

   while (**ps) {
     switch (**ps) {
       case '_' :
         (*ps)++; dpfactor2(program,ps,level,perrval);
         if (*perrval==NumSuccess) {

           if (numprog_append_instruction ( program, 0, MUL, 2, 0, 
             NULL, NULL )) { *perrval = NumProgramError; return; }

         }
         break;
       default  :
         return;
     } /* switch */
     if (*perrval!=NumSuccess) return;
   } /* while */

   return;

} /* dpfactor1 */

// Values
void dpfactor2( NumProg * program, const char **ps, 
                int level, int * perrval)
{
   char * DOUBLE_ = "(double)";
   double value;
   char *pe;

   *perrval = NumSuccess;

   switch (**ps) {
     case '(' :
     /* --- (double) */
       if (!strncmp(*ps,DOUBLE_,strlen(DOUBLE_))) {
           *ps=*ps+strlen(DOUBLE_);
           value = (double) lvfactor(ps,level,perrval);
           if (*perrval != NumSuccess) break;

           if (numprog_append_instruction ( program, 0, PUSHVAL, 0, value, 
             NULL, NULL )) { *perrval = NumProgramError; return; }

           break;}
     /* --- expression */
       (*ps)++; EXPRESSION(program,ps,level+1,perrval);
       if (*perrval != NumSuccess) break;
       if ((**ps)!=')') *perrval = NumBadParenthesis; else (*ps)++;
       break;
     default  :
     /* --- number */
       if ( (('0'<=**ps)&&(**ps<='9'))||('.'==**ps) ) {
         value = strtod(*ps,&pe);
         *ps = (const char *) pe;

         if (numprog_append_instruction ( program, 0, PUSHVAL, 0, value, 
           NULL, NULL )) { *perrval = NumProgramError; return; }

       } /* number */
     /* --- function */
       else if (isfunction(*ps)) {
       dpfunction(program,ps,level,perrval);
       if (*perrval != NumSuccess) break;
       } /* function */
     /* --- variable */
       else if (isvariable(*ps)) {
         (*ps)++; /* skip underscore */
         dpvariable(program,ps,perrval);
         if (*perrval != NumSuccess) break;
       } /* variable */
     /* --- constant */
       else { dpconstant(program,ps,perrval);
       if (*perrval != NumSuccess) break;
       } /* constant */
       break;
   } /* switch */
   if (*perrval!=NumSuccess) return;

   return;
} /* dpfactor2 */

void dpexpression( NumProg * program, const char **ps, 
                   int level, int * perrval)
{
   *perrval = NumSuccess;
   switch (**ps) {
     case '+':
       (*ps)++; dpterm(program,ps,level,perrval);
       break;
     case '-':
       (*ps)++; dpterm(program,ps,level,perrval);
       if (*perrval==NumSuccess) {

         if (numprog_append_instruction ( program, 0, NEG, 1, 0, 
           NULL, NULL )) { *perrval = NumProgramError; return; }

       }
       break;
     default :
       dpterm(program,ps,level,perrval);
       break;
   } /* switch */
   if (*perrval!=NumSuccess) return;

   while (**ps) {
     switch (**ps) {
       case '+':
         (*ps)++; dpterm(program,ps,level,perrval);
         if (*perrval==NumSuccess) {

         if (numprog_append_instruction ( program, 0, ADD, 2, 0, 
           NULL, NULL )) { *perrval = NumProgramError; return; }

         }
         break;
       case '-':
         (*ps)++; dpterm(program,ps,level,perrval);
         if (*perrval==NumSuccess) {

           if (numprog_append_instruction ( program, 0, SUB, 2, 0, 
             NULL, NULL )) { *perrval = NumProgramError; return; }

         }
         break;
       case ')':
         if (level<=0) *perrval = NumBadParenthesis; 
         return;
//       case ';':
//       case ',':
//         return;
       default :
         return; // Return without error
//         if (isspace(**ps)) { //++++++++++++++++++
//           return;
//         }
//         *perrval = NumScanError; break;
     } /* switch */
     if (*perrval!=NumSuccess) return;
   } /* while */
   return;

} /* dpexpression */

//  condition    = logicsum ["?" logicsum ":" logicsum ]
void dpcondition( NumProg * program, const char **ps,
                  int level, int * perrval)
{ *perrval = NumSuccess;

  dplogicsum(program,ps,level,perrval);
  if (*perrval != NumSuccess) return;

  switch (**ps) {
    case '?': 
      (*ps)++; dplogicsum(program,ps,level,perrval);
      if (*perrval != NumSuccess) return;

      switch (**ps) {
        case ':':
          (*ps)++; dplogicsum(program,ps,level,perrval);

          if (*perrval==NumSuccess) {

            if (numprog_append_instruction ( program, 0, IF, 3, 0,
              NULL, NULL )) { *perrval = NumProgramError; return; }

          }
          break;
        default: 
         *perrval = NumScanError;
      } // switch
    default: ; // continue
  } // switch
  if (*perrval != NumSuccess) return;

  return;

} // dpcondition

// logicsum = logicproduct { || logicproduct }
void dplogicsum( NumProg * program, const char **ps,
                 int level, int * perrval)
{ const char *pps;

  *perrval = NumSuccess;

  dplogicproduct(program,ps,level,perrval);
  if (*perrval != NumSuccess) return;

  while (**ps) {
    pps = *ps;
    switch (*pps) {
      case '|': (pps)++;
        switch (*pps) {
          case '|': (pps)++;  // "||" OR
            *ps = pps; dplogicproduct(program,ps,level,perrval);
             if (*perrval==NumSuccess) {

               if (numprog_append_instruction ( program, 0, OR, 2, 0,
                 NULL, NULL )) { *perrval = NumProgramError; return; }

             }
             break;
          default: return; 
        }
        break;
      default: return; 
    } // switch
    if (*perrval != NumSuccess) return;
  } // while

  return;

} // dplogicsum

// logicproduct = equality { && equality }
void dplogicproduct( NumProg * program, const char **ps,
                     int level, int * perrval)
{ const char *pps;

  *perrval = NumSuccess;

  dpequality(program,ps,level,perrval);
  if (*perrval != NumSuccess) return;

  while (**ps) {
    pps = *ps;
    switch (*pps) {
      case '&': (pps)++;
        switch (*pps) {
          case '&': (pps)++;  // "&&" AND
            *ps = pps; dpequality(program,ps,level,perrval);
             if (*perrval==NumSuccess) {

               if (numprog_append_instruction ( program, 0, AND, 2, 0,
                 NULL, NULL )) { *perrval = NumProgramError; return; }

             }
             break;
          default: return;
        }
        break;
      default: return;
    } // switch
    if (*perrval != NumSuccess) return;
  } // while

  return;

} // dplogicproduct

// equality = comparison {  "==" | "!=" comparison }
void dpequality( NumProg * program, const char **ps,
                 int level, int * perrval)
{ const char *pps;

  *perrval = NumSuccess;

  dpcomparison(program,ps,level,perrval);
  if (*perrval != NumSuccess) return;

  pps = *ps;

  switch (*pps) {
    case '=': (pps)++;
      switch (*pps) {
        case '=': (pps)++;  // "==" EQ
          *ps = pps; dpcomparison(program,ps,level,perrval);
           if (*perrval==NumSuccess) {

             if (numprog_append_instruction ( program, 0, EQU, 2, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

           }
           break;

        default: ; // continue 
      }
      break;

    case '!': (pps)++;
      switch (*pps) {
        case '=': (pps)++;  // "==" NE
          *ps = pps; dpcomparison(program,ps,level,perrval);
           if (*perrval==NumSuccess) {

             if (numprog_append_instruction ( program, 0, NEQ, 2, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

           }
           break;

        default: ; //  continue
      }

    default: ; //  continue
  } // switch

  return;

} // dpequality

// comparison = expression { "<" | "<=" | ">" | ">="  expression }
void dpcomparison( NumProg * program, const char **ps, 
                   int level, int * perrval)
{ const char *pps;

  *perrval = NumSuccess;

  dpexpression(program,ps,level,perrval);
  if (*perrval != NumSuccess) return;

  pps = *ps;

  switch (*pps) {
    case '<': (pps)++;
      switch (*pps) {
        case '=': (pps)++;  // "<=" LE
          *ps = pps; dpexpression(program,ps,level,perrval);
           if (*perrval==NumSuccess) {

             if (numprog_append_instruction ( program, 0, LE, 2, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

           }
           break;

        default: // "<" LT
          *ps = pps; dpexpression(program,ps,level,perrval);
           if (*perrval==NumSuccess) {

             if (numprog_append_instruction ( program, 0, LT, 2, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

           }
           break;

      } // switch
      break;

    case '>': (pps)++;
      switch (*pps) {
        case '=': (pps)++;  // ">=" GE
          *ps = pps; dpexpression(program,ps,level,perrval);
           if (*perrval==NumSuccess) {

             if (numprog_append_instruction ( program, 0, GE, 2, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

           }
           break;

        default: // ">" LT
          *ps = pps; dpexpression(program,ps,level,perrval);
           if (*perrval==NumSuccess) {

             if (numprog_append_instruction ( program, 0, GT, 2, 0,
               NULL, NULL )) { *perrval = NumProgramError; return; }

           }
           break;

      } // switch
      break;

  } // switch

  return;

} // dpcomparison

/*--------------------------------------------------------------------------
   num_str2double : reads an expression of the type
               EXPRESSION   = condition
               condition    = logicsum ["?" logicsum ":" logicsum ]
               logicsum     = logicproduct { "||" logicproduct }
               logicproduct = equality { "&&" equality }
                equality    = comparison ["=="|"!=" comparison]
                comparison  = expression ["<"|"<="|">"|">=" expression]
                expression  = ["+"|"-"] term {"+"|"-" term}
                 term       = factor1 {"*"|"/"|"%" factor1}
                  factor0   = ["!"] factor1
                  factor1   = factor2 {"_" factor2}
                  factor2   = number | function | constant | "(" EXPRESSION ")"
                                | "(double)" lvfactor | variable
                   number   = double precision type floating point number
                   function = name "(" expression {"," expression} ")"
                    name    = "a"|..|"z" {"a"|..|"z"|"0"|..|"9"}
                   constant = unit | name
                    unit    = name
                   variable = "_" name
  
        The conversion stops at at the end of the string, a white space, 
        a comma or a semicolon. If tail is not NULL the pointer to the 
        remaining string, including the terminating character, is returned
        in *tail.
        If the evaluated expression is not complete or faulty, an      
        error value is returned.
        A temporary program with the name 'str2double' is created. It is
        exectuted after successful creation. The resulting value is 
        returned. The program is deleted after the run.
  
   	return float value  (o) : value of expression
        const char  *str    (i) : input string
              char **tail   (o) : pointer to rest string if tail!=NULL
        int * perrval       (o) : output status
                                 NumSuccess        : successful conversion
                                 NumBadParenthesis : wrong number of parentheses
                                 NumNoFloatNumber  : mysterious character found
                                 NumDomainError    :
                                 etc.
  --------------------------------------------------------------------------*/
PUBLIC double num_str2double(const char *str, const char **tail, int *perrval)
{
  int errval;

  double value=0.0;
  const char * ps="";

  NumProg * program;

  if (str) ps = str;

  errval = NumSuccess;

  while (isspace(*ps)) ps++; // skip leading white space

  if ( !(program = numprog_new( "str2double" )) ) errval = NumProgramError;

  if (!errval) EXPRESSION( program, &ps, 0 , &errval);

  if (tail) *tail = ps;

  if (!errval) dpprogram_run ( program, &errval );
  if (!errval) value = program->CurrentAccumulator->Value;

  if (errval>=NumProgramError) num_prog_print_list ( stderr, program, 2, 0 );
  else if (NUMIO_debug>2) num_prog_print_list ( stdout, program, 2, 0 );

  if (numprog_free( program )) errval = NumProgramError; 
 
  if (perrval) *perrval=errval;
  return( value );
 
} /* num_str2double */

/*--------------------------------------------------------------------------
   num_str2prog : Converts an expression with nvar variables to a program
                  that can be exectuted with num_runprog. The syntax
                  is like it is described for num_str2double.
                  Constant expressions are evaluated.
                  Exactly nvar variable names must be given.
                  If not, random errors will occur.
  
        return NumProg *    (o) : pointer to the program
        const char *name    (i) : program name
        const char  *str    (i) : input string
              char **tail   (o) : pointer to rest string if tail!=NULL
        int * perrval       (o) : output status
        int nvar            (i) : number of variables
        const char *nam1,
                   ...      (i) : names of the variables
  
  --------------------------------------------------------------------------*/
PUBLIC NumProg * num_str2prog( const char *name,
                               const char *str, const char **tail, int *perrval,
                               int nvar, ... )
{
  int errval;

  va_list ap;
  const char * vname;
  int n;

  const char * ps="";
 
  NumProg * program;
 
  if (str) ps = str;
 
  errval = NumSuccess;

  while (isspace(*ps)) ps++; // skip leading white space
 
  if ( !(program = numprog_new( name )) ) errval = NumProgramError;

  if (!errval) {
    va_start(ap, nvar);
    for (n=0;n<nvar;n++) {
      vname = va_arg(ap, const char *);
      if (!errval)
        if (numprog_append_variable ( program, vname, 1.0, NULL ))
          errval = NumVariableError;
    }
    va_end(ap);
  }

  if (!errval) EXPRESSION( program, &ps, 0 , &errval);
 
  if (tail) *tail = ps;
 
  if (!errval) dpprogram_compile  ( program, &errval );
 
  if (errval>=NumProgramError) num_prog_print_list ( stderr, program, 2, 0 );
  else if (NUMIO_debug>2) num_prog_print_list ( stdout, program, 2, 0 );
 
  if (errval) {
    if (numprog_free( program )) errval = NumProgramError;
    program = (NumProg *) NULL;
  }

  if (perrval) *perrval = errval;
  return( program );
 
} /* num_str2prog */

/*--------------------------------------------------------------------------  
   num_chkvar : Returns, how often the n-th variable is used in program.
                It returns the Used-flag of the n-th variable.
  
        return int          (o) : 0: does not depend on variable n
                                  1: depends on variable n
        NumProg *           (i) : program pointer created with num_str2prog4
        int n               (i) : variable number (according to num_str2prog)
        int * perrval       (o) : output status
  
  --------------------------------------------------------------------------*/
PUBLIC int num_chkvar ( NumProg * program, int n, int *perrval )
{
  int errval;
 
  NumVar * variable = (NumVar *) NULL;
  int used = 0;
  int i;
 
  errval = NumSuccess;
 
  if (!program) errval=NumProgramError;

  if (!errval) { 
    variable=program->VariableList;
    /* go to n-th variable */ 
    for (i=1;i<n;i++) {
      if (variable) variable=variable->Next;
      else break;
    }
    if (variable) used = variable->Used;
    else errval=NumNoVariable;
  }

  if (perrval) *perrval=errval;
  return( used );
 
} /* num_chkvar */

/*--------------------------------------------------------------------------  
   num_runprog : Runs a program with nvar variables that was created with
                 num_str2prog and returns the result.
  
        return double       (o) : evaluated value
        NumProg *           (i) : program pointer created with num_str2prog4
        int * perrval       (o) : output status
        double var1, 
               ...          (i) : variable values. They must be given in the
                                  same order as they have been defined with 
                                  num_str2prog. Exactly the in num_str2prog
                                  defined number of variables values must 
                                  be given. If not, random errors will occur.
                                  ATTENTION: The types of var1, ... are not
                                  known to the compiler, values are not casted 
                                  automatically to double.
  
  --------------------------------------------------------------------------*/
PUBLIC double num_runprog( NumProg * program, int *perrval, ... )
{
  int errval;

  va_list ap;
  double var;

  NumVar * variable = (NumVar *) NULL;
  double value=0.0;

  errval = NumSuccess;

  if (!program) errval=NumProgramError;

  if (!errval) {

    variable  = program->VariableList;

    va_start(ap, perrval); // perrval is the last argument before ...
    while (variable) {
      var = va_arg(ap, double);
      variable->Value = var; variable=variable->Next;
    }
    va_end(ap);
  }

  if (!errval) dpprogram_run ( program, &errval );

  if (!errval) value = program->CurrentAccumulator->Value;

  if (errval>=NumProgramError) num_prog_print_list ( stderr, program, 2, 0 );
  else if (NUMIO_debug>2) num_prog_print_list ( stdout, program, 2, 0 );

  if (perrval) *perrval=errval; 

  return( value );

} /* num_runprog */

/*--------------------------------------------------------------------------  
   num_searchprog: Returns the pointer of a program
  
        return NumProg *    (o) : pointer to the program or 
                                  null-pointer if it was not found 
        const char * name   (i) : program name
        int * perrval       (o) : output status
  
        Attention: *perrval indicates errors only. It does not indicate
                   whether the program was found.
  
  --------------------------------------------------------------------------*/
PUBLIC NumProg *num_searchprog ( const char *name, int *perrval )
{ 
  int errval;

  NumProg *program;

  errval = NumSuccess;

  if ( numprog_search ( name, &program ) ) errval=NumProgramError;

  if (perrval) *perrval=errval;
  return( program );

} /* num_searchprog */
 
/*--------------------------------------------------------------------------  
   num_rmprog : Removes the specified program or all defined programs, if
                the null-pointer is given
  
        return int          (o) : 0: Success, -1: error 
        NumProg *         (i) : pointer to the program that should be remoed,
                                  or null-pointer to remove all programs 
        int * perrval       (o) : output status
  
  --------------------------------------------------------------------------*/
PUBLIC int num_rmprog( NumProg * program, int *perrval ) 
{
  int errval;
  int status;
 
  errval = NumSuccess;

  if ( (status = numprog_free( program )) ) errval = NumProgramError;

  if (perrval) *perrval=errval;
  return( status );

} /* num_rmprog */

long int lvterm( const char **ps, int level, int * perrval)
{
   long int value;
   long int divisor;

   *perrval = NumSuccess;

   value = lvfactor(ps,level,perrval);
   if (*perrval!=NumSuccess) return ( value );

   while (**ps) {
     switch (**ps) {
       case '*' :
         (*ps)++; value *= lvfactor(ps,level,perrval); break;
       case '/' :
         (*ps)++; divisor = lvfactor(ps,level,perrval);
         if ( divisor != 0l) value /= divisor;
           else *perrval=NumDivByZero;
         break;
       case '%' :
         (*ps)++; divisor = lvfactor(ps,level,perrval);
         if ( divisor != 0l) value %= divisor;
           else *perrval=NumDivByZero;
         break;
       default  :
         return ( value );
     } /* switch */
     if (*perrval!=NumSuccess) return ( value );
   } /* while */

   return ( value );

} /* lvterm */

long int lvfactor( const char **ps, int level, int * perrval)
{
   const double long_max = (double) LONG_MAX;
   const double long_min = (double) LONG_MIN;

   NumProg * program;
   double dpargument;
   char *pe;

   long int value;
   char * TRUE_ = "true"; char * FALSE_ = "false";
   char * YES_  = "yes";  char * NO_    = "no";
   char * LONG_ = "(long int)"; char * ROUND_ = "(round)";

   *perrval = NumSuccess;
   value = 1l;

     switch (**ps) {
       case '(' :
       /* --- (long int) */
         if (!strncmp(*ps,LONG_,strlen(LONG_))) {
             *ps=*ps+strlen(LONG_);

             /* Temporary program required to get dpfactor1 result */
             if (!( program = numprog_new( "lvfactor" ) ) ) {
               *perrval = NumProgramError; break;
             }

             dpfactor1(program,ps,level,perrval);
             if (*perrval!=NumSuccess) {
               numprog_free( program ); break;
             }
             dpprogram_run ( program, perrval );
             if (*perrval!=NumSuccess) {
               numprog_free( program ); break;
             }
             dpargument = program->CurrentAccumulator->Value;
             if (numprog_free( program )) {
               *perrval = NumProgramError; break;
             }

             if ((long_min <= dpargument) && (dpargument <= long_max))
                value=(long int) dpargument; 
             else *perrval = NumIntegerOverflow;
             break; }
         if (!strncmp(*ps,ROUND_,strlen(ROUND_))) {
             *ps=*ps+strlen(ROUND_);

             /* Temporary program required to get dpfactor1 result */
             if (!( program = numprog_new( "lvfactor" ) ) ) {
               *perrval = NumProgramError; break;
             }
             dpfactor1(program,ps,level,perrval);
             if (*perrval!=NumSuccess) {
               numprog_free( program ); break;
             }
             dpprogram_run ( program, perrval );
             if (*perrval!=NumSuccess) {
               numprog_free( program ); break;
             }
             dpargument = program->CurrentAccumulator->Value;
             if (numprog_free( program )) {
               *perrval = NumProgramError; break;
             }                                                                  

             dpargument = floor(dpargument+0.5);

             if ((long_min <= dpargument) && (dpargument <= long_max))
                value=(long int) dpargument; 
             else *perrval = NumIntegerOverflow;
             break; }
       /* --- expression */
         (*ps)++; value *= lvexpression(ps,level+1,perrval);
         if (*perrval != NumSuccess) break;
            if ((**ps)!=')') *perrval = NumBadParenthesis; else (*ps)++;
         break;
       default  :
       /* --- number */
         if (('0'<=**ps) && (**ps<='9')) {
           value=strtol(*ps,&pe,10);
           *ps = (const char *) pe;
         }
       /* --- function */
         else if (isfunction(*ps)) {

           /* Temporary program required to get dpfunction result */
           if (!( program = numprog_new( "lvfactor" ) ) ) {
             *perrval = NumProgramError; break;
           }
           dpfunction(program,ps,level,perrval);
           if (*perrval!=NumSuccess) {
             numprog_free( program ); break;
           }
           dpprogram_run ( program, perrval );
           if (*perrval!=NumSuccess) {
             numprog_free( program ); break;
           }
           dpargument = program->CurrentAccumulator->Value;
           if (numprog_free( program )) {
             *perrval = NumProgramError; break;
           }                                                                  
           dpargument = floor(dpargument+0.5);
           if ((long_min <= dpargument) && (dpargument <= long_max))
              value=(long int) dpargument; else *perrval = NumIntegerOverflow;
           if (*perrval != NumSuccess) break;
         } /* function */
       /* --- constant */
         else if (!num_strncasecmp(*ps,TRUE_,strlen(TRUE_))) {
                  value=1l; *ps=*ps+strlen(TRUE_);}
         else if (!num_strncasecmp(*ps,FALSE_,strlen(FALSE_))) {
                  value=0l; *ps=*ps+strlen(FALSE_);}
         else if (!num_strncasecmp(*ps,YES_,strlen(YES_))) {
                  value=1l; *ps=*ps+strlen(YES_);}
         else if (!num_strncasecmp(*ps,NO_,strlen(NO_))) {
                  value=0l; *ps=*ps+strlen(NO_);}
         else {
           /* Temporary program required to get dpconstant result */
           if (!( program = numprog_new( "lvfactor" ) ) ) {
             *perrval = NumProgramError; break;
           }
           dpconstant(program,ps,perrval);
           if (*perrval!=NumSuccess) {
             numprog_free( program ); break;
           }
           dpprogram_run ( program, perrval );
           if (*perrval!=NumSuccess) {
             numprog_free( program ); break;
           }
           dpargument = program->CurrentAccumulator->Value;
           if (numprog_free( program )) {
             *perrval = NumProgramError; break;
           }                                                                    
           dpargument = floor(dpargument+0.5);
           if ((long_min <= dpargument) && (dpargument <= long_max))
              value=(long int) dpargument; else *perrval = NumIntegerOverflow;
           if (*perrval != NumSuccess) break; 
         }
       /* --- exit */
         break;
     } /* switch */
     if (*perrval!=NumSuccess) return ( value );

   return( value );
} /* lvfactor */

long int lvexpression( const char **ps, int level, int * perrval)
{  long int value;

   *perrval = NumSuccess;
   value = 0l;
   switch (**ps) {
     case '+':
       (*ps)++; value += lvterm(ps,level,perrval); break;
     case '-':
       (*ps)++; value -= lvterm(ps,level,perrval); break;
     default :
       value += lvterm(ps,level,perrval); break;
   } /* switch */
   if (*perrval!=NumSuccess) return ( value );

   while (**ps) {
     switch (**ps) {
       case '+':
         (*ps)++; value += lvterm(ps,level,perrval); break;
       case '-':
         (*ps)++; value -= lvterm(ps,level,perrval); break;
       case ')':
         if (level<=0) *perrval = NumBadParenthesis;
         return( value );
       case ';':
       case ',':
         return( value );
       default :
         if (isspace(**ps)) return( value );
         *perrval = NumScanError; break;
     } /* switch */
     if (*perrval!=NumSuccess) return ( value );
   } /* while */
   return(value);

} /* lvexpression */

/*--------------------------------------------------------------------------
   num_str2long : reads an expression of the type
                lvexpression  = ["+"|"-"] lvterm {"+"|"-" lvterm}
                 lvterm       = lvfactor {"*"|"/" lvfactor}
                  lvfactor    = lvnumber | lvconstant |"(" lvexpression ")"
                                   | dpfunction | "(long int)" dpfactor
                                   | "(round)" dpfactor1 | dpconstant
                   lvnumber   = long integer type number
                   lvconstant = "true" | "false" | "yes" | "no"
                   dpfunction = see above
                   dpfactor1  = see above
        
        The result of dpfunction is rounded to the closest long integer value.
  
        The conversion stops at at the end of the string, a white space, 
        a comma or a semicolon. If tail is not NULL the pointer to the 
        remaining string, including the terminating character, is returned
        in *tail.
        If the evaluated expression is not complete or faulty, an           
        error value is returned.
        
   	return long value  (o) : value of expression
        const char  *str   (i) : input string
              char **tail  (o) : pointer to rest string if tail!=NULL
        int * perrval      (o) : output status
                                 NumSuccess      : successful conversion
                                 NumBadParenthesis : wrong number of parentheses
                                 NumNoIntegerNumber : mysterious character found
  --------------------------------------------------------------------------*/
PUBLIC long num_str2long(const char *str, const char **tail, int *perrval)
{ 
  int errval;
  long int value;
  const char * ps="";

  if (str) ps = str;

  while (isspace(*ps)) ps++; // skip leading white space

  value = lvexpression( &ps, 0 , &errval);

  if (tail) *tail = ps;

  if (perrval) *perrval=errval;
  return( value );

} /* num_str2long */

/*--------------------------------------------------------------------------  
   num_long2str : writes a long value to buffer
  
        return char *      (o)   : pointer to buffer, 
                                   char * NULL in case of an error
        unsigned long buflen (i) : buffer length (includes terminating NULL)
        long value          (i) : value to write 
        int * perrval      (o)  : 0 Success, otherwise error
  --------------------------------------------------------------------------*/
PUBLIC char *num_long2str( char buffer[], unsigned long buflen,
                           long value, int * perrval )
{
  int errval;
  char tmp[128];
  
  errval = NumWriteError;

  if ( sprintf(tmp,"%ld", value ) < 1 ) goto num_long2str_error;
  strncpy( buffer, tmp, buflen-1 );
  buffer[buflen-1] = '\0';

  errval = NumSuccess;

  if (perrval) *perrval=errval;
  return( buffer );

num_long2str_error:

  if (perrval) *perrval=errval;
  return( (char *) NULL );

} /* num_long2str */

/*--------------------------------------------------------------------------
   num_long2hex : writes a long value hexadecimal to buffer

        return char *      (o)   : pointer to buffer,
                                   char * NULL in case of an error
        unsigned long buflen (i) : buffer length (includes terminating NULL)
        long value          (i) : value to write
        int * perrval      (o)  : 0 Success, otherwise error
  --------------------------------------------------------------------------*/
PUBLIC char *num_long2hex( char buffer[], unsigned long buflen,
                           long value, int * perrval )
{
  int errval;
  char tmp[128];

  errval = NumWriteError;

  if ( sprintf(tmp,"0x%lx", value ) < 1 ) goto num_long2hex_error;
  strncpy( buffer, tmp, buflen-1 );
  buffer[buflen-1] = '\0';

  errval = NumSuccess;

  if (perrval) *perrval=errval;
  return( buffer );

num_long2hex_error:

  if (perrval) *perrval=errval;
  return( (char *) NULL );


} /* num_long2hex */

/*---------------------------------------------------------------------------
double2s( buffer, value, ndigits )
Conversion of double to string and output to buffer. The pointer to
buffer is returned. In case of an error the null pointer is returned.
The length of buffer must be 32 or larger.
Only the absolute value of ndigits is used.
---------------------------------------------------------------------------*/
char * double2s( char buffer[], double value, int ndigits )
# define FORMAT_LEN 20
{ char format[FORMAT_LEN];

  if (ndigits<0) ndigits=-ndigits;
  if (ndigits>80) ndigits=80;

  if (ndigits==0) sprintf(format,"%%lg");
  else sprintf(format,"%%.%dlg",ndigits);

  if ( sprintf(buffer,format,value)<1) return((char *) NULL);

  return(buffer);

} /* double2s */

/*--------------------------------------------------------------------------  
   num_double2str : writes a float value with unit into buffer
                    The input value must have a normalized form, e.g. it
                    must be given in meters, rad, seconds or Joule.
                    The value is expressed relative to uniti,
                    i.e. if value is 1 and unit is "mm" the
                    output string will be "1000_mm" = (1/mm)_mm. 
  
        return char *      (o)   : pointer to buffer,
                                   char * NULL in case of an error
        unsigned long buflen (i) : buffer length (includes terminating NULL)
        double value         (i) : value to write
        const char * unit    (i) : unit to use or empty string or NULL
        int ndigits          (i) : number of digits to write
        int * perrval       (o)  : 0 Success, otherwise error
  --------------------------------------------------------------------------*/
PUBLIC char *num_double2str( char buffer[], unsigned long buflen,
                             double value, const char * unit, int ndigits,
                             int * perrval )
{
  int errval;
  char tmp[128], *tmp_unit;
  double val, unit_val, tmp_unit_val;

  if ((unit) && (strlen(unit)>0)) {
    // get unit
    unit_val = num_str2double( unit, NULL, &errval );
    if (errval) goto num_double2str_error;
    
    errval = NumDivByZero; 
    if ( unit_val == 0.0 ) goto num_double2str_error; 
    val = value/unit_val;

    // test unit (0 multiplied with unit)
    errval = NumMemoryAllocationError;
    tmp_unit = (char *) malloc( sizeof(char)*(strlen(unit)+3) );
    if (!tmp_unit) goto num_double2str_error;

    sprintf(tmp_unit,"0_%s",unit);
    tmp_unit_val = num_str2double( tmp_unit, NULL, &errval );
    free(tmp_unit);
    if (errval) goto num_double2str_error;

    // write val to tmp
    errval = NumWriteError;
    if ( !double2s( tmp, val, ndigits ) ) goto num_double2str_error;

    // copy tmp and unit to buffer
    errval = NumWriteError;
    if (tmp_unit_val == 0.0 ) {
      if ( buflen < ( strlen(tmp)+strlen(unit)+2 ) ) goto num_double2str_error;
      if ( sprintf(buffer,"%s_%s", tmp, unit ) < 2 ) goto num_double2str_error;
    } else {
      if ( buflen < ( strlen(tmp)+strlen(unit)+4 ) ) goto num_double2str_error;
      if ( sprintf(buffer,"%s_(%s)", tmp,unit ) < 2 ) goto num_double2str_error;
    }
  } else {
    // write value to tmp with ndigits
    errval = NumWriteError;
    if ( !double2s( tmp, value, ndigits ) ) goto num_double2str_error;

    // copy tmp to buffer
    strncpy( buffer, tmp, buflen-1 );
    buffer[buflen-1] = '\0';
  }

  errval = NumSuccess;

  if (perrval) *perrval=errval;
  return( buffer );

num_double2str_error:

  if (perrval) *perrval=errval;
  return( (char *) NULL );

} /* num_double2str */

/*--------------------------------------------------------------------------
   num_double2hex : rounds a double value and writes it hexadecimal to buffer
                    (without unit, without decimals)

                    If ndigits is negative it is only used for negative
                    values.

        return char *      (o)   : pointer to buffer,
                                   char * NULL in case of an error
        unsigned long buflen (i) : buffer length (includes terminating NULL)
        double value          (i) : value to write
        int ndigits          (i) : number of digits to write
        int * perrval      (o)  : 0 Success, otherwise error
  --------------------------------------------------------------------------*/
PUBLIC char *num_double2hex( char buffer[], unsigned long buflen,
                             double value, int ndigits, int * perrval )
{
  int errval;

  char *ps;
  double nhex, hex, rest, m;
  double base=16.0;
  int sign;

  errval = NumWriteError;

  if (buflen<4) goto num_double2hex_error; // too short for "0x0\n"

  ps = &(buffer[0]);

  *ps='0';ps++;
  *ps='x';ps++;

  if (value<0.0) {
    // use complement
    rest = -(value+1.0); sign=-1;
  } else {
    rest = value; sign=+1.0;
  }

  if (rest>0.0) 
    hex = pow(base,floor(log(rest)/log(base)));
  else hex = 1.0;

  if ((value<0.0)||(ndigits>0)) {
    if (fabs(ndigits)>1) nhex = pow(base,fabs(ndigits)-1.0);
    else nhex = 1.0;
    if (hex<nhex) hex=nhex;
  }

  while ((hex>=1.0)&&(ps<buffer+buflen-1)) {
    m = floor(rest/hex);
    rest -= m*hex ;
    if (sign<0) m = base-m-1.0; // use complement
    if ((0.0<=m)&&(m<=base)) {
      switch ( (int) m ) {
        case 0: *ps = '0'; break;
        case 1: *ps = '1'; break;
        case 2: *ps = '2'; break;
        case 3: *ps = '3'; break;
        case 4: *ps = '4'; break;
        case 5: *ps = '5'; break;
        case 6: *ps = '6'; break;
        case 7: *ps = '7'; break;
        case 8: *ps = '8'; break;
        case 9: *ps = '9'; break;
        case 10: *ps = 'a'; break;
        case 11: *ps = 'b'; break;
        case 12: *ps = 'c'; break;
        case 13: *ps = 'd'; break;
        case 14: *ps = 'e'; break;
        case 15: *ps = 'f'; break;
      }
      ps++;
    } else break;
    hex /= base;
  }

  *ps = '\0'; 

  errval = NumSuccess;

  if (perrval) *perrval = errval;
  return( buffer );

num_double2hex_error:

  if (perrval) *perrval = errval;
  return( (char *) NULL );

} /* num_double2hex */

/*--------------------------------------------------------------------------  
   num_errval2str : writes error message into a buffer
  
        char buffer[]        (i) : output buffer for message
        unsigned long buflen (i) : length of buffer (including terminating NULL)
        int errval           (i) : error message to write
        return char *        (o) : pointer to buffer 
  
        NumSuccess, NumNoFloatNumber, NumDomainError, NumCommaExpected
        NumBadParenthesis, NumDivByZero, NumMemoryAllocationError
        NumIntegerOverflow, NumNoIntegerNumber, NumNoFloatFunction,
        NumScanError
  --------------------------------------------------------------------------*/
PUBLIC char *num_errval2str( char buffer[], unsigned long buflen, int errval )
{
  char * value;

  value=buffer;

  if ( (buffer)&&(buflen>0) ) {
    switch (errval) {
      case NumSuccess : 
             strncpy(buffer,"success",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumMemoryAllocationError : 
             strncpy(buffer,"memory allocation failed",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumScanError : 
             strncpy(buffer,"error scanning expression",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumCommaExpected : 
             strncpy(buffer,"missing comma in expression",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumBadParenthesis : 
             strncpy(buffer,"bad parenthesis in expression",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumNoFloatNumber : 
             strncpy(buffer,"expression is not a float number",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumNoFloatFunction : 
             strncpy(buffer,"unknown float function in expression",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumDomainError : 
             strncpy(buffer,"domain error",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumNoIntegerNumber : 
             strncpy(buffer,"expression is not an integer number",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumIntegerOverflow : 
             strncpy(buffer,"integer overflow",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumDivByZero : 
             strncpy(buffer,"division by zero",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumWriteError :
             strncpy(buffer,"error writing value",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumProgramError :
             strncpy(buffer,"error creating program",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumNoVariable :
             strncpy(buffer,"undefined variable",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumNoInstruction :
             strncpy(buffer,"unknown program instruction",buflen-1);
             buffer[buflen-1]='\0';
             break;
      case NumNoAccumulator :
             strncpy(buffer,"not enough program registers",buflen-1);
             buffer[buflen-1]='\0';
             break;
  
      default:
             strncpy(buffer,"unknown error value",buflen-1);
             buffer[buflen-1]='\0';

    } // switch
  } else value=NULL;

  return( value );

} /* num_errval2str */

/*---------------------------------------------------------------------------
NAME

        num_strncasecmp ---  strncasecmp

SYNOPSIS

       int num_strncasecmp(const char *s1, const char *s2, size_t n);


DESCRIPTION
       The function compares the first n characters ot the two
       strings s1  and s2,  ignoring  the  case of the characters.
       It returns an integer less than, equal to, or greater than
       zero if s1 is less  than, to matches, or is greater than s2.
       It can be used instead of the function strncasecmp if this 
       function is not available.

RETURN VALUE
       The function returns an integer less than, equal to, or
       greater than  zero.

---------------------------------------------------------------------------*/
int num_strncasecmp(const char *s1, const char *s2, size_t n)
{ int value;
  size_t s1len, s2len;
  char *_s1, *_s2;
  register unsigned int i;

  s1len = strlen(s1);
  s1len = (s1len<n)?s1len:n;
  _s1 = (char *) malloc( s1len+1 );
  for (i=0;i<s1len;i++) _s1[i]=(char)tolower(s1[i]);
  _s1[s1len]='\0';

  s2len = strlen(s2);
  s2len = (s2len<n)?s2len:n;
  _s2 = (char *) malloc( s2len+1 );
  for (i=0;i<s2len;i++) _s2[i]=(char)tolower(s2[i]);
  _s2[s2len]='\0';

  value = strcmp(_s1,_s2);

  free(_s2);
  free(_s1);

  return(value);

} /* num_strncasecmp */

/*--------------------------------------------------------------------------*/
/* char *numio_version : returns pointer to the version string              */
/*--------------------------------------------------------------------------*/
PUBLIC char *numio_version ( void )
{ return ( NUMIO_VERSION );
} /* numio_version */

/*--------------------------------------------------------------------------*/
/* void numio_debug : selects debug mode 0|1|2                              */
/*   0: off, 1: show constant replacement, 2: + show initialization         */
/*--------------------------------------------------------------------------*/
PUBLIC extern void numio_debug ( int debug )
{ NUMIO_debug = debug;
} /* numio_debug */

/****************************************************************************/

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

    numio.h

SYNOPSIS

    #include "numio.h"

DESCRIPTION
    Header of the module "numio.c"

***********************************************************************---*/
#ifndef _numio_
# define _numio_

/***************************************************************************
* General Definitions                                                      *
***************************************************************************/

#ifndef PRIVATE
#  define PRIVATE static // used to declare variables of private type 
#endif

#ifndef PUBLIC
#  define PUBLIC         // used to declare variables of public type 
#endif

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

# include "gamma.h"

/******************************************************************************
* Constants                                                                   *
******************************************************************************/

# define NUM_PI        3.1415926535897932384626

/****************************************************************************
* Number Program List Structure Definition                                  *
****************************************************************************/
typedef struct Num_Var {
  char * Key;                            /* Pointer to variable key string */
  double Value;                                   /* Value of the variable */
  int    Used;                                           /* number of uses */
  struct Num_Var *Previous,*Next;                  /* prev./next variables */
} NumVar;
 
typedef struct Num_Accu {
  long   Number;                                     /* Accumulator number */
  double Value;                                /* Value of the accumulator */
  struct Num_Accu *Previous,*Next;              /* prev./next accumulators */
} NumAccu;
 
typedef struct Num_Instr {
  int    Command;                                               /* Command */
  int    Nargs;                           /* Number of required parameters */
  double Value;                                                   /* Value */
  double *Address;                                              /* Address */
  struct Num_Instr *Previous,*Next;                 /*prev./next instructs.*/
} NumInstr;
 
typedef struct Num_Prog {
  char * Name;                                   /* pointer to name string */
  NumVar * VariableList;                              /* list of variables */
  NumAccu * AccumulatorList;                       /* list of accumulators */
  NumAccu * CurrentAccumulator;         /* pointer to current accumulators */
  NumInstr * InstructionList;                      /* list of instructions */
  NumInstr * CompiledList;                /* list of compiled instructions */
  struct Num_Prog *Previous,*Next;            /* previous and next program */
} NumProg;

/***************************************************************************
* Functions                                                                *
***************************************************************************/

PUBLIC extern long       num_str2long ( const char *str, const char **tail, 
                                        int *perrval);

PUBLIC extern double   num_str2double ( const char *str, const char **tail, 
                                        int *perrval);

PUBLIC extern char      *num_long2str ( char buffer[], unsigned long buflen,
                                        long value, int * perrval );

PUBLIC extern char      *num_long2hex ( char buffer[], unsigned long buflen,
                                        long value, int * perrval );
 
PUBLIC extern char    *num_double2str ( char buffer[], unsigned long buflen,
                                        double value, const char * unit,
                                        int ndigits, int * perrval );

PUBLIC extern char     *num_double2hex( char buffer[], unsigned long buflen,
                                        double value, int ndigits, 
                                        int * perrval );
 
PUBLIC extern NumProg   *num_str2prog ( const char *name,
                                        const char *str, const char **tail, 
                                        int *perrval, int nvar, ... );

PUBLIC extern int          num_chkvar ( NumProg * program, int n, 
                                        int *perrval );

PUBLIC extern double      num_runprog ( NumProg * program, int *perrval,
                                        ... );

PUBLIC extern NumProg *num_searchprog ( const char *name, int *perrval );

PUBLIC extern int          num_rmprog ( NumProg * program, int *perrval );

PUBLIC extern char    *num_errval2str ( char buffer[], unsigned long buflen, 
                                        int errval );

PUBLIC extern int num_strncasecmp(const char *s1, const char *s2, size_t n);

PUBLIC extern void numio_debug ( int debug );

PUBLIC extern char *numio_version ( void );

PUBLIC extern long num_prog_variables             ( NumProg *program );

PUBLIC extern long num_prog_accumulators          ( NumProg *program );

PUBLIC extern long num_prog_instructions          ( NumProg *program, int mode);

PUBLIC extern size_t num_prog_variable_size       ( NumProg *program );

PUBLIC extern size_t num_prog_accumulator_size    ( NumProg *program );

PUBLIC extern size_t num_prog_instruction_size    ( NumProg *program, int mode);

PUBLIC extern size_t num_prog_size                ( NumProg * program );

PUBLIC extern size_t num_prog_size_all            ( void );

PUBLIC extern int num_prog_print_variable_list    ( FILE * out, 
                                                   NumProg *program,
                                                   int level, int verbose );

PUBLIC extern int num_prog_print_accumulator_list ( FILE * out, 
                                                   NumProg *program,
                                                   int level, int verbose );

PUBLIC extern int num_prog_print_instruction_list ( FILE * out, 
                                                   NumProg *program, int mode, 
                                                   int level, int verbose );

PUBLIC extern int num_prog_print_list             ( FILE * out, 
                                                    NumProg * program,
                                                    int level, int verbose );

/***************************************************************************
* Error Values                                                             *
***************************************************************************/

# define NumSuccess                0
# define NumMemoryAllocationError  1
# define NumScanError              2 
# define NumCommaExpected          3 
# define NumBadParenthesis         4 
# define NumNoFloatNumber          5
# define NumNoFloatFunction        6
# define NumDomainError            7
# define NumNoIntegerNumber        8
# define NumIntegerOverflow        9 
# define NumDivByZero             10 
# define NumWriteError            11 
# define NumNoVariable            12
# define NumVariableError         13
# define NumProgramError          14 
# define NumNoInstruction         15
# define NumNoAccumulator         16

#endif


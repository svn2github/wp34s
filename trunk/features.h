/* This file is part of 34S.
 * 
 * 34S is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * 34S is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with 34S.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FEATURES_H__
#define FEATURES_H__

#if !defined(REALBUILD) && !defined(WINGUI) && !defined(QTGUI)
#define CONSOLE
#endif

/*
 *  Select optional features here
 */

// Allow for any generic argument taking commands in XROM
// #define XROM_RARG_COMMANDS

// Define this to support a STOPWATCH function like the StopWatch on the HP-41C
// Time Module or the HP-55
#if !defined(REALBUILD) || (defined(XTAL) /* && !defined(INFRARED) */)
#define INCLUDE_STOPWATCH
#else
//#define INCLUDE_STOPWATCH
#endif

// Build a tiny version of the device
// #define TINY_BUILD

// Include a catalogue of the internal commands
#define INCLUDE_INTERNAL_CATALOGUE

// Include a mechanism for a user defined catalogue
#define INCLUDE_USER_CATALOGUE

// Include the CNSTS command to access constants via indirection
#define INCLUDE_INDIRECT_CONSTS

// Code to allow access to caller's local data from xIN-code
// #define ENABLE_COPYLOCALS

#ifndef TINY_BUILD

// Include the Mantissa and exponent function
// Space cost is approximately 180 bytes
#define INCLUDE_MANTISSA

// Include the xroot function for reals, integers and complex numbers
// Space cost is approximately 400 bytes
#define INCLUDE_XROOT

// Include the user mode serial commands SOPEN, SCLOSE, RECV1, SEND1, aRECV, aSEND
// Space cost approximately 700 bytes.
// #define INCLUDE_USER_IO

// Include the SAVEM/RESTM user mode save and restore commands
#define INCLUDE_USER_MODE

// Include the Gudermannian functions and their inverses in the real
// and complex domain.
#define INCLUDE_GUDERMANNIAN

// Include first and second order Bessel functions Jn, In, Yn and Kn for
// both real and complex arguments.  These are implemented in XROM.
// #define INCLUDE_XROM_BESSEL

// Inlcude real and complex flavours of the digamma function.  These are
// implemented in XROM.  THe first setting is sufficient for accuracy for
// single precision, the second needs to be enabled as well to get good
// results for double precision..
// #define INCLUDE_XROM_DIGAMMA
// #define XROM_DIGAMMA_DOUBLE_PRECISION

// Include a fused multiply add instruction
// This isn't vital since this can be done using a complex addition.
// Space cost 108 bytes.
// #define INCLUDE_MULADD

// Include a date function to determine the date of Easter in a given year
//#define INCLUDE_EASTER

// Include code to use a Ridder's method step after a bisection in the solver.
// For some functions this seems to help a lot, for others there is limited
// benefit.
#define USE_RIDDERS

// Include code to find integer factors
// Space cost 480 bytes.
// #define INCLUDE_FACTOR

// Include matrix functions better implemented in user code
// #define SILLY_MATRIX_SUPPORT

// Include matrix row/row operations.
// M.R<->, M.R*, M.R+
#define MATRIX_ROWOPS

// Include the LU decomposition as a user command
// M.LU
#define MATRIX_LU_DECOMP

// Include fast path code to calculate factorials and gamma functions
// for positive integers using a string of multiplications.
#define GAMMA_FAST_INTEGERS

// Include the flash register recall routines RCF and their variants
// #define INCLUDE_FLASH_RECALL

// Include iBACK, etc. as user visible commands
// #define INCLUDE_INDIRECT_BRANCHES

// Include the upper tail cumulative distribution functions
#define INCLUDE_CDFU

/*******************************************************************/
/* Below here are the automatic defines depending on other defines */
/*******************************************************************/

#if defined(INCLUDE_COMPLEX_ZETA) && ! defined(INCLUDE_ZETA)
/* Complex zeta implies real zeta */
#define INCLUDE_ZETA
#endif

#if defined(INCLUDE_BERNOULLI) && ! defined(INCLUDE_ZETA)
/* Bernoulli numbers need real zeta */
#define INCLUDE_ZETA
#endif

#if defined(XROM_DIGAMMA_DOUBLE_PRECISION) && ! defined(INCLUDE_XROM_DIGAMMA)
/* Accurate digamma needs normal digamma */
#define INCLUDE_XROM_DIGAMMA
#endif

#endif  /* TINY_BUILD*/
#endif  /* FEATURES_H__ */

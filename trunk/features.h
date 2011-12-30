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
// Enable double precision mode
#define INCLUDE_DOUBLE_PRECISION

#if !defined(REALBUILD) && !defined(POST_PROCESSING)
// Push commands into XROM instead of being in C
// postpone this define for the real thing until xIN and xOUT are working
#define XROM_COMMANDS
#endif

// Define this to support a STOPWATCH function like the StopWatch on the HP-41C
// Time Module or the HP-55
// #define INCLUDE_STOPWATCH
#ifdef INCLUDE_STOPWATCH
// Define this to activate directly the STOPWATCH function by pressing the F then
// G prefixes quickly
#define INCLUDE_STOPWATCH_HOTKEY
#endif

// Build a tiny version of the device
// #define TINY_BUILD

// Include a catalogue of the internal commands
//#define INCLUDE_INTERNAL_CATALOGUE

#ifndef TINY_BUILD

// Include the Mantissa and exponent function
// Space cost is approximately 180 bytes
#define INCLUDE_MANTISSA

// Include the xroot function for reals, integers and complex numbers
// Space cost is approximately 400 bytes
#define INCLUDE_XROOT

// Include Rieman's Zeta function
// There is an XROM implementation of these now that is much smaller.
//
// The complex version isn't accurate for large imaginary numbers
// and the series approximations I've located thus far converge too slowly.
// #define INCLUDE_ZETA
// #define INCLUDE_COMPLEX_ZETA

// Include Bernoulli numbers as functions
// #define INCLUDE_BERNOULLI

// Include the user mode serial commands SOPEN, SCLOSE, RECV1, SEND1, aRECV, aSEND
// Space cost approximately 700 bytes.
// #define INCLUDE_USER_IO

// Include the SAVEM/RESTM user mode save and restore commands
#define INCLUDE_USER_MODE

// Include the digamma function for real and complex arguments
// Space cost 1156 bytes
//#define INCLUDE_DIGAMMA

// Include Jacobi's Elliptical Functions: SN, CN & DN for real & complex arguments
// Space cost 1624 bytes
//#define INCLUDE_ELLIPTIC

// Include Bessel functions of first and second kind
// Complex versions aren't working properly yet.
// Space cost for real version only 1704 bytes
//#define INCLUDE_BESSEL
//#define COMPLEX_BESSEL

// Include a fused multiply add instruction
// This isn't vital since this can be done using a complex addition.
// Space cost 108 bytes.
// #define INCLUDE_MULADD

// Include the x!! function defined over the complex plane
// Space cost for this and !n 368 bytes.
//#define INCLUDE_DBLFACT

// Include the !n function defined over the reals (integers)
//#define INCLUDE_SUBFACT

// Include a date function to determine the date of Easter in a given year
//#define INCLUDE_EASTER

// Include code to use a Ridder's method step after a bisection in the solver.
// This increases code size and doesn't see to help the solver's convergence.
//#define USE_RIDDERS

// Include code to find integer factors
// Space cost 480 bytes.
// #define INCLUDE_FACTOR

// Define this to support HP16c integer/real mode conversions.
// The 16c breaks a real into a mantissa and base two exponent on entry
// to integer mode and reverses the process on exit.  It clears the rest of
// the stack.  The alternative (which is much smaller), converts the stack and
// last X on the mode switch (so e.g. 34 stays 34).
// #define HP16C_MODE_CHANGE

// Include some basic matrix support commands.
#define MATRIX_SUPPORT

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

/*******************************************************************/
/* Below here are the automatic defines depending on other defines */
/*******************************************************************/
#if defined(XROM_COMMANDS) && ! defined(INCLUDE_DOUBLE_PRECISION)
#define INCLUDE_DOUBLE_PRECISION
#endif

#if defined(INCLUDE_COMPLEX_ZETA) && ! defined(INCLUDE_ZETA)
/* Complex zeta implies real zeta */
#define INCLUDE_ZETA
#endif

#if defined(INCLUDE_BERNOULLI) && ! defined(INCLUDE_ZETA)
/* Bernoulli numbers need real zeta */
#define INCLUDE_ZETA
#endif

#if defined(COMPLEX_BESSEL) && ! defined(INCLUDE_BESSEL)
/* Complex bessel functions require real versions */
#define INCLUDE_BESSEL
#endif
#if defined(INCLUDE_BESSEL) && ! defined(INCLUDE_DIGAMMA)
/* Second kind functions of integer order need digamma */
#define INCLUDE_DIGAMMA
#endif

/* Check for matrix options that imply matrix itself */
#if defined(SILLY_MATRIX_SUPPORT) && ! defined(MATRIX_SUPPORT)
#define MATRIX_SUPPORT
#endif
#if defined(MATRIX_ROWOPS) && ! defined(MATRIX_SUPPORT)
#define MATRIX_SUPPORT
#endif
#if defined(MATRIX_LU_DECOMP) && ! defined(MATRIX_SUPPORT)
#define MATRIX_SUPPORT
#endif

#endif  /* TINY_BUILD*/
#endif  /* FEATURES_H__ */

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
/*
 *  Select optional features here
 */

// Define this to use inline quick check macros for determining
// a bit of information about decNumbers.  Enabling this will create
// larger possibly slightly faster code.
#define DECNUMBER_QUICK_MACROS

// Build a tiny version of the device
// #define TINY_BUILD

// Include a catalogue of the internal commands
#define INCLUDE_INTERNAL_CATALOGUE

#ifndef TINY_BUILD

// Include Rieman's Zeta function
// The complex version isn't accurate for large imaginary numbers
// and the series approximations I've located thus far converge too slowly.
#define INCLUDE_ZETA
// #define INCLUDE_COMPLEX_ZETA

// Include Bernoulli numbers as functions
#define INCLUDE_BERNOULLI


// Incude the digamma function for real and complex arguments
//#define INCLUDE_DIGAMMA

// Include Jacobi's Elliptical Functions: SN, CN & DN for real & complex arguments
//#define INCLUDE_ELLIPTIC

// Include Bessel functions of first and second kind
// Complex versions aren't working properly yet.
//#define INCLUDE_BESSEL
//#define COMPLEX_BESSEL

// Inlcude a fused multiply add instruction
// This isn't vital since this can be done using a complex addition.
//#define INCLUDE_MULADD

// Include the x!! function defined over the complex plane
//#define INCLUDE_DBLFACT

// Include the !n function defined over the reals (integers)
//#define INCLUDE_SUBFACT

// Include multi-character alpha constants (not keystroke programmable)
//#define MULTI_ALPHA

// Include a date function to determine the date of Easter in a given year
//#define INCLUDE_EASTER

// Include code to use a Ridder's method step after a bisection in the solver.
// This increases code size and doesn't see to help the colver's convergence.
//#define USE_RIDDERS

// include code to find integer factors
// #define INCLUDE_FACTOR


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

#if defined(COMPLEX_BESSEL) && ! defined(INCLUDE_BESSEL)
/* Complex bessel functions require real versions */
#define INCLUDE_BESSEL
#endif
#if defined(INCLUDE_BESSEL) && ! defined(INCLUDE_DIGAMMA)
/* Second kind functions of integer order need digamma */
#define INCLUDE_DIGAMMA
#endif

#endif	/* TINY_BUILD*/
#endif	/* FEATURES_H__ */

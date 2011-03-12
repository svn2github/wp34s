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

/*
 *  Select optional features here
 */

// Include a catalogue of the internal commands
#define INCLUDE_INTERNAL_CATALOGUE

// Include the aritmetic/geometric mean iteration
//#define INCLUDE_AGM

// Include Reiman's Zeta function for real and complex arguments
//#define INCLUDE_ZETA

// Incude the digamma function
//#define INCLUDE_DIGAMMA

// Include Jacobi's Elliptical Functions: SN, CN & DN for real & complex arguments
//#define INCLUDE_ELLIPTIC

// Include Bessel functions of first and second kind
// Second kind functions of integer order need digamma
//#define INCLUDE_BESSEL
//#define INCLUDE_DIGAMMA
//#define COMPLEX_BESSEL

// Inlcude a fused multiply add instruction
// This isn't vital since this can be done using a complex addition.
//#define INCLUDE_MULADD

// Include cube and cube root functions
//#define INCLUDE_CUBES

// Include the !! function defined over the complex plane
//#define INCLUDE_DBLFACT

// Include the !! function defined over the reals
//#define INCLUDE_SUBFACT

// Inlcude multi-character alpha constants (not keystroke programmable)
//#define MULTI_ALPHA

// Include a date function to determine the date of Easter in a given year
//#define INCLUDE_EASTER

// Include code to use a Ridder's method step after a bisection in the solver.
//#define USE_RIDDERS

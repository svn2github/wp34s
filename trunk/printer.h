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

#ifndef __PRINTER_H__
#define __PRINTER_H__

#include "xeq.h"

#define PRINTING_ANNUNCIATOR LIT_EQ

enum print_modes {
	PMODE_DEFAULT = 0,
	PMODE_GRAPHICS = 1,
	PMODE_SMALLGRAPHICS = 2,
	PMODE_SERIAL = 3
};

#define TRACE_DATA_ENTRY 0xffffffff

// User visible routines
extern int print( int c );
extern int print_advance( void );
extern int print_tab( unsigned int col );
extern int print_line( const char *buff, int with_lf );
extern int print_justified( const char *buff );
extern int print_reg( int reg, const char *label, int eq );
extern void print_trace( opcode op, int phase );
extern void print_program( enum nilop op );
extern void print_registers( enum nilop op );
extern void print_sigma( enum nilop op );
extern void print_alpha( enum nilop op );
extern void print_lf( enum nilop op );
extern void cmdprint( unsigned int arg, enum rarg op );
extern void cmdprintreg( unsigned int arg, enum rarg op );
extern void cmdprintmode( unsigned int arg, enum rarg op );

// Implemented by the hardware layer
extern int put_ir( int byte );

#ifdef REALBUILD
#define PRINT_DELAY 18	// 1.8 seconds
extern volatile SMALL_INT PrintDelay;
#endif
extern unsigned int PrinterColumn;

#ifndef INFRARED
#define print_trace( op, phase ) /**/
#endif

#endif

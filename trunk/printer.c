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

#ifdef INFRARED

#include "printer.h"
#include "xeq.h"
#include "serial.h"
#include "stats.h"

/*
 *  Print to IR or serial port, depending on the PMODE setting
 */
static void print( unsigned char c )
{
	if ( UState.print_mode == PMODE_SERIAL ) {
		open_port_default();
		if ( c == '\n' ) {
			put_byte( '\r' );
		}
		put_byte( c );
		close_port_reset_state();
	}
	else {
		put_ir( c );
	}
}

/*
 *  Print using character set translation
 */
static void print_char( unsigned char c )
{
	switch ( UState.print_mode ) {

	case PMODE_DEFAULT:
		if ( c >= ' ' && c <= 126 ) {
			put_ir( c );
			break;
		}
		// fall through
	case PMODE_GRAPHICS:
	case PMODE_SMALLGRAPHICS:
		// Spit out the character as a graphic
		// ...
		put_ir( '.' );
		break;

	case PMODE_SERIAL:
		print( c );
	}
}

/*
 *  Print a program listing
 */
void print_program( enum nilop op )
{

}

/*
 *  Print register data
 */
void print_registers( enum nilop op )
{

}

/*
 *  Print the statistical registers
 */
void print_sigma( enum nilop op )
{

}

/*
 *  Print a single byte from X
 */
void print_byte( enum nilop op )
{
	int sgn;
	const unsigned char byte = getX_int_sgn( &sgn ) & 0xff;

	print( byte );
}

/*
 *  Print the contents of the Alpha register, terminated by a LF
 */
void print_alpha( enum nilop op )
{
	const char *p;
	for ( p = Alpha; *p != '\0'; ++p ) {
		print_char( *p );
	}
	print( '\n' );
}

/*
 *  Send a LF to the printer
 */
void print_lf( enum nilop op )
{
	print( '\n' );
}

/*
 *  Print a named register
 */
void cmdprint( unsigned int arg, enum rarg op )
{

}

/*
 *  Set printing modes
 */
void cmdprintmode( unsigned int arg, enum rarg op )
{
	UState.print_mode = arg;
}

#ifndef REALBUILD
/*
 *  Simple emulation for debug purposes
 */
#include <stdio.h>
void put_ir( unsigned char c )
{
	static FILE *f;

	if ( f == NULL ) {
		f = fopen( "wp34s.ir", "wb" );
	}
	fputc( c, f );
	if ( c == 0x04 || c == '\n' ) {
		fflush( f );
	}
}
#endif

#endif

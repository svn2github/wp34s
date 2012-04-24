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
 *  Where will the next data be printed?
 *  Columns are in pixel units from 0 to 165
 */
unsigned char PrinterColumn;

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

static void advance( void )
{
	print( '\n' );
	PrinterColumn = 0;
#ifdef REALBUILD
	PrintDelay = PRINT_DELAY;
#endif
}

/*
 *  Wrap if line is full
 */
static void wrap( int width )
{
	if ( PrinterColumn + width >= 166 ) {
		advance();
	}
	PrinterColumn += width;
}

/*
 *  Print a complete line using character set translation
 */
static void print_line( const char *buff, int with_lf )
{
	const int mode = UState.print_mode;
	unsigned int c;
	unsigned short int posns[ 257 ];
	unsigned char pattern[ 6 ];	// rows
	unsigned char i, j, m, w = 0;

	// Import code from generated file font.c
	extern unsigned int charlengths( unsigned int c );
	extern void findlengths( unsigned short int posns[ 257 ], int smallp );
	extern void unpackchar( unsigned int c, unsigned char d[ 6 ], int smallp, 
				const unsigned short int posns[ 257 ] );
	extern const unsigned char printer_chars[ 31 + 129 ];

	// Determine character sizes and pointers
	findlengths( posns, mode == PMODE_SMALLGRAPHICS );

	// Print line
	while ( ( c = *( (const unsigned char *) buff++ ) ) != '\0' ) {

		w= 0;
		switch ( mode ) {

		case PMODE_DEFAULT:			// Mixed character and graphic printing
			i = c < ' ' ? printer_chars[ c - 1 ]
			  : c > 126 ? printer_chars[ c - 127 + 31 ]
			  : c;

			w = PrinterColumn == 0 ? 5 : 7;
			if ( i != 0 ) {
				wrap( w );
				put_ir( i );
				break;
			}
			goto graphic_print;

		case PMODE_SMALLGRAPHICS:		// Smalll font
			c += 256;

		case PMODE_GRAPHICS:			// Standard font
		graphic_print:
			// Spit out the character as a graphic
			unpackchar( c, pattern, mode == PMODE_SMALLGRAPHICS, posns );
			if ( w == 0 ) {
				w = charlengths( c );
			}
			wrap( w );
			put_ir( 27 );
			put_ir( w );
			if ( w == 7 ) {
				// Add spacing between characters
				put_ir( 0 );
				put_ir( 0 );
				w = 5;
			}
			// Transpose the pattern
			m = 1;
			for ( i = 0; i < w; ++i ) {
				c = 0;
				for ( j = 0; j < 6; ++j ) {
					if ( pattern[ j ] & m ) {
						c |= ( 2 << j );
					}
				}
				put_ir( c );
				m <<= 1;
			}
			break;

		case PMODE_SERIAL:
			print( c );
		}
	}
	if ( with_lf ) {
		advance();
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
 *  Print the contents of the Alpha register, terminated by a LF
 */
void print_alpha( enum nilop op )
{
	print_line( Alpha, op == OP_PRINT_ALPHA );
}

/*
 *  Send a LF to the printer
 */
void print_lf( enum nilop op )
{
	advance();
}

/*
 *  Print a single character or control code
 */
void cmdprint( unsigned int arg, enum rarg op )
{
	char buff[ 2 ];
	switch ( op ) {

	case RARG_PRINT_BYTE:
		// Transparent printing of bytes
		print( arg );
		break;
	
	case RARG_PRINT_CHAR:
		// Character printing, depending on mode
		buff[ 0 ] = (char) arg;
		buff[ 1 ] = '\0';
		print_line( buff, 0 );
		break;

	case RARG_PRINT_TAB:
		// Move to specific column
		if ( PrinterColumn < arg ) {
			int i = arg - PrinterColumn;
			PrinterColumn = arg;
			put_ir( 27 );
			put_ir( i );
			while ( i-- )
				put_ir( 0 );
		}
		break;

	default:
		break;
	}
}

/*
 *  Print a named register
 */
void cmdprintreg( unsigned int arg, enum rarg op )
{

}

/*
 *  Set printing modes
 */
void cmdprintmode( unsigned int arg, enum rarg op )
{
	UState.print_mode = arg;
}

#ifdef WINGUI
/*
 *  Send printer output to emulated HP 82440B by Christoph Gieselink
 */
#undef State
#undef Alpha
#define shutdown shutdown_socket

#include <winsock.h>
#define UDPPORT 5025
#define UDPHOST "127.0.0.1"

void put_ir( unsigned char c )
{
	int s;
	WSADATA ws;
	struct sockaddr_in sa;

	WSAStartup( 0x0101, &ws );

	sa.sin_family = AF_INET;
	sa.sin_port = htons( UDPPORT );
	sa.sin_addr.s_addr = inet_addr( UDPHOST );

	s = socket( AF_INET, SOCK_DGRAM, 0 );
	sendto( s, (const char *) &c, 1, 0, (struct sockaddr *) &sa, sizeof( struct sockaddr_in ) );
	closesocket( s );
}

#elif !defined(REALBUILD)
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


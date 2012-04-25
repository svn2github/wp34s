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
#include "display.h"

#define SERIAL_LINE_DELAY 3

/*
 *  Where will the next data be printed?
 *  Columns are in pixel units from 0 to 165
 */
unsigned char PrinterColumn;

/*
 *  Print to IR or serial port, depending on the PMODE setting
 */
static int print( unsigned char c )
{
	if ( UState.print_mode == PMODE_SERIAL ) {
		int abort = 0;
		open_port_default();
		if ( c == '\n' ) {
			put_byte( '\r' );
			put_byte( '\n' );
			abort = recv_byte( SERIAL_LINE_DELAY ) == R_BREAK;
		}
		else {
			put_byte( c );
		}
		close_port_reset_state();
		return abort;
	}
	else {
		return put_ir( c );
	}
}

static int advance( void )
{
	int abort = print( '\n' );
	PrinterColumn = 0;
#ifdef REALBUILD
	PrintDelay = State.print_delay;
#endif
	return abort;
}

/*
 *  Print a graphic sequence
 */
static int print_graphic( int glen, unsigned char *graphic )
{
	if ( glen > 0 ) {
		if ( put_ir( 27 ) ) {
			return 1;
		}
		put_ir( glen );
		while ( glen-- ) {
			put_ir( *graphic++ );
		}
	}
	return 0;
}

/*
 *  Wrap if line is full
 */
static int wrap( int width )
{
	if ( PrinterColumn + width > 166 ) {
		if ( advance() ) {
			return 1;
		}
		if ( width == 7 ) {
			width = 5;
		}
	}
	PrinterColumn += width;
	return 0;
}

/*
 *  Print a complete line using character set translation
 */
static int print_line( const char *buff, int with_lf )
{
	const int mode = UState.print_mode;
	unsigned int c;
	unsigned short int posns[ 257 ];
	unsigned char pattern[ 6 ];	// Rows
	unsigned char graphic[ 166 ];	// Columns
	unsigned char glen = 0;
	unsigned char i, j, m, w = 0;
	int abort = 0;

	// Import code from generated file font.c
	extern const unsigned char printer_chars[ 31 + 129 ];

	// Determine character sizes and pointers
	findlengths( posns, mode == PMODE_SMALLGRAPHICS );

	// Print line
	while ( ( c = *( (const unsigned char *) buff++ ) ) != '\0' && !abort ) {

		w= 0;
		switch ( mode ) {

		case PMODE_DEFAULT:			// Mixed character and graphic printing
			if ( c == 006 && *buff == 006 ) {
				// merge small spaces
				continue;
			}
			i = c < ' ' ? printer_chars[ c - 1 ]
			  : c > 126 ? printer_chars[ c - 127 + 31 ]
			  : c;

			if ( i != 0 ) {
				// Use printer character set
#ifdef WINGUI
				w = PrinterColumn == 0 ? 5 : 7;
#else
				w = PrinterColumn == 0 || PrinterColumn == 160 ? 6 : 7;
#endif		
				abort = print_graphic( glen, graphic );
				glen = 0;
				abort |= wrap( w );
				abort |= put_ir( i );
			}
			else {
				// graphic printing of characters unknown to the printer
#ifdef WINGUI
				w = 5;
#else
				w = 6;
#endif
				if ( PrinterColumn > 0 && PrinterColumn < 166 ) {
					// Add horizontal spacing
					graphic[ glen++ ] = 0;
					++PrinterColumn;
#ifdef WINGUI
					graphic[ glen++ ] = 0;
					++PrinterColumn;
#else
					if ( PrinterColumn == 161 ) {
						w = 5;
					}
#endif
				}
				goto graphic_print;
			}
			break;

		case PMODE_SMALLGRAPHICS:		// Smalll font
			c += 256;

		case PMODE_GRAPHICS:			// Standard font
		graphic_print:
			// Spit out the character as a graphic
			unpackchar( c, pattern, mode == PMODE_SMALLGRAPHICS, posns );
			if ( w == 0 ) {
				w = charlengths( c );
				if ( PrinterColumn + w == 167 ) {
					// drop last column on last character in line
					--w;
				}
			}
			if ( PrinterColumn + w > 166 ) {
				abort = print_graphic( glen, graphic );
				glen = 0;
			}
			abort |= wrap( w );
			// Transpose the pattern
			m = 1;
			for ( i = 0; i < w; ++i ) {
				c = 0;
				for ( j = 0; j < 6; ++j ) {
					if ( pattern[ j ] & m ) {
						c |= ( 2 << j );
					}
				}
				graphic[ glen++ ] = c;
				m <<= 1;
			}
			break;

		case PMODE_SERIAL:
			print( c );
		}
	}
	abort |= print_graphic( glen, graphic );
	if ( with_lf ) {
		abort |= advance();
	}
	return abort;
}

/*
 *  Print a single register
 */
int print_reg( int reg, const char *label )
{
	char buf[ 65 ];
	int abort = 0;
	int len;
	const int pmode = UState.print_mode;

	if ( label != NULL ) {
		abort = print_line( label, 0 );
	}
	xset( buf, '\0', sizeof( buf ) );
	format_reg( reg, buf );
	len = pmode == PMODE_DEFAULT ? slen( buf ) * 7
	    : pmode == PMODE_SERIAL  ? 0
	    : pixel_length( buf, pmode == PMODE_SMALLGRAPHICS );

	if ( len >= 166 ) {
		len = 166;
	}
	if ( len ) {
		cmdprint( 166 - len, RARG_PRINT_TAB );
	}
	abort |= print_line( buf, 1 );
	return abort;
}	


/*
 *  Print a block of registers with labels
 */
void print_registers( enum nilop op )
{
	int s, n;

	if ( op == OP_PRINT_STACK ) {
		s = regX_idx;
		n = stack_size();
	}
	else {
		if ( reg_decode( &s, &n, NULL, 0 ) ) {
			return;
		}
	}

	while ( n-- ) {
		int r = s;
		char name[ 6 ], *p = name;

		if ( r >= regX_idx && r <= regK_idx ) {
			*p++ = REGNAMES[ r - regX_idx ];
		}
		else {
			if ( r > LOCAL_REG_BASE ) {
				*p++ = '.';
				r -= LOCAL_REG_BASE;
				if ( r >= 100 ) {
					*p++ = '1';
					r -= 100;
				}
			}
			p = num_arg_0( p, r, 2 );
		}
		*p++ = '=';
		*p = '\0';
		if ( 0 != print_reg( s++, name ) ) {
			return;
		}
	}
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
		if ( PrinterColumn > arg ) {
			advance();
		}
		if ( PrinterColumn < arg ) {
			int i = arg - PrinterColumn;
			int j = i / 7;
			i %= 7;
			PrinterColumn = arg;
			if ( i ) {
				put_ir( 27 );
				put_ir( i );
				while ( i-- )
					put_ir( 0 );
			}
			while ( j-- )
				put_ir( ' ' );
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
	print_reg( arg, NULL );
}


/*
 *  Set printing modes
 */
void cmdprintmode( unsigned int arg, enum rarg op )
{
	if ( op == RARG_PMODE ) {
		UState.print_mode = arg;
	}
	else {
		State.print_delay = arg;
	}
}


/*
 *  Trace an instruction
 */
void print_trace( opcode op )
{
	char buffer[ 16 ];
	const char *p = NULL;

	if ( get_user_flag( regT_idx ) ) {
		if ( op != 0xffffffff ) {
			p = prt( op, buffer );
		}
		print_reg( regX_idx, p );
	}
}


/*
 *  Print a program listing
 */
void print_program( enum nilop op )
{

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

int put_ir( unsigned char c )
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
	return 0;
}

#elif !defined(REALBUILD)
/*
 *  Simple emulation for debug purposes
 */
#include <stdio.h>
int put_ir( unsigned char c )
{
	static FILE *f;

	if ( f == NULL ) {
		f = fopen( "wp34s.ir", "wb" );
	}
	fputc( c, f );
	if ( c == 0x04 || c == '\n' ) {
		fflush( f );
	}
	return 0;
}
#endif

#endif


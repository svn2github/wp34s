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
#include "complex.h"

#define SERIAL_LINE_DELAY 3

/*
 *  Where will the next data be printed?
 *  Columns are in pixel units from 0 to 165
 */
unsigned int PrinterColumn;

/*
 *  Print to IR or serial port, depending on the PMODE setting
 */
int print( int c )
{
	const int mode = UState.print_mode;

	if ( mode == PMODE_SERIAL ) {
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
		if ( c == '\n' && ( mode == PMODE_GRAPHICS || mode == PMODE_SMALLGRAPHICS ) ) {
			// better LF for graphics printing
			return put_ir( 0x04 );
		}
		return put_ir( c );
	}
}


/*
 *  New line
 */
int print_advance( void )
{
	int abort;
	PrinterColumn = 0;
	abort = print( '\n' );
#ifdef REALBUILD
	PrintDelay = State.print_delay;
#endif
	return abort;
}


/*
 *  New line if tracing is active
 */
static int advance_if_trace()
{
	if ( PrinterColumn != 0 && Tracing ) {
		return print_advance();
	}
	return 0;
}


/*
 *  Move to column
 */
int print_tab( unsigned int col )
{
	int abort = 0;
	if ( PrinterColumn > col ) {
		abort = print_advance();
	}
	if ( !abort && PrinterColumn < col ) {
		int i = col - PrinterColumn;
		int j = i / 7;
		i %= 7;
		PrinterColumn = col;
		if ( i ) {
			put_ir( 27 );
			put_ir( i );
			while ( i-- )
				put_ir( 0 );
		}
		while ( j-- )
			put_ir( ' ' );
	}
	return abort;
}


/* Print a buffer from the given tab position and add a new line
 */
static void print_string_from_tab(const char *s, int tab)
{
	print_tab(tab);
	print_line(s, 1);
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
 *  Determine the length of a string in printer pixels based on the current mode.
 */
static int buffer_width(const char *buff)
{
	const int mode = UState.print_mode;
	unsigned short int posns[ 257 ];
	unsigned char c;
	int l = 0;

	findlengths( posns, mode == PMODE_SMALLGRAPHICS );
	while ((c = 0xff & *buff++) != '\0') {
		switch (mode) {
		case PMODE_DEFAULT:	l += 7;		break;
		default:		l += posns[c];	break;
		}
	}
	return l;
}


/*
 *  Wrap if line is full
 */
static int wrap( int width )
{
	if ( PrinterColumn + width > 166 ) {
		if ( print_advance() ) {
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
int print_line( const char *buff, int with_lf )
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
				w = PrinterColumn == 0 || PrinterColumn == 160 ? 6 : 7;
				abort = print_graphic( glen, graphic );
				glen = 0;
				abort |= wrap( w );
				abort |= put_ir( i );
			}
			else {
				// graphic printing of characters unknown to the printer
				w = 6;
				if ( PrinterColumn > 0 && PrinterColumn < 166 ) {
					// Add horizontal spacing
					graphic[ glen++ ] = 0;
					++PrinterColumn;
					if ( PrinterColumn == 161 ) {
						w = 5;
					}
				}
				goto graphic_print;
			}
			break;

		case PMODE_SMALLGRAPHICS:		// Small font
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
		abort |= print_advance();
	}
	return abort;
}


/*
 *  Print buffer right justified
 */
int print_justified( const char *buff )
{
	const int pmode = UState.print_mode;
	int len = pmode == PMODE_DEFAULT ? slen( buff ) * 7 - 1
	        : pmode == PMODE_SERIAL  ? 0
	        : pixel_length( buff, pmode == PMODE_SMALLGRAPHICS );

	if ( len >= 166 ) {
		len = 166;
	}
	if ( len > 0 ) {
		print_tab( 166 - len );
	}
	return print_line( buff, 1 );
}


/*
 *  Print a single register
 */
int print_reg( int reg, const char *label, int eq )
{
	char buffer[ 65 ];
	int abort = 0;

	if ( label != NULL ) {
		abort = print_line( label, 0 ) || ( eq && print_line( " =", 0 ) );
	}
	xset( buffer, '\0', sizeof( buffer ) );
	format_reg( reg, buffer );
        return abort || print_justified( buffer );
}	


/*
 *  Print a block of registers with labels
 */
void print_registers( enum nilop op )
{
	int s, n;

	int abort = advance_if_trace();

	if ( op == OP_PRINT_STACK ) {
		s = regX_idx;
		n = stack_size();
	}
	else {
		if ( reg_decode( &s, &n, (int *) NULL, 0 ) ) {
			return;
		}
	}

	while ( !abort && n-- ) {
		int r = s;
		char name[ 6 ], *p = name;

		if ( r >= regX_idx && r <= regK_idx ) {
			*p++ = REGNAMES[ r - regX_idx ] | 0x60; // force lower case
		}
		else {
			*p++ = 'r';
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
		*p = '\0';
		abort = print_reg( s++, name, 1 );
	}
	if ( op == OP_PRINT_STACK && !abort ) {
		print_reg( regL_idx, "l", 1 );
		print_reg( regI_idx, "i", 1 );
	}
}

/*
 *  Print the statistical registers
 */
void print_sigma( enum nilop op )
{
	// Use the user commands to get the values

	static const char ops[] = {
		OP_sigmaN,
		OP_sigmaX, OP_sigmaY, 
		OP_sigmaX2, OP_sigmaY2, OP_sigmaXY, OP_sigmaX2Y, 
		OP_sigmalnX, OP_sigmalnY, 
		OP_sigmaXlnY, OP_sigmaYlnX,
		OP_sigmalnXlnX, OP_sigmalnYlnY,
		OP_sigmalnXlnY, 
	};
	int i, abort = advance_if_trace();
	REGISTER save_x;
	char buffer[ 16 ];

	// We need to save and restore X
	copyreg( &save_x, StackBase );

	for ( i = 0; !abort && i < sizeof( ops ); ++i ) {
		const char *p;
		sigma_val( (enum nilop) ops[ i ] );
		p = prt( OP_NIL | ops[ i ], buffer );
		abort = print_reg( regX_idx, p, 1 );
	}
	copyreg( StackBase, &save_x );
}

/*
 *  Print the contents of the Alpha register, terminated by a LF
 */
void print_alpha( enum nilop op )
{
	if ( !advance_if_trace() ) {
		if ( op == OP_PRINT_ALPHA_JUST ) {
			print_justified( Alpha );
		}
		else {
			print_line( Alpha, op == OP_PRINT_ALPHA );
		}
	}
}

/*
 *  Send a LF to the printer
 */
void print_lf( enum nilop op )
{
	if ( !advance_if_trace()) {
		print_advance();
	}
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
		print_tab( arg );
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
	if ( !advance_if_trace()) {
		print_reg( arg, CNULL, 0 );
	}
}

/*
 *  Print a pair of registers as a complex number.
 */
void cmdprintcmplxreg( unsigned int reg, enum rarg op)
{
	decNumber x, y;
	int lenx, leny, maxlen;
	char bufx[ 50 ];
	char bufy[ 54 ];
	char buffer[104];
	char *p;

	if (advance_if_trace())
		return;

	if (is_intmode()) {
		bad_mode_error();
		return;
	}

	getRegister(&x, reg);
	getRegister(&y, reg+1);

	xset( bufx, '\0', sizeof( bufx ) );
	set_x_dn( &x, bufx);
	p = find_char(bufx, '\0');
	scopy(p, " , ");
	lenx = buffer_width(bufx);

	xset( bufy, '\0', sizeof( bufy ) );
	set_x_dn( &y, bufy );
	leny = buffer_width(bufy);

	if (lenx + leny > 166) {
		p[2] = '\0';
		lenx = buffer_width(bufx);
		maxlen = lenx > leny ? lenx : leny;
		print_string_from_tab(bufx, maxlen);
		print_string_from_tab(bufy, maxlen);
	} else {
		scopy(scopy(buffer, bufx), bufy);
		print_justified(buffer);
	}
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
void print_trace( opcode op, int phase )
{
	char buffer[ 16 ];
	const char *p;
	
	if ( Tracing || op == RARG( RARG_SF, T_FLAG ) ) {
		/*
		 *  We trace when flag T has been set
		 */
		if (op == (OP_SPEC | OP_CHS)) {
			if (CmdLineLength > 0)
				return;
		}
		else if (op >= (OP_SPEC | OP_EEX) && op <= (OP_SPEC | OP_F))
			return;
		else if (op == (OP_SPEC | OP_CLX))
			op = OP_NIL | OP_rCLX;
		else if (! Running && isRARG(op) && RARG_CMD(op) == RARG_ALPHA )
			return;

		// Format the command
		p = prt( op, buffer );

		if ( phase == 0 ) {
			// Left part of print
			if ( CmdLineLength ) {
				process_cmdline();
			}
			print_tab( 0 );
			print_line( prt( op, buffer ), 0 );
			print_line( " ", 0 );
		}
		else {
			// right part of print
			print_reg( regX_idx,
				   op == TRACE_DATA_ENTRY ? ">>>" :
				   PrinterColumn == 0     ? ( !Tracing ? p : "***"  ) :
				   CNULL,
				   0 );
			if ( State2.wascomplex ) {
				print_reg( regY_idx, "cpx", 0 );
			}
		}
	}
}


/*
 *  Print a program listing
 *  Start at the PC location
 */
void print_program( enum nilop op )
{
	unsigned int pc = state_pc();
	int abort = advance_if_trace();
	const int runmode = State2.runmode;
	const int pmode = UState.print_mode;
	const int numlen = isRAM( pc ) ? 3 : 4;
	const int tab = numlen * ( 6 - pmode ) + 1;

	if ( runmode ) {
		pc = ProgBegin;
	}
	if ( pc == 0 ) {
		++pc;
	}

	PcWrapped = 0;
	while ( !PcWrapped && !abort ) {
		char buffer[ 16 ];
		const char *p;
		opcode op = getprog( pc );
		unsigned int upc = user_pc( pc );
		*num_arg_0( buffer, upc, numlen ) = '\0';
		abort = print_line( buffer, 0 );
		if ( pmode == PMODE_GRAPHICS || pmode == PMODE_SMALLGRAPHICS ) {
			print_tab( tab );
		}
		print( ' ' );
		p = prt( op, buffer );
		abort = print_line( p, 1 );
		pc = do_inc( pc, runmode );
	}
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

int put_ir( int c )
{
	int s;
	WSADATA ws;
	struct sockaddr_in sa;

	set_IO_annunciator();
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
int put_ir( int c )
{
	static FILE *f;

	set_IO_annunciator();
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


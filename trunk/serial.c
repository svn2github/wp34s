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

#include "serial.h"
#include "xeq.h"
#include "storage.h"

#define STX 2
#define ETX 3
#define ENQ 5
#define ACK 6
#define NAK 0x15
#define MAXCONNECT 10
#define CHARTIME 10

#define IN_BUFF_LEN 32
#define IN_BUFF_MASK 0x1f
#define DATA_LEN 2048

#define TAG_PROGRAM  0x5250 // "PR"
#define TAG_REGISTER 0x4552 // "RE"
#define TAG_ALLMEM   0x4C41 // "AL"


/*
 *  Flags and hardware buffer for received data
 */
short InBuffer[ IN_BUFF_LEN ];
volatile char InRead, InWrite, InCount;
char SerialOn;


/*
 * Receive a single byte from the serial port and return the
 * value in X.
 * If the transfer times out or an error is detected return a negative error code.
 */
int recv_byte( int timeout )
{
	int byte;
	unsigned int ticks = Ticker + timeout;

	while ( InCount == 0 && ( timeout < 0 || Ticker < ticks ) ) {
		/*
		 *  No bytes in buffer
		 */
		idle();
	}
	if ( InCount == 0 ) {
		return R_TIMEOUT;
	}
	lock();
	byte = InBuffer[ (int) InRead ];
	InRead = ( InRead + 1 ) & IN_BUFF_MASK;
	--InCount;
	unlock();
	return byte;
}


/*
 *  Add a received byte to the buffer, called from interrupt.
 */
void byte_received( short byte )
{
	if ( InCount == IN_BUFF_LEN ) {
		/*
		 *  Sorry, no room
		 */
		byte = R_ERROR;
		InWrite = ( InWrite - 1 ) & IN_BUFF_MASK;
		--InCount;
	}

	InBuffer[ (int) InWrite ] = byte;
	InWrite = ( InWrite + 1 ) & IN_BUFF_MASK;
	++InCount;
}


static void clear_buffer( void )
{
	lock();
	InRead = InWrite = InCount = 0;
	unlock();
}


/*
 *  Open port with default settings.
 *  Returns non zero in case of failure.
 */
static int open_port_default( void )
{
	return open_port( 9600, 8, 1, 'N' );
}


/*
 *  Connect to partner.
 *  Opens the port and sends ENQ until ACK is received.
 *  Returns non zero in case of failure.
 */
static int connect( void )
{
	int c, i = MAXCONNECT;

	if ( open_port_default() ) return 1;
	do {
		put_byte( ENQ );
		c = recv_byte( CHARTIME );
	} while ( --i && c != ACK );
	return c != ACK;
}


/*
 *  Accept connection from partner.
 *  Opens the port and waits for ENQ.
 *  Returns non zero in case of failure.
 */
static int accept_connection( void )
{
	int i = MAXCONNECT;

	if ( open_port_default() ) return 1;
	while ( i-- ) {
		if ( recv_byte( CHARTIME ) == ENQ ) {
			clear_buffer();
			put_byte( ACK );
			return 0;
		}
		put_byte( NAK );
	}
	return 1;
}


/*
 *  Transmit a 16 bit word lsb first
 */
static void put_word( unsigned short w )
{
	put_byte( (unsigned char) ( w >> 8 ) );
	put_byte( (unsigned char) w );
}


/*
 *  Receive a 16 bit word. Negative return values are errors.
 */
static int get_word( void )
{
	int cl;
	int ch;

	cl = recv_byte( CHARTIME );
	if ( cl < 0 ) return cl;
	ch = recv_byte( CHARTIME );
	if ( ch < 0 ) return ch;
	return cl | ( ch << 8 );
}


/*
 *  Transmits block of data to the serial port.
 *  Returns non zero in case of error.
 *  The protocol is as follows:
 *    Connect (Send ENQ, wait for ACK, see above)
 *    Send tag (2 bytes)
 *    Send length (16 bit, lsb first)
 *    Send CRC^tag^length (16 bit, lsb first)
 *    Send data
 *    Send ETX
 *    Wait for ACK
 */
static int put_block( unsigned short tag, unsigned short length, void *data )
{
	const unsigned short cc = crc16( data, length ) ^ tag ^ length;
	unsigned char *p = (unsigned char *) data;

	if ( connect() ) return 1;
	put_word( tag );
	put_word( length );
	put_word( cc );
	while ( length-- ) {
		put_byte( *p++ );
	}
	clear_buffer();
	put_byte( ETX );
	if ( ACK != recv_byte( 2 * CHARTIME ) ) {
		err( ERR_IO );
		return 1;
	}
	close_port();
	return 0;
}


/*
 *  Receive block from the serial port and validate the checksum.
 *  If the checksum doesn't match, set error condition.
 *  Depending on the tag received, copy the data to its destination.
 *  This implements the RECV command.
 */
void recv_any( decimal64 *nul1, decimal64 *nul2 )
{
	int i, c;
	unsigned char buffer[ DATA_LEN ];
	int tag, length, crc;
	void *dest;

	if ( accept_connection() ) goto err;

	tag = get_word();
	if ( tag < 0 ) goto err;

	length = get_word();
	if ( length < 0 ) goto err;

	crc = get_word();
	if ( crc < 0 ) goto err;

	for ( i = 0; i < length; ++i ) {
		c = recv_byte( CHARTIME );
		if ( c < 0 ) goto err;
		buffer[ i ] = c;
	}
	c = recv_byte( CHARTIME );
	if ( c != ETX ) goto err;

	if ( crc != ( crc16( buffer, length ) ^ tag ^ length ) ) goto err;

	/*
	 *  Check the tag value and copy the data if valid
	 */
	switch ( tag ) {

	case TAG_PROGRAM:
		/*
		 *  Program area received
		 */
		if ( check_return_stack_segment( -1 )
		  || length > sizeof( s_opcode ) * NUMPROG )
		{
			  goto invalid;
		}
		dest = Prog;
		LastProg = 1 + length / sizeof( s_opcode );
		DispMsg = "Program";
		break;

	case TAG_ALLMEM:
		/*
		 *  All memory received
		 */
		if ( check_return_stack_segment( -1 )
		  || length != sizeof( PersistentRam ) )
		{
			  goto invalid;
		}
		dest = &PersistentRam;
		DispMsg = "All RAM";
		break;

	case TAG_REGISTER:
		/*
		 *  Registers received
		 */
		if ( length > sizeof( Regs ) ) {
			  goto invalid;
		}
		dest = Regs;
		DispMsg = "Register";
		break;

	default:
		goto invalid;
	}

	/*
	 *  Copy the data and recompute the checksums
	 */
	xcopy( dest, buffer, length );
	checksum_all();

	/*
	 *  All is well
	 */
	c = ACK;
	goto close;

	/*
	 *  Various error conditions
	 */
invalid:
	err( ERR_INVALID );
	goto nak;
err:
	err( ERR_IO );
nak:
	c = NAK;

close:
	/*
	 *  Send reply to partner
	 */
	put_byte( c );
	close_port();
	return;
}


/*
 * Open the serial port
 */
void serial_open( decimal64 *nul1, decimal64 *nul2 )
{
}
 
 /*
 * Close the serial port
 */
void serial_close( decimal64 *nul1, decimal64 *nul2 )
{
}


/*
 * Transmit the program space from RAM to the serial port.
 */
void send_program( decimal64 *nul1, decimal64 *nul2 )
{
	put_block( TAG_PROGRAM, ( LastProg - 1 ) * sizeof( s_opcode ), Prog );
}


/*
 * Send registers 00 through 99 to the serial port.
 */
void send_registers( decimal64 *nul1, decimal64 *nul2 )
{
	put_block( TAG_REGISTER, sizeof( decimal64 ) * TOPREALREG, Regs );
}


/*
 * Send all of RAM to the serial port.  2kb in total.
 */
void send_all( decimal64 *nul1, decimal64 *nul2 )
{
	put_block( TAG_ALLMEM, sizeof( PersistentRam ), &PersistentRam );
}


/*
 * Send a single byte as specified in X to the serial port.
 */
void send_byte( decimal64 *nul1, decimal64 *nul2 )
{
	int sgn;
	const unsigned char byte = get_int( &regX, &sgn ) & 0xff;

	put_byte( byte );
}



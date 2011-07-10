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
#define MAXWAIT 10
#define R_TIMEOUT (-1)
#define R_ERROR (-2)
#define R_BREAK (-3)

#define IN_BUFF_LEN 32
#define IN_BUFF_MASK 0x1f
#define DATA_LEN 2048

/*
 *  Format of a serial data block
 */
typedef struct _io_block {
	char tag[ 2 ];
	unsigned short length;
	unsigned short crc;
	unsigned char data[ DATA_LEN ];
} IO_block;

/*
 *  Flags and hardware buffer for received data
 */
short InBuffer[ IN_BUFF_LEN ];
volatile char InRead, InWrite, InCount;
char SerialOn;


/*
 *  Get a single byte from the serial port
 */
static int get_byte( int timeout )
{
	int byte;
	unsigned int ticks = Ticker + timeout;
	while ( InCount == 0 && Ticker < ticks ) {
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
 *  Add a received byte to the buffer
 */
extern void received_byte( short byte )
{
	if ( InCount == IN_BUFF_LEN ) {
		/*
		 *  Sorry, no room
		 */
		byte = R_ERROR;
		InWrite = ( InWrite - 1 ) & IN_BUFF_MASK;
		--InCount;
	}

	lock();
	InBuffer[ (int) InWrite ] = byte;
	InWrite = ( InWrite + 1 ) & IN_BUFF_MASK;
	++InCount;
	unlock();
}


#if 0
/*
 *  Connect to partner
 *  Opens the port and sends ENQ until ACK is received
 */
static int connect( void )
{
	int c, i = MAXWAIT;

	c = open_port( 9600, 8, 1, 'N' );
	if ( c ) {
		// may fail, at least on the emulator
		err( ERR_IO );
		return -1;
	}
	do {
		put_byte( ENQ );
		c = get_byte( 10 );
	} while ( --i && c != ACK );
	return c != ACK;
}
#endif



/*
 * Transmit a number of bytes and a checksum to the serial port.
 */
static void put_block( void *bytes, unsigned int n ) {
	unsigned int i;
	const unsigned short int checksum = crc16( bytes, n );

	for ( i = 0; i < n; i++ )
		put_byte(((unsigned char *)bytes)[i]);
	put_byte((checksum >> 8) & 0xff);
	put_byte(checksum & 0xff);
}

/*
 * Receive a number of bytes from the serial port and validate the checksum.
 * If the checksum doesn't match, nothing happens.
 */
static void get_block(void *bytes, unsigned int n) {
	unsigned int i;
	unsigned char buffer[2048];
	unsigned short crc;
	unsigned int byte;

	for (i=0; i<n; i++)
		buffer[i] = get_byte( 10 );

	byte = get_byte( 10 );
	crc = (byte & 0xff) << 8;
	byte = get_byte( 10 );
	crc |= (byte & 0xff);
	if (crc != crc16(buffer, n)) {
		err(ERR_IO);
		return;
	}
	xcopy(bytes, buffer, n);
}


/*
 * Open the serial port
 */
void serial_open(decimal64 *nul1, decimal64 *nul2) {
}
 
 /*
 * Close the serial port
 */
void serial_close(decimal64 *nul1, decimal64 *nul2) {
}


/*
 * Transmit the program space from RAM to the serial port.
 * 506 * 2 = 1012 bytes plus checksums.
 */
void send_program(decimal64 *nul1, decimal64 *nul2) {
	put_block(Prog, sizeof(s_opcode) * NUMPROG);
}


/*
 * Load the RAM program space from the serial port.
 */
void recv_program(decimal64 *nul1, decimal64 *nul2) {
	if (check_return_stack_segment(-1)) {
		err( ERR_INVALID );
		return;
	}
	get_block(Prog, sizeof(s_opcode) * NUMPROG);
}


/*
 * Send registers 00 through 99 to the serial port.
 * 100 registers at 8 bytes each is 800 bytes in total plus checksums.
 */
void send_registers(decimal64 *nul1, decimal64 *nul2) {
	put_block(Regs, sizeof(decimal64) * TOPREALREG);
}


/*
 * Load registers 00 through 99 from the serial port.
 */
void recv_registers(decimal64 *nul1, decimal64 *nul2) {
	get_block(Regs, sizeof(decimal64) * TOPREALREG);
}


/*
 * Send all of RAM to the serial port.  2kb in total.
 */
void send_all(decimal64 *nul1, decimal64 *nul2) {
	put_block(&PersistentRam, sizeof(TPersistentRam));
}


/*
 * Load all of RAM from the serial port.
 */
void recv_all(decimal64 *nul1, decimal64 *nul2) {
	if (check_return_stack_segment(-1)) {
		err( ERR_INVALID );
		return;
	}
	get_block(&PersistentRam, sizeof(TPersistentRam));
}


/*
 * Send a single byte as specified in X to the serial port.
 */
void send_byte(decimal64 *nul1, decimal64 *nul2) {
	int sgn;
	const unsigned char byte = get_int(&regX, &sgn) & 0xff;

	put_byte(byte);
}


/*
 * Receive a single byte from the serial port and return the
 * value in X.
 * If the transfer times out, ???.
 */
int recv_byte(int timeout) {
	return get_byte(timeout);
}

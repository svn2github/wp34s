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

/* Serial stuff for Windows */

#include <stdio.h>
#define shutdown _shutdown
#include <windows.h>
#undef shutdown
#include "serial.h"
#ifdef WINGUI
#include "emulator_dll.h"
#include "keys.h"
#endif

/*
 *  Global data
 */
#define COMM_BUFF_SIZE 256
HANDLE CommHandle;
DWORD CommError;
int CommTxCount;
unsigned char CommTxBuffer[ COMM_BUFF_SIZE ];
unsigned long CommThreadId;


/*
 *  Set timeouts
 */
static void set_comm_timeouts( DWORD ri, DWORD rtm, DWORD rtc,
			       DWORD wtm, DWORD wtc )
{
	COMMTIMEOUTS timeouts;

	timeouts.ReadIntervalTimeout = ri;
	timeouts.ReadTotalTimeoutMultiplier = rtm;
	timeouts.ReadTotalTimeoutConstant = rtc;
	timeouts.WriteTotalTimeoutMultiplier = wtm;
	timeouts.WriteTotalTimeoutConstant = wtc;

	if ( 0 == SetCommTimeouts( CommHandle, &timeouts ) ) {
		CommError = GetLastError();
	}
}


/*
 *  Insert data in the buffer, wait if full
 */
void insert_data( short data )
{
	while ( byte_received( data ) ) {
		Sleep( 10 );
	}
}


/*
 *  The receiver thread
 */
unsigned long __stdcall CommThread( void *p )
{
	int i, c;
	BOOL res;
	DWORD count;
	COMSTAT status;
	unsigned char buffer[ COMM_BUFF_SIZE ];

	set_comm_timeouts( MAXDWORD, 0, 0, 0, 0 );

	while( SerialOn ) {
		/*
		 *  Read all characters which are already bufferrd
		 */
		res = ReadFile( CommHandle, buffer, COMM_BUFF_SIZE, &count, NULL );
		if ( res == 0 ) {
			/*
			 *  File system error termintes the thread
			 */
			CommError = GetLastError();
			insert_data( R_ERROR );
			break;
		}
#ifdef WINGUI
		if ( GetLastKey() == K60 ) {
			/*
			 *  User abort
			 */
			insert_data( R_BREAK );
			continue;
		}
#endif
		if ( count == 0 ) {
			/*
			 *  No data received, test again
			 */
			Sleep( 50 );
			continue;
		}
		for ( i = 0; i < (int) count && SerialOn; ++i ) {
			/*
			 *  Check for communication error
			 */
			c = buffer[ i ];
			if ( c == 0xff ) {
				res = ClearCommError( CommHandle, &CommError, &status );
				if ( res == 0 ) {
					CommError = GetLastError();
				}
				c = R_ERROR;
			}

			/*
			 *  Insert result into queue
			 */
			insert_data( c );
		}
	}
	CommThreadId = 0;
	return 0;
}


/*
 *  Open a COM port for transmission
 *  The name is fetched from the file wp34s.ini
 *  If no name can be found, "COM1:" is assumed.
 */
int open_port( int baud, int bits, int parity, int stopbits )
{
	char name[ 20 ];
	FILE *f = fopen( "wp34s.ini", "rt" );
	char *p = NULL;
	BOOL res;
	DCB dcb;

	if ( f != NULL ) {
		p = fgets( name, sizeof( name ), f );
		strtok( name, "\r\n\t " );
		fclose( f );
	}
	if ( p == NULL ) {
		strcpy( name, "COM1:" );
	}
	CommHandle = CreateFile( name,
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,    /* EXCLUSIVE */
                                 NULL, /* NOINHERIT */
                                 OPEN_EXISTING,
                                 0,    /* NO ATTRIBUTES */
                                 0 );  /* NO TEMPLATE */

	if ( CommHandle == INVALID_HANDLE_VALUE ) {
		CommHandle = 0;
		return 1;
	}

	/*
	 *  Set up DCB
	 */
	dcb.DCBlength = sizeof( DCB );

	res = GetCommState( CommHandle, &dcb );
	if ( res == 0 ) goto fail;

	/*
	 *  Baudrate
	 */
	dcb.BaudRate = baud;

	/*
	 *  Datenformat
	 */
	dcb.ByteSize = (BYTE) bits;
	dcb.Parity = (BYTE) ( parity == 'N' ? 0 :
			      parity == 'O' ? 1 :
			      parity == 'E' ? 2 :
			      0 );
	dcb.fParity = parity != 'N';
	dcb.StopBits = (BYTE) ( stopbits <= 1 ? 0 :
				baud == 110 ? 1 :
				2 );

	/*
	 *  Handshake etc.
	 */
	dcb.fBinary = TRUE;
	dcb.fErrorChar = TRUE;
	dcb.fNull = FALSE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fAbortOnError = FALSE;
	dcb.ErrorChar = 0xff;

	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.fOutxCtsFlow = FALSE;

	/*
	 *  Setze the paraeters
	 */
	res = SetCommState( CommHandle, &dcb );
	if ( res == 0 ) goto fail;

	/*
	 *  Start the receiver thread
	 */
	SerialOn = 1;
	CreateThread( NULL, 1024 * 16, CommThread, NULL, 0, &CommThreadId );

	/*
	 *  All is OK
	 */
	CommTxCount = 0;
	return 0;

fail:
	/*
	 *  Close in case of error
	 */
	CloseHandle( CommHandle );
	CommHandle = 0;
	CommError = GetLastError();
	return 1;
}


/*
 *  Close the COM port after transmission is complete.
 *  A short wait assures that the receiver thread terminates.
 */
extern void close_port( void )
{
	if ( SerialOn ) {
		SerialOn = 0;
		Sleep( 200 );
		CloseHandle( CommHandle );
	}
}


/*
 *  Output a single byte to the serial
 */
void put_byte( unsigned char byte ) 
{
	CommTxBuffer[ CommTxCount++ ] = byte;
	if ( CommTxCount == COMM_BUFF_SIZE ) {
		flush_comm();
	}
}


/*
 *  Force buffer flush
 */
void flush_comm( void )
{
	BOOL res;
	DWORD written;

	if ( CommTxCount != 0 ) {
		res = WriteFile( CommHandle, CommTxBuffer, CommTxCount, &written, NULL );
		if ( res == 0 || written != CommTxCount ) {
			CommError = GetLastError();
		}
		FlushFileBuffers( CommHandle );
		CommTxCount = 0;
	}
}

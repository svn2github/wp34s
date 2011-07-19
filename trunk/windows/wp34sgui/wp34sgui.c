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
 * This is some glue to make the Windows GUI work with wp34s
 * Some functions will move to more generic modules later
 * when the real hardware port will be attacked
 *
 * Module written by MvC
 */
#include <string.h>
#include <stdio.h>

#include "emulator_dll.h"

#include "builddate.h"
#include "display.h"
#include "storage.h"
#include "serial.h"
#include "data.h"
#include "keys.h"

/*
 *  Setup the LCD area, persistent RAM and not so persistent RAM
 */
unsigned int LcdData[ 20 ];
TPersistentRam PersistentRam;
TStateWhileOn StateWhileOn;

/*
 *  More data (see main.c of real build)
 */
HANDLE CommHandle;
DWORD CommError;
unsigned long CommThreadId;

/*
 *  Keyboard time out ticker
 */
//volatile unsigned short Keyticks;

/*
 *  Used by Emulator only
 */
static int EmulatorFlags;

static char SvnRevision[] = SVN_REVISION;

/*
 *  Tell the revision number
 */
const char *get_revision( void )
{
	return SvnRevision + 7;
}

/*
 *  Main entry point
 *  Update the callback pointers and start application
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow )
{
	unsigned long id;
	extern unsigned long __stdcall HeartbeatThread( void *p );
	int revision;

	/*
	 *  Get the revision information and put it into the build date
	 */
	revision = atoi( SvnRevision + 7 );

	/*
	 *  Create the heartbeat at 100ms
	 */
	CreateThread( NULL, 1024 * 16, HeartbeatThread, NULL, 0, &id );

	/*
	 *  Start the emulator
	 */
	start_emulator( hInstance, hPrevInstance, pCmdLine, nCmdShow,
		        "wp34s Scientific Calculator " VERSION_STRING,
		        BuildDate | ( revision << 12 ),
		        LcdData,
		        Init, Reset, Shutdown,
		        KeyPress, UpdateScreen, 
		        NULL,
		        GetFlag, SetFlag, ClearFlag,
		        NULL,
			GetTopLine,
		        GetBottomLine,
		        NULL );
}

/*
 *  Load/Reset/Save state
 */
void Init( void )
{
	load_statefile();
	DispMsg = NULL;
	init_34s();
	display();
}

void Reset( bool keep )
{
	memset( &PersistentRam, 0, sizeof( PersistentRam ) );
	init_34s();
	display();
}

void Shutdown( void )
{
	save_statefile();
}

/*
 *  main action is here
 */
void KeyPress( int i )
{
	process_keycode( i );
	if ( i != K_HEARTBEAT ) Keyticks = 0;
}

void UpdateScreen( bool forceUpdate )
{
	if ( forceUpdate ) {
		UpdateDlgScreen( true );
	}
}

/*
 *  some helper functions
 */
bool GetFlag( int flag )
{
	return 0 != ( EmulatorFlags & flag );
}

void SetFlag( int flag )
{
	flag &= ~shift; // We handle shift differently then the 20b
	EmulatorFlags |= flag;
}

void ClearFlag( int flag)
{
	flag &= ~shift; // We handle shift differently then the 20b
	EmulatorFlags &= ~flag;
}


char *GetTopLine( void )
{
 	return (char *) (DispMsg == NULL ? Alpha : DispMsg);
}


char *GetBottomLine( void )
{
	static char buffer[ 30 ];
	xset( buffer, '\0', sizeof( buffer ) );
//	decimal64ToString( &regX, buffer );
	format_reg( &regX, buffer );
	return buffer;
}


/*
 *  The Heartbeat
 */
unsigned long __stdcall HeartbeatThread( void *p )
{
	while( 1 ) {
		Sleep( 100 );
		++Ticker;
		if ( Pause ) {
			--Pause;
		}
		if ( ++Keyticks > 1000 ) {
			Keyticks = 1000;
		}
		AddKey( K_HEARTBEAT, true );  // add only if buffewr is empty
	}
}

// These are called from the application

/*
 *  Check if something is waiting for attention
 */
int is_key_pressed(void)
{
	return !KeyBuffEmpty();  // in DLL
}

/*
 *  Get key from buffer
 */
int get_key(void)
{
	return KeyBuffGetKey();
}

/*
 *  Add key to buffer
 */
int put_key(int k)
{
	return AddKeyInBuffer( k );
}


/*
 *  Check if a shift key is held down
 */
int is_shift_down(int s)
{
	return 0;
}


/*
 *  Shut down the emulator from the application
 */
void shutdown( void )
{
	// Shutdown();
	ExitEmulator();
}


/***************************************************************************
 * Serial stuff
 ***************************************************************************/

/*
 *  Setze Timeouts
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
 *  The receiver thread
 */
unsigned long __stdcall CommThread( void *p )
{
	signed short c;
	BOOL res;
	DWORD count;
	COMSTAT status;

	while( SerialOn ) {

		/*
		 * Read a single character with a 0.1 second timeout
		 * This will stop the thread automatically if SerialOn is cleared
		 */
		set_comm_timeouts( 0, 0, 100, 0, 0 );

		/*
		 *  Read with timeout
		 */
		c = 0;
		res = ReadFile( CommHandle, &c, 1, &count, NULL );
		if ( res == 0 ) {
			/*
			 *  Communication error
			 */
			CommError = GetLastError();
			c = R_ERROR;
		}
		if ( GetLastKey() == K60 ) {
			/*
			 *  User abort
			 */
			byte_received( R_BREAK );
			continue;
		}
		if ( count == 0 ) {
			/*
			 *  No data received, test again
			 */
			continue;
		}
		if ( SerialOn ) {
			/*
			 *  Check for communication error
			 */
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
			byte_received( c );
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
	SerialOn = 0;
	Sleep( 200 );
	CloseHandle( CommHandle );
}


/*
 *  Output a single byte to the serial
 */
void put_byte( unsigned char byte ) 
{
	BOOL res;
	char buffer = (char) byte;
	DWORD written;

	res = WriteFile( CommHandle, &buffer, 1, &written, NULL );
	FlushFileBuffers( CommHandle );
	if ( res == 0 || written != 1 ) {
		CommError = GetLastError();
	}
}



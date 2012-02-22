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
#ifdef INCLUDE_STOPWATCH
#include "stopwatch.h"
#endif

/*
 *  Setup the LCD area, persistent RAM and not so persistent RAM
 */
unsigned int LcdData[ 20 ];
TPersistentRam PersistentRam;
TStateWhileOn StateWhileOn;

/*
 *  Keyboard time out ticker
 */
//volatile unsigned short Keyticks;

/*
 *  Used by Emulator only
 */
static int EmulatorFlags;

/*
 *  Main entry point
 *  Update the callback pointers and start application
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow )
{
	unsigned long id;
	extern unsigned long __stdcall HeartbeatThread( void *p );
	extern const char SvnRevision[];
	char buffer[ 5 ];
	int revision;

	/*
	 *  Get the revision information and put it into the build date
	 */
	memset( buffer, 0, sizeof( buffer ) );
	memcpy( buffer, SvnRevision, 4 );
	revision = atoi( buffer );

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
			GetTopLineW,
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
#ifdef INCLUDE_STOPWATCH
	if(KeyCallback!=NULL) {
		i=(*KeyCallback)(i);
	} else {
#endif

	process_keycode( i );
	if ( i != K_HEARTBEAT && i != K_RELEASE ) {
		Keyticks = 0;
	}

#ifdef INCLUDE_STOPWATCH
	}
#endif
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

void ClearFlag( int flag )
{
	flag &= ~shift; // We handle shift differently then the 20b
	EmulatorFlags &= ~flag;
}

#include "../../translate.c"	// Contains Unicode translation table

wchar_t *GetTopLineW( void )
{
	static wchar_t buffer[ NUMALPHA + 1 ];
 	const unsigned char *p = (const unsigned char *) ( DispMsg == NULL ? Alpha : DispMsg );
	wchar_t *q = buffer;

	memset( buffer, 0, sizeof( buffer ) );
	while ( *p != '\0' ) {
		*q++ = (wchar_t) unicode[ *p++ ]; 
	}
	return buffer;
}


char *GetBottomLine( void )
{
	static char buffer[ 30 ];
	xset( buffer, '\0', sizeof( buffer ) );
//	decimal64ToString( &regX, buffer );
	format_reg( regX_idx, buffer );
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
enum shifts shift_down(void)
{
	int map = (int) GetKeyboardMap();
	return map & ( 0x100 << SHIFT_H ) ? SHIFT_H
	     : map & ( 0x100 << SHIFT_G ) ? SHIFT_G
	     : map & ( 0x100 << SHIFT_F ) ? SHIFT_F
	     : SHIFT_N;
}


/*
 *  Shut down the emulator from the application
 */
void shutdown( void )
{
	// Shutdown();
	ExitEmulator();
}


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
#include "xeq.h"
#include "storage.h"
#include "serial.h"
#include "data.h"
#include "keys.h"
#ifdef INCLUDE_STOPWATCH
#include "stopwatch.h"
#include "lcd.h"
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
	 *  Extract the state file name from the command line
	 */
	strtok( pCmdLine, " " );
	if ( *pCmdLine != '\0' ) {
		strncpy( StateFile, pCmdLine, FILENAME_MAX );
	}

	/*
	 *  Create the heartbeat at 100ms
	 */
	CreateThread( NULL, 1024 * 16, HeartbeatThread, NULL, 0, &id );

	/*
	 *  Start the emulator
	 */
	start_emulator( hInstance, hPrevInstance, pCmdLine, nCmdShow,
		        "WP 34S Scientific Calculator " VERSION_STRING,
		        BuildDate | ( revision << 12 ),
		        LcdData,
		        Init, Reset, Save, Import, Export,
		        KeyPress, UpdateScreen, 
		        GetFlag, SetFlag, ClearFlag,
			GetTopLineW,
		        GetBottomLine,
			SetBottomLine );
}

/*
 *  Load/Reset/Save state
 */
void Init( char *filename )
{
	load_statefile( filename );
	DispMsg = NULL;
	init_34s();
	display();
}

void Reset( void )
{
	memset( &PersistentRam, 0, sizeof( PersistentRam ) );
	init_34s();
	display();
}

void Save( char *filename )
{
	save_statefile( filename );
}

/*
 *  Import and export as text
 */
void Import( char *filename )
{
	import_textfile( filename );
	if ( Error ) {
		State2.runmode = 1;
		error_message( Error );
	}
	display();
}

void Export( char *filename )
{
	export_textfile( filename );
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
		if(StopWatchRunning && (Ticker % STOPWATCH_BLINK)==0) {
			dot(LIT_EQ, !is_dot(LIT_EQ));
			finish_display();
		}
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
 	char *p = *LastDisplayedText == '\0' ? Alpha : LastDisplayedText;
	wchar_t *q = buffer;
	int only_blanks = 1;

	memset( buffer, 0, sizeof( buffer ) );
	while ( *p != '\0' ) {
		if ( *p != ' ' && *p != 006 ) {
			only_blanks = 0;
		}
		*q++ = (wchar_t) unicode[ *p++ & 0xff ]; 
	}
	if ( only_blanks ) {
		*LastDisplayedText = '\0';
		return GetTopLineW();
	}
	return buffer;
}


char *GetBottomLine( bool raw )
{
	static char buffer[ 100 ];
	xset( buffer, '\0', sizeof( buffer ) );
	if ( raw ) {
		fill_buffer_from_raw_x( buffer );
	}
	else {
		format_reg( regX_idx, buffer );
	}
	return buffer;
}


void SetBottomLine( const char *buffer )
{
	paste_raw_x( buffer );
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


/*
 *  sleep a while (in ms)
 */
void winsleep( int ms )
{
    Sleep( ms );
}


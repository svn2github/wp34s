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
#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "emulator_dll.h"

#include "builddate.h"
#define T_PERSISTANT_RAM_DEFINED
#include "application.h"
#include "display.h"

/*
 *  setup the LCD area and perstent RAM
 */
static int EmulatorFlags;
unsigned int LcdData[ 20 ];

/*
 *  Main entry point
 *  Update the callback pointers and start application
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow )
{
	unsigned long id;
	extern unsigned long __stdcall HeartbeatThread( void *p );

	/*
	 *  Create the heartbeat at 100ms
	 */
	CreateThread(NULL, 1024 * 16, HeartbeatThread, NULL, 0, &id);

	/*
	 *  Start the emulator
	 */
	start_emulator( hInstance, hPrevInstance, pCmdLine, nCmdShow,
		        "wp34s Scientific Calculator " VERSION_STRING,
		        BuildDate,
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
	FILE *f = fopen( "wp34s.dat", "rb" );
	if ( f != NULL ) {
		fread( &PersistentRam, sizeof( PersistentRam ), 1, f );
		fclose( f );
	}
	init_34s();
}

void Reset( bool keep )
{
	memset( &PersistentRam, 0, sizeof( PersistentRam ) );
	init_34s();
}

void Shutdown( void )
{
	FILE *f = fopen( "wp34s.dat", "wb" );
	if ( f == NULL ) return;
	fwrite( &PersistentRam, sizeof( PersistentRam ), 1, f );
	fclose( f );
}

/*
 *  main action is here
 */
void KeyPress( int i )
{
	process_keycode( i );
}

void UpdateScreen( bool forceUpdate )
{
	if ( forceUpdate ) {
		UpdateDlgScreen( true );
	}
}

/*
 *  some helper fuctions
 */
bool GetFlag( int flag )
{
	if ( flag & shift ) {
		// Special handling of shift key
		if ( State.shifts == SHIFT_G ) {
			EmulatorFlags |= shift;
		}
		else {
			EmulatorFlags &= ~shift;
		}
	}

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
//  decimal64ToString( &regX, buffer );
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
		if ( State.pause ) {
			--State.pause;
		}
		AddKey( K_HEARTBEAT, true );  // add only if buffewr is empty
	}
}


/*
 *  Check if something is waiting for attention
 */
int is_key_pressed(void)
{
	return !KeyBuffEmpty();  // in DLL
}

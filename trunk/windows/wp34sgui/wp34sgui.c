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

#include "emulator_dll.h"

#include "builddate.h"
#define T_PERSISTANT_RAM_DEFINED
#define reset _reset
#include "application.h"
#include "display.h"


static int EmulatorFlags;

/*
 *  Main entry point
 *  Update the callback pointers and start application
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow )
{
	start_emulator( hInstance, hPrevInstance, pCmdLine, nCmdShow,
		       "wp34s Scientific Calculator " VERSION_STRING,
		       BuildDate,
		       init, _reset, KeyPress, updatescreen, 
		       NULL,
		       GetFlag, SetFlag, ClearFlag,
		       NULL,
		       GetBottomLine,
		       NULL );
}

void init(TMyApplication *MyApp)
{
	init_34s();
}

void _reset(TMyApplication *MyApp,bool keep)
{
	memset( MyApp, 0, PERSISTENT_RAM_SIZE );
	init_34s();
}

void KeyPress(TMyApplication *MyApp,int i)
{
	process_keycode( i );
}

void updatescreen(TMyApplication *MyApp,bool forceUpdate)
{
	display();
	UpdateDlgScreen(false);
}

bool GetFlag(TMyApplication *MyApp, int flag)
{
	return 0 != ( EmulatorFlags & flag );
}

void SetFlag(TMyApplication *MyApp, int flag)
{
	EmulatorFlags |= flag;
}

void ClearFlag(TMyApplication *MyApp, int flag)
{
	EmulatorFlags &= ~flag;
}


char *GetBottomLine(TMyApplication *MyApp)
{
	return (char *) DispMsg;
}


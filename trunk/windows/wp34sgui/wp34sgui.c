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
#include "builddate.h"
#define T_PERSISTANT_RAM_DEFINED
#define reset _reset
#include "application.h"
#include "display.h"

char MyName[] = "wp34s Scientific Calculator " VERSION_STRING;

static int EmulatorFlags;

void init(TMyApplication *MyApplication)
{
	extern void init_34s(void);
	init_34s();
}

void KeyPress(TMyApplication *MyApplication,int i)
{
	process_keycode( i );
}

void updatescreen(TMyApplication *MyApplication,bool forceUpdate)
{
	extern void UpdateDlgScreen(bool force);
	display();
	UpdateDlgScreen(false);
}

bool ScrollTopLine(TMyApplication *MyApplication)
{
	return false;
}

bool GetFlag(TMyApplication *MyApplication, int flag)
{
	return 0 != ( EmulatorFlags & flag );
}

void SetFlag(TMyApplication *MyApplication, int flag)
{
	EmulatorFlags |= flag;
}

void ClearFlag(TMyApplication *MyApplication, int flag)
{
	EmulatorFlags &= ~flag;
}

unsigned short GetOffset(TMyApplication *MyApplication)
{
	return 0;
}

char *GetBottomLine(TMyApplication *MyApplication)
{
	return (char *) DispMsg;
}

bool CheckCommunication()
{
	return false;
}




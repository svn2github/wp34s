/*
 *  emulator_dll.c
 *
 *  This is the main file for compiling HP's HP20b emulator kernel as a DLL
 *
 *  Written by Marcus von Cube
 */
#pragma warning(disable:4996) 

#include <string.h>

#include "emulator_dll.h"
#include "application.h"

EXPORT u64 BuildDate;

char EXPORT MyName[ 256 ];  // should be long enough

static void  (*P_init)(TMyApplication*);
static void  (*P_reset)(TMyApplication*,bool);
static void  (*P_KeyPress)(TMyApplication*,int);
static void  (*P_updatescreen)(TMyApplication*,bool);
static bool  (*P_ScrollTopLine)(TMyApplication*);
static bool  (*P_GetFlag)(TMyApplication*,int);
static void  (*P_SetFlag)(TMyApplication*,int);
static void  (*P_ClearFlag)(TMyApplication*,int);
static short (*P_GetOffset)(TMyApplication*);
static char *(*P_GetBottomLine)(TMyApplication*);
static bool  (*P_CheckCommunication)();
static int EmulatorFlags;

/*
 *  Main entry point, must be called from a static initializer in a C++ module
 */  
int start_emulator( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow,
		    char *name,
		    long long builddate,
		    void *p_init,
		    void *p_reset,
		    void *p_KeyPress,
		    void *p_updatescreen,
		    void *p_ScrollTopLine,
		    void *p_GetFlag,
		    void *p_SetFlag,
		    void *p_ClearFlag,
		    void *p_GetOffset,
		    void *p_GetBottomLine,
		    void *p_CheckCommunication )
{
	strncpy( MyName, name, sizeof( MyName - 1 ) );
	BuildDate = builddate;

	P_init               = (void (*)(TMyApplication*))      p_init;
	P_reset              = (void (*)(TMyApplication*,bool)) p_reset;
	P_KeyPress           = (void (*)(TMyApplication*,int))  p_KeyPress;
	P_updatescreen       = (void (*)(TMyApplication*,bool)) p_updatescreen;
	P_ScrollTopLine      = (bool (*)(TMyApplication*))      p_ScrollTopLine;
	P_GetFlag            = (bool (*)(TMyApplication*,int))  p_GetFlag;
	P_SetFlag            = (void (*)(TMyApplication*,int))  p_SetFlag;
	P_ClearFlag          = (void (*)(TMyApplication*,int))  p_ClearFlag;
	P_GetOffset          = (short (*)(TMyApplication*))     p_GetOffset;
	P_GetBottomLine      = (char *(*)(TMyApplication*))     p_GetBottomLine;
	P_CheckCommunication = (bool (*)())                     p_CheckCommunication;

	/*
	 *  Delegate to WinMain
	 */
	return WinMain( hInstance, hPrevInstance, pCmdLine, nCmdShow );
}


void init(TMyApplication *MyApp)
{
	if ( P_init ) P_init( MyApp );
}

void reset(TMyApplication *MyApp,bool KeepTestMode)
{
	if ( P_reset ) P_reset( MyApp, KeepTestMode );
}

void KeyPress(TMyApplication *MyApp,int i)
{
	if ( P_KeyPress ) P_KeyPress( MyApp, i );
}

void updatescreen(TMyApplication *MyApp,bool forceUpdate)
{
	if ( P_updatescreen ) P_updatescreen( MyApp, forceUpdate );
}

bool ScrollTopLine(TMyApplication *MyApp)
{
	if ( P_ScrollTopLine ) return P_ScrollTopLine( MyApp );
	return false;
}

bool GetFlag(TMyApplication *MyApp, int flag)
{
	if ( P_GetFlag ) return P_GetFlag( MyApp, flag );
	return 0 != ( EmulatorFlags & flag );
}

void SetFlag(TMyApplication *MyApp, int flag)
{
	EmulatorFlags |= flag;
	if ( P_SetFlag ) P_SetFlag( MyApp, flag );
}

void ClearFlag(TMyApplication *MyApp, int flag)
{
	EmulatorFlags &= ~flag;
	if ( P_ClearFlag ) P_ClearFlag( MyApp, flag );
}

unsigned short GetOffset(TMyApplication *MyApp)
{
	if ( P_GetOffset ) return P_GetOffset( MyApp );
	return 0;
}

char *GetBottomLine(TMyApplication *MyApp)
{
	if ( P_GetBottomLine ) return P_GetBottomLine( MyApp );
	return "0";
}

bool CheckCommunication()
{
	if ( P_CheckCommunication ) return P_CheckCommunication();
	return false;
}


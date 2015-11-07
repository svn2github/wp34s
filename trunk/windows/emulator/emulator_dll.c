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

/*
 *  (Mostly) pointers to data and functions in the main module
 */
char *MyName;
long long BuildDate;
unsigned int *LcdData;

static void  (*P_Init)(void);
static void  (*P_Reset)(bool);
static void  (*P_Shutdown)(void);
static void  (*P_KeyPress)(int);
static void  (*P_UpdateScreen)(bool);
static bool  (*P_GetFlag)(int);
static void  (*P_SetFlag)(int);
static void  (*P_ClearFlag)(int);
static wchar_t *(*P_GetTopLineW)(void);
static char *(*P_GetBottomLine)(bool);
static void  (*P_SetBottomLine)(const char *);
static bool  (*P_CheckCommunication)(void);
static int EmulatorFlags;

/*
 *  Main entry point, must be called from a static initializer in a C++ module
 */  
int start_emulator( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow,
		    char *name,
		    long long builddate,
		    unsigned int *p_LcdData,
		    void *p_Init,
		    void *p_Reset,
		    void *p_Shutdown,
		    void *p_KeyPress,
		    void *p_UpdateScreen,
		    void *p_GetFlag,
		    void *p_SetFlag,
		    void *p_ClearFlag,
		    void *p_GetTopLineW,
		    void *p_GetBottomLine,
		    void *p_SetBottomLine )
{
	MyName = name;
	BuildDate = builddate;
	LcdData = p_LcdData;

	P_Init               = (void (*)(void))         p_Init;
	P_Reset              = (void (*)(bool))     	p_Reset;
	P_Shutdown           = (void (*)(void))     	p_Shutdown;
	P_KeyPress           = (void (*)(int))      	p_KeyPress;
	P_UpdateScreen       = (void (*)(bool))     	p_UpdateScreen;
	P_GetFlag            = (bool (*)(int))      	p_GetFlag;
	P_SetFlag            = (void (*)(int))      	p_SetFlag;
	P_ClearFlag          = (void (*)(int))      	p_ClearFlag;
	P_GetTopLineW        = (wchar_t *(*)(void)) 	p_GetTopLineW;
	P_GetBottomLine      = (char *(*)(bool))    	p_GetBottomLine;
	P_SetBottomLine      = (void (*)(const char *)) p_SetBottomLine;

	/*
	 *  Delegate to WinMain
	 */
	return WinMain( hInstance, hPrevInstance, pCmdLine, nCmdShow );
}


void Init(void)
{
	if ( P_Init ) P_Init();
}

void Shutdown(void)
{
	if ( P_Shutdown ) P_Shutdown();
}

void Reset( bool KeepTestMode )
{
	if ( P_Reset ) P_Reset(KeepTestMode );
}

void KeyPress( int i )
{
	if ( P_KeyPress ) P_KeyPress( i );
}

void UpdateScreen( bool forceUpdate )
{
	if ( P_UpdateScreen ) P_UpdateScreen( forceUpdate );
}

bool GetFlag( int flag )
{
	if ( P_GetFlag ) return P_GetFlag( flag );
	return 0 != ( EmulatorFlags & flag );
}

void SetFlag( int flag )
{
	EmulatorFlags |= flag;
	if ( P_SetFlag ) P_SetFlag( flag );
}

void ClearFlag( int flag )
{
	EmulatorFlags &= ~flag;
	if ( P_ClearFlag ) P_ClearFlag( flag );
}

wchar_t *GetTopLineW( void )
{
	if ( P_GetTopLineW ) return P_GetTopLineW();
	return L"";
}

char *GetBottomLine( bool raw )
{
	if ( P_GetBottomLine ) return P_GetBottomLine( raw );
	return "0";
}

 void SetBottomLine( const char *buffer )
{
	if ( P_SetBottomLine ) P_SetBottomLine( buffer );
}

bool CheckCommunication( void )
{
	if ( P_CheckCommunication ) return P_CheckCommunication();
	return false;
}


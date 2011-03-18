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
static bool  (*P_ScrollTopLine)(void);
static bool  (*P_GetFlag)(int);
static void  (*P_SetFlag)(int);
static void  (*P_ClearFlag)(int);
static short (*P_GetOffset)(void);
static char *(*P_GetTopLine)(void);
static char *(*P_GetBottomLine)(void);
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
		    void *p_ScrollTopLine,
		    void *p_GetFlag,
		    void *p_SetFlag,
		    void *p_ClearFlag,
		    void *p_GetOffset,
		    void *p_GetTopLine,
		    void *p_GetBottomLine,
		    void *p_CheckCommunication )
{
	MyName = name;
	BuildDate = builddate;
	LcdData = p_LcdData;

	P_Init               = (void (*)(void))  p_Init;
	P_Reset              = (void (*)(bool))  p_Reset;
	P_Shutdown           = (void (*)(void))  p_Shutdown;
	P_KeyPress           = (void (*)(int))   p_KeyPress;
	P_UpdateScreen       = (void (*)(bool))  p_UpdateScreen;
	P_ScrollTopLine      = (bool (*)(void))  p_ScrollTopLine;
	P_GetFlag            = (bool (*)(int))   p_GetFlag;
	P_SetFlag            = (void (*)(int))   p_SetFlag;
	P_ClearFlag          = (void (*)(int))   p_ClearFlag;
	P_GetOffset          = (short (*)(void)) p_GetOffset;
	P_GetTopLine         = (char *(*)(void)) p_GetTopLine;
	P_GetBottomLine      = (char *(*)(void)) p_GetBottomLine;
	P_CheckCommunication = (bool (*)(void))  p_CheckCommunication;

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

bool ScrollTopLine( void )
{
	if ( P_ScrollTopLine ) return P_ScrollTopLine();
	return false;
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

unsigned short GetOffset( void )
{
	if ( P_GetOffset ) return P_GetOffset();
	return 0;
}

char *GetBottomLine( void )
{
	if ( P_GetBottomLine ) return P_GetBottomLine();
	return "0";
}

char *GetTopLine( void )
{
	if ( P_GetTopLine ) return P_GetTopLine();
	return "";
}

bool CheckCommunication( void )
{
	if ( P_CheckCommunication ) return P_CheckCommunication();
	return false;
}


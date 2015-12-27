/*
 *  emulator_dll.c
 *
 *  This is the include file for using the modified HP20b emulator kernel as a DLL
 *  
 *  Written by Marcus von Cube
 */
#ifdef __cplusplus
extern "C" {
#else
#define bool int
#define true 1
#define false 0
#endif

#define shutdown _shutdown
#include <windows.h>
#undef shutdown

// Some defines and decalrations from old "application.h"
#define shift 16               // shift state on?

void Init( char *filename );   // initialization of the calculator keeps memory if possible.. 
void Reset( void );            // reset everything to zero
void Shutdown( void );         // turn off, save state
void KeyPress( int i );        // call when the user presses a key to get action
void UpdateScreen( bool forceUpdate );
bool GetFlag( int flag );
void SetFlag( int flag );
void ClearFlag( int flag );
wchar_t *GetTopLineW( void );
char *GetBottomLine( bool raw );
void SetBottomLine( const char * );

// Our stuff

#ifdef _WINDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

int EXPORT start_emulator( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow,
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
			   void *p_SetBottomLine );

void EXPORT UpdateDlgScreen( int force );

void EXPORT AddKey( int k, bool ifnotfull );

int  EXPORT KeyBuffEmpty( void );

int  EXPORT KeyBuffGetKey( void );

int  EXPORT AddKeyInBuffer( int k );

int  EXPORT GetLastKey( void );

long long EXPORT GetKeyboardMap( void );

void EXPORT ExitEmulator( void );
#ifdef __cplusplus
}
#endif

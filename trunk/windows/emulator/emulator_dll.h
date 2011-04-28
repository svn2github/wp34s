/*
 *  emulator_dll.c
 *
 *  This is the include file for using HP's HP20b emulator kernel as a DLL
 *  
 *  In WinMain, call
 *  init_emulator( name, init, KeyPress, updatescreen, ScrollTopLine, 
 *                 GetFlag, SetFlag, ClearFlag, GetOffset, GetBottomLine,
 *                 CheckCommmunication );
 *  NULLs are valid pointers and are replaced by default behaviour.
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
#define shift 16            // shift state on?

void Init(void);                   // initialization of the calculator keeps memory if possible.. 
void Reset(bool KeepTestMode); // reset everything to zero (except the test system ON/OFF if KeepTestMode=true
void Shutdown(void);           // turn off, save state
void KeyPress(int i);        // call when the user presses a key to get action. returns true if calc needs to be turned off
void UpdateScreen(bool forceUpdate);
bool ScrollTopLine(void);  // function to be called every 200 ms when the screen needs scrolling... returns true if it needs to continue scrolling
bool GetFlag(int flag);
void SetFlag(int flag);
void ClearFlag(int flag);
unsigned short GetOffset(void);
char *GetTopLine(void);
char *GetBottomLine(void);
bool CheckCommunication(void);

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
			   void *p_ScrollTopLine,
			   void *p_GetFlag,
			   void *p_SetFlag,
			   void *p_ClearFlag,
			   void *p_GetOffset,
			   void *p_GetTopLine,
			   void *p_GetBottomLine,
			   void *p_CheckCommunication );

void EXPORT UpdateDlgScreen(int force);

void EXPORT AddKey(int k,bool ifnotfull);

int  EXPORT KeyBuffEmpty(void);

int  EXPORT KeyBuffGetKey(void);

int  EXPORT AddKeyInBuffer(int k);

void EXPORT ExitEmulator(void);
#ifdef __cplusplus
}
#endif

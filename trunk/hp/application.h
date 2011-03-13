/*
Copyright (c) 2007 Hewlett-Packard development company llc
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the disclaimer below.
//
//  - Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the disclaimer below in the documentation and/or
//  other materials provided with the distribution. Unless a specific license is granted
//  to te licencee
//
//  - This software can only be used on a HP hardware. You are not permited to leaverage
//  any part of this software for products/project that do not work on hardware sold by HP
//
//  HP's name may not be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//  DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY HP "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
//  DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
//  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  ----------------------------------------------------------------------------
file created by Cyrille de Brebisson
modified by Marcus von Cube
*/

#ifdef __cplusplus
extern "C" {
#else
#define bool int
#define true 1
#define false 0
#endif

/*
 *  The file contains the application independent definitions.
 *  All external functions need to be implemented by the application.
 *  
 *  If INCLUDE_MYAPPLICATION is defined, the header myapplication.h is included 
 *  in order to properly define the global data structure TMyApplication.
 *  Otherwise, the data structure TMyApplication is defined as just 2048 bytes.
 */
#include "types.h"

/*
 *  Common flags
 */
#define entryNeg 1          // true if negative number
#define entryExponent 2     // true if the exponent is being entered
#define entryExponentNeg 4  // true if exponent entered is negative
#define LowPower 8          // set to true if low power...
#define shift 16            // shift state on?
#define OFF 32768           // true if the calcaultor was turned off
#define TestSystem 65536    // set to true to enable the test system
#define LastIsShiftOperation 131072 // Set to true if the last operation was a shifted one...
#define ScreenValid 8388608 // true if the screen is valid and does not need to be redisplayed...
#define VirtualKey 16777216 // true if the currently executed key comes from serial port... this is used to remove pauses in testing
#define OffsetNoScroll 0xffff

/*
 *  Definition of persistent RAM area
 */
#ifdef _ARM_
#include "board.h"
#define PERSISTANT_RAM_SIZE AT91C_ISRAM_2_SIZE
#else
#define PERSISTENT_RAM_SIZE 2048
#endif

#ifndef T_PERSISTANT_RAM_DEFINED
typedef struct _ram {
	unsigned char ram[ PERSISTENT_RAM_SIZE ];
} TPersistentRam;
#endif

#ifdef _ARM_
static TPersistentRam *const PersistentRam = (TPersistenRam *) AT91C_ISRAM_2;
#else
#undef EXPORT
#endif

#define MagicMarker 0x12BC67D9FE3954AF // stored in RAM to check if RAM is still valid after a reboot

#ifdef _WINDLL
extern u64 BuildDate;
#endif

bool IsShift(void);
void Init(void);                   // initialization of the calculator keeps memory if possible.. 
void Reset(bool KeepTestMode); // reset everything to zero (except the test system ON/OFF if KeepTestMode=true
void Shutdown(void);           // turn off, save state
void KeyPress(int i);        // call when the user presses a key to get action. returns true if calc needs to be turned off
void InternalKeyPress(int key); // same as above, but does not handle shifts...
void UpdateScreen(bool forceUpdate);
bool ScrollTopLine(void);  // function to be called every 200 ms when the screen needs scrolling... returns true if it needs to continue scrolling
bool GetFlag(int flag);
void SetFlag(int flag);
void SetFlag2(int flag, bool setclear);
void ClearFlag(int flag);
#ifndef _ARM_
unsigned short GetOffset(void);
char *GetBottomLine(void);
#endif
void SendString(char const *s);
bool CheckCommunication(void);
void SendChar(u8 c);
void SendCharNoForce(u8 c);
void SendChars(u8 const *b, u32 size, bool ForceSend);
u32 GetChars(u8 *b, u32 nb, u32 timeoutms); // return the number of chars that were NOT read!!!! ie, return of 0 indicate ALL chars were read
int GetChar(); // imediate return with -1 if no chr were present...
int GetChar2(u32 timeout);
void SendBinary(u8 code, u32 size, u8 const *d);
int GetBinary(u8 *b);

#ifdef _ARM_
// restart the watchdog
#define Watchdog() (AT91C_BASE_WDTC->WDTC_WDCR=0xA5000001)
void watchdog(); // same function as above, but not inlined...
#else
#define Watchdog()
#endif

#ifdef __cplusplus
}
#endif
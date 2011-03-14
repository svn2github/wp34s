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
*/

#ifdef __cplusplus
extern "C" {
#endif

//#define FORCE20HW // uncomment to remove test on not HP20hw...


//                 int   long   long long
// LLP64 model is: 32     32    64        (windows)
//  LP64 model is: 32     64    64        (mac and linux)
#include "types.h"

typedef int Boolean;
typedef unsigned short int const r16;

extern int SlowClock; // slow clock speed, for the moment, 11khz, but will move to 32Khz in the new chips
extern int CpuSpeed;   // contains the current CPU speed in hz
extern int RealCpuSpeed; // real speed the CPU is set at, canbe different from the "freq" parametter due to discreete frequency range
extern int BaseCpuSpeed; // internal RC speed, mesured using oscilator
void SetCPUSpeed(int freq); // set the cpu frequency, input is in Mhz and should be <30Mhz...

typedef enum { //TODO
  KNKeyOn= 71
} TKeyNumbers;

#define MAXKEYBUFSIZE 16              // keyboard buffer

typedef struct {
  volatile u64 KeyboardMap;
  volatile signed char LastKeyPress;
  volatile char ONPlusTask;
  volatile char BufRead;
  volatile char BufWrite;
  volatile char KeyBuffer[MAXKEYBUFSIZE];
  u32 rs232buf;
} TSystem;

// mask for keys in the keyboard map
static u64 const Key11InKeyMap= (u64)1<<0;
static u64 const Key12InKeyMap= (u64)1<<1;
static u64 const Key13InKeyMap= (u64)1<<2;
static u64 const Key14InKeyMap= (u64)1<<3;
static u64 const Key16InKeyMap= (u64)1<<5;
static u64 const KeyMinusInKeyMap= (u64)1<<(5*6+4);
static u64 const KeyPlusInKeyMap= (u64)1<<(6*6+4);
static u64 const OnKeyInKeyMap= (u64)1<<36;

extern TSystem System;
int KeyBuffGetKey();         //!<  returns -1 if there is no keys in the buffer
int KeyBuffPeek();           //!<  returns -1 if there is no keys in the buffer, else return the key, but does not remove it from buffer
int KeyBuffEmpty();          //!< returns true/false if there is a key in the buffer or not...
/*! \fn void KeyboardBufferClear()
    \brief remove all keys from keyboard buffer
*/
void KeyboardBufferClear();
/*! \fn void AddKeyInBuffer(int k)
    \brief puts a key in the keyboard buffer.

     Can be used to "simulate" a key press.
    \param k specifies the key to be added to the keyboard buffer
*/
void AddKeyInBuffer(int k);

void initKeyBuffer();

#define ShiftDown() ((System.KeyboardMap&((u64)1<<30))!=0)

#ifdef _ARM_
char * const __checksum= (char*)0x11ffef;
#define checksum *__checksum
__arm char TestChecksum();
#else
#define checksum 0
#define TestChecksum() 0
#endif

// return the current VBAT voltage in 1/10 of volts (34 -> 3.4V)
// perform testing for voltages between min and max. mornally, voltage bellow 2.1V should never hapen
// as the calculator should turn off as soon as you try to turn it on/
// voltages above 3V should never hapen also as lithum battery are 3v.
// mesures can be taken between 1.9 and 3.4V...
int TestVoltage(int min, int max);

// Wait wait for n mili seconds...
#ifdef _ARM_
#define Wait(a) wait(a*(RealCpuSpeed/1024)/4)
#else
#define Wait(a) Sleep(a)
#endif

#ifdef USERS232
void UpdateRS232();
#else
#define UpdateRS232()
#endif

#define KEYN 0
#define KEYI 1
#define KEYPV 2
#define KEYPMT 3
#define KEYFV 4
#define KEYAMORT 5
#define KEYCF 6
#define KEYIRR 7
#define KEYNPV 8
#define KEYBOND 9
#define KEYPERCENT 10
#define KEYRCL 11
#define KEYINPUT 12
#define KEYOPENP 13
#define KEYCLOSEP 14
#define KEYPLUSMOINS 15
#define KEYBACK 16
#define KEYUP 18
#define KEY7 19
#define KEY8 20
#define KEY9 21
#define KEYDIV 22
#define KEYDOWN 24
#define KEY4 25
#define KEY5 26
#define KEY6 27
#define KEYMUL 28
#define KEYSHIFT 30
#define KEY1 31
#define KEY2 32
#define KEY3 33
#define KEYMINUS 34
#define KEYON 36
#define KEY0 37
#define KEYDOT 38
#define KEYEQUAL 39
#define KEYPLUS 40

#define KEYSHIFTPLAN 42

#define SHKEYN (0+KEYSHIFTPLAN*2)
#define SHKEYI (1+KEYSHIFTPLAN*2)
#define SHKEYPV (2+KEYSHIFTPLAN*2)
#define SHKEYPMT (3+KEYSHIFTPLAN*2)
#define SHKEYFV (4+KEYSHIFTPLAN*2)
#define SHKEYAMORT (5+KEYSHIFTPLAN*2)
#define SHKEYCF (6+KEYSHIFTPLAN*2)
#define SHKEYIRR (7+KEYSHIFTPLAN*2)
#define SHKEYNPV (8+KEYSHIFTPLAN*2)
#define SHKEYBOND (9+KEYSHIFTPLAN*2)
#define SHKEYPERCENT (10+KEYSHIFTPLAN*2)
#define SHKEYRCL (11+KEYSHIFTPLAN*2)
#define SHKEYINPUT (12+KEYSHIFTPLAN*2)
#define SHKEYOPENP (13+KEYSHIFTPLAN*2)
#define SHKEYCLOSEP (14+KEYSHIFTPLAN*2)
#define SHKEYPLUSMOINS (15+KEYSHIFTPLAN*2)
#define SHKEYBACK (16+KEYSHIFTPLAN*2)
#define SHKEYAFTERBACK (17+KEYSHIFTPLAN)
#define SHKEYUP (18+KEYSHIFTPLAN*2)
#define SHKEY7 (19+KEYSHIFTPLAN*2)
#define SHKEY8 (20+KEYSHIFTPLAN*2)
#define SHKEY9 (21+KEYSHIFTPLAN*2)
#define SHKEYDIV (22+KEYSHIFTPLAN*2)
#define SHKEYDOWN (24+KEYSHIFTPLAN*2)
#define SHKEY4 (25+KEYSHIFTPLAN*2)
#define SHKEY5 (26+KEYSHIFTPLAN*2)
#define SHKEY6 (27+KEYSHIFTPLAN*2)
#define SHKEYMUL (28+KEYSHIFTPLAN*2)
#define SHKEYSHIFT (30+KEYSHIFTPLAN*2)
#define SHKEY1 (31+KEYSHIFTPLAN*2)
#define SHKEY2 (32+KEYSHIFTPLAN*2)
#define SHKEY3 (33+KEYSHIFTPLAN*2)
#define SHKEYMINUS (34+KEYSHIFTPLAN*2)
#define SHKEYON (36+KEYSHIFTPLAN*2)
#define SHKEY0 (37+KEYSHIFTPLAN*2)
#define SHKEYDOT (38+KEYSHIFTPLAN*2)
#define SHKEYEQUAL (39+KEYSHIFTPLAN*2)
#define SHKEYPLUS (40+KEYSHIFTPLAN*2)

#define KEYXPPYR (0+KEYSHIFTPLAN)
#define KEYICONV (1+KEYSHIFTPLAN)
#define KEYBEG (2+KEYSHIFTPLAN)
#define KEYPPYR (3+KEYSHIFTPLAN)
#define KEYEND (4+KEYSHIFTPLAN)
#define KEYDEPR (5+KEYSHIFTPLAN)
#define KEYDATA (6+KEYSHIFTPLAN)
#define KEYSTATS (7+KEYSHIFTPLAN)
#define KEYBRKEV (8+KEYSHIFTPLAN)
#define KEYDATE (9+KEYSHIFTPLAN)
#define KEYPCALC (10+KEYSHIFTPLAN)
#define KEYSTO (11+KEYSHIFTPLAN)
#define KEYMEMORY (12+KEYSHIFTPLAN)
#define KEYMODE (13+KEYSHIFTPLAN)
#define KEYKEYPAD (14+KEYSHIFTPLAN)
#define KEYPRGM (14+KEYSHIFTPLAN)
#define KEYEEX (15+KEYSHIFTPLAN)
#define KEYRESET (16+KEYSHIFTPLAN)
#define KEYINS (18+KEYSHIFTPLAN)
#define KEYSIN (19+KEYSHIFTPLAN)
#define KEYCOS (20+KEYSHIFTPLAN)
#define KEYTAN (21+KEYSHIFTPLAN)
#define KEYMATH (22+KEYSHIFTPLAN)
#define KEYDEL (24+KEYSHIFTPLAN)
#define KEYLN (25+KEYSHIFTPLAN)
#define KEYEXP (26+KEYSHIFTPLAN)
#define KEYSQR (27+KEYSHIFTPLAN)
#define KEYSQRT (28+KEYSHIFTPLAN)
#define KEYSSHIFT (30+KEYSHIFTPLAN)
#define KEYRAND (31+KEYSHIFTPLAN)
#define KEYFACT (32+KEYSHIFTPLAN)
#define KEYPOW (33+KEYSHIFTPLAN)
#define KEYINV (34+KEYSHIFTPLAN)
#define KEYOFF (36+KEYSHIFTPLAN)
#define KEYPERM (37+KEYSHIFTPLAN)
#define KEYCOMB (38+KEYSHIFTPLAN)
#define KEYANS (39+KEYSHIFTPLAN)
#define KEYRND (40+KEYSHIFTPLAN)

#define KEYONTEST 17
#define KEYRESETSETTINGS 23
#define KEYONMINUS 35
#define KEYONPLUS 41

#define KEYTESTEQ0 29
#define KEYTESTLE0 (17+KEYSHIFTPLAN)
#define KEYTESTGE0 (23+KEYSHIFTPLAN)
#define KEYTESTLT0 (29+KEYSHIFTPLAN)
#define KEYTESTGT0 (35+KEYSHIFTPLAN)
#define KEYTESTNEQ0 (41+KEYSHIFTPLAN)
#define KEYTESTLE (17+KEYSHIFTPLAN*2)
#define KEYTESTGE (23+KEYSHIFTPLAN*2)
#define KEYTESTLT (29+KEYSHIFTPLAN*2)
#define KEYTESTGT (35+KEYSHIFTPLAN*2)
#define KEYTESTNEQ (41+KEYSHIFTPLAN*2)
#define KEYTESTEQ (42+KEYSHIFTPLAN*2)

#ifdef __cplusplus
}
#endif

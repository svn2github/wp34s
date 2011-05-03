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
 *  All more or less persistent global data definitions
 *  Included by xeq.h
 */

#ifndef DATA_H_
#define DATA_H_

#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif

struct _state {
// User noticeable state
#define SB(f, p)	unsigned int f : p
#include "statebits.h"
#undef SB

	unsigned int base : 8;		// Base value for a command with an argument

	unsigned int digval2 : 8;
	unsigned int digval : 10;
	unsigned int numdigit : 4;
	unsigned int shifts : 2;

	unsigned int denom_mode : 2;	// Fractions denominator mode
	unsigned int denom_max : 14;	// Maximum denominator

	unsigned int last_prog : 9;	// Position of the last program statement
	unsigned int int_len : 6;	// Length of Integers
	unsigned int intm : 1;		// In integer mode

	unsigned int state_pc : 15;	// XEQ internal - don't use
	unsigned int state_lift : 1;	// XEQ internal - don't use

	unsigned int retstk_ptr : 4;	// XEQ internal - don't use
	unsigned int usrpc : 9;		// XEQ internal - don't use
	unsigned int smode : 3;		// Single short display mode

	unsigned int catalogue : 5;	// In catalogue mode
	unsigned int int_base : 4;	// Integer input/output base
	unsigned int test : 3;		// Waiting for a test command entry
	unsigned int int_window : 3;	// Which window to display 0=rightmost
	unsigned int gtodot : 1;	// GTO . sequence met

	unsigned int state_running : 1;	// XEQ internal - don't use

	unsigned int alphas : 1;	// Alpha shift key pressed
	unsigned int cmplx : 1;		// Complex prefix pressed
	unsigned int wascomplex : 1;	// Previous operation was complex

	unsigned int arrow : 1;		// Conversion in progress
	unsigned int multi : 1;		// Multi-word instruction being entered
	unsigned int alphashift : 1;	// Alpha shifted to lower case
	unsigned int version : 1;	// Version display mode
	unsigned int implicit_rtn : 1;	// End of program is an implicit return
	unsigned int hyp : 1;		// Entering a HYP or HYP-1 operation
	unsigned int confirm : 2;	// Confirmation of operation required

	unsigned int dot : 1;		// misc use
	unsigned int improperfrac : 1;	// proper or improper fraction display
	unsigned int nothousands : 1;	// , or nothing for thousands separator
	unsigned int ind : 1;		// Indirection STO or RCL
	unsigned int arrow_alpha : 1;	// display alpha conversion
	unsigned int rarg : 1;		// In argument accept mode
	unsigned int runmode : 1;	// Program mode or run mode
	unsigned int flags : 1;		// Display state flags
	unsigned int jg1582 : 1;	// Julian/Gregorian change over in 1582 instead of 1752

	unsigned int disp_small : 1;	// Display the status message in small font
	unsigned int int_maxw : 3;	// maximum available window

	unsigned int hms : 1;		// H.MS mode
	unsigned int fract : 1;		// Fractions mode
	unsigned int leadzero : 1;	// forced display of leading zeros in integer mode

	unsigned int error : 5;		// Did an error occur, if so what code?
	unsigned int status : 4;	// display status screen line

	unsigned int show_register : 7; // temporary display (not X)

	unsigned int contrast : 4;	// Display contrast
	unsigned int alpha_pos : 3;	// Display position inside alpha
	unsigned int entryp : 1;	// Has the user entered something since the last program stop

#ifndef REALBUILD
	unsigned int trace : 1;
#endif

};

#ifdef WIN32
#pragma pack(pop)
#endif

typedef struct _ram {

	/*
	 * Define storage for the machine's registers.
	 */
	decimal64 _regs[NUMREG];
	decimal64 _bank_regs[NUMBANKREGS];

	/*
	 * Define storage for the machine's program space.
	 */
	s_opcode _prog[NUMPROG];

	/*
	 * Generic state
	 */
	struct _state _state;

	/*
	 * Random number seeds
	 */
	unsigned long int _rand_s1, _rand_s2, _rand_s3;

	/*
	 * The program return stack
	 */
	unsigned short int _retstk[RET_STACK_SIZE];

	/*
	 * Storage space for our user flags
	 */
	unsigned short _bank_flags;
	unsigned char _user_flags[(NUMFLG+7) >> 3];

	/*
	 *  Alpha register gets its own space
	 */
	char _alpha[NUMALPHA+1];

	/*
	 *  Magic marker to detect failed RAM
	 */
	unsigned short _crc;

	/*
	 *  Magic marker for deep sleep mode
	 */
	unsigned short _deep_sleep_marker;

} TPersistentRam;

extern TPersistentRam PersistentRam;

#define State		 (PersistentRam._state)
#define Alpha		 (PersistentRam._alpha)
#define Regs		 (PersistentRam._regs)
#define BankRegs	 (PersistentRam._bank_regs)
#define BankFlags	 (PersistentRam._bank_flags)
#define UserFlags	 (PersistentRam._user_flags)
#define RetStk		 (PersistentRam._retstk)
#define RandS1		 (PersistentRam._rand_s1)
#define RandS2		 (PersistentRam._rand_s2)
#define RandS3		 (PersistentRam._rand_s3)
#define Crc              (PersistentRam._crc)
#define DeepSleepMarker  (PersistentRam._deep_sleep_marker)

/*
 *  State that may get lost while the calculator is visibly off
 */
typedef struct _while_on {
	/*
	 * What to display in message area
	 */
	const char *_disp_msg;

	/*
	 *  A ticker, incremented every 100ms
	 */
	volatile unsigned long _ticker;

	/*
	 *  Another ticker which is reset on every keystroke
	 *  In fact, it counts the time between keystrokes
	 */
	volatile unsigned short _keyticks;

	/*
	 *  Time at entering deep sleep mode
	 */
	unsigned short _last_active_second;

	/*
	 *  Last measured voltage
	 */
	volatile unsigned char _voltage;

	/*
	 *  Most recent key pressed while program is running
	 */
	char _last_key;

	/*
	 *  What the user was just typing in
	 */
	unsigned char _cmdlinelength;	// XEQ internal - don't use
	unsigned char _cmdlineeex;	// XEQ internal - don't use
	unsigned char _cmdlinedot;	// XEQ internal - don't use

	char _cmdline[CMDLINELEN + 1];

} TStateWhileOn;

extern TStateWhileOn StateWhileOn;

#define DispMsg		 (StateWhileOn._disp_msg)
#define Ticker		 (StateWhileOn._ticker)
#define Keyticks         (StateWhileOn._keyticks)
#define LastActiveSecond (StateWhileOn._last_active_second)
#define Voltage          (StateWhileOn._voltage)
#define LastKey		 (StateWhileOn._last_key)
#define CmdLineLength	 (StateWhileOn._cmdlinelength)
#define CmdLineEex	 (StateWhileOn._cmdlineeex)
#define CmdLineDot	 (StateWhileOn._cmdlinedot)
#define Cmdline		 (StateWhileOn._cmdline)

/*
 *  More state, only kept while not idle
 */
extern volatile int Pause;         // count down for programmed pause
extern volatile int WaitForLcd;	   // Sync with display refresh

#endif /* DATA_H_ */

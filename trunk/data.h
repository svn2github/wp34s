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

#define MAGIC_MARKER 0xA53C

#pragma pack(push)
#pragma pack(4)

/*
 *  State that must be saved across power off
 */

/*
 *  User visible state
 */
struct _ustate {
	unsigned int contrast :      4;	// Display contrast

	unsigned int denom_mode :    2;	// Fractions denominator mode
	unsigned int denom_max :    14;	// Maximum denominator
	unsigned int improperfrac :  1;	// proper or improper fraction display
	unsigned int fract :         1;	// Fractions mode
	unsigned int dispmode :      2;	// Display mode (ALL, FIX, SCI, ENG)
	unsigned int dispdigs :      4;	// Display digits
	unsigned int fixeng :        1;	// Fix flips to ENG instead of SCI
	unsigned int fraccomma :     1;	// radix mark . or ,
	unsigned int nothousands :   1;	// , or nothing for thousands separator
	unsigned int jg1582 :        1;	// Julian/Gregorian change over in 1582 instead of 1752
// 32 bits
	unsigned int intm :          1;	// In integer mode
	unsigned int leadzero :      1;	// forced display of leading zeros in integer mode
	unsigned int int_mode :      2;	// Integer sign mode
	unsigned int int_base :      4;	// Integer input/output base
	unsigned int int_len :       7;	// Length of Integers
	unsigned int t12 :           1;	// 12 hour time mode
	unsigned int int_maxw :      3;	// maximum available window
	unsigned int stack_depth :   1;	// Stack depth

	unsigned int date_mode :     2;	// Date input/output format
	unsigned int trigmode :      2;	// Trig mode (DEG, RAD, GRAD)
// 24 bits
	unsigned int sigma_mode :    3;	// Which sigma regression mode we're using
	unsigned int slow_speed :    1;	// Speed setting, 1 = slow, 0 = fast
	unsigned int rounding_mode : 3;	// Which rounding mode we're using
	unsigned int padding :       1;	// The last bit
};


/*
 *  System state
 */
struct _state {
	unsigned int last_cat :      5;	// Most recent catalogue browsed
	unsigned int last_catpos :   7;	// Last position in said catalogue
	unsigned int entryp :        1;	// Has the user entered something since the last program stop
	unsigned int state_lift :    1;	// XEQ internal - don't use
	unsigned int implicit_rtn :  1;	// End of program is an implicit return
	unsigned int deep_sleep :    1; // Used to wake up correctly
	unsigned int usrpc :        16;	// XEQ internal - don't use

	/*
	 *  Not bit fields
	 */
	unsigned short state_pc;	// XEQ internal - don't use
	signed short retstk_ptr;	// XEQ internal - don't use
};

/*
 *  This data is stored in battery backed up SRAM.
 *  The total size is limited to 2 KB.
 *  The alignment is carefully chosen to just fill the available space.
 */
typedef struct _ram {
	/*
	 *  Header information for the program space.
	 *  A flash region looks the same.
	 */
	unsigned short _crc_prog;	// checksum
	unsigned short _last_prog;	// Position of the last program statement

	/*
	 *  Define storage for the machine's program space.
	 *  It spans almost 1 KB, leaving just 8 bytes at the end.
	 */
	s_opcode _prog[NUMPROG];

	/*
	 *  The program return stack.
	 */
	unsigned short int _retstk[RET_STACK_SIZE];

	/*
	 *  Banked registers, will be replaced by local registers eventually
	 */
	decimal64 _bank_regs[NUMBANKREGS];

	/*
	 *  Define storage for the machine's registers.
	 */
	decimal64 _regs[NUMREG];

	/*
	 *  Random number seeds
	 */
	unsigned long int _rand_s1, _rand_s2, _rand_s3;

	/*
	 *  Generic state
	 */
	struct _state _state;

	/*
	 *  User state
	 */
	struct _ustate _ustate;

	/*
	 *  Position on return stack where current local variables start
	 */
	signed short _local_regs;
	
	/*
	 *  Storage space for our user flags
	 */
	unsigned short _bank_flags;
	unsigned char _user_flags[(NUMFLG+7) >> 3];

	/*
	 *  Alpha register gets its own space
	 */
	char _alpha[NUMALPHA+1];

	/*
	 *  Magic marker to detect failed RAM
	 *  The CRC excludes the program space which has its own checksum
	 *  And the return stack which may get clobbered by a PSTO command
	 */
	unsigned short _crc;

} TPersistentRam;

extern TPersistentRam PersistentRam;

#define State		 (PersistentRam._state)
#define UState		 (PersistentRam._ustate)
#define CrcProg		 (PersistentRam._crc_prog)
#define LastProg	 (PersistentRam._last_prog)
#define Alpha		 (PersistentRam._alpha)
#define Regs		 (PersistentRam._regs)
#define Tags		 (PersistentRam._tags)
#define BankRegs	 (PersistentRam._bank_regs)
#define Prog		 (PersistentRam._prog)
#define BankFlags	 (PersistentRam._bank_flags)
#define UserFlags	 (PersistentRam._user_flags)
#define RetStk		 (PersistentRam._retstk + RET_STACK_SIZE) // Point to end of stack
#define LocalRegs	 (PersistentRam._local_regs)
#define RandS1		 (PersistentRam._rand_s1)
#define RandS2		 (PersistentRam._rand_s2)
#define RandS3		 (PersistentRam._rand_s3)
#define Crc              (PersistentRam._crc)


/*
 *  State that may be lost on power off
 */
struct _state2 {
	unsigned short digval;
	unsigned char digval2;
	unsigned char numdigit;		// All three used during argument entry
	unsigned char status;		// display status screen line
	unsigned char alpha_pos;	// Display position inside alpha
	unsigned char catalogue;	// In catalogue mode
	unsigned char test;		// Waiting for a test command entry
	unsigned char shifts;		// f, g, or h shift?
	unsigned char smode;		// Single short display mode
	volatile unsigned char voltage; // Last measured voltage
	signed char last_key;		// Most recent key pressed while program is running

	unsigned int confirm : 2;	// Confirmation of operation required
	unsigned int int_window : 3;	// Which window to display 0=rightmost
	unsigned int gtodot : 1;	// GTO . sequence met
	unsigned int cmplx : 1;		// Complex prefix pressed
	unsigned int wascomplex : 1;	// Previous operation was complex
	unsigned int arrow : 1;		// Conversion in progress
	unsigned int multi : 1;		// Multi-word instruction being entered
	unsigned int version : 1;	// Version display mode
	unsigned int hyp : 1;		// Entering a HYP or HYP-1 operation
	unsigned int dot : 1;		// misc use
	unsigned int ind : 1;		// Indirection STO or RCL
#ifdef ENABLE_LOCALS
	unsigned int local : 1;		// entering a local register number .00 to.15
#endif
	unsigned int arrow_alpha : 1;	// display alpha conversion
	unsigned int alphas : 1;        // Alpha shift key pressed
	unsigned int alphashift : 1;	// Alpha shifted to lower case
	unsigned int rarg : 1;		// In argument accept mode
	unsigned int runmode : 1;	// Program mode or run mode
	unsigned int disp_small : 1;	// Display the status message in small font
	unsigned int hms : 1;		// H.MS mode
	unsigned int invalid_disp : 1;  // Display contents is invalid
	unsigned int labellist : 1;	// Displaying the alpha label navigator
	unsigned int registerlist : 1;	// Displaying the register's contents
	unsigned int disp_freeze : 1;   // Set by VIEW to avoid refresh
	unsigned int disp_temp : 1;     // Indicates a temporary display, disables <-
#ifndef REALBUILD
	unsigned int trace : 1;
	unsigned int flags : 1;		// Display state flags
#else
	unsigned int test_flag : 1;	// Test flag for various software tests
#endif

};

/*
 *  State that may get lost while the calculator is visibly off.
 *  This is saved to SLCD memory during deep sleep. The total size
 *  is restricted to 50 bytes. Current size is 49 bytes, so the space
 *  is pretty much exhausted.
 */
typedef struct _while_on {
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
	 * Generic state (2)
	 */
	struct _state2 _state2;

	/*
	 *  What the user was just typing in
	 */
	struct _cline {
		unsigned char cmdlinelength;	// XEQ internal - don't use
		unsigned char cmdlineeex;	// XEQ internal - don't use
		unsigned char cmdlinedot;	// XEQ internal - don't use
		unsigned char cmdbase;		// Base value for a command with an argument
						// fits nicely int his place (alignment)
	} _command_line;
	char _cmdline[CMDLINELEN + 1];

} TStateWhileOn;

extern TStateWhileOn StateWhileOn;

#define State2		 (StateWhileOn._state2)
#define TestFlag	 (State2.test_flag)
#define Voltage          (State2.voltage)
#define LastKey		 (State2.last_key)
#define Ticker		 (StateWhileOn._ticker)
#define Keyticks         (StateWhileOn._keyticks)
#define LastActiveSecond (StateWhileOn._last_active_second)
#define CommandLine	 (StateWhileOn._command_line)
#define CmdLineLength	 (StateWhileOn._command_line.cmdlinelength)
#define CmdLineEex	 (StateWhileOn._command_line.cmdlineeex)
#define CmdLineDot	 (StateWhileOn._command_line.cmdlinedot)
#define CmdBase		 (StateWhileOn._command_line.cmdbase)
#define Cmdline		 (StateWhileOn._cmdline)

#pragma pack(pop)

/*
 *  More state, only kept while not idle
 */
extern volatile int WaitForLcd;	   // Sync with display refresh
extern volatile int Pause;         // Count down for programmed pause
extern int Running;		   // Program is active
extern int JustStopped;            // Set on program stop to ignore the next R/S key in the buffer
extern int Error;		   // Did an error occur, if so what code?
extern int ShowRegister; 	   // Temporary display (not X)
extern int PcWrapped;		   // decpc() or incpc() have wrapped around
extern int ShowRPN;		   // controls the RPN annunciator
extern const char *DispMsg;	   // What to display in message area
extern char TraceBuffer[];         // Display current instruction
extern unsigned int OpCode;        // Pending execution waiting for key-release
extern unsigned char GoFast;	   // Speed-up might be necessary


extern decContext Ctx;

#if !defined(REALBUILD) && !defined(WINGUI)
extern int just_displayed;
#endif
#if !defined(REALBUILD) && !defined(WINGUI)
extern unsigned long long int instruction_count;
extern int view_instruction_counter;
#endif

#endif /* DATA_H_ */

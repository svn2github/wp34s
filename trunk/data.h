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

#ifndef COMPILE_XROM
#define MAGIC_MARKER 0xA53C

#pragma pack(push)
#pragma pack(4)

/*
 *  State that must be saved across power off
 *
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
	unsigned int nothousands :   1;	// opposite of radix mark or nothing for thousands separator
	unsigned int nointseparator: 1;	// opposite of radix mark or nothing for integer display separator
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
	unsigned int jg1582 :        1;	// Julian/Gregorian change over in 1582 instead of 1752
};
#endif

/*
 *  Bit offsets for XROM use
 */
#define UState_contrast        0 // 4	// Display contrast
#define UState_denom_mode      4 // 2	// Fractions denominator mode
#define UState_denom_max       6 // 14	// Maximum denominator
#define UState_improperfrac   20 // 1	// proper or improper fraction display
#define UState_fract          21 // 1	// Fractions mode
#define UState_dispmode       22 // 2	// Display mode (ALL, FIX, SCI, ENG)
#define UState_dispdigs       24 // 4	// Display digits
#define UState_fixeng         28 // 1	// Fix flips to ENG instead of SCI
#define UState_fraccomma      29 // 1	// radix mark . or ,
#define UState_nothousands    30 // 1	// opposite of radix mark or nothing for thousands separator
#define UState_nointseparator 31 // 1	// opposite of radix mark or nothing for integer display separator
#define UState_intm           32 // 1	// In integer mode
#define UState_leadzero       33 // 1	// forced display of leading zeros in integer mode
#define UState_int_mode       34 // 2	// Integer sign mode
#define UState_int_base       36 // 4	// Integer input/output base
#define UState_int_len        40 // 7	// Length of Integers
#define UState_t12            47 // 1	// 12 hour time mode
#define UState_int_maxw       48 // 3	// maximum available window
#define UState_stack_depth    51 // 1	// Stack depth
#define UState_date_mode      52 // 2	// Date input/output format
#define UState_trigmode1      54 // 2	// Trig mode (DEG, RAD, GRAD)
#define UState_trigmode2      55 // 2	// Trig mode (DEG, RAD, GRAD)
#define UState_sigma_mode     56 // 3	// Which sigma regression mode we're using
#define UState_slow_speed     59 // 1	// Speed setting, 1 = slow, 0 = fast
#define UState_rounding_mode  60 // 3	// Which rounding mode we're using
#define UState_jg1582         63 // 1	// Julian/Gregorian change over in 1582 instead of 1752

#ifndef COMPILE_XROM
/*
 *  System state
 */
struct _state {
	unsigned int last_cat :      5;	// Most recent catalogue browsed
	unsigned int catpos :        7;	// Position in said catalogue
	unsigned int entryp :        1;	// Has the user entered something since the last program stop
	unsigned int state_lift :    1;	// XEQ internal - don't use
	unsigned int deep_sleep :    1; // Used to wake up correctly
#ifdef INCLUDE_DOUBLE_PRECISION
	unsigned int mode_double :   1;	// Double precision mode, should go to UState but that is full. :-(
#else
	unsigned int filler :        1;	// force alignment
#endif
	signed   int local_regs :   16; // Position on return stack where current local variables start

	/*
	 *  Not bit fields
	 */
	unsigned short pc;		// XEQ internal - don't use
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
	 */
	unsigned short _prog_max;	// maximum size of program
	unsigned short _prog_size;	// actual size of program

	/*
	 *  Define storage for the machine's program memory and return stack.
	 *  The program return stack is at the end of this area.
	 */
	s_opcode _prog[RET_STACK_SIZE];

	/*
	 *  Define storage for the machine's registers.
	 */
	decimal64 _regs[NUMREG];

	/*
	 *  Alpha register gets its own space
	 */
	char _alpha[NUMALPHA+1];

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
	 *  Begin and end of current program
	 */
	unsigned short int _prog_begin;
	unsigned short int _prog_end;

	/*
	 *  Storage space for our user flags (7 short integers)
	 */
	unsigned short int _user_flags[(NUMFLG+15) >> 4];

	/*
	 *  Number of currently allocated global registers
	 */
	unsigned char _numregs;		// in registers
	unsigned char _sizestatregs;	// in levels

	unsigned short filler;		// alignment

	/*
	 *  CRC or magic marker to detect failed RAM
	 */
	unsigned short _crc;

} TPersistentRam;

extern TPersistentRam PersistentRam;

#define State		(PersistentRam._state)
#define UState		(PersistentRam._ustate)
#define ProgMax		(PersistentRam._prog_max)
#define ProgSize	(PersistentRam._prog_size)
#define Alpha		(PersistentRam._alpha)
#define Regs		(PersistentRam._regs)
#define Prog		(PersistentRam._prog)
#define Prog_1		(PersistentRam._prog - 1)
#define RetStkBase	(PersistentRam._prog + RET_STACK_SIZE) // Point to end of stack
#define ProgBegin	(PersistentRam._prog_begin)
#define ProgEnd		(PersistentRam._prog_end)
#define NumRegs		(PersistentRam._numregs)
#define UserFlags	(PersistentRam._user_flags)
#define SizeStatRegs	(PersistentRam._sizestatregs)
#define RetStkPtr	(PersistentRam._state.retstk_ptr)
#define LocalRegs	(PersistentRam._state.local_regs)
#define RandS1		(PersistentRam._rand_s1)
#define RandS2		(PersistentRam._rand_s2)
#define RandS3		(PersistentRam._rand_s3)
#define Crc             (PersistentRam._crc)


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

	unsigned int confirm : 3;	// Confirmation of operation required
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
	unsigned int local : 1;		// entering a local flag or register number .00 to.15
	unsigned int disp_as_alpha : 1;	// display alpha conversion
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
						// fits nicely into his place (alignment)
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

/*
 *  A private set of registers for non recursive, non interruptible XROM code
 *  They are addressed as local registers from .00 to .15
 */
#ifdef INCLUDE_DOUBLE_PRECISION
#define NUMXREGS 32
#else
#define NUMXREGS 16
#endif
typedef struct _xrom_local
{
	decimal64 _regs[NUMXREGS];
#ifdef INCLUDE_DOUBLE_PRECISION
	// Save area for DBL mode switch
	decimal64 _regsAtoD[4];
	// Private location for J & K in DBLON mode
	decimal64 _regsJK[4];
#endif
	unsigned short int _flags;
} TXromLocal;

extern TXromLocal XromLocal;

#define XromRegs (XromLocal._regs)
#define XromFlags (XromLocal._flags)
#define XromAtoD (XromLocal._regsAtoD)
#define XromJK (XromLocal._regsJK)

#pragma pack(pop)

/*
 *  More state, only kept while not idle
 */
extern volatile int WaitForLcd;	   // Sync with display refresh
extern volatile int Pause;         // Count down for programmed pause
extern int Running;		   // Program is active
#ifndef CONSOLE
extern int JustStopped;            // Set on program stop to ignore the next R/S key in the buffer
#endif
extern int Error;		   // Did an error occur, if so what code?
extern int ShowRegister; 	   // Temporary display (not X)
extern int PcWrapped;		   // decpc() or incpc() have wrapped around
extern int ShowRPN;		   // controls the RPN annunciator
extern const char *DispMsg;	   // What to display in message area
extern char TraceBuffer[];         // Display current instruction
extern unsigned int OpCode;        // Pending execution waiting for key-release
extern s_opcode XeqOpCode;	   // Currently executed function
extern unsigned char GoFast;	   // Speed-up might be necessary
extern unsigned short *RetStk;	   // Pointer to current top of return stack
extern int RetStkSize;		   // actual size of retiurn stack
extern int ProgFree;		   // Remaining program steps
extern decContext Ctx;		   // decNumber library context
extern int JustDisplayed;	   // Avoid duplicate calls to display();
#ifdef CONSOLE
extern unsigned long long int instruction_count;
extern int view_instruction_counter;
#endif

#endif /* COMPILE_XROM */
#endif /* DATA_H_ */

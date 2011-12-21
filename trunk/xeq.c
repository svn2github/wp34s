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

#ifndef REALBUILD
#if defined(WIN32) && !defined(QTGUI) && !defined(__GNUC__)
#include <stdlib.h>  // sleep
#include "win32.h"
#define sleep _sleep
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#include <stdio.h>   // (s)printf
#endif // REALBUILD

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#define GNUC_POP_ERROR
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

#define XEQ_INTERNAL 1
#include "xeq.h"
#include "storage.h"
#include "decn.h"
#include "complex.h"
#include "stats.h"
#include "display.h"
#include "consts.h"
#include "int.h"
#include "date.h"
#include "lcd.h"
#include "xrom.h"
#include "alpha.h"

/* Define the number of program Ticks that must elapse between flashing the
 * RCL annunciator.
 */
#define TICKS_PER_FLASH	(5)

/*
 *  A program is running
 */
int Running;

#ifndef CONSOLE
/*
 *  A program has just stopped
 */
int JustStopped;
#endif

/*
 *  Stopwatch for a programmed pause
 */
volatile int Pause;

/*
 *  Some long running function has called busy();
 */
int Busy;

/*
 *  Error code
 */
int Error;

/*
 *  Indication of PC wrap around
 */
int PcWrapped;

/*
 *  Currently executed function
 */
s_opcode XeqOpCode;

/*
 *  Temporary display (not X)
 */
int ShowRegister;

/*
 *  User code being called from XROM
 */
unsigned short XromUserPc;
unsigned short UserLocalRegs;

/* We need various different math contexts.
 * More efficient to define these globally and reuse them as needed.
 */
decContext Ctx;

/*
 * A buffer for instruction display
 */
char TraceBuffer[25];

/*
 *  Total Size of the return stack
 */
int RetStkSize;

/*
 *  Number of remaining program steps
 */
int ProgFree;

/*
 * The actual top of the return stack
 */
unsigned short *RetStk;

/*
 *  Shift the return stack.
 *  The distance is in levels.
 *  If argument is negative, return stack will shrink.
 *  Returns 1 if unsuccessful (error is set)
 */
int move_retstk(int distance)
{
	if (RetStkSize + RetStkPtr + distance < 0) {
		err(ERR_RAM_FULL);
		return 1;
	}
	xcopy(RetStk + distance, RetStk, (-RetStkPtr) << 1);
	RetStk += distance;
	RetStkSize += distance;
	return 0;
}

/*
 *  How many stack levels with local data have we?
 */
int local_levels(void) {
	return LocalRegs < 0 ? LOCAL_LEVELS(RetStk[LocalRegs]) : 0;
}

/*
 *  How many local registers have we?
 */
int local_regs(void) {
	const int l = local_levels();
	return l == 1 ? NUMXREGS : l >> 2;
}


#ifdef CONSOLE
// Console screen only
unsigned int get_local_flags(void) {
	if (LocalRegs == 0)
		return 0;
	return RetStk[LocalRegs + 1];
}
#endif

void version(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	State2.version = 1;
	if (!State2.runmode)
		display();
}

void cmd_off(decimal64 *a, decimal64 *nul2, enum nilop op) {
	shutdown();
}

#ifndef state_pc
unsigned int state_pc(void) {
	return State.pc;	
}
#endif
static void raw_set_pc(unsigned int pc) {
	State.pc = pc;
	update_program_bounds(0);
}

/*
 *  Where do the program regions start?
 */
static const s_opcode *const RegionTab[] = {
	Prog,
	UserFlash.prog,
	BackupFlash._prog,
	xrom
};

/*
 *  Size of a program segment
 */
int sizeLIB(int region) {
	if (region == REGION_XROM)
		return xrom_size;
	else
		return (int)RegionTab[region][-1];
}


/*
 *  Get an opcode, check for double length codes
 */
static opcode get_opcode( const s_opcode *loc )
{
	opcode r = *loc;
	if ( isDBL(r) ) {
		r |= loc[1] << 16;
	}
	return r;
}


/* 
 * Return the program memory location specified.
 */
opcode getprog(unsigned int pc) {

	const int region = nLIB(pc);
	int offset = offsetLIB(pc);

	if (offset < 0 || offset >= sizeLIB(region))
		return OP_NIL | OP_END;
	return get_opcode(RegionTab[region] + offset);
}


/* 
 * Return the physical start-address of the current program
 */
const s_opcode *get_current_prog(void) {

	const int region = nLIB(ProgBegin);
	return RegionTab[region] + offsetLIB(ProgBegin);
}


/*
 *  Set PC with sanity check
 */
void set_pc(unsigned int pc) {
	if (isRAM(pc)) {
		if (pc > ProgSize)
			pc = ProgSize;
		if (pc > 1 && isDBL(Prog_1[pc - 1]))
			pc--;
	} else if (!isXROM(pc)) {
		const unsigned int n = startLIB(pc) + sizeLIB(nLIB(pc));
		if (pc > n - 1)
			pc = n - 1;
		if (pc > startLIB(pc) && isDBL(getprog(pc - 1)))
			--pc;
	}
	raw_set_pc(pc);
}


/* Set a flag to indicate that a complex operation has taken place
 * This only happens if we're not in a program.
 */
static void set_was_complex(void) {
	if (! Running)
		State2.wascomplex = 1;
}


/* Produce an error and stop
 */
int err(const enum errors e) {
	if (Error == ERR_NONE) {
		Error = e;
		error_message(e);
		return 1;
	}
	return e != ERR_NONE;
}


/* Display a warning
 */
int warn(const enum errors e) {
	if (Running) {
		return err(e);
	}
	error_message(e);
#ifndef CONSOLE
	State2.disp_freeze = 0;
	JustDisplayed = 1;
#endif
	return e != ERR_NONE;
}


/* Doing something in the wrong mode */
static void bad_mode_error(void) {
	err(ERR_BAD_MODE);
}


/* User command to produce an error */
void cmderr(unsigned int arg, enum rarg op) {
	err((enum errors) arg);
}


/* User command to display a warning */
void cmdmsg(unsigned int arg, enum rarg op) {
	error_message((enum errors) arg);
}


#if defined(DEBUG) && defined(CONSOLE)
#include <stdlib.h>
static void error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	putchar('\n');
	exit(1);
}

#define illegal(op)	do { err(ERR_PROG_BAD); printf("illegal opcode 0x%08x\n", op); } while (0)
#else
#define illegal(op)	do { err(ERR_PROG_BAD); } while (0)
#endif

/* Real rounding mode access routine
 */
static unsigned int get_rounding_mode() {
	return UState.rounding_mode;
}

void op_roundingmode(decimal64 *x, decimal64 *nul2, enum nilop op) {
	put_int(get_rounding_mode(), 0, x);
}

void rarg_roundingmode(unsigned int arg, enum rarg op) {
	UState.rounding_mode = arg;
}


/* Pack a number into our DPD register format
 */
static const unsigned char rounding_modes[DEC_ROUND_MAX] = {
	DEC_ROUND_HALF_EVEN, DEC_ROUND_HALF_UP, DEC_ROUND_HALF_DOWN,
	DEC_ROUND_UP, DEC_ROUND_DOWN,
	DEC_ROUND_CEILING, DEC_ROUND_FLOOR
};

void packed_from_number(decimal64 *r, const decNumber *x) {
	decContext ctx64;

	decContextDefault(&ctx64, DEC_INIT_DECIMAL64);
	ctx64.round = rounding_modes[get_rounding_mode()];
	decimal64FromNumber(r, x, &ctx64);
}

void packed128_from_number(decimal128 *r, const decNumber *x) {
	decContext ctx128;

	decContextDefault(&ctx128, DEC_INIT_DECIMAL128);
	ctx128.round = rounding_modes[get_rounding_mode()];
	decimal128FromNumber(r, x, &ctx128);
}

// Repack a decimal128 to decimal64
void packed_from_packed128(decimal64 *r, const decimal128 *s) {
	decNumber temp;
	packed_from_number(r, decimal128ToNumber(s, &temp));
}

/* Check if a value is bogus and error out if so.
 */
static int check_special(const decNumber *x) {
	decNumber y;
	decimal64 z;

	packed_from_number(&z, x);
	decimal64ToNumber(&z, &y);

	if (decNumberIsSpecial(&y)) {
		if (! get_user_flag(NAN_FLAG)) {
			if (decNumberIsNaN(&y))
				err(ERR_DOMAIN);
			else if (decNumberIsNegative(&y))
				err(ERR_MINFINITY);
			else
				err(ERR_INFINITY);
			return 1;
		}
	}
	return 0;
}


int stack_size(void) {
	if (isXROM(state_pc()))
		return 4;
	return UState.stack_depth?8:4;
}

static decimal64 *get_stack(int pos) {
	return &regX + pos;
}

static decimal64 *get_stack_top(void) {
	return get_stack(stack_size()-1);
}

/* Lift the stack one level.
 */
void lift(void) {
	const int n = stack_size();
	int i;

	for (i=n-1; i>0; i--)
		*get_stack(i) = *get_stack(i-1);
}

static void lift_if_enabled(void) {
	if (State.state_lift)
		lift();
}

static void lift2_if_enabled(void) {
	lift_if_enabled();
	lift();
}

static void lower(void) {
	const int n = stack_size();
	int i;

	for (i=1; i<n; i++)
		*get_stack(i-1) = *get_stack(i);
}

static void lower2(void) {
	const int n = stack_size();
	int i;

	for (i=2; i<n; i++)
		*get_stack(i-2) = *get_stack(i);
}


void setlastX(void) {
	regL = regX;
}

static void setlastXY(void) {
	setlastX();
	regI = regY;
}


decNumber *getX(decNumber *x) {
	decimal64ToNumber(&regX, x);
	return x;
}

void setX(const decNumber *x) {
	decNumber xn;

	if (! check_special(x)) {
		decNumberNormalize(&xn, x, &Ctx);
		packed_from_number(&regX, &xn);
	}
}

void getY(decNumber *y) {
	decimal64ToNumber(&regY, y);
}

void setY(const decNumber *y) {
	decNumber yn;

	if (! check_special(y)) {
		decNumberNormalize(&yn, y, &Ctx);
		packed_from_number(&regY, &yn);
	}
}

void setXY(const decNumber *x, const decNumber *y) {
	setX(x);
	setY(y);
}

static void getZ(decNumber *z) {
	decimal64ToNumber(&regZ, z);
}

static void getT(decNumber *q) {
	decimal64ToNumber(&regT, q);
}


void getXY(decNumber *x, decNumber *y) {
	getX(x);
	getY(y);
}

void getXYZ(decNumber *x, decNumber *y, decNumber *z) {
	getXY(x, y);
	getZ(z);
}

void getXYZT(decNumber *x, decNumber *y, decNumber *z, decNumber *t) {
	getXYZ(x, y, z);
	getT(t);
}

void getYZ(decNumber *y, decNumber *z) {
	getY(y);
	getZ(z);
}

void roll_down(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	const decimal64 r = regX;
	lower();
	*get_stack_top() = r;
}

void roll_up(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	const decimal64 r = *get_stack_top();
	lift();
	regX = r;
}

void cpx_roll_down(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	roll_down(NULL, NULL, OP_RDOWN);
	roll_down(NULL, NULL, OP_RDOWN);
}

void cpx_roll_up(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	roll_up(NULL, NULL, OP_RUP);
	roll_up(NULL, NULL, OP_RUP);
}

void cpx_enter(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	decimal64 x = regX, y = regY;
	cpx_roll_up(NULL, NULL, OP_CRUP);
	regX = x;
	regY = y;
}

void cpx_fill(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	const int n = stack_size();
	int i;

	for (i=2; i<n; i++)
		if (i & 1)	*get_stack(i) = regY;
		else		*get_stack(i) = regX;
}

void fill(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	const int n = stack_size();
	int i;

	for (i=1; i<n; i++)
		*get_stack(i) = regX;
}

void drop(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	lower();
	if (op == OP_DROPXY)
		lower();
}


int is_intmode(void) {
	return UState.intm;
}

void lead0(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	UState.leadzero = (op == OP_LEAD0) ? 1 : 0;
}


/* Locate the beginning and end of a section from a PC that points anywhere within
 */
static unsigned short int find_section_bounds(const unsigned int pc, const int endp, unsigned short int *const p_top) {
	unsigned short int top, bottom;

	if (endp && State2.runmode) { 
		// Use the current program as bounds
		top = ProgBegin;
		bottom = ProgEnd;
	}
	else if (isXROM(pc)) {
		top = addrXROM(1);
		bottom = addrXROM(xrom_size);
	} 
	else if (isLIB(pc)) {
		top = startLIB(pc);
		bottom = top + sizeLIB(nLIB(pc)) - 1;
	}
	else {
		top = State2.runmode;  // step 001 if not entering a program
		bottom = ProgSize;
	}
	*p_top = top;
	return bottom;
}


/* Increment the passed PC.  Account for wrap around but nothing else.
 * Return the updated PC.
 * Set PcWrapped on wrap around
 */
unsigned int do_inc(const unsigned int pc, int endp) {
	const unsigned short int npc = pc + 1 + isDBL(getprog(pc));
	unsigned short int top = 0;
	unsigned short int bottom = 0;

	PcWrapped = 0;
	bottom = find_section_bounds(pc, endp, &top);

	if (npc > bottom) {
		PcWrapped = 1;
		return top;
	}
	return npc;
}

/* Decrement the passed PC.  Account for wrap around but nothing else.
 * Return the updated PC.
 * Set PcWrapped on wrap around
 */
unsigned int do_dec(unsigned int pc, int endp) {
	unsigned short int top = 0;
	unsigned short int bottom = 0;

	PcWrapped = 0;
	bottom = find_section_bounds(pc, endp, &top);

	if (pc <= top) {
		PcWrapped = 1;
		pc = bottom;
	}
	else
		--pc;
	if (pc > top && isDBL(getprog(pc - 1)))
		--pc;
	return pc;
}

/* Increment the PC keeping account of wrapping around and stopping
 * programs on such.  Return non-zero if we wrapped.
 */
int incpc(void) {
	raw_set_pc(do_inc(state_pc(), 1));
	return PcWrapped;
}

void decpc(void) {
	raw_set_pc(do_dec(state_pc(), 1));
}

/*
 * Update the pointers to the current program delimited by END statements
 */
void update_program_bounds(const int force) {
	unsigned int pc = state_pc();
	if (pc == 0)
		pc = 1;
	if (! force && pc >= ProgBegin && pc <= ProgEnd)
		return;
	for (PcWrapped = 0; !PcWrapped; pc = do_inc(pc, 0)) {
		ProgEnd = pc;
		if (getprog(pc) == (OP_NIL | OP_END)) {
			break;
		}
	}
	for (pc = state_pc();;) {
		const unsigned int opc = pc;
		pc = do_dec(opc, 0);
		if (PcWrapped || getprog(pc) == (OP_NIL | OP_END)) {
			ProgBegin = opc == 0 ? 1 : opc;
			break;
		}
	}
}

/* Determine where in program space the PC really is
 */
unsigned int user_pc(void) {
	unsigned int pc = state_pc();
	unsigned int n = 1;
	unsigned int base;

	if (pc == 0)
		return 0;

	base = startLIB(pc);
	while (base < pc) {
		base = do_inc(base, 0);
		if (PcWrapped)
			return n;
		++n;
	}
	return n;
}

/* Given a target user PC, figure out the real matching PC
 */
unsigned int find_user_pc(unsigned int target) {
	unsigned int upc = state_pc();
	const int libp = isLIB(upc);
	unsigned int base = libp ? startLIB(upc) : 0;
	unsigned int n = libp ? 1 : 0;

	while (n++ < target) {
		const unsigned int oldbase = base;
		base = do_inc(oldbase, 0);
		if (PcWrapped)
			return oldbase;
	}
	return base;
}


/* Convert a possibly signed string to an integer
 */
int s_to_i(const char *s) {
	int x = 0;
	int neg;

	if (*s == '-') {
		s++;
		neg = 1;
	} else {
		if (*s == '+')
			s++;
		neg = 0;
	}

	for (;;) {
		const char c = *s++;

		if (c < '0' || c > '9')
			break;
		x = 10 * x + (c - '0');
	}
	if (neg)
		return -x;
	return x;
}

/* Convert a string in the given base to an unsigned integer
 */
unsigned long long int s_to_ull(const char *s, unsigned int base) {
	unsigned long long int x = 0;

	for (;;) {
		unsigned int n;
		const char c = *s++;

		if (c >= '0' && c <= '9')
			n = c - '0';
		else if (c >= 'A' && c <= 'F')
			n = c - 'A' + 10;
		else
			break;
		if (n >= base)
			break;
		x = x * base + n;
	}
	return x;
}

const char *get_cmdline(void) {
	if (CmdLineLength) {
		Cmdline[CmdLineLength] = '\0';
		return Cmdline;
	}
	return NULL;
}


static int fract_convert_number(decNumber *x, const char *s) {
	if (*s == '\0') {
		err(ERR_DOMAIN);
		return 1;
	}
	decNumberFromString(x, s, &Ctx);
	return check_special(x);
}

/* Process the command line if any
 */
void process_cmdline(void) {
	decNumber a, b, x, t, z;

	if (CmdLineLength) {
		const unsigned int cmdlinedot = CmdLineDot;
		char cmdline[CMDLINELEN + 1];

		xcopy(cmdline, Cmdline, CMDLINELEN + 1);

		cmdline[CmdLineLength] = '\0';
		if (!is_intmode()) {
			if (cmdline[CmdLineLength-1] == 'E')
				cmdline[CmdLineLength-1] = '\0';
			else if (CmdLineLength > 1 && cmdline[CmdLineLength-2] == 'E' && cmdline[CmdLineLength-1] == '-')
				cmdline[CmdLineLength-2] = '\0';
		}
		CmdLineLength = 0;
		lift_if_enabled();
		State.state_lift = 1;
		CmdLineDot = 0;
		CmdLineEex = 0;
		if (is_intmode()) {
			const int sgn = (cmdline[0] == '-')?1:0;
			unsigned long long int x = s_to_ull(cmdline+sgn, int_base());
			d64fromInt(&regX, build_value(x, sgn));
		} else if (cmdlinedot == 2) {
			char *d0, *d1, *d2;
			int neg;

			UState.fract = 1;
			if (cmdline[0] == '-') {
				neg = 1;
				d0 = cmdline+1;
			} else {
				neg = 0;
				d0 = cmdline;
			}
			d1 = find_char(d0, '.');
			*d1++ = '\0';
			d2 = find_char(d1, '.');
			*d2++ = '\0';
			if (fract_convert_number(&b, d2))
				return;
			if (dn_eq0(&b)) {
				err(ERR_DOMAIN);
				return;
			}
			if (fract_convert_number(&z, d0))	return;
			if (fract_convert_number(&a, d1))	return;
			if (cmdlinedot == 2) {
				dn_divide(&t, &a, &b);
				dn_add(&x, &z, &t);
			} else {
				if (dn_eq0(&a)) {
					err(ERR_DOMAIN);
					return;
				}
				dn_divide(&x, &z, &a);
			}
			if (neg)
				dn_minus(&x, &x);
			setX(&x);
		} else {
			decNumberFromString(&x, cmdline, &Ctx);
			setX(&x);
		}
		set_entry();
	}
}

void process_cmdline_set_lift(void) {
	process_cmdline();
	State.state_lift = 1;
}


/*
 *  Return a pointer to a numbered register.
 *  If locals are enabled and a non existent local register
 *  is accessed, the respective global register is returned.
 *  Error checking must be done outside this routine.
 *  We force the beginning of the local registers on an even stack position.
 *  This ensures 32 bit alignment of the decima64 object.
 */
decimal64 *get_reg_n(int n) {
	if (n >= LOCAL_REG_BASE && LocalRegs < 0) {
		n -= LOCAL_REG_BASE;
		if (local_levels() == 1) {
			// Local XROM register in volatile RAM
			return XromRegs + n;
		} else {
		// local register on the return stack
			return (decimal64 *)(RetStk + (short)((LocalRegs + 2) & 0xfffe)) + n;
		}
	}
	return Regs + (n >= TOPREALREG ? 0 : TOPREALREG - NumRegs) + n;
}

decimal64 *get_flash_reg_n(int n) {
	return BackupFlash._regs + TOPREALREG - BackupFlash._numregs + n;
}

void get_reg_n_as_dn(int n, decNumber *x) {
	decimal64ToNumber(get_reg_n(n), x);
}

void put_reg_n(int n, const decNumber *x) {
	if (! check_special(x))
		packed_from_number(get_reg_n(n), x);
}

long long int get_reg_n_as_int(int n) {
	return d64toInt(get_reg_n(n));
}

void put_reg_n_from_int(int n, const long long int x) {
	d64fromInt(get_reg_n(n), x);
}

void reg_put_int(int n, unsigned long long int val, int sgn) {
	put_int(val, sgn, get_reg_n(n));
}

unsigned long long int reg_get_int(int n, int *sgn) {
	return get_int(get_reg_n(n), sgn);
}

void zero_regs(decimal64 *dest, int n) {
	int i;

	if (is_intmode())
		xset(dest, 0, n << 3);
	else
		for (i=0; i<n; i++)
			dest[i] = CONSTANT_INT(OP_ZERO);
}

void move_regs(decimal64 *dest, decimal64 *src, int n) {
	xcopy(dest, src, n << 3);
}

/* Put an integer into the specified real
 */
void put_int(unsigned long long int val, int sgn, decimal64 *x) {
	if (is_intmode()) {
		d64fromInt(x, build_value(val, sgn));
	} else {
		decNumber t;

		ullint_to_dn(&t, val);
		if (sgn)
			dn_minus(&t, &t);
		packed_from_number(x, &t);
	}
}

unsigned long long int get_int(const decimal64 *x, int *sgn) {
	if (is_intmode()) {
		return extract_value(d64toInt(x), sgn);
	} else {
		decNumber n;

		decimal64ToNumber(x, &n);
		return dn_to_ull(&n, sgn);
	}
}


/* Some conversion routines to take decimal64s and produce integers
 */
long long int d64toInt(const decimal64 *n) {
#if MAX_WORD_SIZE < 52
	decNumber t;
	char buf[50];
	int sgn;

	decimal64ToNumber(n, &t);
	if (dn_eq0(&t))
		return 0;
	if (decNumberIsSpecial(&t))
		return 0;
	if (decNumberIsNegative(&t)) {
		dn_minus(&t, &t);
		sgn = 1;
	} else
		sgn = 0;
	decNumberToString(&t, buf);
	return build_value(s_to_ull(buf, 10), sgn);
#else
	long long int x;

	xcopy(&x, n, sizeof(x));
	return x;
#endif
}

void d64fromInt(decimal64 *n, const long long int z) {
#if MAX_WORD_SIZE < 52
	int sgn;
	unsigned long long int nv;
	char buf[30];
	decNumber t;

	nv = extract_value(z, &sgn);

	ullint_to_dn(&t, nv);
	if (sgn)
		dn_minus(&t, &t);
	packed_from_number(n, &t);
#else
	xcopy(n, &z, sizeof(decimal64));
#endif
}


/* Zero a register
 */
static void set_zero(decimal64 *x) {
#if 0
	if (is_intmode())
		d64fromInt(x, 0);
	else
		*x = CONSTANT_INT(OP_ZERO);
#else
	zero_regs(x, 1);
#endif
}

void clrx(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	set_zero(&regX);
	State.state_lift = 0;
}

/* Zero out the stack
 */
void clrstk(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	set_zero(&regX);
	fill(NULL, NULL, OP_FILL);

	CmdLineLength = 0;
	State.state_lift = 1;
}


/* Zero out all registers excluding the stack and lastx
 */	
void clrreg(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	process_cmdline_set_lift();

	// erase register memory
	zero_regs(get_reg_n(0), NumRegs);

	// erase lettered registers
	if (stack_size() == 4)
		zero_regs(&regA, 4);
	zero_regs(&regJ, 2);

	// erase local registers but keep them allocated
	if (LocalRegs < 0) {
		zero_regs(get_reg_n(LOCAL_REG_BASE), local_regs());
	}
}


/* Clear the subroutine return stack
 */
void clrretstk(void) {
	RetStkPtr = LocalRegs = 0;
}

void clrretstk_pc(void) {
	clrretstk();
	raw_set_pc(0);
	update_program_bounds(1);
}





/***************************************************************************
 * Monadic function handling.
 */

/* Dispatch routine for monadic operations.
 * Since these functions take an argument from the X register, save it in
 * lastx and then replace it with their result, we can factor out the common
 * stack manipulatin code.
 */
static void monadic(const opcode op) 
{
	unsigned int f;
	process_cmdline_set_lift();

	f = argKIND(op);
	if (f < num_monfuncs) {
		if (is_intmode()) {
			if (! isNULL(monfuncs[f].monint)) {
				long long int x = d64toInt(&regX);
				x = ICALL(monfuncs[f].monint)(x);
				setlastX();
				d64fromInt(&regX, x);
			} else
				bad_mode_error();
		} else {
			if (! isNULL(monfuncs[f].mondreal)) {
				decNumber x, r;

				getX(&x);

				if ( NULL == DCALL(monfuncs[f].mondreal)(&r, &x) )
					set_NaN(&r);
				setlastX();
				setX(&r);
			} else
				bad_mode_error();
		}
	} else
		illegal(op);
}

static void monadic_cmplex(const opcode op) {
	decNumber x, y, rx, ry;
	unsigned int f;

	process_cmdline_set_lift();

	f = argKIND(op);

	if (f < num_monfuncs) {
		if (! isNULL(monfuncs[f].mondcmplx)) {
			getXY(&x, &y);

			CALL(monfuncs[f].mondcmplx)(&rx, &ry, &x, &y);

			setlastXY();
			setXY(&rx, &ry);
			set_was_complex();
		} else
			bad_mode_error();
	} else
		illegal(op);
}

/***************************************************************************
 * Dyadic function handling.
 */

/* Dispatch routine for dyadic operations.
 * Again, these functions have a common argument decode and record and
 * common stack manipulation.
 */
static void dyadic(const opcode op) {

	unsigned int f;
	process_cmdline_set_lift();

	f = argKIND(op);
	if (f < num_dyfuncs) {
		if (is_intmode()) {
			if (! isNULL(dyfuncs[f].dydint)) {
				long long int x = d64toInt(&regX);
				long long int y = d64toInt(&regY);
				x = ICALL(dyfuncs[f].dydint)(y, x);
				setlastX();
				lower();
				d64fromInt(&regX, x);
			} else
				bad_mode_error();
		} else {
			if (! isNULL(dyfuncs[f].dydreal)) {
				decNumber x, y, r;

				getXY(&x, &y);

				if (NULL == DCALL(dyfuncs[f].dydreal)(&r, &y, &x))
					set_NaN(&r);
				setlastX();
				lower();
				setX(&r);
			} else
				bad_mode_error();
		}
	} else
		illegal(op);
}

static void dyadic_cmplex(const opcode op) {
	decNumber x1, y1, x2, y2, xr, yr;
	unsigned int f;

	process_cmdline_set_lift();

	f = argKIND(op);
	if (f < num_dyfuncs) {
		if (! isNULL(dyfuncs[f].dydcmplx)) {
			getXYZT(&x1, &y1, &x2, &y2);

			CALL(dyfuncs[f].dydcmplx)(&xr, &yr, &x2, &y2, &x1, &y1);

			setlastXY();
			lower2();
			setXY(&xr, &yr);
			set_was_complex();
		} else
			bad_mode_error();
	} else
		illegal(op);
}
/***************************************************************************
 * Triadic function handling.
 */

/* Dispatch routine for dyadic operations.
 * Again, these functions have a common argument decode and record and
 * common stack manipulation.
 */
static void triadic(const opcode op) {
	unsigned int f;
	process_cmdline_set_lift();

	f = argKIND(op);
	if (f < num_trifuncs) {
		if (is_intmode()) {
			if (! isNULL(trifuncs[f].triint)) {
				long long int x = d64toInt(&regX);
				long long int y = d64toInt(&regY);
				long long int z = d64toInt(&regZ);
				x = ICALL(trifuncs[f].triint)(z, y, x);
				setlastX();
				lower();
				lower();
				d64fromInt(&regX, x);
			} else
				bad_mode_error();
		} else {
			if (! isNULL(trifuncs[f].trireal)) {
				decNumber x, y, z, r;

				getXYZ(&x, &y, &z);

				if (NULL == DCALL(trifuncs[f].trireal)(&r, &z, &y, &x))
					set_NaN(&r);
				setlastX();
				lower();
				lower();
				setX(&r);
			} else
				bad_mode_error();
		}
	} else
		illegal(op);
}


/* Commands to allow access to constants
 */
void cmdconst(unsigned int arg, enum rarg op) {
	if (is_intmode()) {
		bad_mode_error();
		return;
	}
	lift_if_enabled();
	regX = CONSTANT(arg);
}

void cmdconstcmplx(unsigned int arg, enum rarg op) {
	if (is_intmode()) {
		bad_mode_error();
		return;
	}
	lift2_if_enabled();
	regX = CONSTANT(arg);
	regY = CONSTANT_INT(OP_ZERO);
}

/* Commands to allow access to internal constants
 */
void cmdconstint(unsigned int arg, enum rarg op) {
	if (is_intmode()) {
		bad_mode_error();
		return;
	}
	lift_if_enabled();
	regX = CONSTANT_INT(arg);
}

/* Store/recall code here.
 * These two are pretty much the same so we define some utility routines first.
 */

/* Do a basic STO/RCL arithmetic operation.
 */
static int storcl_op(unsigned short opr, const decimal64 *yr, decNumber *r, int rev) {
	decNumber x, y;

	if (rev) {
		getX(&y);
		decimal64ToNumber(yr, &x);
	} else {
		getX(&x);
		decimal64ToNumber(yr, &y);
	}

	switch (opr) {
	case 1:
		dn_add(r, &y, &x);
		break;
	case 2:
		dn_subtract(r, &y, &x);
		break;
	case 3:
		dn_multiply(r, &y, &x);
		break;
	case 4:
		dn_divide(r, &y, &x);
		break;
	case 5:
		dn_min(r, &y, &x);
		break;
	case 6:
		dn_max(r, &y, &x);
		break;
	default:
		return 1;
	}
	return 0;
}

static int storcl_intop(unsigned short opr, const decimal64 *yr, long long int *r, int rev) {
	long long int x, y;

	x = d64toInt(&regX);
	y = d64toInt(yr);

	if (rev) {
		const long long int t = x;
		x = y;
		y = t;
	}

	switch (opr) {
	case 1:
		*r = intAdd(y, x);
		break;
	case 2:
		*r = intSubtract(y, x);
		break;
	case 3:
		*r = intMultiply(y, x);
		break;
	case 4:
		*r = intDivide(y, x);
		break;
	case 5:
		*r = intMin(y, x);
		break;
	case 6:
		*r = intMax(y, x);
		break;
	default:
		return 1;
	}
	return 0;
}

/* We've got a STO operation to do.
 */
void cmdsto(unsigned int arg, enum rarg op) {
	decimal64 *rn = get_reg_n(arg);

	if (op == RARG_STO) {
		*rn = regX;
	} else {
		if (is_intmode()) {
			long long int r;

			if (storcl_intop(op - RARG_STO, rn, &r, 0))
				illegal(op);
			d64fromInt(rn, r);
		} else {
			decNumber r;

			if (storcl_op(op - RARG_STO, rn, &r, 0))
				illegal(op);
			packed_from_number(rn, &r);
		}
	}
}

/* We've got a RCL operation to do.
 */
static void do_rcl(const decimal64 *rn, enum rarg op) {
	if (op == RARG_RCL) {
		decimal64 temp = *rn;
		lift_if_enabled();
		regX = temp;
	} else {
		if (is_intmode()) {
			long long int r;

			if (storcl_intop(op - RARG_RCL, rn, &r, 1))
				illegal(op);
			setlastX();
			d64fromInt(&regX, r);
		} else {
			decNumber r;

			if (storcl_op(op - RARG_RCL, rn, &r, 1))
				illegal(op);
			setlastX();
			setX(&r);
		}
	}
}

void cmdrcl(unsigned int arg, enum rarg op) {
	do_rcl(get_reg_n(arg), op);
}

#ifdef INCLUDE_FLASH_RECALL
void cmdflashrcl(unsigned int arg, enum rarg op) {
	do_rcl(get_flash_reg_n(arg), op - RARG_FLRCL + RARG_RCL);
}
#endif

/* And the complex equivalents for the above.
 * We pair registers arg & arg+1 to provide a complex number
 */
static int storcl_cop(unsigned short opr,
		const decimal64 *y1r, const decimal64 *y2r,
		decNumber *r1, decNumber *r2, int rev) {
	decNumber x1, x2, y1, y2;

	if (rev) {
		getXY(&y1, &y2);
		decimal64ToNumber(y1r, &x1);
		decimal64ToNumber(y2r, &x2);
	} else {
		getXY(&x1, &x2);
		decimal64ToNumber(y1r, &y1);
		decimal64ToNumber(y2r, &y2);
	}

	switch (opr) {
	case 1:
		cmplxAdd(r1, r2, &y1, &y2, &x1, &x2);
		break;
	case 2:
		cmplxSubtract(r1, r2, &y1, &y2, &x1, &x2);
		break;
	case 3:
		cmplxMultiply(r1, r2, &y1, &y2, &x1, &x2);
		break;
	case 4:
		cmplxDivide(r1, r2, &y1, &y2, &x1, &x2);
		break;
	default:
		return 1;
	}
	return 0;
}


void cmdcsto(unsigned int arg, enum rarg op) {
	decNumber r1, r2;
	decimal64 *t1, *t2;

	t1 = get_reg_n(arg);
	t2 = get_reg_n(arg+1);

	if (op == RARG_CSTO) {
		*t1 = regX;
		*t2 = regY;
	} else {
		if (is_intmode())
			bad_mode_error();
		else if (storcl_cop(op - RARG_CSTO, t1, t2, &r1, &r2, 0))
			illegal(op);
		else {
			packed_from_number(t1, &r1);
			packed_from_number(t2, &r2);
		}
	}
	set_was_complex();
}

static void do_crcl(const decimal64 *t1, const decimal64 *t2, enum rarg op) {
	decNumber r1, r2;

	if (op == RARG_CRCL) {
		decimal64 x = *t1;
		decimal64 y = *t2;
		lift2_if_enabled();
		regX = x;
		regY = y;
	} else {
		if (is_intmode())
			bad_mode_error();
		else if (storcl_cop(op - RARG_CRCL, t1, t2, &r1, &r2, 1))
			illegal(op);
		else {
			setlastXY();
			setXY(&r1, &r2);
		}
	}
	set_was_complex();
}

void cmdcrcl(unsigned int arg, enum rarg op) {
	const decimal64 *t1, *t2;

	t1 = get_reg_n(arg);
	t2 = get_reg_n(arg+1);
	do_crcl(t1, t2, op);
}

#ifdef INCLUDE_FLASH_RECALL
void cmdflashcrcl(unsigned int arg, enum rarg op) {
	const decimal64 *t1, *t2;

	t1 = BackupFlash._regs+arg;
	t2 = t1+1;
	do_crcl(t1, t2, op - RARG_FLCRCL + RARG_CRCL);
}
#endif

/* SWAP x with the specified register
 */
void swap_reg(decimal64 *a, decimal64 *b) {
	decimal64 t;

	t = *a;
	*a = *b;
	*b = t;
}

void cmdswap(unsigned int arg, enum rarg op) {
	decimal64 *reg;

	if (op == RARG_CSWAPX)
		reg = &regX;
	else if (op == RARG_CSWAPZ)
		reg = &regZ;
	else
		reg = &regX + (int)(op - RARG_SWAPX);

	swap_reg(reg, get_reg_n(arg));
	if (op >= RARG_CSWAPX) {
		swap_reg(reg+1, get_reg_n(arg+1));
		set_was_complex();
	}
}


/* View a specified register
 */
void cmdview(unsigned int arg, enum rarg op) {
	ShowRegister = arg;
	State2.disp_freeze = 0;
	display();
	State2.disp_freeze = Running || arg != regX_idx;
}


#ifdef INCLUDE_USER_MODE
/* Save and restore user state.
 */
void cmdsavem(unsigned int arg, enum rarg op) {
	xcopy( get_reg_n(arg), &UState, sizeof(unsigned long long int) );
}

void cmdrestm(unsigned int arg, enum rarg op) {
	xcopy( &UState, get_reg_n(arg), sizeof(unsigned long long int) );
	if ( UState.contrast == 0 )
		UState.contrast = 7;
}
#endif

/* Set the stack size */
void set_stack_size(decimal64 *a, decimal64 *nul2, enum nilop op) {
	UState.stack_depth = (op == OP_STK4) ? 0 : 1;
}

/* Get the stack size */
void get_stack_size(decimal64 *a, decimal64 *nul2, enum nilop op) {
	put_int(stack_size(), 0, a);
}

void get_word_size(decimal64 *a, decimal64 *nul2, enum nilop op) {
	put_int((int)word_size(), 0, a);
}

void get_sign_mode(decimal64 *a, decimal64 *nul2, enum nilop op) {
	static const unsigned char modes[4] = {
		0x02,		// 2's complement
		0x01,		// 1's complement
		0x00,		// unsigned
		0x81		// sign and mantissa
	};
	const unsigned char v = modes[(int)int_mode()];
	put_int(v & 3, v & 0x80, a);
}

void get_base(decimal64 *a, decimal64 *nul2, enum nilop op) {
	put_int((int)int_base(), 0, a);
}

/* Get the current ticker value */
void op_ticks(decimal64 *a, decimal64 *nul2, enum nilop op) {
#ifndef CONSOLE
    put_int(Ticker, 0, a);
#else
    struct timeval tv;
    long long int t;
    gettimeofday(&tv, NULL);
    t = tv.tv_sec * 10 + tv.tv_usec / 100000;
    put_int(t, 0, a);
#endif
}

/* Display the battery voltage */
void op_voltage(decimal64 *a, decimal64 *nul2, enum nilop op) {
	decNumber t, u;
#ifdef REALBUILD
	unsigned long long int v = 19 + Voltage;
#else
	unsigned long long int v = 32;
#endif

	if (is_intmode()) {
		put_int(v, 0, a);
	} else {
		ullint_to_dn(&t, v);
		dn_mulpow10(&u, &t, -1);
		packed_from_number(a, &u);
	}
}

/*
 *  Commands to determine free memory
 */
int free_mem(void) {
	return RetStkSize + RetStkPtr;
}

int free_flash(void) {
	return NUMPROG_FLASH_MAX - UserFlash.size;
}

void get_mem(decimal64 *a, decimal64 *nul2, enum nilop op) {
	put_int( op == OP_MEMQ ? free_mem() : 
		 op == OP_LOCRQ ? local_regs() :
		 op == OP_FLASHQ ? free_flash() :
		 NumRegs,
		 0, a );
}


/* Check if a keystroke is pending in the buffer, if so return it to the specified
 * register, if not skip the next step.
 */
void op_keyp(unsigned int arg, enum rarg op) {
	int cond = LastKey == 0;
	if (!cond) {
		int k = LastKey - 1;
		LastKey = 0;
		reg_put_int(arg, keycode_to_row_column(k), 0);
	}
	fin_tst(cond);
}

/*
 *  Get a key code from a register and translate it from row/colum to internal
 *  Check for valid arguments
 */
static int get_keycode_from_reg(unsigned int n)
{
	int sgn;
	const int c = row_column_to_keycode((int) reg_get_int((int) n, &sgn));
	if ( c < 0 )
		err(ERR_RANGE);
	return c;
}

/*
 *  Take a row/column key code and feed it to the keyboard buffer
 *  This stops program execution first to make sure, the key is not
 *  read in by KEY? again.
 */
void op_putkey(unsigned int arg, enum rarg op)
{
	const int c = get_keycode_from_reg(arg);

	if (c >= 0) {
		set_running_off();
		put_key(c);
	}
}

/*
 *  Return the type of the keycode in register n
 *  returns 0-9 for digits, 10 for ., +/-, EEX, 11 for f,g,h, 12 for all other keys.
 *  Invalid codes produce an error.
 */
void op_keytype(unsigned int arg, enum rarg op)
{
	const int c = get_keycode_from_reg(arg);
	if ( c >= 0 ) {
		const char types[] = {
			12, 12, 12, 12, 12, 12,
			12, 12, 12, 11, 11, 11,
			12, 12, 10, 10, 12, 12,
			12,  7,  8,  9, 12, 12,
			12,  4,  5,  6, 12, 12,
			12,  1,  2,  3, 12, 12,
			12,  0, 10, 12, 12 };
		lift_if_enabled();
		put_int(types[c], 0, &regX);
	}
}


/* Check which operating mode we're in -- integer or real -- they both
 * vector through this routine.
 */
void check_mode(decimal64 *a, decimal64 *nul2, enum nilop op) {
	const int intmode = is_intmode() ? 1 : 0;
	const int desired = (op == OP_ISINT) ? 1 : 0;

	fin_tst(intmode == desired);
}


/* Save and restore the entire stack to sequential registers */
static int check_stack_overlap(unsigned int arg) {
	const int n = stack_size();

	if (arg + n <= NumRegs || arg >= NUMREG) {
		return n;
	}
	err(ERR_STK_CLASH);
	return 0;
}

void cmdstostk(unsigned int arg, enum rarg op) {
	int i, n = check_stack_overlap(arg);

	for (i=0; i<n; i++)
		*get_reg_n(arg+i) = *get_stack(i);
}

void cmdrclstk(unsigned int arg, enum rarg op) {
	int i, n = check_stack_overlap(arg);

	for (i=0; i<n; i++)
		*get_stack(i) = *get_reg_n(arg+i);
}


/*
 *  Move up the return stack, skipping any local variables
 */
static void retstk_up(void)
{
	if (RetStkPtr < 0) {
		int sp = RetStkPtr++;
		unsigned int s = RetStk[sp++];
		if (isLOCAL(s)) {
			sp += LOCAL_LEVELS(s);
			RetStkPtr = sp;
			// Re-adjust the LocalRegs pointer
			LocalRegs = 0;
			while (sp < 0) {
				if (isLOCAL(RetStk[sp])) {
					LocalRegs = sp;
					break;
				}
				++sp;
			}
		}
	}
}


/* Search from the given position for the specified numeric label.
 */
unsigned int find_opcode_from(unsigned int pc, const opcode l, const int flags) {
	unsigned short int top;
	int count;
	const int endp = flags & FIND_OP_ENDS;
	const int errp = flags & FIND_OP_ERROR;

	count = 1 + find_section_bounds(pc, endp, &top);
	count -= top;
	while (count--) {
		// Wrap around doesn't hurt, we just limit the search to the number of possible steps
		// If we don't find the label, we may search a little too far if many double word
		// instructions are in the code, but this doesn't do any harm.
		if (getprog(pc) == l)
			return pc;
		pc = do_inc(pc, endp);
	}
	if (errp)
		err(ERR_NO_LBL);
	return 0;
}


unsigned int find_label_from(unsigned int pc, unsigned int arg, int flags) {
	return find_opcode_from(pc, RARG(RARG_LBL, arg), flags);
}



/* Handle a GTO/GSB instruction
 */
static void gsbgto(unsigned int pc, int gsb, unsigned int oldpc) {
	raw_set_pc(pc);
	if (gsb) {
		if (!Running) {
			// XEQ or hot key from keyboard
			clrretstk();
			set_running_on();
		}
		if (-RetStkPtr >= RetStkSize) {
			// Stack is full
			err(ERR_RAM_FULL);
			// clrretstk();
		}
		else {
			// Push PC on return stack
			RetStk[--RetStkPtr] = oldpc;
		}
	}
}

// Handle a RTN
static void do_rtn(int plus1) {
	if (Running) {
		if (RetStkPtr < 0) {
			// Pop any LOCALS off the stack
			retstk_up();
		}
		if (RetStkPtr <= 0) {
			// Normal RTN within program
			unsigned short pc = RetStk[RetStkPtr - 1];
			raw_set_pc(pc);
			// If RTN+1 inc PC if not at END or a POPUSR command would be skipped
			fin_tst(! plus1 || getprog(pc) == (OP_NIL | OP_POPUSR));
		}
		else {
			// program was started without a valid return address on the stack
			clrretstk_pc();
		}
		if (RetStkPtr == 0) {
			// RTN with empty stack stops
			set_running_off();
		}
	} else {
		// Manual return goes to step 0 and clears the return stack
		clrretstk_pc();
	}
}

// RTN and RTN+1
void op_rtn(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	do_rtn(op == OP_RTNp1 ? 1 : 0);
}


// Called by XEQ, GTO and CAT browser
void cmdgtocommon(int gsb, unsigned int pc) {
	if (pc == 0)
		set_running_off();
	else
		gsbgto(pc, gsb, state_pc());
}

void cmdlblp(unsigned int arg, enum rarg op) {
	fin_tst(find_label_from(state_pc(), arg, FIND_OP_ENDS) != 0);
}

void cmdgto(unsigned int arg, enum rarg op) {
	cmdgtocommon(op != RARG_GTO, find_label_from(state_pc(), arg, FIND_OP_ERROR | FIND_OP_ENDS));
}

static unsigned int findmultilbl(const opcode o, int flags) {
	const opcode dest = (o & 0xfffff0ff) + (DBL_LBL << DBL_SHIFT);
	unsigned int lbl;

	lbl = find_opcode_from(0, dest, 0);					// RAM
	if (lbl == 0)
		lbl = find_opcode_from(addrLIB(0, REGION_LIBRARY), dest, 0);	// Library
	if (lbl == 0)
		lbl = find_opcode_from(addrLIB(0, REGION_BACKUP), dest, 0);	// Backup
	if (lbl == 0)
		lbl = find_opcode_from(addrXROM(0), dest, 0);			// XROM
	if (lbl == 0 && (flags & FIND_OP_ERROR) != 0)
		err(ERR_NO_LBL);
	return lbl;
}

void cmdmultilblp(const opcode o, enum multiops mopr) {
	fin_tst(findmultilbl(o, 0) != 0);
}

static void do_multigto(int is_gsb, unsigned int lbl) {
	if (!Running && isXROM(lbl) && ! is_gsb) {
		lbl = 0;
		err(ERR_RANGE);
	}

	cmdgtocommon(is_gsb, lbl);
}

void cmdmultigto(const opcode o, enum multiops mopr) {
	unsigned int lbl = findmultilbl(o, FIND_OP_ERROR);
	int is_gsb = mopr != DBL_GTO;

	do_multigto(is_gsb, lbl);
}


static void branchtoalpha(int is_gsb, char buf[]) {
	unsigned int op, lbl;

	op = OP_DBL + (DBL_LBL << DBL_SHIFT);
	op |= buf[0] & 0xff;
	op |= (buf[1] & 0xff) << 16;
	op |= (buf[2] & 0xff) << 24;
	lbl = findmultilbl(op, FIND_OP_ERROR);

	do_multigto(is_gsb, lbl);
}

void cmdalphagto(unsigned int arg, enum rarg op) {
	char buf[12];

	xset(buf, '\0', sizeof(buf));
	branchtoalpha(op != RARG_ALPHAGTO, alpha_rcl_s(get_reg_n(arg), buf));
}

static void do_branchalpha(int is_gsb) {
	char buf[4];

	xcopy(buf, Alpha, 3);
	buf[3] = '\0';
	branchtoalpha(is_gsb, buf);
}

void op_gtoalpha(decimal64 *a, decimal64 *b, enum nilop op) {
	do_branchalpha((op == OP_GTOALPHA) ? 0 : 1);
}


// XEQ to an XROM command
static void do_xrom(int lbl) {
	const unsigned int oldpc = state_pc();
	const unsigned int pc = find_label_from(addrXROM(0), lbl, 0);

	UserLocalRegs = LocalRegs;
	gsbgto(pc, 1, oldpc);
}

static void xromargcommon(int lbl, unsigned int userpc) {
	if (userpc != 0) {
		XromUserPc = userpc;
		do_xrom(lbl);
	}
}

void xromarg(unsigned int arg, enum rarg op) {
	xromargcommon(ENTRY_SIGMA - (op - RARG_SUM), find_label_from(state_pc(), arg, 0));
}

void multixromarg(const opcode o, enum multiops mopr) {
	xromargcommon(ENTRY_SIGMA - (mopr - DBL_SUM), findmultilbl(o, FIND_OP_ERROR));
}

void xrom_routines(decimal64 *a, decimal64 *nul2, enum nilop op) {
	do_xrom(ENTRY_QUAD - (op - OP_QUAD));
}

void cmddisp(unsigned int arg, enum rarg op) {
	UState.dispdigs = arg;
	if (op != RARG_DISP)
		UState.dispmode = (op - RARG_STD) + MODE_STD;
	op_float(NULL, NULL, OP_FLOAT);
}


/* Metric / Imperial conversion code */
decNumber *convC2F(decNumber *r, const decNumber *x) {
	decNumber s;

	dn_multiply(&s, x, &const_9on5);
	return dn_add(r, &s, &const_32);
}

decNumber *convF2C(decNumber *r, const decNumber *x) {
	decNumber s;

	dn_subtract(&s, x, &const_32);
	return dn_divide(r, &s, &const_9on5);
}

decNumber *convDB2AR(decNumber *r, const decNumber *x) {
	decNumber t;
	dn_multiply(&t, x, &const_0_05);
	return decNumberPow10(r, &t);
}

decNumber *convAR2DB(decNumber *r, const decNumber *x) {
	decNumber t;
	dn_log10(&t, x);
	return dn_multiply(r, &t, &const_20);
}

decNumber *convDB2PR(decNumber *r, const decNumber *x) {
	decNumber t;
	dn_mulpow10(&t, x, -1);
	return decNumberPow10(r, &t);
}

decNumber *convPR2DB(decNumber *r, const decNumber *x) {
	decNumber t;
	dn_log10(&t, x);
	return dn_mulpow10(r, &t, 1);
}

/* Scale conversions */
void do_conv(decNumber *r, unsigned int arg, const decNumber *x) {
	decNumber m;
	const unsigned int conv = arg / 2;
	const unsigned int dirn = arg & 1;

	if (conv > NUM_CONSTS_CONV) {
		decNumberCopy(r, x);
		return;
	}

	decimal64ToNumber(&CONSTANT_CONV(conv), &m);
	
	if (dirn == 0)		// metric to imperial
		dn_divide(r, x, &m);
	else			// imperial to metric
		dn_multiply(r, x, &m);
}

void cmdconv(unsigned int arg, enum rarg op) {
	decNumber x, r;

	if (is_intmode())
		return;

	getX(&x);
	do_conv(&r, arg, &x);
	setlastX();
	setX(&r);
}

/*  Finish up a test -- if the value is non-zero, the test passes.
 *  If it is zero, the test fails.
 */
void fin_tst(const int a) {
	if (Running) {
		if (! a && incpc())
			decpc();
	}
	else
		DispMsg = a ? "true" : "false";
}


/* Skip a number of instructions forwards */
void cmdskip(unsigned int arg, enum rarg op) {
	unsigned int pc = state_pc();
	if (isXROM(pc))
		raw_set_pc(pc + arg);
	else {
		while (arg-- && !incpc());
		if (PcWrapped) {
			err(ERR_RANGE);
		}
	}
}

/* Skip backwards */
void cmdback(unsigned int arg, enum rarg op) {
	unsigned int pc = state_pc();
	if (isXROM(pc))
		pc -= arg + 1;
        else if (arg) {
		if ( Running ) {
			// Handles the case properly that we are on last step
			pc = do_dec(pc, 1);
		}
		do {
			pc = do_dec(pc, 1);
		} while (--arg && !PcWrapped);
		if (PcWrapped) {
			err(ERR_RANGE);
			return;
		}
	}
	raw_set_pc(pc);
}


/* We've encountered a CHS while entering the command line.
 */
static void cmdlinechs(void) {
	if (CmdLineEex) {
		const unsigned int pos = CmdLineEex + 1;
		if (CmdLineLength < pos) {
			if (CmdLineLength < CMDLINELEN)
				Cmdline[CmdLineLength++] = '-';
		} else if (Cmdline[pos] == '-') {
			if (CmdLineLength != pos)
				xcopy(Cmdline + pos, Cmdline + pos + 1, CmdLineLength-pos);
			CmdLineLength--;
		} else if (CmdLineLength < CMDLINELEN) {
			xcopy(Cmdline+pos+1, Cmdline+pos, CmdLineLength-pos);
			Cmdline[pos] = '-';
			CmdLineLength++;
		}
	} else {
		if (Cmdline[0] == '-') {
			if (CmdLineLength > 1)
				xcopy(Cmdline, Cmdline+1, CmdLineLength);
			CmdLineLength--;
		} else if (CmdLineLength < CMDLINELEN) {
			xcopy(Cmdline+1, Cmdline, CmdLineLength);
			Cmdline[0] = '-';
			CmdLineLength++;
		}
	}
}


/* Output a niladic function.
 */
static void niladic(const opcode op) {
	const unsigned int idx = argKIND(op);

	process_cmdline();
	if (idx < num_niladics) {
		if (is_intmode() && NILADIC_NOTINT(niladics[idx]))
			bad_mode_error();
		else if (! isNULL(niladics[idx].niladicf)) {
			decimal64 *x = NULL, *y = NULL;

			switch (NILADIC_NUMRESULTS(niladics[idx])) {
			case 2:	lift_if_enabled();
				y = &regY;
			case 1:	x = &regX;
				lift_if_enabled();
			default:
				CALL(niladics[idx].niladicf)(x, y, (enum nilop)idx);
				break;
			}
		}
	} else
		illegal(op);
	if (idx != OP_rCLX)
		State.state_lift = 1;
}


/* Execute a tests command
 */
static void do_tst(const decimal64 *cmp, const enum tst_op op, int cnst) {
	int a = 0;
	int iszero, isneg;

	process_cmdline_set_lift();

	if (is_intmode()) {
		unsigned long long int xv, yv;
		int xs, ys;

		xv = extract_value(d64toInt(&regX), &xs);
		if (cnst >= 0) {
			yv = cnst;
			ys = 1;
		} else
			yv = extract_value(d64toInt(cmp), &ys);

		if (xv == 0 && yv == 0)
			iszero = 1;
		else
			iszero = (xv == yv) && (xs == ys);

		if (xs == ys) {		// same sign
			if (xs)		// both negative
				isneg = xv > yv;
			else		// both positive
				isneg = xv < yv;
		} else
			isneg = xs;	// opposite signs
	} else {
		decNumber t, x, r;

		getX(&x);
		if (decNumberIsNaN(&x))
			goto flse;

		decimal64ToNumber(cmp, &t);
		if (decNumberIsNaN(&t))
			goto flse;

		if (op == TST_APX) {
			decNumberRnd(&x, &x);
			if (cnst < 0)
				decNumberRnd(&t, &t);
		}
		dn_compare(&r, &x, &t);
		iszero = dn_eq0(&r);
		isneg = decNumberIsNegative(&r);
	}

	switch (op) {
	case TST_APX:
	case TST_EQ:	a = iszero;		break;
	case TST_NE:	a = !iszero;		break;
	case TST_LT:	a = isneg && !iszero;	break;
	case TST_LE:	a = isneg || iszero;	break;
	case TST_GT:	a = !isneg && !iszero;	break;
	case TST_GE:	a = !isneg || iszero;	break;
	default:	a = 0;			break;
	}

flse:	fin_tst(a);
}

void check_zero(decimal64 *a, decimal64 *nul2, enum nilop op) {
	int neg;
	int zero;

	if (is_intmode()) {
		const unsigned long long int xv = extract_value(d64toInt(&regX), &neg);
		zero = (xv == 0);
	} else {
		decNumber x;
		getX(&x);
		neg = decNumberIsNegative(&x);
		zero = dn_eq0(&x);
	}
	if (op == OP_Xeq_pos0)
		fin_tst(zero && !neg);
	else /* if (op == OP_Xeq_neg0) */
		fin_tst(zero && neg);
}

void cmdtest(unsigned int arg, enum rarg op) {
	do_tst(get_reg_n(arg), (enum tst_op)(op - RARG_TEST_EQ), -1);
}

static void do_ztst(const decimal64 *r, const decimal64 *i, const enum tst_op op) {
	int c = 0;
	decNumber x, y, t, a, b;
	int eq = 1;

	process_cmdline_set_lift();

	if (is_intmode()) {
		bad_mode_error();
		return;
	}
	getXY(&x, &y);
	if (decNumberIsNaN(&x) || decNumberIsNaN(&y))
		goto flse;
	decimal64ToNumber(r, &a);
	decimal64ToNumber(i, &b);
	if (decNumberIsNaN(&a) || decNumberIsNaN(&b))
		goto flse;
#if 0
	if (op == TST_APX) {
		decNumberRnd(&x, &x);
		decNumberRnd(&y, &y);
		decNumberRnd(&a, &a);
		decNumberRnd(&b, &b);
	}
#endif
	dn_compare(&t, &x, &a);
	if (!dn_eq0(&t))
		eq = 0;
	else {
		dn_compare(&t, &y, &b);
		if (!dn_eq0(&t))
			eq = 0;
	}
	if (op != TST_NE)
		c = eq;
	else
		c = !eq;
flse:	fin_tst(c);
}

void cmdztest(unsigned int arg, enum rarg op) {
	do_ztst(get_reg_n(arg), get_reg_n(arg+1), (enum tst_op)(op - RARG_TEST_ZEQ));
}

static int incdec(unsigned int arg, int inc) {
	if (is_intmode()) {
		long long int x = get_reg_n_as_int(arg);
		int xs;
		unsigned long long int xv;

		if (inc)
			x = intAdd(x, 1LL);
		else
			x = intSubtract(x, 1LL);
		put_reg_n_from_int(arg, x);

		xv = extract_value(x, &xs);
		return xv != 0;
	} else {
		decNumber x, y;

		get_reg_n_as_dn(arg, &x);
		if (inc)
			dn_inc(&x);
		else
			dn_dec(&x);
		put_reg_n(arg, &x);
		decNumberTrunc(&y, &x);
		return ! dn_eq0(&y);
	}
}

void cmdlincdec(unsigned int arg, enum rarg op) {
	incdec(arg, op == RARG_INC);
}

void cmdloopz(unsigned int arg, enum rarg op) {
	fin_tst(incdec(arg, op == RARG_ISZ));
}

void cmdloop(unsigned int arg, enum rarg op) {
	if (is_intmode()) {
		long long int x = get_reg_n_as_int(arg);
		int xs;
		unsigned long long int xv;

		if (op == RARG_ISG || op == RARG_ISE)
			x = intAdd(x, 1LL);
		else
			x = intSubtract(x, 1LL);
		put_reg_n_from_int(arg, x);

		xv = extract_value(x, &xs);
		if (op == RARG_ISG)
			fin_tst(! (xs == 0 && xv > 0));		// > 0
		else if (op == RARG_DSE)
			fin_tst(! (xs != 0 || xv == 0));	// <= 0
		else if (op == RARG_ISE)
			fin_tst(! (xs == 0 || xv == 0));	// >= 0
		else // if (op == RARG_DSL)
			fin_tst(! (xs != 0 && xv > 0));		// < 0
		return;
	} else {
		decNumber x, i, f, n, u;

		get_reg_n_as_dn(arg, &x);

		// Break the number into the important bits
		// nnnnn.fffii
		dn_abs(&f, &x);
		decNumberTrunc(&n, &f);			// n = nnnnn
		dn_subtract(&u, &f, &n);		// u = .fffii
		if (decNumberIsNegative(&x))
			dn_minus(&n, &n);
		dn_mulpow10(&i, &u, 3);			// i = fff.ii
		decNumberTrunc(&f, &i);			// f = fff
		dn_subtract(&i, &i, &f);		// i = .ii		
		dn_mul100(&x, &i);
		decNumberTrunc(&i, &x);			// i = ii
		if (dn_eq0(&i))
			dn_1(&i);

		if (op == RARG_ISG || op == RARG_ISE) {
			dn_add(&n, &n, &i);
			dn_compare(&x, &f, &n);
			if (op == RARG_ISE)
				fin_tst(dn_gt0(&x));
			else
				fin_tst(! dn_lt0(&x));
		} else {
			dn_subtract(&n, &n, &i);
			dn_compare(&x, &f, &n);
			if (op == RARG_DSL)
				fin_tst(dn_le0(&x));
			else
				fin_tst(dn_lt0(&x));
		}

		// Finally rebuild the result
		if (decNumberIsNegative(&n)) {
			dn_subtract(&x, &n, &u);
		} else
			dn_add(&x, &n, &u);
		put_reg_n(arg, &x);
	}
}


/* Shift a real number by 10 to the specified power
 */
void op_shift_digit(unsigned int n, enum rarg op) {
	decNumber x;
	int adjust = n;

	if (is_intmode()) {
		bad_mode_error();
		return;
	}
	getX(&x);
	setlastX();
	if (decNumberIsSpecial(&x) || dn_eq0(&x))
		return;
	if (op == RARG_SRD)
		adjust = -adjust;
	x.exponent += adjust;
	setX(&x);
}


/* Return a pointer to the byte with the indicated flag in it.
 * also return a byte with the relevant bit mask set up.
 * Also, handle local flags.
 */
static unsigned short int *flag_word(int n, unsigned short int *mask) {
	unsigned short int *p = UserFlags;
	if (n >= LOCAL_FLAG_BASE) {
		const int l = local_levels();
		n -= LOCAL_FLAG_BASE;
		if (l == 1) {
			// XROM special
			p = &XromFlags;
		}
		else if (LocalRegs & 1) {
			// Odd frame: flags are at end of frame
			p = RetStk + LocalRegs + l - 1;
		}
		else {
			// Even frame: Flags are at beginning of frame
			p = RetStk + LocalRegs + 1;
		}
	}
	if (mask != NULL)
		*mask = 1 << (n & 15);
	return p + (n >> 4);
}

int get_user_flag(int n) {
	unsigned short mask;
	const unsigned short *const f = flag_word(n, &mask);

	return (*f & mask)? 1 : 0;
}

void put_user_flag(int n, int f) {
	if (f)	set_user_flag(n);
	else	clr_user_flag(n);
}

#ifndef set_user_flag
void set_user_flag(int n) {
	unsigned short mask;
	unsigned short *const f = flag_word(n, &mask);

	*f |= mask;
}

void clr_user_flag(int n) {
	unsigned short mask;
	unsigned short *const f = flag_word(n, &mask);

	*f &= ~mask;
}
#endif

void cmdflag(unsigned int arg, enum rarg op) {
	unsigned short mask;
	unsigned short *const f = flag_word(arg, &mask);
	int flg = *f & mask;

	switch (op) {
	case RARG_SF:	flg = 1;			   break;
	case RARG_CF:	flg = 0;			   break;
	case RARG_FF:	flg = flg? 0 : 1;		   break;

	case RARG_FS:	fin_tst(flg);			   return;
	case RARG_FC:	fin_tst(! flg);			   return;

	case RARG_FSC:	fin_tst(flg); flg = 0;		   break;
	case RARG_FSS:	fin_tst(flg); flg = 1;		   break;
	case RARG_FSF:	fin_tst(flg); flg = flg ? 0 : 1;   break;

	case RARG_FCC:	fin_tst(! flg);	flg = 0;	   break;
	case RARG_FCS:	fin_tst(! flg);	flg = 1;	   break;
	case RARG_FCF:	fin_tst(! flg);	flg = flg ? 0 : 1; break;

	default:					
		return;
	}

	// And write the value back
	if (flg)
		*f |= mask;
	else
		*f &= ~mask;

	if ( arg == A_FLAG ) {
		dot( BIG_EQ, flg );
		finish_display();
	}
}

/* Reset all flags to off/false
 */
void clrflags(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	xset(UserFlags, 0, sizeof(UserFlags));
	if (LocalRegs < 0) {
		* flag_word(LOCAL_REG_BASE, NULL) = 0;
	}
}


/* Integer word size
 */
void intws(unsigned int arg, enum rarg op) {
	if (is_intmode()) {
		int i, ss = stack_size();
		unsigned int oldlen = UState.int_len;
		long long int v;

		for (i=0; i<ss; i++) {
			v = get_reg_n_as_int(regX_idx + i);
			UState.int_len = arg;
			put_reg_n_from_int(regX_idx + i, mask_value(v));
			UState.int_len = oldlen;
		}
		v = get_reg_n_as_int(regL_idx);
		UState.int_len = arg;
		put_reg_n_from_int(regL_idx, mask_value(v));
	} else
	    UState.int_len = arg;
}


/* Convert from a real to a fraction
 */

void get_maxdenom(decNumber *d) {
	const unsigned int dm = UState.denom_max;
	int_to_dn(d, dm==0?9999:dm);
}

void op_2frac(decimal64 *x, decimal64 *b, enum nilop op) {
	decNumber z, n, d, t;

	if (UState.intm) {
		d64fromInt(x, 1);
		return;
	}

	getY(&z);			// Stack has been lifted already
	decNumber2Fraction(&n, &d, &z);
	setXY(&d, &n);			// Set numerator and denominator
	if (State2.runmode) {
		dn_divide(&t, &n, &d);
		dn_compare(&n, &t, &z);
		if (dn_eq0(&n))
			DispMsg = "y/x =";
		else if (decNumberIsNegative(&n))
			DispMsg = "y/x \017";
		else
			DispMsg = "y/x \020";
	}
}

void op_fracdenom(decimal64 *a, decimal64 *b, enum nilop op) {
	int s;
	unsigned long long int i;

	i = get_int(&regX, &s);
	if (i > 9999)
		UState.denom_max = 0;
	else if (i != 1)
		UState.denom_max = (unsigned int) i;
	else {
		setlastX();
		put_int(UState.denom_max, 0, &regX);
	}
}

void op_denom(decimal64 *a, decimal64 *b, enum nilop op) {
	UState.denom_mode = DENOM_ANY + (op - OP_DENANY);
}


/* Switching from an integer mode to real mode requires us
 * to make an effort at converting x and y into a real numbers
 * x' = y . 2^x
 */
#ifdef HP16C_MODE_CHANGE
static void int2dn(decNumber *x, decimal64 *a) {
	int s;
	unsigned long long int v = extract_value(d64toInt(a), &s);

	ullint_to_dn(x, v);
	if (s)
		dn_minus(x, x);
}
#else
static void float_mode_convert(decimal64 *r) {
	decNumber x;
	int s;
	unsigned long long int v = extract_value(d64toInt(r), &s);

	ullint_to_dn(&x, v);
	if (s)
		dn_minus(&x, &x);
	packed_from_number(r, &x);
}
#endif

void op_float(decimal64 *a, decimal64 *b, enum nilop op) {
#ifdef HP16C_MODE_CHANGE
	decNumber x, y, z;
#else
	int i;
#endif

	if (is_intmode()) {
		UState.intm = 0;
		// UState.int_len = 0;
#ifdef HP16C_MODE_CHANGE
		int2dn(&x, &regX);
		int2dn(&y, &regY);
		clrstk(NULL, NULL, NULL);
		decNumberPower(&z, &const_2, &x);
		dn_multiply(&x, &z, &y);
		set_overflow(decNumberIsInfinite(&x));
		packed_from_number(&regX, &x);
#else
		for (i=0; i<stack_size(); i++)
			float_mode_convert(get_stack(i));
		float_mode_convert(&regL);
#endif
	}
	UState.fract = 0;
        State2.hms = (op == OP_HMS) ? 1 : 0;
}

void op_fract(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	op_float(NULL, NULL, OP_FLOAT);
	UState.fract = 1;
	if (op == OP_FRACIMPROPER)
		UState.improperfrac = 1;
	else if (op == OP_FRACPROPER)
		UState.improperfrac = 0;
}

static int is_digit(const char c) {
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

static int is_xdigit(const char c) {
	if (is_digit(c) || (c >= 'A' && c <= 'F'))
		return 1;
	return 0;
}

/* Process a single digit.
 */
static void digit(unsigned int c) {
	const int intm = is_intmode();
	int i, j;
	int lim = 12;

	if (CmdLineLength >= CMDLINELEN) {
		warn(ERR_TOO_LONG);
		return;
	}
	if (intm) {
		if (c >= int_base()) {
			warn(ERR_DIGIT);
			return;
		}
		for (i=j=0; i<(int)CmdLineLength; i++)
			j += is_xdigit(Cmdline[i]);
		if (j == lim) {
			warn(ERR_TOO_LONG);
			return;
		}
		if (c >= 10) {
			Cmdline[CmdLineLength++] = c - 10 + 'A';
			return;
		}
	} else {
		if (c >= 10) {
			warn(ERR_DIGIT);
			return;
		}
		for (i=j=0; i<(int)CmdLineLength; i++)
			if (Cmdline[i] == 'E') {
				lim++;
				break;
			} else
				j += is_digit(Cmdline[i]);
		if (j == lim) {
			warn(ERR_TOO_LONG);
			return;
		}
	}

	Cmdline[CmdLineLength++] = c + '0';
	Cmdline[CmdLineLength] = '\0';

	if (! intm && CmdLineEex) {
		char *p = &Cmdline[CmdLineEex + 1];
		int emax = 384;
		int n;

		/* Figure out the range limit for the exponent */
		if (*p == '-') {
			p++;
			emax = 383;
		}

		/* Now, check if the current exponent exceeds the range.
		 * If so, shift it back a digit and validate a second time
		 * in case the first digit is too large.
		 */
		for (n=0; n<2; n++) {
			if (s_to_i(p) > emax) {
				int i;

				for (i=0; p[i] != '\0'; i++)
					p[i] = p[i+1];
				CmdLineLength--;
				Cmdline[CmdLineLength] = '\0';
			} else
				break;
		}
	}
}


void set_entry() {
	State.entryp = 1;
}


/* Decode and process the specials.  These are niladic functions and
 * commands with non-standard stack operation.
 */
static void specials(const opcode op) {
	int opm = argKIND(op);

	switch (opm) {
	case OP_0:	case OP_1:	case OP_2:
	case OP_3:	case OP_4:	case OP_5:
	case OP_6:	case OP_7:	case OP_8:
	case OP_9:	case OP_A:	case OP_B:
	case OP_C:	case OP_D:	case OP_E:
	case OP_F:
		digit(opm - OP_0);
		break;

	case OP_DOT:
		if (is_intmode())
			break;
		if (CmdLineDot < 2 && !CmdLineEex && CmdLineLength < CMDLINELEN) {
			if (CmdLineLength == 0 || Cmdline[CmdLineLength-1] == '.')
				digit(0);
			CmdLineDot++;
			Cmdline[CmdLineLength++] = '.';
		}
		break;

	case OP_EEX:
		if (is_intmode() || UState.fract || CmdLineDot == 2)
			break;
		if (!CmdLineEex && CmdLineLength < CMDLINELEN) {
			if (CmdLineLength == 0)
				digit(1);
			CmdLineEex = CmdLineLength;
			Cmdline[CmdLineLength++] = 'E';
		}
		break;

	case OP_CHS:
		if (CmdLineLength)
			cmdlinechs();
		else if (is_intmode()) {
			d64fromInt(&regX, intChs(d64toInt(&regX)));
			State.state_lift = 1;
		} else {
			decNumber x, r;

			getX(&x);
			dn_minus(&r, &x);
			setX(&r);
			State.state_lift = 1;
		}
		break;

	case OP_CLX:
		if (Running)
			illegal(op);
		else if (CmdLineLength) {
			CmdLineLength--;
			if (Cmdline[CmdLineLength] == 'E')
				CmdLineEex = 0;
			else if (Cmdline[CmdLineLength] == '.')
				CmdLineDot--;
		} else
			clrx(NULL, NULL, OP_rCLX);
		break;

	case OP_ENTER:
		process_cmdline();
		lift();
		State.state_lift = 0;
		break;

	case OP_SIGMAPLUS:
	case OP_SIGMAMINUS:
		if (is_intmode()) {
			bad_mode_error();
			break;
		}
		process_cmdline();
		State.state_lift = 0;
		setlastX();
		if (opm == OP_SIGMAPLUS)
			sigma_plus();
		else
			sigma_minus();
		sigma_val(&regX, NULL, OP_sigmaN);
		break;

	// Conditional tests vs registers....
	case OP_Xeq0:	case OP_Xlt0:	case OP_Xgt0:
	case OP_Xne0:	case OP_Xle0:	case OP_Xge0:
	case OP_Xapx0:
		do_tst(&CONSTANT_INT(OP_ZERO), (enum tst_op)(opm - OP_Xeq0), 0);
		break;
	case OP_Zeq0: case OP_Zne0:
	//case OP_Zapr0:
		do_ztst(&CONSTANT_INT(OP_ZERO), &CONSTANT_INT(OP_ZERO), (enum tst_op)(opm - OP_Zeq0));
		break;

	case OP_Xeq1:	case OP_Xlt1:	case OP_Xgt1:
	case OP_Xne1:	case OP_Xle1:	case OP_Xge1:
	case OP_Xapx1:
		do_tst(&CONSTANT_INT(OP_ONE), (enum tst_op)(opm - OP_Xeq1), 1);
		break;
	case OP_Zeq1:	case OP_Zne1:
	//case OP_Zapx1:
		do_ztst(&CONSTANT_INT(OP_ONE), &CONSTANT_INT(OP_ZERO), (enum tst_op)(opm - OP_Zeq1));
		break;

	default:
		illegal(op);
	}
}

enum trig_modes get_trig_mode(void) {
	if (State2.cmplx)
		return TRIG_RAD;
	//if (State2.hyp)	return TRIG_RAD;
	return UState.trigmode;
}

static void set_trig_mode(enum trig_modes m) {
	UState.trigmode = m;
}

void op_trigmode(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	set_trig_mode((enum trig_modes)(TRIG_DEG + (op - OP_DEG)));
}

void op_radix(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	UState.fraccomma = (op == OP_RADCOM) ? 1 : 0;
	UState.fract = 0;
}


void op_separator(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	int x = (op - OP_THOUS_ON);
	int state = x & 1;
	if ((x&2) != 0)
		UState.nointseparator = state;
	else
		UState.nothousands = state;
}

void op_fixscieng(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	UState.fixeng = (op == OP_FIXSCI) ? 0 : 1;
	UState.fract = 0;
}

#ifdef INCLUDE_DOUBLE_PRECISION
void op_double(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	State2.mode_double = (op == OP_DBLON) ? 1 : 0;
}
#endif

void op_pause(unsigned int arg, enum rarg op) {
	display();
#ifndef CONSOLE
	// decremented in the low level heartbeat
	Pause = arg;
	GoFast = (arg == 0);
#else
#if defined(WIN32) && !defined(__GNUC__)
#pragma warning(disable:4996)
	sleep(arg/10);
#else
	usleep(arg * 100000);
#endif
#endif
}

void op_intsign(decimal64 *a, decimal64 *b, enum nilop op) {
	UState.int_mode = (op - OP_2COMP) + MODE_2COMP;
}


/* Switch to integer mode.
 * If we're coming from real mode we do funny stuff with the stack,
 * if we're already in int mode we leave alone.
 *
 * We take the real X register and put it into the x and y registers
 * such that the mantissa is in y and the exponent is in x.  There
 * is also an additional condition that 2^31 <= |y| < 2^32.
 *
 * Since the word size gets reset when we enter real mode, there is
 * plenty of space to do this and overflow isn't possible -- we have
 * to account for zero, infinities and NaNs.
 */
#ifndef HP16C_MODE_CHANGE
void int_mode_convert(decimal64 *r) {
	decNumber x;
	int s;
	unsigned long long int n;

	decimal64ToNumber(r, &x);
        decNumberTrunc(&x, &x);
	n = dn_to_ull(&x, &s);
	d64fromInt(r, build_value(n, s));
}
#endif

static void check_int_switch(void) {
	if (!is_intmode()) {
#ifdef HP16C_MODE_CHANGE
		decNumber x, y, z;
		int ex;			/* exponent |ex| < 1000 */
		unsigned long int m;	/* Mantissa 32 bits */
		int sgn, i;

		getX(&x);
		lift();
		if (decNumberIsSpecial(&x)) {
			/* Specials all have 0 mantissa and a coded exponent
			 * We cannot use +/- a number for the infinities since
			 * we might be in unsigned mode so we code them as 1 & 2.
			 * NaN's get 3.
			 */
			d64fromInt(&regY, 0);
			if (decNumberIsNaN(&x))
				d64fromInt(&regX, 3);
			else if (decNumberIsNegative(&x))
				d64fromInt(&regX, 2);
			else
				d64fromInt(&regX, 1);

		} else if (dn_eq0(&x)) {
			/* 0 exponent, 0 mantissa -- although this can be negative zero */
			d64fromInt(&regY, build_value(0, decNumberIsNegative(&x)?1:0));
			d64fromInt(&regX, 0);
		} else {
			/* Deal with the sign */
			if (decNumberIsNegative(&x)) {
				dn_minus(&x, &x);
				sgn = 1;
			} else
				sgn = 0;
			/* Figure the exponent */
			decNumberLog2(&y, &x);
			decNumberTrunc(&z, &y);
			ex = dn_to_int(&z);
			/* On to the mantissa */
			decNumberPow2(&y, &z);
			dn_divide(&z, &x, &y);
			m = 1;
			decNumberFrac(&y, &z);
			for (i=0; i<31; i++) {
				dn_mul2(&z, &y);
				decNumberTrunc(&y, &z);
				m += m;
				if (! dn_eq0(&y))
					m++;
				decNumberFrac(&y, &z);
			}
			ex -= 31;
			/* Finally, round up if required */
			dn_mul2(&z, &y);
			decNumberTrunc(&y, &z);
			if (! dn_eq0(&y)) {
				m++;
				if (m == 0) {
					ex++;
					m = 0x80000000;
				}
			}
			/* The mantissa */
			d64fromInt(&regY, build_value(m, sgn));
			/* The exponent */
			if (ex < 0) {
				ex = -ex;
				sgn = 1;
			} else
				sgn = 0;
			d64fromInt(&regX, build_value(ex, sgn));
		}
#else
		int i;
		for (i=0; i<stack_size(); i++)
			int_mode_convert(get_stack(i));
		int_mode_convert(&regL);
#endif
		UState.intm = 1;
	}
}

static void set_base(unsigned int b) {
	UState.int_base = b - 1;
	check_int_switch();
}

void set_int_base(unsigned int arg, enum rarg op) {
	if (arg < 2) {
		if (arg == 0)
			op_float(NULL, NULL, OP_FLOAT);
		else
			op_fract(NULL, NULL, OP_FRACT);
	} else
		set_base(arg);
}

#if 0
void op_locale(decimal64 *a, decimal64 *nul, enum nilop op) {
	enum {
		LOCALE_RADIX_COM=1,	LOCALE_RADIX_DOT=0,
		LOCALE_TIME_24=2,	LOCALE_TIME_12=0,
		LOCALE_THOUS_OFF=4,	LOCALE_THOUS_ON=0,
		LOCALE_JG1582=8,	LOCALE_JG1752=0,
		LOCALE_DATE_MDY=16,	LOCALE_DATE_DMY=0,
		LOCALE_DATE_YMD=32,
	};
	static const unsigned char locales[] = {
		// Europe
		LOCALE_RADIX_COM | LOCALE_THOUS_ON | LOCALE_TIME_24 |
			LOCALE_JG1582 | LOCALE_DATE_DMY,
		// UK/British
		LOCALE_RADIX_DOT | LOCALE_THOUS_ON | LOCALE_TIME_12 |
			LOCALE_JG1752 | LOCALE_DATE_DMY,
		// USA
		LOCALE_RADIX_DOT | LOCALE_THOUS_ON | LOCALE_TIME_12 |
			LOCALE_JG1752 | LOCALE_DATE_MDY,
		// India
		LOCALE_RADIX_DOT | LOCALE_THOUS_OFF | LOCALE_TIME_24 |
			LOCALE_JG1752 | LOCALE_DATE_DMY,
		// China
		LOCALE_RADIX_DOT | LOCALE_THOUS_OFF | LOCALE_TIME_24 |
			LOCALE_JG1752 | LOCALE_DATE_YMD,
		// Japan
		LOCALE_RADIX_DOT | LOCALE_THOUS_ON | LOCALE_TIME_24 |
			LOCALE_JG1752 | LOCALE_DATE_YMD,
	};
	const unsigned char f = locales[op - OP_SETEUR];

	op_radix(NULL, NULL, (f & LOCALE_RADIX_COM) ? OP_RADCOM : OP_RADDOT);
	//op_separator(NULL, NULL, (f & LOCALE_THOUS_OFF) ? OP_THOUS_OFF : OP_THOUS_ON);
	UState.nothousands = (f & LOCALE_THOUS_OFF) ? 1 : 0;

	op_timemode(NULL, NULL, (f & LOCALE_TIME_24) ? OP_24HR : OP_12HR);
	op_datemode(NULL, NULL, (f & LOCALE_DATE_MDY) ? OP_DATEMDY : ((f & LOCALE_DATE_YMD) ? OP_DATEYMD : OP_DATEDMY));
	op_jgchange(NULL, NULL, (f & LOCALE_JG1582) ? OP_JG1582 : OP_JG1752);
}
#endif

void op_datemode(decimal64 *a, decimal64 *nul, enum nilop op) {
	UState.date_mode = (op - OP_DATEDMY) + DATE_DMY;
}

void op_timemode(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	UState.t12 = (op == OP_12HR) ? 1 : 0;
}

void op_setspeed(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	UState.slow_speed = (op == OP_SLOW) ? 1 : 0;
	update_speed(1);
}

void op_rs(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	if (Running)
		set_running_off();
	else {
		set_running_on();
		if (RetStkPtr == 0)
			RetStk[--RetStkPtr] = state_pc();
	}
}

void op_prompt(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	set_running_off();
	alpha_view_common(regX_idx);
}

// XEQUSR
// Command pushes 4 values on stack, needs to be followed by POPUSR
void do_usergsb(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	const unsigned int pc = state_pc();
	if (isXROM(pc) && XromUserPc != 0) {
		gsbgto(pc, 1, XromUserPc);    // push address of callee
		gsbgto(pc, 1, LocalRegs);     // push my local registers 
		gsbgto(pc, 1, UserLocalRegs); // push former local registers
		gsbgto(XromUserPc, 1, pc);    // push return address, transfer control
		XromUserPc = 0;
		LocalRegs = UserLocalRegs;    // reestablish user environment
	}
}

// POPUSR
void op_popusr(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	if (isXROM(state_pc())) {
		UserLocalRegs = RetStk[RetStkPtr++]; // previous local registers
		LocalRegs =     RetStk[RetStkPtr++]; // my local registers
		XromUserPc =    RetStk[RetStkPtr++]; // adress of callee
	}
}

/* Tests if the user program is at the top level */
void isTop(decimal64 *a, decimal64 *b, enum nilop op) {
	int top = 0;
	
	if (Running) {
		top = RetStkPtr >= -1 - local_levels();
	}
	fin_tst(top);
}

/* Test if a number is an integer or fractional */
/* Special numbers are neither */
void XisInt(decimal64 *a, decimal64 *b, enum nilop op) {
	decNumber x;
	int result, op_int = (op == OP_XisINT);
	if ( is_intmode() )
		result = op_int;
	else if (decNumberIsSpecial(getX(&x)))
		result = 0;
	else
	        result = (is_int(&x) == op_int);
	fin_tst(result);
}

/* Test if a number is an even or odd integer */
/* fractional or special values are neither even nor odd */
void XisEvenOrOdd(decimal64 *a, decimal64 *b, enum nilop op) {
	decNumber x;
	int odd = (op == OP_XisODD);

	if (is_intmode()) {
		fin_tst((d64toInt(&regX) & 1) == odd);
	} else {
		fin_tst(is_even(getX(&x)) == !odd);
	}
}


/* Test if a number is prime */
void XisPrime(decimal64 *a, decimal64 *b, enum nilop op) {
	int sgn;

	fin_tst(isPrime(get_int(&regX, &sgn)) && sgn == 0);
}

/* Test is a number is infinite.
 */
void isInfinite(decimal64 *a, decimal64 *b, enum nilop op) {
	decNumber x;

	getX(&x);
	fin_tst(!is_intmode() && decNumberIsInfinite(&x));
}

/* Test for NaN.
 * this could be done by testing x != x, but having a special command
 * for it reads easier.
 */
void isNan(decimal64 *a, decimal64 *b, enum nilop op) {
	decNumber x;

	getX(&x);
	fin_tst(!is_intmode() && decNumberIsNaN(&x));
}

void isSpecial(decimal64 *a, decimal64 *b, enum nilop op) {
	decNumber x;

	getX(&x);
	fin_tst(!is_intmode() && decNumberIsSpecial(&x));
}

void op_entryp(decimal64 *a, decimal64 *b, enum nilop op) {
	fin_tst(State.entryp);
}

/* Bulk register operations */
static int reg_decode(decimal64 **s, unsigned int *n, decimal64 **d, int flash) {
	decNumber x, y;
	int rsrc, num, rdest, q, mx_src, mx_dest;

	if (is_intmode())
		return 1;

	getX(&x);			// sss.nnddd~
	dn_mulpow10(&y, &x, 2 + 3);	// sssnnddd.~
	decNumberTrunc(&x, &y);		// sssnnddd.0
	rsrc = dn_to_int(&x);		// sssnnddd

	if (rsrc < 0) {
		if (!flash)
			goto range_error;
		rsrc = -rsrc;
	}
	else 
		flash = 0;

	rdest = rsrc % 1000;		// ddd
	rsrc /= 1000;			// sssnn
	num = rsrc % 100;		// nn
	rsrc /= 100;			// sss

	mx_src = flash ? BackupFlash._numregs :
		 rsrc >= LOCAL_REG_BASE ? local_regs() : NumRegs;
	if (rsrc >= mx_src)
		goto range_error;

	if (d != NULL) {
		mx_dest = rdest >= LOCAL_REG_BASE ? local_regs() : NumRegs;

		if (num == 0) {
			/* Calculate the maximum non-overlapping size */
			if (flash || (rsrc >= LOCAL_REG_BASE) != (rdest >= LOCAL_REG_BASE))
				// source & destination in different memory areas
				num = mx_dest < mx_src ? mx_dest : mx_src;
			else {
				if (rsrc > rdest) {
					num = mx_src - rsrc;
					q = rsrc - rdest;
				} 
				else {
					num = mx_dest - rdest;
					q = rdest - rsrc;
				}
				if (num > q)
					num = q;
			}
		}
		if (rdest >= LOCAL_REG_BASE)
			mx_dest += LOCAL_REG_BASE;
		if (rdest + num > mx_dest)
			goto range_error;
		// Set pointer
		*d = get_reg_n(rdest);
	}
	else {
		if (num == 0) {
			num = mx_src - rsrc;
		}
	}
	if (rsrc >= LOCAL_REG_BASE)
		mx_src += LOCAL_REG_BASE;
	if (rsrc + num > mx_src)
		goto range_error;

	// Now point to the correct source register
	*s = flash ? get_flash_reg_n(rsrc) : get_reg_n(rsrc);
	*n = num;

	return 0;

range_error:
	err(ERR_RANGE);
	return 1;
}

void op_regcopy(decimal64 *a, decimal64 *b, enum nilop op) {
	decimal64 *s, *d;
	unsigned int n;

	if (reg_decode(&s, &n, &d, 1))
		return;
	move_regs(d, s, n);
}

void op_regswap(decimal64 *a, decimal64 *b, enum nilop op) {
	decimal64 *s, *d;
	unsigned int n, i;

	if (reg_decode(&s, &n, &d, 0) || s == d)
		return;
	if ((s < d && (s + n) > d) || (d < s && (d + n) > s))
		err(ERR_RANGE);
	else {
		for (i = 0; i < n; i++)
			swap_reg(s + i, d + i);
	}
}

void op_regclr(decimal64 *a, decimal64 *b, enum nilop op) {
	decimal64 *s;
	unsigned int n;

	if (reg_decode(&s, &n, NULL, 0))
		return;
	zero_regs(s, n);
}

void op_regsort(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	decimal64 *s;
	unsigned int n;
	decNumber pivot, a, t;
	int beg[10], end[10], i;

	if (reg_decode(&s, &n, NULL, 0) || n == 1)
		return;

	/* Non-recursive quicksort */
	beg[0] = 0;
	end[0] = n;
	i = 0;
	while (i>=0) {
		int L = beg[i];
		int R = end[i] - 1;
		if (L<R) {
			const decimal64 pvt = s[L];
			decimal64ToNumber(&pvt, &pivot);
			while (L<R) {
				while (L<R) {
					decimal64ToNumber(s + R, &a);
					if (dn_lt0(dn_compare(&t, &a, &pivot)))
						break;
					R--;
				}
				if (L<R)
					s[L++] = s[R];
				while (L<R) {
					decimal64ToNumber(s + L, &a);
					if (dn_lt0(dn_compare(&t, &pivot, &a)))
						break;
					L++;
				}
				if (L<R)
					s[R--] = s[L];
			}
			s[L] = pvt;
			if (L - beg[i] < end[i] - (L+1)) {
				beg[i+1] = beg[i];
				end[i+1] = L;
				beg[i] = L+1;
			} else {
				beg[i+1] = L+1;
				end[i+1] = end[i];
				end[i] = L;
			}
			i++;
		} else
			i--;
	}
}


/* Handle a command that takes an argument.  The argument is encoded
 * in the low order bits of the opcode.  We also have to take
 * account of the indirection flag and various limits -- we always work modulo
 * the limit.
 */
static void rargs(const opcode op) {
	unsigned int arg = op & RARG_MASK;
	int ind = op & RARG_IND;
	const unsigned int cmd = RARG_CMD(op);
	decNumber x;
	unsigned int lim = argcmds[cmd].lim;

	XeqOpCode = (s_opcode) cmd;
	if (lim == 0) lim = 256; // default

	process_cmdline();

	if (cmd >= num_argcmds) {
		illegal(op);
		return;
	}
	if (isNULL(argcmds[cmd].f)) {
		State.state_lift = 1;
		return;
	}

	if (ind && argcmds[cmd].indirectokay) {
		if (is_intmode()) {
			arg = (unsigned int) get_reg_n_as_int(arg);
		} else {
			get_reg_n_as_dn(arg, &x);
			arg = dn_to_int(&x);
		}
	} else {
		if (lim > 128 && ind)		// put the top bit back in
			arg |= RARG_IND;
	}
	if (argcmds[cmd].reg && arg < TOPREALREG) {
		// Range checking for registers against variable boundary
		lim = NumRegs;
		if (argcmds[cmd].cmplx)
			--lim;
	}
	else if (argcmds[cmd].local) {
		// Range checking for local registers or flags
		lim = NUMREG + local_regs();
		if (argcmds[cmd].cmplx)
			--lim;
		else if (argcmds[cmd].stos)
			lim -= stack_size() - 1;
		if ((int)arg < 0)
			arg = NUMREG - (int)arg;
	}
	else if (argcmds[cmd].flag) {
		if (LocalRegs == 0)
			lim -= 16;
		if ((int)arg < 0)
			arg = NUMFLG - (int)arg;
	}
	if (arg >= lim )
		err(ERR_RANGE);
	else if (argcmds[cmd].cmplx && arg >= TOPREALREG-1 && arg < NUMREG && (arg & 1))
		err(ERR_ILLEGAL);
	else {
		CALL(argcmds[cmd].f)(arg, (enum rarg)cmd);
		State.state_lift = 1;
	}
}

static void multi(const opcode op) {
	const int cmd = opDBL(op);
	XeqOpCode = (s_opcode) cmd;

	process_cmdline_set_lift();

	if (cmd >= num_multicmds) {
		illegal(op);
		return;
	}
	if (isNULL(multicmds[cmd].f))	// LBL does nothing
		return;
	CALL(multicmds[cmd].f)(op, (enum multiops)cmd);
}


/* Print a single program step nicely.
 */
static void print_step(const opcode op) {
	char buf[16];
	const unsigned int pc = state_pc();
	char *p = TraceBuffer;

	if (isXROM(pc)) {
		*p++ = 'x';
	} else if (isLIB(pc)) {
		p = num_arg_0(p, nLIB(pc), 1);
		*p++ = ' ';
	} 
	if (pc == 0)
		scopy(p, "000:");
	else {
		p = num_arg_0(p, user_pc(), 3);
		*p++ = ':';
		scopy_char(p, prt(op, buf), '\0');
		if (*p == '?')
			*p = '\0';
	}
	State2.disp_small = 1;
	DispMsg = TraceBuffer;
}


/* When stuff gets done, there are some bits of state that need
 * to be reset -- SHOW, ->base change the display mode until something
 * happens.  This should be called on that something.
 */
void reset_volatile_state(void) {
	State2.int_window = 0;
	UState.int_maxw = 0;
	State2.smode = SDISP_NORMAL;
}


/*
 *  Called by any long running function
 */
void busy(void)
{
	/*
	 *  Serve the hardware watch dog
	 */
	watchdog();

	/*
	 *  Increase the speed
	 */
	update_speed(1);

	/*
	 *  Indicate busy state to the user
	 */
	if ( !Busy && !Running ) {
		Busy = 1;
		message( "Wait...", NULL );
	}
}


/* Main dispatch routine that decodes the top level of the opcode and
 * goes to the appropriate lower level dispatch routine.
 */
void xeq(opcode op) 
{
	decimal64 save[STACK_SIZE+2];
	struct _ustate old = UState;
	unsigned short old_pc = state_pc();
	int old_cl = *((int *)&CommandLine);

#ifdef CONSOLE
	instruction_count++;
#endif
#ifndef REALBUILD
	if (State2.trace) {
		char buf[16];
		if (Running)
			print_step(op);
		else
			sprintf(TraceBuffer, "%04X:%s", op, prt(op, buf));
		DispMsg = TraceBuffer;
	}
#endif
	Busy = 0;
	xcopy(save, &regX, (STACK_SIZE+2) * sizeof(decimal64));
	if (isDBL(op))
		multi(op);
	else if (isRARG(op))
		rargs(op);
	else {
		XeqOpCode = (s_opcode) op;
		switch (opKIND(op)) {
		case KIND_SPEC:	specials(op);	break;
		case KIND_NIL:	niladic(op);	break;
		case KIND_MON:	monadic(op);	break;
		case KIND_DYA:	dyadic(op);	break;
		case KIND_TRI:	triadic(op);	break;
		case KIND_CMON:	monadic_cmplex(op);	break;
		case KIND_CDYA:	dyadic_cmplex(op);	break;
		default:	illegal(op);
		}
	}

	if (Error != ERR_NONE) {
		// Repair stack and state
		// Clear return stack
		Error = ERR_NONE;
		xcopy(&regX, save, (STACK_SIZE+2) * sizeof(decimal64));
		UState = old;
		raw_set_pc(old_pc);
		*((int *)&CommandLine) = old_cl;
		process_cmdline_set_lift();
		if (Running) {
#ifndef REALBUILD
			if (! State2.trace ) {
#endif
				unsigned short int pc = state_pc();
				while (isXROM(pc)) {
					// Leave XROM
					if (RetStkPtr != 0) {
						retstk_up();
						pc = RetStk[RetStkPtr - 1];
					}
					if (RetStkPtr == 0)
						++pc; // compensate for decpc below
				}
				raw_set_pc(pc);
#ifndef REALBUILD
			}
#endif
			decpc();	// Back to error instruction
			RetStkPtr = 0;  // clear return stack
			set_running_off();
		}
	} 
	reset_volatile_state();
}

/* Execute a single step and return.
 */
static void xeq_single(void) {
	const opcode op = getprog(state_pc());

	incpc();
	xeq(op);
}

/* Continue execution trough xrom code
 */
static void xeq_xrom(void) {
#ifndef REALBUILD
	if (State2.trace)
		return;
#endif
	/* Now if we've stepped into the xROM area, keep going until
	 * we break free.
	 */
	while (!Pause && isXROM(state_pc()))
		xeq_single();
}

/* Check to see if we're running a program and if so execute it
 * for a while.
 *
 */
void xeqprog(void) 
{
	int state = 0;

	if ( Running || Pause ) {
#ifndef CONSOLE
		long long last_ticker = Ticker;
		state = ((int) last_ticker % (2*TICKS_PER_FLASH) < TICKS_PER_FLASH);
#else
		state = 1;
#endif
		dot(RCL_annun, state);
		finish_display();

		while (!Pause && Running) {
			xeq_single();
			if (is_key_pressed()) {
				xeq_xrom();
				break;
			}
		}
	}
	if (!Running && !Pause) {
		// Program has terminated
		clr_dot(RCL_annun);
		display();
#ifndef CONSOLE
		// Avoid accidental restart with R/S or APD after program ends
		JustStopped = 1;
#endif
	}
}

/* Single step and back step routine
 */
void xeq_sst_bst(int kind) 
{
	opcode op;

	reset_volatile_state();
	if (kind == -1)
		decpc();

	if (State2.runmode) {
		// Display the step
		op = getprog(state_pc());
		print_step(op);
		if (kind == 1) {
			// Execute the step on key up
#ifndef REALBUILD
			unsigned int trace = State2.trace;
			State2.trace = 0;
#endif
			set_running_on_sst();
			incpc();
			xeq(op);
#ifndef REALBUILD
			State2.trace = trace;
#endif
			xeq_xrom();
			set_running_off_sst();
		}
	}
	else if (kind == 0) {
		// Key down in program mode
		incpc();
		OpCode = 0;
	}
}


/* 
 *  The following needs to be done each time before any user input is processed.
 *  On the hardware, RAM is volatile and these pointers and structures need valid values!
 */
void xeq_init_contexts(void) {
	/*
	 *  Compute the sizes of the various memory portions
	 */
	const short int s = ((TOPREALREG - NumRegs) << 2) - SizeStatRegs; // additional register space
	RetStk = RetStkBase + s;					  // Move RetStk up or down
	RetStkSize = s + RET_STACK_SIZE - ProgSize;
	ProgMax = s + RET_STACK_SIZE - MINIMUM_RET_STACK_SIZE;
	ProgFree = ProgMax - ProgSize + RetStkPtr;

	/*
	 *  Initialise our standard contexts.
	 *  We bump the digits for internal calculations.
	 */
	decContextDefault(&Ctx, DEC_INIT_BASE);
	Ctx.digits = DECNUMDIGITS;
	Ctx.emax=DEC_MAX_MATH;
	Ctx.emin=-DEC_MAX_MATH;
	Ctx.round = DEC_ROUND_HALF_EVEN;
}


/*
 *  We don't allow some commands from a running program
 */
int not_running(void) {
	if ( Running ) {
		err(ERR_ILLEGAL);
		return 0;
	}
	return 1;
}

/*
 *  Handle the Running Flag
 */
void set_running_off_sst() {
	Running = 0;
}

void set_running_on_sst() {
	Running = 1;
	if (state_pc() == 0)
		incpc();
}

void set_running_off() {
	set_running_off_sst();
	State.entryp = 0;
	dot( RCL_annun, 0);
}

void set_running_on() {
	update_speed(0);
	GoFast = 1;
	set_running_on_sst();
	LastKey = 0;
	error_message(ERR_NONE);
	dot(BEG, 0);
	finish_display();
}

/*
 *  Command to support local variables.
 *  A stack frame is constructed:
 *	marker including size of frame,
 *	register + flag area.
 *  Registers must reside on even stack positions
 *  so the flag word is either at the top or at the bottom of the frame.
 */
void cmdlocr(unsigned int arg, enum rarg op) {
	short int sp = RetStkPtr;
	int size = (++arg << 2) + 2;
	const unsigned short marker = LOCAL_MASK | size;
	int old_size = 0;
	short unsigned int old_flags = 0;

	if (sp == LocalRegs) {
		// resize required
		old_size = local_levels();
		sp += old_size;
		old_flags = *flag_word(LOCAL_FLAG_BASE, NULL);
	}
	// compute space needed
	sp -= size;
	if (-sp > RetStkSize) {
		err(ERR_RAM_FULL);
		return;
	}
	if ( old_size > 0 ) {
		// move previous contents to new destination
		int n;
		if (size > old_size) {
			n = old_size;
			size -= old_size;
		}
		else {
			n = size;
			size = 0;
		}
		xcopy(RetStk + sp, RetStk + LocalRegs, n + n);
	}
	// fill the rest with 0
	xset(RetStk + sp + old_size, 0, size + size);

	// set marker, pointers and flags
	RetStk[sp] = marker;
	RetStkPtr = LocalRegs = sp;
	*flag_word(LOCAL_FLAG_BASE, NULL) = old_flags;
}

/*
 *  Command to support a single set of local variables
 *  for non recursive non interruptible XROM routines.
 *  We need a single stack level for a special marker
 */
void cmdxlocal(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	if (isXROM(state_pc())) {
		if (RetStkPtr >= RetStkSize) {
			err(ERR_RAM_FULL);
			return;
		}
		RetStk[--RetStkPtr] = LOCAL_MASK | 1;
		// fill with 0
		zero_regs(XromRegs, NUMXREGS);
		XromFlags = 0;
		LocalRegs = RetStkPtr;
	}
	else {
		cmdlocr(16, RARG_LOCR);
	}
}

/*
 *  Undo the effect of LOCL by popping the current local frame.
 *  Needs to be executed from the same level that has established the frame.
 */
void cmdlpop(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	if (LocalRegs != RetStkPtr) {
		err(ERR_ILLEGAL);
		return;
	}
	RetStkPtr = LocalRegs;
	retstk_up();
	--RetStkPtr;
}

/*
 *  Reduce the number of global registers in favour of local data on the return stack
 */
void cmdregs(unsigned int arg, enum rarg op) {
	int distance = NumRegs - (++arg);

	// Move return stack, check for room
	if (move_retstk(distance << 2))
		return;
	// Move register contents, including the statistics registers
	xcopy((unsigned short *)(Regs + TOPREALREG - arg)     - SizeStatRegs,
	      (unsigned short *)(Regs + TOPREALREG - NumRegs) - SizeStatRegs,
	      (arg << 3) + (SizeStatRegs << 1));
	if (distance < 0)
		zero_regs(Regs + TOPREALREG + distance, -distance);
	NumRegs = arg;
}


/*
 *  Debugging output for the console version
 */
#if defined(DEBUG) && defined(CONSOLE) && !defined(WP34STEST)
extern unsigned char remap_chars(unsigned char ch);

static int compare(s_opcode a1, s_opcode a2, int cata) {
	char b1[16], b2[16];
	const unsigned char *s1, *s2;
	int i;

	xset(b1, 0, sizeof(b1));
	xset(b2, 0, sizeof(b2));
	s1 = (unsigned char *)catcmd(a1, b1);
	s2 = (unsigned char *)catcmd(a2, b2);
	if (*s1 == COMPLEX_PREFIX) s1++;
	if (*s2 == COMPLEX_PREFIX) s2++;

	for (i=0;;i++) {
		unsigned char c1 = *s1++;
		unsigned char c2 = *s2++;
		c1 = remap_chars(c1);
		c2 = remap_chars(c2);

		if (c1 != c2) {
			if (c1 > c2) {
				return 1;
			}
			return 0;
		} else if (c1 == '\0')
			break;
	}
	return 0;
}

static void check_cat(const enum catalogues cata, const char *name) {
	int i;
	char b1[16], b2[16];
	const int oldcata = State2.catalogue;
	int n;

	State2.catalogue = cata;
	n = current_catalogue_max();
	for (i=1; i<n; i++) {
		opcode cold = current_catalogue(i-1);
		opcode c = current_catalogue(i);
		if (compare(cold, c, cata))
			error("catalogue %s row %04x / %04x  %d / %d: %04o / %04o (%s / %s)", name, cold, c, i-1, i,
					0xff & cold, 0xff & c,
					catcmd(cold, b1), catcmd(c, b2));
	}
	State2.catalogue = oldcata;
}

static void check_const_cat(void) {
	int i;
	char b1[16], b2[16];
	char p1[64], p2[64];

	for (i=1; i<NUM_CONSTS; i++) {
		if (compare(CONST(i-1), CONST(i), 0)) {
			prettify(catcmd(CONST(i-1), b1), p1);
			prettify(catcmd(CONST(i), b2), p2);
			error("constants row %d / %d: %s / %s", i, i+1, p1, p2);
		}
	}
}

static void bad_table(const char *t, int row, const char *n, int nlen) {
	char buf[64], name[20];
	int i;

	for (i=0; i<nlen; i++)
		name[i] = n[i];
	name[nlen] = '\0';
	prettify(name, buf);
	error("%s table row %d: %6s", t, row, buf);
}

#endif

/* Main initialisation routine that sets things up for us.
 * Returns a nonzero result if it has cleared ram.
 */
int init_34s(void)
{
	int cleared = checksum_all();
	if ( cleared ) {
		reset();
	}
	init_state();
	xeq_init_contexts();
	ShowRPN = 1;

#if defined(CONSOLE) && !defined(WP34STEST) && defined(DEBUG)
	{
		int i;
	/* Sanity check the function table indices.
	 * These indicies must correspond exactly with the enum definition.
	 * This code validates that this is true and prints error messages
	 * if it isn't.
	 */
	for (i=0; i<num_monfuncs; i++)
		if (monfuncs[i].n != i)
			bad_table("monadic function", i, monfuncs[i].fname, NAME_LEN);
	for (i=0; i<num_dyfuncs; i++)
		if (dyfuncs[i].n != i)
			bad_table("dyadic function", i, dyfuncs[i].fname, NAME_LEN);
	for (i=0; i<num_trifuncs; i++)
		if (trifuncs[i].n != i)
			bad_table("triadic function", i, trifuncs[i].fname, NAME_LEN);
	for (i=0; i<num_niladics; i++)
		if (niladics[i].n != i)
			bad_table("niladic function", i, niladics[i].nname, NAME_LEN);
	for (i=0; i<num_argcmds; i++)
		if (argcmds[i].n != i)
			bad_table("argument command", i, argcmds[i].cmd, NAME_LEN);
	for (i=0; i<num_multicmds; i++)
		if (multicmds[i].n != i)
			bad_table("multi command", i, multicmds[i].cmd, NAME_LEN);
	check_const_cat();
	check_cat(CATALOGUE_COMPLEX, "complex");
	check_cat(CATALOGUE_STATS, "statistics");
	check_cat(CATALOGUE_SUMS, "summations");
	check_cat(CATALOGUE_PROB, "probability");
	check_cat(CATALOGUE_PROG, "programme");
	check_cat(CATALOGUE_MODE, "mode");
	check_cat(CATALOGUE_TEST, "tests");
	check_cat(CATALOGUE_INT, "int");
#ifdef MATRIX_SUPPORT
	check_cat(CATALOGUE_MATRIX, "matrix");
#endif
	/*
	check_cat(CATALOGUE_ALPHA, "alpha");
	check_cat(CATALOGUE_ALPHA_LETTERS, "alpha special upper case letters");
	// check_cat(CATALOGUE_ALPHA_LETTERS_LOWER, "alpha special lower letters");
	check_cat(CATALOGUE_ALPHA_SUBSCRIPTS, "alpha subscripts");
	check_cat(CATALOGUE_ALPHA_SYMBOLS, "alpha symbols");
	check_cat(CATALOGUE_ALPHA_COMPARES, "alpha compares");
	check_cat(CATALOGUE_ALPHA_ARROWS, "alpha arrows");
	*/
	check_cat(CATALOGUE_CONV, "conversion");
	check_cat(CATALOGUE_NORMAL, "float");
#ifdef INCLUDE_INTERNAL_CATALOGUE
	check_cat(CATALOGUE_INTERNAL, "internal");
#endif
        if (sizeof(unsigned long long int) != sizeof(UState))
            error("sizeof register (%u) != sizeof user state (%u)\n", sizeof(unsigned long long int), sizeof(UState));
	}
#endif
	return cleared;
}

#ifdef GNUC_POP_ERROR
#pragma GCC diagnostic pop
#endif



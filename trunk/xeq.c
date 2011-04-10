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
#ifdef WIN32
#include <stdlib.h>  // sleep
#define sleep _sleep
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#include <stdio.h>   // (s)printf
#endif // REALBUILD

#define XEQ_INTERNAL 1
#include "xeq.h"
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


#define EMPTY_PROGRAM_OPCODE	RARG(RARG_ERROR, ERR_PROG_BAD)

/* Define the number of program Ticks that must elapse between flashing the
 * RCL annunicator.
 */
#define TICKS_PER_FLASH	(5)


/* Define storage for the machine's program space.
 */
#define prog	((s_opcode *)(PersistentRam._prog - 1))

/* We need various different maths contexts.
 * More efficient to define these globally and reuse them as needed.
 */
decContext *Ctx, *Ctx64;


#define RetStkPtr	(State.retstk_ptr)


unsigned int get_bank_flags(void) {
	return BankFlags;
}

void set_bank_flags(unsigned int f) {
	BankFlags = f;
}

void version(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	State.version = 1;
	if (!State.runmode)
		display();
}

void cmd_off(decimal64 *a, decimal64 *nul2, decContext *ctx64) {
	shutdown();
}

unsigned int state_pc(void) {	return State.state_pc;	}
static void raw_set_pc(unsigned int pc) {
	State.state_pc = pc;
}

void set_pc(unsigned int pc) {
	if (pc >= State.last_prog)
		pc = State.last_prog - 1;
	if (!isXROM(pc) && pc > 1 && isDBL(prog[pc-1]))
		pc--;
	raw_set_pc(pc);
}


/* Return the program memory location specified.
 */
opcode getprog(unsigned int n) {
	opcode r;

	if (isXROM(n)) {
		r = xrom[n & ~XROM_MASK];
		if (isDBL(r))
			r |= xrom[(n+1) & ~XROM_MASK] << 16;
		return r;
	}
	if (n >= State.last_prog || n > NUMPROG)
		return EMPTY_PROGRAM_OPCODE;
	if (n == 0)
		return OP_NIL | OP_NOP;
	r = prog[n];
	if (isDBL(r))
		r |= prog[n+1] << 16;
	return r;
}


int running(void) {
	return State.state_running;
}

/* Set a flag to indicate that a complex operation has taken place
 * This only happens if we're not in a program.
 */
static void set_was_complex(void) {
	if (! running())
		State.wascomplex = 1;
}


/* Produce a warning and stop
 */
static void warn(const enum errors e) {
}

/* Produce an error and stop
 */
void err(const enum errors e) {
	State.error = e;
	if (running()) {
		set_running_off();
		decpc();			// back up to errant statement
	}
}


/* Doing something in the wrong mode */
static void bad_mode_error(void) {
	err(ERR_BAD_MODE);
}


/* User commands to produce warnings and errors */
void cmderr(unsigned int arg, enum rarg op) {
	err(arg);
}

#if defined(DEBUG) && !defined(WINGUI)
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

/* Check if a value is bogus and error out if so.
 */
static int check_special(const decNumber *x) {
	decNumber y;
	decimal64 z;

	decimal64FromNumber(&z, x, Ctx64);
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
	return State.stack_depth?8:4;
}

static decimal64 *get_stack(int pos) {
	return Regs + TOPREALREG + pos;
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

static void lower(void) {
	const int n = stack_size();
	int i;

	for (i=1; i<n; i++)
		*get_stack(i-1) = *get_stack(i);
}

static void lift2(void) {
	const int n = stack_size();
	int i;

	for (i=n-1; i>1; i--)
		*get_stack(i) = *get_stack(i-2);
}

static void lower2(void) {
	const int n = stack_size();
	int i;

	for (i=2; i<n; i++)
		*get_stack(i-2) = *get_stack(i);
}


void lastX(decimal64 *x, decimal64 *nul, decContext *ctx64) {
	*x = regL;
}

void setlastX(void) {
	regL = regX;
}



void lastXY(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = regL;
	*y = regI;
}

static void setlastXY(void) {
	setlastX();
	regI = regY;
}


void getX(decNumber *x) {
	decimal64ToNumber(&regX, x);
}

void setX(const decNumber *x) {
	decNumber xn;

	if (! check_special(x)) {
		decNumberNormalize(&xn, x, Ctx);
		decimal64FromNumber(&regX, &xn, Ctx64);
	}
}

void getY(decNumber *y) {
	decimal64ToNumber(&regY, y);
}

static void setY(const decNumber *y) {
	decNumber yn;

	if (! check_special(y)) {
		decNumberNormalize(&yn, y, Ctx);
		decimal64FromNumber(&regY, &yn, Ctx64);
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

void getYZ(decNumber *y, decNumber *z) {
	getY(y);
	getZ(z);
}

void roll_down(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	const decimal64 r = regX;
	lower();
	*get_stack_top() = r;
}

void roll_up(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	const decimal64 r = *get_stack_top();
	lift();
	regX = r;
}

void cpx_roll_down(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	roll_down(NULL, NULL, NULL);
	roll_down(NULL, NULL, NULL);
}

void cpx_roll_up(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	roll_up(NULL, NULL, NULL);
	roll_up(NULL, NULL, NULL);
}

void swap(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	swap_reg(&regX, &regY);
}

void cpx_swap(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	swap_reg(&regX, &regZ);
	swap_reg(&regY, &regT);
}

void cpx_enter(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	decimal64 x = regX, y = regY;
	cpx_roll_up(NULL, NULL, NULL);
	regX = x;
	regY = y;
}

void cpx_fill(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	const int n = stack_size();
	int i;

	for (i=2; i<n; i++)
		if (i & 1)	*get_stack(i) = regY;
		else		*get_stack(i) = regX;
}

void fill(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	const int n = stack_size();
	int i;

	for (i=1; i<n; i++)
		*get_stack(i) = regX;
}

void drop(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	lower();
}

void dropxy(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	lower();
	lower();
}


int is_intmode(void) {
	return State.intm;
}

void showlead0(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	State.leadzero = 1;
}

void hidelead0(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	State.leadzero = 0;
}

/* Increment the passed PC.  Account for wrap around but nothing else.
 * Return the updated PC.
 */
unsigned int inc(const unsigned int pc) {
	const unsigned int off = isDBL(getprog(pc))?2:1;
	const unsigned int npc = pc + off;
	if (isXROM(pc)) {
		if (pc >= addrXROM(xrom_size - 1))
			return addrXROM(0);
		return npc;
	}
	if (npc >= State.last_prog) {
		return 0;
		//if (!running() || pc >= State.last_prog)
			//return 0;
	}
	return npc;
}

unsigned int dec(unsigned int pc) {
	if (isXROM(pc) && pc == addrXROM(0))
		return addrXROM(xrom_size - 1);
	if (pc == 0)
		pc = State.last_prog;
	if (--pc > 1 && pc != addrXROM(0) && isDBL(getprog(pc-1)))
		pc--;
	return pc;
}

/* Increment the PC keeping account of wrapping around and stopping
 * programs on such.  Return non-zero if we wrapped.
 */
int incpc(void) {
	const unsigned int pc = inc(state_pc());

	raw_set_pc(pc);
	if (pc == 0 && running())
		State.implicit_rtn = 1;
	return pc == 0;
}

void decpc(void) {
	raw_set_pc(dec(state_pc()));
	set_running_off();
}

/* Zero out the stack
 */
void clrstk(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	if (is_intmode())
		d64fromInt(&regX, 0);
	else
		regX = CONSTANT_INT(OP_ZERO);
	fill(NULL, NULL, NULL);
	regL = regX;
	regI = regX;

	State.eol = 0;
	State.state_lift = 1;
}

/* Reset all flags to off/false
 */
void clrflags(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	xset(UserFlags, 0, sizeof(UserFlags));
}

/* Zero out all registers including the stack and lastx
 */	
void clrreg(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	int i;

	if (is_intmode())
		d64fromInt(Regs, 0);
	else
		Regs[0] = CONSTANT_INT(OP_ZERO);

	for (i=1; i<TOPREALREG; i++)
		Regs[i] = Regs[0];

	for (i=TOPREALREG+stack_size(); i<NUMREG; i++)
		Regs[i] = Regs[0];

	State.eol = 0;
	State.state_lift = 1;
}

/* Clear the program space
 */
void clrprog(void) {
	int i;

	for (i=1; i<=NUMPROG; i++)
		prog[i] = EMPTY_PROGRAM_OPCODE;
	raw_set_pc(0);
	RetStkPtr = 0;
	State.last_prog = 1;
}

/* Clear all - programs and registers
 */
void clrall(decimal64 *a, decimal64 *b, decContext *nulc) {
	int i;

	sigma_clear(NULL, NULL, NULL);
	clrreg(NULL, NULL, NULL);
	clrstk(NULL, NULL, NULL);
	clralpha(NULL, NULL, NULL);
	clrflags(NULL, NULL, NULL);
	clrprog();

	/* Clear out the banked registers and flags */
	for (i=0; i<NUMBANKREGS; i++)
		BankRegs[i] = Regs[0];
	BankFlags = 0;

	set_shift(SHIFT_N);
	State.test = TST_NONE;

	DispMsg = NULL;
}


/* Clear everything
 */
void reset(decimal64 *a, decimal64 *b, decContext *nulc) {
	xset(&PersistentRam, 0, sizeof( PersistentRam ));
	clrall(NULL,NULL,NULL);
	init_state();
	State.contrast = 9;
	DispMsg = "Erased";
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
	if (State.eol) {
		Cmdline[State.eol] = '\0';
		return Cmdline;
	}
	return NULL;
}


static int fract_convert_number(decNumber *x, const char *s) {
	if (*s == '\0') {
		err(ERR_DOMAIN);
		return 1;
	}
	decNumberFromString(x, s, Ctx);
	return check_special(x);
}

/* Process the command line if any
 */
static void process_cmdline(void) {
	decNumber a, b, x, t, z;

	if (State.eol) {
		const unsigned int cmdlinedot = State.cmdlinedot;
		Cmdline[State.eol] = '\0';
		if (Cmdline[State.eol-1] == 'E')
			Cmdline[State.eol-1] = '\0';
		State.eol = 0;
		if (State.state_lift)
			lift();
		State.state_lift = 1;
		State.cmdlinedot = 0;
		State.cmdlineeex = 0;
		if (is_intmode()) {
			const int sgn = (Cmdline[0] == '-')?1:0;
			unsigned long long int x = s_to_ull(Cmdline+sgn, int_base());
			d64fromInt(&regX, build_value(x, sgn));
		} else if (cmdlinedot == 2 || (State.fract && cmdlinedot)) {
			char *d0, *d1;
			int neg;

			State.fract = 1;
			if (Cmdline[0] == '-') {
				neg = 1;
				d0 = Cmdline+1;
			} else {
				neg = 0;
				d0 = Cmdline;
			}
			d1 = find_char(Cmdline, '.');
			*d1++ = '\0';
			// if (*d1 == '\0') goto real;
			if (cmdlinedot == 2) {
				char *d2 = find_char(d1, '.');
				*d2++ = '\0';
				// if (*d2 == '\0') cmdlinedot--; else
				if (fract_convert_number(&b, d2))
					return;
			}
			if (fract_convert_number(&z, d0))	return;
			if (fract_convert_number(&a, d1))	return;
			if (cmdlinedot == 2) {
				if (decNumberIsZero(&b)) {
					err(ERR_DOMAIN);
					return;
				}
				decNumberDivide(&t, &a, &b, Ctx);
				decNumberAdd(&x, &z, &t, Ctx);
			} else {
				if (decNumberIsZero(&a)) {
					err(ERR_DOMAIN);
					return;
				}
				decNumberDivide(&x, &z, &a, Ctx);
			}
			if (neg)
				decNumberMinus(&x, &x, Ctx);
			setX(&x);
		} else {
			decNumberFromString(&x, Cmdline, Ctx);
			setX(&x);
		}
	}
}

void process_cmdline_set_lift(void) {
	process_cmdline();
	State.state_lift = 1;
}


/* Wrapper routine to extract a specific register from a number.
 * Since we've some bank switched registers, we detect xROM space
 * here and return the alternate registers if required.
 */
decimal64 *get_reg_n(int n) {
	if (isXROM(state_pc()) && n < NUMBANKREGS)
		return BankRegs + n;
	return Regs + (n % NUMREG);
}

void get_reg_n_as_dn(int n, decNumber *x) {
	decimal64ToNumber(get_reg_n(n), x);
}

void put_reg_n(int n, const decNumber *x) {
	if (! check_special(x))
		decimal64FromNumber(get_reg_n(n), x, Ctx64);
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

//unsigned long long int reg_get_int(int n, int *sgn) {
//	return get_int(get_reg_n(n), sgn);
//}


/* Put an integer into the specified real
 */
void put_int(unsigned long long int val, int sgn, decimal64 *x) {
	if (is_intmode()) {
		d64fromInt(x, build_value(val, sgn));
	} else {
		decNumber t;

		ullint_to_dn(&t, val, Ctx);
		if (sgn)
			decNumberMinus(&t, &t, Ctx);
		decimal64FromNumber(x, &t, Ctx64);
	}
}

unsigned long long int get_int(const decimal64 *x, int *sgn) {
	if (is_intmode()) {
		return extract_value(d64toInt(x), sgn);
	} else {
		decNumber n;

		decimal64ToNumber(x, &n);
		return dn_to_ull(&n, Ctx, sgn);
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
	if (decNumberIsZero(&t))
		return 0;
	if (decNumberIsSpecial(&t))
		return 0;
	if (decNumberIsNegative(&t)) {
		decNumberMinus(&t, &t, Ctx);
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

	ullint_to_dn(&t, nv, Ctx);
	if (sgn)
		decNumberMinus(&t, &t, Ctx);
	decimal64FromNumber(n, &t, Ctx64);
#else
	xcopy(n, &z, sizeof(decimal64));
#endif
}

/***************************************************************************
 * Monadic function handling.
 */

/* Dispatch routine for monadic operations.
 * Since these functions take an argument from the X register, save it in
 * lastx and then replace it with their result, we can factor out the common
 * stack manipulatin code.
 */
static void monadic(const opcode op) {

	unsigned int f;
	process_cmdline_set_lift();

	f = argKIND(op);
	if (f < num_monfuncs) {
		if (is_intmode()) {
			if (monfuncs[f].monint != NULL) {
				long long int x = d64toInt(&regX);
				x = (*monfuncs[f].monint)(x);
				setlastX();
				d64fromInt(&regX, x);
			} else
				bad_mode_error();
		} else {
			if (monfuncs[f].mondreal != NULL) {
				decNumber x, r;

				getX(&x);

				(*monfuncs[f].mondreal)(&r, &x, Ctx);

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
		if (monfuncs[f].mondcmplx != NULL) {
			getXY(&x, &y);

			(*monfuncs[f].mondcmplx)(&rx, &ry, &x, &y, Ctx);

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
			if (dyfuncs[f].dydint != NULL) {
				long long int x = d64toInt(&regX);
				long long int y = d64toInt(&regY);
				x = (*dyfuncs[f].dydint)(y, x);
				setlastX();
				lower();
				d64fromInt(&regX, x);
			} else
				bad_mode_error();
		} else {
			if (dyfuncs[f].dydreal != NULL) {
				decNumber x, y, r;

				getXY(&x, &y);

				(*dyfuncs[f].dydreal)(&r, &y, &x, Ctx);

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
		if (dyfuncs[f].dydcmplx != NULL) {
			getXY(&x1, &y1);
			getZ(&x2);
			getT(&y2);

			(*dyfuncs[f].dydcmplx)(&xr, &yr, &x2, &y2, &x1, &y1, Ctx);

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
			if (trifuncs[f].triint != NULL) {
				long long int x = d64toInt(&regX);
				long long int y = d64toInt(&regY);
				long long int z = d64toInt(&regZ);
				x = (*trifuncs[f].triint)(z, y, x);
				setlastX();
				lower();
				lower();
				d64fromInt(&regX, x);
			} else
				bad_mode_error();
		} else {
			if (trifuncs[f].trireal != NULL) {
				decNumber x, y, z, r;

				getXY(&x, &y);
				getZ(&z);

				(*trifuncs[f].trireal)(&r, &z, &y, &x, Ctx);

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
	lift();
	regX = CONSTANT(arg);
}

void cmdconstcmplx(unsigned int arg, enum rarg op) {
	if (is_intmode()) {
		bad_mode_error();
		return;
	}
	lift2();
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
	lift();
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
		decNumberAdd(r, &y, &x, Ctx64);
		break;
	case 2:
		decNumberSubtract(r, &y, &x, Ctx64);
		break;
	case 3:
		decNumberMultiply(r, &y, &x, Ctx64);
		break;
	case 4:
		decNumberDivide(r, &y, &x, Ctx64);
		break;
	case 5:
		decNumberMin(r, &y, &x, Ctx);
		break;
	case 6:
		decNumberMax(r, &y, &x, Ctx);
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
			decimal64FromNumber(rn, &r, Ctx64);
		}
	}
}

/* We've got a RCL operation to do.
 */
void cmdrcl(unsigned int arg, enum rarg op) {
	const decimal64 *rn = get_reg_n(arg);

	if (op == RARG_RCL) {
		decimal64 temp = *rn;
		lift();
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

/* And the complex equivalents for the above.
 * We pair registers arg & arg+1 to provide a complex number
 */
static int storcl_cop(unsigned short opr,
		const decimal64 *y1r, const decimal64 *y2r,
		decNumber *r1, decNumber *r2) {
	decNumber x1, x2, y1, y2;

	getXY(&x1, &x2);
	decimal64ToNumber(y1r, &y1);
	decimal64ToNumber(y2r, &y2);

	switch (opr) {
	case 1:
		cmplxAdd(r1, r2, &y1, &y2, &x1, &x2, Ctx64);
		break;
	case 2:
		cmplxSubtract(r1, r2, &y1, &y2, &x1, &x2, Ctx64);
		break;
	case 3:
		cmplxMultiply(r1, r2, &y1, &y2, &x1, &x2, Ctx);
		break;
	case 4:
		cmplxDivide(r1, r2, &y1, &y2, &x1, &x2, Ctx);
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
		else if (storcl_cop(op - RARG_STO, t1, t2, &r1, &r2))
			illegal(op);
		else {
			decimal64FromNumber(t1, &r1, Ctx64);
			decimal64FromNumber(t2, &r2, Ctx64);
		}
	}
	set_was_complex();
}

void cmdcrcl(unsigned int arg, enum rarg op) {
	decNumber r1, r2;
	const decimal64 *t1, *t2;

	t1 = get_reg_n(arg);
	t2 = get_reg_n(arg+1);

	if (op == RARG_CRCL) {
		lift2();
		regX = *t1;
		regY = *t2;
	} else {
		if (is_intmode())
			bad_mode_error();
		else if (storcl_cop(op - RARG_CRCL, t1, t2, &r1, &r2))
			illegal(op);
		else {
			setlastXY();
			setXY(&r1, &r2);
		}
	}
	set_was_complex();
}

/* SWAP x with the specified register
 */
void swap_reg(decimal64 *a, decimal64 *b) {
	decimal64 t;

	t = *a;
	*a = *b;
	*b = t;
}

void cmdswap(unsigned int arg, enum rarg op) {
	swap_reg(&regX, get_reg_n(arg));
	if (op == RARG_CSWAP) {
		swap_reg(&regY, get_reg_n(arg+1));
		set_was_complex();
	}
}


/* View a specified register
 */
void cmdview(unsigned int arg, enum rarg op) {
	State.show_register = arg;
	display();
	State.show_register = arg;
}


/* Set the stack size */
void set_stack_size4(decimal64 *a, decimal64 *nul2, decContext *ctx64) {
	State.stack_depth = 0;
}
void set_stack_size8(decimal64 *a, decimal64 *nul2, decContext *ctx64) {
	State.stack_depth = 1;
}

/* Get the stack size */
void get_stack_size(decimal64 *a, decimal64 *nul2, decContext *ctx64) {
	put_int(stack_size(), 0, a);
}

void get_word_size(decimal64 *a, decimal64 *nul2, decContext *ctx64) {
	put_int((int)word_size(), 0, a);
}

/* Get the current ticker value */
void op_ticks(decimal64 *a, decimal64 *nul2, decContext *ctx64) {
#ifdef REALBULID
	put_int(Ticker, 0, a);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long long int t = tv.tv_sec * 10ll + tv.tv_usec / 100000;
	put_int(t, 0, a);
#endif
}

/* Save and restore the entire stack to sequential registers */
static int check_stack_overlap(unsigned int arg, int *nout) {
	const int n = stack_size();

	if (arg + n <= TOPREALREG) {
		*nout = n;
		return 1;
	}
	err(ERR_STK_CLASH);
	return 0;
}

void cmdstostk(unsigned int arg, enum rarg op) {
	int i, n;

	if (check_stack_overlap(arg, &n))
		for (i=0; i<n; i++)
			*get_reg_n(arg+i) = *get_stack(i);
}

void cmdrclstk(unsigned int arg, enum rarg op) {
	int i, n;

	if (check_stack_overlap(arg, &n))
		for (i=0; i<n; i++)
			*get_stack(i) = *get_reg_n(arg+i);
}


/* Search from the given position for the specified numeric label.
 */
static unsigned int find_opcode_from(unsigned int pc, const opcode l, int quiet) {
	unsigned int z = pc;
	unsigned int max;
	unsigned int min;

	if (isXROM(z)) {
		min = addrXROM(0);
		max = addrXROM(xrom_size);
	} else {
		if (z == 0)
			z++;
		min = 1;
		max = State.last_prog;
	}

	while (z < max && z != 0)
		if (getprog(z) == l)
			return z;
		else
			z = inc(z);
	for (z = min; z<pc; z = inc(z))
		if (getprog(z) == l)
			return z;
	if (!quiet)
		err(ERR_NO_LBL);
	return 0;
}

unsigned int find_label_from(unsigned int pc, unsigned int arg, int quiet) {
	return find_opcode_from(pc, OP_RARG + (RARG_LBL << RARG_OPSHFT) + arg, quiet);
}

/* Handle a GTO/GSB instruction
 */
static void gsbgto(unsigned int pc, int gsb, unsigned int oldpc) {
	raw_set_pc(pc);
	if (gsb) {
		if (running()) {
			if (!State.implicit_rtn) {
				RetStk[RetStkPtr++] = oldpc;
				if (RetStkPtr >= RET_STACK_SIZE) {
					err(ERR_XEQ_NEST);
					RetStkPtr = 0;
					set_running_off();
				}
			}
		} else {
			RetStkPtr = 0;
			set_running_on();
		}
	}
	State.implicit_rtn = 0;
}

static void cmdgtocommon(int gsb, unsigned int pc) {
	const unsigned int oldpc = state_pc();

	if (pc == 0)
		set_running_off();
	else
		gsbgto(pc, gsb, oldpc);
}

void cmdlblp(unsigned int arg, enum rarg op) {
	fin_tst(find_label_from(0, arg, 1) != 0);
}

void cmdgto(unsigned int arg, enum rarg op) {
	cmdgtocommon(op != RARG_GTO, find_label_from(state_pc(), arg, 0));
}

void cmdmultilblp(const opcode o, enum multiops mopr) {
	const opcode dest = (o & 0xfffff0ff) + (DBL_LBL << DBL_SHIFT);
	unsigned int lbl = find_opcode_from(0, dest, 1);
	fin_tst(lbl != 0);
}

void cmdmultigto(const opcode o, enum multiops mopr) {
	const opcode dest = (o & 0xfffff0ff) + (DBL_LBL << DBL_SHIFT);
	unsigned int lbl = find_opcode_from(0, dest, 0);
	cmdgtocommon(mopr != DBL_GTO, lbl);
}

static void do_xrom(int lbl) {
	const unsigned int oldpc = state_pc();
	const unsigned int pc = find_label_from(addrXROM(0), lbl, 1);

	gsbgto(pc, 1, oldpc);
}

static void xromargcommon(int lbl, unsigned int userpc) {
	if (userpc != 0) {
		State.usrpc = userpc;
		do_xrom(lbl);
	}
}

void xromarg(unsigned int arg, enum rarg op) {
	xromargcommon(ENTRY_SIGMA - (op - RARG_SUM), find_label_from(state_pc(), arg, 0));
}

void multixromarg(const opcode o, enum multiops mopr) {
	const opcode dest = (o & 0xfffff0ff) + (DBL_LBL << DBL_SHIFT);
	unsigned int lbl = find_opcode_from(0, dest, 0);
	xromargcommon(ENTRY_SIGMA - (mopr - DBL_SUM), lbl);
}

void xrom_tvm(decimal64 *a, decimal64 *nul2, decContext *ctx64) {
	do_xrom(ENTRY_TVM);
}

void cmddisp(unsigned int arg, enum rarg op) {
	State.dispdigs = arg;
	if (op == RARG_FIX)		State.dispmode = MODE_FIX;
	else if (op == RARG_SCI)	State.dispmode = MODE_SCI;
	else if (op == RARG_ENG)	State.dispmode = MODE_ENG;
	// RARG_DISP just falls through having had its work done already
	State.fract = 0;
}


/* Metric / Imperial conversion code */
decNumber *convC2F(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber s;

	decNumberMultiply(&s, x, &const_9on5, ctx);
	return decNumberAdd(r, &s, &const_32, ctx);
}

decNumber *convF2C(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber s;

	decNumberSubtract(&s, x, &const_32, ctx);
	return decNumberDivide(r, &s, &const_9on5, ctx);
}

decNumber *convDB2AR(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;
	decNumberMultiply(&t, x, &const_0_05, ctx);
	return decNumberPow10(r, &t, ctx);
}

decNumber *convAR2DB(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;
	decNumberLog10(&t, x, ctx);
	return decNumberMultiply(r, &t, &const_20, ctx);
}

decNumber *convDB2PR(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;
	decNumberMultiply(&t, x, &const_0_1, ctx);
	return decNumberPow10(r, &t, ctx);
}

decNumber *convPR2DB(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;
	decNumberLog10(&t, x, ctx);
	return decNumberMultiply(r, &t, &const_10, ctx);
}

/* Scale conversions */
void do_conv(decNumber *r, unsigned int arg, const decNumber *x, decContext *ctx) {
	decNumber m;
	const unsigned int conv = arg / 2;
	const unsigned int dirn = arg & 1;

	if (conv > NUM_CONSTS_CONV) {
		decNumberCopy(r, x);
		return;
	}

	decimal64ToNumber(&CONSTANT_CONV(conv), &m);
	
	if (dirn == 0)		// metric to imperial
		decNumberDivide(r, x, &m, ctx);
	else			// imperial to metric
		decNumberMultiply(r, x, &m, ctx);
}

void cmdconv(unsigned int arg, enum rarg op) {
	decNumber x, r;

	if (is_intmode())
		return;

	getX(&x);
	do_conv(&r, arg, &x, Ctx);
	setlastX();
	setX(&r);
}

void cmdcontrast(unsigned int arg, enum rarg op) {
	State.contrast = arg & 0xf;
}

/* Finish up a test -- if the value is non-zero, the test passes.
 * If it is zero, the test fails.
 */
void fin_tst(const int a) {
	if (a) {
		if (!running())
			DispMsg = "true";
	} else {
		if (running()) {
			if (! State.implicit_rtn)
				incpc();
		} else
			DispMsg = "false";
	}
}


/* Skip a number of instructions forwards */
void cmdskip(unsigned int arg, enum rarg op) {
	while (arg-- > 0 && !incpc());
}

/* Skip backwards */
void cmdback(unsigned int arg, enum rarg op) {
	unsigned int pc = state_pc();
        if (arg++) {
		while (arg-- > 0 && pc != 0)
			pc = dec(pc);
		raw_set_pc(pc);
		if (pc == 0)
			set_running_off();
	}
}


/* We've encountered a CHS while entering the command line.
 */
static void cmdlinechs(void) {
	if (State.cmdlineeex) {
		const unsigned int pos = State.cmdlineeex + 1;
		if (State.eol < pos) {
			if (State.eol < CMDLINELEN)
				Cmdline[State.eol++] = '-';
		} else if (Cmdline[pos] == '-') {
			if (State.eol != pos)
				xcopy(Cmdline + pos, Cmdline + pos + 1, State.eol-pos);
			State.eol--;
		} else if (State.eol < CMDLINELEN) {
			xcopy(Cmdline+pos+1, Cmdline+pos, State.eol-pos);
			Cmdline[pos] = '-';
			State.eol++;
		}
	} else {
		if (Cmdline[0] == '-') {
			if (State.eol > 1)
				xcopy(Cmdline, Cmdline+1, State.eol);
			State.eol--;
		} else if (State.eol < CMDLINELEN) {
			xcopy(Cmdline+1, Cmdline, State.eol);
			Cmdline[0] = '-';
			State.eol++;
		}
	}
}


/* Output a niladic function.
 */
static void niladic(const opcode op) {
	const unsigned int idx = argKIND(op);

	process_cmdline();
	if (idx < num_niladics) {
		if (niladics[idx].niladicf != NULL) {
			decimal64 *x = NULL, *y = NULL;

			switch (niladics[idx].numresults) {
			case 2:	lift();
				y = &regY;
			case 1:	x = &regX;
				if (State.state_lift)
					lift();
			default:
				if (niladics[idx].niladicf != NULL)
					(*niladics[idx].niladicf)(x, y, Ctx64);
				break;
			}
		}
	} else
		illegal(op);
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
			decNumberRnd(&x, &x, Ctx);
			if (cnst < 0)
				decNumberRnd(&t, &t, Ctx);
		}
		decNumberCompare(&r, &x, &t, Ctx);
		iszero = decNumberIsZero(&r);
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

void cmdtest(unsigned int arg, enum rarg op) {
	do_tst(get_reg_n(arg), (enum tst_op)(op - RARG_TEST_EQ), -1);
}

static void do_ztst(const decimal64 *r, const decimal64 *i, const enum tst_op op) {
	int c = 0;
	decNumber x, y, t, a, b;
	int eq = 1;

	process_cmdline_set_lift();

	if (is_intmode()) {
		err(ERR_BAD_MODE);
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
		decNumberRnd(&x, &x, Ctx);
		decNumberRnd(&y, &y, Ctx);
		decNumberRnd(&a, &a, Ctx);
		decNumberRnd(&b, &b, Ctx);
	}
#endif
	decNumberCompare(&t, &x, &a, Ctx);
	if (!decNumberIsZero(&t))
		eq = 0;
	else {
		decNumberCompare(&t, &y, &b, Ctx);
		if (!decNumberIsZero(&t))
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
			dn_inc(&x, Ctx);
		else
			dn_dec(&x, Ctx);
		put_reg_n(arg, &x);
		decNumberTrunc(&y, &x, Ctx);
		return ! decNumberIsZero(&y);
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

		if (op == RARG_ISG)
			x = intAdd(x, 1LL);
		else
			x = intSubtract(x, 1LL);
		put_reg_n_from_int(arg, x);

		xv = extract_value(x, &xs);
		if (op == RARG_ISG)
			fin_tst(xs == 0 && xv > 0);	// > 0
		else
			fin_tst(xs != 0 || xv == 0);	// <= 0
		return;
	} else {
		decNumber x, i, f, n, u;

		get_reg_n_as_dn(arg, &x);

		// Break the number into the important bits
		// nnnnn.fffii
		decNumberAbs(&f, &x, Ctx);
		decNumberTrunc(&n, &f, Ctx);			// n = nnnnn
		decNumberSubtract(&u, &f, &n, Ctx);		// u = .fffii
		if (decNumberIsNegative(&x))
			decNumberMinus(&n, &n, Ctx);
		decNumberMultiply(&i, &u, &const_1000, Ctx);	// i = fff.ii
		decNumberTrunc(&f, &i, Ctx);			// f = fff
		decNumberSubtract(&i, &i, &f, Ctx);		// i = .ii		
		decNumberMultiply(&x, &i, &const_100, Ctx);
		decNumberTrunc(&i, &x, Ctx);			// i = ii
		if (decNumberIsZero(&i))
			decNumberCopy(&i, &const_1);

		if (op == RARG_ISG) {
			decNumberAdd(&n, &n, &i, Ctx);
			decNumberCompare(&x, &f, &n, Ctx);
			fin_tst(! decNumberIsNegative(&x));
		} else {
			decNumberSubtract(&n, &n, &i, Ctx);
			decNumberCompare(&x, &f, &n, Ctx);
			fin_tst(decNumberIsNegative(&x));
		}

		// Finally rebuild the result
		if (decNumberIsNegative(&n)) {
			decNumberSubtract(&x, &n, &u, Ctx);
		} else
			decNumberAdd(&x, &n, &u, Ctx);
		put_reg_n(arg, &x);
	}
}

/* Return a pointer to the byte with the indicated flag in it.
 * also return a byte with the relevant bit mask set up.
 * Also, handle bank switched flags in xROM code.
 */
static unsigned char *flag_byte(const int n, unsigned char *mask) {
	if (n < 0 || n >= NUMFLG)
		return NULL;
	*mask = 1 << (n % 8);
	if (isXROM(state_pc()) && n < NUMBANKFLAGS)
		return ((unsigned char *)&BankFlags) + n / 8;
	return UserFlags + n / 8;
}

int get_user_flag(int n) {
	unsigned char mask;
	const unsigned char *const f = flag_byte(n, &mask);

	return f != NULL && (*f & mask)?1:0;
}

void put_user_flag(int n, int f) {
	if (f)	set_user_flag(n);
	else	clr_user_flag(n);
}

void set_user_flag(int n) {
	unsigned char mask;
	unsigned char *const f = flag_byte(n, &mask);

	if (f != NULL)
		*f |= mask;
}

void clr_user_flag(int n) {
	unsigned char mask;
	unsigned char *const f = flag_byte(n, &mask);

	if (f != NULL)
		*f &= ~mask;
}

void cmdflag(unsigned int arg, enum rarg op) {
	unsigned char mask;
	unsigned char *const f = flag_byte(arg, &mask);
	int flg = *f & mask;

	switch (op) {
	case RARG_SF:	flg = 1;			break;
	case RARG_CF:	flg = 0;			break;
	case RARG_FF:	flg = flg?0:1;			break;

	case RARG_FS:	fin_tst(flg);			return;
	case RARG_FC:	fin_tst(! flg);			return;

	case RARG_FSC:	fin_tst(flg);	flg = 0;	break;
	case RARG_FSS:	fin_tst(flg);	flg = 1;	break;
	case RARG_FSF:	fin_tst(flg);	flg = flg?0:1;	break;

	case RARG_FCC:	fin_tst(! flg);	flg = 0;	break;
	case RARG_FCS:	fin_tst(! flg);	flg = 1;	break;
	case RARG_FCF:	fin_tst(! flg);	flg = flg?0:1;	break;

	default:					return;
	}

	// And write the value back
	if (flg)
		*f |= mask;
	else
		*f &= ~mask;
}

void intws(unsigned int arg, enum rarg op) {
	State.int_len = arg;
}


/* Convert from a real to a fraction
 */

void get_maxdenom(decNumber *d) {
	const unsigned int dm = State.denom_max;
	int_to_dn(d, dm==0?9999:dm, Ctx);
}

void op_2frac(decimal64 *x, decimal64 *b, decContext *ctx64) {
	decNumber z, n, d, t;

	if (State.intm) {
		d64fromInt(x, 1);
		return;
	}

	getY(&z);			// Stack has been lifted already
	decNumber2Fraction(&n, &d, &z, Ctx);
	setXY(&d, &n);			// Set numerator and denominator
	if (State.runmode) {
		decNumberDivide(&t, &n, &d, Ctx);
		decNumberCompare(&n, &t, &z, ctx64);
		if (decNumberIsZero(&n))
			DispMsg = "y/x =";
		else if (decNumberIsNegative(&n))
			DispMsg = "y/x \017";
		else
			DispMsg = "y/x \020";
	}
}

void op_fracdenom(decimal64 *a, decimal64 *b, decContext *nulc) {
	int s;
	unsigned long long int i;

	i = get_int(&regX, &s);
	if (i > 9999)
		State.denom_max = 0;
	else if (i != 1)
		State.denom_max = (unsigned int) i;
	else {
		setlastX();
		put_int(State.denom_max, 0, &regX);
	}
}

void op_denany(decimal64 *a, decimal64 *b, decContext *nulc) {
	State.denom_mode = DENOM_ANY;
}

void op_denfix(decimal64 *a, decimal64 *b, decContext *nulc) {
	State.denom_mode = DENOM_FIXED;
}

void op_denfac(decimal64 *a, decimal64 *b, decContext *nulc) {
	State.denom_mode = DENOM_FACTOR;
}

/* Switching from an integer mode to real mode requires us
 * to make an effort at converting x and y into a real numbers
 * x' = y . 2^x
 */
static void int2dn(decNumber *x, decimal64 *a) {
	int s;
	unsigned long long int v = extract_value(d64toInt(a), &s);

	ullint_to_dn(x, v, Ctx);
	if (s)
		decNumberMinus(x, x, Ctx);
}

void op_float(decimal64 *a, decimal64 *b, decContext *nulc) {
	decNumber x, y, z;

	if (is_intmode()) {
		State.intm = 0;
		State.int_len = 0;

		int2dn(&x, &regX);
		int2dn(&y, &regY);
		clrstk(NULL, NULL, NULL);
		decNumberPower(&z, &const_2, &x, Ctx);
		decNumberMultiply(&x, &z, &y, Ctx);
		set_overflow(decNumberIsInfinite(&x));
		decimal64FromNumber(&regX, &x, Ctx64);
	}
	State.fract = 0;
}

void op_fract(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	op_float(NULL, NULL, NULL);
	State.fract = 1;
}

void op_fracimp(decimal64 *a, decimal64 *b, decContext *nulc) {
	op_fract(a, b, nulc);
	State.improperfrac = 1;
}

void op_fracpro(decimal64 *a, decimal64 *b, decContext *nulc) {
	op_fract(a, b, nulc);
	State.improperfrac = 0;
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

	if (State.eol >= CMDLINELEN) {
		warn(ERR_TOO_LONG);
		return;
	}
	if (intm) {
		if (c >= int_base()) {
			warn(ERR_DIGIT);
			return;
		}
		for (i=j=0; i<(int)State.eol; i++)
			j += is_xdigit(Cmdline[i]);
		if (j == lim) {
			warn(ERR_TOO_LONG);
			return;
		}
		if (c >= 10) {
			Cmdline[State.eol++] = c - 10 + 'A';
			return;
		}
	} else {
		if (c >= 10) {
			warn(ERR_DIGIT);
			return;
		}
		for (i=j=0; i<(int)State.eol; i++)
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

	Cmdline[State.eol++] = c + '0';
	Cmdline[State.eol] = '\0';

	if (! intm && State.cmdlineeex) {
		char *p = &Cmdline[State.cmdlineeex + 1];
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
				State.eol--;
				Cmdline[State.eol] = '\0';
			} else
				break;
		}
	}
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
		if (State.cmdlinedot < 2 && !State.cmdlineeex && State.eol < CMDLINELEN) {
			if (State.eol == 0 || Cmdline[State.eol-1] == '.')
				digit(0);
			State.cmdlinedot++;
			Cmdline[State.eol++] = '.';
		}
		break;

	case OP_EEX:
		if (is_intmode() || State.fract || State.cmdlinedot == 2)
			break;
		if (!State.cmdlineeex && State.eol < CMDLINELEN) {
			if (State.eol == 0)
				digit(1);
			State.cmdlineeex = State.eol;
			Cmdline[State.eol++] = 'E';
		}
		break;

	case OP_CHS:
		if (State.eol)
			cmdlinechs();
		else if (is_intmode()) {
			d64fromInt(&regX, intChs(d64toInt(&regX)));
			State.state_lift = 1;
		} else {
			decNumber x, r;

			getX(&x);
			decNumberMinus(&r, &x, Ctx);
			setX(&r);
			State.state_lift = 1;
		}
		break;

	case OP_CLX:
		if (State.eol) {
			State.eol--;
			if (Cmdline[State.eol] == 'E')
				State.cmdlineeex = 0;
			else if (Cmdline[State.eol] == '.')
				State.cmdlinedot--;
		} else {
			if (is_intmode())
				d64fromInt(&regX, 0);
			else
				regX = CONSTANT_INT(OP_ZERO);
			State.state_lift = 0;
		}
		break;

	case OP_ENTER:
		process_cmdline();
		lift();
		State.state_lift = 0;
		break;

	case OP_SIGMAPLUS:
		if (is_intmode())
			break;
		process_cmdline();
		State.state_lift = 0;
		setlastX();
		sigma_plus(Ctx);
		sigma_N(&regX, NULL, Ctx64);
		break;

	case OP_SIGMAMINUS:
		if (is_intmode())
			break;
		process_cmdline();
		State.state_lift = 0;
		setlastX();
		sigma_minus(Ctx);
		sigma_N(&regX, NULL, Ctx64);
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
	if (State.cmplx)
		return TRIG_RAD;
	//if (State.hyp)	return TRIG_RAD;
	return State.trigmode;
}

void set_trig_mode(enum trig_modes m) {
	State.trigmode = m;
}

void op_deg(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	set_trig_mode(TRIG_DEG);
}

void op_rad(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	set_trig_mode(TRIG_RAD);
}

void op_grad(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	set_trig_mode(TRIG_GRAD);
}

void op_all(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	State.dispmode = MODE_STD;
	State.dispdigs = 0;
	State.fract = 0;
}

void op_radixcom(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	State.fraccomma = 1;
	State.fract = 0;
}

void op_radixdot(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	State.fraccomma = 0;
	State.fract = 0;
}

void op_thousands_off(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	State.nothousands = 1;
}

void op_thousands_on(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	State.nothousands = 0;
}

void op_fixsci(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	State.fixeng = 0;
	State.fract = 0;
}

void op_fixeng(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	State.fixeng = 1;
	State.fract = 0;
}


void op_pause(unsigned int arg, enum rarg op) {
	if (running()) {
		display();
#if defined(REALBUILD) || defined(WINGUI)
		// decremented in the low level heartbeat
		State.pause = arg;
#else
#ifdef WIN32
#pragma warning(disable:4996)
		sleep(arg/10);
#else
		usleep(arg * 100000);
#endif
#endif
	}
}

void op_2comp(decimal64 *a, decimal64 *b, decContext *nulc) {
	State.int_mode = MODE_2COMP;
}

void op_1comp(decimal64 *a, decimal64 *b, decContext *nulc) {
	State.int_mode = MODE_1COMP;
}

void op_unsigned(decimal64 *a, decimal64 *b, decContext *nulc) {
	State.int_mode = MODE_UNSIGNED;
}

void op_signmant(decimal64 *a, decimal64 *b, decContext *nulc) {
	State.int_mode = MODE_SGNMANT;
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
static void check_int_switch(void) {
	decNumber x, y, z;
	int ex;			/* exponent |ex| < 1000 */
	unsigned long int m;	/* Mantissa 32 bits */
	int sgn, i;

	if (!is_intmode()) {
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

		} else if (decNumberIsZero(&x)) {
			/* 0 exponent, 0 mantissa -- although this can be negative zero */
			d64fromInt(&regY, build_value(0, decNumberIsNegative(&x)?1:0));
			d64fromInt(&regX, 0);
		} else {
			/* Deal with the sign */
			if (decNumberIsNegative(&x)) {
				decNumberMinus(&x, &x, Ctx);
				sgn = 1;
			} else
				sgn = 0;
			/* Figure the exponent */
			decNumberLog2(&y, &x, Ctx);
			decNumberTrunc(&z, &y, Ctx);
			ex = dn_to_int(&z, Ctx);
			/* On to the mantissa */
			decNumberPow2(&y, &z, Ctx);
			decNumberDivide(&z, &x, &y, Ctx);
			m = 1;
			decNumberFrac(&y, &z, Ctx);
			for (i=0; i<31; i++) {
				decNumberMultiply(&z, &y, &const_2, Ctx);
				decNumberTrunc(&y, &z, Ctx);
				m += m;
				if (! decNumberIsZero(&y))
					m++;
				decNumberFrac(&y, &z, Ctx);
			}
			ex -= 31;
			/* Finally, round up if required */
			decNumberMultiply(&z, &y, &const_2, Ctx);
			decNumberTrunc(&y, &z, Ctx);
			if (! decNumberIsZero(&y)) {
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
		State.intm = 1;
	}
}

static void set_base(unsigned int b) {
	State.int_base = b - 1;
	check_int_switch();
}

void set_int_base(unsigned int arg, enum rarg op) {
	if (arg < 2) {
		if (arg == 0)
			op_float(NULL, NULL, NULL);
		else
			op_fract(NULL, NULL, NULL);
	} else
		set_base(arg);
}

void date_ymd(decimal64 *a, decimal64 *nul, decContext *ctx) {
	State.date_mode = DATE_YMD;
}

void date_dmy(decimal64 *a, decimal64 *nul, decContext *ctx) {
	State.date_mode = DATE_DMY;
}

void date_mdy(decimal64 *a, decimal64 *nul, decContext *ctx) {
	State.date_mode = DATE_MDY;
}

void time_24(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	State.t12 = 0;
}

void time_12(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	State.t12 = 1;
}

/* Save and restore used State.
 */
void op_rclflag(decimal64 *x, decimal64 *b, decContext *ctx64) {
	unsigned long long int n = 0;
	unsigned int posn = 0;
	decNumber r;

#define SB(f, p)					\
		n |= (State.f) << posn;			\
		posn += (p)
#include "statebits.h"
#undef SB
	if (State.intm) {
		if (word_size() < posn)
			err(ERR_INT_SIZE);
		else
			d64fromInt(x, n);
	} else {
		ullint_to_dn(&r, n, Ctx);
		decimal64FromNumber(x, &r, Ctx);
	}
}

void op_stoflag(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	unsigned long long int n;
	int sgn;
	decNumber x;

	if (State.intm) {
	/* Figure out the number of bits required.  This is optimised
	 * into a single constant assignment.
	 */
		unsigned int nb = 0;
#define SB(f, p)	nb += (p);
#include "statebits.h"
#undef SB
		if (word_size() < nb) {
			err(ERR_INT_SIZE);
			return;
		}
		n = d64toInt(&regX);
	} else {
		getX(&x);
		n = dn_to_ull(&x, Ctx, &sgn);
	}

#define SB(f, p)					\
		(State.f) = n & ((1 << (p)) - 1);	\
		n >>= (p)
#include "statebits.h"
#undef SB
}

static void do_rtn(int plus1) {
	if (RetStkPtr > 0) {
		raw_set_pc(RetStk[--RetStkPtr]);
		RetStk[RetStkPtr] = 0;
		if (plus1)
			incpc();
	} else {
		set_running_off();
		raw_set_pc(0);
	}
}

void op_rtn(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	if (!State.implicit_rtn)
		do_rtn(0);
}

void op_rtnp1(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	if (!State.implicit_rtn)
		do_rtn(1);
}

void op_rs(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	if (running())	set_running_off();
	else		set_running_on();
}

void op_prompt(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	set_running_off();
	alpha_view(NULL, NULL, NULL);
}

void do_usergsb(decimal64 *a, decimal64 *b, decContext *nulc) {
	unsigned int usrpc = State.usrpc;
	const unsigned int pc = state_pc();
	if (usrpc != 0 && isXROM(pc))
		gsbgto(usrpc, 1, pc);
}

/* Test if a number is an integer */
void XisInt(decimal64 *a, decimal64 *b, decContext *nulc) {
	decNumber x;

	getX(&x);
	fin_tst(is_intmode() || is_int(&x, Ctx));
}

/* Test if a number has a fractional component */
void XisFrac(decimal64 *a, decimal64 *b, decContext *nulc) {
	decNumber x;

	getX(&x);
	fin_tst(!is_intmode() && !is_int(&x, Ctx));
}

/* Utility routine that checks if the X register is even or odd or neither.
 * Returns positive if even, zero if odd, -1 for special, -2 for fractional.
 */
int is_even(const decNumber *x) {
	decNumber y, z;

	if (decNumberIsSpecial(x))
		return -1;
	decNumberAbs(&z, x, Ctx);
	decNumberRemainder(&y, &z, &const_2, Ctx);
	if (decNumberIsZero(&y))
		return 1;
	decNumberCompare(&z, &y, &const_1, Ctx);
	if (decNumberIsZero(&z))
		return 0;
	return -2;
}

static int evenX() {
	decNumber x;

	getX(&x);
	return is_even(&x);
}

/* Test if a number is an even integer */
void XisEven(decimal64 *a, decimal64 *b, decContext *nulc) {
	if (is_intmode()) {
		fin_tst((d64toInt(&regX) & 1) == 0);
	} else {
		fin_tst(evenX() == 1);
	}
}

/* Test if a number is an odd integer */
void XisOdd(decimal64 *a, decimal64 *b, decContext *nulc) {
	if (is_intmode()) {
		fin_tst((d64toInt(&regX) & 1) != 0);
	} else {
		fin_tst(evenX() == 0);
	}
}

/* Test if a number is prime */
void XisPrime(decimal64 *a, decimal64 *b, decContext *nulc) {
	int sgn;

	fin_tst(isPrime(get_int(&regX, &sgn)) && sgn == 0);
}

/* Test is a number is infinite.
 */
void isInfinite(decimal64 *a, decimal64 *b, decContext *nulc) {
	decNumber x;

	getX(&x);
	fin_tst(!is_intmode() && decNumberIsInfinite(&x));
}

/* Test for NaN.
 * this could be done by testing x != x, but having a special command
 * for it reads easier.
 */
void isNan(decimal64 *a, decimal64 *b, decContext *nulc) {
	decNumber x;

	getX(&x);
	fin_tst(!is_intmode() && decNumberIsNaN(&x));
}

void isSpecial(decimal64 *a, decimal64 *b, decContext *nulc) {
	decNumber x;

	getX(&x);
	fin_tst(!is_intmode() && decNumberIsSpecial(&x));
}


/* Bulk register operations */
static int reg_decode(unsigned int *s, unsigned int *n, unsigned int *d) {
	decNumber x, y;
	int rsrc, num, rdest, q;

	if (is_intmode())
		return 1;
	getX(&x);
	if (decNumberIsNegative(&x) && !decNumberIsZero(&x)) {
		err(ERR_RANGE);
		return 1;
	}
	decNumberTrunc(&y, &x, Ctx);
	*s = rsrc = dn_to_int(&y, Ctx);
	if (rsrc >= TOPREALREG) {
		err(ERR_RANGE);
		return 1;
	}
	decNumberFrac(&y, &x, Ctx);
	decNumberMultiply(&x, &y, &const_100, Ctx);
	decNumberTrunc(&y, &x, Ctx);
	*n = num = dn_to_int(&y, Ctx);
	if (d != NULL) {
		decNumberFrac(&y, &x, Ctx);
		decNumberMultiply(&x, &y, &const_100, Ctx);
		decNumberTrunc(&y, &x, Ctx);
		*d = rdest = dn_to_int(&y, Ctx);
		if (num == 0) {
			/* Calculate the maxium non-ovelapping size */
			if (rsrc > rdest) {
				num = TOPREALREG - rsrc;
				q = rsrc - rdest;
			} else {
				num = TOPREALREG - rdest;
				q = rdest - rsrc;
			}
			if (num > q)
				num = q;
			*n = num;
		} else if (rsrc+num > TOPREALREG || rdest+num > TOPREALREG) {
			err(ERR_RANGE);
			return 1;
		}
	} else {
		if (num == 0) {
			*n = TOPREALREG - rsrc;
		} else if (rsrc+num > TOPREALREG) {
			err(ERR_RANGE);
			return 1;
		}
	}
	return 0;
}

void op_regcopy(decimal64 *a, decimal64 *b, decContext *nulc) {
	unsigned int s, d, n;

	if (reg_decode(&s, &n, &d) || s == d)
		return;
	xcopy(Regs+d, Regs+s, n*sizeof(Regs[0]));
}

void op_regswap(decimal64 *a, decimal64 *b, decContext *nulc) {
	unsigned int s, d, n, i;

	if (reg_decode(&s, &n, &d) || s == d)
		return;
	if (s < d && (s+n) > d)
		err(ERR_RANGE);
	else if (d < s && (d+n) > s)
		err(ERR_RANGE);
	else {
		for (i=0; i<n; i++)
			swap_reg(Regs+s+i, Regs+d+i);
	}
}

void op_regclr(decimal64 *a, decimal64 *b, decContext *nulc) {
	unsigned int s, n, i;

	if (reg_decode(&s, &n, NULL))
		return;
	for (i=0; i<n; i++)
		Regs[i+s] = CONSTANT_INT(OP_ZERO);
}

void op_regsort(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	unsigned int s, n;
	decNumber pivot, a, t;
	int beg[10], end[10], i;

	if (reg_decode(&s, &n, NULL) || n == 1)
		return;

	/*( Non-recursive quickqort */
	beg[0] = s;
	end[0] = s + n;
	i = 0;
	while (i>=0) {
		int L = beg[i];
		int R = end[i] - 1;
		if (L<R) {
			const decimal64 pvt = Regs[L];
			decimal64ToNumber(&pvt, &pivot);
			while (L<R) {
				while (L<R) {
					decimal64ToNumber(Regs+R, &a);
					decNumberCompare(&t, &a, &pivot, Ctx);
					if (decNumberIsNegative(&t) && !decNumberIsZero(&t))
						break;
					R--;
				}
				if (L<R)
					Regs[L++] = Regs[R];
				while (L<R) {
					decimal64ToNumber(Regs+L, &a);
					decNumberCompare(&t, &pivot, &a, Ctx);
					if (decNumberIsNegative(&t) && !decNumberIsZero(&t))
						break;
					L++;
				}
				if (L<R)
					Regs[R--] = Regs[L];
			}
			Regs[L] = pvt;
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
 * account of the indirction flag and various limits -- we always work modulo
 * the limit.
 */
static void rargs(const opcode op) {
	unsigned int arg = op & RARG_MASK;
	int ind = op & RARG_IND;
	const unsigned int cmd = (op & ~OP_RARG) >> RARG_OPSHFT;
	decNumber x;
	unsigned int lim = argcmds[cmd].lim;
	if (lim == 0) lim = 256; // default

	process_cmdline_set_lift();

	if (cmd >= num_argcmds) {
		illegal(op);
		return;
	}
	if (argcmds[cmd].f == NULL)
		return;
	if (ind && argcmds[cmd].indirectokay) {
		if (is_intmode()) {
			arg = (unsigned int) get_reg_n_as_int(arg);
		} else {
			get_reg_n_as_dn(arg, &x);
			arg = dn_to_int(&x, Ctx);
		}
	} else {
		if (lim > 128 && ind)		// put the top bit back in
			arg |= RARG_IND;
	}
	if (arg >= lim)
		err(ind?ERR_RANGE:ERR_PROG_BAD);
	else
		(argcmds[cmd].f)(arg, (enum rarg)cmd);
}

static void multi(const opcode op) {
	const int cmd = opDBL(op);

	process_cmdline_set_lift();

	if (cmd >= num_multicmds) {
		illegal(op);
		return;
	}
	if (multicmds[cmd].f == NULL)	// LBL does nothing
		return;
	(multicmds[cmd].f)(op, (enum multiops)cmd);
}


/* Print a single program step nicely.
 */
static void print_step(char *tracebuf, const opcode op) {
	char buf[16], *p;
	const unsigned int pc = state_pc();

	if (isXROM(pc)) {
		tracebuf[0] = 'x';
		p = num_arg_0(tracebuf+1, pc - addrXROM(0), 3);
		*p++ = ' ';
	} else if (pc == 0) {
		scopy(tracebuf, "000:");
		return;
	} else {
		p = num_arg_0(tracebuf, pc, 3);
		*p++ = ':';
	}
	scopy_char(p, prt(op, buf), '\0');
}


/* When stuff gets done, there are some bits of state that need
 * to be reset -- SHOW, ->base change the display mode until something
 * happens.  This should be called on that something.
 */
void reset_volatile_state(void) {
	if (State.implicit_rtn)
		process_cmdline_set_lift();
	State.int_window = 0;
	State.int_maxw = 0;
	State.implicit_rtn = 0;

	State.smode = SDISP_NORMAL;
}


/* Main dispatch routine that decodes the top level of the opcode and
 * goes to the appropriate lower level dispatch routine.
 */
void xeq(opcode op) 
{
	const int ss = stack_size();
	const int nreg = ss + 2;
	decimal64 save[STACK_SIZE+2];
	struct _state old = State;
	enum errors er;

#ifndef REALBUILD
	if (State.trace) {
		char buf[16];
		static char tracebuf[24];

		if (running())
			print_step(tracebuf, op);
		else
			sprintf(tracebuf, "%04X:%s", op, prt(op, buf));
		DispMsg = tracebuf;
	}
#endif

	if (ss == 4)
		xcopy(save, &regX, nreg * sizeof(decimal64));
	else	xcopy(save, get_stack(ss-2), nreg * sizeof(decimal64));
	if (isDBL(op))
		multi(op);
	else if (isRARG(op))
		rargs(op);
	else {
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

	if ((er = State.error) != ERR_NONE) {
		if (ss == 4)
			xcopy(&regX, save, nreg * sizeof(decimal64));
		else	xcopy(get_stack(ss-2), save, nreg * sizeof(decimal64));
		State = old;
		State.error = er;
		set_running_off();
		RetStkPtr = 0;
		BankFlags = 0;
		if (isXROM(state_pc()))
			set_pc(0);
		process_cmdline_set_lift();
	} else if (State.implicit_rtn) {
		do_rtn(0);
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

/* Check to see if we're running a program and if so execute it
 * for a while.
 *
 */
void xeqprog(void) 
{
	int state = 0;

	if ( running() ) {
#if defined(REALBUILD) || defined(WINGUI)
		long long last_ticker = Ticker;
		state = ((int) last_ticker % (2*TICKS_PER_FLASH) < TICKS_PER_FLASH);
#else
		state = 1;
#endif
		dot(RCL_annun, state);
		finish_display();

		while (!State.pause && running() && !is_key_pressed()) {
			xeq_single();
		}
	}
	if (!running()) {
		// Program has terminated
		clr_dot(RCL_annun);
		finish_display();
	}
}

/* Single step routine
 */
void xeqone(char *tracebuf) {
	const opcode op = getprog(state_pc());
#ifdef REALBUILD
	const unsigned int trace = 0;
#else
	unsigned int trace = State.trace;
#endif

	set_running_on();
#ifndef REALBUILD
	State.trace = 0;
#endif
	print_step(tracebuf, op);
	DispMsg = tracebuf;
	State.disp_small = 1;
	incpc();
	xeq(op);

	/* Now if we've stepped into the xROM area, keep going until
	 * we break free.
	 */
	while (!trace && isXROM(state_pc()))
		xeq_single();

	set_running_off();
#ifndef REALBUILD
	State.trace = trace;
#endif
}

/* Single step
 */
void xeq_sst(char *tracebuf) {
	reset_volatile_state();
	if (State.runmode)
		xeqone(tracebuf);
	else
		incpc();
}

/* Back step
 */
void xeq_bst(char *tracebuf) {
	reset_volatile_state();
	decpc();
	if (State.runmode) {
		State.disp_small = 1;
		print_step(tracebuf, getprog(state_pc()));
		DispMsg = tracebuf;
	}
}

/* Store into program space.
 */
void stoprog(opcode c) {
	int i;
	int off;
	unsigned int pc;

	if (isXROM(state_pc()))
		return;
	off = isDBL(c)?2:1;
	if (State.last_prog + off > NUMPROG+1) {
		return;
	}
	State.last_prog += off;
	incpc();
	pc = state_pc();
	for (i=State.last_prog; i>(int)pc; i--)
		prog[i] = prog[i-off];
	if (pc != 0) {
		if (isDBL(c))
			prog[pc + 1] = c >> 16;
		prog[pc] = c;
	}
}


/* Delete the current step in the program
 */
void delprog(void) {
	int i;
	const unsigned pc = state_pc();
	int off;

	if (pc == 0 || isXROM(pc))
		return;
	off = isDBL(prog[pc])?2:1;
	for (i=pc; i<(int)State.last_prog-1; i++)
		prog[i] = prog[i+off];
	do {
		prog[State.last_prog] =  EMPTY_PROGRAM_OPCODE;
		State.last_prog--;
	} while (--off);
	decpc();
}


void xeq_init_contexts(void) {
	/* Initialise our standard contexts.
	 * We have to disable traps and bump the digits for internal calculations.
	 */
	decContextDefault(Ctx, DEC_INIT_BASE);
	Ctx->traps = 0;
	Ctx->digits = DECNUMDIGITS;
	Ctx->emax=DEC_MAX_MATH;
	Ctx->emin=-DEC_MAX_MATH;
	Ctx->round = DEC_ROUND_HALF_EVEN;

	decContextDefault(Ctx64, DEC_INIT_DECIMAL64);
	Ctx64->traps = 0;
}


#if defined(DEBUG) && !defined(WINGUI)
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
	const int oldcata = State.catalogue;
	int n;

	State.catalogue = cata;
	n = current_catalogue_max();
	for (i=1; i<n; i++) {
		opcode cold = current_catalogue(i-1);
		opcode c = current_catalogue(i);
		if (compare(cold, c, cata))
			error("catalogue %s row %04x / %04x  %d / %d: %04o / %04o (%s / %s)", name, cold, c, i-1, i,
					0xff & cold, 0xff & c,
					catcmd(cold, b1), catcmd(c, b2));
	}
	State.catalogue = oldcata;
}

static void check_const_cat(void) {
	int i;
	char b1[16], b2[16];

	for (i=1; i<NUM_CONSTS; i++) {
		if (compare(CONST(i-1), CONST(i), 0))
			error("constants row %d / %d: %s / %s", i, i+1,
					catcmd(CONST(i-1), b1), catcmd(CONST(i), b2));
	}
}
#endif

/* Main initialisation routine that sets things up for us.
 */
void xeq_init(void) {
	if (State.magic != MAGIC_MARKER)
		reset(NULL, NULL, NULL);


#if defined(DEBUG) && !defined(WINGUI)
    {
	/* Sanity check the function table indices.
	 * These indicies must correspond exactly with the enum definition.
	 * This code validates that this is true and prints error messages
	 * if it isn't.
	 */
	int i;
	for (i=0; i<num_monfuncs; i++)
		if (monfuncs[i].n != i)
			error("monadic function table row %d: %6s", i, monfuncs[i].fname);
	for (i=0; i<num_dyfuncs; i++)
		if (dyfuncs[i].n != i)
			error("dyadic function table row %d: %6s", i, dyfuncs[i].fname);
	for (i=0; i<num_trifuncs; i++)
		if (trifuncs[i].n != i)
			error("triadic function table row %d: %6s", i, trifuncs[i].fname);
	for (i=0; i<num_niladics; i++)
		if (niladics[i].n != i)
			error("niladic function table row %d: %6s", i, niladics[i].nname);
	for (i=0; i<num_argcmds; i++)
		if (argcmds[i].n != i)
			error("argument command table row %d: %6s", i, argcmds[i].cmd);
	for (i=0; i<num_multicmds; i++)
		if (multicmds[i].n != i)
			error("multi command table row %d: %6s", i, multicmds[i].cmd);
	check_const_cat();
	check_cat(CATALOGUE_COMPLEX, "complex");
	check_cat(CATALOGUE_STATS, "statistics");
	check_cat(CATALOGUE_PROB, "probability");
	check_cat(CATALOGUE_PROG, "programme");
	check_cat(CATALOGUE_MODE, "mode");
	check_cat(CATALOGUE_TEST, "tests");
	check_cat(CATALOGUE_INT, "int");
	check_cat(CATALOGUE_ALPHA, "alpha");
	check_cat(CATALOGUE_ALPHA_LETTERS_UPPER, "alpha special upper case letters");
	check_cat(CATALOGUE_ALPHA_LETTERS_LOWER, "alpha special lower letters");
	check_cat(CATALOGUE_ALPHA_SUPERSCRIPTS, "alpha superscripts");
	check_cat(CATALOGUE_ALPHA_SUBSCRIPTS, "alpha subscripts");
	check_cat(CATALOGUE_ALPHA_SYMBOLS, "alpha symbols");
	check_cat(CATALOGUE_ALPHA_COMPARES, "alpha compares");
	check_cat(CATALOGUE_ALPHA_ARROWS, "alpha arrows");
	check_cat(CATALOGUE_CONV, "conversion");
	check_cat(CATALOGUE_NORMAL, "float");
#ifdef INCLUDE_INTERNAL_CATALOGUE
	check_cat(CATALOGUE_INTERNAL, "internal");
#endif
    }
#endif
}


static unsigned int crc_step(unsigned int crc, unsigned char byte, unsigned int ct[256]) {
	return ct[0xff & (crc^byte)] ^ (crc >> 8);
}

static unsigned int crc_step2(unsigned int crc, unsigned short s, unsigned int ct[256]) {
	unsigned int t = crc_step(crc, s & 0xff, ct);
	return crc_step(t, s >> 8, ct);
}

unsigned int checksum_code(void) {
	unsigned int ct[256];
	int i;
	unsigned int crc = 0;
	int n = State.last_prog;

	/* Build up a CRC table */
	for (i=0; i<256; i++) {
		unsigned int r = i;
		int j;
		for (j=8; j > 0; j--) {
			const unsigned int s = r>>1;
			r = (r&1)?(s^0xEDB88320):s;
		}
		ct[i] = r;
	}

	/* Now calculate the checksum */
	crc = crc_step2(0, n, ct);
	for (i=1; i<n; i++)
		crc = crc_step2(crc, prog[i], ct);
	return crc;
}

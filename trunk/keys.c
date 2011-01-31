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

#include "xeq.h" 
#include "keys.h"
#include "display.h"
#include "lcd.h"
#include "int.h"
#include "consts.h"

#include "catalogues.h"


#define STATE_UNFINISHED	-1
#define STATE_BACKSPACE		-2
#define STATE_SST		-3

/* Define this if the keycodes map rows sequentially */
#define SEQUENTIAL_ROWS


typedef enum {
	K00 = 0,  K01 = 1,  K02 = 2,  K03 = 3,  K04 = 4,  K05 = 5,
	K10 = 6,  K11 = 7,  K12 = 8,
#define K13	9		/* Shift keys aren't in the enum since they are handled */
#define K14	10		/* Directly in the main key processing loop */
#define K15	11
	K20 = 12, K21 = 13, K22 = 14, K23 = 15, K24 = 16,
	K30 = 17, K31 = 18, K32 = 19, K33 = 20, K34 = 21,
	K40 = 22, K41 = 23, K42 = 24, K43 = 25, K44 = 26,
	K50 = 27, K51 = 28, K52 = 29, K53 = 30, K54 = 31,
	K60 = 32, K61 = 33, K62 = 34, K63 = 35, K64 = 36,
} keycode;


#define K_UNKNOWN	-1
#define K_F		K13
#define K_G		K14
#define K_H		K15
#define K_ARROW		K04
#define K_CMPLX		K05


#define CH_QUIT		'Q'
#define CH_TRACE	'T'
#define CH_FLAGS	'F'
#define CH_REFRESH	12	/* ^L */

#define TEST_EQ		0
#define TEST_NE		1
#define TEST_LT		2
#define TEST_LE		3
#define TEST_GT		4
#define TEST_GE		5

enum confirmations {
	confirm_none=0, confirm_clall, confirm_reset, confirm_clprog
};

enum shifts cur_shift(void) {
	return state.shifts;
}


static void init_arg(const enum rarg base) {
	state.base = base;
	state.ind = 0;
	state.digval = 0;
	state.numdigit = 0;
	state.rarg = 1;
	state.dot = 0;
}

static void init_cat(enum catalogues cat) {
	process_cmdline_set_lift();
	state.catalogue = cat;
	state.cmplx = (cat == CATALOGUE_COMPLEX || cat == CATALOGUE_COMPLEX_CONST)?1:0;
	state.digval = 0;
	state.shifts = SHIFT_N;
}

static void init_state(void) {
	struct state s;
	xset(&s, 0, sizeof(s));

#define C(n)	s.n = state.n
	C(runmode);
#ifndef REALBUILD
	C(trace);
#endif
	C(fraccomma);
	C(last_prog);
	C(sigma_mode);
	C(intm);
	C(int_len);
	C(dispdigs);
	C(trigmode);
	C(dispmode);
	C(int_base);
	C(int_mode);
	C(date_mode);
#undef C
	s.shifts = SHIFT_N;
	s.test = TST_NONE;

	xcopy(&state, &s, sizeof(struct state));
}

static void init_confirm(enum confirmations n) {
	state.confirm = n;
}

static void set_smode(const enum single_disp d) {
	state.smode = (state.smode == d)?SDISP_NORMAL:d;
}

/* Mapping from key position to alpha in the four key planes plus
 * the two lower case planes.
 */
static const unsigned char alphamap[][6] = {
    "A\221\200\221a\240", // K00
    "BB\201\043b\241",  
    "CC\202\0c\242",
    "D\003\203?d\243",    // K03
    "E\015\204\015e\244",       
    "F\024\224\043f\264",  // K05
    
    "G\0\202\020g\242", // K10
    "H\0\225\016h\265", // K11
    "I\0\210\0i\250", // K12
    "??????", 
    "??????",
    "??????", // K15
    
    "\0\240\0\0\0\0", // K20
    "J\027\206\027j\246", // K21
    "K\010\211\\k\251",
    "L\246\212\257l\252",
    "\0\0\0\0\0\0", // K24

    "\0{}\0\0}", // K30
    "M7\213&m\253", // K31
    "N8\214|n\254", // K32
    "O9\227\013o\267", // K33
    "P/\217\235p\257", // K34

    "\020[]\0\0]", // K40
    "Q4\216?q\256", // K41
    "R5\220\0r\260",
    "S6\221$s\261",
    "T\034\207\217t\247", // K44
    
    "\017()\0\0)", // K50
    "1\0\0\0\0\0", // K51
    "U2\0\014u\0",
    "V3\0\036v\0",
    "W-\222%w\262", // K54

    "\0\0\0 \0\0", // K60
    "00\226\0\0\266",
    "X.\215\0x\255",
    "Y\0\223\037y\263",
    "Z+\205%z\245", // K64
};

static unsigned char keycode_to_alpha(const keycode c, unsigned int s) 
{
	if (state.alphashift) {
		if (s == SHIFT_N)
			s = SHIFT_LC_N;
		else if (s == SHIFT_G)
			s = SHIFT_LC_G;
	}
	return alphamap[c][s];
}

static int check_f_key(int n, const int dflt) {
	int code = 99 + n;

	if (find_label_from(1, code, 1))
		return RARG(RARG_XEQ, code);
	return dflt;
}

/* Return non-zero if the current mode is integer and we acept letters
 * as digits.
 */
static int intltr(int d) {
	return (is_intmode() && int_base() > d);
}

static int process_normal(const keycode c) {
	switch (c) {
	case K00:
		if (intltr(10))
			return OP_SPEC | OP_A;
		return OP_SPEC | OP_SIGMAPLUS;
	case K01:
		if (intltr(11))
			return OP_SPEC | OP_B;
		return check_f_key(1, OP_MON | OP_RECIP);
	case K02:
		if (intltr(12))
			return OP_SPEC | OP_C;
		return check_f_key(2, OP_DYA | OP_POW);
	case K03:
		if (intltr(13))
			return OP_SPEC | OP_D;
		return check_f_key(3, OP_MON | OP_SQRT);
	case K_ARROW:
		if (intltr(14))
			return OP_SPEC | OP_E;
		state.arrow = 1;
		break;
	case K_CMPLX:
		if (intltr(15))
			return OP_SPEC | OP_F;
		if (!state.intm)
			state.cmplx = 1;
		break;

	case K10:					// STO
		init_arg(RARG_STO);
		break;

	case K11:					// RCL
		init_arg(RARG_RCL);
		break;

	case K12:					// R down
		return OP_NIL | OP_RDOWN;

	case K20:	return OP_SPEC | OP_ENTER;
	case K21:					// swap
		return OP_NIL | OP_SWAP;
	case K22:	return OP_SPEC | OP_CHS;	// CHS
	case K23:	return OP_SPEC | OP_EEX;	// EEX
	case K24:
		if (state.runmode)
			return OP_SPEC | OP_CLX;
		return STATE_BACKSPACE;

	case K30:	init_arg(RARG_XEQ);	break;	// XEQ/GSB
#ifdef SEQUENTIAL_ROWS
	case K31:	case K32:	case K33:
		return OP_SPEC | (c - K31 + OP_7);
#else
	case K31:	return OP_SPEC | OP_7;
	case K32:	return OP_SPEC | OP_8;
	case K33:	return OP_SPEC | OP_9;
#endif
	case K34:	return OP_DYA | OP_DIV;

	case K40:					// BST
		reset_volatile_state();
		decpc();
		break;

#ifdef SEQUENTIAL_ROWS
	case K41:	case K42:	case K43:
		return OP_SPEC | (c - K41 + OP_4);
#else
	case K41:	return OP_SPEC | OP_4;
	case K42:	return OP_SPEC | OP_5;
	case K43:	return OP_SPEC | OP_6;
#endif
	case K44:	return OP_DYA | OP_MUL;

	case K50:	return STATE_SST;		// SST

#ifdef SEQUENTIAL_ROWS
	case K51:	case K52:	case K53:
		return OP_SPEC | (c - K51 + OP_1);
#else
	case K51:	return OP_SPEC | OP_1;
	case K52:	return OP_SPEC | OP_2;
	case K53:	return OP_SPEC | OP_3;
#endif
	case K54:	return OP_DYA | OP_SUB;

	case K60:	break;				// ON/C
	case K61:	return OP_SPEC | OP_0;
	case K62:	return OP_SPEC | OP_DOT;
	case K63:	return OP_NIL | OP_RS;		// R/S
	case K64:	return OP_DYA | OP_ADD;
	}
	return STATE_UNFINISHED;
}


static int process_f_shifted(const keycode c) {
	state.shifts = SHIFT_N;
	switch (c) {
	case K00:
		if (intltr(10))
			return OP_SPEC | OP_SIGMAPLUS;
		state.hyp = 1;
		state.dot = 1;
		state.cmplx = 0;
		break;
	case K01:
		if (intltr(11))
			return check_f_key(1, OP_MON | OP_RECIP);
		return OP_MON | OP_SIN;
	case K02:
		if (intltr(12))
			return check_f_key(2, OP_DYA | OP_POW);
		return OP_MON | OP_COS;
	case K03:
		if (intltr(13))
			return check_f_key(3, OP_MON | OP_SQRT);
		return OP_MON | OP_TAN;
	case K_ARROW:
		if (intltr(14)) {
			state.arrow = 1;
			break;
		}
		return OP_NIL | OP_P2R;
	case K_CMPLX:
		return OP_NIL | OP_FRACPROPER;

	case K10:	return OP_NIL | OP_HMS;
	case K11:	return OP_NIL | OP_FLOAT;
	case K12:	return OP_NIL | OP_RANDOM;

	case K20:
		process_cmdline_set_lift();
		state.alpha = 1;
		break;

	case K21:	return OP_NIL | OP_ALPHATOX;
	case K22:	return RARG(RARG_BASE, 2);
	case K23:	return RARG(RARG_BASE, 10);
	case K24:	return OP_NIL | OP_CLRALPHA;

	case K30:	return OP_MON | OP_EXP;
	case K31:	return OP_MON | OP_10POWX;
	case K32:	return OP_MON | OP_2POWX;
	case K33:	return OP_DYA | OP_POW;
	case K34:	return OP_MON | OP_RECIP;

	case K40:	return OP_DYA | OP_COMB;
	case K41:	return OP_MON | OP_cdf_Q;
	case K42:	return OP_NIL | OP_statMEAN;
	case K43:	return OP_MON | OP_yhat;
	case K44:	return OP_MON | OP_SQRT;

	case K50:
		if (state.intm && state.int_winl)
			state.int_window++;
		break;
	case K51:	state.test = TST_EQ;	break;
	case K52:	init_arg(RARG_SOLVE);	break;
	case K53:	init_arg(RARG_PROD);	break;
	case K54:	return OP_MON | OP_PERCHG;

	case K60:
		init_state();
		break;

	case K61:	return OP_MON | OP_ABS;
	case K62:	return OP_MON | OP_TRUNC;
	case K63:	init_arg(RARG_LBL);	break;
	case K64:
		//init_arg(state.intm?RARG_DSZ:RARG_DSE);
		init_arg(RARG_DSE);
		break;
	}
	return STATE_UNFINISHED;
}

static int process_g_shifted(const keycode c) {
	state.shifts = SHIFT_N;
	switch (c) {
	case K00:
		state.hyp = 1;
		state.dot = 0;
		state.cmplx = 0;
		break;
	case K01:	return OP_MON | OP_ASIN;
	case K02:	return OP_MON | OP_ACOS;
	case K03:	return OP_MON | OP_ATAN;
	case K04:	return OP_NIL | OP_R2P;
	case K05:	return OP_NIL | OP_FRACIMPROPER;

	case K10:	return OP_NIL | OP_DEG;
	case K11:	return OP_NIL | OP_RAD;
	case K12:	return OP_NIL | OP_GRAD;

	case K20:	return OP_NIL | OP_FILL;
	case K21:	return OP_NIL | OP_XTOALPHA;
	case K22:	return RARG(RARG_BASE, 8);
	case K23:	return RARG(RARG_BASE, 16);
	case K24:	return OP_NIL | OP_SIGMACLEAR;

	case K30:	return OP_MON | OP_LN;
	case K31:	return OP_MON | OP_LOG;
	case K32:	return OP_MON | OP_LG2;
	case K33:	return OP_DYA | OP_LOGXY;
	case K34:	return OP_DYA | OP_PARAL;

	case K40:       return OP_DYA | OP_PERM;
	case K41:	return OP_MON | OP_qf_Q;
	case K42:	return OP_NIL | OP_statS;
	case K43:       return OP_NIL | OP_statR;
	case K44:	return OP_MON | OP_SQR;

	case K50:	
		if (state.intm && state.int_winr)
			state.int_window--;
		break;
	case K51:	state.test = TST_NE;	break;
	case K52:	init_arg(RARG_INTG);	break;
	case K53:	init_arg(RARG_SUM);	break;
	case K54:	return OP_MON | OP_PERCNT;

	case K60:
		init_state();
		break;

	case K61:	return OP_MON | OP_RND;
	case K62:	return OP_MON | OP_FRAC;
	case K63:	return OP_NIL | OP_RTN;
	case K64:
		//init_arg(state.intm?RARG_ISZ:RARG_ISG);
		init_arg(RARG_ISG);
		break;
	}
	return STATE_UNFINISHED;
}

static int process_h_shifted(const keycode c) {
	state.shifts = SHIFT_N;
	switch (c) {
	case K00:	return OP_SPEC | OP_SIGMAMINUS;
	case K01:	init_arg(RARG_FIX);	break;
	case K02:	init_arg(RARG_SCI);	break;
	case K03:	init_arg(RARG_ENG);	break;
	case K04:	
		init_cat(CATALOGUE_CONV);
		break;
	case K05:
		init_cat(CATALOGUE_MODE);
		break;

	case K10:	init_arg(RARG_VIEW);	break;
	case K11:	return OP_NIL | OP_TIME;
	case K12:					// R^
		return OP_NIL | OP_RUP;

	case K20:
		if (! state.intm)
			init_cat(CATALOGUE_CONST);
		break;
	case K21:	init_arg(RARG_SWAP);	break;	// x<>
	case K22:	return OP_MON | OP_NOT;
	case K23:
		if (state.runmode)
			set_pc(0);
		else
			init_confirm(confirm_clprog);
		break;
	case K24:	return OP_SPEC | OP_CLX;

	case K30:	init_arg(RARG_GTO);		break;
	case K31:	return OP_DYA | OP_LAND;
	case K32:	return OP_DYA | OP_LOR;
	case K33:	return OP_DYA | OP_LXOR;
	case K34:	return OP_DYA | OP_MOD;

	case K40:	return OP_MON | OP_FACT;
	case K41:	init_cat(CATALOGUE_PROB);	break;
	case K42:	init_cat(CATALOGUE_STATS);	break;
	case K43:	return OP_NIL | OP_statLR;
	case K44:	state.status = 1;		break;

	case K50:
		init_cat(state.intm?CATALOGUE_INT:CATALOGUE_NORMAL);
		break;
	case K51:	init_cat(CATALOGUE_TEST);	break;
	case K52:	init_cat(CATALOGUE_PROG);	break;
	case K53:	return CONST(OP_PI);
	case K54:	return OP_DYA | OP_PERSB;

	case K60:	set_smode(SDISP_SHOW);		break;
	case K61:	return OP_NIL | OP_PAUSE;
	case K62:
		if (state.fraccomma)
			return OP_NIL | OP_RADDOT;
		return OP_NIL | OP_RADCOM;

	case K63:					// Program<->Run mode
		process_cmdline_set_lift();
		state.runmode = 1 - state.runmode;
		break;
	case K64:	return OP_DYA | OP_PERAD;
	}
	return STATE_UNFINISHED;
}


static int process_normal_cmplx(const keycode c) {
	state.cmplx = 0;
	switch (c) {
	case K01:	return check_f_key(1, OP_CMON | OP_RECIP);
	case K02:	return check_f_key(2, OP_CDYA | OP_POW);
	case K03:	return check_f_key(3, OP_CMON | OP_SQRT);
	case K_CMPLX:	break;

	case K10:	init_arg(RARG_CSTO);	break;	// complex STO
	case K11:	init_arg(RARG_CRCL);	break;	// complex RCL
	case K12:	return OP_NIL | OP_CRDOWN;

	case K20:	return OP_NIL | OP_CENTER;
	case K21:	return OP_NIL | OP_CSWAP;
	case K22:
		return OP_CMON | OP_CCHS;		// CHS
	case K24:				break;

	case K34:	return OP_CDYA | OP_DIV;
	case K44:	return OP_CDYA | OP_MUL;
	case K50:	init_cat(CATALOGUE_COMPLEX);	break;
	case K54:	return OP_CDYA | OP_SUB;
	case K60:	break;
	case K64:	return OP_CDYA | OP_ADD;

	case K00:							case K04:
							case K23:
	case K30:	case K31:	case K32:	case K33:
	case K40:	case K41:	case K42:	case K43:
			case K51:	case K52:	case K53:
			case K61:	case K62:	case K63:
		state.cmplx = 1;
		break;
	}
	return STATE_UNFINISHED;
}

static int process_f_shifted_cmplex(const keycode c) {
	state.shifts = SHIFT_N;
	state.cmplx = 0;
	switch (c) {
	case K00:
		state.hyp = 1;
		state.dot = 1;
		state.cmplx = 1;
		break;
	case K01:	return OP_CMON | OP_SIN;
	case K02:	return OP_CMON | OP_COS;
	case K03:	return OP_CMON | OP_TAN;
	case K04:	return OP_NIL | OP_P2R;

	case K30:	return OP_CMON | OP_EXP;
	case K31:	return OP_CMON | OP_10POWX;
	case K32:	return OP_CMON | OP_2POWX;
	case K33:	return OP_CDYA | OP_POW;
	case K34:	return OP_CMON | OP_RECIP;

	case K40:	return OP_CDYA | OP_COMB;
	case K44:	return OP_CMON | OP_SQRT;

	case K50:	init_cat(CATALOGUE_COMPLEX);	break;
	case K51:
		state.cmplx = 1;
		state.test = TST_EQ;
		break;

	case K60:
		init_state();
		break;

	case K61:	return OP_CMON | OP_ABS;
	case K62:	return OP_CMON | OP_TRUNC;

	case K05:
	case K10:	case K11:	case K12:
	case K20:	case K21:	case K22:	case K23:	case K24:
			case K41:	case K42:	case K43:
					case K52:	case K53:	case K54:
							case K63:	case K64:
		state.shifts = SHIFT_F;
		state.cmplx = 1;
		break;
	}
	return STATE_UNFINISHED;
}

static int process_g_shifted_cmplx(const keycode c) {
	state.shifts = SHIFT_N;
	state.cmplx = 0;
	switch (c) {
	case K00:
		state.hyp = 1;
		state.dot = 0;
		state.cmplx = 1;
		break;
	case K01:	return OP_CMON | OP_ASIN;
	case K02:	return OP_CMON | OP_ACOS;
	case K03:	return OP_CMON | OP_ATAN;
	case K04:	return OP_NIL | OP_R2P;

	case K20:	return OP_NIL | OP_CFILL;

	case K30:	return OP_CMON | OP_LN;
	case K31:	return OP_CMON | OP_LOG;
	case K32:	return OP_CMON | OP_LG2;
	case K33:	return OP_CDYA | OP_LOGXY;
	case K34:	return OP_CDYA | OP_PARAL;

	case K40:	return OP_CDYA | OP_PERM;
	case K44:	return OP_CMON | OP_SQR;

	case K50:	init_cat(CATALOGUE_COMPLEX);	break;
	case K51:
		state.cmplx = 1;
		state.test = TST_NE;
		break;

	case K60:
		init_state();
		break;

	case K61:	return OP_CMON | OP_RND;
	case K62:	return OP_CMON | OP_FRAC;

		case K05:
	case K10:	case K11:	case K12:
			case K21:	case K22:	case K23:	case K24:
			case K41:	case K42:	case K43:
					case K52:	case K53:	case K54:
							case K63:	case K64:
		state.cmplx = 1;
		state.shifts = SHIFT_G;
		break;
	}
	return STATE_UNFINISHED;
}

static int process_h_shifted_cmplx(const keycode c) {
	state.shifts = SHIFT_N;
	state.cmplx = 0;
	switch (c) {
	case K11:
		if (! state.intm)
			init_cat(CATALOGUE_COMPLEX_CONST);
		break;
	case K12:	return OP_NIL | OP_CRUP;

	case K21:	init_arg(RARG_CSWAP);	break;	// x<>
	case K22:	return OP_CMON | OP_CCONJ;
	case K23:	return CONST_CMPLX(OP_PI);

	case K40:	return OP_CMON | OP_FACT;	// z!

	case K50:	init_cat(CATALOGUE_COMPLEX);	break;
	case K52:	init_cat(CATALOGUE_PROG);	break;

	case K60:	break;


	case K00:	case K01:	case K02:	case K03:	case K04:	case K05:
	case K10:
	case K20:							case K24:
	case K30:	case K31:	case K32:	case K33:	case K34:
			case K41:	case K42:	case K43:	case K44:
			case K51:			case K53:	case K54:
			case K61:	case K62:	case K63:	case K64:
		state.cmplx = 1;
		state.shifts = SHIFT_H;
		break;
	}
	return STATE_UNFINISHED;
}


/* Fairly simple routine for dealing with the HYP prefix.
 * This setting can only be followed by 4, 5, or 6 to specify
 * the function.  The inverse routines use the code too, the state.dot
 * is 1 for normal and 0 for inverse hyperbolic.  We also have to
 * deal with the complex versions and the handling of that key and
 * the ON key are dealt with by our caller.
 */
static int process_hyp(const keycode c) {
	const int cmplx = state.cmplx;
	const int dot = state.dot;
	const opcode x = cmplx?OP_CMON:OP_MON;

	state.hyp = 0;
	state.cmplx = 0;
	state.dot = 0;

	switch (c) {
	case K01:	return x + (dot?OP_SINH:OP_ASINH);
	case K02:	return x + (dot?OP_COSH:OP_ACOSH);
	case K03:	return x + (dot?OP_TANH:OP_ATANH);

	case K60:
	case K24:
		break;
	default:
		state.hyp = 1;
		state.cmplx = cmplx;
		state.dot = dot;
		break;
	}
	return STATE_UNFINISHED;
}


static int process_arrow(const keycode c) {
	const enum shifts oldstate = cur_shift();

	state.shifts = SHIFT_N;
	state.arrow = 0;
	switch (c) {
	case K10:
		if (oldstate == SHIFT_N || oldstate == SHIFT_G)
			return OP_MON | OP_2DEG;
		return OP_MON | OP_2HMS;
	case K11:
		if (oldstate == SHIFT_N || oldstate == SHIFT_G)
			return OP_MON | OP_2RAD;
		return OP_MON | OP_HMS2;
	case K12:	return OP_MON | OP_2GRAD;

	case K20:
		if (oldstate == SHIFT_N || oldstate == SHIFT_F)
			state.arrow_alpha = 1;
		break;

	case K22:
		set_smode((oldstate == SHIFT_G)?SDISP_OCT:SDISP_BIN);
		process_cmdline_set_lift();
		break;
	case K23:
		set_smode((oldstate == SHIFT_G)?SDISP_HEX:SDISP_DEC);
		process_cmdline_set_lift();
		break;

	case K04:
		switch (oldstate) {
		case SHIFT_F:	return OP_NIL | OP_P2R;
		case SHIFT_G:	return OP_NIL | OP_R2P;
		default:	break;
		}
		state.shifts = oldstate;
		break;

	case K05:
		if (oldstate == SHIFT_F || oldstate == SHIFT_G)
			return OP_NIL | OP_2FRAC;
		// falling through
	default:
		state.arrow = 1;
		state.shifts = oldstate;
		break;
	case K60:
	case K24:
		break;
	}
	return STATE_UNFINISHED;
}


/* Process a GTO . sequence
 */
static int gtodot_digit(const int n) {
	unsigned int dv = state.digval;
	const unsigned int val = dv * 10 + n;

	state.numdigit++;
	if (state.numdigit == 2) {		// two digits starting large
		if (val > NUMPROG / 10)
			return val;
	}
	if (state.numdigit == 3)
		return val;
	state.digval = val;
	return -1;
}

static int gtodot_fkey(int n) {
	const int code = 100 + n;

	unsigned int pc = find_label_from(1, code, 0);
	if (pc > 0)
		return pc;
	return state_pc();
}

static int process_gtodot(const keycode c) {
	int pc = -1;

	switch (c) {
	case K61:	pc = gtodot_digit(0);	break;
#ifdef SEQUENTIAL_ROWS
	case K51:	case K52:	case K53:
		pc = gtodot_digit(1 + (c - K51));
		break;
	case K41:	case K42:	case K43:
		pc = gtodot_digit(4 + (c - K41));
		break;
	case K31:	case K32:	case K33:
		pc = gtodot_digit(7 + (c - K31));
		break;
#else
	case K51:	pc = gtodot_digit(1);	break;
	case K52:	pc = gtodot_digit(2);	break;
	case K53:	pc = gtodot_digit(3);	break;
	case K41:	pc = gtodot_digit(4);	break;
	case K42:	pc = gtodot_digit(5);	break;
	case K43:	pc = gtodot_digit(6);	break;
	case K31:	pc = gtodot_digit(7);	break;
	case K32:	pc = gtodot_digit(8);	break;
	case K33:	pc = gtodot_digit(9);	break;
#endif

#ifdef SEQUENTIAL_ROWS
	case K02:	case K03:	case K04:
		pc = gtodot_fkey(c - K01);
		break;
#else
	case K02:	pc = gtodot_fkey(0);	break;
	case K03:	pc = gtodot_fkey(1);	break;
	case K04:	pc = gtodot_fkey(2);	break;
#endif

	case K62:		// .
		if (state.numdigit == 0)
			pc = 0;
		break;

	case K20:		// ENTER - short circuit processing
		pc = state.digval;
		break;

	case K24:		// backspace
		if (state.numdigit == 0) {
			pc = state_pc();
		} else {
			state.numdigit--;
			state.digval /= 10;
		}
	default:
		return STATE_UNFINISHED;
	}
	if (pc >= 0) {
		set_pc(pc);
		state.gtodot = 0;
		state.digval = 0;
		state.numdigit = 0;
	}
	return STATE_UNFINISHED;
}


/* Process a keystroke in alpha mode
 */
static int process_alpha(const keycode c) {
	const enum shifts oldstate = cur_shift();
	unsigned char ch;
#ifndef SEQUENTIAL_ROWS
	int idx;
#endif

	state.shifts = SHIFT_N;

	switch (c) {
#ifdef SEQUENTIAL_ROWS
	case K01:
	case K02:
	case K03:
		if (oldstate != SHIFT_F)
			break;
		ch = keycode_to_alpha(c, oldstate);
		if (ch == '\0')
			return STATE_UNFINISHED;
		return check_f_key(c - K01 + 1, RARG(RARG_ALPHA, ch));
#else
	case K03:	idx = 3;	goto fkey;
	case K02:	idx = 2;	goto fkey;
	case K01:	idx = 1;
fkey:		if (oldstate != SHIFT_F)
			break;
		ch = keycode_to_alpha(c, oldstate);
		if (ch == '\0')
			return STATE_UNFINISHED;
		return check_f_key(idx, RARG(RARG_ALPHA, ch));
#endif

	case K10:	// STO
		if (oldstate != SHIFT_F)
			break;
		init_arg(RARG_ASTO);
		return STATE_UNFINISHED;

	case K11:	// RCL - maybe view
		if (oldstate == SHIFT_F) {
			init_arg(RARG_ARCL);
			return STATE_UNFINISHED;
		} else if (oldstate == SHIFT_H)
			return OP_NIL | OP_VIEWALPHA;
		break;

	case K12:
		if (oldstate == SHIFT_F)
			init_cat(CATALOGUE_ALPHA_SUBSCRIPTS);
		else if (oldstate == SHIFT_H)
			init_cat(CATALOGUE_ALPHA_SUPERSCRIPTS);
		else
			break;
		return STATE_UNFINISHED;

	case K_ARROW:	// Alpha comparison characters
		if (oldstate == SHIFT_F) {
			init_cat(CATALOGUE_ALPHA_ARROWS);
			return STATE_UNFINISHED;
		}
		break;

	case K_CMPLX:	// Complex character menu
		if (oldstate == SHIFT_F) {
			init_cat(state.alphashift?CATALOGUE_ALPHA_LETTERS_LOWER:
						CATALOGUE_ALPHA_LETTERS_UPPER);
			return STATE_UNFINISHED;
		}
		break;

	case K20:	// Enter - maybe exit alpha mode
		if (oldstate == SHIFT_F)
			break;
		state.alpha = 0;
		state.alphashift = 0;
		return STATE_UNFINISHED;

	case K24:	// Clx - backspace, clear alpha
		if (oldstate == SHIFT_F)
			return OP_NIL | OP_CLRALPHA;
		if (oldstate == SHIFT_N)
			return STATE_BACKSPACE;
		break;

	case K30:
		if (oldstate == SHIFT_N)
			init_arg(RARG_XEQ);
		else if (oldstate == SHIFT_H)
			init_arg(RARG_GTO);
		else
			break;
		return STATE_UNFINISHED;

	case K40:
		if (oldstate == SHIFT_H) {
			init_cat(CATALOGUE_ALPHA_STATS);
			return STATE_UNFINISHED;
		}
		break;

	case K44:
		if (oldstate == SHIFT_H) {
			state.status = 1;
			return STATE_UNFINISHED;
		}
		break;


	case K50:	// Alpha command catalogue
		if (oldstate == SHIFT_H) {
			init_cat(CATALOGUE_ALPHA);
			return STATE_UNFINISHED;
		}
		break;

	case K51:	// Alpha comparison characters
		if (oldstate == SHIFT_F) {
			init_cat(CATALOGUE_ALPHA_COMPARES);
			return STATE_UNFINISHED;
		}
		break;

	case K60:	// EXIT/ON maybe case switch, otherwise exit alpha
		if (oldstate == SHIFT_F)
			state.alphashift = 1 - state.alphashift;
		else if (oldstate == SHIFT_H) {
			ch = ' ';
			goto gotc;
		} else
			init_state();
		return STATE_UNFINISHED;

	case K62:	// Alpha maths symbol characters
		if (oldstate == SHIFT_F) {
			init_cat(CATALOGUE_ALPHA_SYMBOLS);
			return STATE_UNFINISHED;
		}
		break;

	case K63:
		if (oldstate == SHIFT_F)
			return OP_NIL | OP_RS;		// R/S
		break;

	default:
		break;
	}

	/* Look up the character and return an alpha code if okay */
	ch = keycode_to_alpha(c, oldstate);
	if (ch == 0) {
		state.shifts = oldstate;
		return STATE_UNFINISHED;
	}
gotc:	return RARG(RARG_ALPHA, ch & 0xff);
}

/* Code to handle all commands with arguments */
static int arg_eval(unsigned int dv) {
	const int r = OP_RARG + ((state.base) << 8) +
			(state.ind?RARG_IND:0) + dv;
	init_arg(0);
	state.rarg = 0;
	return r;
}

static int arg_digit(int n) {
	const unsigned int base = state.base;
	const int mx = state.ind?NUMREG:argcmds[base].lim;
	const unsigned int val = state.digval * 10 + n;

	if (val == TOPREALREG-1 && argcmds[base].cmplx)
		return STATE_UNFINISHED;
	if (state.numdigit == 0) {
		if (n * 10 >= mx)
			return arg_eval(n);
	} else {
		if (val >= mx)
			return STATE_UNFINISHED;
		if (argcmds[base].notzero && val == 0)
			return STATE_UNFINISHED;
	}
	state.digval = val;
	state.numdigit++;
	if (state.numdigit == 2)
		return arg_eval(val);
	return STATE_UNFINISHED;
}

static int arg_fkey(int n) {
	const unsigned int b = state.base;

	if ((b >= RARG_LBL && b <= RARG_INTG) || (b >= RARG_SF && b <= RARG_FCF)) {
		if (state.ind || state.numdigit > 0)
			return STATE_UNFINISHED;
		if (argcmds[state.base].lim <= 100)
			return STATE_UNFINISHED;
		return arg_eval(n + 100);
	}
	return STATE_UNFINISHED;
}

static int arg_storcl(const unsigned int n, int cmplx) {
	unsigned int b = state.base;

	if (b == RARG_STO || b == RARG_RCL ||
			(cmplx && (b == RARG_CSTO || b == RARG_CRCL))) {
		state.base += n;
		return 1;
	}
	/* And we can turn off the operation too */
	if (b >= n) {
		b -= n;
		if (b == RARG_STO || b == RARG_RCL ||
				(cmplx && (b == RARG_CSTO || b == RARG_CRCL))) {
			state.base = b;
			return 1;
		}
	}
	return 0;
}

static int process_arg(const keycode c) {
	unsigned int base = state.base;

	if (base >= num_argcmds) {
		init_arg(0);
		state.rarg = 0;
		return STATE_UNFINISHED;
	}
	switch (c) {
	case K61:	return arg_digit(0);
#ifdef SEQUENTIAL_ROWS
	case K51:	case K52:	case K53:
		return arg_digit(1 + (c - K51));
	case K41:	case K42:	case K43:
		return arg_digit(4 + (c - K41));
	case K31:	case K32:	case K33:
		return arg_digit(7 + (c - K31));
#else
	case K51:	return arg_digit(1);
	case K52:	return arg_digit(2);
	case K53:	return arg_digit(3);
	case K41:	return arg_digit(4);
	case K42:	return arg_digit(5);
	case K43:	return arg_digit(6);
	case K31:	return arg_digit(7);
	case K32:	return arg_digit(8);
	case K33:	return arg_digit(9);
#endif

	case K_ARROW:		// arrow
		if (!state.dot && argcmds[base].indirectokay) {
			state.ind = 1 - state.ind;
			if (state.ind == 0 && !argcmds[base].stckreg)
				state.dot = 0;
		}
		break;

	case K_CMPLX:
	case K12:	  // I (lastY)
		if (state.dot || argcmds[base].stckreg || state.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regI_idx);
		break;

	case K23:	  // L (lastX)
		if (state.dot || argcmds[base].stckreg || state.ind)
			return arg_eval(regL_idx);
		break;
	case K21:	// J
		if (state.dot || argcmds[base].stckreg || state.ind)
			return arg_eval(regJ_idx);
		break;
	case K22:	// K
		if (state.dot || argcmds[base].stckreg || state.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regK_idx);
		break;
	case K62:		// X
		if (state.dot || argcmds[base].stckreg || state.ind)
			return arg_eval(regX_idx);
		if (base == RARG_GTO && state.numdigit == 0 && ! state.ind) {
			// Special GTO . sequence
			init_arg(0);
			state.rarg = 0;
			state.gtodot = 1;
		}
		break;
	case K63:		// Y
		if (state.dot || argcmds[base].stckreg || state.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regY_idx);
		break;

	/* STO and RCL can take an arithmetic argument */
	case K64:		// Z register
		if (state.dot || ( ! arg_storcl(RARG_STO_PL - RARG_STO, 1) &&
					(argcmds[base].stckreg || state.ind)))
			return arg_eval(regZ_idx);
		break;
	case K54:
		arg_storcl(RARG_STO_MI - RARG_STO, 1);
		break;

	case K44:		// T register
		if (state.dot || ( ! arg_storcl(RARG_STO_MU - RARG_STO, 1) &&
					(argcmds[base].stckreg || state.ind)))
			if (!argcmds[base].cmplx)
				return arg_eval(regT_idx);
		break;
	case K34:
		arg_storcl(RARG_STO_DV - RARG_STO, 1);
		break;
	case K40:
		arg_storcl(RARG_STO_MAX - RARG_STO, 0);
		break;
	case K50:
		arg_storcl(RARG_STO_MIN - RARG_STO, 0);
		break;

	case K00:
		if (state.dot || argcmds[base].stckreg || state.ind)
			return arg_eval(regA_idx);
		break;
	case K02:
		if (state.dot || argcmds[base].stckreg || state.ind)
			return arg_eval(regC_idx);
		return arg_fkey(1);	// F2
#ifdef SEQUENTIAL_ROWS
	case K01:	case K03:
		if (state.dot || argcmds[base].stckreg || state.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regB_idx + c - K01);
		return arg_fkey(c - K01);
#else
	case K01:
		if (state.dot || argcmds[base].stckreg || state.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regB_idx);
		return arg_fkey(0);	// F1
	case K03:
		if (state.dot || argcmds[base].stckreg || state.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regD_idx);
		return arg_fkey(2);	// F3
#endif

	case K20:				// Enter is a short cut finisher
		if (state.numdigit == 0 && !state.ind && !state.dot) {
			if (base >= RARG_LBL && base <= RARG_INTG) {
				init_arg(base - RARG_LBL);
				state.multi = 1;
				state.rarg = 0;
			} else if (argcmds[base].stckreg)
				state.dot = 1;
		} else if (state.numdigit > 0)
			return arg_eval(state.digval);
		else if (argcmds[base].stckreg || state.ind)
			state.dot = 1 - state.dot;
		break;

	case K60:
	case K24:
		init_arg(0);
		state.rarg = 0;
		break;

	default:
		break;
	}
	return STATE_UNFINISHED;
}


/* Multi (2) word instruction entry
 */
static int process_multi(const keycode c) {
	const enum shifts oldstate = cur_shift();
	unsigned char ch;

	state.shifts = SHIFT_N;

	switch (c) {
	case K20:	// Enter - exit multi mode, maybe return a result
		if (oldstate == SHIFT_F)
			break;
		state.multi = 0;
		if (state.numdigit == 0) {
			return STATE_UNFINISHED;
		} else if (state.numdigit == 1) {
			return OP_DBL + (state.base << DBL_SHIFT) + state.digval;
		} else {
			return OP_DBL + (state.base << DBL_SHIFT) + state.digval +
				(state.digval2 << 16);
		}

	case K24:	// Clx - backspace, clear alpha
		if (oldstate == SHIFT_N || oldstate == SHIFT_F) {
			if (state.numdigit == 0)
				state.multi = 0;
			else
				state.numdigit--;
			return STATE_UNFINISHED;
		}
		break;

	case K60:	// EXIT/ON maybe case switch, otherwise exit alpha
		if (oldstate == SHIFT_F)
			state.alphashift = 1 - state.alphashift;
		else if (oldstate == SHIFT_H) {
			ch = ' ';
			goto gotc;
		} else
			init_state();
		return STATE_UNFINISHED;

	default:
		break;
	}

	/* Look up the character and return an alpha code if okay */
	ch = keycode_to_alpha(c, oldstate);
	if (ch == 0) {
		state.shifts = oldstate;
		return STATE_UNFINISHED;
	}
gotc:	if (state.numdigit == 0) {
		state.digval = ch;
		state.numdigit = 1;
		return STATE_UNFINISHED;
	} else if (state.numdigit == 1) {
		state.digval2 = ch;
		state.numdigit = 2;
		return STATE_UNFINISHED;
	}
	state.multi = 0;
	return OP_DBL + (state.base << DBL_SHIFT) + state.digval +
			(state.digval2 << 16) + (ch << 24);
}


static int process_test(const keycode c) {
	int r = state.test;
	int cmpx = state.cmplx;

	state.test = TST_NONE;
	state.cmplx = 0;
	switch (c) {
	case K_CMPLX:
	case K12:
		if (cmpx)
			break;
		return RARG(RARG_TEST_EQ+r, regI_idx);
	case K23:
		return RARG((cmpx?RARG_TEST_ZEQ:RARG_TEST_EQ)+r, regL_idx);
	case K44:
		if (cmpx)
			break;
		return RARG(RARG_TEST_EQ+r, regT_idx);
	case K63:
		if (cmpx)
			break;
		return RARG(RARG_TEST_EQ+r, regY_idx);
	case K62:
		return RARG((cmpx?RARG_TEST_ZEQ:RARG_TEST_EQ)+r, regX_idx);
	case K64:
		return RARG((cmpx?RARG_TEST_ZEQ:RARG_TEST_EQ)+r, regZ_idx);
	case K00:
		return RARG((cmpx?RARG_TEST_ZEQ:RARG_TEST_EQ)+r, regA_idx);
	case K01:
		if (cmpx)
			break;
		return RARG(RARG_TEST_EQ+r, regB_idx);
	case K02:
		return RARG((cmpx?RARG_TEST_ZEQ:RARG_TEST_EQ)+r, regC_idx);
	case K03:
		if (cmpx)
			break;
		return RARG(RARG_TEST_EQ+r, regD_idx);

	case K31:	case K32:	case K33:	// 7 8 9
	case K41:	case K42:	case K43:	// 4 5 6
			case K52:	case K53:	//   2 3
		init_arg((cmpx?RARG_TEST_ZEQ:RARG_TEST_EQ) + r);
		return process_arg(c);

	case K61:	return OP_SPEC + (cmpx?OP_Zeq0:OP_Xeq0) + r;	// tests vs 0
	case K51:	return OP_SPEC + (cmpx?OP_Zeq1:OP_Xeq1) + r;	// tests vs 1

	case K11:					// tests vs register
	case K20:
		init_arg((cmpx?RARG_TEST_ZEQ:RARG_TEST_EQ) + r);
		return STATE_UNFINISHED;

	case K60:
	case K24:
		return STATE_UNFINISHED;
	default:
		break;
	}
	state.test = r;
	state.cmplx = cmpx;
	return STATE_UNFINISHED;
}


/* Return the number of entries in the current catalogue.
 * These are all fixed size known at compile time so a table lookup will
 * likely be the most space efficient method.
 */
int current_catalogue_max(void) {
	// A quick table of catalogue sizes
	// NB: the order here MUST match that in `enum catalogues' 
	static const unsigned char catalogue_sizes[] = 
	{
		0, // NONE
		sizeof(catalogue) / sizeof(const s_opcode),
		sizeof(cplx_catalogue) / sizeof(const s_opcode),
		sizeof(stats_catalogue) / sizeof(const s_opcode),
		sizeof(prob_catalogue) / sizeof(const s_opcode),
		sizeof(int_catalogue) / sizeof(const s_opcode),
		sizeof(prog_catalogue) / sizeof(const s_opcode),
		sizeof(test_catalogue) / sizeof(const s_opcode),
		sizeof(mode_catalogue) / sizeof(const s_opcode),
		sizeof(alpha_catalogue) / sizeof(const s_opcode),
		sizeof(alpha_symbols),
		sizeof(alpha_compares),
		sizeof(alpha_arrows),
		sizeof(alpha_stats),
		sizeof(alpha_letters_upper),
		sizeof(alpha_letters_lower),
		sizeof(alpha_superscripts),
		sizeof(alpha_subscripts),
		NUM_CONSTS,
		NUM_CONSTS,
		sizeof(conv_catalogue) / sizeof(const s_opcode),
	};
	return catalogue_sizes[state.catalogue];
}


/* Look up the character position in the given byte array and
 * build the alpha op-code for it.
 */
static opcode alpha_code(int n, const char tbl[]) {
	return RARG(RARG_ALPHA, tbl[n] & 0xff);
}


/* Return the opcode for entry n from the current catalogue
 */
opcode current_catalogue(int n) {
	switch (state.catalogue) {
	default:
		return OP_NIL | OP_NOP;

	case CATALOGUE_CONST:
		return CONST(n);

	case CATALOGUE_COMPLEX_CONST:
		return CONST_CMPLX(n);

	case CATALOGUE_INT:
		return int_catalogue[n];

	case CATALOGUE_ALPHA:
		return alpha_catalogue[n];

	/* Alpha character menus are all similar */
	case CATALOGUE_ALPHA_SYMBOLS:
		return alpha_code(n, alpha_symbols);
	case CATALOGUE_ALPHA_COMPARES:
		return alpha_code(n, alpha_compares);
	case CATALOGUE_ALPHA_ARROWS:
		return alpha_code(n, alpha_arrows);
	case CATALOGUE_ALPHA_STATS:
		return alpha_code(n, alpha_stats);
	case CATALOGUE_ALPHA_LETTERS_UPPER:
		return alpha_code(n, alpha_letters_upper);
	case CATALOGUE_ALPHA_LETTERS_LOWER:
		return alpha_code(n, alpha_letters_lower);
	case CATALOGUE_ALPHA_SUPERSCRIPTS:
		return alpha_code(n, alpha_superscripts);
	case CATALOGUE_ALPHA_SUBSCRIPTS:
		return alpha_code(n, alpha_subscripts);

	case CATALOGUE_COMPLEX:
		return cplx_catalogue[n];

	case CATALOGUE_STATS:
		return stats_catalogue[n];

	case CATALOGUE_PROB:
		return prob_catalogue[n];

	case CATALOGUE_PROG:
		return prog_catalogue[n];

	case CATALOGUE_MODE:
		return mode_catalogue[n];

	case CATALOGUE_TEST:
		return test_catalogue[n];

	case CATALOGUE_NORMAL:
		return catalogue[n];

	case CATALOGUE_CONV:
		return conv_catalogue[n];
	}
}


static int process_catalogue(const keycode c) {
	unsigned int dv;
	int last = -1;
	unsigned char ch;
	const int ctmax = current_catalogue_max();

	if (cur_shift() == SHIFT_N) {
		switch (c) {
		case K30:			// XEQ accepts command
		case K20:			// Enter accepts command
			dv = state.digval;
			if (dv < ctmax) {
				const opcode op = current_catalogue(dv);

				init_cat(CATALOGUE_NONE);

				if (op & OP_RARG) {
					const unsigned int rarg = (op & ~OP_RARG) >> RARG_OPSHFT;

					if (rarg == RARG_CONST || rarg == RARG_CONST_CMPLX || rarg == RARG_CONV || rarg == RARG_ALPHA)
						return op;
					if (rarg >= RARG_TEST_EQ && rarg <= RARG_TEST_GE) {
						state.test = TST_EQ + (rarg - RARG_TEST_EQ);
						return STATE_UNFINISHED;
					}
					init_arg(rarg);
				} else {
					if (op == (OP_NIL | OP_CLALL))
						init_confirm(confirm_clall);
					else if (op == (OP_NIL | OP_RESET))
						init_confirm(confirm_reset);
					else
						return op;
				}
			}
			init_cat(CATALOGUE_NONE);
			return STATE_UNFINISHED;

		case K24:
		case K60:
			init_cat(CATALOGUE_NONE);
			return STATE_UNFINISHED;

		case K40:
			if (state.digval > 0)
				state.digval--;
			else
				state.digval = ctmax-1;
			return STATE_UNFINISHED;

		case K50:
			if (++state.digval >= ctmax)
				state.digval = 0;
			return STATE_UNFINISHED;

		default:
			break;
		}
	}

	/* We've got a key press, map it to a character and try to
	 * jump to the appropriate catalogue entry.
	 */
	ch = remap_chars(keycode_to_alpha(c, cur_shift()));
	state.shifts = SHIFT_N;
	if (ch == '\0')
		return STATE_UNFINISHED;
	for (dv = 0; dv < ctmax; dv++) {
		char buf[16];
		const char *cmd = catcmd(current_catalogue(dv), buf);
		unsigned char c2 = *cmd;

		if (c2 == COMPLEX_PREFIX || (c2 == 0240 && state.catalogue == CATALOGUE_ALPHA))
			c2 = cmd[1];
		c2 = remap_chars(c2);
		last = -1;
		if (ch <= c2) {
			state.digval = dv;
			return STATE_UNFINISHED;
		}
	}
	if (last >= 0)
		state.digval = last;

	return STATE_UNFINISHED;
}


static int process_confirm(const keycode c) {
	switch (c) {
	case K63:			// Yes
		switch (state.confirm) {
		case confirm_clall:	clrall(NULL, NULL, NULL);	break;
		case confirm_reset:	reset(NULL, NULL, NULL);	break;
		case confirm_clprog:	clrprog();
		}
		state.confirm = confirm_none;
		state.digval = 0;
		break;

	//case K60:
	case K24:
	case K32:			// No
		state.confirm = 0;
		state.digval = 0;
		break;
	default:			// No state change
		break;
	}
	return STATE_UNFINISHED;
}

static int process_status(const keycode c) {
	int n = state.status - 1;

	switch (c) {
	case K61:	n = 0;	break;
#ifdef SEQUENTIAL_ROWS
	case K51:	case K52:	case K53:
		n = c - K51 + 1;
		break;
	case K41:	case K42:	case K43:
		n = c - K41 + 4;
		break;
	case K31:	case K32:	case K33:
		n = c - K31 + 7;
		break;
#else
	case K51:	n = 1;	break;
	case K52:	n = 2;	break;
	case K53:	n = 3;	break;
	case K41:	n = 4;	break;
	case K42:	n = 5;	break;
	case K43:	n = 6;	break;
	case K31:	n = 7;	break;
	case K32:	n = 8;	break;
	case K33:	n = 9;	break;
#endif
	case K40:
		if ((n -= 1) < 0)
			n = 0;
		break;
	case K50:
		if ((n += 1) > 9)
			n = 9;
		break;
	default:
		n = -1; 
	}
	state.status = n+1;
	return STATE_UNFINISHED;
}

static int process(const int c) {
	const enum shifts s = cur_shift();

	/* Check for ON in the unshifted state -- this is a reset sequence
	 * common across all modes.  Shifted modes need to check this themselves
	 * if required.
	 */
	if (c == K60 && s == SHIFT_N) {
		init_state();
		return STATE_UNFINISHED;
	}

	if (state.status)
		return process_status((const keycode)c);

	if (state.confirm)
		return process_confirm((const keycode)c);

	if (state.rarg)
		return process_arg((const keycode)c);

	if (state.gtodot)
		return process_gtodot((const keycode)c);

	if (state.hyp)
		return process_hyp((const keycode)c);

	if (state.test != TST_NONE)
		return process_test((const keycode)c);

	// Process shift keys directly
	if (c == K_F) {
		state.shifts = (s == SHIFT_F)?SHIFT_N:SHIFT_F;
		return STATE_UNFINISHED;
	}
	if (c == K_G) {
		state.shifts = (s == SHIFT_G)?SHIFT_N:SHIFT_G;
		return STATE_UNFINISHED;
	}
	if (c == K_H) {
		state.shifts = (s == SHIFT_H)?SHIFT_N:SHIFT_H;
		return STATE_UNFINISHED;
	}

	if (state.catalogue)
		return process_catalogue((const keycode)c);

	if (state.multi)
		return process_multi((const keycode)c);

	if (state.alpha)
		return process_alpha((const keycode)c);

	if (state.arrow)
		return process_arrow((const keycode)c);

	if (state.cmplx) {
		if (s == SHIFT_F)
			return process_f_shifted_cmplex((const keycode)c);
		if (s == SHIFT_G)
			return process_g_shifted_cmplx((const keycode)c);
		if (s == SHIFT_H)
			return process_h_shifted_cmplx((const keycode)c);
		return process_normal_cmplx((const keycode)c);
	} else {
		if (s == SHIFT_F)
			return process_f_shifted((const keycode)c);
		if (s == SHIFT_G)
			return process_g_shifted((const keycode)c);
		if (s == SHIFT_H)
			return process_h_shifted((const keycode)c);
		return process_normal((const keycode)c);
	}
}

void process_keycode(int c) {
	char tracebuf[25];
	decContext ctx, ctx64;

	g_ctx = &ctx;
	g_ctx64 = &ctx64;
	xeq_init_contexts();
	c = process(c);
	switch (c) {
	case STATE_SST:
		reset_volatile_state();
		if (state.runmode)
			xeqone(tracebuf);
		else
			incpc();
		break;

	case STATE_BACKSPACE:
		if (! state.runmode)
			delprog();
		else if (state.alpha) {
			char *p = find_char(alpha, '\0');
			if (p > alpha)
				*--p = '\0';
		}
		break;

	default:
		if (state.runmode) {
			xeq(c);
			xeqprog();
		} else
			stoprog(c);
	case STATE_UNFINISHED:
		break;
	}
	display();
}

void init_34s(void) {
	xeq_init();
#ifdef REALBUILD
	display();
#endif
}

#ifndef REALBUILD
static int remap(const int c) {
	switch (c) {
	case 'F':	return K_F;
	case 'G':	return K_G;
	case 'H':	return K_H;

	case 'q':	return K00;
	case 'w':	return K01;
	case 'B':	return K01;
	case 'e':	return K02;
	case 'C':	return K02;
	case 'r':	return K03;
	case 'D':	return K03;
	case 't':	return K04;
	case 'y':	return K05;

	case 'a':	return K10;
	case 's':	return K11;
	case 'd':	return K12;
	case 'f':	return K_F;
	case 'g':	return K_G;
	case 'h':	return K_H;

	case 'z':	return K20;
	case 'x':	return K20;
	case '\r':	return K20;
	case '\n':	return K20;
	case 'c':	return K21;
	case 'v':	return K22;
	case 'b':	return K23;
	case 'n':	return K24;	// Backspace
	case 127:	return K24;
	case 8:		return K24;

	case 'u':	return K30;
	case '7':	return K31;
	case '8':	return K32;
	case '9':	return K33;
	case '/':	return K34;

	case 'j':	return K40;
	case '4':	return K41;
	case '5':	return K42;
	case '6':	return K43;
	case '*':	return K44;

	case 'm':	return K50;
	case '1':	return K51;
	case '2':	return K52;
	case '3':	return K53;
	case '-':	return K54;

	case ' ':	return K60;	// ON
	case '0':	return K61;
	case '.':	return K62;
	case ',':	return K63;
	case '+':	return K64;
	}
	return K_UNKNOWN;
}

#include <stdio.h>

/* Mappings from our internal character codes to readable strings.
 * The first table is for characters below space and the second for those
 * >=127 (del).
 */
static const char *map32[32] = {
	NULL,	"x-bar", "y-bar", "sqrt", "integral", "degree", "space", "grad",
	"+/-", "<=", ">=", "!=", "euro", "->", "<-", "v",
	"^", "f-shift", "g-shift", "h-shift", "cmplx", "-->", "<--", "<->",
	"sz", "x-hat", "y-hat", "sub-m", "times", NULL, "pound", "yen"
};

static const char *maptop[129] = {
	"del",
	"ALPHA", "BETA", "GAMMA", "DELTA", "EPSILON", "ZETA", "ETA", "THETA",
	"IOTA", "KAPPA", "LAMBDA", "MU", "NU", "XI", "OMICRON", "PI",
	"RHO", "SIGMA", "TAU", "UPSILON", "PHI", "CHI", "PSI", "OMEGA",
	"sub-B", "sub-mu", "^2", "sub-infinity", "^x", "^-1", "h-bar", "infinity",
	"alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta",
	"iota", "kappa", "lambda", "mu", "nu", "xi", "omicron", "pi",
	"rho", "sigma", "tau", "upsilon", "phi", "chi", "psi", "omega",
	"sub-0", "sub-1", "sub-2", "sub-c", "sub-e", "sub-n", "sub-p", "sub-u",
	"A-grave", "A-acute", "A-tilde", "A-umlaut", "A-dot", "C-acute", "C-hook", "C-cedilla",
	"E-grave", "E-acute", "E-filde", "E-trema", "I-grave", "I-acute", "I-tilde", "I-trema",
	"N-tilde", "O-grave", "O-acute", "O-tilde", "O-umlaut", "R-hook", "S-hook", "sub-A",
	"U-grave", "U-acute", "U-tilde", "U-umlaut", "U-dot", "Y-acute", "Y-trema", "Z-hook",
	"a-grave", "a-acute", "a-tilde", "a-umlaut", "a-dot", "c-acute", "c-hook", "c-cedilla",
	"e-grave", "e-acute", "e-tilde", "e-trema", "i-grave", "i-acute", "i-tilde", "i-trema",
	"n-tilde", "o-grave", "o-acute", "o-tilde", "o-umlaut", "r-hook", "s-hook", "sub-k",
	"u-grave", "u-acute", "u-tilde", "u-umlaut", "u-dot", "y-acute", "y-trema", "z-hook"
};

const char *pretty(unsigned char z) {
	if (z < 32)
		return map32[z & 0x1f];
	if (z >= 127)
		return maptop[z - 127];
	return NULL;
}

static void rarg_values(const opcode c, int l) {
	char bf[100];

	if (isRARG(c)) {
		const unsigned int cmd = (c & ~OP_RARG) >> RARG_OPSHFT;
		const unsigned int idx = c & 0x7f;

		if (cmd == RARG_CONV) {
			while (l-- > 0)
				putchar(' ');
			printf("%c %s", (idx & 1)?'*':'/', decimal64ToString(&CONSTANT_CONV(idx/2), bf));
		} else if (cmd == RARG_CONST || cmd == RARG_CONST_CMPLX) {
			while (l-- > 0)
				putchar(' ');
			printf("%s", decimal64ToString(&CONSTANT(c & 0x7f), bf));
		}
	}
}
	
static int dumpop(const opcode c, int pt) {
	char tracebuf[25];
	const char *s, *m;

	xset(tracebuf, '\0', sizeof(tracebuf));
	s = prt(c, tracebuf);
	if (s[0] != '?' || s[1] != '?' || s[2] != '?') {
		char t[100], *q = t;
		int l = 35;
		q += sprintf(t, "%04x  ", (unsigned int)c);
		while (*s != '\0') {
			const unsigned char z = 0xff & *s++;

			m = pretty(z);
			if (m == NULL) {
				*q++ = z;
				l--;
			} else {
				l -= slen(m) + 2;
				q += sprintf(q, "[%s]", m);
			}
		}
		*q++ = '\0';
		if (pt) {
			printf("%s", t);
			rarg_values(c, l);
			putchar('\n');
		}
		return 1;
	}
	return 0;
}


static void dump_menu(const char *name, const char *prefix, const enum catalogues cata) {
	int i;
	char cmd[16];
	const char *p;
	const char *buf;
	const char *m;
	const int oldcata = state.catalogue;
	int n;

	state.catalogue = cata;
	n = current_catalogue_max();
	printf("%s catalogue:\n", name);
	for (i=0; i<n; i++) {
		int l = 35 - slen(prefix);
		const opcode cati = current_catalogue(i);
		buf = catcmd(cati, cmd);

		printf("\t%d\t%s", i+1, prefix);
		for (p=buf; *p != '\0'; p++) {
			const unsigned char c = 0xff & *p;
			m = pretty(c);
			if (m == NULL) {
				printf("%c", c);
				l--;
			} else {
				printf("[%s]", m);
				l -= slen(m) + 2;
			}
		}
		rarg_values(cati, l);
		printf("\n");
	}
	printf("\n");
	state.catalogue = oldcata;
}


int main(int argc, char *argv[]) {
	int c, n = 0;
	if (argc > 1) {
		if (argc == 2) {
			dump_menu("float", "", CATALOGUE_NORMAL);
			dump_menu("complex", "[cmplx]", CATALOGUE_COMPLEX);
			dump_menu("statistics", "", CATALOGUE_STATS);
			dump_menu("probability", "", CATALOGUE_PROB);
			dump_menu("integer", "", CATALOGUE_INT);
			dump_menu("alpha", "", CATALOGUE_ALPHA);
			dump_menu("alpha special letters upper", "", CATALOGUE_ALPHA_LETTERS_UPPER);
			dump_menu("alpha special letters lower", "", CATALOGUE_ALPHA_LETTERS_LOWER);
			dump_menu("alpha superscripts", "", CATALOGUE_ALPHA_SUPERSCRIPTS);
			dump_menu("alpha subscripts", "", CATALOGUE_ALPHA_SUBSCRIPTS);
			dump_menu("alpha symbols", "", CATALOGUE_ALPHA_SYMBOLS);
			dump_menu("alpha compares", "", CATALOGUE_ALPHA_COMPARES);
			dump_menu("alpha arrows", "", CATALOGUE_ALPHA_ARROWS);
			dump_menu("alpha statistics", "", CATALOGUE_ALPHA_STATS);
			dump_menu("programming", "", CATALOGUE_PROG);
			dump_menu("modes", "", CATALOGUE_MODE);
			dump_menu("test", "", CATALOGUE_TEST);
			dump_menu("conversions", "", CATALOGUE_CONV);
			dump_menu("constants", "# ", CATALOGUE_CONST);
			dump_menu("complex constants", "[cmplx]# ", CATALOGUE_COMPLEX_CONST);
		}
		for (c=0; c<65536; c++) {
			if (isDBL(c) && (c & 0xff))	/* Don't show all multi-word instructions */
				continue;
			n += dumpop(c, argc > 2);
		}
		printf("total number of opcodes %d\n", n);
		printf("\tniladic commands %d\n", num_niladics);
		printf("\tmonadic commands %d\n", num_monfuncs);
		printf("\tdyadic commands %d\n", num_dyfuncs);
		printf("\ttriadic commands %d\n", num_trifuncs);
		printf("\targument commands %d\n", num_argcmds);
		printf("\tmultiword commands %d\n", num_multicmds);
		printf("\tspecial commands %d\n", SPECIAL_MAX);
		return 0;
	}

	init_34s();
	if (setuptty(0) == 0) {
		display();
		while ((c = GETCHAR()) != GETCHAR_ERR && c != CH_QUIT) {
#ifdef USECURSES
			if (c == CH_TRACE) {
				state.trace = 1 - state.trace;
				display();
			} else if (c == CH_FLAGS) {
				state.flags = 1 - state.flags;
				display();
			} else if (c == CH_REFRESH) {
				clear();
				display();
			} else
#endif
				process_keycode(remap(c));
		}
		setuptty(1);
	}
	return 0;
}
#endif

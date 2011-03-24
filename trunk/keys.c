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
#define STATE_BST		-4
#define STATE_RUNNING		-5
#define STATE_IGNORE		-6

/* Define this if the keycodes map rows sequentially */
#define SEQUENTIAL_ROWS

// MvC: changed keycodes for rows with just 5 keys to match HP SDK

typedef enum {
	K00 = 0,  K01 = 1,  K02 = 2,  K03 = 3,  K04 = 4,  K05 = 5,
	K10 = 6,  K11 = 7,  K12 = 8,
#define K13	9		/* Shift keys aren't in the enum since they are handled */
#define K14	10		/* Directly in the main key processing loop */
#define K15	11
	K20 = 12, K21 = 13, K22 = 14, K23 = 15, K24 = 16,
	K30 = 18, K31 = 19, K32 = 20, K33 = 21, K34 = 22,
	K40 = 24, K41 = 25, K42 = 26, K43 = 27, K44 = 28,
	K50 = 30, K51 = 31, K52 = 32, K53 = 33, K54 = 34,
	K60 = 36, K61 = 37, K62 = 38, K63 = 39, K64 = 40,
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
	return State.shifts;
}

void set_shift(enum shifts s) {
	State.shifts = s;
}


static void init_arg(const enum rarg base) {
	State.base = base;
	State.ind = 0;
	State.digval = 0;
	State.numdigit = 0;
	State.rarg = 1;
	State.dot = 0;
}

static void init_cat(enum catalogues cat) {
	if (cat == CATALOGUE_NONE)
		State.eol = 0;
	process_cmdline_set_lift();
	State.catalogue = cat;
	State.cmplx = (cat == CATALOGUE_COMPLEX || cat == CATALOGUE_COMPLEX_CONST)?1:0;
	State.digval = 0;
	set_shift(SHIFT_N);
}

static void init_state(void) {
	struct _state s;
	xset(&s, 0, sizeof(s));

#define C(n)	s.n = State.n
	C(runmode);
#ifndef REALBUILD
	C(trace);
#endif
	C(fraccomma);
	C(last_prog);
	C(sigma_mode);
	C(intm);
	C(int_len);
        C(t12);
        C(stack_depth);
	C(dispdigs);
	C(trigmode);
	C(dispmode);
	C(int_base);
	C(int_mode);
	C(date_mode);
	C(denom_mode);
        C(denom_max);
        C(improperfrac);
        C(nothousands);
        C(leadzero);
        C(fract);
	C(alphas);
#undef C
	s.shifts = SHIFT_N;
	s.test = TST_NONE;

        s.magic = MAGIC_MARKER;

	xcopy(&State, &s, sizeof(struct _state));
}

static void init_confirm(enum confirmations n) {
	State.confirm = n;
}

static void set_smode(const enum single_disp d) {
	State.smode = (State.smode == d)?SDISP_NORMAL:d;
}

/* Mapping from key position to alpha in the four key planes plus
 * the two lower case planes.
 * MvC: added fillers to adjust modified key codes
 */
static const unsigned char alphamap[41][6] = {
	/*upper f-sft g-sft h-sft lower g-shf lower */
        { 'A',  0221, 0200, 0000, 'a',  0240,  },  // K00
        { 'B',  'B',  0201, 0000, 'b',  0241,  },  // K01
        { 'C',  'C',  0202, 0000, 'c',  0242,  },  // K02
        { 'D',  0003, 0203, 0000, 'd',  0243,  },  // K03
        { 'E',  0015, 0204, 0000, 'e',  0244,  },  // K04
        { 'F',  0024, 0224, 0000, 'f',  0264,  },  // K05

        { 'G',  0000, 0202, 0020, 'g',  0242,  },  // K10
        { 'H',  0000, 0225, 0016, 'h',  0265,  },  // K11
        { 'I',  0000, 0210, 0000, 'i',  0250,  },  // K12
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },

        { 0000, 0000, 0206, 0000, 0000, 0246,  },  // K20
        { 'J',  0000, 0000, 0027, 'j',  0000,  },  // K21
        { 'K',  0010, 0211, '\\', 'k',  0251,  },  // K22
        { 'L',  0246, 0212, 0257, 'l',  0252,  },  // K23
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },

        { 0000, 0000, 0000, 0000, 0000, 0000,  },  // K30
        { 'M',  '7',  0213, '&',  'm',  0253,  },  // K31
        { 'N',  '8',  0214, '|',  'n',  0254,  },  // K32
        { 'O',  '9',  0227, 0013, 'o',  0267,  },  // K33
        { 'P',  '/',  0217, 0235, 'p',  0257,  },  // K34
        { 0, 0, 0, 0, 0, 0 },

        { 0000, 0000, 0000, '!',  0020, 0000,  },  // K40
        { 'Q',  '4',  0000, '?',  'q',  0000,  },  // K41
        { 'R',  '5',  0220, 0000, 'r',  0260,  },  // K42
        { 'S',  '6',  0221, '$',  's',  0261,  },  // K43
        { 'T',  0034, 0222, 0217, 't',  0262,  },  // K44
        { 0, 0, 0, 0, 0, 0 },

        { 0000, '(',  ')',  0000, 0000, ')',   },  // K50
        { '1',  '1',  0207, 0000, '1',  0247,  },  // K51
        { 'U',  '2',  0000, 0014, 'u',  0000,  },  // K52
        { 'V',  '3',  0000, 0036, 'v',  0000,  },  // K53
        { 'W',  '-',  0000, '%',  'w',  0000,  },  // K54
        { 0, 0, 0, 0, 0, 0 },

        { 0000, 0000, 0000, 0000, 0000, 0000,  },  // K60
        { '0',  '0',  0226, ' ',  '0',  0266,  },  // K61
        { 'X',  '.',  0215, 0000, 'x',  0255,  },  // K62
        { 'Y',  0000, 0223, 0037, 'y',  0263,  },  // K63
        { 'Z',  '+',  0205, '%',  'z',  0245,  },  // K64
};

static unsigned char keycode_to_alpha(const keycode c, unsigned int s) 
{
	if (State.alphashift) {
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
	return (is_intmode() && (int) int_base() > d);
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
		State.arrow = 1;
		break;
	case K_CMPLX:
		if (intltr(15))
			return OP_SPEC | OP_F;
		if (!State.intm)
			State.cmplx = 1;
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
		if (State.runmode)
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
		return STATE_BST;


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
	set_shift(SHIFT_N);
	switch (c) {
	case K00:
		if (intltr(10))
			return OP_SPEC | OP_SIGMAPLUS;
		State.hyp = 1;
		State.dot = 1;
		State.cmplx = 0;
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
			State.arrow = 1;
			break;
		}
		return OP_NIL | OP_P2R;
	case K_CMPLX:
		return OP_NIL | OP_FRACPROPER;

	case K10:
		if (State.runmode)
			State.hms = 1;
		process_cmdline_set_lift();
		break;
	case K11:	return OP_NIL | OP_FLOAT;
	case K12:	return OP_NIL | OP_RANDOM;

	case K20:
		State.alphas = 1;
		process_cmdline_set_lift();
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
		if (State.intm && State.int_maxw > State.int_window)
			State.int_window++;
		break;
	case K51:	State.test = TST_EQ;	break;
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
		//init_arg(State.intm?RARG_DSZ:RARG_DSE);
		init_arg(RARG_DSE);
		break;
	}
	return STATE_UNFINISHED;
}

static int process_g_shifted(const keycode c) {
	set_shift(SHIFT_N);
	switch (c) {
	case K00:
		State.hyp = 1;
		State.dot = 0;
		State.cmplx = 0;
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
		if (State.intm && State.int_maxw > 0 && State.int_window > 0)
			State.int_window--;
		break;
	case K51:	State.test = TST_NE;	break;
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
		//init_arg(State.intm?RARG_ISZ:RARG_ISG);
		init_arg(RARG_ISG);
		break;
	}
	return STATE_UNFINISHED;
}

static int process_h_shifted(const keycode c) {
	set_shift(SHIFT_N);
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
		if (! State.intm)
			init_cat(CATALOGUE_CONST);
		break;
	case K21:	init_arg(RARG_SWAP);	break;	// x<>
	case K22:	return OP_MON | OP_NOT;
	case K23:
		if (State.runmode)
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
	case K44:	State.status = 1;		break;

	case K50:
		init_cat(State.intm?CATALOGUE_INT:CATALOGUE_NORMAL);
		break;
	case K51:	init_cat(CATALOGUE_TEST);	break;
	case K52:	init_cat(CATALOGUE_PROG);	break;
	case K53:	return CONST(OP_PI);
	case K54:	return OP_DYA | OP_PERSB;

	case K60:	set_smode(SDISP_SHOW);		break;
	case K61:	return OP_NIL | OP_PAUSE;
	case K62:
		if (State.fraccomma)
			return OP_NIL | OP_RADDOT;
		return OP_NIL | OP_RADCOM;

	case K63:					// Program<->Run mode
		State.runmode = 1 - State.runmode;
		process_cmdline_set_lift();
		break;
	case K64:	return OP_DYA | OP_PERAD;
	}
	return STATE_UNFINISHED;
}


static int process_normal_cmplx(const keycode c) {
	State.cmplx = 0;
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
		break;
	}
	return STATE_UNFINISHED;
}

static int process_f_shifted_cmplex(const keycode c) {
	set_shift(SHIFT_N);
	State.cmplx = 0;
	switch (c) {
	case K00:
		State.hyp = 1;
		State.dot = 1;
		State.cmplx = 1;
		break;
	case K01:	return OP_CMON | OP_SIN;
	case K02:	return OP_CMON | OP_COS;
	case K03:	return OP_CMON | OP_TAN;
	case K04:	return OP_NIL | OP_P2R;
	case K_CMPLX:	set_shift(SHIFT_F);	break;

	case K30:	return OP_CMON | OP_EXP;
	case K31:	return OP_CMON | OP_10POWX;
	case K32:	return OP_CMON | OP_2POWX;
	case K33:	return OP_CDYA | OP_POW;
	case K34:	return OP_CMON | OP_RECIP;

	case K40:	return OP_CDYA | OP_COMB;
	case K44:	return OP_CMON | OP_SQRT;

	case K50:	init_cat(CATALOGUE_COMPLEX);	break;
	case K51:
		State.cmplx = 1;
		State.test = TST_EQ;
		break;

	case K60:
		init_state();
		break;

	case K61:	return OP_CMON | OP_ABS;
	case K62:	return OP_CMON | OP_TRUNC;

	case K10:	case K11:	case K12:
	case K20:	case K21:	case K22:	case K23:	case K24:
			case K41:	case K42:	case K43:
					case K52:	case K53:	case K54:
							case K63:	case K64:
		break;
	}
	return STATE_UNFINISHED;
}

static int process_g_shifted_cmplx(const keycode c) {
	set_shift(SHIFT_N);
	State.cmplx = 0;
	switch (c) {
	case K00:
		State.hyp = 1;
		State.dot = 0;
		State.cmplx = 1;
		break;
	case K01:	return OP_CMON | OP_ASIN;
	case K02:	return OP_CMON | OP_ACOS;
	case K03:	return OP_CMON | OP_ATAN;
	case K04:	return OP_NIL | OP_R2P;
	case K_CMPLX:	set_shift(SHIFT_G);	break;

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
		State.cmplx = 1;
		State.test = TST_NE;
		break;

	case K60:
		init_state();
		break;

	case K61:	return OP_CMON | OP_RND;
	case K62:	return OP_CMON | OP_FRAC;

	case K10:	case K11:	case K12:
			case K21:	case K22:	case K23:	case K24:
			case K41:	case K42:	case K43:
					case K52:	case K53:	case K54:
							case K63:	case K64:
		break;
	}
	return STATE_UNFINISHED;
}

static int process_h_shifted_cmplx(const keycode c) {
	set_shift(SHIFT_N);
	State.cmplx = 0;
	switch (c) {
	case K12:	return OP_NIL | OP_CRUP;

	case K20:	init_cat(CATALOGUE_COMPLEX_CONST);	break;
	case K21:	init_arg(RARG_CSWAP);	break;	// x<>
	case K22:	return OP_CMON | OP_CCONJ;

	case K40:	return OP_CMON | OP_FACT;	// z!

	case K50:	init_cat(CATALOGUE_COMPLEX);	break;
	case K52:	init_cat(CATALOGUE_PROG);	break;
	case K53:	return CONST_CMPLX(OP_PI);

	case K60:	break;


	case K05:
#ifdef INCLUDE_INTERNAL_CATALOGUE
		init_cat(CATALOGUE_INTERNAL);
#else
		set_shift(SHIFT_h);
#endif		
		break;

	case K00:	case K01:	case K02:	case K03:	case K04:
	case K10:	case K11:
							case K23:	case K24:
	case K30:	case K31:	case K32:	case K33:	case K34:
			case K41:	case K42:	case K43:	case K44:
			case K51:					case K54:
			case K61:	case K62:	case K63:	case K64:
		break;
	}
	return STATE_UNFINISHED;
}


/* Fairly simple routine for dealing with the HYP prefix.
 * This setting can only be followed by 4, 5, or 6 to specify
 * the function.  The inverse routines use the code too, the State.dot
 * is 1 for normal and 0 for inverse hyperbolic.  We also have to
 * deal with the complex versions and the handling of that key and
 * the ON key are dealt with by our caller.
 */
static int process_hyp(const keycode c) {
	const int cmplx = State.cmplx;
	const int dot = State.dot;
	const opcode x = cmplx?OP_CMON:OP_MON;

	State.hyp = 0;
	State.cmplx = 0;
	State.dot = 0;

	switch (c) {
	case K01:	return x + (dot?OP_SINH:OP_ASINH);
	case K02:	return x + (dot?OP_COSH:OP_ACOSH);
	case K03:	return x + (dot?OP_TANH:OP_ATANH);

	case K60:
	case K24:
		break;
	default:
		State.hyp = 1;
		State.cmplx = cmplx;
		State.dot = dot;
		break;
	}
	return STATE_UNFINISHED;
}


static int process_arrow(const keycode c) {
	const enum shifts oldstate = cur_shift();

	set_shift(SHIFT_N);
	State.arrow = 0;
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
			State.arrow_alpha = 1;
		break;

	case K22:
		set_smode((oldstate != SHIFT_F)?SDISP_OCT:SDISP_BIN);
		process_cmdline_set_lift();
		break;
	case K23:
		set_smode((oldstate != SHIFT_F)?SDISP_HEX:SDISP_DEC);
		process_cmdline_set_lift();
		break;

	case K04:
		switch (oldstate) {
		case SHIFT_F:	return OP_NIL | OP_P2R;
		case SHIFT_G:	return OP_NIL | OP_R2P;
		default:	break;
		}
		break;

//	case K05:
//		return OP_NIL | OP_2FRAC;

	default:
	case K60:
	case K24:
		break;
	}
	return STATE_UNFINISHED;
}


/* Process a GTO . sequence
 */
static int gtodot_digit(const int n) {
	unsigned int dv = State.digval;
	const unsigned int val = dv * 10 + n;

	State.numdigit++;
	if (State.numdigit == 2) {		// two digits starting large
		if (val > NUMPROG / 10)
			return val;
	}
	if (State.numdigit == 3)
		return val;
	State.digval = val;
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
		if (State.numdigit == 0)
			pc = 0;
		break;

	case K20:		// ENTER - short circuit processing
		pc = State.digval;
		break;

	case K24:		// backspace
		if (State.numdigit == 0) {
			pc = state_pc();
		} else {
			State.numdigit--;
			State.digval /= 10;
		}
	default:
		return STATE_UNFINISHED;
	}
	if (pc >= 0) {
		set_pc(pc);
		State.gtodot = 0;
		State.digval = 0;
		State.numdigit = 0;
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

	set_shift(SHIFT_N);

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
		if (oldstate == SHIFT_F)
			init_arg(RARG_ASTO);
		else if (oldstate == SHIFT_H)
			return OP_NIL | OP_VIEWALPHA;
		else
			break;
		return STATE_UNFINISHED;

	case K11:	// RCL - maybe view
		if (oldstate == SHIFT_F) {
			init_arg(RARG_ARCL);
			return STATE_UNFINISHED;
		} else if (oldstate == SHIFT_H)
			return OP_NIL | OP_TIME;
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
			init_cat(State.alphashift?CATALOGUE_ALPHA_LETTERS_LOWER:
						CATALOGUE_ALPHA_LETTERS_UPPER);
			return STATE_UNFINISHED;
		}
		break;

	case K20:	// Enter - maybe exit alpha mode
		if (oldstate == SHIFT_G)
			break;
		State.alphas = 0;
		State.alphashift = 0;
		return STATE_UNFINISHED;

	case K21:
		if (oldstate == SHIFT_F)
			return OP_NIL | OP_ALPHATOX;
		else if (oldstate == SHIFT_G)
			return OP_NIL | OP_XTOALPHA;
		break;

	case K24:	// Clx - backspace, clear Alpha
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

	case K44:
		if (oldstate == SHIFT_H) {
			State.status = 1;
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
		if (oldstate == SHIFT_H) {
			init_cat(CATALOGUE_ALPHA_COMPARES);
			return STATE_UNFINISHED;
		}
		break;

	case K60:	// EXIT/ON maybe case switch, otherwise exit alpha
		if (oldstate == SHIFT_F)
			State.alphashift = 1 - State.alphashift;
		else
			init_state();
		return STATE_UNFINISHED;

	case K62:	// Alpha maths symbol characters
		if (oldstate == SHIFT_H) {
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
	if (ch == 0)
		return STATE_UNFINISHED;
	return RARG(RARG_ALPHA, ch & 0xff);
}

/* Code to handle all commands with arguments */
static int arg_eval(unsigned int dv) {
	const int r = OP_RARG + ((State.base) << 8) +
			(State.ind?RARG_IND:0) + dv;
	init_arg(0);
	State.rarg = 0;
	return r;
}

static int arg_digit(int n) {
	const unsigned int base = State.base;
	const int mx = State.ind ? NUMREG : argcmds[base].lim;
	const unsigned int val = State.digval * 10 + n;

	if (val == TOPREALREG-1 && argcmds[base].cmplx)
		return STATE_UNFINISHED;
	if (State.numdigit == 0) {
		if (n * 10 >= mx)
			return arg_eval(n);
	} else {
		if ((int) val >= mx)
			return STATE_UNFINISHED;
		if (argcmds[base].notzero && val == 0)
			return STATE_UNFINISHED;
	}
	State.digval = val;
	State.numdigit++;
	if (State.numdigit == 2)
		return arg_eval(val);
	return STATE_UNFINISHED;
}

static int arg_fkey(int n) {
	const unsigned int b = State.base;

	if ((b >= RARG_LBL && b <= RARG_INTG) || (b >= RARG_SF && b <= RARG_FCF)) {
		if (State.ind || State.numdigit > 0)
			return STATE_UNFINISHED;
		if (argcmds[State.base].lim <= 100)
			return STATE_UNFINISHED;
		return arg_eval(n + 100);
	}
	return STATE_UNFINISHED;
}

static int arg_storcl(const unsigned int n, int cmplx) {
	unsigned int b = State.base;

	if (b == RARG_STO || b == RARG_RCL ||
			(cmplx && (b == RARG_CSTO || b == RARG_CRCL))) {
		State.base += n;
		return 1;
	}
	/* And we can turn off the operation too */
	if (b >= n) {
		b -= n;
		if (b == RARG_STO || b == RARG_RCL ||
				(cmplx && (b == RARG_CSTO || b == RARG_CRCL))) {
			State.base = b;
			return 1;
		}
	}
	return 0;
}

static int process_arg(const keycode c) {
	unsigned int base = State.base;

	if (base >= num_argcmds) {
		init_arg(0);
		State.rarg = 0;
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
		if (!State.dot && argcmds[base].indirectokay) {
			State.ind = 1 - State.ind;
			if (State.ind == 0 && !argcmds[base].stckreg)
				State.dot = 0;
		}
		break;

	case K_CMPLX:
	case K12:	  // I (lastY)
		if (State.dot || argcmds[base].stckreg || State.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regI_idx);
		break;

	case K23:	  // L (lastX)
		if (State.dot || argcmds[base].stckreg || State.ind)
			return arg_eval(regL_idx);
		break;
	case K21:	// J
		if (State.dot || argcmds[base].stckreg || State.ind)
			return arg_eval(regJ_idx);
		break;
	case K22:	// K
		if (State.dot || argcmds[base].stckreg || State.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regK_idx);
		break;
	case K62:		// X
		if (State.dot || argcmds[base].stckreg || State.ind)
			return arg_eval(regX_idx);
		if (base == RARG_GTO && State.numdigit == 0 && ! State.ind) {
			// Special GTO . sequence
			init_arg(0);
			State.rarg = 0;
			State.gtodot = 1;
		} else if (base == RARG_FIX) {
			init_arg(0);
			State.rarg = 0;
			return OP_NIL | OP_ALL;
		}
		break;
	case K63:		// Y
		if (State.dot || argcmds[base].stckreg || State.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regY_idx);
		break;

	/* STO and RCL can take an arithmetic argument */
	case K64:		// Z register
		if (State.dot || ( ! arg_storcl(RARG_STO_PL - RARG_STO, 1) &&
					(argcmds[base].stckreg || State.ind)))
			return arg_eval(regZ_idx);
		break;
	case K54:
		arg_storcl(RARG_STO_MI - RARG_STO, 1);
		break;

	case K44:		// T register
		if (State.dot || ( ! arg_storcl(RARG_STO_MU - RARG_STO, 1) &&
					(argcmds[base].stckreg || State.ind)))
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
		if (State.dot || argcmds[base].stckreg || State.ind)
			return arg_eval(regA_idx);
		break;
	case K02:
		if (State.dot || argcmds[base].stckreg || State.ind)
			return arg_eval(regC_idx);
		return arg_fkey(1);	// F2
#ifdef SEQUENTIAL_ROWS
	case K01:	case K03:
		if (State.dot || argcmds[base].stckreg || State.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regB_idx + c - K01);
		return arg_fkey(c - K01);
#else
	case K01:
		if (State.dot || argcmds[base].stckreg || State.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regB_idx);
		return arg_fkey(0);	// F1
	case K03:
		if (State.dot || argcmds[base].stckreg || State.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regD_idx);
		return arg_fkey(2);	// F3
#endif

	case K20:				// Enter is a short cut finisher
		if (State.numdigit == 0 && !State.ind && !State.dot) {
			if (base >= RARG_LBL && base <= RARG_INTG) {
				init_arg(base - RARG_LBL);
				State.multi = 1;
				State.rarg = 0;
			} else if (argcmds[base].stckreg)
				State.dot = 1;
		} else if (State.numdigit > 0)
			return arg_eval(State.digval);
		else if (argcmds[base].stckreg || State.ind)
			State.dot = 1 - State.dot;
		break;

	case K60:
	case K24:
		init_arg(0);
		State.rarg = 0;
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

	set_shift(SHIFT_N);

	switch (c) {
	case K20:	// Enter - exit multi mode, maybe return a result
		if (oldstate == SHIFT_F)
			break;
		State.multi = 0;
		if (State.numdigit == 0) {
			return STATE_UNFINISHED;
		} else if (State.numdigit == 1) {
			return OP_DBL + (State.base << DBL_SHIFT) + State.digval;
		} else {
			return OP_DBL + (State.base << DBL_SHIFT) + State.digval +
				(State.digval2 << 16);
		}

	case K24:	// Clx - backspace, clear alpha
		if (oldstate == SHIFT_N || oldstate == SHIFT_F) {
			if (State.numdigit == 0)
				State.multi = 0;
			else
				State.numdigit--;
			return STATE_UNFINISHED;
		}
		break;

	case K60:	// EXIT/ON maybe case switch, otherwise exit alpha
		if (oldstate == SHIFT_F)
			State.alphashift = 1 - State.alphashift;
		else
			init_state();
		return STATE_UNFINISHED;

	default:
		break;
	}

	/* Look up the character and return an alpha code if okay */
	ch = keycode_to_alpha(c, oldstate);
	if (ch == 0)
		return STATE_UNFINISHED;
	if (State.numdigit == 0) {
		State.digval = ch;
		State.numdigit = 1;
		return STATE_UNFINISHED;
	} else if (State.numdigit == 1) {
		State.digval2 = ch;
		State.numdigit = 2;
		return STATE_UNFINISHED;
	}
	State.multi = 0;
	return OP_DBL + (State.base << DBL_SHIFT) + State.digval +
			(State.digval2 << 16) + (ch << 24);
}


static int process_test(const keycode c) {
	int r = State.test;
	int cmpx = State.cmplx;

	State.test = TST_NONE;
	State.cmplx = 0;
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
	State.test = r;
	State.cmplx = cmpx;
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
		sizeof(alpha_letters_upper),
		sizeof(alpha_letters_lower),
		sizeof(alpha_superscripts),
		sizeof(alpha_subscripts),
		NUM_CONSTS,
		NUM_CONSTS,
		sizeof(conv_catalogue) / sizeof(const s_opcode),
#ifdef INCLUDE_INTERNAL_CATALOGUE
		sizeof(internal_catalogue) / sizeof(const s_opcode),
#endif
	};
	return catalogue_sizes[State.catalogue];
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
	switch (State.catalogue) {
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

#ifdef INCLUDE_INTERNAL_CATALOGUE
	case CATALOGUE_INTERNAL:
		return internal_catalogue[n];
#endif

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
	unsigned char ch;
	const int ctmax = current_catalogue_max();

	if (cur_shift() == SHIFT_N) {
		switch (c) {
		case K30:			// XEQ accepts command
		case K20:			// Enter accepts command
			dv = State.digval;
			if ((int) dv < ctmax) {
				const opcode op = current_catalogue(dv);

				init_cat(CATALOGUE_NONE);

				if (op & OP_RARG) {
					const unsigned int rarg = (op & ~OP_RARG) >> RARG_OPSHFT;

					if (rarg == RARG_CONST || rarg == RARG_CONST_CMPLX || rarg == RARG_CONV || rarg == RARG_ALPHA)
						return op;
					if (rarg >= RARG_TEST_EQ && rarg <= RARG_TEST_GE) {
						State.test = TST_EQ + (rarg - RARG_TEST_EQ);
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

		case K24:			// backspace
			if (State.eol > 0) {
				if (--State.eol > 0)
					goto search;
				State.digval = 0;
			} else
				init_cat(CATALOGUE_NONE);
			return STATE_UNFINISHED;

		case K60:
			init_cat(CATALOGUE_NONE);
			return STATE_UNFINISHED;

		case K40:
			if (State.digval > 0)
				State.digval--;
			else
				State.digval = ctmax-1;
			State.eol = 0;
			return STATE_UNFINISHED;

		case K50:
			if ((int) ++State.digval >= ctmax)
				State.digval = 0;
			State.eol = 0;
			return STATE_UNFINISHED;

		default:
			break;
		}
	}
	/* We've got a key press, map it to a character and try to
	 * jump to the appropriate catalogue entry.
	 */
	ch = remap_chars(keycode_to_alpha(c, cur_shift()));
	set_shift(SHIFT_N);
	if (ch == '\0')
		return STATE_UNFINISHED;
	if (State.eol < 10)
		Cmdline[State.eol++] = ch;

	/* Search for the current buffer in the catalogue */
search:
	Cmdline[State.eol] = '\0';
	for (dv = 0; dv < (unsigned int)ctmax; dv++) {
		char buf[16];
		const char *cmd = catcmd(current_catalogue(dv), buf);
		int i;

		if (*cmd == COMPLEX_PREFIX || (*cmd == (char)0240 && State.catalogue == CATALOGUE_ALPHA))
			cmd++;
		for (i=0; cmd[i] != '\0'; i++) {
			const unsigned char c = remap_chars(cmd[i]);
			const unsigned char cl = 0xff&Cmdline[i];
			if (c > cl) {
				State.digval = dv;
				return STATE_UNFINISHED;
			} else if (c < cl)
				break;
		}
		if (cmd[i] == '\0') {
			State.digval = dv;
			return STATE_UNFINISHED;
		}
	}
	State.digval = ctmax-1;
	return STATE_UNFINISHED;
}


static int process_confirm(const keycode c) {
	switch (c) {
	case K63:			// Yes
		switch (State.confirm) {
		case confirm_clall:	clrall(NULL, NULL, NULL);	break;
		case confirm_reset:	reset(NULL, NULL, NULL);	break;
		case confirm_clprog:	clrprog();
		}
		State.confirm = confirm_none;
		State.digval = 0;
		break;

	//case K60:
	case K24:
	case K32:			// No
		State.confirm = 0;
		State.digval = 0;
		break;
	default:			// No state change
		break;
	}
	return STATE_UNFINISHED;
}

static int process_status(const keycode c) {
	int n = State.status - 1;

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
	State.status = n+1;
	return STATE_UNFINISHED;
}

static int process(const int c) {
	const enum shifts s = cur_shift();

	if (c == K_HEARTBEAT) {
		/*
		 *  Heartbeat processing goes here.
		 *  This is totally thread safe!
		 *  For example, the Ticker value could be converted to a register.
		 */

		/*
		 * Toggle the RPN annunciator as a visual feedback
		 */
		dot(RPN, !State.busy_blink);
		State.busy_blink = 0;
		finish_display();		

		/*
		 *  Do nothing if not running a program
		 */
		if (!running())
			return STATE_IGNORE;
	}

	if (running()) {
		/*
		 *  Abort a running program with R/S
		 */
		if (c == K63) {
			set_running_off();
			return STATE_UNFINISHED;
		}
		return STATE_RUNNING;		// continue execution
	}

	/*
	 * Turn off the RPN annunciator as a visual feedback
	 */
	State.busy_blink = 1;
	dot(RPN, 0);
	finish_display();

	/* Check for ON in the unshifted state -- this is a reset sequence
	 * common across all modes.  Shifted modes need to check this themselves
	 * if required.
	 */
	if (c == K60 && s == SHIFT_N) {
		init_state();
		return STATE_UNFINISHED;
	}

	if (State.status)
		return process_status((const keycode)c);

	if (State.confirm)
		return process_confirm((const keycode)c);

	if (State.rarg)
		return process_arg((const keycode)c);

	if (State.gtodot)
		return process_gtodot((const keycode)c);

	if (State.hyp)
		return process_hyp((const keycode)c);

	if (State.test != TST_NONE)
		return process_test((const keycode)c);

	// Process shift keys directly
	if (c == K_F) {
		set_shift((s == SHIFT_F)?SHIFT_N:SHIFT_F);
		return STATE_UNFINISHED;
	}
	if (c == K_G) {
		set_shift((s == SHIFT_G)?SHIFT_N:SHIFT_G);
		return STATE_UNFINISHED;
	}
	if (c == K_H) {
		set_shift((s == SHIFT_H)?SHIFT_N:SHIFT_H);
		return STATE_UNFINISHED;
	}

	if (State.catalogue)
		return process_catalogue((const keycode)c);

	if (State.multi)
		return process_multi((const keycode)c);

	if (State.alphas)
		return process_alpha((const keycode)c);

	if (State.arrow)
		return process_arrow((const keycode)c);

	if (State.cmplx) {
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

	Ctx = &ctx;
	Ctx64 = &ctx64;
	xeq_init_contexts();
	c = process(c);
	switch (c) {
	case STATE_SST:
		xeq_sst(tracebuf);
		break;

	case STATE_BST:
		xeq_bst(tracebuf);
		break;

	case STATE_BACKSPACE:
		if (! State.runmode)
			delprog();
		else if (State.alphas) {
			char *p = find_char(Alpha, '\0');
			if (p > Alpha)
				*--p = '\0';
		}
		break;

	case STATE_RUNNING:
		xeqprog();  // continue execution
		break;

	case STATE_UNFINISHED:
	case STATE_IGNORE:
		break;

	default:
		if (State.runmode) {
			xeq(c);
			xeqprog();
		} else
			stoprog(c);
	}
	if (!running() && c != STATE_IGNORE ) {
		// The condition "running()" is still questionable. 
		// Do we want the display updated while a program runs?
		// A probable solution is a user controlable system flag
		display();
	}
}

void init_34s(void) {
	xeq_init();
        init_state();
#if defined(REALBUILD) || defined(WINGUI)
	display();
#endif
}

#ifndef REALBUILD

/*
 *  Create the persistant RAM area
 */
struct _ram PersistentRam;

#ifndef WINGUI

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
static const char *const map32[32] = {
	NULL,	"x-bar", "y-bar", "sqrt", "integral", "degree", "space", "grad",
	"+/-", "<=", ">=", "!=", "euro", "->", "<-", "v",
	"^", "f-shift", "g-shift", "h-shift", "cmplx", "O-slash", "o-slash", "<->",
	"sz", "x-hat", "y-hat", "sub-m", "times", "approx", "pound", "yen"
};

static const char *const maptop[129] = {
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
	const int oldcata = State.catalogue;
	int n;

	State.catalogue = cata;
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
	State.catalogue = oldcata;
}

#include "xrom.h"
static const struct {
	opcode op;
	const char *const name;
} xrom_labels[] = {
#define X(op, n, s)	{ RARG(RARG_ ## op, (n) & RARG_MASK), s},
#define XL(n, s)	X(LBL, n, s)
	XL(ENTRY_SIGMA, "Entry: SUMMATION")
	XL(ENTRY_PI, "Entry: PRODUCT")
	XL(ENTRY_SOLVE, "Entry: SOLVE")
	XL(ENTRY_INTEGRATE, "Entry: INTEGRATE")
	XL(XROM_CHECK, "Internal: Common entry code")
	XL(XROM_EXIT, "Internal: Normal exit code")
	XL(XROM_EXITp1, "Internal: Abnormal exit code")
#undef XL
#define XG(n, s)	X(GTO, n, s)	X(XEQ, n, s)
	XG(XROM_CHECK, "-> Common entry code")
	XG(XROM_EXIT, "-> Normal exit code")
	XG(XROM_EXITp1, "-> Abnormal exit code")
#undef XG
#define XE(n, s)	X(ERROR, n, s)
	XE(ERR_DOMAIN, "Error: Domain Error")
	XE(ERR_BAD_DATE, "Error: Bad Date Error")
	XE(ERR_PROG_BAD, "Error: Undefined Op-code")
	XE(ERR_INFINITY, "Error: +infinity")
	XE(ERR_MINFINITY, "Error: -infinity")
	XE(ERR_NO_LBL, "Error: no such label")
	XE(ERR_XROM_NEST, "Error: Slv integrate sum product nested")
	XE(ERR_RANGE, "Error: out of range error")
	XE(ERR_DIGIT, "Error: bad digit error")
	XE(ERR_TOO_LONG, "Error: too long error")
	XE(ERR_XEQ_NEST, "Error: >8 levels nested")
	XE(ERR_STK_CLASH, "Error: stack clash")
	XE(ERR_BAD_MODE, "Error: bad mode error")
	XE(ERR_INT_SIZE, "Error: word size too small")
#undef XE
#undef X
};
#define num_xrom_labels		(sizeof(xrom_labels) / sizeof(*xrom_labels))

static void dump_xrom(void) {
	unsigned int pc = addrXROM(0);
	const unsigned int max = addrXROM(xrom_size);

	printf("%u XROM instructions\n\n", max-pc);
	printf("ADDR  MNEMONIC\t\tComment\n\n");
	do {
		char instr[16];
		const opcode op = getprog(pc);
		const char *p = prt(op, instr);
		int i;
		printf("%04x  ", pc);
		pc = inc(pc);
		while (*p != '\0') {
			char c = *p++;
			const char *q = pretty(c);
			if (q == NULL) putchar(c);
			else printf("[%s]", q);
		}
		for (i=0; i<num_xrom_labels; i++)
			if (xrom_labels[i].op == op)
				printf("\t\t%s", xrom_labels[i].name);
		putchar('\n');
	} while (pc != addrXROM(0));
}

/*
 *  Save/Load state
 */
void save_state( void )
{
	FILE *f = fopen( "wp34s.dat", "wb" );
	if ( f == NULL ) return;
	fwrite( &PersistentRam, sizeof( PersistentRam ), 1, f );
#ifdef DEBUG
	printf( "sizeof struct _state = %d\n", (int)sizeof( struct _state ) );
	printf( "sizeof RAM = %d (%d free)\n", (int)sizeof(PersistentRam), 2048 - (int)sizeof(PersistentRam));
	printf( "sizeof pointer = %d\n", (int)sizeof( char * ) );
	printf( "sizeof decNumber = %d\n", (int)sizeof(decNumber));
	printf( "sizeof decContext = %d\n", (int)sizeof(decContext));
#endif
}

void load_state( void )
{
	FILE *f = fopen( "wp34s.dat", "rb" );
	if ( f == NULL ) return;
	fread( &PersistentRam, sizeof( PersistentRam ), 1, f );
}

int main(int argc, char *argv[]) {
	int c, n = 0;
	decContext ctx, ctx64;

	Ctx = &ctx;
	Ctx64 = &ctx64;
	xeq_init_contexts();
	if (argc > 1) {
		if (argc == 2) {
			if (argv[1][0] == 'x' && argv[1][1] == 'r' && argv[1][2] == 'o' && argv[1][3] == 'm' && argv[1][4] == '\0') {
				dump_xrom();
				return 0;
			}
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
			dump_menu("programming", "", CATALOGUE_PROG);
			dump_menu("modes", "", CATALOGUE_MODE);
			dump_menu("test", "", CATALOGUE_TEST);
			dump_menu("conversions", "", CATALOGUE_CONV);
			dump_menu("constants", "# ", CATALOGUE_CONST);
			dump_menu("complex constants", "[cmplx]# ", CATALOGUE_COMPLEX_CONST);
#ifdef INCLUDE_INTERNAL_CATALOGUE
			dump_menu("internals", "", CATALOGUE_INTERNAL);
#endif
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

	load_state();
	init_34s();
	if (setuptty(0) == 0) {
		display();
		while ((c = GETCHAR()) != GETCHAR_ERR && c != CH_QUIT) {
#ifdef USECURSES
			if (c == CH_TRACE) {
				State.trace = 1 - State.trace;
				display();
			} else if (c == CH_FLAGS) {
				State.flags = 1 - State.flags;
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
	save_state();
	return 0;
}
#endif
#endif

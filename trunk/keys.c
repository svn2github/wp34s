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

#define TEST_EQ		0
#define TEST_NE		1
#define TEST_LT		2
#define TEST_LE		3
#define TEST_GT		4
#define TEST_GE		5

/* Short blink of PRN annunciator with every key */
int BusyBlink;

enum confirmations {
	confirm_none=0, confirm_clall, confirm_reset, confirm_clprog
};

enum shifts cur_shift(void) {
	return (enum shifts) State2.shifts;
}

void set_shift(enum shifts s) {
	State2.shifts = s;
	State2.alpha_pos = 0;
}


static void init_arg(const enum rarg base) {
	State.base = base;
	State2.ind = 0;
	State2.digval = 0;
	State2.numdigit = 0;
	State2.rarg = 1;
	State2.dot = 0;
}

static void init_cat(enum catalogues cat) {
	if (cat == CATALOGUE_NONE) {
		// Save last catalogue for a later restore
		State.last_cat = State2.catalogue;
		State.last_catpos = State2.digval;
		CmdLineLength = 0;
	}
	process_cmdline_set_lift();
	State2.catalogue = cat;
	State2.cmplx = (cat == CATALOGUE_COMPLEX || cat == CATALOGUE_COMPLEX_CONST)?1:0;
	if (cat != CATALOGUE_NONE && State.last_cat == cat) {
		// Same catalogue again, restore position
		State2.digval = State.last_catpos;
	}
	else {
		State2.digval = 0;
	}
	set_shift(SHIFT_N);
}

void init_state(void) {
	struct _state s;
	xset(&s, 0, sizeof(s));

#define C(n)	s.n = State.n
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
	C(fixeng);
	C(int_base);
	C(int_mode);
	C(date_mode);
	C(denom_mode);
        C(denom_max);
        C(improperfrac);
        C(nothousands);
        C(leadzero);
        C(fract);
        C(contrast);
	C(jg1582);
#undef C
	xcopy(&State, &s, sizeof(struct _state));

	xset(&State2, 0, sizeof(State2));
	//State2.shifts = SHIFT_N;
	State2.test = TST_NONE;
	State2.runmode = 1;
	ShowRegister = regX_idx;
}

static void init_confirm(enum confirmations n) {
	State2.confirm = n;
}

static void set_smode(const enum single_disp d) {
	State2.smode = (State2.smode == d)?SDISP_NORMAL:d;
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
        { 'Q',  '4',  0000, 0000, 'q',  0000,  },  // K41
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
	if (State2.alphashift) {
		if (s == SHIFT_N)
			s = SHIFT_LC_N;
		else if (s == SHIFT_G)
			s = SHIFT_LC_G;
	}
	return alphamap[c][s];
}

static int check_f_key(int n, const int dflt) {
	const int code = 100 + n;

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
		return check_f_key(0, OP_SPEC | OP_SIGMAPLUS);
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
		State2.arrow = 1;
		break;
	case K_CMPLX:
		if (intltr(15))
			return OP_SPEC | OP_F;
		if (!State.intm)
			State2.cmplx = 1;
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
	case K21:	return OP_NIL | OP_SWAP;	// x<>y
	case K22:	return OP_SPEC | OP_CHS;	// CHS
	case K23:	return OP_SPEC | OP_EEX;	// EEX
	case K24:
		if (State2.runmode)
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
		State2.hyp = 1;
		State2.dot = 1;
		State2.cmplx = 0;
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
			State2.arrow = 1;
			break;
		}
		return OP_NIL | OP_P2R;
	case K_CMPLX:
		return OP_NIL | OP_FRACPROPER;

	case K10:	return OP_NIL | OP_HMS;
	case K11:	return OP_NIL | OP_FLOAT;
	case K12:	return OP_NIL | OP_RANDOM;

	case K20:
		State2.alphas = 1;
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
		if (State.intm && State.int_maxw > State2.int_window)
			State2.int_window++;
		break;
	case K51:	State2.test = TST_EQ;	break;
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
		State2.hyp = 1;
		State2.dot = 0;
		State2.cmplx = 0;
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
		if (State.intm && State.int_maxw > 0 && State2.int_window > 0)
			State2.int_window--;
		break;
	case K51:	State2.test = TST_NE;	break;
	case K52:	init_arg(RARG_INTG);	break;
	case K53:	init_arg(RARG_SUM);	break;
	case K54:	return OP_MON | OP_PERCNT;

	case K60:       return OP_NIL | OP_OFF;
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
	case K00:	return OP_NIL | OP_ALL;
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
		if (State2.runmode) {
			set_pc(0);
			clrretstk();
		} else
			init_confirm(confirm_clprog);
		break;
	case K24:	return OP_NIL | OP_rCLX;

	case K30:	init_arg(RARG_GTO);		break;
	case K31:	return OP_DYA | OP_LAND;
	case K32:	return OP_DYA | OP_LOR;
	case K33:	return OP_DYA | OP_LXOR;
	case K34:	return OP_DYA | OP_MOD;

	case K40:	return OP_MON | OP_FACT;
	case K41:	init_cat(CATALOGUE_PROB);	break;
	case K42:	init_cat(CATALOGUE_STATS);	break;
	case K43:	return OP_NIL | OP_statLR;
	case K44:	State2.status = 1;		break;

	case K50:
		init_cat(State.intm?CATALOGUE_INT:CATALOGUE_NORMAL);
		break;
	case K51:	init_cat(CATALOGUE_TEST);	break;
	case K52:	init_cat(CATALOGUE_PROG);	break;
	case K53:	return CONST(OP_PI);
	case K54:	return OP_SPEC | OP_SIGMAMINUS;

	case K60:	set_smode(SDISP_SHOW);		break;
	case K61:	init_arg(RARG_PAUSE);		break;
	case K62:
		if (State.fraccomma)
			return OP_NIL | OP_RADDOT;
		return OP_NIL | OP_RADCOM;

	case K63:					// Program<->Run mode
		State2.runmode = 1 - State2.runmode;
		process_cmdline_set_lift();
		break;
	case K64:	return OP_SPEC | OP_SIGMAPLUS;
	}
	return STATE_UNFINISHED;
}


static int process_normal_cmplx(const keycode c) {
	State2.cmplx = 0;
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
	State2.cmplx = 0;
	switch (c) {
	case K00:
		State2.hyp = 1;
		State2.dot = 1;
		State2.cmplx = 1;
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
		State2.cmplx = 1;
		State2.test = TST_EQ;
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
	State2.cmplx = 0;
	switch (c) {
	case K00:
		State2.hyp = 1;
		State2.dot = 0;
		State2.cmplx = 1;
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
		State2.cmplx = 1;
		State2.test = TST_NE;
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
	State2.cmplx = 0;
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
 * the function.  The inverse routines use the code too, the State2.dot
 * is 1 for normal and 0 for inverse hyperbolic.  We also have to
 * deal with the complex versions and the handling of that key and
 * the ON key are dealt with by our caller.
 */
static int process_hyp(const keycode c) {
	const int cmplx = State2.cmplx;
	const int dot = State2.dot;
	const opcode x = cmplx?OP_CMON:OP_MON;

	State2.hyp = 0;
	State2.cmplx = 0;
	State2.dot = 0;

	switch (c) {
	case K01:	return x + (dot?OP_SINH:OP_ASINH);
	case K02:	return x + (dot?OP_COSH:OP_ACOSH);
	case K03:	return x + (dot?OP_TANH:OP_ATANH);

	case K60:
	case K24:
		break;
	default:
		State2.hyp = 1;
		State2.cmplx = cmplx;
		State2.dot = dot;
		break;
	}
	return STATE_UNFINISHED;
}


static int process_arrow(const keycode c) {
	const enum shifts oldstate = cur_shift();

	set_shift(SHIFT_N);
	State2.arrow = 0;
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
			State2.arrow_alpha = 1;
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
	unsigned int dv = State2.digval;
	const unsigned int val = dv * 10 + n;

	State2.numdigit++;
	if (State2.numdigit == 2) {		// two digits starting large
		if (val > NUMPROG / 10)
			return val;
	}
	if (State2.numdigit == 3)
		return val;
	State2.digval = val;
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
	unsigned int rawpc;

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
	case K00:	case K01:	case K02:	case K03:
		rawpc = gtodot_fkey(c - K00);
		goto fin;
#else
	case K00:	rawpc = gtodot_fkey(0);	goto fin;
	case K01:	rawpc = gtodot_fkey(1);	goto fin;
	case K02:	rawpc = gtodot_fkey(2);	goto fin;
	case K03:	rawpc = gtodot_fkey(3);	goto fin;
#endif

	case K62:		// .
		if (State2.numdigit == 0) {
			rawpc = 0;
			goto fin;
		}
		break;

	case K20:		// ENTER - short circuit processing
		pc = State2.digval;
		break;

	case K24:		// backspace
		if (State2.numdigit == 0) {
			pc = state_pc();
		} else {
			State2.numdigit--;
			State2.digval /= 10;
		}
	default:
		return STATE_UNFINISHED;
	}
	if (pc >= 0) {
		rawpc = find_user_pc(pc);
fin:		set_pc(rawpc);
		State2.gtodot = 0;
		State2.digval = 0;
		State2.numdigit = 0;
	}
	return STATE_UNFINISHED;
}


/* Process a keystroke in alpha mode
 */
static int process_alpha(const keycode c) {
	const enum shifts oldstate = cur_shift();
	unsigned char ch;
	unsigned int alpha_pos = State2.alpha_pos, n;
	State2.alpha_pos = 0;
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
			init_cat(State2.alphashift?CATALOGUE_ALPHA_LETTERS_LOWER:
						CATALOGUE_ALPHA_LETTERS_UPPER);
			return STATE_UNFINISHED;
		}
		break;

	case K20:	// Enter - maybe exit alpha mode
		if (oldstate == SHIFT_G)
			break;
		State2.alphas = 0;
		State2.alphashift = 0;
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

	case K40:
		if (oldstate == SHIFT_N) {	// Alpha scroll left
			n = alpha_pos + 1;
			State2.alpha_pos = (6*n < alen()) ? n : alpha_pos;
			return STATE_UNFINISHED;
		}
		break;

	case K44:
		if (oldstate == SHIFT_H) {
			State2.status = 1;
			return STATE_UNFINISHED;
		}
		break;

	case K50:
		if (oldstate == SHIFT_N) {	// Alpha scroll right
			if (alpha_pos > 0)
				State2.alpha_pos = alpha_pos-1;
			return STATE_UNFINISHED;
		}
		if (oldstate == SHIFT_H) {	// Alpha command catalogue
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
			State2.alphashift = 1 - State2.alphashift;
		else if (oldstate == SHIFT_G)
			return OP_NIL | OP_OFF;
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
			(State2.ind?RARG_IND:0) + dv;
	init_arg(0);
	State2.rarg = 0;
	return r;
}

static int arg_digit(int n) {
	const unsigned int base = State.base;
	const int mx = State2.ind ? NUMREG : argcmds[base].lim;
	const unsigned int val = State2.digval * 10 + n;

	if (val == TOPREALREG-1 && argcmds[base].cmplx)
		return STATE_UNFINISHED;
	if (State2.numdigit == 0) {
		if (n * 10 >= mx)
			return arg_eval(n);
	} else {
		if ((int) val >= mx)
			return STATE_UNFINISHED;
	}
	State2.digval = val;
	State2.numdigit++;
	if (State2.numdigit == 2)
		return arg_eval(val);
	return STATE_UNFINISHED;
}

static int arg_fkey(int n) {
	const unsigned int b = State.base;

	if ((b >= RARG_LBL && b <= RARG_INTG) || (b >= RARG_SF && b <= RARG_FCF)) {
		if (State2.ind || State2.numdigit > 0)
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

static int process_arg_dot(const unsigned int base) {
	int r = STATE_UNFINISHED;
	if (State2.dot || argcmds[base].stckreg || State2.ind)
		return arg_eval(regX_idx);
	switch (base) {
	case RARG_SCI:	r = OP_NIL | OP_FIXSCI;	break;
	case RARG_ENG:	r = OP_NIL | OP_FIXENG;	break;

	case RARG_GTO:
		// Special GTO . sequence
		if (State2.numdigit == 0 && ! State2.ind) {
			State2.gtodot = 1;
			break;
		}
		return r;

	default:
		return r;
	}
	/* Clean up and return the specified code */
	init_arg(0);
	State2.rarg = 0;
	return r;
}

static int process_arg(const keycode c) {
	unsigned int base = State.base;

	if (base >= num_argcmds) {
		init_arg(0);
		State2.rarg = 0;
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
		if (!State2.dot && argcmds[base].indirectokay) {
			State2.ind = 1 - State2.ind;
			if (State2.ind == 0 && !argcmds[base].stckreg)
				State2.dot = 0;
		}
		break;

	case K_CMPLX:
	case K12:	  // I (lastY)
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regI_idx);
		break;

	case K23:	  // L (lastX)
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			return arg_eval(regL_idx);
		break;
	case K21:	// J
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			return arg_eval(regJ_idx);
		break;
	case K22:	// K
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regK_idx);
		break;
	case K62:		// X
		return process_arg_dot(base);
	case K63:		// Y
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regY_idx);
		break;

	/* STO and RCL can take an arithmetic argument */
	case K64:		// Z register
		if (State2.dot || ( ! arg_storcl(RARG_STO_PL - RARG_STO, 1) &&
					(argcmds[base].stckreg || State2.ind)))
			return arg_eval(regZ_idx);
		break;
	case K54:
		arg_storcl(RARG_STO_MI - RARG_STO, 1);
		break;

	case K44:		// T register
		if (State2.dot || ( ! arg_storcl(RARG_STO_MU - RARG_STO, 1) &&
					(argcmds[base].stckreg || State2.ind)))
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
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			return arg_eval(regA_idx);
		return arg_fkey(0);
	case K02:
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			return arg_eval(regC_idx);
		return arg_fkey(2);	// F2
#ifdef SEQUENTIAL_ROWS
	case K01:	case K03:
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regB_idx + c - K01);
		return arg_fkey(c - K00);
#else
	case K01:
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regB_idx);
		return arg_fkey(1);	// F1
	case K03:
		if (State2.dot || argcmds[base].stckreg || State2.ind)
			if (!argcmds[base].cmplx)
				return arg_eval(regD_idx);
		return arg_fkey(3);	// F3
#endif

	case K20:				// Enter is a short cut finisher
		if (State2.numdigit == 0 && !State2.ind && !State2.dot) {
			if (base >= RARG_LBL && base <= RARG_INTG) {
				init_arg(base - RARG_LBL);
				State2.multi = 1;
				State2.rarg = 0;
			} else if (argcmds[base].stckreg)
				State2.dot = 1;
		} else if (State2.numdigit > 0)
			return arg_eval(State2.digval);
		else if (argcmds[base].stckreg || State2.ind)
			State2.dot = 1 - State2.dot;
		break;

	case K60:
	case K24:
		init_arg(0);
		State2.rarg = 0;
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
		State2.multi = 0;
		if (State2.numdigit == 0) {
			return STATE_UNFINISHED;
		} else if (State2.numdigit == 1) {
			return OP_DBL + (State.base << DBL_SHIFT) + State2.digval;
		} else {
			return OP_DBL + (State.base << DBL_SHIFT) + State2.digval +
				(State2.digval2 << 16);
		}

	case K24:	// Clx - backspace, clear alpha
		if (oldstate == SHIFT_N || oldstate == SHIFT_F) {
			if (State2.numdigit == 0)
				State2.multi = 0;
			else
				State2.numdigit--;
			return STATE_UNFINISHED;
		}
		break;

	case K60:	// EXIT/ON maybe case switch, otherwise exit alpha
		if (oldstate == SHIFT_F)
			State2.alphashift = 1 - State2.alphashift;
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
	if (State2.numdigit == 0) {
		State2.digval = ch;
		State2.numdigit = 1;
		return STATE_UNFINISHED;
	} else if (State2.numdigit == 1) {
		State2.digval2 = ch;
		State2.numdigit = 2;
		return STATE_UNFINISHED;
	}
	State2.multi = 0;
	return OP_DBL + (State.base << DBL_SHIFT) + State2.digval +
			(State2.digval2 << 16) + (ch << 24);
}


static int process_test(const keycode c) {
	int r = State2.test;
	int cmpx = State2.cmplx;

	State2.test = TST_NONE;
	State2.cmplx = 0;
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
	State2.test = r;
	State2.cmplx = cmpx;
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
	return catalogue_sizes[State2.catalogue];
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
	switch (State2.catalogue) {
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
			dv = State2.digval;

			if ((int) dv < ctmax) {
				const opcode op = current_catalogue(dv);

				init_cat(CATALOGUE_NONE);

				if (op & OP_RARG) {
					const unsigned int rarg = (op & ~OP_RARG) >> RARG_OPSHFT;

					if (rarg == RARG_CONST || rarg == RARG_CONST_CMPLX || rarg == RARG_CONV || rarg == RARG_ALPHA)
						return op;
					if (rarg >= RARG_TEST_EQ && rarg <= RARG_TEST_GE) {
						State2.test = TST_EQ + (rarg - RARG_TEST_EQ);
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
			} else
				init_cat(CATALOGUE_NONE);
			return STATE_UNFINISHED;

		case K24:			// backspace
			if (CmdLineLength > 0 && Keyticks < 30) {
				if (--CmdLineLength > 0)
					goto search;
				State2.digval = 0;
			} else
				init_cat(CATALOGUE_NONE);
			return STATE_UNFINISHED;

		case K60:
			init_cat(CATALOGUE_NONE);
			return STATE_UNFINISHED;

		case K40:
			if (State2.digval > 0)
				State2.digval--;
			else
				State2.digval = ctmax-1;
			CmdLineLength = 0;
			return STATE_UNFINISHED;

		case K50:
			if ((int) ++State2.digval >= ctmax)
				State2.digval = 0;
			CmdLineLength = 0;
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
	if (Keyticks >= 30)
		CmdLineLength = 0;	// keyboard search timed out
	if (CmdLineLength < 10)
		Cmdline[CmdLineLength++] = ch;
	/* Search for the current buffer in the catalogue */

search:
	Cmdline[CmdLineLength] = '\0';
	for (dv = 0; dv < (unsigned int)ctmax; dv++) {
		char buf[16];
		const char *cmd = catcmd(current_catalogue(dv), buf);
		int i;

		if (*cmd == COMPLEX_PREFIX)
			cmd++;
		for (i=0; cmd[i] != '\0'; i++) {
			const unsigned char c = remap_chars(cmd[i]);
			const unsigned char cl = 0xff&Cmdline[i];
			if (c > cl) {
				State2.digval = dv;
				return STATE_UNFINISHED;
			} else if (c < cl)
				break;
		}
		if (Cmdline[i] == '\0') {
			State2.digval = dv;
			return STATE_UNFINISHED;
		}
	}
	State2.digval = ctmax-1;
	return STATE_UNFINISHED;
}


static int process_confirm(const keycode c) {
	switch (c) {
	case K63:			// Yes
		switch (State2.confirm) {
		case confirm_clall:	clrall(NULL, NULL, NULL);	break;
		case confirm_reset:	reset(NULL, NULL, NULL);	break;
		case confirm_clprog:	clrprog();
		}
		State2.confirm = confirm_none;
		State2.digval = 0;
		break;

	//case K60:
	case K24:
	case K32:			// No
		State2.confirm = 0;
		State2.digval = 0;
		break;
	default:			// No state change
		break;
	}
	return STATE_UNFINISHED;
}

static int process_status(const keycode c) {
	int n = State2.status - 1;

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
	State2.status = n+1;
	return STATE_UNFINISHED;
}


static int process(const int c) {
	const enum shifts s = cur_shift();

	if (c == K_HEARTBEAT) {
		/*
		 *  Heartbeat processing goes here.
		 *  This is totally thread safe!
		 */

		/*
		 *  Toggle the RPN annunciator as a visual feedback
		 */
		dot(RPN, !BusyBlink);
		BusyBlink = 0;
		finish_display();

		/*
		 *  Serve the watchdog
		 */
		watchdog();

		/*
		 *  If buffer is empty re-allow R/S to start a program
		 */
		if ( JustStopped && !is_key_pressed() ) {
			JustStopped = 0;
		}

		/*
		 *  Do nothing if not running a program
		 */
		if (!Running && !Pause)
			return STATE_IGNORE;
	}

	if (Running || Pause) {
		/*
		 *  Abort a running program with R/S or EXIT
		 */
		if (c == K60 || c == K63) {
			set_running_off();
			return STATE_UNFINISHED;
		}
		if ( c != K_HEARTBEAT ) {
			LastKey = (char) (c + 1);	// Store for KEY?
			Pause = 0;			// leave PSE statement
		}
		// continue execution if really running, else ignore (PSE)
		return STATE_RUNNING;
	}

	/*
	 * Turn off the RPN annunciator as a visual feedback
	 */
	BusyBlink = 1;
	dot(RPN, 0);
	finish_display();

	/* Check for ON in the unshifted state -- this is a reset sequence
	 * common across all modes.  Shifted modes need to check this themselves
	 * if required.
	 */
	if (c == K60 && s == SHIFT_N && ! State2.catalogue) {
		init_state();
		return STATE_UNFINISHED;
	}
	if ( c == K63 && JustStopped ) {
		// Avoid an accidental restart with R/S
		JustStopped = 0;
		return STATE_IGNORE;
	}

	if (State2.status)
		return process_status((const keycode)c);

	if (State2.confirm)
		return process_confirm((const keycode)c);

	if (State2.rarg)
		return process_arg((const keycode)c);

	if (State2.gtodot)
		return process_gtodot((const keycode)c);

	if (State2.hyp)
		return process_hyp((const keycode)c);

	if (State2.test != TST_NONE)
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

	if (State2.catalogue)
		return process_catalogue((const keycode)c);

	if (State2.multi)
		return process_multi((const keycode)c);

	if (State2.alphas)
		return process_alpha((const keycode)c);

	if (State2.arrow)
		return process_arrow((const keycode)c);

	if (State2.cmplx) {
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
		if (! State2.runmode)
			delprog();
		else if (State2.alphas) {
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
		if (State2.runmode) {
			xeq(c);
			xeqprog();
		} else
			stoprog(c);
	}
	if (!Running && !Pause && c != STATE_IGNORE) {
		display();
	}
}




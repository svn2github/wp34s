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
#include "storage.h"
#include "display.h"
#include "lcd.h"
#include "int.h"
#include "consts.h"
#include "alpha.h"
#include "stats.h"
#include "decn.h"
#include "revision.h"

static enum separator_modes { SEP_NONE, SEP_COMMA, SEP_DOT } SeparatorMode;
static enum decimal_modes { DECIMAL_DOT, DECIMAL_COMMA } DecimalMode;

static void set_status_sized(const char *, int);
static void set_status(const char *);
static void set_status_right(const char *);

const char *DispMsg;	   // What to display in message area

int ShowRPN;		   // controls visibility of RPN annunciator
int JustDisplayed;	   // Avoid duplicate calls to display()

/* Message strings
 * Strings starting S7_ are for the lower 7 segment line.  Strings starting S_
 * are for the upper dot matrix line.
 */
static const char S_SURE[] = "Sure?";

static const char S7_ERROR[] = "Error";		/* Default lower line error display */
static const char S7_NaN[] = "not nuMmerIc";	/* Displaying NaN in lower line */
static const char S7_INF[] = "infinity";	/* Displaying infinity in lower line */

static const char S7_STEP[] = "Step ";		/* Step marker in program mode (lower line) */

static const char S7_fract_EQ[] = " = ";	/* Exponent in fraction mode indicates low, equal or high */
static const char S7_fract_LT[] = " Lt";
static const char S7_fract_GT[] = " Gt";

static const char libname[][5] = {
	"rAMm", "Lib ", "Bup ",
#ifndef REALBUILD
	"roMm"
#endif
};


/* Table of error messages.
 * These consist of a double string.  The first is displayed in the
 * top line, the second in the bottom.  If the second is empty, "Error"
 * is displayed instead.  To get a blank lower line, include a space.
 */
void error_message(const enum errors e) 
{
	// NB: this MUST be in the same order as `enum errors'
	static const char *const error_table[] = 
	{
		// manually get the order correct!
		"Running\0PrograMm", 
		"Domain\0",
		"Bad time\0or dAtE",
		"Undefined\0Op-CODE",
		"+\237\0",
		"-\237\0",
		"No such\0LABEL",
		"Illegal\0OperAtion",
		"Out of range\0",
		"Bad digit\0",
		"Too long\0",
		"RAM is\0Full",
		"Stack\0CLASH",
		"Bad mode\0",
		"Word size\0too SMmALL",
		"Too few\0dAtA Points",
		"Invalid\0ParaMmEtEr",
		"I/O\0Error",
		"Invalid\0dAtA",
		"Write\0Protect",
		"Solve\0FAILEd",
	#ifdef MATRIX_SUPPORT
		"Matrix\0MmISMmAtCH",
		"Singular\0",
	#endif
		"Flash is\0Full",
	};

	if (e != ERR_NONE || Running) {
		const char *p = error_table[e];
		const char *q = find_char(p, '\0') + 1;
		if (*q == '\0')
			q = S7_ERROR;
		message( p, q );
		State2.disp_freeze = (e != ERR_NONE);
	}
}


/* Variable width font for the dot matrix part of the display.
 * Each character consists of a length and six
 * five bit bit masks that define the character.
 * This means that the maximum character width is six,
 * five real bits and a space column.  It is possible to
 * set make a character wider than this, but the right side
 * will be blank.  We store the lengths and definitions in
 * separate arrays to make for shorter/faster access.
 */

static void unpack6(unsigned long s, unsigned char d[6]) {
	int i;

	for (i=5; i>=0; i--) {
		d[i] = s & 31;
		s >>= 5;
	}
}

#define pack6(a, b, c, x, y, z)					\
		(	((a & 31) << 25) | ((b & 31) << 20) |	\
			((c & 31) << 15) | ((x & 31) << 10) |	\
			((y & 31) << 5) | (z & 31)	)

static unsigned int charlengths(unsigned int c) {
	return (charlengthtbl[c/5] >> (3*(c%5))) & 7;
}

static const unsigned long chars[512] = {
#define C(len, a, b, c, x, y, z)	pack6(a, b, c, x, y, z)
#include "charset.h"
#undef C
};

/* Define a limited character set for the 7-segment portion of the
 * display.
 */
#define D_TOP 64
#define D_TL 32
#define D_TR 8
#define D_MIDDLE 16
#define D_BL 4
#define D_BR 1
#define D_BOTTOM 2

#include "charset7.h"

static int getdig(int ch)
{
	// perform index lookup
	return digtbl[ch&0xff];
}

void dot(int n, int on) {
	if (on)	set_dot(n);
	else	clr_dot(n);
}



/* Set the decimal point *after* the indicated digit
 * The marker can be either a comma or a dot depending on the value
 * of decimal.
 */
static char *set_decimal(const int posn, const enum decimal_modes decimal, char *res) {
	if (res) {
		*res++ = (decimal == DECIMAL_DOT)?'.':',';
	} else {
		set_dot(posn+7);
		if (decimal != DECIMAL_DOT)
			set_dot(posn+8);
	}
	return res;
}

/* Set the digit group separator *before* the specified digit.
 * This can be nothing, a comma or a dot depending on the state of the
 * sep argument.
 */
static void set_separator(int posn, const enum separator_modes sep, char *res) {
	if (sep == SEP_NONE)
		return;
	if (res) {
		if (sep == SEP_COMMA) *res++ = ',';
		else *res++ = '.';
	} else {
		posn -= SEGS_PER_DIGIT;
		set_dot(posn+7);
		if (sep == SEP_COMMA)
			set_dot(posn+8);
	}
}


/* Set a digit in positions [base, base+6] */
static void set_dig(int base, char ch) 
{
	int i;
	unsigned char c = getdig(ch);
	for (i=6; i>=0; i--)
	{
//		dot(base, c & (1 << i));
		if (c & (1 << i))
			set_dot(base);
		else
			clr_dot(base);
		base++;
	}
}

static char *set_dig_s(int base, char ch, char *res) {
	if (res) *res++ = ch;
	else	set_dig(base, ch);
	return res;
}


static void set_digits_string(const char *msg, int j) {
	for (; *msg != '\0'; msg++) {
		if (*msg == '.' || *msg == ',')
			set_decimal(j - SEGS_PER_DIGIT, *msg == '.' ? DECIMAL_DOT : DECIMAL_COMMA, NULL);
		else {
			set_dig_s(j, *msg, NULL);
			j += SEGS_PER_DIGIT;
		}
	}
}


/* Force the exponent display */
static void set_exp(short exp, int zerop, char *res) {
	char buf[6], *p;
	int j = SEGS_EXP_BASE;

	if (res) *res++ = 'e';
	if (exp < 0) {
		if (res) *res++ = '-';
		else set_dot(EXP_SIGN);
		exp = -exp;
	}
#ifdef INCLUDE_DOUBLE_PRECISION
	if (res == NULL && exp > 999)
		scopy(buf, "HIG");
	else
#endif
	{
		xset(buf, '\0', sizeof(buf));
		if (zerop)
			num_arg_0(buf, exp, 3);
		else
			num_arg(buf, exp);
	}
	for (p = buf; *p !='\0'; p++) {
		res = set_dig_s(j, *p, res);
		j += SEGS_PER_EXP_DIGIT;
	}
}

static void carry_overflow(void) {
	const int base = SEGS_EXP_BASE;
	char c;
	unsigned int b;

	// Figure out the base
	switch (State2.smode) {
	case SDISP_BIN:	b = 2;		break;
	case SDISP_OCT:	b = 8;		break;
	case SDISP_DEC:	b = 10;		break;
	case SDISP_HEX:	b = 16;		break;
	default:	b = UState.int_base+1;	break;
	}

	// Display the base as the first exponent digit
	if (b > 10 && b < 16)
		set_dot(EXP_SIGN);
	c = "B34567o9D12345h"[b-2];
	set_dig(base, c);

	// Carry and overflow are the next two exponent digits if they are set
	if (get_carry())
		set_dig(base + SEGS_PER_EXP_DIGIT, 'c');
	if (get_overflow())
		set_dig(base + 2*SEGS_PER_EXP_DIGIT, 'o');
}


/* Display the annunicator text line.
 * Care needs to be taken to keep things aligned.
 * Spaces are 5 pixels wide, \006 is a single pixel space.
 */
static void annunciators(void) {
	char buf[42], *p = buf, *q;
	int n;

	/* Set the shift key indicator */
	switch (cur_shift()) {
	default:
	case SHIFT_N:
#ifdef INCLUDE_DOUBLE_PRECISION
		if (is_dblmode())
			*p++ = 'D';
		else
#endif
		if (State2.wascomplex) {
			*p++ = 'C';
			State2.wascomplex = 0;
		} else
			p = scopy(p, " \006");
		break;
	case SHIFT_F:	p = scopy(p, "\021\006");	break;
	case SHIFT_G:	p = scopy(p, "\022\006");	break;
	case SHIFT_H:	p = scopy(p, "\023\006");	break;
	}

	if (State2.cmplx) {
		*p++ = ' ';
		*p++ = COMPLEX_PREFIX;
		goto skip;
	}

	if (State2.arrow) {
		*p++ = ' ';
		*p++ = '\015';
		goto skip;
	}

	if (!State2.runmode && State2.alphas) {
		*p++ = '\240';
		*p++ = ':';
	} else if (!is_intmode()) {
		switch (UState.date_mode) {
		//case DATE_DMY:	q = "d.my\006\006";	break;
		case DATE_YMD:	q = "y.md\006\006";	break;
		case DATE_MDY:	q = "m.dy\006\006";	break;
		default:	q = "    \006";		break;
		}
		p = scopy(p, q);
		p = scopy(p, (get_trig_mode() == TRIG_GRAD)?"\006\006\007":" \006\006\006");
	} else {
		switch(int_mode()) {
		default:
		case MODE_2COMP:	q = "2c\006";		break;
		case MODE_UNSIGNED:	q = "un\006";		break;
		case MODE_1COMP:	q = "\0061c\006\006";	break;
		case MODE_SGNMANT:	q = "sm";		break;
		}
		q = scopy(p, q);
		*q++ = '\006';
		p = num_arg_0(q, word_size(), 2);

		if (UState.int_maxw > 0) {
			n = 4 + 2 * (5 - UState.int_maxw);
			if (*q == '1')
				n += 2;
			if (q[1] == '1')
				n += 2;
			while (n-- > 0)
				*p++ = '\006';

			for (n=UState.int_maxw; n>=0; n--)
				*p++ = State2.int_window == n ? '|':'\'';
		}

	}

skip:	*p = '\0';
	set_status(buf);
}

static void disp_x(const char *p) {
	int i;
	int gotdot = -1;

	if (*p == '-') {
		set_dot(MANT_SIGN);
		p++;
	}

	if (is_intmode()) {
		for (i=0; *p != '\0'; p++) {
			set_dig(i, *p);
			i += SEGS_PER_DIGIT;
		}
		carry_overflow();
	} else {
		for (i=0; *p != '\0' && *p != 'E'; p++) {
			if (*p == '.') {
				if (gotdot == -1)
					gotdot = i;
				if (i > 0)
					set_decimal(i - SEGS_PER_DIGIT, DecimalMode, NULL);
				else {
					set_dig(i, '0');
					set_decimal(i, DecimalMode, NULL);
					i += SEGS_PER_DIGIT;	
				}
			} else {
				set_dig(i, *p);
				i += SEGS_PER_DIGIT;
			}
		}

		/* Implement a floating comma */
		if (gotdot < 0)
			gotdot = i;
		for (;;) {
			gotdot -= 3 * SEGS_PER_DIGIT;
			if (gotdot <= 0)			// MvC: was '<', caused crash
				break;
			set_separator(gotdot, SeparatorMode, NULL);
		}

		if (*p == 'E') {
			p++;
			if (*p != '\0') {
				if (*p == '-') {
					set_dot(EXP_SIGN);
					p++;
				}
			}
			set_exp(s_to_i(p), 1, NULL);
		}
	}
}

static const char DIGITS[] = "0123456789ABCDEF";

static void set_int_x(const long long int value, char *res) {
	const int ws = word_size();
	unsigned int b;
	long long int vs = value;
	unsigned long long int v;
	char buf[MAX_WORD_SIZE + 1];
	int i, j, k;
	int sign = 0;
	int dig = SEGS_PER_DIGIT * 11;

	switch (State2.smode) {
	case SDISP_BIN:	b = 2;		break;
	case SDISP_OCT:	b = 8;		break;
	case SDISP_DEC:	b = 10;		break;
	case SDISP_HEX:	b = 16;		break;
	default:	b = int_base();	break;
	}

	if (!res) {
		UState.int_maxw = 0;
		carry_overflow();
	}

	if (b == 10) {
		v = extract_value(value, &sign);
		if (int_mode() == MODE_2COMP && sign == 1 && v == 0)
			v = value;
		if (v == 0) {
			if (sign)
				set_dig_s(dig-SEGS_PER_DIGIT, '-', res);
			set_dig_s(dig, '0', res);
			return;
		} else
			for (i=0; v != 0; i++) {
				const int r = v % b;
				v /= b;
				buf[i] = DIGITS[r];
			}
	} else {
		// Truncate down to the current word size and then sign extend it back
		if (ws < 64) {
			const long long int mask = (1LL << ws) - 1;
			vs &= mask;
			if (b == 10 && (vs & (1LL << (ws-1))))
				vs |= ~mask;
		}

		if (!UState.leadzero && vs == 0) {
			set_dig_s(dig, '0', res);
			return;
		} else if (!UState.leadzero) {
			v = (unsigned long long int)vs;
			for (i=0; v != 0; i++) {
				const int r = v % b;
				v /= b;
				buf[i] = DIGITS[r];
			}
		} else {
			int n;
			v = (unsigned long long int)vs;

			if (b == 2)		n = ws;
			else if (b == 8)	n = (ws + 2) / 3;
			else			n = (ws + 3) / 4;
			for (i=0; i<n; i++) {
				const int r = v % b;
				v /= b;
				buf[i] = DIGITS[r];
			}
		}
	}

	/* At this point i is the number of digits in the output */
	if (res) {
		if (sign) *res++ = '-';
		while (--i >= 0)
			*res++ = buf[i];
	} else {
#if 0
		// Allows configuration of digit grouping per base
		static const char grouping[] = 
			{       0x84, 0xb3, 0xb4, 0xb3, 0xb3, 0xb3, 0xb3, 
		      //	   2     3     4     5     6     7     8
		          0xb3, 0xb3, 0xb3, 0xb3, 0xb3, 0xb3, 0xb3, 0xb2 };
		      //     9    10    11    12    13    14    15    16
		const int shift = SeparatorMode == SEP_NONE ? 12 
			        : grouping[b - 2] >> 4;
		const int group = SeparatorMode == SEP_NONE ? 16
				: (grouping[b - 2] & 0xf);
#else
		// Less flexible but shorter
		const int shift = b == 2 ? 8 : 12;
		const int group = (b == 2 || b == 4) ? 4
				: b == 16 ? 2 : 3;
#endif
		const int window = State2.int_window;

		UState.int_maxw = (i - 1) / shift;
		buf[i] = '\0';

		j = window * shift;	// digits at a time
		for (k = 0; k < 12; k++)
			if (buf[j + k] == '\0')
				break;
		for (i=0; --k >= 0; i++) {
			char ch = buf[j++];
			if (i >= shift)
				ch -= 030;
			set_dig(dig, ch);
			if ((j % group) == 0 && k != 0)
				set_separator(dig, SeparatorMode, NULL);
			dig -= SEGS_PER_DIGIT;
		}
		if (sign) {
			if (dig >= 0)
				set_dig(dig, '-');
			else	set_dot(MANT_SIGN);
		}
	}
}

/* Handle special cases.
 * return non-zero if the number is special.
 */
static int check_special_dn(const decNumber *x, char *res) {
	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x)) {
			if (res) {
				scopy(res, "NaN");
			} else {
				set_digits_string(S7_NaN, 0);
			}
			return 1;
		} else {
			if (decNumberIsNegative(x)) {
				if (res) *res++ = '-';
				else	set_dig(SEGS_PER_DIGIT, '-');
			}
			if (res)
				*res++ = '\237';
			else {
				set_digits_string(S7_INF, SEGS_PER_DIGIT * 2);
			}
			return 1;
		}
	}
	return 0;
}


/* Extract the two lowest integral digits from the number
 */
static void hms_step(decNumber *res, decNumber *x, unsigned int *v) {
	decNumber n;

	decNumberMod(&n, x, &const_100);
	*v = dn_to_int(&n);
	dn_mulpow10(&n, x, -2);
	decNumberTrunc(res, &n);
}

static char *hms_render(unsigned int v, char *str, int *jin, int n, int spaces) {
	char b[32];
	int i, j;

	for (i=0; i<n; i++) {
		if (v == 0)
			b[i] = spaces?' ':'0';
		else {
			j = v % 10;
			v /= 10;
			b[i] = j + '0';
		}
	}
	if (b[0] == ' ')
		b[0] = '0';

	/* Copy across and appropriately leading space things
	 */
	j = *jin;
	while (--i >= 0) {
		str = set_dig_s(j, b[i], str);
		j += SEGS_PER_DIGIT;
	}
	*jin = j;
	return str;
}


/* Display the number in H.MS mode.
 * HMS is hhh[degrees]mm'ss.ss" fixed formated modulo reduced to range
 */
static void set_x_hms(const decNumber *rgx, char *res) {
	decNumber x, y, a, t, u;
	int j=0;
	const int exp_last = SEGS_EXP_BASE + 2*SEGS_PER_EXP_DIGIT;
	unsigned int hr, min, sec, fs;

	if (check_special_dn(rgx, res)) {
		if (decNumberIsInfinite(rgx))
			res = set_dig_s(exp_last, 'o', res);
		return;
	}

	decNumberMod(&x, rgx, &const_9000);
	dn_abs(&a, rgx);
	if (decNumberIsNegative(&x)) {
		if (res != NULL)
			*res += '-';
		else
			set_dot(MANT_SIGN);
		dn_minus(&x, &x);
	}

	decNumberHR2HMS(&y, &x);
	dn_mulpow10(&t, &y, 6);
	decNumberRound(&u, &t);

	hms_step(&t, &u, &fs);
	hms_step(&u, &t, &sec);
	hms_step(&t, &u, &min);
	hr = dn_to_int(&t);
	if (sec >= 60) { sec -= 60; min++;	}
	if (min >= 60) { min -= 60; hr++;	}

	// degrees
	res = hms_render(hr, res, &j, 4, 1);
	res = set_dig_s(j, '@', res);
	j += SEGS_PER_DIGIT;

	// minutes
	res = hms_render(min, res, &j, 2, 1);
	res = set_dig_s(j, '\'', res);
	j += SEGS_PER_DIGIT;

	// seconds
	res = hms_render(sec, res, &j, 2, 1);
	res = set_decimal(j - SEGS_PER_DIGIT, DecimalMode, res);

	// Fractional seconds
	res = hms_render(fs, res, &j, 2, 0);

	// We're now pointing at the exponent's first digit...
	res = set_dig_s(j, '"', res);
	// j += SEGS_PER_EXP_DIGIT;

	// Check for values too big or small
	if (dn_le0(dn_compare(&x, &const_9000, &a))) {
		res = set_dig_s(exp_last, 'o', res);
	} else if (! dn_eq0(&a)) {
		if (decNumberIsNegative(dn_compare(&x, &a, &const_0_0000005))) {
			res = set_dig_s(exp_last, 'u', res);
		}
	}
}


static int set_x_fract(const decNumber *rgx, char *res) {
	decNumber x, w, n, d, t;
	char buf[32], *p = buf;
	int j;

	if (check_special_dn(rgx, res))
		return 1;
	dn_abs(&x, rgx);
	dn_compare(&d, &const_100000, &x);
	if (dn_le0(&d))
		return 0;
	dn_compare(&d, &x, &const_0_0001);
	if (decNumberIsNegative(&d))
		return 0;
	if (decNumberIsNegative(rgx)) {
		if (res != NULL)
			*res += '-';
		else
			set_dot(MANT_SIGN);
	}
	decNumberFrac(&w, &x);
	decNumber2Fraction(&n, &d, &w);	/* Get the number as a numerator & denominator */

	dn_divide(&t, &n, &d);
	dn_compare(&t, &t, &w);
	decNumberTrunc(&w, &x);		/* Extract the whole part */

	if (!UState.improperfrac) {
		if (!dn_eq0(&w)) {
			p = num_arg(p, dn_to_int(&w));
			*p++ = ' ';
		}
	} else {
		dn_multiply(&x, &w, &d);
		dn_add(&n, &n, &x);
	}
	p = num_arg(p, dn_to_int(&n));
	*p++ = '/';
	p = num_arg(p, dn_to_int(&d));
	*p = '\0';
	if ((p - 12) > buf) {
		p -= 12;
		*p = '<';
	} else	p = buf;
	for (j=0; *p != '\0'; p++) {
		set_dig_s(j, *p, res);
		j += SEGS_PER_DIGIT;
	}

	if (dn_eq0(&t))
		p = (char *)S7_fract_EQ;
	else if (decNumberIsNegative(&t))
		p = (char *)S7_fract_LT;
	else
		p = (char *)S7_fract_GT;
	for (j = SEGS_EXP_BASE; *p != '\0'; p++) {
		set_dig_s(j, *p, res);
		j += SEGS_PER_EXP_DIGIT;
	}
	return 1;
}


static void show_x(char *x) {
	int i, j = 0;
	char *p = find_char(x, '\0');

	for (i=0; i<16; i++)
		p[i] = '0';

#if 0
	// 1 + 12 + 3 version
	for (i=1; i<=12; i++) {
		set_dig_s(j, x[i], NULL);
		j += SEGS_PER_DIGIT;
	}
	for (i=13; i<=15; i++) {
		set_dig_s(j, x[i], NULL);
		j += SEGS_PER_EXP_DIGIT;
	}
	x[1] = '\0';
#else
	// 4 + 12 version
	for (i=4; i<=15; i++) {
		set_dig_s(j, x[i], NULL);
		j += SEGS_PER_DIGIT;
	}
	x[4] = '\0';
#endif
	set_status(x);
}


enum display_modes std_round_fix(const decNumber *z) {
	decNumber b, c;

	dn_1(&b);
	b.exponent -= UState.dispdigs;
	dn_abs(&c, z);
	dn_compare(&b, &c, &b);
	dn_compare(&c, &const_1, &c);
	if (dn_gt0(&b) && dn_gt0(&c))
		return MODE_FIX;
	return MODE_STD;
}


/* Display the X register in the numeric portion of the display.
 * We have to account for the various display modes and numbers of
 * digits.
 */
#ifdef INCLUDE_DOUBLE_PRECISION
static void set_x(const REGISTER *rgx, char *res, int dbl) {
#else
#define set_x(rgx, res, dbl) set_x_(rgx, res)
static void set_x_(const REGISTER *rgx, char *res) {
#endif
	char x[50], *obp = x;
	int odig = 0;
	int show_exp = 0;
	int j;
#ifdef INCLUDE_DOUBLE_PRECISION
	char mantissa[64];
#else
	char mantissa[32];
#endif
	int exp;
	char *p = mantissa;
	char *r;
	const char *q;
	int count, i;
	int extra_digits = 0;
	int dd = UState.dispdigs;
	int mode = UState.dispmode;
	char c;
	int negative = 0;
	decNumber z;
	int trimzeros = 0;

#ifdef INCLUDE_DOUBLE_PRECISION
	if (dbl)
		decimal128ToNumber(&(rgx->d), &z);
	else
		decimal64ToNumber(&(rgx->s), &z);
#else
	decimal64ToNumber(rgx, &z);
#endif
	if (!State2.smode && ! State2.cmplx) {
		if (State2.hms) {
			set_x_hms(&z, res);
			State2.hms = 0;
			return;
		} else if (UState.fract) {
			if (set_x_fract(&z, res))
				return;
		}
	}
	if (check_special_dn(&z, res))
		return;

	if (State2.smode == SDISP_SHOW) {
		dn_abs(&z, &z);
		decNumberNormalize(&z, &z, &Ctx);
		z.exponent = 0;
	}

	xset(x, '\0', sizeof(x));

	if (dn_eq0(&z)) {
		if (decNumberIsNegative(&z) && get_user_flag(NAN_FLAG)) {
			x[0] = '-';
			x[1] = '0';
		} else
			x[0] = '0';
	} else
		decNumberToString(&z, x);

	if (State2.smode == SDISP_SHOW) {
		show_x(x);
		return;
	}

	if (mode == MODE_STD) {
		mode = std_round_fix(&z);
		if (mode == MODE_FIX)
			trimzeros = 1;
		dd = DISPLAY_DIGITS - 1;
	}

	xset(mantissa, '0', sizeof(mantissa)-1);
	mantissa[sizeof(mantissa)-1] = '\0';

	q = find_char(x, 'E');
	if (q == NULL) exp = 0;
	else exp = s_to_i(q+1);

	// Skip leading spaces and zeros.  Also grab the sign if it is there
	for (q=x; *q == ' '; q++);
	if (!res) {
		clr_dot(EXP_SIGN);
		clr_dot(MANT_SIGN);
	}
	if (*q == '-') {
		negative = 1;
		q++;
	} else if (*q == '+')
		q++;
	for (; *q == '0'; q++);
	if (*q == '.') {
		do
			exp--;
		while (*++q == '0');
		while (*q >= '0' && *q <= '9')
			*p++ = *q++;
	} else {
		if (*q >= '0' && *q <= '9')
			*p++ = *q++;
		while (*q >= '0' && *q <= '9') {
			*p++ = *q++;
			exp++;
		}
		if (*q == '.') {
			q++;
			while (*q >= '0' && *q <= '9')
				*p++ = *q++;
		}
	}

	if (mode == MODE_FIX) {
		if (exp > (DISPLAY_DIGITS - 1) || exp < -dd)
			mode = UState.fixeng?MODE_ENG:MODE_SCI;
		else {
			extra_digits = exp;
			/* We might have push the fixed decimals off the
			 * screen so adjust if so.
			 */
			if (extra_digits + dd > (DISPLAY_DIGITS - 1))
				dd = (DISPLAY_DIGITS - 1) - extra_digits;
		}
	}

	// Round the mantissa to the number of digits desired
	p = mantissa + dd + extra_digits + 1;
	if (*p >= '5') {	// Round up
		*p = '0';
		for (r = mantissa; *r == '9'; r++);
		if (r == p) {   // Special case 9.9999999
			for (r = mantissa; *r == '9'; *r++ = '0');
			mantissa[0] = '1';
			exp++;
			if (mode == MODE_FIX && exp > (DISPLAY_DIGITS - 1)) {
				mode = UState.fixeng?MODE_ENG:MODE_SCI;
				extra_digits = 0;
			}
		} else {
			while (*--p == '9')
				*p = '0';
			(*p)++;
		}
	}

	// Zap what is left
	for (p = mantissa + dd + extra_digits + 1; *p != '\0'; *p++ = '0');

	p = mantissa;
	switch (mode) {
	default:
	case MODE_STD:   
		for (count = DISPLAY_DIGITS; mantissa[count] == '0'; count--);
		if (count != DISPLAY_DIGITS)
			count++;
		// Too big or too small to fit on display
		if (exp >= DISPLAY_DIGITS || exp < (count - DISPLAY_DIGITS)) {
			switch ((exp % 3) * UState.fixeng) {
			case -1:
			case 2:
				*obp++ = *p++;
				odig++;
				dd--;
				exp--;
			case -2:
			case 1:
				*obp++ = *p++;
				odig++;
				dd--;
				exp--;
			case 0:
				;
			};
			*obp++ = *p++;
			odig++;
			*obp++ = '.';
			for (i=1; i<count; i++) {
				*obp++ = *p++;
				odig++;
			}
			show_exp = 1;
		} else if (exp >= 0) {  // Some digits to left of decimal point
			for(i=0; i<=exp; i++) {
				if (i > 0 && (exp - i) % 3 == 2)
					*obp++ = ',';
				*obp++ = *p++;
				odig++;
			}
			*obp++ = '.';
			if (count > (exp + 1)) {
				for (i=exp+1; i<count; i++) {
					*obp++ = *p++;
					odig++;
				}
			}
		} else {		// All digits to right of decimal point
			*obp++ = '0';
			odig++;
			*obp++ = '.';
			for (i=exp+1; i<0; i++) {
				*obp++ = '0';
				odig++;
			}
			for (i=0; i<count; i++) {
				*obp++ = *p++;
				odig++;
			}
		}
		break;

	case MODE_FIX:
		j = 0;
		if (exp >= 0) {		// Some digits to left of decimal point
			for (i=0; i<=exp; i++) {
				if (i > 0 && (exp - i) % 3 == 2)
					*obp++ = ',';
				*obp++ = *p++;
				odig++;
			}
			*obp++ = '.';
			for (i=0; i<dd && j < SEGS_EXP_BASE; i++) {
				*obp++ = *p++;
				odig++;
			}
		} else {		// All digits to right of decimal point
			*obp++ = '0';
			odig++;
			*obp++ = '.';
			for (i=exp+1; i<0; i++) {
				*obp++ = '0';
				odig++;
				dd--;
			}
			while (dd-- > 0) {
				*obp++ = *p++;
				odig++;
			}
		}
		if (trimzeros)
			while (obp > x && obp[-1] == '0') {
				obp--;
				odig--;
			}
		break;

	case MODE_ENG:
		switch (exp % 3) {
		case -1:
		case 2:
			*obp++ = *p++;
			odig++;
			dd--;
			exp--;
		case -2:
		case 1:
			*obp++ = *p++;
			odig++;
			dd--;
			exp--;
		case 0:
			;
		};
	// Falling through

	case MODE_SCI:
		*obp++ = *p++;
		odig++;
		*obp++ = '.';
		dd--;
		while (dd-- >= 0) {
			*obp++ = *p++;
			odig++;
		}
		show_exp = 1;
	}

	/* Finally, send the output to the display */
	*obp = '\0';
	if (odig > DISPLAY_DIGITS)
		odig = DISPLAY_DIGITS;
	j = (DISPLAY_DIGITS - odig) * SEGS_PER_DIGIT;
	if (negative) {
		if (res) *res++ = '-';
		else {
			if (j == 0)
				set_dot(MANT_SIGN);
			else
				set_dig(j - SEGS_PER_DIGIT, '-');
		}
	}
	for (i=0; (c = x[i]) != '\0' && j < SEGS_EXP_BASE; i++) {
		if (c == '.') {
			res = set_decimal(j - SEGS_PER_DIGIT, DecimalMode, res);
		} else if (c == ',') {
			set_separator(j, SeparatorMode, res);
		} else {
			res = set_dig_s(j, c, res);
			j += SEGS_PER_DIGIT;
		}
	}
	if (show_exp)
		set_exp(exp, 0, res);
	if (obp[-1] == '.')
		set_decimal((DISPLAY_DIGITS - 1) * SEGS_PER_DIGIT, DecimalMode, res);
}


void format_reg(int index, char *buf) {
	const REGISTER *const r = get_reg_n(index);

	if (is_intmode())
		set_int_x(get_reg_n_int(index), buf);
#ifndef HP16C_MODE_CHANGE
	else if (buf == NULL && State2.smode > SDISP_SHOW) {
		decNumber x;
		int s;
		unsigned long long int v;

		getRegister(&x, index);
		decNumberTrunc(&x, &x);
		v = dn_to_ull(&x, &s);
		set_int_x(build_value(v, s), NULL);
	}
#endif
	else
		set_x(r, buf, State.mode_double);
}

/* Display the status screen */
static void show_status(void) {
	int i, n;
	int j = SEGS_EXP_BASE;
	const int status = State2.status - 3;
	char buf[16], *p = buf;
	unsigned int pc;

	if (status == -2) {
		set_status("Steps:");
		p = num_arg(buf, free_mem());
		p = scopy(p, " , FL. ");
		p = num_arg(p, free_flash());
		*p = '\0';
		set_digits_string(buf, 0);
	}
	else if (status == -1) {
		const int l = local_regs();
		set_status("Regs:");
		p = num_arg(buf, global_regs());
		if (l) {
			p = scopy(p, " , Loc. ");
			p = num_arg(p, l);
		}
		*p = '\0';
		set_digits_string(buf, 0);
	} else {
		int base;
		int end;
		int group = 10;
		int start = 0;
		
		if (status <= 9) {
			base = 10 * status;
			end = base >= 70 ? 99 : base + 29;
			p = scopy(buf, "FL ");
			p = num_arg_0(p, base, 2);
			*p++ = '-';
			p = num_arg_0(p, end, 2);
			*p = '\0';
			set_status(buf);
		}
		else if (status == 10) {
			base = regX_idx;
			end = regK_idx;
			start = 3;
			group = 4;
			set_status("XYZT\006A:D\006LIJK");
		}
		else { // status == 11
			base = LOCAL_FLAG_BASE;
			end = LOCAL_FLAG_BASE + 15;
			set_status("FL.00-.15");
		}
		j = start * SEGS_PER_DIGIT;
		set_decimal(j, DECIMAL_DOT, NULL);
		j += SEGS_PER_DIGIT;
		for (i = start; i < group + start; i++) {
			int k = i + base - start;
			int l = get_user_flag(k);
			k += group;
			if (end >= k) {
				l |= (get_user_flag(k) << 1);
				k += group;
				if (end >= k)
					l |= (get_user_flag(k) << 2);
			}
			set_dig(j, l);
			set_decimal(j, DECIMAL_DOT, NULL);
			j += SEGS_PER_DIGIT;
			if (i == 4) {
				set_dig(j, 8);
				set_decimal(j, DECIMAL_DOT, NULL);
				j += SEGS_PER_DIGIT;
			}
		}
	}

	j = SEGS_EXP_BASE;
	pc = state_pc();
	if (isXROM(pc))
		pc = 1;
	for (n=i=0; i<4; i++) {
		if (find_label_from(pc, 100+i, FIND_OP_ENDS)) {
			if (++n == 4) {
				set_dig(SEGS_EXP_BASE + SEGS_PER_EXP_DIGIT, 'L');
				set_dig(SEGS_EXP_BASE + 2*SEGS_PER_EXP_DIGIT, 'L');
			} else {
				set_dig(j, 'A'+i);
				j += SEGS_PER_EXP_DIGIT;
			}
		}
	}
}


/* Display the list of alpha labels */
static void show_label(void) {
	char buf[15];
	unsigned short int pc = State2.digval;
	unsigned int op = getprog(pc);
	const int n = nLIB(pc);

	set_status(prt((opcode)op, buf));
	set_digits_string(libname[n], 0);
}

/* Display a list of register contents */
static void show_registers(void) {
	char buf[16], *bp;
	const int n = State2.digval;
	
	const int reg = State2.digval2 ? FLASH_REG_BASE + n : 
			State2.local   ? LOCAL_REG_BASE + n : 
			n;

	if (State2.disp_as_alpha) {
		set_status(alpha_rcl_s(reg, buf));
	}
	else {
		xset(buf, '\0', 16);
		bp = scopy_spc(buf, State2.digval2 ? "Bkup" : "Reg ");
		if (State2.local)
			*bp++ = '.';
		if (n < 100)
			num_arg_0(bp, n, 2);
		else
			*bp++ = REGNAMES[n - regX_idx];
		set_status(buf);
	}
	format_reg(reg, NULL);
}


static void set_annunciators(void)
{
	const enum trig_modes tm = get_trig_mode();

	/* Turn INPUT on for alpha mode.  Turn down arrow on if we're
	 * typing lower case in alpha mode.  Turn the big equals if we're
	 * browsing constants.
	 */
	dot(BEG, state_pc() <= 1);
	dot(INPUT, State2.catalogue || State2.alphas || State2.confirm);
	dot(DOWN_ARR, (State2.alphas || State2.multi) && State2.alphashift);
	dot(BIG_EQ, get_user_flag(A_FLAG));

	/* Set the trig mode indicator 360 or RAD.  Grad is handled elsewhere.
	 */
	dot(DEG, !is_intmode() && tm == TRIG_DEG);
	dot(RAD, !is_intmode() && tm == TRIG_RAD);

	/* Show the RPN indicator only if display shows X in the normal mode
	 */
	dot(RPN, ShowRPN >= 1 && ! Running);
}

/*
 *  Update the display
 */
void display(void) {
	int i, j;
	char buf[40], *bp = buf;
	const char *p;
	int annuc = 0;
	const enum catalogues cata = (enum catalogues) State2.catalogue;
	int skip = 0;
	int x_disp = 0;

	if (State2.disp_freeze) {
		State2.disp_freeze = 0;
		ShowRPN = 0;
		State2.disp_temp = 1;
#ifdef CONSOLE
	        JustDisplayed = 1;
#endif
		return;
	}

	// Separators used by various modes
	if (UState.fraccomma) {
		SeparatorMode = SEP_DOT;
		DecimalMode = DECIMAL_COMMA;
	}
	else {
		SeparatorMode = SEP_COMMA;
		DecimalMode = DECIMAL_DOT;
	}
	if ((UState.intm && UState.nointseparator) || (!UState.intm && UState.nothousands))
		SeparatorMode = SEP_NONE;

	// Clear display
	reset_disp();

	xset(buf, '\0', sizeof(buf));
	if (State2.cmplx  && !cata) {
		*bp++ = COMPLEX_PREFIX;
		set_status(buf);
	}
	if (State2.version) {
		char vers[] = "34S\006" VERSION_STRING "\006????";
		set_digits_string("pAULI, WwALtE", 0);
		set_dig_s(SEGS_EXP_BASE, 'r', NULL);
		xcopy( vers + sizeof(vers) - 5, SvnRevision, 4 );
		set_status(vers);
		skip = 1;
		goto nostk;
	} else if (State2.confirm) {
		set_status(S_SURE);
	} else if (State2.hyp) {
		bp = scopy(bp, "HYP");
		if (! State2.dot)
			*bp++ = '\235';
		set_status(buf);
	} else if (State2.gtodot) {
		const int n = 3 + (nLIB(state_pc()) & 1); // Number of digits to display/expect
		bp = scopy_char(bp, argcmds[RARG_GTO].cmd, '.');
		if (State2.numdigit > 0)
			bp = num_arg_0(bp, (unsigned int)State2.digval, (int)State2.numdigit);
		for (i=State2.numdigit; i<n; i++)
			*bp++ = '_';
		set_status(buf);
	} else if (State2.rarg) {
		/* Commands with arguments */
		bp = scopy_char(bp, argcmds[CmdBase].cmd, State2.ind?'\015':' ');
		if (State2.dot) {
			*bp++ = 's';
			*bp++ = '_';
		} else if (cur_shift() == SHIFT_F) {
			*bp++ = '\021';
			*bp++ = '_';
		} else {
			const int maxdigits = State2.ind ? 2 : num_arg_digits(CmdBase);
			if (State2.local)
				*bp++ = '.';
			if (State2.numdigit > 0)
				bp = num_arg_0(bp, (unsigned int)State2.digval, (int)State2.numdigit);
			for (i=State2.numdigit; i<maxdigits; i++)
				*bp++ = '_';
		}
		set_status(buf);
	} else if (State2.test != TST_NONE) {
		*bp++ = 'x';
		*bp++ = "=\013\035<\011>\012"[State2.test];
		*bp++ = '_';
		*bp++ = '?';
		set_status(buf);
	} else if (cata) {
		const opcode op = current_catalogue(State.catpos);
		char b2[16];
		const char *p;

		bp = scopy(bp, "\177\006\006");
		p = catcmd(op, b2);
		if (*p != COMPLEX_PREFIX && State2.cmplx)
			*bp++ = COMPLEX_PREFIX;
		bp = scopy(bp, p);
		set_status(buf);
		if (cata == CATALOGUE_CONST || cata == CATALOGUE_COMPLEX_CONST) {
			set_x((REGISTER *) &CONSTANT(op & RARG_MASK), NULL, 0);
			skip = 1;
		} else if (State2.runmode) {
			if (cata == CATALOGUE_CONV) {
				decNumber x, r;
				decimal64 z;

				getX(&x);
				if (opKIND(op) == KIND_MON) {
					const unsigned int f = argKIND(op);
					if (f < num_monfuncs && ! isNULL(monfuncs[f].mondreal)) {
						FP_MONADIC_REAL fp = (FP_MONADIC_REAL) EXPAND_ADDRESS(monfuncs[f].mondreal);
						update_speed(0);
						fp(&r, &x);
					}
					else
						set_NaN(&r);
				} else
					do_conv(&r, op & RARG_MASK, &x);
				decNumberNormalize(&r, &r, &Ctx);
				packed_from_number(&z, &r);
				set_x((REGISTER *)&z, NULL, 0);
				skip = 1;
			} else if (op >= (OP_NIL | OP_sigmaX2Y) && op < (OP_NIL | OP_sigmaX2Y) + NUMSTATREG) {
				REGISTER z, *const x = get_reg_n(regX_idx);
				copyreg(&z, x);
				sigma_val((enum nilop) argKIND(op));
				set_x(x, NULL, 0);
				copyreg(x, &z);
				skip = 1;
			}
		}
	} else if (State2.multi) {
		bp = scopy_char(bp, multicmds[CmdBase].cmd, '\'');
		if (State2.numdigit > 0) {
			*bp++ = (char) State2.digval;
			if (State2.numdigit > 1)
				*bp++ = State2.digval2;
		}
		set_status(buf);
	} else if (State2.status) {
		show_status();
		skip = 1;
	} else if (State2.labellist) {
		show_label();
		skip = 1;
	} else if (State2.registerlist) {
		show_registers();
		skip = 1;
#ifdef SHIFT_HOLD_TEMPVIEW
	} else if (State2.disp_as_alpha) {
		set_status(alpha_rcl_s(regX_idx, buf));
#endif
	} else if (State2.runmode) {
		if (DispMsg) {
			set_status(DispMsg);
		} else if (State2.alphas) {
#if 0
			set_digits_string("AlpHA", 0);
#endif
			bp = scopy(buf, Alpha);
			j = State2.alpha_pos;
			if (j != 0) {
				i = slen(buf);
				j *= 6;
				if ( i - j >= 12 ) {
					buf[ (i - j) ] = '\0';
					set_status_right(buf);
				}
				else {
					set_status(buf);
				}
			} else {
				i = cur_shift();
				if (i != SHIFT_N) {
					*bp++ = 021 + i - SHIFT_F;
					*bp++ = '\0';
				}
				set_status_right(buf);
			}
		} else if (State2.smode != SDISP_SHOW) {
			annuc = 1;
		}
	} else {
		show_progtrace(buf);
		i = state_pc();
		if (i > 0)
			set_status(prt(getprog(i), buf));
		else
			set_status("");
		set_dot(STO_annun);
		if (State2.smode == SDISP_SHOW) {
			unsigned short int crc;
			crc = checksum_program();
			j = SEGS_PER_DIGIT * 0;
			for (i=0; i<4; i++) {
				set_dig(j, "0123456789ABCDEF"[crc & 0xf]);
				crc >>= 4;
				j += SEGS_PER_DIGIT;
			}
			skip = 1;
		}
		else if (cur_shift() != SHIFT_N || State2.cmplx || State2.arrow)
			annuc = 1;
		goto nostk;
	}
	show_stack();
nostk:	show_flags();
	if (!skip) {
		if (State2.runmode) {
			p = get_cmdline();
			if (p == NULL || cata) {
				if (ShowRegister != -1) {
					x_disp = (ShowRegister == regX_idx) && !State2.hms;
					format_reg(ShowRegister, NULL);
				}
				else
					set_digits_string(" ---", 4 * SEGS_PER_DIGIT);
			} else {
				disp_x(p);
				x_disp = 1;
			}
		} else {
			unsigned int upc = user_pc();
			unsigned int pc = state_pc();
			const int n = nLIB(pc);
			xset(buf, '\0', sizeof(buf));
			set_exp(ProgFree, 1, NULL);
			num_arg_0(scopy_spc(buf, n == 0 ? S7_STEP : libname[n]), 
				  upc, 3 + (n & 1));  // 4 digits in ROM and Library
			set_digits_string(buf, SEGS_PER_DIGIT);
		}
	}
	if (x_disp == 0 || State2.smode != SDISP_NORMAL || DispMsg != NULL || State2.disp_as_alpha)
		ShowRPN = 0;
	set_annunciators();

	State2.disp_temp = (ShowRPN == 0 && State2.runmode 
		            && (! State2.registerlist || State2.disp_as_alpha));
	if (annuc && !State2.disp_temp)
		annunciators();
	State2.version = 0;
	State2.disp_as_alpha = 0;
	State2.smode = SDISP_NORMAL;
	State2.invalid_disp = 0;
	ShowRegister = regX_idx;
	DispMsg = NULL;
	State2.disp_small = 0;
	finish_display();
#ifdef CONSOLE
        JustDisplayed = 1;
#endif
}

/*
 *  Frozen display will revert to normal only after another call to display();
 */
void frozen_display()
{
	State2.disp_freeze = 0;
	ShowRPN = 0;
	display();
	State2.disp_freeze = 1;
}

/* Take the given string and display as much of it as possible on the top
 * line of the display.  The font size is set by the smallp parameter.
 * We allow character to go one pixel beyond the display since the rightmost
 * column is almost always blank.
 */
static void set_status_sized(const char *str, int smallp) {
	unsigned int x = 0;
	int i, j;
	const unsigned short szmask = smallp?0x100:0;
#ifndef CONSOLE
	unsigned long long mat[6];

	xset(mat, 0, sizeof(mat));
#endif

	while (*str != '\0' && x <= BITMAP_WIDTH+1)  {
		const unsigned char ch = *str++;
		const unsigned short c = ch | szmask;
		//const unsigned char *cmap;
		int width;
		unsigned char cmap[6];

		//cmap = &charset[c][0];
		width = charlengths(c);
		if (x + width > BITMAP_WIDTH+1)
			break;

		/* Decode the packed character bytes */
		unpack6(chars[c], cmap);
		for (i=0; i<6; i++)
			for (j=0; j<width; j++) {
				if (x+j >= BITMAP_WIDTH)
					break;
#ifndef CONSOLE
				if (cmap[i] & (1 << j))
					mat[i] |= 1LL << (x+j);
#else
				dot((x+j)*6+i+MATRIX_BASE, (cmap[i] & (1 << j))?1:0);
#endif
			}
		x += width;
	}
#ifndef CONSOLE
	set_status_grob(mat);
#else
	for (i=MATRIX_BASE + 6*x; i<400; i++)
		clr_dot(i);
#endif
}


/* Determine the pixel length of the string if it were displayed.
 * If this is larger than the display, return true.
 */
static int string_too_large(const char *s) {
	int x = 0;
	while (*s != '\0') {
		const unsigned char c = *s++;
		x += charlengths(c);
	}
	return x > BITMAP_WIDTH+1;
}


/* Display the given string on the screen.
 */
static void set_status(const char *str) {
	set_status_sized(str, State2.disp_small || string_too_large(str));
}


/*
 *  Display messages (global function)
 */
extern void message(const char *str1, const char *str2)
{
	State2.disp_freeze = 0;
	if ( State2.invalid_disp && str2 == NULL ) {
		// Complete redraw necessary
		DispMsg = str1;
		display();
	}
	else {
		if ( str2 != NULL ) {
			reset_disp();
			set_annunciators();
			set_digits_string( str2, 0 );
		}
		set_status( str1 );
		finish_display();
	}
}

#ifdef INCLUDE_STOPWATCH

static void stopwatch_exponent(const char* exponent) {
	int j = SEGS_EXP_BASE;
	for (; *exponent!=0; exponent++) {
		set_dig_s(j, *exponent, NULL);
		j += SEGS_PER_EXP_DIGIT;
	}
}

void stopwatch_message(const char *str1, const char *str2, int dot_pos, char* exponent)
{
	reset_disp();
	set_annunciators();
	set_digits_string( str2, 0 );
	if( dot_pos>=0 ) {
		set_decimal( dot_pos*SEGS_PER_DIGIT, SEP_DOT, NULL );
	}
	if( exponent!=NULL ) {
		stopwatch_exponent(exponent);
	}
	set_status( str1 );
	finish_display();
}


#endif // INCLUDE_STOPWATCH

/* Display the right hand characters from the given string.
 * Trying to fit as many as possible into the bitmap area,
 * and reduce font size if required.
 */
static void set_status_right(const char *str) {
	unsigned int x = 0;
	const char *p;
	const int toolarge = State2.disp_small || string_too_large(str);
	const unsigned short szmask = toolarge ? 0x100 : 0;

	for (p=str; *p != '\0'; p++);
	while (--p >= str) {
		const unsigned char ch = *p;
		const unsigned short c = ch | szmask;

		x += charlengths(c);
		if (x > BITMAP_WIDTH+1)
			break;
	}
	set_status_sized(p+1, toolarge);
}



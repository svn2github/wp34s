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
#include "display.h"
#include "lcd.h"
#include "int.h"
#include "consts.h"
#include "alpha.h"
#include "decn.h"

enum seperator_modes { SEP_NONE, SEP_COMMA, SEP_DOT };
enum decimal_modes { DECIMAL_DOT, DECIMAL_COMMA, DECIMAL_DASH };

static void set_status_sized(const char *, int);
static void set_status(const char *);
static void set_status_right(const char *);


/* Message strings
 * Strings starting S7_ are for the lower 7 segment line.  Strings starting S_
 * are for the upper dot matrix line.
 */
static const char S_SURE[] = "Sure?";

static const char S7_ERROR[] = "Error";		/* Default lower line error display */
static const char S7_NaN[] = "NOt NUMmerIc";	/* Displaying NaN in lower line */
static const char S7_INF[] = "infinity";	/* Displaying infinity in lower line */

static const char S7_STEP[] = " Step ";		/* Step marker in program mode (lower line) */

static const char S7_fract_EQ[] = " = ";	/* Exponent in fraction mode indicates low, equal or high */
static const char S7_fract_LT[] = " Lt";
static const char S7_fract_GT[] = " Gt";


/* Table of error messages.
 * These consist of a double string.  The first is displayed in the
 * top line, the second in the bottom.  If the second is empty, "Error"
 * is displayed instead.  To get a blank lower line, include a space.
 */

// NB: this MUST be in the same order as `enum errors'
static const char *const error_table[] = 
{
    // manually get the order correct!
    0, 
    "domain\0",
    "bad date\0",
    "undefined\0Op-CODE",
    "+\237\0",
    "-\237\0",
    "no such\0LABEL",
    "SLV \004 \221 \217\0NEStED",
    "out of range\0",
    "bad digit\0",
    "too long\0",
    ">8\006\006\006levels\0NEStED",
    "stack\0CLASH",	
    "bad mode\0",
    "word size\0too SMmALL",
};



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

static const unsigned char charlengths[512] = {
#define C(len, a, b, c, x, y, z)	len
#include "charset.h"
#undef C
};

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


/* Set a digit in positions [base, base+6] */
static void set_dig(int base, char ch) 
{
    int i;
    unsigned char c = getdig(ch);
    for (i=6; i>=0; i--)
    {
        if (c & (1 << i))
            set_dot(base);
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
		set_dig_s(j, *msg, NULL);
		j += SEGS_PER_DIGIT;
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
	xset(buf, '\0', sizeof(buf));
	if (zerop)
		num_arg_0(buf, exp, 3);
	else
		num_arg(buf, exp);
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
	switch (State.smode) {
	case SDISP_BIN:	b = 2;		break;
	case SDISP_OCT:	b = 8;		break;
	case SDISP_DEC:	b = 10;		break;
	case SDISP_HEX:	b = 16;		break;
	default:	b = State.int_base+1;	break;
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

/* Set the decimal point *after* the indicated digit
 * The marker can be either a comma or a dot depending on the value
 * of decimal.
 */
static char *set_decimal(const int posn, const enum decimal_modes decimal, char *res) {
	if (res) {
		*res++ = (decimal == DECIMAL_DOT)?'.':',';
	} else {
		if (decimal != DECIMAL_DASH)
			set_dot(posn+7);
		if (decimal != DECIMAL_DOT)
			set_dot(posn+8);
	}
	return res;
}

/* Set the digit group seperator *before* the specified digit.
 * This can be nothing, a comma or a dot depending on the state of the
 * sep argument.
 */
static void set_seperator(int posn, const enum seperator_modes sep, char *res) {
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


/* Display the annunicator text line.
 * Care needs to be taken to keep things aligned.
 * Spaces are 5 pixels wide, \006 is a single pixel space.
 */
static void annunicators(void) {
	char buf[42], *p = buf, *q;
	int n;

	/* Set the shift key indicator */
	switch (cur_shift()) {
	default:
	case SHIFT_N:
		if (State.wascomplex) {
			*p++ = 'C';
			State.wascomplex = 0;
		} else
			p = scopy(p, " \006");
		break;
	case SHIFT_F:	p = scopy(p, "\021\006");	break;
	case SHIFT_G:	p = scopy(p, "\022\006");	break;
	case SHIFT_H:	p = scopy(p, "\023\006");	break;
	}

	if (State.cmplx) {
		*p++ = ' ';
		*p++ = COMPLEX_PREFIX;
		goto skip;
	}

	if (State.arrow) {
		*p++ = ' ';
		*p++ = '\015';
		goto skip;
	}

	if (!State.runmode && State.alphas) {
		*p++ = '\240';
		*p++ = ':';
	} else if (!is_intmode()) {
		switch (State.date_mode) {
		case DATE_DMY:	q = "D.MY";	break;
		case DATE_MDY:	q = "M.DY";	break;
		default:	q = "    \006";	break;
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

		if (State.int_maxw > 0) {
			n = 4 + 2 * (5 - State.int_maxw);
			if (*q == '1')
				n += 2;
			if (q[1] == '1')
				n += 2;
			while (n-- > 0)
				*p++ = '\006';

			for (n=State.int_maxw; n>=0; n--)
				*p++ = State.int_window == n ? '|':'\'';
		}

	}

skip:	*p = '\0';
	set_status(buf);
}

static void disp_x(const char *p) {
	int i;
	enum decimal_modes decimal = DECIMAL_DOT;
	enum seperator_modes seperator = SEP_COMMA;
	int gotdot = -1;

	if (State.fraccomma) {
		decimal = DECIMAL_COMMA;
		seperator = SEP_DOT;
	}
	if (State.nothousands)
		seperator = SEP_NONE;

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
					set_decimal(i-SEGS_PER_DIGIT, decimal, NULL);
				else {
					set_dig(i, '0');
					set_decimal(i, decimal, NULL);
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
			if (gotdot <= 0)                    // MvC: was '<', caused crash
				break;
			set_seperator(gotdot, seperator, NULL);
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

static void set_int_x(decimal64 *rgx, char *res) {
	const int ws = word_size();
	unsigned int b;
	const long long int value = d64toInt(rgx);
	long long int vs = value;
	unsigned long long int v;
	char buf[MAX_WORD_SIZE + 1];
	int i, j, k;
	int sign;
	int dig = SEGS_PER_DIGIT * 11;

	switch (State.smode) {
	case SDISP_BIN:	b = 2;		break;
	case SDISP_OCT:	b = 8;		break;
	case SDISP_DEC:	b = 10;		break;
	case SDISP_HEX:	b = 16;		break;
	default:	b = int_base();	break;
	}

	if (!res) {
		State.int_maxw = 0;
		carry_overflow();
	}

	if (b == 10) {
		v = extract_value(value, &sign);
		if (int_mode() == MODE_2COMP && sign == 1 && v == 0)
			v = value;
		if (sign) {
			if (res) *res++ = '-';
			else	set_dot(MANT_SIGN);
		}
		if (v == 0) {
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

		if (!State.leadzero && vs == 0) {
			set_dig_s(dig, '0', res);
			return;
		} else if (!State.leadzero) {
			v = (unsigned long long int)vs;
			for (i=0; v != 0; i++) {
				const int r = v % b;
				v /= b;
				buf[i] = DIGITS[r];
			}
		} else {
			int n;
			v = (unsigned long long int)vs;

			if (b == 2)         n = ws;
			else if (b == 8)    n = (ws + 2) / 3;
			else                n = (ws + 3) / 4;
			for (i=0; i<n; i++) {
				const int r = v % b;
				v /= b;
				buf[i] = DIGITS[r];
			}
		}
	}

	/* At this point i is the number of digits in the output */
	if (res) {
		while (--i >= 0)
			*res++ = buf[i];
	} else {
		const int window = State.int_window;
		State.int_maxw = i / 12;
		buf[i] = '\0';

		j = window * 12;	// 12 digits at a time
		for (k=0; k<12; k++)
			if (buf[j+k] == '\0')
				break;
		while (--k >= 0) {
			set_dig(dig, buf[j++]);
			dig -= SEGS_PER_DIGIT;
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

static int check_special_x(const decimal64 *rgx, char *res) {
	decNumber x;

	decimal64ToNumber(rgx, &x);
	return check_special_dn(&x, res);
}


/* Perform a single step of the H.MS display.
 * Multiply the number by mult (optional).
 * Store the fractional part of this into res (optional).
 * Divide this by div (optional).
 * Format and display.
 */
static char *hms_process(decNumber *res, const decNumber *x, const decNumber *mult,
				char *str, int *jin, int n, int spaces) {
	decNumber r, s;
	char b[32];
	int i, j;
	unsigned int v;

	if (mult != NULL)
		decNumberMultiply(&r, x, mult, Ctx);
	else	decNumberCopy(&r, x);
	if (res != NULL) {
		// Rounding step
		decContext c = *Ctx;
		c.round = DEC_ROUND_HALF_UP;
		c.digits = 6;

		decNumberFrac(res, &r, &c);
		decNumberTrunc(&s, &r, Ctx);
	} else
		decNumberRound(&s, &r, Ctx);

	v = dn_to_int(&s, Ctx);

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
static void set_x_hms(const decimal64 *rgx, char *res, const enum decimal_modes decimal) {
	decNumber x, y, a;
	int j=0;
	const int exp_last = SEGS_EXP_BASE + 2*SEGS_PER_EXP_DIGIT;

	decimal64ToNumber(rgx, &y);
	if (check_special_dn(&y, res)) {
		if (decNumberIsInfinite(&y))
			res = set_dig_s(exp_last, 'o', res);
		return;
	}

	decNumberRemainder(&x, &y, &const_9000, Ctx);
	decNumberAbs(&a, &y, Ctx);
	if (decNumberIsNegative(&x)) {
		if (res != NULL)
			*res += '-';
		else
			set_dot(MANT_SIGN);
		decNumberMinus(&x, &x, Ctx);
	}

	decNumberHR2HMS(&y, &x, Ctx);
	
	// degrees
	res = hms_process(&x, &y, NULL, res, &j, 4, 1);
	res = set_dig_s(j, '@', res);
	j += SEGS_PER_DIGIT;

	// minutes
	res = hms_process(&y, &x, &const_100, res, &j, 2, 1);
	res = set_dig_s(j, '\'', res);
	j += SEGS_PER_DIGIT;

	// seconds
	res = hms_process(&x, &y, &const_100, res, &j, 2, 1);
	res = set_decimal(j - SEGS_PER_DIGIT, decimal, res);

	// Fractional seconds
	res = hms_process(NULL, &x, &const_100, res, &j, 2, 0);

	// We're now pointing at the exponent's first digit...
	res = set_dig_s(j, '"', res);
	// j += SEGS_PER_EXP_DIGIT;

	// Check for values too big or small
	decNumberCompare(&x, &const_9000, &a, Ctx);
	if (decNumberIsNegative(&x) || decNumberIsZero(&x)) {
		res = set_dig_s(exp_last, 'o', res);
	} else {
		decNumberCompare(&x, &a, &const_0_0000005, Ctx);
		if (decNumberIsNegative(&x)) {
			res = set_dig_s(exp_last, 'u', res);
		}
	}
}


static int set_x_fract(const decimal64 *rgx, char *res) {
	decNumber x, w, n, d, t;
	char buf[32], *p = buf;
	int j;

	decimal64ToNumber(rgx, &w);
	if (check_special_dn(&w, res))
		return 1;
	decNumberAbs(&x, &w, Ctx);
	decNumberCompare(&d, &const_100000, &x, Ctx);
	if (decNumberIsNegative(&d) || decNumberIsZero(&d))
		return 0;
	decNumberCompare(&d, &x, &const_0_00001, Ctx);
	if (decNumberIsNegative(&d))
		return 0;
	if (decNumberIsNegative(&w)) {
		if (res != NULL)
			*res += '-';
		else
			set_dot(MANT_SIGN);
	}
	decNumberFrac(&w, &x, Ctx);
	decNumber2Fraction(&n, &d, &w, Ctx);	/* Get the number as a numerator & denominator */

	decNumberDivide(&t, &n, &d, Ctx);
	decNumberCompare(&t, &t, &w, Ctx64);
	decNumberTrunc(&w, &x, Ctx);		/* Extract the whole part */

	if (!State.improperfrac) {
		if (!decNumberIsZero(&w)) {
			p = num_arg(p, dn_to_int(&w, Ctx));
			*p++ = ' ';
		}
	} else {
		decNumberMultiply(&x, &w, &d, Ctx);
		decNumberAdd(&n, &n, &x, Ctx);
	}
	p = num_arg(p, dn_to_int(&n, Ctx));
	*p++ = '/';
	p = num_arg(p, dn_to_int(&d, Ctx));
	*p = '\0';
	if ((p - 12) > buf) {
		p -= 12;
		*p = '<';
	} else	p = buf;
	for (j=0; *p != '\0'; p++) {
		set_dig_s(j, *p, res);
		j += SEGS_PER_DIGIT;
	}

	if (decNumberIsZero(&t))
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


/* Display the X register in the numeric portion of the display.
 * We have to account for the various display modes and numbers of
 * digits.
 */
static void set_x(const decimal64 *rgx, char *res, int nohms) {
	char x[50], *obp = x;
	int odig = 0;
	int show_exp = 0;
	int j;
	char mantissa[32];
	int exp;
	char *p = mantissa;
	char *r;
	const char *q;
	int count, i;
	int extra_digits = 0;
	int dd = State.dispdigs;
	int mode = State.dispmode;
	enum decimal_modes decimal = DECIMAL_DOT;
	enum seperator_modes seperator = SEP_COMMA;
	char c;

	if (State.fraccomma) {
		decimal = DECIMAL_COMMA;
		seperator = SEP_DOT;
	}
	if (State.nothousands)
		seperator = SEP_NONE;

	if (!nohms && !State.smode && ! State.cmplx) {
		if (State.hms) {
			set_x_hms(rgx, res, decimal);
			return;
		} else if (State.fract) {
			if (set_x_fract(rgx, res))
				return;
		}
	}

	if (check_special_x(rgx, res))
		return;

	decimal64ToString(rgx, x);

	if (State.smode == SDISP_SHOW)
		mode = MODE_STD;

	if (mode == MODE_STD)
		dd = DISPLAY_DIGITS - 1;

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
		if (res) *res++ = '-';
		else set_dot(MANT_SIGN);
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
			mode = MODE_SCI;
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
	if (*p >= '5') {    // Round up
		*p = '0';
		for (r = mantissa; *r == '9'; r++);
		if (r == p) {   // Special case 9.9999999
			for (r = mantissa; *r == '9'; *r++ = '0');
			mantissa[0] = '1';
			exp++;
			if (mode == MODE_FIX && exp > (DISPLAY_DIGITS - 1)) {
				mode = MODE_SCI;
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
			if (count > (exp + 1)) {
				*obp++ = '.';
				for (i=exp+1; i<count; i++) {
					*obp++ = *p++;
					odig++;
				}
			}
		} else {                // All digits to right of decimal point
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
		if (exp >= 0) {         // Some digits to left of decimal point
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
		} else {                // All digits to right of decimal point
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
	for (i=0; (c = x[i]) != '\0' && j < SEGS_EXP_BASE; i++) {
		if (c == '.') {
			res = set_decimal(j-SEGS_PER_DIGIT, decimal, res);
		} else if (c == ',') {
			set_seperator(j, seperator, res);
		} else {
			res = set_dig_s(j, c, res);
			j += SEGS_PER_DIGIT;
		}
	}
	if (show_exp)
		set_exp(exp, 0, res);
}


void format_reg(decimal64 *r, char *buf) {
	if (is_intmode())
		set_int_x(r, buf);
	else
		set_x(r, buf, 0);
}

/* Display the status screen */
static void show_status(void) {
	int i;
	int j = SEGS_PER_DIGIT;
	int base = 10 * (State.status - 1);
	char buf[12], *p;

        p = scopy(buf, "FL ");
	p = num_arg_0(p, base, 2);
	*p++ = '-';
	i = base+29>=NUMFLG?NUMFLG-1:base+29;
	if (i < 100)
		p = num_arg_0(p, i, 2);
	else
		*p++ = (i-100) + 'B';
	*p = '\0';
	set_status(buf);
	set_decimal(0, DECIMAL_DOT, NULL);
	for (i=0; i<10; i++) {
		const int k = i + base;
		const int l = get_user_flag(k) + (get_user_flag(k+10)<<1) + (get_user_flag(k+20)<<2);
		set_dig(j, l);
		set_decimal(j, DECIMAL_DOT, NULL);
		j += SEGS_PER_DIGIT;
		if (i == 4) {
			set_dig(j, 8);
			set_decimal(j, DECIMAL_DOT, NULL);
			j += SEGS_PER_DIGIT;
		}
	}
	set_seperator(SEGS_PER_DIGIT * 5, SEP_DOT, NULL);

	j = SEGS_EXP_BASE;
	for (i=0; i<3; i++) {
		set_dig(j, find_label_from(1, 100+i, 1)?('B'+i):'_');
		j += SEGS_PER_EXP_DIGIT;
	}
}


/* Display the X register as alpha */
static void show_alpha(void) {
	char buf[12];

	set_status(alpha_rcl_s(&regX, buf));
	State.arrow_alpha = 0;
}

void display(void) {
	int i, j;
	char buf[32], *bp = buf;
	const char *p;
	int annuc = 0;
	const enum trig_modes tm = get_trig_mode();
	const enum catalogues cata = State.catalogue;
	int skip = 0;

	reset_disp();

	/* Turn INPUT on for alpha mode.  Turn down arrow on if we're
	 * typing lower case in alpha mode.  Turn the big equals if we're
	 * browsing constants.
	 */
	dot(RPN, 1);
	dot(BEG, state_pc() == 0);
	dot(INPUT,  cata || State.alphas || State.confirm);
	dot(DOWN_ARR, (State.alphas || State.multi) && State.alphashift);
	dot(BIG_EQ, cata == CATALOGUE_CONST || cata == CATALOGUE_COMPLEX_CONST);
        dot(LIT_EQ, cata);

	/* Set the trig mode indicator 360 or RAD.  Grad is handled elsewhere.
	 */
	dot(DEG, !is_intmode() && tm == TRIG_DEG);
	dot(RAD, !is_intmode() && tm == TRIG_RAD);

	xset(buf, '\0', sizeof(buf));
	if (State.cmplx) {
		*bp++ = COMPLEX_PREFIX;
		set_status(buf);
	}
	if (State.error != ERR_NONE) {
		const enum errors e = (const enum errors)State.error;
		State.error = 0;
		p = error_table[e];
		set_status(p);
		p = find_char(p, '\0')+1;
		if (*p == '\0')
			p = S7_ERROR;
		set_digits_string(p, 0);
		goto skpall;
	} else if (State.version) {
		set_digits_string("pAULI WwALtE", 0);
		set_dig_s(SEGS_EXP_BASE, 'r', NULL);
		set_decimal(SEGS_PER_DIGIT * 4, DECIMAL_COMMA, NULL);
		set_status("34s " VERSION_STRING);
		goto nostk;
	} else if (State.confirm) {
		set_status(S_SURE);
	} else if (State.hyp) {
		bp = scopy(bp, "HYP");
		if (! State.dot)
			*bp++ = '\235';
		set_status(buf);
	} else if (State.gtodot) {
		bp = scopy_char(bp, argcmds[RARG_GTO].cmd, '.');
		if (State.numdigit > 0)
			bp = num_arg_0(bp, (unsigned int)State.digval, (int)State.numdigit);
		for (i=State.numdigit; i<3; i++)
			*bp++ = '_';
		set_status(buf);
	} else if (State.multi) {
		bp = scopy_char(bp, multicmds[State.base].cmd, '\'');
		if (State.numdigit > 0) {
			*bp++ = State.digval;
			if (State.numdigit > 1)
				*bp++ = State.digval2;
		}
		set_status(buf);
	} else if (State.rarg) {
		/* Commands with arguments */
		bp = scopy_char(bp, argcmds[State.base].cmd, State.ind?'\015':' ');
		if (State.dot) {
			*bp++ = 's';
			*bp++ = '_';
		} else {
			if (State.numdigit > 0)
				bp = num_arg_0(bp, (unsigned int)State.digval, (int)State.numdigit);
			for (i=State.numdigit; i<2; i++)
				*bp++ = '_';
		}
		set_status(buf);
	} else if (State.test != TST_NONE) {
		*bp++ = 'x';
		*bp++ = "=\013\035<\011>\012"[State.test];
		*bp++ = '_';
		*bp++ = '?';
		set_status(buf);
	} else if (cata) {
		const opcode op = current_catalogue(State.digval);
		char b2[16];
		bp = scopy(bp, catcmd(op, b2));
		if (buf[0] == COMPLEX_PREFIX && buf[1] == COMPLEX_PREFIX)
			set_status(buf+1);
		else
			set_status(buf);
		if (cata == CATALOGUE_CONST || cata == CATALOGUE_COMPLEX_CONST) {
			set_x(&CONSTANT(State.digval), NULL, 1);
			skip = 1;
		} else if (cata == CATALOGUE_CONV && State.runmode) {
			decNumber x, r;
			decimal64 z;

			getX(&x);
			if (opKIND(op) == KIND_MON) {
				const unsigned int f = argKIND(op);
				if (f < num_monfuncs && monfuncs[f].mondreal != NULL) {
					(*monfuncs[f].mondreal)(&r, &x, Ctx);
				} else
					set_NaN(&r);
			} else
				do_conv(&r, op & RARG_MASK, &x, Ctx);
			decNumberNormalize(&r, &r, Ctx);
			decimal64FromNumber(&z, &r, Ctx64);
			set_x(&z, NULL, 1);
			skip = 1;
		}
	} else if (State.status) {
		show_status();
		skip = 1;
	} else if (State.arrow_alpha) {
		show_alpha();
	} else if (State.runmode) {
		if (DispMsg) {
			if (State.disp_small) {
				set_status_sized(DispMsg, 1);
				State.disp_small = 0;
			} else
				set_status(DispMsg);
			DispMsg = NULL;
		} else if (State.alphas) {
#if 0
			set_digits_string("AlpHA", 0);
#endif
			set_status_right(Alpha);
		} else {
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
		if (State.smode == SDISP_SHOW) {
			unsigned int crc = checksum_code();
			j = SEGS_PER_DIGIT * 0;
			for (i=0; i<8; i++) {
				set_dig(j, "0123456789ABCDEF"[crc & 0xf]);
				crc >>= 4;
				j += SEGS_PER_DIGIT;
			}
		} else {
			if (isXROM(state_pc())) {
				num_arg_0(scopy_spc(buf, "l1B "), state_pc() - addrXROM(0), 5);
			} else {
				set_exp(NUMPROG + 1 - State.last_prog, 1, NULL);
#if 0
				bp = scopy_spc(buf, State.alphas?"AlpHA":" StEp");
				*bp++ = ' ';
				num_arg_0(bp, state_pc(), 3);
#else
				num_arg_0(scopy_spc(buf, S7_STEP), state_pc(), 3);
#endif
			}
			for (i=0, bp=buf; *bp != '\0'; bp++, i += SEGS_PER_DIGIT)
				set_dig(i, *bp);
		}
		if (cur_shift() != SHIFT_N || State.cmplx || State.arrow)
			annuc = 1;
		goto nostk;
	}
	show_stack();
nostk:	show_flags();
	if (!skip && State.runmode && !State.version) {
		p = get_cmdline();
		if (p == NULL)
			format_reg(&regX, NULL);
		else
			disp_x(p);
	}
	if (annuc)
		annunicators();
skpall:	finish_display();
	State.version = 0;
	State.smode = SDISP_NORMAL;
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
#if defined(REALBUILD) || defined(WINGUI)
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
		width = charlengths[c];
		if (x + width > BITMAP_WIDTH+1)
			return;

		/* Decode the packed character bytes */
		unpack6(chars[c], cmap);
		for (i=0; i<6; i++)
			for (j=0; j<width; j++) {
				if (x+j >= BITMAP_WIDTH)
					break;
#if defined(REALBUILD) || defined(WINGUI)
				if (cmap[i] & (1 << j))
					mat[i] |= 1LL << (x+j);
#else
				dot((x+j)*6+i+MATRIX_BASE, (cmap[i] & (1 << j))?1:0);
#endif
			}
		x += width;
	}
#if defined(REALBUILD) || defined(WINGUI)
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
		x += charlengths[c];
	}
	return x > BITMAP_WIDTH+1;
}


/* Display the given string on the screen.
 */
static void set_status(const char *str) {
	set_status_sized(str, string_too_large(str));
}


/* Display the right hand characters from the given string.
 * Trying to fit as many as possible into the bitmap area,
 * and reduce font size if required.
 */
static void set_status_right(const char *str) {
	unsigned int x = 0;
	const char *p;
	const int toolarge = string_too_large(str);
	const unsigned short szmask = toolarge?0x100:0;

	for (p=str; *p != '\0'; p++);
	while (--p >= str) {
		const unsigned char ch = *p;
		const unsigned short c = ch | szmask;

		x += charlengths[c];
		if (x > BITMAP_WIDTH+1)
			break;
	}
	set_status_sized(p+1, toolarge);
}


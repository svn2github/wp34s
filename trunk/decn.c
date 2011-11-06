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

#include "decn.h"
#include "xeq.h"
#include "consts.h"
#include "complex.h"
#include "stats.h"
#include "int.h"
#include "serial.h"
#include "lcd.h"

// #define DUMP1
#ifdef DUMP1
#include <stdio.h>
static FILE *debugf = NULL;

static void open_debug(void) {
	if (debugf == NULL) {
		debugf = fopen("/dev/ttys001", "w");
	}
}
static void dump1(const decNumber *a, const char *msg) {
	char buf[2000], *b = buf;

	open_debug();
	if (decNumberIsNaN(a)) b= "NaN";
	else if (decNumberIsInfinite(a)) b = decNumberIsNegative(a)?"-inf":"inf";
	else
		decNumberToString(a, b);
	fprintf(debugf, "%s: %s\n", msg ? msg : "???", b);
	fflush(debugf);
}
#else
#define dump1(a,b)
#endif


#define MOD_DIGITS	450		/* Big enough for 1e384 mod small integer */
#define BIGMOD_DIGITS	820		/* Big enough for maxreal mod minreal */


/* Forward declaration */
static void sincosTaylor(const decNumber *a, decNumber *s, decNumber *c);

/* Some basic conditional tests */
int dn_lt0(const decNumber *x) {
	return decNumberIsNegative(x) && ! decNumberIsZero(x);
}
int dn_le0(const decNumber *x) {
	return decNumberIsNegative(x) || decNumberIsZero(x);
}
int dn_gt0(const decNumber *x) {
	return ! decNumberIsNegative(x) && ! decNumberIsZero(x);
}
int dn_eq0(const decNumber *x) {
	return decNumberIsZero(x);
}

/* Some wrapper rountines to save space
 */
decNumber *dn_add(decNumber *r, const decNumber *a, const decNumber *b) {
	return decNumberAdd(r, a, b, &Ctx);
}

decNumber *dn_subtract(decNumber *r, const decNumber *a, const decNumber *b) {
	return decNumberSubtract(r, a, b, &Ctx);
}

decNumber *dn_multiply(decNumber *r, const decNumber *a, const decNumber *b) {
	return decNumberMultiply(r, a, b, &Ctx);
}

decNumber *dn_divide(decNumber *r, const decNumber *a, const decNumber *b) {
	return decNumberDivide(r, a, b, &Ctx);
}

decNumber *dn_compare(decNumber *r, const decNumber *a, const decNumber *b) {
	return decNumberCompare(r, a, b, &Ctx);
}

decNumber *dn_min(decNumber *r, const decNumber *a, const decNumber *b) {
	return decNumberMin(r, a, b, &Ctx);
}

decNumber *dn_max(decNumber *r, const decNumber *a, const decNumber *b) {
	return decNumberMax(r, a, b, &Ctx);
}

decNumber *dn_abs(decNumber *r, const decNumber *a) {
	return decNumberAbs(r, a, &Ctx);
}

decNumber *dn_minus(decNumber *r, const decNumber *a) {
	return decNumberMinus(r, a, &Ctx);
}

decNumber *dn_sqrt(decNumber *r, const decNumber *a) {
	return decNumberSquareRoot(r, a, &Ctx);
}

decNumber *dn_exp(decNumber *r, const decNumber *a) {
	return decNumberExp(r, a, &Ctx);
}


/* Define a table of small integers.
 * This should be equal or larger than any of the summation integers required in the
 * various series approximations to avoid needless computation.
 */
#define MAX_SMALL_INT	9
static const decNumber *const small_ints[MAX_SMALL_INT+1] = {
	&const_0, &const_1, &const_2, &const_3, &const_4,
	&const_5, &const_6, &const_7, &const_8, &const_9,
};

const decNumber *small_int(int i) {
	if (i >= 0 && i<= MAX_SMALL_INT)
		return small_ints[i];
	return NULL;
}

void ullint_to_dn(decNumber *x, unsigned long long int n) {
	/* Check to see if the number is small enough to be in our table */
	if (n <= MAX_SMALL_INT) {
		decNumberCopy(x, small_ints[n]);
	} else {
		/* Got to do this the long way */
		decNumber p10, z;

		dn_1(&p10);
		decNumberZero(x);

		while (n != 0) {
			const int r = n%10;
			n /= 10;
			if (r != 0) {
				dn_multiply(&z, &p10, small_ints[r]);
				dn_add(x, x, &z);
			}
			dn_mulpow10(&p10, &p10, 1);
		}
	}
}

void int_to_dn(decNumber *x, int n) {
	int sgn;

	/* Account for negatives */
	if (n < 0) {
		sgn = 1;
		n = -n;
	} else
		sgn = 0;

	ullint_to_dn(x, n);

	if (sgn)
		dn_minus(x, x);
}

int dn_to_int(const decNumber *x) {
	decNumber y;
	char buf[64];

	decNumberRescale(&y, x, &const_0, &Ctx);
	decNumberToString(&y, buf);
	return s_to_i(buf);
}

unsigned long long int dn_to_ull(const decNumber *x, int *sgn) {
	decNumber y;
	char buf[64];

	decNumberRescale(&y, x, &const_0, &Ctx);
	if (decNumberIsNegative(x)) {
		dn_minus(&y, &y);
		*sgn = 1;
	} else
		*sgn = 0;
	decNumberToString(&y, buf);
	return s_to_ull(buf, 10);
}


decNumber *set_NaN(decNumber *x) {
	if (x != NULL)
		decNumberCopy(x, &const_NaN);
	return x;
}

decNumber *set_inf(decNumber *x) {
	return decNumberCopy(x, &const_inf);
}

decNumber *set_neginf(decNumber *x) {
	return decNumberCopy(x, &const__inf);
}


void decNumberPI(decNumber *pi) {
	decNumberCopy(pi, &const_PI);
}
void decNumberPIon2(decNumber *pion2) {
	decNumberCopy(pion2, &const_PIon2);
}

/* Check if a number is an integer.
 */
int is_int(const decNumber *x) {
	enum rounding a = Ctx.round;
	decNumber r, y;

	if (decNumberIsNaN(x))
		return 0;
	if (decNumberIsInfinite(x))
		return 1;

	Ctx.round = DEC_ROUND_DOWN;
	decNumberToIntegralValue(&y, x, &Ctx);
	Ctx.round = a;

	dn_subtract(&r, x, &y);
	if (! dn_eq0(&r))
		return 0;
	return 1;
}

/* Utility routine that checks if the X register is even or odd or neither.
 * Returns positive if even, zero if odd, -1 for special, -2 for fractional.
 */
int is_even(const decNumber *x) {
	decNumber y, z;

	if (decNumberIsSpecial(x))
		return -1;
	dn_abs(&z, x);
	decNumberMod(&y, &z, &const_2);
	if (dn_eq0(&y))
		return 1;
	dn_compare(&z, &y, &const_1);
	if (dn_eq0(&z))
		return 0;
	return -2;
}

decNumber *dn_inc(decNumber *x) {
	return dn_add(x, x, &const_1);
}

decNumber *dn_dec(decNumber *x) {
	return dn_subtract(x, x, &const_1);
}

decNumber *dn_p1(decNumber *r, const decNumber *x) {
	return dn_add(r, x, &const_1);
}

decNumber *dn_m1(decNumber *r, const decNumber *x) {
	return dn_subtract(r, x, &const_1);
}

decNumber *dn_1m(decNumber *r, const decNumber *x) {
	return dn_subtract(r, &const_1, x);
}

decNumber *dn_1(decNumber *r) {
	return decNumberCopy(r, &const_1);
}

decNumber *dn__1(decNumber *r) {
	return decNumberCopy(r, &const__1);
}

decNumber *dn_p2(decNumber *r, const decNumber *x) {
	return dn_add(r, x, &const_2);
}

decNumber *dn_mul2(decNumber *r, const decNumber *x) {
	return dn_multiply(r, x, &const_2);
}

decNumber *dn_div2(decNumber *r, const decNumber *x) {
	return dn_multiply(r, x, &const_0_5);
}

decNumber *dn_mul100(decNumber *r, const decNumber *x) {
#ifdef DECNUMBER
	return dn_mulpow10(r, x, 2);
#else
	return dn_multiply(r, x, &const_100);
#endif
}

decNumber *dn_mul1000(decNumber *r, const decNumber *x) {
#ifdef DECNUMBER
	return dn_mulpow10(r, x, 3);
#else
	return dn_multiply(r, x, &const_1000);
#endif
}

decNumber *dn_mulPI(decNumber *r, const decNumber *x) {
	return dn_multiply(r, x, &const_PI);
}

decNumber *dn_mulpow10(decNumber *r, const decNumber *x, int p) {
	decNumberCopy(r, x);
	r->exponent += p;
	return r;
}


int relative_error(const decNumber *x, const decNumber *y, const decNumber *tol) {
	decNumber a, b;

	if (dn_eq0(x)) {
		if (dn_eq0(y))
			return 1;
		x = y;
	}
	dn_subtract(&a, x, y);
	dn_divide(&b, &a, x);
	dn_abs(&a, &b);
	return decNumberIsNegative(dn_compare(&a, &a, tol));
}

int absolute_error(const decNumber *x, const decNumber *y, const decNumber *tol) {
	decNumber a, b;

	dn_subtract(&a, x, y);
	dn_abs(&b, &a);
	return decNumberIsNegative(dn_compare(&a, &b, tol));
}


/* Mantissa of a number
 */
#ifdef INCLUDE_MANTISSA
decNumber *decNumberMantissa(decNumber *r, const decNumber *x) {
	if (decNumberIsSpecial(x))
		return set_NaN(r);
	if (dn_eq0(x))
		return decNumberCopy(r, x);
#ifdef DECNUMBER
	decNumberCopy(r, x);
	r->exponent = 1 - r->digits;
	return r;
#else
	{
		decNumber e, p;

		decNumberExponent(&e, x);
		dn_minus(&e, &e);
		decNumberPow10(&p, &e);
		return dn_multiply(r, &p, x);
	}
#endif
}

/* Exponenet of a number
 */
decNumber *decNumberExponent(decNumber *r, const decNumber *x) {
	if (decNumberIsSpecial(x))
		return set_NaN(r);
	if (dn_eq0(x))
		return decNumberZero(r);
#ifdef DECNUMBER
	int_to_dn(r, x->exponent + x->digits - 1);
	return r;
#else
	{
		decNumber z, l;

		dn_abs(&z, x);
		dn_log10(&l, &z);
		return decNumberFloor(r, &l);
	}
#endif
}
#endif

/* Multiply Add: x + y * z
 */
#ifdef INCLUDE_MULADD
decNumber *decNumberMAdd(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x) {
	decNumber t;

	dn_multiply(&t, x, y);
	return dn_add(r, &t, z);
}
#endif


/* Reciprocal of a number.
 * Division is correctly rounded so we just use that instead of coding
 * something special (that could be more efficient).
 */
decNumber *decNumberRecip(decNumber *r, const decNumber *x) {
	return dn_divide(r, &const_1, x);
}

/* Reciprocal of a function's result.
 * This routine calls the specified function and then multiplicatively
 * inverts its result.
 */
#if 0
static decNumber *dn_recip(decNumber *r, const decNumber *x,
		decNumber *(*func)(decNumber *r, const decNumber *x)) {
	decNumber z;

	(*func)(&z, x);
	return decNumberRecip(r, &z);
}
#endif

/* A plethora of round to integer functions to support the large variety
 * of possibilities in this area.  Start with a common utility function
 * that saves the current rounding mode, rounds as required and restores
 * the rounding mode properly.
 */
static decNumber *round2int(decNumber *r, const decNumber *x, int mode) {
	enum rounding a = Ctx.round;

	Ctx.round = mode;
	decNumberToIntegralValue(r, x, &Ctx);
	Ctx.round = a;
	return r;
}

/* Floor - truncate to minus infinity.
 */
decNumber *decNumberFloor(decNumber *r, const decNumber *x) {
	return round2int(r, x, DEC_ROUND_FLOOR);
}

/* Ceiling - truncate to plus infinity.
 */
decNumber *decNumberCeil(decNumber *r, const decNumber *x) {
	return round2int(r, x, DEC_ROUND_CEILING);
}

/* Trunc - truncate to zero.
 */
decNumber *decNumberTrunc(decNumber *r, const decNumber *x) {
	return round2int(r, x, DEC_ROUND_DOWN);
}

/* Round - round 0.5 up.
 */
decNumber *decNumberRound(decNumber *r, const decNumber *x) {
	return round2int(r, x, DEC_ROUND_HALF_UP);
}

/* Intg - round 0.5 even.
 */
static decNumber *decNumberIntg(decNumber *r, const decNumber *x) {
	return round2int(r, x, DEC_ROUND_HALF_EVEN);
}

/* Frac - round 0.5 up.
 */
decNumber *decNumberFrac(decNumber *r, const decNumber *x) {
	decNumber y;

	round2int(&y, x, DEC_ROUND_DOWN);
	return dn_subtract(r, x, &y);
}

decNumber *decNumberSign(decNumber *r, const decNumber *x) {
	const decNumber *z;

	if (decNumberIsNaN(x))
		z = x;
	else if (dn_eq0(x))
		z = &const_0;
	else if (decNumberIsNegative(x))
		z = &const__1;
	else
		z = &const_1;
	return decNumberCopy(r, z);
}


static void dn_gcd(decNumber *r, const decNumber *x, const decNumber *y) {
	decNumber b, t;

	decNumberCopy(&b, y);
	decNumberCopy(r, x);
	while (! dn_eq0(&b)) {
		decNumberCopy(&t, &b);
		decNumberMod(&b, r, &t);
		decNumberCopy(r, &t);
	}
}

static int dn_check_gcd(decNumber *r, const decNumber *x, const decNumber *y,
		int *sign, decNumber *a, decNumber *b) {
	*sign = (decNumberIsNegative(x)?1:0) != (decNumberIsNegative(y)?1:0);
	if (decNumberIsSpecial(x) || decNumberIsSpecial(y)) {
		if (decNumberIsNaN(x) || decNumberIsNaN(y))
			set_NaN(r);
		else if (*sign)
			set_neginf(r);
		else
			set_inf(r);
	} else if (!is_int(x) || !is_int(y))
		set_NaN(r);
	else {
		dn_abs(a, x);
		dn_abs(b, y);
		return 0;
	}
	return 1;
}

decNumber *decNumberGCD(decNumber *r, const decNumber *x, const decNumber *y) {
	decNumber a, b;
	int sign;

	if (dn_check_gcd(r, x, y, &sign, &a, &b))
		return r;

	if(dn_eq0(x))
		decNumberCopy(r, &b);
	else if (dn_eq0(y))
		decNumberCopy(r, &a);
	else
		dn_gcd(r, &a, &b);
	if (sign)
		dn_minus(r, r);
	return r;
}

decNumber *decNumberLCM(decNumber *r, const decNumber *x, const decNumber *y) {
	int sign;
	decNumber gcd, a, b, t;

	if (dn_check_gcd(r, x, y, &sign, &a, &b))
		return r;

	if(dn_eq0(x) || dn_eq0(y))
		decNumberCopy(r, x);
	dn_gcd(&gcd, &a, &b);
	dn_divide(&t, &a, &gcd);
	dn_multiply(r, &t, &b);
	if (sign)
		dn_minus(r, r);
	return r;
}


/* The extra logarithm and power functions */

/* Raise y^x */
decNumber *dn_power(decNumber *r, const decNumber *y, const decNumber *x) {
	decNumber s, t, my;
	int isxint, xodd, ynegative;
	int negate = 0;

	if (dn_eq0(dn_compare(&t, &const_1, y)))
		return dn_1(r);				// 1^x = 1

	if (dn_eq0(x))
		return dn_1(r);				// y^0 = 1

	if (decNumberIsNaN(x) || decNumberIsNaN(y))
		return set_NaN(r);

	if (dn_eq0(dn_compare(&t, &const_1, x)))
		return decNumberCopy(r, y); 		// y^1 = y

	isxint = is_int(x);
	if (decNumberIsInfinite(x)) {
		dn_compare(&t, y, &const__1);
		if (dn_eq0(&t))
			return dn_1(r);			// -1 ^ +/-inf = 1

		dn_abs(&t, y);
		dn_compare(&s, &t, &const_1);
		if (decNumberIsNegative(x)) {
			if (decNumberIsNegative(&s))
				return set_inf(r);		// y^-inf |y|<1 = +inf
			return decNumberZero(r);		// y^-inf |y|>1 = +0
		}
		if (decNumberIsNegative(&s))
			return decNumberZero(r);		// y^inf |y|<1 = +0
		return set_inf(r);				// y^inf |y|>1 = +inf
	}

	ynegative = decNumberIsNegative(y);
	if (decNumberIsInfinite(y)) {
		if (ynegative) {
			xodd = isxint && is_even(x) == 0;
			if (decNumberIsNegative(x)) {
				decNumberZero(r);		// -inf^x x<0 = +0
				if (xodd)			// -inf^x odd x<0 = -0
					return decNumberCopy(r, &const__0);
				return r;
			}
			if (xodd)
				return set_neginf(r);		// -inf^x odd x>0 = -inf
			return set_inf(r);			// -inf^x x>0 = +inf
		}
		if (decNumberIsNegative(x))
			return decNumberZero(r);		// +inf^x x<0 = +0
		return set_inf(r);				// +inf^x x>0 = +inf
	}

	if (dn_eq0(y)) {
		xodd = isxint && is_even(x) == 0;
		if (decNumberIsNegative(x)) {
			if (xodd && ynegative)
				return set_neginf(r);		// -0^x odd x<0 = -inf
			return set_inf(r);			// 0^x x<0 = +inf
		}
		if (xodd && ynegative)
			return decNumberCopy(r, &const__0);	// -0^x odd x>0 = -/+0
		return decNumberZero(r);			// 0^x x>0 = +0
	}

	if (ynegative) {
		if (!isxint)
			return set_NaN(r);			// y^x y<0, x not odd int = NaN
		if (is_even(x) == 0)				// y^x, y<0, x odd = - ((-y)^x)
			negate = 1;
		dn_minus(&my, y);
		y = &my;
	}
	dn_ln(&t, y);
	dn_multiply(&s, &t, x);
	dn_exp(r, &s);
	if (negate)
		return dn_minus(r, r);
	return r;
}


/* ln(1+x) */
decNumber *decNumberLn1p(decNumber *r, const decNumber *x) {
	decNumber u, v, w;

	if (decNumberIsSpecial(x) || dn_eq0(x)) {
		return decNumberCopy(r, x);
	}
	dn_p1(&u, x);
	dn_m1(&v, &u);
	if (dn_eq0(&v)) {
		return decNumberCopy(r, x);
	}
	dn_divide(&w, x, &v);
	dn_ln(&v, &u);
	return dn_multiply(r, &v, &w);
}

/* exp(x)-1 */
decNumber *decNumberExpm1(decNumber *r, const decNumber *x) {
	decNumber u, v, w;

	if (decNumberIsSpecial(x)) {
		return decNumberCopy(r, x);
	}
	dn_exp(&u, x);
	dn_m1(&v, &u);
	if (dn_eq0(&v)) {
		return decNumberCopy(r, x);
	}
	dn_compare(&w, &v, &const__1);
	if (dn_eq0(&w)) {
		return dn__1(r);
	}
	dn_multiply(&w, &v, x);
	dn_ln(&v, &u);
	return dn_divide(r, &w, &v);
}


decNumber *do_log(decNumber *r, const decNumber *x, const decNumber *base) {
	decNumber y;

	if (decNumberIsInfinite(x)) {
		return set_inf(r);
	}
	dn_ln(&y, x);
	return dn_divide(r, &y, base);
}

/* Natural logarithm.
 *
 * Take advantage of the fact that we store our numbers in the form: m * 10^e
 * so log(m * 10^e) = log(m) + e * log(10)
 * do this so that m is always in the range 0.1 <= m < 2.  However if the number
 * is already in the range 0.5 .. 1.5, this step is skipped.
 *
 * Then use the fact that ln(x^2) = 2 * ln(x) to range reduce the mantissa
 * into 1/sqrt(2) <= m < 2.
 *
 * Finally, apply the series expansion:
 *   ln(x) = 2(a+a^3/3+a^5/5+...) where a=(x-1)/(x+1)
 * which converges quickly for an argument near unity.
 */
decNumber *dn_ln(decNumber *r, const decNumber *x) {
	decNumber z, t, f, n, m, i, v, w, e;
	int expon;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x) || decNumberIsNegative(x))
			return set_NaN(r);
		return set_inf(r);
	}
	if (dn_le0(x))
		return set_NaN(r);
	decNumberCopy(&z, x);
	decNumberCopy(&f, &const_2);
	dn_m1(&t, x);
	dn_abs(&v, &t);
	if (dn_gt0(dn_compare(&t, &v, &const_0_5))) {
		expon = z.exponent + z.digits;
		z.exponent = -z.digits;
	} else
		expon = 0;

/* The too high case never happens
	while (dn_le0(dn_compare(&t, &const_2, &z))) {
		dn_mul2(&f, &f);
		dn_sqrt(&z, &z);
	}
*/
	// Range reduce the value by repeated square roots.
	// Making the constant here larger will reduce the number of later
	// iterations at the expense of more square root operations.
	while (dn_le0(dn_compare(&t, &z, &const_root2on2))) {
		dn_mul2(&f, &f);
		dn_sqrt(&z, &z);
	}
	dn_p1(&t, &z);
	dn_m1(&v, &z);
	dn_divide(&n, &v, &t);
	decNumberCopy(&v, &n);
	decNumberSquare(&m, &v);
	decNumberCopy(&i, &const_3);

	for (;;) {
		dn_multiply(&n, &m, &n);
		dn_divide(&e, &n, &i);
		dn_add(&w, &v, &e);
		if (relative_error(&w, &v, &const_1e_32))
			break;
		decNumberCopy(&v, &w);
		dn_p2(&i, &i);
	}
	dn_multiply(r, &f, &w);
	if (expon == 0)
		return r;
	int_to_dn(&e, expon);
	dn_multiply(&w, &e, &const_ln10);
	return dn_add(r, r, &w);
}

decNumber *dn_log2(decNumber *r, const decNumber *x) {
	return do_log(r, x, &const_ln2);
}

decNumber *dn_log10(decNumber *r, const decNumber *x) {
	return do_log(r, x, &const_ln10);
}

decNumber *decNumberLogxy(decNumber *r, const decNumber *y, const decNumber *x) {
	decNumber lx;

	dn_ln(&lx, x);
	return do_log(r, y, &lx);
}

decNumber *decNumberPow2(decNumber *r, const decNumber *x) {
	return dn_power(r, &const_2, x);
}

decNumber *decNumberPow10(decNumber *r, const decNumber *x) {
	return dn_power(r, &const_10, x);
}

decNumber *decNumberPow_1(decNumber *r, const decNumber *x) {
	int even = is_even(x);
	decNumber t, u;

	if (even == 1)
		return dn_1(r);
	if (even == 0)
		return dn__1(r);
	decNumberMod(&u, x, &const_2);
	dn_mulPI(&t, &u);
	sincosTaylor(&t, NULL, r);
	return r;
}

decNumber *decNumberLamW(decNumber *r, const decNumber *x) {
#ifndef TINY_BUILD
	decNumber s, t, u, v, w;
	int i;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x) || decNumberIsNegative(x))
			return set_NaN(r);
		return set_inf(r);
	}

	decNumberRecip(&t, &const_e);
	dn_minus(&s, &t);	// -1/e
	dn_compare(&t, &s, x);
	if (! decNumberIsNegative(&t)) {
		return set_NaN(r);
	}

	// Make an initial guess as to the value
	dn_compare(&t, &const_500, x);
	if (! decNumberIsNegative(&t)) {
		// x<500, lx1 = ln(x+1); est = .665 * (1 + .0195*lx1) * lx1 + 0.04
		dn_p1(&t, x);
		dn_ln(&w, &t);

		dn_multiply(&s, &const_0_0195, &w);
		dn_p1(&t, &s);
		dn_multiply(&u, &t, &const_0_665);
		dn_multiply(&t, &u, &w);
		dn_add(r, &const_0_04, &t);
	} else {
		// x>=500, est = ln(x-4) - (1 - 1/ln(x)) * ln(ln(x))
		dn_ln(&w, x);
		dn_ln(&t, &w);
		decNumberRecip(r, &w);
		dn_1m(&s, r);
		dn_multiply(r, &s, &t);

		dn_subtract(&s, x, &const_4);
		dn_ln(&t, &s);

		dn_subtract(r, &t, r);
	}

	for (i=0; i<20; i++) {
		// Now iterate to refine the estimate
		dn_p1(&u, r);			// u = wj + 1
		dn_exp(&t, r);			// t = e^wj
		dn_multiply(&s, &u, &t);	// s = (wj+1)e^wj

		dn_p1(&v, &u);			// v = wj + 2
		dn_mul2(&w, &u);		// w = 2wj + 2
		dn_divide(&u, &v, &w);		// u = (wj+2)/(2wj+2)
		dn_multiply(&w, &t, r);		// w = wj e^wj

		// Check for termination w, x, u & s are live here
		dn_subtract(&v, x, &w);	// v = x - wj e^wj
		dn_divide(&t, &v, &s);
		dn_abs(&t, &t);
		dn_compare(&t, &t, &const_1e_32);
		if (decNumberIsNegative(&t))
			break;

		// Continue the increment update
		dn_minus(&v, &v);		// v = wj e^wj - x
		dn_multiply(&t, &v, &u);	// t = (wj+2).(wj e^wj - x) / (2wj + 2)
		dn_subtract(&w, &s, &t);	// w = denominator
		dn_divide(&t, &v, &w);
		dn_subtract(r, r, &t);	// wj+1
	}
	return r;
#else
	return NULL;
#endif
}

decNumber *decNumberInvW(decNumber *r, const decNumber *x) {
	decNumber t;

	dn_exp(&t, x);
	return dn_multiply(r, &t, x);
}


/* Square - this almost certainly could be done more efficiently
 */
decNumber *decNumberSquare(decNumber *r, const decNumber *x) {
	return dn_multiply(r, x, x);
}

/* Cube - again could be done more efficiently */
decNumber *decNumberCube(decNumber *r, const decNumber *x) {
	decNumber z;

	decNumberSquare(&z, x);
	return dn_multiply(r, &z, x);
}

/* Cube root */
decNumber *decNumberCubeRoot(decNumber *r, const decNumber *x) {
	decNumber third, t;

	decNumberRecip(&third, &const_3);

	if (decNumberIsNegative(x)) {
		dn_minus(r, x);
		dn_power(&t, r, &third);
		return dn_minus(r, &t);
	}
	return dn_power(r, x, &third);
}

#ifdef INCLUDE_XROOT
decNumber *decNumberXRoot(decNumber *r, const decNumber *a, const decNumber *b) {
	decNumber s, t;

	decNumberRecip(&s, b);

	if (decNumberIsNegative(a)) {
		if (is_even(b) == 0) {
			dn_minus(r, a);
			dn_power(&t, r, &s);
			return dn_minus(r, &t);
		}
		return set_NaN(r);
	}
	return dn_power(r, a, &s);
}

/* Integer xth root piggy backs on the real code.
 */
long long int intXRoot(long long int y, long long int x) {
	int sx, sy;
	unsigned long long int vx = extract_value(x, &sx);
	unsigned long long int vy = extract_value(y, &sy);
	decNumber rx, ry, rz;

	if (sx) {
		if (vy == 1) {
			set_carry(0);
			vy = 1;
		} else {
			set_carry(1);
			vy = 0;
		}
		if (sy != 0)
			sy = vx & 1;
		return build_value(vy, sy);
	}

	ullint_to_dn(&rx, vx);
	ullint_to_dn(&ry, vy);
	decNumberXRoot(&rz, &ry, &rx);
	if (decNumberIsSpecial(&rz)) {
		set_overflow(1);
		set_carry(0);
		return 0;
	}
	vy = dn_to_ull(decNumberFloor(&rx, &rz), &sx);
	if (sy) {
		sx = (vx & 1) ? 0 : 1;
		y = -y;
	} else
		sx = 0;
	set_carry(intPower(vy, x) != y);
	set_overflow(sx);
	return build_value(vy, sy);
}

#endif


decNumber *decNumberMod(decNumber *res, const decNumber *x, const decNumber *y) {
	/* Declare a structure large enough to hold a really long number.
	 * This structure is likely to be larger than is required.
	 */
	struct {
		decNumber n;
		decNumberUnit extra[((MOD_DIGITS-DECNUMDIGITS+DECDPUN-1)/DECDPUN)];
	} out;

	int digits = Ctx.digits;

	Ctx.digits = MOD_DIGITS;
	decNumberRemainder(&out.n, x, y, &Ctx);
	Ctx.digits = digits;

	return decNumberPlus(res, &out.n, &Ctx);
}


decNumber *decNumberBigMod(decNumber *res, const decNumber *x, const decNumber *y) {
	/* Declare a structure large enough to hold a really long number.
	 * This structure is likely to be larger than is required.
	 */
	struct {
		decNumber n;
		decNumberUnit extra[((BIGMOD_DIGITS-DECNUMDIGITS+DECDPUN-1)/DECDPUN)];
	} out;

	int digits = Ctx.digits;

	Ctx.digits = BIGMOD_DIGITS;
	decNumberRemainder(&out.n, x, y, &Ctx);
	Ctx.digits = digits;

	return decNumberPlus(res, &out.n, &Ctx);
}


/* Calculate sin and cos by Taylor series
 */
static void sincosTaylor(const decNumber *a, decNumber *s, decNumber *c) {
	decNumber a2, t, j, z;
	int i, fins = 0, finc = 0;

	dn_multiply(&a2, a, a);
	dn_1(&j);
	dn_1(&t);
	if (s != NULL)
		dn_1(s);
	if (c != NULL)
		dn_1(c);

	for (i=1; !fins && !finc && i < 1000; i++) {
		dn_inc(&j);
		dn_divide(&z, &a2, &j);
		dn_multiply(&t, &t, &z);
		if (!finc && c != NULL) {
			decNumberCopy(&z, c);
			if (i & 1)
				dn_subtract(c, c, &t);
			else
				dn_add(c, c, &t);
			dn_compare(&z, c, &z);
			if (dn_eq0(&z))
				finc = 1;
		}

		dn_inc(&j);
		dn_divide(&t, &t, &j);
		if (!fins && s != NULL) {
			decNumberCopy(&z, s);
			if (i & 1)
				dn_subtract(s, s, &t);
			else
				dn_add(s, s, &t);
			dn_compare(&z, s, &z);
			if (dn_eq0(&z))
				fins = 1;
		}
	}
	if (s != NULL)
		dn_multiply(s, s, a);
}


/* Check for right angle multiples and if exact, return the apropriate
 * quadrant constant directly.
 */
static int right_angle(decNumber *res, const decNumber *x,
		const decNumber *quad, const decNumber *r0, const decNumber *r1,
		const decNumber *r2, const decNumber *r3) {
	decNumber r;
	const decNumber *z;

	decNumberRemainder(&r, x, quad, &Ctx);
	if (!dn_eq0(&r))
		return 0;

	if (dn_eq0(x))
		z = r0;
	else {
		dn_add(&r, quad, quad);
		dn_compare(&r, &r, x);
		if (dn_eq0(&r))
			z = r2;
		else if (decNumberIsNegative(&r))
			z = r3;
		else	z = r1;
	}
	decNumberCopy(res, z);
	return 1;
}

/* Convert the number into radians.
 * We take the opportunity to reduce angles modulo 2 * PI here
 * For degrees and gradians, the reduction is exact and easy.
 * For radians, it involves a lot more computation.
 * For degrees and gradians, we return exact results
 * for right angles and multiples thereof.
 */
static int cvt_2rad(decNumber *res, const decNumber *x,
		const decNumber *r0, const decNumber *r1,
		const decNumber *r2, const decNumber *r3) {
	decNumber fm;

	switch (get_trig_mode()) {
	case TRIG_RAD:
		decNumberMod(res, x, &const_2PI);
		break;
	case TRIG_DEG:
		decNumberMod(&fm, x, &const_360);
                if (decNumberIsNegative(&fm))
                    dn_add(&fm, &fm, &const_360);
		if (r0 != NULL && right_angle(res, &fm, &const_90, r0, r1, r2, r3))
			return 0;
		decNumberD2R(res, &fm);
		break;
	case TRIG_GRAD:
		decNumberMod(&fm, x, &const_400);
                if (decNumberIsNegative(&fm))
                    dn_add(&fm, &fm, &const_400);
		if (r0 != NULL && right_angle(res, &fm, &const_100, r0, r1, r2, r3))
			return 0;
		decNumberG2R(res, &fm);
		break;
	}
	return 1;
}

static void cvt_rad2(decNumber *res, const decNumber *x) {
	switch (get_trig_mode()) {
	case TRIG_RAD:	decNumberCopy(res, x);		break;
	case TRIG_DEG:	decNumberR2D(res, x);	break;
	case TRIG_GRAD:	decNumberR2G(res, x);	break;
	}
}

/* Calculate sin and cos of the given number in radians.
 * We need to do some range reduction to guarantee that our Taylor series
 * converges rapidly.
 */
void dn_sincos(const decNumber *v, decNumber *sinv, decNumber *cosv)
{
	decNumber x;

	if (decNumberIsSpecial(v))
		cmplx_NaN(sinv, cosv);
	else {
		decNumberMod(&x, v, &const_2PI);
		sincosTaylor(&x, sinv, cosv);
	}
}

decNumber *decNumberSin(decNumber *res, const decNumber *x) {
	decNumber x2;

	if (decNumberIsSpecial(x))
		return set_NaN(res);
	else {
		if (cvt_2rad(&x2, x, &const_0, &const_1, &const_0, &const__1))
			sincosTaylor(&x2, res, NULL);
		else
			decNumberCopy(res, &x2);
	}
	return res;
}

decNumber *decNumberCos(decNumber *res, const decNumber *x) {
	decNumber x2;

	if (decNumberIsSpecial(x))
		return set_NaN(res);
	else {
		if (cvt_2rad(&x2, x, &const_1, &const_0, &const__1, &const_0))
			sincosTaylor(&x2, NULL, res);
		else
			decNumberCopy(res, &x2);
	}
	return res;
}

decNumber *decNumberTan(decNumber *res, const decNumber *x) {
	decNumber x2, s, c;

	if (decNumberIsSpecial(x))
		return set_NaN(res);
	else {
		if (cvt_2rad(&x2, x, &const_0, &const_NaN, &const_0, &const_NaN)) {
			sincosTaylor(&x2, &s, &c);
			dn_divide(res, &s, &c);
		} else
			decNumberCopy(res, &x2);
	}
	return res;
}

#if 0
decNumber *decNumberSec(decNumber *res, const decNumber *x) {
	return dn_recip(res, x, &decNumberCos);
}

decNumber *decNumberCosec(decNumber *res, const decNumber *x) {
	return dn_recip(res, x, &decNumberSin);
}

decNumber *decNumberCot(decNumber *res, const decNumber *x) {
	return dn_recip(res, x, &decNumberTan);
}
#endif

decNumber *decNumberSinc(decNumber *res, const decNumber *x) {
	decNumber s, t;

	decNumberSquare(&s, x);
	dn_p1(&t, &s);
	dn_compare(&s, &t, &const_1);
	if (dn_eq0(&s))
		dn_1(res);
	else {
		decNumberSin(&s, x);
		dn_divide(res, &s, x);
	}
	return res;
}

void do_atan(decNumber *res, const decNumber *x) {
	decNumber a, b, a1, a2, t, j, z, last;
	int doubles = 0;
	int invert;
	int neg = decNumberIsNegative(x);
	int n;

	// arrange for a >= 0
	if (neg)
		dn_minus(&a, x);
	else
		decNumberCopy(&a, x);

	// reduce range to 0 <= a < 1, using atan(x) = pi/2 - atan(1/x)
	dn_compare(&b, &a, &const_1);
	invert = dn_gt0(&b);
	if (invert)
		dn_divide(&a, &const_1, &a);

	// Range reduce to small enough limit to use taylor series
	// using:
	//  tan(x/2) = tan(x)/(1+sqrt(1+tan(x)^2))
	for (n=0; n<1000; n++) {
		dn_compare(&b, &a, &const_0_1);
		if (dn_le0(&b))
			break;
		doubles++;
		// a = a/(1+sqrt(1+a^2)) -- at most 3 iterations.
		dn_multiply(&b, &a, &a);
		dn_inc(&b);
		dn_sqrt(&b, &b);
		dn_inc(&b);
		dn_divide(&a, &a, &b);
	}

	// Now Taylor series
	// tan(x) = x(1-x^2/3+x^4/5-x^6/7...)
	// We calculate pairs of terms and stop when the estimate doesn't change
	decNumberCopy(res, &const_3);
	decNumberCopy(&j, &const_5);
	dn_multiply(&a2, &a, &a);        // a^2
	decNumberCopy(&t, &a2);
	dn_divide(res, &t, res);         // s = 1-t/3 -- first two terms
	dn_1m(res, res);

	do {    // Loop until there is no digits changed
		decNumberCopy(&last, res);

		dn_multiply(&t, &t, &a2);
		dn_divide(&z, &t, &j);
		dn_add(res, res, &z);
		dn_p2(&j, &j);

		dn_multiply(&t, &t, &a2);
		dn_divide(&z, &t, &j);
		dn_subtract(res, res, &z);
		dn_p2(&j, &j);

		dn_compare(&a1, res, &last);
	} while (!dn_eq0(&a1));
	dn_multiply(res, res, &a);

	while (doubles) {
		dn_add(res, res, res);
		doubles--;
	}

	if (invert) {
		dn_subtract(res, &const_PIon2, res);
	}

	if (neg)
		dn_minus(res, res);
}

void do_asin(decNumber *res, const decNumber *x) {
	decNumber abx, z;

	if (decNumberIsNaN(x)) {
		set_NaN(res);
		return;
	}

	dn_abs(&abx, x);
	dn_compare(&z, &abx, &const_1);
	if (dn_gt0(&z)) {
		set_NaN(res);
		return;
	}

	// res = 2*atan(x/(1+sqrt(1-x*x)))
	dn_multiply(&z, x, x);
	dn_1m(&z, &z);
	dn_sqrt(&z, &z);
	dn_inc(&z);
	dn_divide(&z, x, &z);
	do_atan(&abx, &z);
	dn_mul2(res, &abx);
}

void do_acos(decNumber *res, const decNumber *x) {
	decNumber abx, z;

	if (decNumberIsNaN(x)) {
		set_NaN(res);
		return;
	}

	dn_abs(&abx, x);
	dn_compare(&z, &abx, &const_1);

	if (dn_gt0(&z)) {
		set_NaN(res);
		return;
	}

	// res = 2*atan((1-x)/sqrt(1-x*x))
	dn_compare(&z, x, &const_1);
	if (dn_eq0(&z))
		decNumberZero(res);
    else {
        dn_multiply(&z, x, x);
        dn_1m(&z, &z);
        dn_sqrt(&z, &z);
        dn_1m(&abx, x);
        dn_divide(&z, &abx, &z);
        do_atan(&abx, &z);
        dn_mul2(res, &abx);
    }
}

decNumber *decNumberArcSin(decNumber *res, const decNumber *x) {
	decNumber z;

	do_asin(&z, x);
	cvt_rad2(res, &z);
	return res;
}

decNumber *decNumberArcCos(decNumber *res, const decNumber *x) {
	decNumber z;

	do_acos(&z, x);
	cvt_rad2(res, &z);
	return res;
}

decNumber *decNumberArcTan(decNumber *res, const decNumber *x) {
	decNumber z;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			return set_NaN(res);
		else {
			decNumberCopy(res, &const_PIon2);
			if (decNumberIsNegative(x))
				dn_minus(res, res);
		}
	} else
		do_atan(&z, x);
	cvt_rad2(res, &z);
	return res;
}

decNumber *do_atan2(decNumber *at, const decNumber *y, const decNumber *x) {
	decNumber r, t;
	const int xneg = decNumberIsNegative(x);
	const int yneg = decNumberIsNegative(y);

	if (decNumberIsNaN(x) || decNumberIsNaN(y)) {
		return set_NaN(at);
	}
	if (dn_eq0(y)) {
		if (yneg) {
			if (dn_eq0(x)) {
				if (xneg) {
					decNumberPI(at);
					dn_minus(at, at);
				} else
					decNumberCopy(at, y);
			} else if (xneg) {
				decNumberPI(at);
				dn_minus(at, at);
			} else
				decNumberCopy(at, y);
		} else {
			if (dn_eq0(x)) {
				if (xneg)
					decNumberPI(at);
				else
					decNumberZero(at);
			} else if (xneg)
				decNumberPI(at);
			else
				decNumberZero(at);
		}
		return at;
	}
	if (dn_eq0(x)) {
		decNumberPIon2(at);
		if (yneg)
			dn_minus(at, at);
		return at;
	}
	if (decNumberIsInfinite(x)) {
		if (xneg) {
			if (decNumberIsInfinite(y)) {
				decNumberPI(&t);
				dn_multiply(at, &t, &const_0_75);
				if (yneg)
					dn_minus(at, at);
			} else {
				decNumberPI(at);
				if (yneg)
					dn_minus(at, at);
			}
		} else {
			if (decNumberIsInfinite(y)) {
				decNumberPIon2(&t);
				dn_div2(at, &t);
				if (yneg)
					dn_minus(at, at);
			} else {
				decNumberZero(at);
				if (yneg)
					dn_minus(at, at);
			}
		}
		return at;
	}
	if (decNumberIsInfinite(y)) {
		decNumberPIon2(at);
		if (yneg)
			dn_minus(at, at);
		return at;
	}

	dn_divide(&t, y, x);
	do_atan(&r, &t);
	if (xneg) {
		decNumberPI(&t);
		if (yneg)
			dn_minus(&t, &t);
	} else
		decNumberZero(&t);
	dn_add(at, &r, &t);
	if (dn_eq0(at) && yneg)
		dn_minus(at, at);
	return at;
}

decNumber *decNumberArcTan2(decNumber *res, const decNumber *a, const decNumber *b) {
	decNumber z;

	do_atan2(&z, a, b);
	cvt_rad2(res, &z);
	return res;	
}

void op_r2p(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	decNumber x, y, rx, ry;

	getXY(&x, &y);
	cmplxToPolar(&rx, &ry, &x, &y);
	cvt_rad2(&y, &ry);
	setlastX();
	setXY(&rx, &y);
}

void op_p2r(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	decNumber x, y, rx, ry;

	getXY(&x, &ry);
	cvt_2rad(&y, &ry, NULL, NULL, NULL, NULL);
	cmplxFromPolar(&rx, &ry, &x, &y);
	setlastX();
	setXY(&rx, &ry);
}	


/* Hyperbolic functions.
 * We start with a utility routine that calculates sinh and cosh.
 * We do the sihn as (e^x - 1) / e^x + e^x - 1 for numerical stability
 * reasons if the value of x is smallish.
 */
void dn_sinhcosh(const decNumber *x, decNumber *sinhv, decNumber *coshv) {
	decNumber t, u, v;

	if (sinhv != NULL) {
		if (decNumberIsNegative(dn_compare(&u, dn_abs(&t, x), &const_0_5))) {
			decNumberExpm1(&u, x);
			dn_div2(&t, &u);
			dn_inc(&u);
			dn_divide(&v, &t, &u);
			dn_inc(&u);
			dn_multiply(sinhv, &u, &v);
		} else {
			dn_exp(&u, x);			// u = e^x
			decNumberRecip(&v, &u);		// v = e^-x
			dn_subtract(&t, &u, &v);	// r = e^x - e^-x
			dn_div2(sinhv, &t);
		}
	}
	if (coshv != NULL) {
		dn_exp(&u, x);			// u = e^x
		decNumberRecip(&v, &u);		// v = e^-x
		dn_add(&t, &v, &u);		// r = e^x + e^-x
		dn_div2(coshv, &t);
	}
}

decNumber *decNumberSinh(decNumber *res, const decNumber *x) {
	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			return set_NaN(res);
		return decNumberCopy(res, x);
	}
	dn_sinhcosh(x, res, NULL);
	return res;
}

decNumber *decNumberCosh(decNumber *res, const decNumber *x) {
	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			return set_NaN(res);
		return set_inf(res);
	}
	dn_sinhcosh(x, NULL, res);
	return res;
}

decNumber *decNumberTanh(decNumber *res, const decNumber *x) {
	decNumber a, b;

	if (decNumberIsNaN(x))
		return set_NaN(res);
	dn_abs(&a, x);
	if (decNumberIsNegative(dn_compare(&b, &const_100, &a))) {
		if (decNumberIsNegative(x))
			return dn__1(res);
		return dn_1(res);
	}
	dn_add(&a, x, x);
	decNumberExpm1(&b, &a);
	dn_p2(&a, &b);
	return dn_divide(res, &b, &a);
}


#if 0
decNumber *decNumberSech(decNumber *res, const decNumber *x) {
	return dn_recip(res, x, &decNumberCosh);
}

decNumber *decNumberCosech(decNumber *res, const decNumber *x) {
	return dn_recip(res, x, &decNumberSinh);
}

decNumber *decNumberCoth(decNumber *res, const decNumber *x) {
	return dn_recip(res, x, &decNumberTanh);
}
#endif

decNumber *decNumberArcSinh(decNumber *res, const decNumber *x) {
	decNumber y, z;

	decNumberSquare(&y, x);		// y = x^2
	dn_p1(&z, &y);			// z = x^2 + 1
	dn_sqrt(&y, &z);		// y = sqrt(x^2+1)
	dn_inc(&y);			// y = sqrt(x^2+1)+1
	dn_divide(&z, x, &y);
	dn_inc(&z);
	dn_multiply(&y, x, &z);
	return decNumberLn1p(res, &y);
}


decNumber *decNumberArcCosh(decNumber *res, const decNumber *x) {
	decNumber z;

	decNumberSquare(res, x);	// r = x^2
	dn_m1(&z, res);			// z = x^2 + 1
	dn_sqrt(res, &z);		// r = sqrt(x^2+1)
	dn_add(&z, res, x);		// z = x + sqrt(x^2+1)
	return dn_ln(res, &z);
}

decNumber *decNumberArcTanh(decNumber *res, const decNumber *x) {
	decNumber y, z;

	if (decNumberIsNaN(x))
		return set_NaN(res);
	dn_abs(&y, x);
	if (dn_eq0(dn_compare(&z, &y, &const_1))) {
		if (decNumberIsNegative(x))
			return set_neginf(res);
		return set_inf(res);
	}
	// Not the obvious formula but more stable...
	dn_1m(&z, x);
	dn_divide(&y, x, &z);
	dn_mul2(&z, &y);
	decNumberLn1p(&y, &z);
	return dn_div2(res, &y);
}


decNumber *decNumberD2R(decNumber *res, const decNumber *x) {
	return dn_multiply(res, x, &const_PIon180);
}

decNumber *decNumberR2D(decNumber *res, const decNumber *x) {
	return dn_divide(res, x, &const_PIon180);
}


decNumber *decNumberG2R(decNumber *res, const decNumber *x) {
	return dn_multiply(res, x, &const_PIon200);
}

decNumber *decNumberR2G(decNumber *res, const decNumber *x) {
	return dn_divide(res, x, &const_PIon200);
}

decNumber *decNumberG2D(decNumber *res, const decNumber *x) {
	return dn_multiply(res, x, &const_0_9);
}

decNumber *decNumberD2G(decNumber *res, const decNumber *x) {
	return dn_divide(res, x, &const_0_9);
}

decNumber *decNumber2Deg(decNumber *res, const decNumber *x) {
	switch (get_trig_mode()) {
	case TRIG_DEG:	decNumberCopy(res, x);		break;
	case TRIG_RAD:	decNumberR2D(res, x);	break;
	case TRIG_GRAD:	decNumberG2D(res, x);	break;
	}
	return res;
}

decNumber *decNumber2Rad(decNumber *res, const decNumber *x) {
	switch (get_trig_mode()) {
	case TRIG_DEG:	decNumberD2R(res, x);	break;
	case TRIG_RAD:	decNumberCopy(res, x);		break;
	case TRIG_GRAD:	decNumberG2R(res, x);	break;
	}
	return res;
}

decNumber *decNumber2Grad(decNumber *res, const decNumber *x) {
	switch (get_trig_mode()) {
	case TRIG_DEG:	decNumberD2G(res, x);	break;
	case TRIG_RAD:	decNumberR2G(res, x);	break;
	case TRIG_GRAD:	decNumberCopy(res, x);		break;
	}
	return res;
}

decNumber *decNumberDeg2(decNumber *res, const decNumber *x) {
	switch (get_trig_mode()) {
	case TRIG_DEG:	decNumberCopy(res, x);		break;
	case TRIG_RAD:	decNumberD2R(res, x);	break;
	case TRIG_GRAD:	decNumberD2G(res, x);	break;
	}
	return res;
}

decNumber *decNumberRad2(decNumber *res, const decNumber *x) {
	switch (get_trig_mode()) {
	case TRIG_DEG:	decNumberR2D(res, x);	break;
	case TRIG_RAD:	decNumberCopy(res, x);		break;
	case TRIG_GRAD:	decNumberR2G(res, x);	break;
	}
	return res;
}

decNumber *decNumberGrad2(decNumber *res, const decNumber *x) {
	switch (get_trig_mode()) {
	case TRIG_DEG:	decNumberG2D(res, x);	break;
	case TRIG_RAD:	decNumberG2R(res, x);	break;
	case TRIG_GRAD:	decNumberCopy(res, x);		break;
	}
	return res;
}

/* Check the arguments a little and perform the computation of
 * ln(permutation) which is common across both our callers.
 *
 * This is the real version.
 */
enum perm_opts { PERM_INVALID=0, PERM_INTG, PERM_NORMAL };
static enum perm_opts perm_helper(decNumber *r, const decNumber *x, const decNumber *y) {
	decNumber n, s;

	if (decNumberIsSpecial(x) || decNumberIsSpecial(y) || dn_lt0(x) || dn_lt0(y)) {
		if (decNumberIsInfinite(x) && !decNumberIsInfinite(y))
			set_inf(r);
		else
			set_NaN(r);
		return PERM_INVALID;
	}
	dn_p1(&n, x);				// x+1
	decNumberLnGamma(&s, &n);		// lnGamma(x+1) = Ln x!

	dn_subtract(r, &n, y);	// x-y+1
	if (dn_le0(r)) {
		set_NaN(r);
		return PERM_INVALID;
	}
	decNumberLnGamma(&n, r);		// LnGamma(x-y+1) = Ln (x-y)!
	dn_subtract(r, &s, &n);

	if (is_int(x) && is_int(y))
		return PERM_INTG;
	return PERM_NORMAL;
}


/* Calculate permutations:
 * C(x, y) = P(x, y) / y! = x! / ( (x-y)! y! )
 */
decNumber *decNumberComb(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber r, n, s;
	const enum perm_opts code = perm_helper(&r, x, y);

	if (code != PERM_INVALID) {
		dn_p1(&n, y);				// y+1
		decNumberLnGamma(&s, &n);		// LnGamma(y+1) = Ln y!
		dn_subtract(&n, &r, &s);

		dn_exp(res, &n);
		if (code == PERM_INTG)
			decNumberIntg(res, res);
	} else
		decNumberCopy(res, &r);
	return res;
}

/* Calculate permutations:
 * P(x, y) = x! / (x-y)!
 */
decNumber *decNumberPerm(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber t;
	const enum perm_opts code = perm_helper(&t, x, y);

	if (code != PERM_INVALID) {
		dn_exp(res, &t);
		if (code == PERM_INTG)
			decNumberIntg(res, res);
	} else
		decNumberCopy(res, &t);
	return res;
}


const decNumber *const gamma_consts[21] = {
	&const_gammaC01, &const_gammaC02, &const_gammaC03,
	&const_gammaC04, &const_gammaC05, &const_gammaC06,
	&const_gammaC07, &const_gammaC08, &const_gammaC09,
	&const_gammaC10, &const_gammaC11, &const_gammaC12,
	&const_gammaC13, &const_gammaC14, &const_gammaC15,
	&const_gammaC16, &const_gammaC17, &const_gammaC18,
	&const_gammaC19, &const_gammaC20, &const_gammaC21,
};

static void dn_LnGamma(decNumber *res, const decNumber *x) {
	decNumber s, t, u, v;
	int k;

	decNumberZero(&s);
	dn_add(&t, x, &const_21);
	for (k=20; k>=0; k--) {
		dn_divide(&u, gamma_consts[k], &t);
		dn_dec(&t);
		dn_add(&s, &s, &u);
	}
	dn_add(&t, &s, &const_gammaC00);
	dn_multiply(&u, &t, &const_2rootEonPI);
	dn_ln(&s, &u);

	dn_add(&t, x, &const_gammaR);
	dn_divide(&u, &t, &const_e);
	dn_ln(&t, &u);
	dn_add(&u, x, &const_0_5);
	dn_multiply(&v, &u, &t);
	dn_add(res, &v, &s);
}

decNumber *decNumberFactorial(decNumber *res, const decNumber *xin) {
	decNumber x;

	dn_p1(&x, xin);
	return decNumberGamma(res, &x);
}

#ifdef INCLUDE_DBLFACT
decNumber *decNumberDblFactorial(decNumber *r, const decNumber *x) {
	decNumber t, u, v;

	dn_p2(&t, x);				// t = x+2
	decNumberPow2(&u, &t);			// u = 2^(x+1)
	dn_sqrt(&t, &u);
	dn_multiply(&u, &t, &const_recipsqrt2PI);
	dn_div2(&t, x);
	dn_p1(&v, &t);
	decNumberGamma(&t, &v);
	return dn_multiply(r, &u, &t);
}
#endif

#ifdef INCLUDE_SUBFACT
decNumber *decNumberSubFactorial(decNumber *r, const decNumber *x) {
	decNumber t, u;

	if (is_int(x)) {
		decNumberFactorial(&t, x);
		dn_divide(&u, &t, &const_e);
		dn_add(&t, &u, &const_0_5);
		return decNumberFloor(r, &t);
	}
	return set_NaN(r);
}
#endif

decNumber *decNumberGamma(decNumber *res, const decNumber *xin) {
	decNumber x, s, t, u;
	int reflec = 0;

	// Check for special cases
	if (decNumberIsSpecial(xin)) {
		if (decNumberIsInfinite(xin) && !decNumberIsNegative(xin))
			return set_inf(res);
		return set_NaN(res);
	}

	// Correct our argument and begin the inversion if it is negative
	if (decNumberIsNegative(xin)) {
		reflec = 1;
		dn_1m(&t, xin);
		if (is_int(&t)) {
			return set_NaN(res);
		}
		dn_m1(&x, &t);
	} else
		dn_m1(&x, xin);

	dn_LnGamma(&t, &x);
	dn_exp(res, &t);

	// Finally invert if we started with a negative argument
	if (reflec) {
		// figure out xin * PI mod 2PI
		decNumberMod(&s, xin, &const_2);
		dn_mulPI(&t, &s);
		sincosTaylor(&t, &s, &u);
		dn_multiply(&u, &s, res);
		dn_divide(res, &const_PI, &u);
	}
	return res;
}

// The log gamma function.
decNumber *decNumberLnGamma(decNumber *res, const decNumber *xin) {
	decNumber x, s, t, u;
	int reflec = 0;

	// Check for special cases
	if (decNumberIsSpecial(xin)) {
		if (decNumberIsInfinite(xin) && !decNumberIsNegative(xin))
			return set_inf(res);
		return set_NaN(res);
	}

	// Correct out argument and begin the inversion if it is negative
	if (decNumberIsNegative(xin)) {
		reflec = 1;
		dn_1m(&t, xin);
		if (is_int(&t)) {
			return set_NaN(res);
		}
		dn_m1(&x, &t);
	} else
		dn_m1(&x, xin);

	dn_LnGamma(res, &x);

	// Finally invert if we started with a negative argument
	if (reflec) {
		// Figure out S * PI mod 2PI
		decNumberMod(&u, &s, &const_2);
		dn_mulPI(&t, &u);
		sincosTaylor(&t, &s, &u);
		dn_divide(&u, &const_PI, &s);
		dn_ln(&t, &u);
		dn_subtract(res, &t, res);
	}
	return res;
}

// lnBeta(x, y) = lngamma(x) + lngamma(y) - lngamma(x+y)
decNumber *decNumberLnBeta(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber s, t, u;

	decNumberLnGamma(&s, x);
	busy();
	decNumberLnGamma(&t, y);
	busy();
	dn_add(&u, &s, &t);
	dn_add(&s, x, y);
	decNumberLnGamma(&t, &s);
	dn_subtract(res, &u, &t);
	return res;
}

// Beta(x, y) = exp(lngamma(x) + lngamma(y) - lngamma(x+y))
decNumber *decNumberBeta(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber s;

	decNumberLnBeta(&s, x, y);
	dn_exp(res, &s);
	return res;
}

#ifdef INCLUDE_DIGAMMA
#ifndef TINY_BUILD
const decNumber *const digamma_consts[10] = {
	&const_digammaC02,	&const_digammaC04,	&const_digammaC06,
	&const_digammaC08,	&const_digammaC10,	&const_digammaC12,
	&const_digammaC02,	&const_digammaC16,	&const_digammaC18,
	&const_digammaC20
};
#endif

decNumber *decNumberPsi(decNumber *res, const decNumber *xin) {
#ifndef TINY_BUILD
	decNumber x_2, t, r, x;
	int i;

	if (decNumberIsSpecial(xin)) {
		if (decNumberIsNaN(xin) || decNumberIsNegative(xin))
			return set_NaN(res);
		return set_inf(res);
	}

	// Check for reflection
	if (dn_le0(xin)) {
		if (is_int(xin)) {
			return set_NaN(res);
		}
		dn_mulPI(&x_2, xin);
		dn_sincos(&x_2, &t, &r);
		dn_divide(&x_2, &r, &t);		// x_2 = cot(PI.x)
		dn_mulPI(&t, &x_2);
		dn_1m(&x, xin);
		dn_minus(res, &t);
	} else {
		decNumberZero(res);
		decNumberCopy(&x, xin);
	}

	// Use recurrance relation to bring x large enough for our series to converge
	for (;;) {
		dn_compare(&t, &const_8, &x);
		if (decNumberIsNegative(&t))
			break;
		decNumberRecip(&t, &x);
		dn_subtract(res, res, &t);
		dn_inc(&x);
	}

	// Finally the series approximation
	dn_ln(&t, &x);
	dn_add(res, res, &t);
	dn_multiply(&r, &x, &const__2);
	decNumberRecip(&t, &r);
	dn_add(res, res, &t);

	decNumberSquare(&t, &x);
	decNumberRecip(&x_2, &t);
	decNumberCopy(&r, &x_2);
	for (i=0; i<10; i++) {
		dn_multiply(&t, &r, digamma_consts[i]);
		dn_add(res, res, &t);
		dn_multiply(&r, &r, &x_2);
	}
	return res;
#else
	return NULL;
#endif
}
#endif

#ifdef INCLUDE_ZETA
// Reiman's Zeta function
/*
    \zeta(0) = -1/2,\!

    \zeta(1/2) = approx -1.4603545088095868,\!

    \zeta(1) =  = \infty;

    \zeta(3/2) \approx 2.612;\! 
    \zeta(2) =  \frac{\pi^2}{6} \approx 1.645;\! 

    \zeta(5/2) \approx 1.341.\!

    \zeta(3) =  \approx 1.202;\! 

    \zeta(7/2) \approx 1.127\!

    \zeta(4)  = \frac{\pi^4}{90} \approx 1.0823;\! 

    \zeta(6) =  \frac{\pi^6}{945} \approx 1.0173;\! 
*/
#ifndef TINY_BUILD
const decNumber *const zeta_consts[30] = {
	&const_zetaC00, &const_zetaC01, &const_zetaC02, &const_zetaC03,
	&const_zetaC04, &const_zetaC05, &const_zetaC06, &const_zetaC07,
	&const_zetaC08, &const_zetaC09, &const_zetaC10, &const_zetaC11,
	&const_zetaC12, &const_zetaC13, &const_zetaC14, &const_zetaC15,
	&const_zetaC16, &const_zetaC17, &const_zetaC18, &const_zetaC19,
	&const_zetaC20, &const_zetaC21, &const_zetaC22, &const_zetaC23,
	&const_zetaC24, &const_zetaC25, &const_zetaC26, &const_zetaC27,
	&const_zetaC28, &const_zetaC29
};

static void zeta_step(decNumber *sum, const decNumber *x,
		const decNumber *dc, decNumber *k) {
	decNumber t, s;

	busy();
	dn_inc(k);
	dn_power(&s, k, x);
	dn_divide(&t, dc, &s);
	dn_add(sum, sum, &t);
}
#endif

decNumber *decNumberZeta(decNumber *res, const decNumber *xin) {
#ifndef TINY_BUILD
	decNumber s, x, u, reflecfac, sum, t;
	int reflec, i;

	if (decNumberIsSpecial(xin)) {
		if (decNumberIsNaN(xin) || decNumberIsNegative(xin))
			return set_NaN(res);
		return dn_1(res);
	}
	if (dn_eq0(xin)) {
		return decNumberCopy(res, &const__0_5);
	}
	if (decNumberIsNegative(xin)) {
		dn_div2(&s, xin);
		if (is_int(&s)) {
			return decNumberZero(res);
		}
	}

	dn_compare(&s, xin, &const_0_5);
	if (decNumberIsNegative(&s)) {
		/* use reflection formula
		 * zeta(x) = 2^x*Pi^(x-1)*sin(Pi*x/2)*gamma(1-x)*zeta(1-x)
		 */
		reflec = 1;
		dn_1m(&x, xin);
		// Figure out xin * PI / 2 mod 2PI
		decNumberMod(&s, xin, &const_4);
		dn_multiply(&u, &const_PIon2, &s);
		sincosTaylor(&u, &s, res);
		dn_power(res, &const_2, xin);
		dn_multiply(&u, res, &s);
		dn_power(res, &const_PI, &x);
		dn_divide(&s, &u, res);
		decNumberGamma(res, &x);
		dn_multiply(&reflecfac, &s, res);
	} else {
		reflec = 0;
		decNumberCopy(&x, xin);
	}

	/* Now calculate zeta(x) where x >= 0.5 */
	decNumberZero(&sum);
	decNumberZero(&t);
	for (i=0; i<30; i++)
		zeta_step(&sum, &x, zeta_consts[i], &t);

	dn_1m(&t, &x);
	dn_power(&u, &const_2, &t);
	dn_m1(&t, &u);
	dn_multiply(&u, &t, &const_zeta_dn);
	dn_divide(res, &sum, &u);

	/* Finally, undo the reflection if required */
	if (reflec)
		dn_multiply(res, &reflecfac, res);
	return res;
#else
	return NULL;
#endif
}
#endif /* INCLUDE_ZETA */


// % = x . y / 100
decNumber *decNumberPercent(decNumber *res, const decNumber *x) {
	decNumber y, z;

	getY(&y);
	dn_mulpow10(&z, &y, -2);
	return dn_multiply(res, &z, x);
}

// %chg = 100 ( x - y ) / y
decNumber *decNumberPerchg(decNumber *res, const decNumber *x) {
	decNumber w, y, z;

	getY(&y);
	dn_subtract(&z, x, &y);
	dn_divide(&w, &z, &y);
	dn_mul100(res, &w);
	return res;
}

// %tot = 100 . x / y
decNumber *decNumberPertot(decNumber *res, const decNumber *x) {
	decNumber y, z;

	getY(&y);
	dn_divide(&z, x, &y);
	dn_mul100(res, &z);
	return res;
}

// Markup Margin = y / ( 1 - x / 100 )
decNumber *decNumberPerMargin(decNumber *res, const decNumber *y, const decNumber *x) {
	decNumber a, b;

	dn_mulpow10(&a, x, -2);
	dn_1m(&b, &a);
	return dn_divide(res, y, &b);
}

// Margin = 100 (x - y) / x
decNumber *decNumberMargin(decNumber *res, const decNumber *y, const decNumber *x) {
	decNumber a, b;

	dn_subtract(&a, x, y);
	dn_mul100(&b, &a);
	return dn_divide(res, &b, x);
}

decNumber *decNemberPerMRR(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x) {
	decNumber a, b, c;

	dn_divide(&a, x, y);
	decNumberRecip(&b, z);
	dn_power(&c, &a, &b);
	dn_m1(&a, &c);
	return dn_mul100(r, &a);
}


decNumber *decNumberHMS2HR(decNumber *res, const decNumber *x) {
	decNumber m, s, t;

	// decode hhhh.mmss...
	decNumberFrac(&t, x);			// t = .mmss
	dn_mul100(&s, &t);			// s = mm.ss
	decNumberTrunc(&m, &s);			// m = mm
	decNumberFrac(&t, &s);			// t = .ss
	dn_multiply(&s, &t, &const_1on60);	// s = ss.sss / 60
	dn_mulpow10(&s, &s, 2);
	dn_add(&t, &m, &s);			// s = mm + ss.sss / 60
	dn_multiply(&m, &t, &const_1on60);
	decNumberTrunc(&s, x);			// s = hh
	dn_add(res, &m, &s);
	return res;
}

decNumber *decNumberHR2HMS(decNumber *res, const decNumber *x) {
	decNumber m, s, t;

	decNumberFrac(&t, x);			// t = .mmssss
	dn_multiply(&s, &t, &const_60);		// s = mm.ssss
	decNumberTrunc(&m, &s);			// m = mm
	decNumberFrac(&t, &s);			// t = .ssss
	dn_multiply(&s, &t, &const_0_6);	// scale down by 60/100
	dn_add(&t, &s, &m);			// t = mm.ss
	dn_mulpow10(&m, &t, -2);		// t = .mmss
	decNumberTrunc(&s, x);			// s = hh
	dn_add(res, &m, &s);			// res = hh.mmss
	return res;
}

decNumber *decNumberHMSAdd(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber a, b, c;

	decNumberHMS2HR(&a, x);
	decNumberHMS2HR(&b, y);
	dn_add(&c, &a, &b);
	decNumberHR2HMS(res, &c);
	return res;
}

decNumber *decNumberHMSSub(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber a, b, c;

	decNumberHMS2HR(&a, x);
	decNumberHMS2HR(&b, y);
	dn_subtract(&c, &a, &b);
	decNumberHR2HMS(res, &c);
	return res;
}

decNumber *decNumberParallel(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber p, s;

	dn_multiply(&p, x, y);
	dn_add(&s, x, y);
	dn_divide(res, &p, &s);
	return res;
}

decNumber *decNumberAGM(decNumber *res, const decNumber *x, const decNumber *y) {
	int n;
	decNumber a, g, t, u;

	if (decNumberIsNegative(x) || decNumberIsNegative(y))
		goto nan;
	if (decNumberIsSpecial(x) || decNumberIsSpecial(y)) {
		if (decNumberIsNaN(x) || decNumberIsNaN(y))
			goto nan;
		if (dn_eq0(x) || dn_eq0(y))
			goto nan;
		return set_inf(res);
	}
	decNumberCopy(&a, x);
	decNumberCopy(&g, y);
	for (n=0; n<1000; n++) {
		if (relative_error(&a, &g, &const_1e_32))
			return decNumberCopy(res, &a);

		dn_add(&t, &a, &g);
		dn_div2(&u, &t);

		dn_multiply(&t, &a, &g);
		if (dn_eq0(&t))
			return decNumberZero(res);
		dn_sqrt(&g, &t);
		decNumberCopy(&a, &u);
	}
nan:	return set_NaN(res);
}


/* Logical operations on decNumbers.
 * We treat 0 as false and non-zero as true.
 */
static int dn2bool(const decNumber *x) {
	return dn_eq0(x)?0:1;
}

static decNumber *bool2dn(decNumber *res, int l) {
	if (l)
		dn_1(res);
	else
		decNumberZero(res);
	return res;
}

decNumber *decNumberNot(decNumber *res, const decNumber *x) {
	return bool2dn(res, !dn2bool(x));
}

decNumber *decNumberAnd(decNumber *res, const decNumber *x, const decNumber *y) {
	return bool2dn(res, dn2bool(x) && dn2bool(y));
}

decNumber *decNumberOr(decNumber *res, const decNumber *x, const decNumber *y) {
	return bool2dn(res, dn2bool(x) || dn2bool(y));
}

decNumber *decNumberXor(decNumber *res, const decNumber *x, const decNumber *y) {
	return bool2dn(res, dn2bool(x) ^ dn2bool(y));
}

decNumber *decNumberNand(decNumber *res, const decNumber *x, const decNumber *y) {
	return bool2dn(res, !(dn2bool(x) && dn2bool(y)));
}

decNumber *decNumberNor(decNumber *res, const decNumber *x, const decNumber *y) {
	return bool2dn(res, !(dn2bool(x) || dn2bool(y)));
}

decNumber *decNumberNxor(decNumber *res, const decNumber *x, const decNumber *y) {
	return bool2dn(res, !(dn2bool(x) ^ dn2bool(y)));
}


decNumber *decNumberRnd(decNumber *res, const decNumber *x) {
	int numdig = UState.dispdigs + 1;
	decNumber p10;
	decNumber t, u;
	enum display_modes dmode = UState.dispmode;
	enum rounding round;
	int digits;

	if (decNumberIsSpecial(x))
		return decNumberCopy(res, x);

	if (UState.fract) {
		decNumber2Fraction(&t, &u, x);
		return dn_divide(res, &t, &u);
	}

	if (dmode == MODE_STD) {
		dmode = std_round_fix(x);
		numdig = DISPLAY_DIGITS;
	}

	if (dmode == MODE_FIX) {
		/* FIX is different since the number of digits changes */
#if 0
		/* The slow but always correct way */
		int_to_dn(&u, numdig-1);
		decNumberPow10(&p10, &u);
#else
		/* The much faster but relying on base 10 numbers with exponents */
		dn_1(&p10);
		p10.exponent += numdig-1;
		
#endif
		dn_multiply(&t, x, &p10);
		decNumberRound(&u, &t);
		return dn_divide(res, &u, &p10);
	}

	round = Ctx.round;
	digits = Ctx.digits;

	Ctx.round = DEC_ROUND_HALF_UP;
	Ctx.digits = numdig;
	decNumberPlus(res, x, &Ctx);
	Ctx.digits = digits;
	Ctx.round = round;
	return res;
}

void decNumber2Fraction(decNumber *n, decNumber *d, const decNumber *x) {
	decNumber z, dold, t, s, maxd;
	int neg;
	enum denom_modes dm;
	int i;

	if (decNumberIsNaN(x)) {
		cmplx_NaN(n, d);
		return;
	}
	if (decNumberIsInfinite(x)) {
		decNumberZero(d);
		if (decNumberIsNegative(x))
			dn__1(n);
		else
			dn_1(n);
		return;
	}

	dm = UState.denom_mode;
	get_maxdenom(&maxd);

	decNumberZero(&dold);
	dn_1(d);
	neg = decNumberIsNegative(x);
	if (neg)
		dn_minus(&z, x);
	else
		decNumberCopy(&z, x);
	switch (dm) {
	case DENOM_ANY:
		/* Do a partial fraction expansion until the denominator is too large */
		for (i=0; i<1000; i++) {
			decNumberTrunc(&t, &z);
			dn_subtract(&s, &z, &t);
			if (dn_eq0(&s))
				break;
			decNumberRecip(&z, &s);
			decNumberTrunc(&s, &z);
			dn_multiply(&t, &s, d);
			dn_add(&s, &t, &dold);	// s is new denominator estimate
			dn_compare(&t, &maxd, &s);
			if (dn_le0(&t))
				break;
			decNumberCopy(&dold, d);
			decNumberCopy(d, &s);
		}
		break;
	default:
		decNumberCopy(d, &maxd);
		break;
	}
	dn_multiply(&t, x, d);
	decNumberRound(n, &t);
	if (dm == DENOM_FACTOR) {
		decNumberGCD(&t, n, d);
		dn_divide(n, n, &t);
		dn_divide(d, d, &t);
	}
	if (neg)
		dn_minus(n, n);
}

decNumber *decNumberFib(decNumber *res, const decNumber *x) {
	decNumber r, s, t;

	dn_power(&r, &const_phi, x);
	dn_mulPI(&t, x);
	dn_sincos(&t, NULL, &s);
	dn_divide(&t, &s, &r);
	dn_subtract(&s, &r, &t);
	return dn_multiply(res, &s, &const_recipsqrt5);
}


static decNumber *gser(decNumber *res, const decNumber *a, const decNumber *x, const decNumber *gln) {
	decNumber ap, del, sum, t, u;
	int i;

	if (dn_le0(x))
		return decNumberZero(res);
	decNumberCopy(&ap, a);
	decNumberRecip(&sum, a);
	decNumberCopy(&del, &sum);
	for (i=0; i<500; i++) {
		dn_inc(&ap);
		dn_divide(&t, x, &ap);
		dn_multiply(&del, &del, &t);
		dn_add(&t, &sum, &del);
		dn_compare(&u, &t, &sum);
		if (dn_eq0(&u)) {
			dn_ln(&t, x);
			dn_multiply(&u, &t, a);
			dn_subtract(&t, &u, x);
			dn_subtract(&u, &t, gln);
			dn_exp(&t, &u);
			return dn_multiply(res, &sum, &t);
		}
		decNumberCopy(&sum, &t);
	}
	return decNumberZero(res);
}

static decNumber *gcf(decNumber *res, const decNumber *a, const decNumber *x, const decNumber *gln) {
	decNumber an, b, c, d, h, t, u, v, i;
	int n;

	dn_p1(&t, x);
	dn_subtract(&b, &t, a);			// b = (x-1) a
	decNumberCopy(&c, &const_1e32);
	decNumberRecip(&d, &b);
	decNumberCopy(&h, &d);
	decNumberZero(&i);
	for (n=0; n<500; n++) {
		dn_inc(&i);
		dn_subtract(&t, a, &i);		// t = a-i
		dn_multiply(&an, &i, &t);		// an = -i (i-a)
		dn_p2(&b, &b);
		dn_multiply(&t, &an, &d);
		dn_add(&v, &t, &b);
		dn_abs(&t, &v);
			dn_compare(&u, &t, &const_1e_32);
			if (decNumberIsNegative(&u))
				decNumberCopy(&d, &const_1e32);
			else
				decNumberRecip(&d, &v);
			dn_divide(&t, &an, &c);
			dn_add(&c, &b, &t);
			dn_abs(&t, &c);
			dn_compare(&u, &t, &const_1e_32);
			if (decNumberIsNegative(&u))
				decNumberCopy(&c, &const_1e_32);
		dn_multiply(&t, &d, &c);
		dn_multiply(&u, &h, &t);
		dn_compare(&t, &h, &u);
		if (dn_eq0(&t))
			break;
		decNumberCopy(&h, &u);
//		dn_m1(&u, &t);
//		dn_abs(&t, &u);
//		dn_compare(&u, &t, &const_1e_32);
//		if (decNumberIsNegative(&u))
//			break;
	}
	dn_ln(&t, x);
	dn_multiply(&u, &t, a);
	dn_subtract(&t, &u, x);
	dn_subtract(&u, &t, gln);
	dn_exp(&t, &u);
	return dn_multiply(res, &t, &h);
}

decNumber *decNumberGammap(decNumber *res, const decNumber *a, const decNumber *x) {
	decNumber z, lga;

	if (decNumberIsNegative(x) || dn_le0(a) ||
			decNumberIsNaN(x) || decNumberIsNaN(a) || decNumberIsInfinite(a)) {
		return set_NaN(res);
	}
	if (decNumberIsInfinite(x))
		return dn_1(res);
	if (dn_eq0(x))
		return decNumberZero(res);

	dn_p1(&lga, a);
	dn_compare(&z, x, &lga);
	decNumberLnGamma(&lga, a);
	if (decNumberIsNegative(&z))
		return gser(res, a, x, &lga);
	else {
		gcf(&z, a, x, &lga);
		return dn_1m(res, &z);
	}
}

decNumber *decNumberERF(decNumber *res, const decNumber *x) {
	decNumber z;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			return set_NaN(res);
		if (decNumberIsNegative(x))
			return dn__1(res);
		return dn_1(res);
	}
	decNumberSquare(&z, x);
	decNumberGammap(res, &const_0_5, &z);
	if (decNumberIsNegative(x))
		return dn_minus(res, res);
	return res;
	
}

decNumber *decNumberERFC(decNumber *res, const decNumber *x) {
	decNumber a, b;

	dn_divide(&a, x, &const_root2on2);
	dn_minus(&b, &a);
	cdf_Q(&a, &b);
	return dn_mul2(res, &a);
}


/* Jacobi Elliptical functions */
#ifdef INCLUDE_ELLIPTIC
#define ELLIPTIC_N	16

void dn_elliptic(decNumber *sn, decNumber *cn, decNumber *dn, const decNumber *u, const decNumber *m) {
#ifndef TINY_BUILD
	decNumber a, b, e, f, g;
	decNumber s_n, c_n, d_n;
	decNumber MU[ELLIPTIC_N], NU[ELLIPTIC_N], C[ELLIPTIC_N], D[ELLIPTIC_N];
	decNumber sin_umu, cos_umu, t, r;
	int n = 0;
	
#define mu(n)	(MU + (n))
#define nu(n)	(NU + (n))
#define c(n)	(C + (n))
#define d(n)	(D + (n))

	if (sn == NULL) sn = &s_n;
	if (cn == NULL) cn = &c_n;
	if (dn == NULL) dn = &d_n;

	dn_abs(&a, m);
	dn_compare(&b, &const_1, &a);
	if (decNumberIsNegative(&b)) {
		cmplx_NaN(sn, cn);
		set_NaN(dn);
		return;
	}
	dn_compare(&b, &a, &const_1e_32);
	if (decNumberIsNegative(&b)) {
		dn_sincos(u, sn, cn);
		dn_1(dn);
		return;
	}
	dn_m1(&a, m);
	dn_abs(&b, &a);
	dn_compare(&a, &b, &const_1e_32);
	if (decNumberIsNegative(&a)) {
		dn_sinhcosh(u, &a, &b);
		decNumberRecip(cn, &b);
		dn_multiply(sn, &a, cn);
		decNumberCopy(dn, cn);
		return;
	}
	dn_1(mu(0));
	dn_1m(&a, m);
	dn_sqrt(nu(0), &a);
	for (;;) {
		dn_add(&g, mu(n), nu(n));
		dn_abs(&a, &g);
		dn_mulpow10(&b, &a, 32);
		dn_mul2(&a, &b);
		dn_subtract(&e, mu(n), nu(n));
		dn_abs(&f, &e);
		dn_compare(&e, &a, &f);
		if (!decNumberIsNegative(&e) && !dn_eq0(&e))
			break;
		dn_div2(mu(n+1), &g);
		dn_multiply(&a, mu(n), nu(n));
		dn_sqrt(nu(n+1), &a);
		n++;
		if (n >= ELLIPTIC_N-1)
			break;
	}

	dn_multiply(&a, u, mu(n));
	dn_sincos(&a, &sin_umu, &cos_umu);
	dn_abs(&a, &sin_umu);
	dn_abs(&b, &cos_umu);
	dn_compare(&e, &a, &b);
	if (decNumberIsNegative(&e))
		dn_divide(&t, &sin_umu, &cos_umu);
	else
		dn_divide(&t, &cos_umu, &sin_umu);

	dn_multiply(c(n), mu(n), &t);
	dn_1(d(n));

	while (n > 0) {
		n--;
		dn_multiply(c(n), d(n+1), c(n+1));
		decNumberSquare(&a, c(n+1));
		dn_divide(&r, &a, mu(n+1));
		dn_add(&a, &r, nu(n));
		dn_add(&b, &r, mu(n));
		dn_divide(d(n), &a, &b);
	}
	cmplxAbs(&f, &b, &const_1, c(0));
	if (decNumberIsNegative(&e)) {
		dn_1m(&a, m);
		dn_sqrt(&g, &a);
		dn_divide(dn, &g, d(0));

		dn_divide(cn, dn, &f);
		if (decNumberIsNegative(&cos_umu))
			dn_minus(cn, cn);

		dn_divide(&a, c(0), &g);
		dn_multiply(sn, cn, &a);
	} else {
		decNumberCopy(dn, d(0));

		dn_divide(sn, &const_1, &f);
		if (decNumberIsNegative(&sin_umu))
			dn_minus(sn, sn);
		dn_multiply(cn, c(0), sn);
	}
#undef mu
#undef nu
#undef c
#undef d
#endif
}

decNumber *decNumberSN(decNumber *res, const decNumber *k, const decNumber *u) {
	dn_elliptic(res, NULL, NULL, u, k);
	return res;
}

decNumber *decNumberCN(decNumber *res, const decNumber *k, const decNumber *u) {
	dn_elliptic(NULL, res, NULL, u, k);
	return res;
}

decNumber *decNumberDN(decNumber *res, const decNumber *k, const decNumber *u) {
	dn_elliptic(NULL, NULL, res, u, k);
	return res;
}
#endif


#ifdef INCLUDE_BESSEL
#ifndef TINY_BUILD
static decNumber *dn_bessel(decNumber *res, const decNumber *alpha, const decNumber *x, const int neg) {
	decNumber q, r, m;
	decNumber x2on4, term, gfac;
	int n;

	dn_div2(&q, x);				// q = x/2
	decNumberSquare(&x2on4, &q);		// factor each time around
	dn_power(&r, &q, alpha);		// (x/2)^(2m+alpha)

	dn_p1(&gfac, alpha);
	decNumberGamma(&q, &gfac);
	dn_divide(&term, &r, &q);
	decNumberCopy(res, &term);			// first term in series

	decNumberZero(&m);

	for (n=0; n<1000; n++) {
		dn_multiply(&q, &term, &x2on4);
		dn_inc(&m);			// m = m+1
		dn_divide(&r, &q, &m);
		dn_divide(&term, &r, &gfac);
		dn_inc(&gfac);
		if (neg)
			dn_minus(&term, &term);
		dn_add(&q, &term, res);
		dn_compare(&r, &q, res);
		if (dn_eq0(&r))
			return res;
		decNumberCopy(res, &q);
	}
	return set_NaN(res);
}
#endif

decNumber *decNumberBSJN(decNumber *res, const decNumber *alpha, const decNumber *x) {
#ifndef TINY_BUILD
	decNumber a;

	if (decNumberIsNaN(alpha) || decNumberIsSpecial(x))
		return set_NaN(res);
	if (dn_eq0(x)) {
		if (dn_eq0(alpha))
			return dn_1(res);
		return decNumberZero(res);
	}
	if (decNumberIsNegative(alpha) && is_int(alpha)) {
		dn_abs(&a, alpha);
		alpha = &a;
	}
	dn_bessel(res, alpha, x, 1);
	return res;
#else
	return NULL;
#endif
}

decNumber *decNumberBSIN(decNumber *res, const decNumber *alpha, const decNumber *x) {
#ifndef TINY_BUILD
	decNumber a;

	if (decNumberIsNaN(alpha) || decNumberIsNaN(x))
		return set_NaN(res);
	else if (decNumberIsInfinite(x))
		return set_inf(res);
	else if (dn_eq0(x)) {
		if (dn_eq0(alpha))
			return dn_1(res);
		else
			return decNumberZero(res);
	} else {
		if (decNumberIsNegative(alpha) && is_int(alpha)) {
			dn_abs(&a, alpha);
			alpha = &a;
		}
		dn_bessel(res, alpha, x, 0);
	}
	return res;
#else
	return NULL;
#endif
}

#ifndef TINY_BUILD
// See A&S page 360 section 9.1.11
static void bessel2_int_series(decNumber *res, const decNumber *n, const decNumber *x, int modified) {
	const decNumber *const factor = modified?&const_0_5:&const__1onPI;
	decNumber xon2, xon2n, x2on4;
	decNumber k, npk, t, u, v, s, p, nf, absn;
	int i, in, n_odd, n_neg;

	if (decNumberIsNegative(n)) {
		n = dn_abs(&absn, n);
		n_neg = 1;
	} else	n_neg = 0;
	in = dn_to_int(n);
	n_odd = in & 1;

	dn_div2(&xon2, x);			// xon2 = x/2
	dn_power(&xon2n, &xon2, n);		// xon2n = (x/2)^n
	decNumberSquare(&x2on4, &xon2);		// x2on4 = +/- x^2/4

	if (modified)
		dn_minus(&x2on4, &x2on4);
	if (in > 0) {
		dn_m1(&v, n);			// v = n-k-1 = n-1
		decNumberZero(&k);
		decNumberGamma(&p, n);		// p = (n-1)!
		decNumberCopy(&s, &p);
		dn_multiply(&nf, &p, n);	// nf = n!  (for later)
		for (i=1; i<in; i++) {
			dn_divide(&t, &p, &v);
			dn_dec(&v);
			dn_inc(&k);
			dn_multiply(&u, &t, &k);
			dn_multiply(&p, &u, &x2on4);
			dn_add(&s, &s, &p);
		}
		dn_multiply(&t, &s, factor);
		dn_divide(res, &t, &xon2n);
	} else {
		decNumberZero(res);
		dn_1(&nf);
	}

	if (modified) {
		decNumberBSIN(&t, n, x);
		if (!n_odd)
			dn_minus(&t, &t);
	} else {
		decNumberBSJN(&u, n, x);
		dn_divide(&t, &u, &const_PIon2);
	}
	dn_ln(&u, &xon2);
	dn_multiply(&v, &u, &t);
	dn_add(res, res, &v);

	dn_minus(&x2on4, &x2on4);
	dn_p1(&t, n);				// t = n+1
	decNumberPsi(&u, &t);			// u = Psi(n+1)
	dn_subtract(&v, &u, &const_egamma);	// v = psi(k+1) + psi(n+k+1)
	decNumberZero(&k);
	decNumberCopy(&npk, n);
	decNumberRecip(&p, &nf);			// p = (x^2/4)^k/(k!(n+k)!)
	dn_multiply(&s, &v, &p);

	for (i=0;i<1000;i++) {
		dn_inc(&k);
		dn_inc(&npk);
		dn_multiply(&t, &p, &x2on4);
		dn_multiply(&u, &k, &npk);
		dn_divide(&p, &t, &u);

		decNumberRecip(&t, &k);
		dn_add(&u, &v, &t);
		decNumberRecip(&t, &npk);
		dn_add(&v, &u, &t);

		dn_multiply(&t, &v, &p);
		dn_add(&u, &t, &s);
		dn_compare(&t, &u, &s);
		if (dn_eq0(&t))
			break;
		decNumberCopy(&s, &u);
	}
	dn_multiply(&t, &s, &xon2n);
	if (modified) {
		if (n_odd)
			dn_multiply(&u, &t, &const__0_5);
		else
			dn_div2(&u, &t);
	} else
		dn_multiply(&u, &t, &const__1onPI);
	dn_add(res, res, &u);
	if (!modified && n_neg)
		dn_minus(res, res);
}
#endif


decNumber *decNumberBSYN(decNumber *res, const decNumber *alpha, const decNumber *x) {
#ifndef TINY_BUILD
	decNumber t, u, s, c;

	if (decNumberIsNaN(alpha) || decNumberIsSpecial(x))
		return set_NaN(res);
	else if (dn_eq0(x))
		return set_neginf(res);
	else if (decNumberIsInfinite(alpha) || decNumberIsNegative(x))
		return set_NaN(res);
	else if (!is_int(alpha)) {
		dn_mulPI(&t, alpha);
		dn_sincos(&t, &s, &c);
		dn_bessel(&t, alpha, x, 1);
		dn_multiply(&u, &t, &c);
		dn_minus(&c, alpha);
		dn_bessel(&t, &c, x, 1);
		dn_subtract(&c, &u, &t);
		dn_divide(res, &c, &s);
	} else
		bessel2_int_series(res, alpha, x, 0);
	return res;
#else
	return NULL;
#endif
}

decNumber *decNumberBSKN(decNumber *res, const decNumber *alpha, const decNumber *x) {
#ifndef TINY_BUILD
	decNumber t, u, v;

	if (decNumberIsNaN(alpha) || decNumberIsNaN(x))
		return set_NaN(res);
	else if (dn_eq0(x))
		return set_inf(res);
	else if (decNumberIsInfinite(alpha) || decNumberIsNegative(x))
		return set_NaN(res);
	else if (decNumberIsInfinite(x))
		return decNumberZero(res);
	else if (!is_int(alpha)) {
		dn_bessel(&t, alpha, x, 0);
		dn_minus(&u, alpha);
		dn_bessel(&v, &u, x, 0);
		dn_subtract(&u, &v, &t);
		dn_multiply(&v, &u, &const_PIon2);

		dn_mulPI(&t, alpha);
		dn_sincos(&t, &u, NULL);
		dn_divide(res, &v, &u);
	} else
		bessel2_int_series(res, alpha, x, 1);
	return res;
#else
	return NULL;
#endif
}
#endif


/* Solver code from here */


/* Secant iteration */
static void solve_secant(decNumber *s, const decNumber *a, const decNumber *b, const decNumber *fa, const decNumber *fb) {
	decNumber x, y, z;

	dn_subtract(&x, b, a);
	dn_subtract(&y, fb, fa);
	dn_divide(&z, &x, &y);
	dn_multiply(&x, &z, fb);
	dn_subtract(s, b, &x);
}

/* A third of the inverse quadratic interpolation step.
 * Return non-zero is one of the denominators is zero.
 */
static int qstep(decNumber *r, const decNumber *a, const decNumber *fb, const decNumber *fc, const decNumber *fa) {
	decNumber x, y, z;

	dn_subtract(&x, fa, fb);
	if (dn_eq0(&x))
		return -1;
	dn_subtract(&y, fa, fc);
	if (dn_eq0(&y))
		return -1;
	dn_multiply(&z, &x, &y);
	dn_multiply(&x, a, fb);
	dn_multiply(&y, &x, fc);
	dn_divide(r, &y, &z);
	return 0;
}

/* Inverse quadratic interpolation.
 * Return non-zero if interpolation fails due to equal function values
 */
static int solve_quadratic(decNumber *s, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *fa, const decNumber *fb, const decNumber *fc) {
	decNumber x, y, z;

	if (qstep(&x, a, fb, fc, fa))
		return -1;
	if (qstep(&y, b, fa, fc, fb))
		return -1;
	dn_add(&z, &x, &y);
	qstep(&x, c, fa, fb, fc);
	dn_add(s, &z, &x);
	return 0;
}

#ifdef USE_RIDDERS
/* Ridder's method
 */
static int solve_ridder(decNumber *xnew, const decNumber *x0, const decNumber *x2, const decNumber *x1, const decNumber *y0, const decNumber *y2, const decNumber *y1) {
	decNumber a, b, r, s, t, u, v;

	dn_subtract(&r, y1, y2);
	if (dn_eq0(&r))
		return -1;
	dn_subtract(&s, y0, y1);		// s = y0 - y1
	dn_divide(&a, &s, &r);
	dn_multiply(&t, &a, y1);
	dn_subtract(&r, y0, &t);
	if (dn_eq0(&r))
		return -1;
	dn_divide(&b, &s, &r);
	dn_p1(&r, &b);
	if (dn_eq0(&r))
		return -1;
	dn_m1(&s, &b);
	dn_divide(&u, &s, &r);
	dn_p1(&r, &a);
	if (dn_eq0(&r))
		return -1;
	dn_m1(&s, &a);
	dn_divide(&v, &s, &r);

	decNumberSquare(&r, &v);
	dn_add(&s, &r, &const_3);
	dn_multiply(&t, &s, &v);
	decNumberSquare(&r, &u);
	dn_add(&s, &r, &const_3);
	dn_multiply(&r, &s, &u);
	dn_divide(&s, &r, &t);
	dn_subtract(&r, x1, x0);
	dn_abs(&t, &r);
	dn_multiply(&r, &t, &s);
	dn_add(xnew, &r, x1);
	return 0;
}
#endif


/* perform a bisection step.
 */
static void solve_bisect(decNumber *s, const decNumber *a, const decNumber *b) {
	decNumber x;

	dn_add(&x, a, b);
	dn_div2(s, &x);
}


/* Check if the new point is inside the bracketed interval, if not do a bisection
 * step instead.  This means we'll not escape a bracketed interval ever.
 */
static int solve_bracket(decNumber *s, const decNumber *a, const decNumber *b) {
	decNumber x, y;

	dn_subtract(&x, a, b);
	if (decNumberIsNegative(&x)) {		// a<b
		dn_subtract(&y, s, a);
		dn_subtract(&x, b, s);
	} else {				// a>b
		dn_subtract(&y, s, b);
		dn_subtract(&x, a, s);
	}
	/* If out of bracket or the same as a previous, out of bracket */
	return (dn_le0(&y) || dn_le0(&x));
}


/* Limit the distance a new estimate can be to within 100 times the distance
 * between the existing points.
 */
static void limit_jump(decNumber *s, const decNumber *a, const decNumber *b) {
	decNumber x, y, z;

	dn_subtract(&x, a, b);
	dn_abs(&y, &x);
	dn_mul100(&x, &y);			// 100 |a-b|
	dn_subtract(&y, a, &x);
	dn_compare(&z, s, &y);
	if (decNumberIsNegative(&z)) {
		decNumberCopy(s, &z);
		return;
	}
	dn_add(&y, b, &x);
	dn_compare(&z, &y, s);
	if (decNumberIsNegative(&z))
		decNumberCopy(s, &z);
}

/* Swap two numbers in place
 */
void decNumberSwap(decNumber *a, decNumber *b) {
	decNumber z;

	decNumberCopy(&z, a);
	decNumberCopy(a, b);
	decNumberCopy(b, &z);
}


/*; two numbers to see if they are mostly equal.
 * Return non-zero if they are the same.
 */
static int slv_compare(const decNumber *a, const decNumber *b, const decNumber *tol) {
	decNumber ar, br, c;

	decNumberRnd(&ar, a);
	decNumberRnd(&br, b);
	return dn_eq0(dn_compare(&c, &ar, &br));
}

/* Define how with use the flags.
 * The bottom bit is reserved for the xrom code.
 * The second bottom bit indicates if we've bracketed a solution (set if so).
 *
 * If we've bracketed, the upper bits are an iteration counter.
 * If not, we encode more state:
 */

#define _FLAG_BRACKET	(1)
#define _FLAG_CONST	(2)
#ifdef USE_RIDDERS
#define _FLAG_BISECT	(4)
#endif
#define SLV_COUNT(f)		((f) >> 8)
#define SLV_SET_COUNT(f, c)	(((f) & 0xff) | ((c) << 8))

#define IS_BRACKET(f)		((f) & _FLAG_BRACKET)
#define SET_BRACKET(f)		(f) |= _FLAG_BRACKET
#define CLEAR_BRACKET(f)	(f) &= ~_FLAG_BRACKET
#define BRACKET_MAXCOUNT	150

#define IS_CONST(f)		((f) & _FLAG_CONST)
#define SET_CONST(f)		(f) |= _FLAG_CONST
#define CLEAR_CONST(f)		(f) &= ~_FLAG_CONST
#define CONST_MAXCOUNT		20

#define IS_ONESIDE(f)		(((f) & (_FLAG_BRACKET + _FLAG_CONST)) == 0)
#define ONESIDE_MAXCOUNT	100

#ifdef USE_RIDDERS
#define IS_BISECT(f)		((f) & _FLAG_BISECT)
#define CLEAR_BISECT(f)		(f) &= ~_FLAG_BISECT
#define SET_BISECT(f)		(f) |= _FLAG_BISECT
#endif

int solver_step(decNumber *a, decNumber *b, decNumber *c,
		decNumber *fa, decNumber *fb, const decNumber *fc,
		unsigned int *statep, int (*compare)(const decNumber *, const decNumber *, const decNumber *)) {
	decNumber q, x, y, z;
	int s1, s2;
	unsigned int slv_state = *statep;
	int count, r;
#ifdef USE_RIDDERS
	const int was_bisect = IS_BISECT(slv_state);
	CLEAR_BISECT(slv_state);
#endif
	if (IS_BRACKET(slv_state)) {
		count = SLV_COUNT(slv_state);
		if (count >= BRACKET_MAXCOUNT)
			goto failed;
		slv_state = SLV_SET_COUNT(slv_state, count+1);
brcket:
#ifdef USE_RIDDERS

		if (was_bisect)
			r = solve_ridder(&q, a, b, c, fa, fb, fc);
		else
#endif
			r = solve_quadratic(&q, a, b, c, fa, fb, fc);
		s1 = decNumberIsNegative(fc);
		s2 = decNumberIsNegative(fb);
		if (s1 != s2) {
			decNumberCopy(a, c);
			decNumberCopy(fa, fc);
			dn_multiply(&y, b, &const_3);
		} else {
			decNumberCopy(b, c);
			decNumberCopy(fb, fc);
			dn_multiply(&y, a, &const_3);
		}
		dn_add(&z, &y, c);
		dn_multiply(&y, &z, &const_0_25);
		if (r)
			solve_secant(&q, a, b, fa, fb);
		if (solve_bracket(&q, &y, c)) {
			solve_bisect(&q, a, b);
#ifdef USE_RIDDERS
			SET_BISECT(slv_state);
#endif
		}
	} else {
		s1 = decNumberIsNegative(fc);
		s2 = decNumberIsNegative(fb);
		if (s1 != s2) {
			SET_BRACKET(slv_state);
			CLEAR_CONST(slv_state);
			r = -1;
			goto brcket;
		}
		if (IS_CONST(slv_state)) {
			count = SLV_COUNT(slv_state);
			if (count >= CONST_MAXCOUNT)
				goto failed;

			dn_compare(&x, fb, fc);
			if (! dn_eq0(&x)) {
				slv_state = SLV_SET_COUNT(slv_state, 0);
				CLEAR_CONST(slv_state);
				r = -1;
				goto nonconst;
			}
			slv_state = SLV_SET_COUNT(slv_state, count+1);
			if (count & 1) {
				decNumberCopy(b, c);
				decNumberCopy(fb, fc);
				if (decNumberIsNegative(a))
					dn_mul2(&x, a);
				else	dn_div2(&x, a);
				dn_subtract(&q, &x, &const_10);
			} else {
				decNumberCopy(a, c);
				decNumberCopy(fa, fc);
				if (decNumberIsNegative(b))
					dn_div2(&x, b);
				else	dn_mul2(&x, b);
				dn_add(&q, &x, &const_10);
			}
		} else {
			count = SLV_COUNT(slv_state);
			if (count >= ONESIDE_MAXCOUNT)
				goto failed;
			slv_state = SLV_SET_COUNT(slv_state, count+1);
nonconst:
			r = solve_quadratic(&q, a, b, c, fa, fb, fc);
			//MORE: need to check if the new point is worse than the old.
			dn_abs(&x, fa);
			dn_abs(&y, fb);
			dn_compare(&z, &x, &y);
			if (decNumberIsNegative(&z)) {
				decNumberCopy(a, c);
				decNumberCopy(fa, fc);
			} else {
				decNumberCopy(b, c);
				decNumberCopy(fb, fc);
			}
			dn_compare(&x, b, a);
			if (decNumberIsNegative(&x)) {
				decNumberSwap(a, b);
				decNumberSwap(fa, fb);
			}
			if (r)
				solve_secant(&q, a, b, fa, fb);
			limit_jump(&q, a, b);
		}
	}
	if (compare(c, &q, &const_1e_24)) goto failed;
	if (compare(a, &q, &const_1e_24)) goto failed;
	if (compare(b, &q, &const_1e_24)) goto failed;
	decNumberCopy(c, &q);
	*statep = slv_state;
	return 0;
failed:
	*statep = slv_state;
	return -1;
}

void solver_init(decNumber *c, decNumber *a, decNumber *b, decNumber *fa, decNumber *fb, unsigned int *flagp) {
	int sa, sb;
	decNumber y;
	unsigned int flags = 0;

	/*
	if (decNumberIsNegative(dn_compare(&y, b, a))) {
		decNumberSwap(a, b);
		decNumberSwap(fa, fb);
	}
	*/
	sa = decNumberIsNegative(fa);
	sb = decNumberIsNegative(fb);
	if (sa == sb) {				// Same side of line
		dn_compare(&y, fa, fb);
		if (dn_eq0(&y)) {	// Worse equal...
			SET_CONST(flags);
		}
		// Both estimates are the same side of the line.
#if 1
		// A bisection step puts a degree of trust in the user's
		// estimates.
		solve_bisect(c, a, b);
#else
		// A secant step, this runs the risk of flying off
		// into infinity and having to work its way back again.
		solve_secant(c, a, b, fa, fb);
		limit_jump(c, a, b);
#endif
	} else {
		SET_BRACKET(flags);
		solve_secant(c, a, b, fa, fb);
		if (solve_bracket(c, a, b)) {
			solve_bisect(c, a, b);
#ifdef USE_RIDDERS
			SET_BISECT(flags);
#endif
		}
	}
	*flagp = flags;
}

// User code flag numbers
#define _FLAG_BRACKET_N	8
#define _FLAG_CONST_N	9
#ifdef USE_RIDDERS
#define _FLAG_BISECT_N	10
#endif
#define _FLAG_COUNT_N	0	/* 0 - 7, eight flags in all */

// User code interface to the solver
void solver(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	decNumber a, b, c, fa, fb, fc;
	unsigned int flags;
	int r;

	get_reg_n_as_dn(LOCAL_REG_BASE + 0, &a);
	get_reg_n_as_dn(LOCAL_REG_BASE + 1, &b);
	get_reg_n_as_dn(LOCAL_REG_BASE + 3, &fa);
	get_reg_n_as_dn(LOCAL_REG_BASE + 4, &fb);

	if (op == OP_INISOLVE) {
		solver_init(&c, &a, &b, &fa, &fb, &flags);
	} else {
		get_reg_n_as_dn(LOCAL_REG_BASE + 2, &c);
		flags = 0;
		for (r=0; r<8; r++)
			if (get_user_flag(LOCAL_FLAG_BASE + r + _FLAG_COUNT_N))
				flags |= 1<<r;
		flags = SLV_SET_COUNT(0, flags);

		if (get_user_flag(LOCAL_FLAG_BASE + _FLAG_BRACKET_N))
			SET_BRACKET(flags);
		if (get_user_flag(LOCAL_FLAG_BASE + _FLAG_CONST_N))
			SET_CONST(flags);
#ifdef USE_RIDDERS
		if (get_user_flag(NUMFLG + _FLAG_BISECT_N))
			SET_BISECT(flags);
#endif

		getX(&fc);
		r = solver_step(&a, &b, &c, &fa, &fb, &fc, &flags, &slv_compare);
		setX(r==0?&const_0:&const_1);
	}

	put_reg_n(LOCAL_REG_BASE + 0, &a);
	put_reg_n(LOCAL_REG_BASE + 1, &b);
	put_reg_n(LOCAL_REG_BASE + 2, &c);
	put_reg_n(LOCAL_REG_BASE + 3, &fa);
	put_reg_n(LOCAL_REG_BASE + 4, &fb);

	put_user_flag(LOCAL_FLAG_BASE + _FLAG_BRACKET_N, IS_BRACKET(flags));
	put_user_flag(LOCAL_FLAG_BASE + _FLAG_CONST_N, IS_CONST(flags));
#ifdef USE_RIDDERS
	put_user_flag(LOCAL_FLAG_BASE + _FLAG_BISECT_N, IS_BISECT(flags));
#endif
	flags = SLV_COUNT(flags);
	for (r=0; r<8; r++)
		put_user_flag(LOCAL_FLAG_BASE + r + _FLAG_COUNT_N, flags & (1<<r));
}

/**********************************************************************/
/* Orthogonal polynomial evaluations                                  */
/**********************************************************************/

// Orthogonal polynomial types
enum eOrthoPolys {
	ORTHOPOLY_LEGENDRE_PN,
	ORTHOPOLY_CHEBYCHEV_TN,
	ORTHOPOLY_CHEBYCHEV_UN,
	ORTHOPOLY_GEN_LAGUERRE,
	ORTHOPOLY_HERMITE_HE,
	ORTHOPOLY_HERMITE_H,
};

static decNumber *ortho_poly(decNumber *r, const decNumber *param, const decNumber *rn, const decNumber *x, const enum eOrthoPolys type) {
#ifndef TINY_BUILD
	decNumber t0, t1, t, u, v, A, B, C, dA;
	unsigned int i, n;
	int incA, incB, incC;

	// Get argument and parameter
	if (decNumberIsSpecial(x) || decNumberIsSpecial(rn) || dn_lt0(rn)) {
error:		return set_NaN(r);
	}
	if (! is_int(rn))
		goto error;
	n = dn_to_int(rn);
	if (n > 1000)
		goto error;
//	if (type == ORTHOPOLY_GEN_LAGUERRE) {
	if (param != NULL) {
		if (decNumberIsSpecial(param))
			goto error;
		dn_p1(&t, param);
		if (dn_le0(&t))
			goto error;
	}
//	} else
//		param = &const_0;

	// Initialise the first two values t0 and t1
	switch (type) {
	default:
		decNumberCopy(&t1, x);
		break;
	case ORTHOPOLY_HERMITE_H:
	case ORTHOPOLY_CHEBYCHEV_UN:
		dn_mul2(&t1, x);
		break;
	case ORTHOPOLY_GEN_LAGUERRE:
		dn_p1(&t, param);
		dn_subtract(&t1, &t, x);
		break;
	}
	dn_1(&t0);

	if (n < 2) {
		if (n == 0)
			decNumberCopy(r, &t0);
		else
			decNumberCopy(r, &t1);
		return r;
	}

	// Prepare for the iteration
	decNumberCopy(&dA, &const_2);
	dn_1(&C);
	dn_1(&B);
	dn_mul2(&A, x);
	incA = incB = incC = 0;
	switch (type) {
	case ORTHOPOLY_LEGENDRE_PN:
		incA = incB = incC = 1;
		dn_add(&A, &A, x);
		dn_mul2(&dA, x);
		break;
	case ORTHOPOLY_CHEBYCHEV_TN:	break;
	case ORTHOPOLY_CHEBYCHEV_UN:	break;
	case ORTHOPOLY_GEN_LAGUERRE:
		dn_add(&B, &B, param);
		incA = incB = incC = 1;
		dn_add(&t, &const_3, param);
		dn_subtract(&A, &t, x);
		break;
	case ORTHOPOLY_HERMITE_HE:
		decNumberCopy(&A, x);
		incB = 1;
		break;
	case ORTHOPOLY_HERMITE_H:
		decNumberCopy(&B, &const_2);
		incB = 2;
		break;
	}

	// Iterate
	for (i=2; i<=n; i++) {
		dn_multiply(&t, &t1, &A);
		dn_multiply(&u, &t0, &B);
		dn_subtract(&v, &t, &u);
		decNumberCopy(&t0, &t1);
		if (incC) {
			dn_inc(&C);
			dn_divide(&t1, &v, &C);
		} else
			decNumberCopy(&t1, &v);
		if (incA)
			dn_add(&A, &A, &dA);
		if (incB)
			dn_add(&B, &B, small_int(incB));
	}
	return decNumberCopy(r, &t1);
#else
	return NULL;
#endif
}

decNumber *decNumberPolyPn(decNumber *r, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, NULL, y, x, ORTHOPOLY_LEGENDRE_PN);
}

decNumber *decNumberPolyTn(decNumber *r, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, NULL, y, x, ORTHOPOLY_CHEBYCHEV_TN);
}

decNumber *decNumberPolyUn(decNumber *r, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, NULL, y, x, ORTHOPOLY_CHEBYCHEV_UN);
}

decNumber *decNumberPolyLn(decNumber *r, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, &const_0, y, x, ORTHOPOLY_GEN_LAGUERRE);
}

decNumber *decNumberPolyLnAlpha(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, z, y, x, ORTHOPOLY_GEN_LAGUERRE);
}

decNumber *decNumberPolyHEn(decNumber *r, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, NULL, y, x, ORTHOPOLY_HERMITE_HE);
}

decNumber *decNumberPolyHn(decNumber *r, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, NULL, y, x, ORTHOPOLY_HERMITE_H);
}


#ifdef INCLUDE_BERNOULLI
decNumber *decNumberBernBn(decNumber *r, const decNumber *n) {
#ifndef TINY_BUILD
	decNumber a, b;

	if (dn_eq0(n))
		return dn_1(r);
	if (! is_int(n) || decNumberIsNegative(n)) {
		return set_NaN(r);
	}
	dn_1m(&a, n);
	if (dn_eq0(&a))
		return decNumberCopy(r, &const__0_5);
	if (is_even(n)) {
#if 0
		// Values 306 and larger give infinite results, this is shortcut code
		// to save some time -- using 360 as the threshold to reuse a constant.
		if (decNumberIsNegative(dn_compare(&b, &const_360, n))) {
			dn_div2(&b, n);
			if (is_even(&b))
				return set_neginf(r);
			return set_inf(r);
		}
#endif
		decNumberZeta(&b, &a);
		dn_multiply(&a, &b, n);
		return dn_minus(r, &a);
	}
	decNumberZero(r);
	return r;
#else
	return NULL;
#endif
}

decNumber *decNumberBernBnS(decNumber *r, const decNumber *n) {
#ifndef TINY_BUILD
	decNumber a;

	if (dn_eq0(n)) {
		return set_NaN(r);
	}
	dn_mul2(&a, n);
	decNumberBernBn(r, &a);
	if (is_even(n))
		dn_minus(r, r);
	return r;
#else
	return NULL;
#endif
}
#endif

#ifdef INCLUDE_FACTOR
decNumber *decFactor(decNumber *r, const decNumber *x) {
	int sgn;
	unsigned long long int i;

	if (decNumberIsSpecial(x) || ! is_int(x))
		return set_NaN(r);
	i = dn_to_ull(x, &sgn);
	ullint_to_dn(r, doFactor(i));
	if (sgn)
		dn_minus(r, r);
	return r;
}
#endif

#ifdef INCLUDE_USER_IO
decNumber *decRecv(decNumber *r, const decNumber *x) {
	int to;

	if (decNumberIsSpecial(x) || decNumberIsNegative(x)) {
		to = -1;
		if (decNumberIsInfinite(x) && ! decNumberIsNegative(x))
			to = 0x7fffffff;
	} else
		to = dn_to_int(x);
	int_to_dn(r, recv_byte(to));
	return r;
}
#endif

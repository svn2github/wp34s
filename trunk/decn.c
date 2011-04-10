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

// #define DUMP1

#include "decn.h"
#include "xeq.h"
#include "consts.h"
#include "complex.h"

#ifdef DUMP1
#include <stdio.h>
static FILE *debugf = NULL;

static void open_debug(void) {
	if (debugf == NULL) {
		debugf = fopen("wp34s.log", "w");
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


#define MOD_DIGITS  800


/* Define a table of small integers.
 * This should be equal or larger than any of the summation integers required in the
 * various series approximations to avoid needless computation.
 */
#define MAX_SMALL_INT	30
static const decNumber *const small_ints[MAX_SMALL_INT+1] = {
	&const_0, &const_1, &const_2, &const_3, &const_4,
	&const_5, &const_6, &const_7, &const_8, &const_9,
	&const_10, &const_11, &const_12, &const_13, &const_14,
	&const_15, &const_16, &const_17, &const_18, &const_19,
	&const_20, &const_21, &const_22, &const_23, &const_24,
	&const_25, &const_26, &const_27, &const_28, &const_29,
	&const_30
};

const decNumber *small_int(int i) {
	if (i >= 0 && i<= MAX_SMALL_INT)
		return small_ints[i];
	return NULL;
}

void ullint_to_dn(decNumber *x, unsigned long long int n, decContext *ctx) {
	/* Check to see if the number is small enough to be in our table */
	if (n <= MAX_SMALL_INT) {
		decNumberCopy(x, small_ints[n]);
	} else {
		/* Got to do this the long way */
		decNumber p10, z;

		decNumberCopy(&p10, &const_1);
		decNumberZero(x);

		while (n != 0) {
			const int r = n%10;
			n /= 10;
			if (r != 0) {
				decNumberMultiply(&z, &p10, small_ints[r], ctx);
				decNumberAdd(x, x, &z, ctx);
			}
			decNumberMultiply(&p10, &p10, &const_10, ctx);
		}
	}
}

void int_to_dn(decNumber *x, int n, decContext *ctx) {
	int sgn;

	/* Account for negatives */
	if (n < 0) {
		sgn = 1;
		n = -n;
	} else
		sgn = 0;

	ullint_to_dn(x, n, ctx);

	if (sgn)
		decNumberMinus(x, x, ctx);
}

int dn_to_int(const decNumber *x, decContext *ctx) {
	decNumber y;
	char buf[64];

	decNumberRescale(&y, x, &const_0, ctx);
	decNumberToString(&y, buf);
	return s_to_i(buf);
}

unsigned long long int dn_to_ull(const decNumber *x, decContext *ctx, int *sgn) {
	decNumber y;
	char buf[64];

	decNumberRescale(&y, x, &const_0, ctx);
	if (decNumberIsNegative(x)) {
		decNumberMinus(&y, &y, ctx);
		*sgn = 1;
	} else
		*sgn = 0;
	decNumberToString(&y, buf);
	return s_to_ull(buf, 10);
}


void set_NaN(decNumber *x) {
	if (x != NULL)
		decNumberCopy(x, &const_NaN);
}

void set_inf(decNumber *x) {
	decNumberCopy(x, &const_inf);
}

void set_neginf(decNumber *x) {
	decNumberCopy(x, &const__inf);
}


void decNumberPI(decNumber *pi) {
	decNumberCopy(pi, &const_PI);
}
void decNumberPIon2(decNumber *pion2) {
	decNumberCopy(pion2, &const_PIon2);
}

/* Check if a number is an integer.
 */
int is_int(const decNumber *x, decContext *ctx) {
	enum rounding a = ctx->round;
	decNumber r, y;

	if (decNumberIsNaN(x))
		return 0;
	if (decNumberIsInfinite(x))
		return 1;

	ctx->round = DEC_ROUND_DOWN;
	decNumberToIntegralValue(&y, x, ctx);
	ctx->round = a;

	decNumberSubtract(&r, x, &y, ctx);
	if (! decNumberIsZero(&r))
		return 0;
	return 1;
}


void dn_inc(decNumber *x, decContext *ctx) {
	decNumberAdd(x, x, &const_1, ctx);
}

void dn_dec(decNumber *x, decContext *ctx) {
	decNumberSubtract(x, x, &const_1, ctx);
}

/* Multiply Add: x + y * z
 */
#ifdef INCLUDE_MULADD
decNumber *decNumberMAdd(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberMultiply(&t, x, y, ctx);
	return decNumberAdd(r, &t, z, ctx);
}
#endif


/* Reciprocal of a number.
 * Division is correctly rounded so we just use that instead of coding
 * something special (that could be more efficient).
 */
decNumber *decNumberRecip(decNumber *r, const decNumber *x, decContext *ctx) {
	return decNumberDivide(r, &const_1, x, ctx);
}

/* Reciprocal of a function's result.
 * This routine calls the specified function and then multiplicatively
 * inverts its result.
 */
#if 0
static decNumber *dn_recip(decNumber *r, const decNumber *x, decContext *ctx,
		decNumber *(*func)(decNumber *r, const decNumber *x, decContext *ctx)) {
	decNumber z;

	(*func)(&z, x, ctx);
	return decNumberRecip(r, &z, ctx);
}
#endif

/* A plethora of round to integer functions to support the large variety
 * of possibilities in this area.  Start with a common utility function
 * that saves the current rounding mode, rounds as required and restores
 * the rounding mode properly.
 */
static decNumber *round2int(decNumber *r, const decNumber *x, decContext *ctx, int mode) {
	enum rounding a = ctx->round;

	ctx->round = mode;
	decNumberToIntegralValue(r, x, ctx);
	ctx->round = a;
	return r;
}

/* Floor - truncate to minus infinity.
 */
decNumber *decNumberFloor(decNumber *r, const decNumber *x, decContext *ctx) {
	return round2int(r, x, ctx, DEC_ROUND_FLOOR);
}

/* Ceiling - truncate to plus infinity.
 */
decNumber *decNumberCeil(decNumber *r, const decNumber *x, decContext *ctx) {
	return round2int(r, x, ctx, DEC_ROUND_CEILING);
}

/* Trunc - truncate to zero.
 */
decNumber *decNumberTrunc(decNumber *r, const decNumber *x, decContext *ctx) {
	return round2int(r, x, ctx, DEC_ROUND_DOWN);
}

/* Round - round 0.5 up.
 */
decNumber *decNumberRound(decNumber *r, const decNumber *x, decContext *ctx) {
	return round2int(r, x, ctx, DEC_ROUND_HALF_UP);
}

/* Intg - round 0.5 even.
 */
static decNumber *decNumberIntg(decNumber *r, const decNumber *x, decContext *ctx) {
	return round2int(r, x, ctx, DEC_ROUND_HALF_EVEN);
}

/* Frac - round 0.5 up.
 */
decNumber *decNumberFrac(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber y;

	round2int(&y, x, ctx, DEC_ROUND_DOWN);
	return decNumberSubtract(r, x, &y, ctx);
}

decNumber *decNumberSign(decNumber *r, const decNumber *x, decContext *ctx) {
	const decNumber *z;

	if (decNumberIsNaN(x))
		z = x;
	else if (decNumberIsZero(x))
		z = &const_0;
	else if (decNumberIsNegative(x))
		z = &const__1;
	else
		z = &const_1;
	return decNumberCopy(r, z);
}


static void dn_gcd(decNumber *r, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber b, t;

	decNumberCopy(&b, y);
	decNumberCopy(r, x);
	while (! decNumberIsZero(&b)) {
		decNumberCopy(&t, &b);
		decNumberRemainder(&b, r, &t, ctx);
		decNumberCopy(r, &t);
	}
}

static int dn_check_gcd(decNumber *r, const decNumber *x, const decNumber *y,
		int *sign, decNumber *a, decNumber *b, decContext *ctx) {
	*sign = (decNumberIsNegative(x)?1:0) != (decNumberIsNegative(y)?1:0);
	if (decNumberIsSpecial(x) || decNumberIsSpecial(y)) {
		if (decNumberIsNaN(x) || decNumberIsNaN(y))
			set_NaN(r);
		else if (*sign)
			set_neginf(r);
		else
			set_inf(r);
	} else if (!is_int(x, ctx) || !is_int(y, ctx))
		set_NaN(r);
	else {
		decNumberAbs(a, x, ctx);
		decNumberAbs(b, y, ctx);
		return 0;
	}
	return 1;
}

decNumber *decNumberGCD(decNumber *r, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber a, b;
	int sign;

	if (dn_check_gcd(r, x, y, &sign, &a, &b, ctx))
		return r;

	if(decNumberIsZero(x))
		decNumberCopy(r, &b);
	else if (decNumberIsZero(y))
		decNumberCopy(r, &a);
	else
		dn_gcd(r, &a, &b, ctx);
	if (sign)
		decNumberMinus(r, r, ctx);
	return r;
}

decNumber *decNumberLCM(decNumber *r, const decNumber *x, const decNumber *y, decContext *ctx) {
	int sign;
	decNumber gcd, a, b, t;

	if (dn_check_gcd(r, x, y, &sign, &a, &b, ctx))
		return r;

	if(decNumberIsZero(x) || decNumberIsZero(y))
		decNumberCopy(r, x);
	dn_gcd(&gcd, &a, &b, ctx);
	decNumberDivide(&t, &a, &gcd, ctx);
	decNumberMultiply(r, &t, &b, ctx);
	if (sign)
		decNumberMinus(r, r, ctx);
	return r;
}


/* The extra logrithmetic and power functions */

/* ln(1+x) */
decNumber *decNumberLn1p(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber u, v, w;

	if (decNumberIsSpecial(x) || decNumberIsZero(x)) {
		decNumberCopy(r, x);
		return r;
	}
	decNumberAdd(&u, x, &const_1, ctx);
	decNumberSubtract(&v, &u, &const_1, ctx);
	if (decNumberIsZero(&v)) {
		decNumberCopy(r, x);
		return r;
	}
	decNumberDivide(&w, x, &v, ctx);
	decNumberLn(&v, &u, ctx);
	return decNumberMultiply(r, &v, &w, ctx);
}

/* exp(x)-1 */
decNumber *decNumberExpm1(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber u, v, w;

	if (decNumberIsSpecial(x)) {
		decNumberCopy(r, x);
		return r;
	}
	decNumberExp(&u, x, ctx);
	decNumberSubtract(&v, &u, &const_1, ctx);
	if (decNumberIsZero(&v)) {
		decNumberCopy(r, x);
		return r;
	}
	decNumberCompare(&w, &v, &const__1, ctx);
	if (decNumberIsZero(&w)) {
		decNumberCopy(r, &const__1);
		return r;
	}
	decNumberMultiply(&w, &v, x, ctx);
	decNumberLn(&v, &u, ctx);
	return decNumberDivide(r, &w, &v, ctx);
}


decNumber *do_log(decNumber *r, const decNumber *x, const decNumber *base, decContext *ctx) {
	decNumber y;

	if (decNumberIsInfinite(x)) {
		set_inf(r);
		return r;
	}
	decNumberLn(&y, x, ctx);
	return decNumberDivide(r, &y, base, ctx);
}

decNumber *decNumberLog2(decNumber *r, const decNumber *x, decContext *ctx) {
	return do_log(r, x, &const_ln2, ctx);
}

decNumber *decNumberLogxy(decNumber *r, const decNumber *y, const decNumber *x, decContext *ctx) {
	decNumber lx;

	decNumberLn(&lx, x, ctx);
	return do_log(r, y, &lx, ctx);
}

decNumber *decNumberPow2(decNumber *r, const decNumber *x, decContext *ctx) {
	return decNumberPower(r, &const_2, x, ctx);
}

decNumber *decNumberPow10(decNumber *r, const decNumber *x, decContext *ctx) {
	return decNumberPower(r, &const_10, x, ctx);
}

decNumber *decNumberPow_1(decNumber *r, const decNumber *x, decContext *ctx) {
	int even = is_even(x);
	if (even == 1)
		return decNumberCopy(r, &const_1);
	if (even == 0)
		return decNumberCopy(r, &const__1);
	set_NaN(r);
	return r;
}

decNumber *decNumberLamW(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber s, t, u, v, w;
	int i;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x) || decNumberIsNegative(x))
			set_NaN(r);
		else
			set_inf(r);
		return r;
	}

	decNumberRecip(&t, &const_e, ctx);
	decNumberMinus(&s, &t, ctx);	// -1/e
	decNumberCompare(&t, &s, x, ctx);
	if (! decNumberIsNegative(&t)) {
		set_NaN(r);
		return r;
	}

	// Make an initial guess as to the value
	decNumberCompare(&t, &const_500, x, ctx);
	if (! decNumberIsNegative(&t)) {
		// x<500, lx1 = ln(x+1); est = .665 * (1 + .0195*lx1) * lx1 + 0.04
		decNumberAdd(&t, x, &const_1, ctx);
		decNumberLn(&w, &t, ctx);

		decNumberMultiply(&s, &const_0_0195, &w, ctx);
		decNumberAdd(&t, &const_1, &s, ctx);
		decNumberMultiply(&u, &t, &const_0_665, ctx);
		decNumberMultiply(&t, &u, &w, ctx);
		decNumberAdd(r, &const_0_04, &t, ctx);
	} else {
		// x>=500, est = ln(x-4) - (1 - 1/ln(x)) * ln(ln(x))
		decNumberLn(&w, x, ctx);
		decNumberLn(&t, &w, ctx);
		decNumberRecip(r, &w, ctx);
		decNumberSubtract(&s, &const_1, r, ctx);
		decNumberMultiply(r, &s, &t, ctx);

		decNumberSubtract(&s, x, &const_4, ctx);
		decNumberLn(&t, &s, ctx);

		decNumberSubtract(r, &t, r, ctx);
	}

	for (i=0; i<20; i++) {
		// Now iterate to refine the estimate
		decNumberAdd(&u, r, &const_1, ctx);	// u = wj + 1
		decNumberExp(&t, r, ctx);		// t = e^wj
		decNumberMultiply(&s, &u, &t, ctx);	// s = (wj+1)e^wj

		decNumberAdd(&v, &u, &const_1, ctx);	// v = wj + 2
		decNumberAdd(&w, &u, &u, ctx);		// w = 2wj + 2
		decNumberDivide(&u, &v, &w, ctx);	// u = (wj+2)/(2wj+2)
		decNumberMultiply(&w, &t, r, ctx);	// w = wj e^wj

		// Check for termination w, x, u & s are live here
		decNumberSubtract(&v, x, &w, ctx);	// v = x - wj e^wj
		decNumberDivide(&t, &v, &s, ctx);
		decNumberAbs(&t, &t, ctx);
		decNumberCompare(&t, &t, &const_1e_32, ctx);
		if (decNumberIsNegative(&t))
			break;

		// Continue the increment update
		decNumberMinus(&v, &v, ctx);		// v = wj e^wj - x
		decNumberMultiply(&t, &v, &u, ctx);	// t = (wj+2).(wj e^wj - x) / (2wj + 2)
		decNumberSubtract(&w, &s, &t, ctx);	// w = denominator
		decNumberDivide(&t, &v, &w, ctx);
		decNumberSubtract(r, r, &t, ctx);	// wj+1
	}
	return r;
}

decNumber *decNumberInvW(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberExp(&t, x, ctx);
	return decNumberMultiply(r, &t, x, ctx);
}


/* Square - this almost certainly could be done more efficiently
 */
decNumber *decNumberSquare(decNumber *r, const decNumber *x, decContext *ctx) {
	return decNumberMultiply(r, x, x, ctx);
}

/* Cube - again could be done more efficiently */
decNumber *decNumberCube(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber z;

	decNumberSquare(&z, x, ctx);
	return decNumberMultiply(r, &z, x, ctx);
}

/* Cube root */
decNumber *decNumberCubeRoot(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber third, t;

	decNumberRecip(&third, &const_3, ctx);

	if (decNumberIsNegative(x)) {
		decNumberMinus(&t, x, ctx);
		decNumberPower(&t, &t, &third, ctx);
		return decNumberMinus(r, &t, ctx);
	}
	return decNumberPower(r, x, &third, ctx);
}


static void mod2pi(decNumber *res, const decNumber *x, decContext *ctx) {
	/* Declare a structure large enough to hold a really long number.
	 * This structure is likely to be larger than is required.
	 */
	struct {
	    decNumber n;
	    decNumberUnit extra[((MOD_DIGITS-DECNUMDIGITS+DECDPUN-1)/DECDPUN)];
	} out;

	decContext big;
	big = *ctx;
	big.digits = MOD_DIGITS;

	decNumberRemainder(&out.n, x, &const_2PI, &big);
	decNumberPlus(res, &out.n, ctx);
}


/* Calculate sin and cos by Taylor series
 */
static void sincosTaylor(const decNumber *a, decNumber *s, decNumber *c,
                            decContext *ctx) {
	decNumber a2, t, j, z;
	int i, fins = 0, finc = 0;

	decNumberMultiply(&a2, a, a, ctx);
	decNumberCopy(&j, &const_1);
	decNumberCopy(&t, &const_1);
	if (s != NULL)
		decNumberCopy(s, &const_1);
	if (c != NULL)
		decNumberCopy(c, &const_1);

	for (i=1; !fins && !finc && i < 1000; i++) {
		dn_inc(&j, ctx);
		decNumberDivide(&z, &a2, &j, ctx);
		decNumberMultiply(&t, &t, &z, ctx);
		if (!finc && c != NULL) {
			decNumberCopy(&z, c);
			if (i & 1)
				decNumberSubtract(c, c, &t, ctx);
			else
				decNumberAdd(c, c, &t, ctx);
			decNumberCompare(&z, c, &z, ctx);
			if (decNumberIsZero(&z))
				finc = 1;
		}

		dn_inc(&j, ctx);
		decNumberDivide(&t, &t, &j, ctx);
		if (!fins && s != NULL) {
			decNumberCopy(&z, s);
			if (i & 1)
				decNumberSubtract(s, s, &t, ctx);
			else
				decNumberAdd(s, s, &t, ctx);
			decNumberCompare(&z, s, &z, ctx);
			if (decNumberIsZero(&z))
				fins = 1;
		}
	}
	if (s != NULL)
		decNumberMultiply(s, s, a, ctx);
}


/* Check for right angle multiples and if exact, return the apropriate
 * quadrant constant directly.
 */
static int right_angle(decNumber *res, const decNumber *x,
		const decNumber *quad, const decNumber *r0, const decNumber *r1,
		const decNumber *r2, const decNumber *r3, decContext *ctx) {
	decNumber r;
	const decNumber *z;

	decNumberRemainder(&r, x, quad, ctx);
	if (!decNumberIsZero(&r))
		return 0;

	if (decNumberIsZero(x))
		z = r0;
	else {
		decNumberAdd(&r, quad, quad, ctx);
		decNumberCompare(&r, &r, x, ctx);
		if (decNumberIsZero(&r))
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
		const decNumber *r2, const decNumber *r3, decContext *ctx) {
	decNumber fm;

	switch (get_trig_mode()) {
	case TRIG_RAD:
		mod2pi(res, x, ctx);
		break;
	case TRIG_DEG:
		decNumberRemainder(&fm, x, &const_360, ctx);
		if (r0 != NULL && right_angle(res, &fm, &const_90, r0, r1, r2, r3, ctx))
			return 0;
		decNumberD2R(res, &fm, ctx);
		break;
	case TRIG_GRAD:
		decNumberRemainder(&fm, x, &const_400, ctx);
		if (r0 != NULL && right_angle(res, &fm, &const_100, r0, r1, r2, r3, ctx))
			return 0;
		decNumberG2R(res, &fm, ctx);
		break;
	}
	return 1;
}

static void cvt_rad2(decNumber *res, const decNumber *x, decContext *ctx) {
	switch (get_trig_mode()) {
	case TRIG_RAD:	decNumberCopy(res, x);		break;
	case TRIG_DEG:	decNumberR2D(res, x, ctx);	break;
	case TRIG_GRAD:	decNumberR2G(res, x, ctx);	break;
	}
}

/* Calculate sin and cos of the given number in radians.
 * We need to do some range reduction to guarantee that our Taylor series
 * converges rapidly.
 */
void dn_sincos(const decNumber *v, decNumber *sinv, decNumber *cosv,
                    decContext *ctx)
{
	decNumber x;

	if (decNumberIsSpecial(v))
		cmplx_NaN(sinv, cosv);
	else {
		mod2pi(&x, v, ctx);
		sincosTaylor(&x, sinv, cosv, ctx);
	}
}

decNumber *decNumberSin(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber x2;

	if (decNumberIsSpecial(x))
		set_NaN(res);
	else {
		if (cvt_2rad(&x2, x, &const_0, &const_1, &const_0, &const__1, ctx))
			sincosTaylor(&x2, res, NULL, ctx);
		else
			decNumberCopy(res, &x2);
	}
	return res;
}

decNumber *decNumberCos(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber x2;

	if (decNumberIsSpecial(x))
		set_NaN(res);
	else {
		if (cvt_2rad(&x2, x, &const_1, &const_0, &const__1, &const_0, ctx))
			sincosTaylor(&x2, NULL, res, ctx);
		else
			decNumberCopy(res, &x2);
	}
	return res;
}

decNumber *decNumberTan(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber x2, s, c;

	if (decNumberIsSpecial(x))
		set_NaN(res);
	else {
		if (cvt_2rad(&x2, x, &const_0, &const_NaN, &const_0, &const_NaN, ctx)) {
			sincosTaylor(&x2, &s, &c, ctx);
			decNumberDivide(res, &s, &c, ctx);
		} else
			decNumberCopy(res, &x2);
	}
	return res;
}

#if 0
decNumber *decNumberSec(decNumber *res, const decNumber *x, decContext *ctx) {
	return dn_recip(res, x, ctx, &decNumberCos);
}

decNumber *decNumberCosec(decNumber *res, const decNumber *x, decContext *ctx) {
	return dn_recip(res, x, ctx, &decNumberSin);
}

decNumber *decNumberCot(decNumber *res, const decNumber *x, decContext *ctx) {
	return dn_recip(res, x, ctx, &decNumberTan);
}
#endif

decNumber *decNumberSinc(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber s, t;

	decNumberSquare(&s, x, ctx);
	decNumberAdd(&t, &s, &const_1, ctx);
	decNumberCompare(&s, &t, &const_1, ctx);
	if (decNumberIsZero(&s))
		decNumberCopy(res, &const_1);
	else {
		decNumberSin(&s, x, ctx);
		decNumberDivide(res, &s, x, ctx);
	}
	return res;
}

void do_atan(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber a, b, a1, a2, t, j, z, last;
	int doubles = 0;
	int invert;
	int neg = decNumberIsNegative(x);
	int n;

	// arrange for a >= 0
	if (neg)
		decNumberMinus(&a, x, ctx);
	else
		decNumberCopy(&a, x);

	// reduce range to 0 <= a < 1, using atan(x) = pi/2 - atan(1/x)
	decNumberCompare(&b, &a, &const_1, ctx);
	invert = !decNumberIsNegative(&b) && !decNumberIsZero(&b);
	if (invert)
		decNumberDivide(&a, &const_1, &a, ctx);

	// Range reduce to small enough limit to use taylor series
	// using:
	//  tan(x/2) = tan(x)/(1+sqrt(1+tan(x)^2))
	for (n=0; n<1000; n++) {
		decNumberCompare(&b, &a, &const_0_1, ctx);
		if (decNumberIsNegative(&b) || decNumberIsZero(&b))
			break;
		doubles++;
		// a = a/(1+sqrt(1+a^2)) -- at most 3 iterations.
		decNumberMultiply(&b, &a, &a, ctx);
		dn_inc(&b, ctx);
		decNumberSquareRoot(&b, &b, ctx);
		dn_inc(&b, ctx);
		decNumberDivide(&a, &a, &b, ctx);
	}

	// Now Taylor series
	// tan(x) = x(1-x^2/3+x^4/5-x^6/7...)
	// We calculate pairs of terms and stop when the estimate doesn't change
	decNumberCopy(res, &const_3);
	decNumberCopy(&j, &const_5);
	decNumberMultiply(&a2, &a, &a, ctx);        // a^2
	decNumberCopy(&t, &a2);
	decNumberDivide(res, &t, res, ctx);         // s = 1-t/3 -- first two terms
	decNumberSubtract(res, &const_1, res, ctx);

	do {    // Loop until there is no digits changed
		decNumberCopy(&last, res);

		decNumberMultiply(&t, &t, &a2, ctx);
		decNumberDivide(&z, &t, &j, ctx);
		decNumberAdd(res, res, &z, ctx);
		decNumberAdd(&j, &j, &const_2, ctx);

		decNumberMultiply(&t, &t, &a2, ctx);
		decNumberDivide(&z, &t, &j, ctx);
		decNumberSubtract(res, res, &z, ctx);
		decNumberAdd(&j, &j, &const_2, ctx);

		decNumberCompare(&a1, res, &last, ctx);
	} while (!decNumberIsZero(&a1));
	decNumberMultiply(res, res, &a, ctx);

	while (doubles) {
		decNumberAdd(res, res, res, ctx);
		doubles--;
	}

	if (invert) {
		decNumberSubtract(res, &const_PIon2, res, ctx);
	}

	if (neg)
		decNumberMinus(res, res, ctx);
}

void do_asin(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber abx, z;

	if (decNumberIsNaN(x)) {
		set_NaN(res);
		return;
	}

	decNumberAbs(&abx, x, ctx);
	decNumberCompare(&z, &abx, &const_1, ctx);
	if (!decNumberIsNegative(&z) && !decNumberIsZero(&z)) {
		set_NaN(res);
		return;
	}

	// res = 2*atan(x/(1+sqrt(1-x*x)))
	decNumberMultiply(&z, x, x, ctx);
	decNumberSubtract(&z, &const_1, &z, ctx);
	decNumberSquareRoot(&z, &z, ctx);
	dn_inc(&z, ctx);
	decNumberDivide(&z, x, &z, ctx);
	do_atan(&abx, &z, ctx);
	decNumberAdd(res, &abx, &abx, ctx);
}

void do_acos(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber abx, z;

	if (decNumberIsNaN(x)) {
		set_NaN(res);
		return;
	}

	decNumberAbs(&abx, x, ctx);
	decNumberCompare(&z, &abx, &const_1, ctx);

	if (!decNumberIsNegative(&z) && !decNumberIsZero(&z)) {
		set_NaN(res);
		return;
	}

	// res = 2*atan((1-x)/sqrt(1-x*x))
	decNumberCompare(&z, x, &const_1, ctx);
	if (decNumberIsZero(&z))
		decNumberZero(res);

	decNumberMultiply(&z, x, x, ctx);
	decNumberSubtract(&z, &const_1, &z, ctx);
	decNumberSquareRoot(&z, &z, ctx);
	decNumberSubtract(&abx, &const_1, x, ctx);
	decNumberDivide(&z, &abx, &z, ctx);
	do_atan(&abx, &z, ctx);
	decNumberAdd(res, &abx, &abx, ctx);
}

decNumber *decNumberArcSin(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber z;

	do_asin(&z, x, ctx);
	cvt_rad2(res, &z, ctx);
	return res;
}

decNumber *decNumberArcCos(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber z;

	do_acos(&z, x, ctx);
	cvt_rad2(res, &z, ctx);
	return res;
}

decNumber *decNumberArcTan(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber z;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			set_NaN(res);
		else {
			decNumberCopy(res, &const_PIon2);
			if (decNumberIsNegative(x))
				decNumberMinus(res, res, ctx);
		}
	} else
		do_atan(&z, x, ctx);
	cvt_rad2(res, &z, ctx);
	return res;
}

decNumber *do_atan2(decNumber *at, const decNumber *y, const decNumber *x, decContext *ctx) {
	decNumber r, t;
	const int xneg = decNumberIsNegative(x);
	const int yneg = decNumberIsNegative(y);

	if (decNumberIsNaN(x) || decNumberIsNaN(y)) {
		set_NaN(at);
		return at;
	}
	if (decNumberIsZero(y)) {
		if (yneg) {
			if (decNumberIsZero(x)) {
				if (xneg) {
					decNumberPI(at);
					decNumberMinus(at, at, ctx);
				} else
					decNumberCopy(at, y);
			} else if (xneg) {
				decNumberPI(at);
				decNumberMinus(at, at, ctx);
			} else
				decNumberCopy(at, y);
		} else {
			if (decNumberIsZero(x)) {
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
	if (decNumberIsZero(x)) {
		decNumberPIon2(at);
		if (yneg)
			decNumberMinus(at, at, ctx);
		return at;
	}
	if (decNumberIsInfinite(x)) {
		if (xneg) {
			if (decNumberIsInfinite(y)) {
				decNumberPI(&t);
				decNumberMultiply(at, &t, &const_0_75, ctx);
				if (yneg)
					decNumberMinus(at, at, ctx);
			} else {
				decNumberPI(at);
				if (yneg)
					decNumberMinus(at, at, ctx);
			}
		} else {
			if (decNumberIsInfinite(y)) {
				decNumberPIon2(&t);
				decNumberMultiply(at, &t, &const_0_5, ctx);
				if (yneg)
					decNumberMinus(at, at, ctx);
			} else {
				decNumberZero(at);
				if (yneg)
					decNumberMinus(at, at, ctx);
			}
		}
		return at;
	}
	if (decNumberIsInfinite(y)) {
		decNumberPIon2(at);
		if (yneg)
			decNumberMinus(at, at, ctx);
		return at;
	}

	decNumberDivide(&t, y, x, ctx);
	do_atan(&r, &t, ctx);
	if (xneg) {
		decNumberPI(&t);
		if (yneg)
			decNumberMinus(&t, &t, ctx);
	} else
		decNumberZero(&t);
	decNumberAdd(at, &r, &t, ctx);
	if (decNumberIsZero(at) && yneg)
		decNumberMinus(at, at, ctx);
	return at;
}

decNumber *decNumberArcTan2(decNumber *res, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumber z;

	do_atan2(&z, a, b, ctx);
	cvt_rad2(res, &z, ctx);
	return res;	
}

void op_r2p(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	decNumber x, y, rx, ry;

	getXY(&x, &y);
	cmplxToPolar(&rx, &ry, &x, &y, Ctx);
	cvt_rad2(&y, &ry, Ctx);
	setlastX();
	setXY(&rx, &y);
}

void op_p2r(decimal64 *nul1, decimal64 *nul2, decContext *nulc) {
	decNumber x, y, rx, ry;

	getXY(&x, &ry);
	cvt_2rad(&y, &ry, NULL, NULL, NULL, NULL, Ctx);
	cmplxFromPolar(&rx, &ry, &x, &y, Ctx);
	setlastX();
	setXY(&rx, &ry);
}	


/* Hyperbolic functions.
 * We start with a utility routine that calculates sinh and cosh.
 * We do the sihn as (e^x - 1) / e^x + e^x - 1 for numerical stability
 * reasons.
 */
void dn_sinhcosh(const decNumber *x, decNumber *sinhv, decNumber *coshv, decContext *ctx) {
	decNumber t, u, v, s, r;

	decNumberExp(&u, x, ctx);			// u = e^x
	decNumberRecip(&v, &u, ctx);			// v = e^-x

	if (sinhv != NULL) {
		decNumberExpm1(&t, x, ctx);		// t = e^x - 1
		decNumberMultiply(&s, &t, &v, ctx);	// s = (e^x - 1) / e^x = 1 - e^-x
		decNumberAdd(&r, &s, &t, ctx);		// r = e^x - 1 + 1 - e^-x = e^x - e^-x	
		decNumberMultiply(sinhv, &r, &const_0_5, ctx);
	}

	if (coshv != NULL) {
		decNumberAdd(&r, &v, &u, ctx);		// r = e^x + e^-x
		decNumberMultiply(coshv, &r, &const_0_5, ctx);
	}
}

decNumber *decNumberSinh(decNumber *res, const decNumber *x, decContext *ctx) {
	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			set_NaN(res);
		else
			decNumberCopy(res, x);
	} else
		dn_sinhcosh(x, res, NULL, ctx);
	return res;
}

decNumber *decNumberCosh(decNumber *res, const decNumber *x, decContext *ctx) {
	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			set_NaN(res);
		else
			set_inf(res);
	} else
		dn_sinhcosh(x, NULL, res, ctx);
	return res;
}

decNumber *decNumberTanh(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber s, c;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			set_NaN(res);
		else if (decNumberIsNegative(x))
			decNumberCopy(res, &const__1);
		else
			decNumberCopy(res, &const_1);
		return res;
	}
	dn_sinhcosh(x, &s, &c, ctx);
	return decNumberDivide(res, &s, &c, ctx);
}


#if 0
decNumber *decNumberSech(decNumber *res, const decNumber *x, decContext *ctx) {
	return dn_recip(res, x, ctx, &decNumberCosh);
}

decNumber *decNumberCosech(decNumber *res, const decNumber *x, decContext *ctx) {
	return dn_recip(res, x, ctx, &decNumberSinh);
}

decNumber *decNumberCoth(decNumber *res, const decNumber *x, decContext *ctx) {
	return dn_recip(res, x, ctx, &decNumberTanh);
}
#endif

decNumber *decNumberArcSinh(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber y, z;

	decNumberMultiply(&y, x, x, ctx);	// y = x^2
	decNumberAdd(&z, &y, &const_1, ctx);	// z = x^2 + 1
	decNumberSquareRoot(&y, &z, ctx);	// y = sqrt(x^2+1)
	decNumberAdd(&z, &y, x, ctx);		// z = x + sqrt(x^2+1)
	return decNumberLn(res, &z, ctx);
}


decNumber *decNumberArcCosh(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber z;

	decNumberMultiply(res, x, x, ctx);	// r = x^2
	decNumberSubtract(&z, res, &const_1, ctx);	// z = x^2 + 1
	decNumberSquareRoot(res, &z, ctx);	// r = sqrt(x^2+1)
	decNumberAdd(&z, res, x, ctx);		// z = x + sqrt(x^2+1)
	return decNumberLn(res, &z, ctx);
}

decNumber *decNumberArcTanh(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber y, z;

	// Not the obvious formula but more stable...
	decNumberSubtract(&z, &const_1, x, ctx);
	decNumberDivide(&y, x, &z, ctx);
	decNumberAdd(&z, &y, &y, ctx);
	decNumberLn1p(&y, &z, ctx);
	return decNumberMultiply(res, &const_0_5, &y, ctx);
}


decNumber *decNumberD2R(decNumber *res, const decNumber *x, decContext *ctx) {
	return decNumberMultiply(res, x, &const_PIon180, ctx);
}

decNumber *decNumberR2D(decNumber *res, const decNumber *x, decContext *ctx) {
	return decNumberMultiply(res, x, &const_PIunder180, ctx);
}


decNumber *decNumberG2R(decNumber *res, const decNumber *x, decContext *ctx) {
	return decNumberMultiply(res, x, &const_PIon200, ctx);
}

decNumber *decNumberR2G(decNumber *res, const decNumber *x, decContext *ctx) {
	return decNumberMultiply(res, x, &const_PIunder200, ctx);
}


decNumber *decNumber2Deg(decNumber *res, const decNumber *x, decContext *ctx) {
	switch (get_trig_mode()) {
	case TRIG_DEG:	decNumberCopy(res, x);		break;
	case TRIG_RAD:	decNumberR2D(res, x, ctx);	break;
	case TRIG_GRAD:
		decNumberMultiply(res, x, &const_0_9, ctx);
		break;
	}
        set_trig_mode(TRIG_DEG);
	return res;
}

decNumber *decNumber2Rad(decNumber *res, const decNumber *x, decContext *ctx) {
	switch (get_trig_mode()) {
	case TRIG_DEG:	decNumberD2R(res, x, ctx);	break;
	case TRIG_RAD:	decNumberCopy(res, x);		break;
	case TRIG_GRAD:	decNumberG2R(res, x, ctx);	break;
	}
        set_trig_mode(TRIG_RAD);
	return res;
}

decNumber *decNumber2Grad(decNumber *res, const decNumber *x, decContext *ctx) {
	switch (get_trig_mode()) {
	case TRIG_DEG:
		decNumberDivide(res, x, &const_0_9, ctx);
		break;
	case TRIG_RAD:	decNumberR2G(res, x, ctx);	break;
	case TRIG_GRAD:	decNumberCopy(res, x);		break;
	}
        set_trig_mode(TRIG_GRAD);
	return res;
}

/* Check the arguments a little and perform the computation of
 * ln(permutation) which is common across both our callers.
 *
 * This is the real version.
 */
enum perm_opts { PERM_INVALID=0, PERM_INTG, PERM_NORMAL };
static enum perm_opts perm_helper(decNumber *r, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber n, s;

	if (decNumberIsInfinite(x)) {
		if (decNumberIsInfinite(y))
			set_NaN(r);
		else
			set_inf(r);
		return PERM_INVALID;
	}
	decNumberAdd(&n, x, &const_1, ctx);	// x+1
	decNumberLnGamma(&s, &n, ctx);		// lnGamma(x+1) = Ln x!

	decNumberSubtract(r, &n, y, ctx);	// x-y+1
	decNumberLnGamma(&n, r, ctx);		// LnGamma(x-y+1) = Ln (x-y)!
	decNumberSubtract(r, &s, &n, ctx);

	if (is_int(x, ctx) && is_int(y, ctx))
		return PERM_INTG;
	return PERM_NORMAL;
}


/* Calculate permutations:
 * C(x, y) = P(x, y) / y! = x! / ( (x-y)! y! )
 */
decNumber *decNumberComb(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber r, n, s;
	const enum perm_opts code = perm_helper(&r, x, y, ctx);

	if (code != PERM_INVALID) {
		decNumberAdd(&n, y, &const_1, ctx);	// y+1
		decNumberLnGamma(&s, &n, ctx);		// LnGamma(y+1) = Ln y!
		decNumberSubtract(&n, &r, &s, ctx);

		decNumberExp(res, &n, ctx);
		if (code == PERM_INTG)
			decNumberIntg(res, res, ctx);
	} else
		decNumberCopy(res, &r);
	return res;
}

/* Calculate permutations:
 * P(x, y) = x! / (x-y)!
 */
decNumber *decNumberPerm(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber t;
	const enum perm_opts code = perm_helper(&t, x, y, ctx);

	if (code != PERM_INVALID) {
		decNumberExp(res, &t, ctx);
		if (code == PERM_INTG)
			decNumberIntg(res, res, ctx);
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

static void dn_LnGamma(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber s, t, u, v;
	int k;

	decNumberZero(&s);
	decNumberAdd(&t, x, &const_21, ctx);
	for (k=20; k>=0; k--) {
		decNumberDivide(&u, gamma_consts[k], &t, ctx);
		dn_dec(&t, ctx);
		decNumberAdd(&s, &s, &u, ctx);
	}
	decNumberAdd(&t, &s, &const_gammaC00, ctx);
	decNumberMultiply(&u, &t, &const_2rootEonPI, ctx);
	decNumberLn(&s, &u, ctx);

	decNumberAdd(&t, x, &const_gammaR, ctx);
	decNumberDivide(&u, &t, &const_e, ctx);
	decNumberLn(&t, &u, ctx);
	decNumberAdd(&u, x, &const_0_5, ctx);
	decNumberMultiply(&v, &u, &t, ctx);
	decNumberAdd(res, &v, &s, ctx);
}

decNumber *decNumberFactorial(decNumber *res, const decNumber *xin, decContext *ctx) {
	decNumber x;

	decNumberAdd(&x, xin, &const_1, ctx);
	return decNumberGamma(res, &x, ctx);
}

#ifdef INCLUDE_DBLFACT
decNumber *decNumberDblFactorial(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t, u, v;

	decNumberAdd(&t, x, &const_2, ctx);		// t = x+2
	decNumberPow2(&u, &t, ctx);			// u = 2^(x+1)
	decNumberSquareRoot(&t, &u, ctx);
	decNumberMultiply(&u, &t, &const_recipsqrt2PI, ctx);
	decNumberMultiply(&t, x, &const_0_5, ctx);
	decNumberAdd(&v, &t, &const_1, ctx);
	decNumberGamma(&t, &v, ctx);
	return decNumberMultiply(r, &u, &t, ctx);
}
#endif

#ifdef INCLUDE_SUBFACT
decNumber *decNumberSubFactorial(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t, u;

	if (is_int(x, ctx)) {
		decNumberFactorial(&t, x, ctx);
		decNumberDivide(&u, &t, &const_e, ctx);
		decNumberAdd(&t, &u, &const_0_5, ctx);
		return decNumberFloor(r, &t, ctx);
	}
	set_NaN(r);
	return r;
}
#endif

decNumber *decNumberGamma(decNumber *res, const decNumber *xin, decContext *ctx) {
	decNumber x, s, t, u;
	int reflec = 0;

	// Check for special cases
	if (decNumberIsSpecial(xin)) {
		if (decNumberIsInfinite(xin) && !decNumberIsNegative(xin))
			set_inf(res);
		else
			set_NaN(res);
		return res;
	}

	// Correct our argument and begin the inversion if it is negative
	if (decNumberIsNegative(xin)) {
		reflec = 1;
		decNumberSubtract(&t, &const_1, xin, ctx);
		if (is_int(&t, ctx)) {
			set_NaN(res);
			return res;
		}
		decNumberSubtract(&x, &t, &const_1, ctx);
	} else
		decNumberSubtract(&x, xin, &const_1, ctx);

	dn_LnGamma(&t, &x, ctx);
	decNumberExp(res, &t, ctx);

	// Finally invert if we started with a negative argument
	if (reflec) {
		// figure out xin * PI mod 2PI
		decNumberRemainder(&s, xin, &const_2, ctx);
		decNumberMultiply(&t, &const_PI, &s, ctx);
		sincosTaylor(&t, &s, &u, ctx);
		decNumberMultiply(&u, &s, res, ctx);
		decNumberDivide(res, &const_PI, &u, ctx);
	}
	return res;
}

// The log gamma function.
decNumber *decNumberLnGamma(decNumber *res, const decNumber *xin, decContext *ctx) {
	decNumber x, s, t, u;
	int reflec = 0;

	// Check for special cases
	if (decNumberIsSpecial(xin)) {
		if (decNumberIsInfinite(xin) && !decNumberIsNegative(xin))
			set_inf(res);
		else
			set_NaN(res);
		return res;
	}

	// Correct out argument and begin the inversion if it is negative
	if (decNumberIsNegative(xin)) {
		reflec = 1;
		decNumberSubtract(&t, &const_1, xin, ctx);
		if (is_int(&t, ctx)) {
			set_NaN(res);
			return res;
		}
		decNumberSubtract(&x, &t, &const_1, ctx);
	} else
		decNumberSubtract(&x, xin, &const_1, ctx);

	dn_LnGamma(res, &x, ctx);

	// Finally invert if we started with a negative argument
	if (reflec) {
		// Figure out S * PI mod 2PI
		decNumberRemainder(&u, &s, &const_2, ctx);
		decNumberMultiply(&t, &const_PI, &u, ctx);
		sincosTaylor(&t, &s, &u, ctx);
		decNumberDivide(&u, &const_PI, &s, ctx);
		decNumberLn(&t, &u, ctx);
		decNumberSubtract(res, &t, res, ctx);
	}
	return res;
}

// lnBeta(x, y) = lngamma(x) + lngamma(y) - lngamma(x+y)
decNumber *decNumberLnBeta(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber s, t, u;

	decNumberLnGamma(&s, x, ctx);
	decNumberLnGamma(&t, y, ctx);
	decNumberAdd(&u, &s, &t, ctx);
	decNumberAdd(&s, x, y, ctx);
	decNumberLnGamma(&t, &s, ctx);
	decNumberSubtract(res, &u, &t, ctx);
	return res;
}

// Beta(x, y) = exp(lngamma(x) + lngamma(y) - lngamma(x+y))
decNumber *decNumberBeta(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber s;

	decNumberLnBeta(&s, x, y, ctx);
	decNumberExp(res, &s, ctx);
	return res;
}

#ifdef INCLUDE_DIGAMMA
const decNumber *const digamma_consts[10] = {
	&const_digammaC02,	&const_digammaC04,	&const_digammaC06,
	&const_digammaC08,	&const_digammaC10,	&const_digammaC12,
	&const_digammaC02,	&const_digammaC16,	&const_digammaC18,
	&const_digammaC20
};

decNumber *decNumberPsi(decNumber *res, const decNumber *xin, decContext *ctx) {
	decNumber x_2, t, r, x;
	int i;

	if (decNumberIsSpecial(xin)) {
		if (decNumberIsNaN(xin) || decNumberIsNegative(xin)) {
			set_NaN(res);
			return res;
		} else
			set_inf(res);
		return res;
	}

	// Check for reflection
	if (decNumberIsNegative(xin) || decNumberIsZero(xin)) {
		if (is_int(xin, ctx)) {
			set_NaN(res);
			return res;
		}
		decNumberMultiply(&x_2, &const_PI, xin, ctx);
		dn_sincos(&x_2, &t, &r, ctx);
		decNumberDivide(&x_2, &r, &t, ctx);		// x_2 = cot(PI.x)
		decNumberMultiply(&t, &x_2, &const_PI, ctx);
		decNumberSubtract(&x, &const_1, xin, ctx);
		decNumberMinus(res, &t, ctx);
	} else {
		decNumberZero(res);
		decNumberCopy(&x, xin);
	}

	// Use recurrance relation to bring x large enough for our series to converge
	for (;;) {
		decNumberCompare(&t, &const_8, &x, ctx);
		if (decNumberIsNegative(&t))
			break;
		decNumberRecip(&t, &x, ctx);
		decNumberSubtract(res, res, &t, ctx);
		dn_inc(&x, ctx);
	}

	// Finally the series approximation
	decNumberLn(&t, &x, ctx);
	decNumberAdd(res, res, &t, ctx);
	decNumberMultiply(&r, &x, &const__2, ctx);
	decNumberRecip(&t, &r, ctx);
	decNumberAdd(res, res, &t, ctx);

	decNumberSquare(&t, &x, ctx);
	decNumberRecip(&x_2, &t, ctx);
	decNumberCopy(&r, &x_2);
	for (i=0; i<10; i++) {
		decNumberMultiply(&t, &r, digamma_consts[i], ctx);
		decNumberAdd(res, res, &t, ctx);
		decNumberMultiply(&r, &r, &x_2, ctx);
	}
	return res;
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
		const decNumber *dc, decNumber *k, decContext *ctx) {
	decNumber t, s;

	dn_inc(k, ctx);
	decNumberPower(&s, k, x, ctx);
	decNumberDivide(&t, dc, &s, ctx);
	decNumberAdd(sum, sum, &t, ctx);
}

decNumber *decNumberZeta(decNumber *res, const decNumber *xin, decContext *ctx) {
	decNumber s, x, u, reflecfac, sum, t;
	int reflec, i;

	if (decNumberIsSpecial(xin)) {
		if (decNumberIsNaN(xin) || decNumberIsNegative(xin))
			set_NaN(res);
		else
			decNumberCopy(res, &const_1);
		return res;
	}
	if (decNumberIsZero(xin)) {
		decNumberCopy(res, &const__0_5);
		return res;
	}
	if (decNumberIsNegative(xin)) {
		decNumberMultiply(&s, xin, &const_0_5, ctx);
		if (is_int(&s, ctx)) {
			decNumberZero(res);
			return res;
		}
	}

	decNumberCompare(&s, xin, &const_0_5, ctx);
	if (decNumberIsNegative(&s)) {
		/* use reflection formula
		 * zeta(x) = 2^x*Pi^(x-1)*sin(Pi*x/2)*gamma(1-x)*zeta(1-x)
		 */
		reflec = 1;
		decNumberSubtract(&x, &const_1, xin, ctx);
		// Figure out xin * PI / 2 mod 2PI
		decNumberRemainder(&s, xin, &const_4, ctx);
		decNumberMultiply(&u, &const_PIon2, &s, ctx);
		sincosTaylor(&u, &s, res, ctx);
		decNumberPower(res, &const_2, xin, ctx);
		decNumberMultiply(&u, res, &s, ctx);
		decNumberPower(res, &const_PI, &x, ctx);
		decNumberDivide(&s, &u, res, ctx);
		decNumberGamma(res, &x, ctx);
		decNumberMultiply(&reflecfac, &s, res, ctx);
	} else {
		reflec = 0;
		decNumberCopy(&x, xin);
	}

	/* Now calculate zeta(x) where x >= 0.5 */
	decNumberZero(&sum);
	decNumberZero(&t);
	for (i=0; i<30; i++)
		zeta_step(&sum, &x, zeta_consts[i], &t, ctx);

	decNumberSubtract(&t, &const_1, &x, ctx);
	decNumberPower(&u, &const_2, &t, ctx);
	decNumberSubtract(&t, &u, &const_1, ctx);
	decNumberMultiply(&u, &t, &const_zeta_dn, ctx);
	decNumberDivide(res, &sum, &u, ctx);

	/* Finally, undo the reflection if required */
	if (reflec)
		decNumberMultiply(res, &reflecfac, res, ctx);
	return res;
}
#endif /* INCLUDE_ZETA */


// % = x . y / 100
decNumber *decNumberPercent(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber y, z;

	getY(&y);
	decNumberDivide(&z, &y, &const_100, ctx);
	return decNumberMultiply(res, &z, x, ctx);
}

// %chg = 100 ( x - y ) / y
decNumber *decNumberPerchg(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber w, y, z;

	getY(&y);
	decNumberSubtract(&z, x, &y, ctx);
	decNumberDivide(&w, &z, &y, ctx);
	decNumberMultiply(res, &w, &const_100, ctx);
	return res;
}

// %tot = 100 . x / y
decNumber *decNumberPertot(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber y, z;

	getY(&y);
	decNumberDivide(&z, x, &y, ctx);
	decNumberMultiply(res, &z, &const_100, ctx);
	return res;
}

// %+ = x + x * y / 100
decNumber *decNumberPerAdd(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber a, b;

	decNumberMultiply(&a, y, &const_0_01, ctx);
	decNumberMultiply(&b, &a, x, ctx);
	decNumberAdd(res, x, &b, ctx);
	return res;
}

// %- = x - x * y / 100
decNumber *decNumberPerSub(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber a, b;

	decNumberMultiply(&a, y, &const_0_01, ctx);
	decNumberMultiply(&b, &a, x, ctx);
	decNumberSubtract(res, x, &b, ctx);
	return res;
}

// Markup Margin = y / ( 1 - x / 100 )
decNumber *decNumberPerMargin(decNumber *res, const decNumber *y, const decNumber *x, decContext *ctx) {
	decNumber a, b;

	decNumberMultiply(&a, x, &const_0_01, ctx);
	decNumberSubtract(&b, &const_1, &a, ctx);
	return decNumberDivide(res, y, &b, ctx);
}

// Margin = 100 (x - y) / x
decNumber *decNumberMargin(decNumber *res, const decNumber *y, const decNumber *x, decContext *ctx) {
	decNumber a, b;

	decNumberSubtract(&a, x, y, ctx);
	decNumberMultiply(&b, &a, &const_100, ctx);
	return decNumberDivide(res, &b, x, ctx);
}

decNumber *decNemberPerMRR(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x, decContext *ctx) {
	decNumber a, b, c;

	decNumberDivide(&a, x, y, ctx);
	decNumberRecip(&b, z, ctx);
	decNumberPower(&c, &a, &b, ctx);
	decNumberSubtract(&a, &c, &const_1, ctx);
	return decNumberMultiply(r, &a, &const_100, ctx);
}


decNumber *decNumberHMS2HR(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber m, s, t;

	// decode hhhh.mmss...
	decNumberFrac(&t, x, ctx);			// t = .mmss
	decNumberMultiply(&s, &t, &const_100, ctx);	// s = mm.ss
	decNumberTrunc(&m, &s, ctx);			// m = mm
	decNumberFrac(&t, &s, ctx);			// t = .ss
	decNumberMultiply(&s, &t, &const_100on60, ctx);	// s = ss.sss / 60
	decNumberAdd(&t, &m, &s, ctx);			// s = mm + ss.sss / 60
	decNumberMultiply(&m, &t, &const_1on60, ctx);
	decNumberTrunc(&s, x, ctx);			// s = hh
	decNumberAdd(res, &m, &s, ctx);
	return res;
}

decNumber *decNumberHR2HMS(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber m, s, t;

	decNumberFrac(&t, x, ctx);			// t = .mmssss
	decNumberMultiply(&s, &t, &const_60, ctx);	// s = mm.ssss
	decNumberTrunc(&m, &s, ctx);			// m = mm
	decNumberFrac(&t, &s, ctx);			// t = .ssss
	decNumberMultiply(&s, &t, &const_0_6, ctx);	// scale down by 60/100
	decNumberAdd(&t, &s, &m, ctx);			// t = mm.ss
	decNumberMultiply(&m, &t, &const_0_01, ctx);	// t = .mmss
	decNumberTrunc(&s, x, ctx);			// s = hh
	decNumberAdd(res, &m, &s, ctx);			// res = hh.mmss
	return res;
}

decNumber *decNumberHMSAdd(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber a, b, c;

	decNumberHMS2HR(&a, x, ctx);
	decNumberHMS2HR(&b, y, ctx);
	decNumberAdd(&c, &a, &b, ctx);
	decNumberHR2HMS(res, &c, ctx);
	return res;
}

decNumber *decNumberHMSSub(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber a, b, c;

	decNumberHMS2HR(&a, x, ctx);
	decNumberHMS2HR(&b, y, ctx);
	decNumberSubtract(&c, &a, &b, ctx);
	decNumberHR2HMS(res, &c, ctx);
	return res;
}

decNumber *decNumberParallel(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber p, s;

	decNumberMultiply(&p, x, y, ctx);
	decNumberAdd(&s, x, y, ctx);
	decNumberDivide(res, &p, &s, ctx);
	return res;
}

#ifdef INCLUDE_AGM
decNumber *decNumberAGM(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	int n;
	decNumber a, g, t, u;

	if (decNumberIsNegative(x) || decNumberIsNegative(y))
		goto nan;
	if (decNumberIsSpecial(x) || decNumberIsSpecial(y)) {
		if (decNumberIsNaN(x) || decNumberIsNaN(y))
			goto nan;
		if (decNumberIsZero(x) || decNumberIsZero(y))
			goto nan;
		set_inf(res);
		return res;
	}
	decNumberCopy(&a, x);
	decNumberCopy(&g, y);
	for (n=0; n<1000; n++) {
		decNumberSubtract(&t, &a, &g, ctx);
		decNumberAbs(&u, &t, ctx);
		decNumberCompare(&t, &u, &const_1e_32, ctx);
		if (decNumberIsNegative(&t))
			return decNumberCopy(res, &a);

		decNumberAdd(&t, &a, &g, ctx);
		decNumberMultiply(&u, &t, &const_0_5, ctx);

		decNumberMultiply(&t, &a, &g, ctx);
		decNumberSquareRoot(&g, &t, ctx);
		decNumberCopy(&a, &u);
	}
nan:	set_NaN(res);
	return res;
}
#endif


/* Logical operations on decNumbers.
 * We treat 0 as false and non-zero as true.
 */
static int dn2bool(const decNumber *x) {
	return decNumberIsZero(x)?0:1;
}

static decNumber *bool2dn(decNumber *res, int l) {
	if (l)
		decNumberCopy(res, &const_1);
	else
		decNumberZero(res);
	return res;
}

decNumber *decNumberNot(decNumber *res, const decNumber *x, decContext *ctx) {
	return bool2dn(res, !dn2bool(x));
}

decNumber *decNumberAnd(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	return bool2dn(res, dn2bool(x) && dn2bool(y));
}

decNumber *decNumberOr(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	return bool2dn(res, dn2bool(x) || dn2bool(y));
}

decNumber *decNumberXor(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	return bool2dn(res, dn2bool(x) ^ dn2bool(y));
}

decNumber *decNumberNand(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	return bool2dn(res, !(dn2bool(x) && dn2bool(y)));
}

decNumber *decNumberNor(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	return bool2dn(res, !(dn2bool(x) || dn2bool(y)));
}

decNumber *decNumberNxor(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	return bool2dn(res, !(dn2bool(x) ^ dn2bool(y)));
}


decNumber *decNumberRnd(decNumber *res, const decNumber *x, decContext *ctx) {
	int numdig = State.dispdigs + 1;
	decContext c;
	decNumber p10;
	decNumber t, u;
	enum display_modes dmode = State.dispmode;

	if (decNumberIsSpecial(x))
		return decNumberCopy(res, x);

	if (State.fract) {
		decNumber2Fraction(&t, &u, x, ctx);
		return decNumberDivide(res, &t, &u, ctx);
	}

	if (dmode == MODE_STD)
		return decNumberCopy(res, x);

	if (dmode == MODE_FIX) {
		/* FIX is different since the number of digits changes */
		int_to_dn(&u, numdig-1, ctx);
		decNumberPow10(&p10, &u, ctx);
		decNumberMultiply(&t, x, &p10, ctx);
		decNumberRound(&u, &t, ctx);
		return decNumberDivide(res, &u, &p10, ctx);
	}

//	if (dmode == MODE_STD)
//		numdig = 12;

	c = *ctx;
	c.round = DEC_ROUND_HALF_UP;
	c.digits = numdig;

	return decNumberPlus(res, x, &c);
}

void decNumber2Fraction(decNumber *n, decNumber *d, const decNumber *x, decContext *ctx) {
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
			decNumberCopy(n, &const__1);
		else
			decNumberCopy(n, &const_1);
		return;
	}

	dm = State.denom_mode;
	get_maxdenom(&maxd);

	decNumberZero(&dold);
	decNumberCopy(d, &const_1);
	neg = decNumberIsNegative(x);
	if (neg)
		decNumberMinus(&z, x, ctx);
	else
		decNumberCopy(&z, x);
	switch (dm) {
	case DENOM_ANY:
		/* Do a partial fraction expansion until the denominator is too large */
		for (i=0; i<1000; i++) {
			decNumberTrunc(&t, &z, ctx);
			decNumberSubtract(&s, &z, &t, ctx);
			if (decNumberIsZero(&s))
				break;
			decNumberRecip(&z, &s, ctx);
			decNumberTrunc(&s, &z, ctx);
			decNumberMultiply(&t, &s, d, ctx);
			decNumberAdd(&s, &t, &dold, ctx);	// s is new denominator estimate
			decNumberCompare(&t, &maxd, &s, ctx);
			if (decNumberIsNegative(&t) || decNumberIsZero(&t))
				break;
			decNumberCopy(&dold, d);
			decNumberCopy(d, &s);
		}
		break;
	default:
		decNumberCopy(d, &maxd);
		break;
	}
	decNumberMultiply(&t, x, d, ctx);
	decNumberRound(n, &t, ctx);
	if (dm == DENOM_FACTOR) {
		decNumberGCD(&t, n, d, ctx);
		decNumberDivide(n, n, &t, ctx);
		decNumberDivide(d, d, &t, ctx);
	}
	if (neg)
		decNumberMinus(n, n, ctx);
}

decNumber *decNumberFib(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber r, s, t;

	decNumberPower(&r, &const_phi, x, ctx);
	decNumberMultiply(&t, &const_PI, x, ctx);
	dn_sincos(&t, NULL, &s, ctx);
	decNumberDivide(&t, &s, &r, ctx);
	decNumberSubtract(&s, &r, &t, ctx);
	return decNumberMultiply(res, &s, &const_recipsqrt5, ctx);
}


static decNumber *gser(decNumber *res, const decNumber *a, const decNumber *x, const decNumber *gln, decContext *ctx) {
	decNumber ap, del, sum, t, u;
	int i;

	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(res);
	decNumberCopy(&ap, a);
	decNumberRecip(&sum, a, ctx);
	decNumberCopy(&del, &sum);
	for (i=0; i<500; i++) {
		dn_inc(&ap, ctx);
		decNumberDivide(&t, x, &ap, ctx);
		decNumberMultiply(&del, &del, &t, ctx);
		decNumberAdd(&t, &sum, &del, ctx);
		decNumberCompare(&u, &t, &sum, ctx);
		if (decNumberIsZero(&u)) {
			decNumberLn(&t, x, ctx);
			decNumberMultiply(&u, &t, a, ctx);
			decNumberSubtract(&t, &u, x, ctx);
			decNumberSubtract(&u, &t, gln, ctx);
			decNumberExp(&t, &u, ctx);
			return decNumberMultiply(res, &sum, &t, ctx);
		}
		decNumberCopy(&sum, &t);
	}
	return decNumberZero(res);
}

static decNumber *gcf(decNumber *res, const decNumber *a, const decNumber *x, const decNumber *gln, decContext *ctx) {
	decNumber an, b, c, d, h, t, u, v, i;
	int n;

	decNumberAdd(&t, x, &const_1, ctx);
	decNumberSubtract(&b, &t, a, ctx);			// b = (x-1) a
	decNumberCopy(&c, &const_1e32);
	decNumberRecip(&d, &b, ctx);
	decNumberCopy(&h, &d);
	decNumberZero(&i);
	for (n=0; n<500; n++) {
		dn_inc(&i, ctx);
		decNumberSubtract(&t, a, &i, ctx);		// t = a-i
		decNumberMultiply(&an, &i, &t, ctx);		// an = -i (i-a)
		decNumberAdd(&b, &b, &const_2, ctx);
		decNumberMultiply(&t, &an, &d, ctx);
		decNumberAdd(&v, &t, &b, ctx);
		decNumberAbs(&t, &v, ctx);
			decNumberCompare(&u, &t, &const_1e_32, ctx);
			if (decNumberIsNegative(&u))
				decNumberCopy(&d, &const_1e32);
			else
				decNumberRecip(&d, &v, ctx);
			decNumberDivide(&t, &an, &c, ctx);
			decNumberAdd(&c, &b, &t, ctx);
			decNumberAbs(&t, &c, ctx);
			decNumberCompare(&u, &t, &const_1e_32, ctx);
			if (decNumberIsNegative(&u))
				decNumberCopy(&c, &const_1e_32);
		decNumberMultiply(&t, &d, &c, ctx);
		decNumberMultiply(&u, &h, &t, ctx);
		decNumberCompare(&t, &h, &u, ctx);
		if (decNumberIsZero(&t))
			break;
		decNumberCopy(&h, &u);
//		decNumberSubtract(&u, &t, &const_1, ctx);
//		decNumberAbs(&t, &u, ctx);
//		decNumberCompare(&u, &t, &const_1e_32, ctx);
//		if (decNumberIsNegative(&u))
//			break;
	}
	decNumberLn(&t, x, ctx);
	decNumberMultiply(&u, &t, a, ctx);
	decNumberSubtract(&t, &u, x, ctx);
	decNumberSubtract(&u, &t, gln, ctx);
	decNumberExp(&t, &u, ctx);
	return decNumberMultiply(res, &t, &h, ctx);
}

decNumber *decNumberGammap(decNumber *res, const decNumber *a, const decNumber *x, decContext *ctx) {
	decNumber z, lga;

	if (decNumberIsNegative(x) || decNumberIsNegative(a) || decNumberIsZero(a) ||
			decNumberIsNaN(x) || decNumberIsNaN(a) || decNumberIsInfinite(a)) {
		set_NaN(res);
		return res;
	}
	if (decNumberIsInfinite(x))
		return decNumberCopy(res, &const_1);
	if (decNumberIsZero(x))
		return decNumberZero(res);

	decNumberAdd(&lga, a, &const_1, ctx);
	decNumberCompare(&z, x, &lga, ctx);
	decNumberLnGamma(&lga, a, ctx);
	if (decNumberIsNegative(&z))
		return gser(res, a, x, &lga, ctx);
	else {
		gcf(&z, a, x, &lga, ctx);
		return decNumberSubtract(res, &const_1, &z, ctx);
	}
}

decNumber *decNumberERF(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber z;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			set_NaN(res);
		else if (decNumberIsNegative(x))
			return decNumberCopy(res, &const__1);
		else
			return decNumberCopy(res, &const_1);
		return res;
	}
	decNumberSquare(&z, x, ctx);
	decNumberGammap(res, &const_0_5, &z, ctx);
	if (decNumberIsNegative(x))
		return decNumberMinus(res, res, ctx);
	return res;
	
}


/* Jacobi Elliptical functions */
#ifdef INCLUDE_ELLIPTIC
#define ELLIPTIC_N	16

void dn_elliptic(decNumber *sn, decNumber *cn, decNumber *dn, const decNumber *u, const decNumber *m, decContext *ctx) {
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

	decNumberAbs(&a, m, ctx);
	decNumberCompare(&b, &const_1, &a, ctx);
	if (decNumberIsNegative(&b)) {
		cmplx_NaN(sn, cn);
		set_NaN(dn);
		return;
	}
	decNumberCompare(&b, &a, &const_1e_32, ctx);
	if (decNumberIsNegative(&b)) {
		dn_sincos(u, sn, cn, ctx);
		decNumberCopy(dn, &const_1);
		return;
	}
	decNumberSubtract(&a, m, &const_1, ctx);
	decNumberAbs(&b, &a, ctx);
	decNumberCompare(&a, &b, &const_1e_32, ctx);
	if (decNumberIsNegative(&a)) {
		dn_sinhcosh(u, &a, &b, ctx);
		decNumberRecip(cn, &b, ctx);
		decNumberMultiply(sn, &a, cn, ctx);
		decNumberCopy(dn, cn);
		return;
	}
	decNumberCopy(mu(0), &const_1);
	decNumberSubtract(&a, &const_1, m, ctx);
	decNumberSquareRoot(nu(0), &a, ctx);
	for (;;) {
		decNumberAdd(&g, mu(n), nu(n), ctx);
		decNumberAbs(&a, &g, ctx);
		decNumberMultiply(&b, &a, &const_1e_32, ctx);
		decNumberMultiply(&a, &b, &const_2, ctx);
		decNumberSubtract(&e, mu(n), nu(n), ctx);
		decNumberAbs(&f, &e, ctx);
		decNumberCompare(&e, &a, &f, ctx);
		if (!decNumberIsNegative(&e) && !decNumberIsZero(&e))
			break;
		decNumberMultiply(mu(n+1), &const_0_5, &g, ctx);
		decNumberMultiply(&a, mu(n), nu(n), ctx);
		decNumberSquareRoot(nu(n+1), &a, ctx);
		n++;
		if (n >= ELLIPTIC_N-1)
			break;
	}

	decNumberMultiply(&a, u, mu(n), ctx);
	dn_sincos(&a, &sin_umu, &cos_umu, ctx);
	decNumberAbs(&a, &sin_umu, ctx);
	decNumberAbs(&b, &cos_umu, ctx);
	decNumberCompare(&e, &a, &b, ctx);
	if (decNumberIsNegative(&e))
		decNumberDivide(&t, &sin_umu, &cos_umu, ctx);
	else
		decNumberDivide(&t, &cos_umu, &sin_umu, ctx);

	decNumberMultiply(c(n), mu(n), &t, ctx);
	decNumberCopy(d(n), &const_1);

	while (n > 0) {
		n--;
		decNumberMultiply(c(n), d(n+1), c(n+1), ctx);
		decNumberSquare(&a, c(n+1), ctx);
		decNumberDivide(&r, &a, mu(n+1), ctx);
		decNumberAdd(&a, &r, nu(n), ctx);
		decNumberAdd(&b, &r, mu(n), ctx);
		decNumberDivide(d(n), &a, &b, ctx);
	}
	cmplxAbs(&f, &b, &const_1, c(0), ctx);
	if (decNumberIsNegative(&e)) {
		decNumberSubtract(&a, &const_1, m, ctx);
		decNumberSquareRoot(&g, &a, ctx);
		decNumberDivide(dn, &g, d(0), ctx);

		decNumberDivide(cn, dn, &f, ctx);
		if (decNumberIsNegative(&cos_umu))
			decNumberMinus(cn, cn, ctx);

		decNumberDivide(&a, c(0), &g, ctx);
		decNumberMultiply(sn, cn, &a, ctx);
	} else {
		decNumberCopy(dn, d(0));

		decNumberDivide(sn, &const_1, &f, ctx);
		if (decNumberIsNegative(&sin_umu))
			decNumberMinus(sn, sn, ctx);
		decNumberMultiply(cn, c(0), sn, ctx);
	}
#undef mu
#undef nu
#undef c
#undef d
}

decNumber *decNumberSN(decNumber *res, const decNumber *k, const decNumber *u, decContext *ctx) {
	dn_elliptic(res, NULL, NULL, u, k, ctx);
	return res;
}

decNumber *decNumberCN(decNumber *res, const decNumber *k, const decNumber *u, decContext *ctx) {
	dn_elliptic(NULL, res, NULL, u, k, ctx);
	return res;
}

decNumber *decNumberDN(decNumber *res, const decNumber *k, const decNumber *u, decContext *ctx) {
	dn_elliptic(NULL, NULL, res, u, k, ctx);
	return res;
}
#endif


#ifdef INCLUDE_BESSEL
static decNumber *dn_bessel(decNumber *res, const decNumber *alpha, const decNumber *x, decContext *ctx, const int neg) {
	decNumber q, r, m;
	decNumber x2on4, term, gfac;
	int n;

	decNumberMultiply(&q, x, &const_0_5, ctx);	// q = x/2
	decNumberSquare(&x2on4, &q, ctx);		// factor each time around
	decNumberPower(&r, &q, alpha, ctx);		// (x/2)^(2m+alpha)

	decNumberAdd(&gfac, alpha, &const_1, ctx);
	decNumberGamma(&q, &gfac, ctx);
	decNumberDivide(&term, &r, &q, ctx);
	decNumberCopy(res, &term);			// first term in series

	decNumberZero(&m);

	for (n=0; n<1000; n++) {
		decNumberMultiply(&q, &term, &x2on4, ctx);
		dn_inc(&m, ctx);			// m = m+1
		decNumberDivide(&r, &q, &m, ctx);
		decNumberDivide(&term, &r, &gfac, ctx);
		dn_inc(&gfac, ctx);
		if (neg)
			decNumberMinus(&term, &term, ctx);
		decNumberAdd(&q, &term, res, ctx);
		decNumberCompare(&r, &q, res, ctx);
		if (decNumberIsZero(&r))
			return res;
		decNumberCopy(res, &q);
	}
	set_NaN(res);
	return res;
}

decNumber *decNumberBSJN(decNumber *res, const decNumber *alpha, const decNumber *x, decContext *ctx) {
	decNumber a;

	if (decNumberIsNaN(alpha) || decNumberIsSpecial(x))
		set_NaN(res);
	else if (decNumberIsZero(x)) {
		if (decNumberIsZero(alpha))
			decNumberCopy(res, &const_1);
		else
			decNumberZero(res);
	} else {
		if (decNumberIsNegative(alpha) && is_int(alpha, ctx)) {
			decNumberAbs(&a, alpha, ctx);
			alpha = &a;
		}
		dn_bessel(res, alpha, x, ctx, 1);
	}
	return res;
}

decNumber *decNumberBSIN(decNumber *res, const decNumber *alpha, const decNumber *x, decContext *ctx) {
	decNumber a;

	if (decNumberIsNaN(alpha) || decNumberIsNaN(x))
		set_NaN(res);
	else if (decNumberIsInfinite(x))
		set_inf(res);
	else if (decNumberIsZero(x)) {
		if (decNumberIsZero(alpha))
			decNumberCopy(res, &const_1);
		else
			decNumberZero(res);
	} else {
		if (decNumberIsNegative(alpha) && is_int(alpha, ctx)) {
			decNumberAbs(&a, alpha, ctx);
			alpha = &a;
		}
		dn_bessel(res, alpha, x, ctx, 0);
	}
	return res;
}

// See A&S page 360 section 9.1.11
static void bessel2_int_series(decNumber *res, const decNumber *n, const decNumber *x, decContext *ctx, int modified) {
	const decNumber *const factor = modified?&const_0_5:&const__1onPI;
	decNumber xon2, xon2n, x2on4;
	decNumber k, npk, t, u, v, s, p, nf, absn;
	int i, in, n_odd, n_neg;

	if (decNumberIsNegative(n)) {
		n = decNumberAbs(&absn, n, ctx);
		n_neg = 1;
	} else	n_neg = 0;
	in = dn_to_int(n, ctx);
	n_odd = in & 1;

	decNumberMultiply(&xon2, x, &const_0_5, ctx);	// xon2 = x/2
	decNumberPower(&xon2n, &xon2, n, ctx);		// xon2n = (x/2)^n
	decNumberSquare(&x2on4, &xon2, ctx);		// x2on4 = +/- x^2/4

	if (modified)
		decNumberMinus(&x2on4, &x2on4, ctx);
	if (in > 0) {
		decNumberSubtract(&v, n, &const_1, ctx);	// v = n-k-1 = n-1
		decNumberZero(&k);
		decNumberGamma(&p, n, ctx);		// p = (n-1)!
		decNumberCopy(&s, &p);
		decNumberMultiply(&nf, &p, n, ctx);	// nf = n!  (for later)
		for (i=1; i<in; i++) {
			decNumberDivide(&t, &p, &v, ctx);
			dn_dec(&v, ctx);
			dn_inc(&k, ctx);
			decNumberMultiply(&u, &t, &k, ctx);
			decNumberMultiply(&p, &u, &x2on4, ctx);
			decNumberAdd(&s, &s, &p, ctx);
		}
		decNumberMultiply(&t, &s, factor, ctx);
		decNumberDivide(res, &t, &xon2n, ctx);
	} else {
		decNumberZero(res);
		decNumberCopy(&nf, &const_1);
	}

	if (modified) {
		decNumberBSIN(&t, n, x, ctx);
		if (!n_odd)
			decNumberMinus(&t, &t, ctx);
	} else {
		decNumberBSJN(&u, n, x, ctx);
		decNumberDivide(&t, &u, &const_PIon2, ctx);
	}
	decNumberLn(&u, &xon2, ctx);
	decNumberMultiply(&v, &u, &t, ctx);
	decNumberAdd(res, res, &v, ctx);

	decNumberMinus(&x2on4, &x2on4, ctx);
	decNumberAdd(&t, n, &const_1, ctx);		// t = n+1
	decNumberPsi(&u, &t, ctx);			// u = Psi(n+1)
	decNumberSubtract(&v, &u, &const_egamma, ctx);	// v = psi(k+1) + psi(n+k+1)
	decNumberZero(&k);
	decNumberCopy(&npk, n);
	decNumberRecip(&p, &nf, ctx);			// p = (x^2/4)^k/(k!(n+k)!)
	decNumberMultiply(&s, &v, &p, ctx);

	for (i=0;i<1000;i++) {
		dn_inc(&k, ctx);
		dn_inc(&npk, ctx);
		decNumberMultiply(&t, &p, &x2on4, ctx);
		decNumberMultiply(&u, &k, &npk, ctx);
		decNumberDivide(&p, &t, &u, ctx);

		decNumberRecip(&t, &k, ctx);
		decNumberAdd(&u, &v, &t, ctx);
		decNumberRecip(&t, &npk, ctx);
		decNumberAdd(&v, &u, &t, ctx);

		decNumberMultiply(&t, &v, &p, ctx);
		decNumberAdd(&u, &t, &s, ctx);
		decNumberCompare(&t, &u, &s, ctx);
		if (decNumberIsZero(&t))
			break;
		decNumberCopy(&s, &u);
	}
	decNumberMultiply(&t, &s, &xon2n, ctx);
	if (modified) {
		if (n_odd)
			decNumberMultiply(&u, &t, &const__0_5, ctx);
		else
			decNumberMultiply(&u, &t, &const_0_5, ctx);
	} else
		decNumberMultiply(&u, &t, &const__1onPI, ctx);
	decNumberAdd(res, res, &u, ctx);
	if (!modified && n_neg)
		decNumberMinus(res, res, ctx);
}


decNumber *decNumberBSYN(decNumber *res, const decNumber *alpha, const decNumber *x, decContext *ctx) {
	decNumber t, u, s, c;

	if (decNumberIsNaN(alpha) || decNumberIsSpecial(x))
		set_NaN(res);
	else if (decNumberIsZero(x))
		set_neginf(res);
	else if (decNumberIsInfinite(alpha) || decNumberIsNegative(x))
		set_NaN(res);
	else if (!is_int(alpha, ctx)) {
		decNumberMultiply(&t, alpha, &const_PI, ctx);
		dn_sincos(&t, &s, &c, ctx);
		dn_bessel(&t, alpha, x, ctx, 1);
		decNumberMultiply(&u, &t, &c, ctx);
		decNumberMinus(&c, alpha, ctx);
		dn_bessel(&t, &c, x, ctx, 1);
		decNumberSubtract(&c, &u, &t, ctx);
		decNumberDivide(res, &c, &s, ctx);
	} else
		bessel2_int_series(res, alpha, x, ctx, 0);
	return res;
}

decNumber *decNumberBSKN(decNumber *res, const decNumber *alpha, const decNumber *x, decContext *ctx) {
	decNumber t, u, v;

	if (decNumberIsNaN(alpha) || decNumberIsNaN(x))
		set_NaN(res);
	else if (decNumberIsZero(x))
		set_inf(res);
	else if (decNumberIsInfinite(alpha) || decNumberIsNegative(x))
		set_NaN(res);
	else if (decNumberIsInfinite(x))
		decNumberZero(res);
	else if (!is_int(alpha, ctx)) {
		dn_bessel(&t, alpha, x, ctx, 0);
		decNumberMinus(&u, alpha, ctx);
		dn_bessel(&v, &u, x, ctx, 0);
		decNumberSubtract(&u, &v, &t, ctx);
		decNumberMultiply(&v, &u, &const_PIon2, ctx);

		decNumberMultiply(&t, alpha, &const_PI, ctx);
		dn_sincos(&t, &u, NULL, ctx);
		decNumberDivide(res, &v, &u, ctx);
	} else
		bessel2_int_series(res, alpha, x, ctx, 1);
	return res;
}
#endif


/* Sovler code from here */


/* Secant iteration */
static void solve_secant(decNumber *s, const decNumber *a, const decNumber *b, const decNumber *fa, const decNumber *fb, decContext *ctx) {
	decNumber x, y, z;

	decNumberSubtract(&x, b, a, ctx);
	decNumberSubtract(&y, fb, fa, ctx);
	decNumberDivide(&z, &x, &y, ctx);
	decNumberMultiply(&x, &z, fb, ctx);
	decNumberSubtract(s, b, &x, ctx);
}

/* A third of the inverse quadratic interpolation step.
 * Return non-zero is one ofthe denominators is zero.
 */
static int qstep(decNumber *r, const decNumber *a, const decNumber *fb, const decNumber *fc, const decNumber *fa, decContext *ctx) {
	decNumber x, y, z;

	decNumberSubtract(&x, fa, fb, ctx);
	if (decNumberIsZero(&x))
		return -1;
	decNumberSubtract(&y, fa, fc, ctx);
	if (decNumberIsZero(&y))
		return -1;
	decNumberMultiply(&z, &x, &y, ctx);
	decNumberMultiply(&x, a, fb, ctx);
	decNumberMultiply(&y, &x, fc, ctx);
	decNumberDivide(r, &y, &z, ctx);
	return 0;
}

/* Inverse quadratic interpolation.
 * Return non-zero if interpolation fails due to equal function values
 */
static int solve_quadratic(decNumber *s, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *fa, const decNumber *fb, const decNumber *fc, decContext *ctx) {
	decNumber x, y, z;

	if (qstep(&x, a, fb, fc, fa, ctx))
		return -1;
	if (qstep(&y, b, fa, fc, fb, ctx))
		return -1;
	decNumberAdd(&z, &x, &y, ctx);
	qstep(&x, c, fa, fb, fc, ctx);
	decNumberAdd(s, &z, &x, ctx);
	return 0;
}

#ifdef USE_RIDDERS
/* Ridder's method
 */
static int solve_ridder(decNumber *xnew, const decNumber *x0, const decNumber *x2, const decNumber *x1, const decNumber *y0, const decNumber *y2, const decNumber *y1, decContext *ctx) {
	decNumber a, b, r, s, t, u, v;

	decNumberSubtract(&r, y1, y2, ctx);
	if (decNumberIsZero(&r))
		return -1;
	decNumberSubtract(&s, y0, y1, ctx);		// s = y0 - y1
	decNumberDivide(&a, &s, &r, ctx);
	decNumberMultiply(&t, &a, y1, ctx);
	decNumberSubtract(&r, y0, &t, ctx);
	if (decNumberIsZero(&r))
		return -1;
	decNumberDivide(&b, &s, &r, ctx);
	decNumberAdd(&r, &b, &const_1, ctx);
	if (decNumberIsZero(&r))
		return -1;
	decNumberSubtract(&s, &b, &const_1, ctx);
	decNumberDivide(&u, &s, &r, ctx);
	decNumberAdd(&r, &a, &const_1, ctx);
	if (decNumberIsZero(&r))
		return -1;
	decNumberSubtract(&s, &a, &const_1, ctx);
	decNumberDivide(&v, &s, &r, ctx);

	decNumberSquare(&r, &v, ctx);
	decNumberAdd(&s, &r, &const_3, ctx);
	decNumberMultiply(&t, &s, &v, ctx);
	decNumberSquare(&r, &u, ctx);
	decNumberAdd(&s, &r, &const_3, ctx);
	decNumberMultiply(&r, &s, &u, ctx);
	decNumberDivide(&s, &r, &t, ctx);
	decNumberSubtract(&r, x1, x0, ctx);
	decNumberAbs(&t, &r, ctx);
	decNumberMultiply(&r, &t, &s, ctx);
	decNumberAdd(xnew, &r, x1, ctx);
	return 0;
}
#endif


/* perform a bisection step.
 */
static void solve_bisect(decNumber *s, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumber x;

	decNumberAdd(&x, a, b, ctx);
	decNumberMultiply(s, &x, &const_0_5, ctx);
}


/* Check if the new point is inside the bracketed interval, if not do a bisection
 * step instead.  This means we'll not escape a bracketed interval ever.
 */
static int solve_bracket(decNumber *s, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumber x, y;

	decNumberSubtract(&x, a, b, ctx);
	if (decNumberIsNegative(&x)) {		// a<b
		decNumberSubtract(&y, s, a, ctx);
		decNumberSubtract(&x, b, s, ctx);
	} else {				// a>b
		decNumberSubtract(&y, s, b, ctx);
		decNumberSubtract(&x, a, s, ctx);
	}
	/* If out of bracket or the same as a previous, out of bracket */
	return (decNumberIsNegative(&y) || decNumberIsZero(&y) ||
		decNumberIsNegative(&x) || decNumberIsZero(&x));
}


/* Limit the distance a new estimate can be to within 100 times the distance
 * between the existing points.
 */
static void limit_jump(decNumber *s, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumber x, y, z;

	decNumberSubtract(&x, a, b, ctx);
	decNumberAbs(&y, &x, ctx);
	decNumberMultiply(&x, &y, &const_100, ctx);	// 100 |a-b|
	decNumberSubtract(&y, a, &x, ctx);
	decNumberCompare(&z, s, &y, ctx);
	if (decNumberIsNegative(&z)) {
		decNumberCopy(s, &z);
		return;
	}
	decNumberAdd(&y, b, &x, ctx);
	decNumberCompare(&z, &y, s, ctx);
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


/* Compare two numbers to see if they are mostly equal
 */
static int slv_compare(const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumber ar, br, c;

	decNumberRnd(&ar, a, ctx);
	decNumberRnd(&br, b, ctx);
	decNumberCompare(&c, &ar, &br, ctx);
	return decNumberIsZero(&c);
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
		decNumber *fa, decNumber *fb, const decNumber *fc, decContext *ctx,
		unsigned int *statep) {
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
			r = solve_ridder(&q, a, b, c, fa, fb, fc, ctx);
		else
#endif
			r = solve_quadratic(&q, a, b, c, fa, fb, fc, ctx);
		s1 = decNumberIsNegative(fc);
		s2 = decNumberIsNegative(fb);
		if (s1 != s2) {
			decNumberCopy(a, c);
			decNumberCopy(fa, fc);
			decNumberMultiply(&y, b, &const_3, ctx);
		} else {
			decNumberCopy(b, c);
			decNumberCopy(fb, fc);
			decNumberMultiply(&y, a, &const_3, ctx);
		}
		decNumberAdd(&z, &y, c, ctx);
		decNumberMultiply(&y, &z, &const_0_25, ctx);
		if (r)
			solve_secant(&q, a, b, fa, fb, ctx);
		if (solve_bracket(&q, &y, c, ctx)) {
			solve_bisect(&q, a, b, ctx);
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

			decNumberCompare(&x, fb, fc, ctx);
			if (! decNumberIsZero(&x)) {
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
					decNumberMultiply(&x, a, &const_2, ctx);
				else	decNumberMultiply(&x, a, &const_0_5, Ctx);
				decNumberSubtract(&q, &x, &const_10, Ctx);
			} else {
				decNumberCopy(a, c);
				decNumberCopy(fa, fc);
				if (decNumberIsNegative(b))
					decNumberMultiply(&x, b, &const_0_5, ctx);
				else	decNumberMultiply(&x, b, &const_2, Ctx);
				decNumberAdd(&q, &x, &const_10, Ctx);
			}
		} else {
			count = SLV_COUNT(slv_state);
			if (count >= ONESIDE_MAXCOUNT)
				goto failed;
			slv_state = SLV_SET_COUNT(slv_state, count+1);
nonconst:
			r = solve_quadratic(&q, a, b, c, fa, fb, fc, ctx);
			//MORE: need to check if the new point is worse than the old.
			decNumberAbs(&x, fa, Ctx);
			decNumberAbs(&y, fb, Ctx);
			decNumberCompare(&z, &x, &y, Ctx);
			if (decNumberIsNegative(&z)) {
				decNumberCopy(a, c);
				decNumberCopy(fa, fc);
			} else {
				decNumberCopy(b, c);
				decNumberCopy(fb, fc);
			}
			decNumberCompare(&x, b, a, Ctx);
			if (decNumberIsNegative(&x)) {
				decNumberSwap(a, b);
				decNumberSwap(fa, fb);
			}
			if (r)
				solve_secant(&q, a, b, fa, fb, ctx);
			limit_jump(&q, a, b, ctx);
		}
	}
	if (slv_compare(c, &q, ctx)) goto failed;
	if (slv_compare(a, &q, ctx)) goto failed;
	if (slv_compare(b, &q, ctx)) goto failed;
	decNumberCopy(c, &q);
	*statep = slv_state;
	return 0;
failed:
	*statep = slv_state;
	return -1;
}

void solver_init(decNumber *c, decNumber *a, decNumber *b, decNumber *fa, decNumber *fb, decContext *ctx, unsigned int *flagp) {
	int sa, sb;
	decNumber x, y;
	unsigned int flags = 0;

	decNumberCompare(&x, b, a, Ctx);
	if (decNumberIsZero(&x)) {
		decNumberAdd(b, a, &const_10, Ctx);
		decNumberMultiply(fb, fa, &const_10, Ctx);
		goto cnst;
	} else if (decNumberIsNegative(&x)) {
		decNumberSwap(a, b);
		decNumberSwap(fa, fb);
	}
	sa = decNumberIsNegative(fa);
	sb = decNumberIsNegative(fb);
	if (sa == sb) {				// Same side of line
		decNumberCompare(&y, fa, fb, Ctx);
		if (decNumberIsZero(&y)) {	// Worse equal...
cnst:			SET_CONST(flags);
			decNumberMultiply(&x, a, &const_2, Ctx);
			if (decNumberIsNegative(&x))
				decNumberSubtract(c, &x, &const_10, Ctx);
			else
				decNumberAdd(c, &x, &const_10, Ctx);
		} else {
			solve_secant(c, a, b, fa, fb, Ctx);
			limit_jump(c, a, b, Ctx);
		}
	} else {
		SET_BRACKET(flags);
		solve_secant(c, a, b, fa, fb, Ctx);
		if (solve_bracket(c, a, b, Ctx)) {
			solve_bisect(c, a, b, Ctx);
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
void solver(unsigned int arg, enum rarg op) {
	decNumber a, b, c, fa, fb, fc;
	unsigned int flags;
	int r;

	get_reg_n_as_dn(arg + 0, &a);
	get_reg_n_as_dn(arg + 1, &b);
	get_reg_n_as_dn(arg + 3, &fa);
	get_reg_n_as_dn(arg + 4, &fb);

	if (op == RARG_INISOLVE) {
		solver_init(&c, &a, &b, &fa, &fb, Ctx, &flags);
	} else {
		get_reg_n_as_dn(arg + 2, &c);
		flags = 0;
		for (r=0; r<8; r++)
			if (get_user_flag(arg + r + _FLAG_COUNT_N))
				flags |= 1<<r;
		flags = SLV_SET_COUNT(0, flags);

		if (get_user_flag(arg + _FLAG_BRACKET_N))
			SET_BRACKET(flags);
		if (get_user_flag(arg + _FLAG_CONST_N))
			SET_CONST(flags);
#ifdef USE_RIDDERS
		if (get_user_flag(arg + _FLAG_BISECT_N))
			SET_BISECT(flags);
#endif

		getX(&fc);
		r = solver_step(&a, &b, &c, &fa, &fb, &fc, Ctx, &flags);
		setX(r==0?&const_0:&const_1);
	}

	put_reg_n(arg + 0, &a);
	put_reg_n(arg + 1, &b);
	put_reg_n(arg + 2, &c);
	put_reg_n(arg + 3, &fa);
	put_reg_n(arg + 4, &fb);

	put_user_flag(arg + _FLAG_BRACKET_N, IS_BRACKET(flags));
	put_user_flag(arg + _FLAG_CONST_N, IS_CONST(flags));
#ifdef USE_RIDDERS
	put_user_flag(arg + _FLAG_BISECT_N, IS_BISECT(flags));
#endif
	flags = SLV_COUNT(flags);
	for (r=0; r<8; r++)
		put_user_flag(arg + r + _FLAG_COUNT_N, flags & (1<<r));
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

static decNumber *ortho_poly(decNumber *r, const decNumber *param, const decNumber *rn, const decNumber *x, decContext *ctx, const enum eOrthoPolys type) {
	decNumber t0, t1, t, u, v, A, B, C, dA;
	unsigned int i, n;
	int incA, incB, incC;

	// Get argument and parameter
	if (decNumberIsSpecial(x) || decNumberIsSpecial(rn) || (decNumberIsNegative(rn) && !decNumberIsZero(rn))) {
error:		set_NaN(r);
		return r;
	}
	if (! is_int(rn, ctx))
		goto error;
	n = dn_to_int(rn, ctx);
	if (n > 1000)
		goto error;
//	if (type == ORTHOPOLY_GEN_LAGUERRE) {
	if (param != NULL) {
		if (decNumberIsSpecial(param))
			goto error;
		decNumberAdd(&t, param, &const_1, ctx);
		if (decNumberIsNegative(&t) || decNumberIsZero(&t))
			goto error;
	}
//	} else
//		param = &const_0;

	// Initialise the first two values t0 and t1
	switch (type) {
	default:
		decNumberCopy(&t1, x);
		break;
	case ORTHOPOLY_CHEBYCHEV_UN:
		decNumberMultiply(&t1, x, &const_2, ctx);
		break;
	case ORTHOPOLY_GEN_LAGUERRE:
		decNumberAdd(&t, &const_1, param, ctx);
		decNumberSubtract(&t1, &t, x, ctx);
		break;
	case ORTHOPOLY_HERMITE_H:
		decNumberMultiply(&t1, x, &const_2, ctx);
		break;
	}
	decNumberCopy(&t0, &const_1);

	if (n < 2) {
		if (n == 0)
			decNumberCopy(r, &t0);
		else
			decNumberCopy(r, &t1);
		return r;
	}

	// Prepare for the iteration
	decNumberCopy(&dA, &const_2);
	decNumberCopy(&C, &const_1);
	decNumberCopy(&B, &const_1);
	decNumberMultiply(&A, x, &const_2, ctx);
	incA = incB = incC = 0;
	switch (type) {
	case ORTHOPOLY_LEGENDRE_PN:
		incA = incB = incC = 1;
		decNumberAdd(&A, &A, x, ctx);
		decNumberMultiply(&dA, x, &const_2, ctx);
		break;
	case ORTHOPOLY_CHEBYCHEV_TN:	break;
	case ORTHOPOLY_CHEBYCHEV_UN:	break;
	case ORTHOPOLY_GEN_LAGUERRE:
		decNumberAdd(&B, &B, param, ctx);
		incA = incB = incC = 1;
		decNumberAdd(&t, &const_3, param, ctx);
		decNumberSubtract(&A, &t, x, ctx);
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
		decNumberMultiply(&t, &t1, &A, ctx);
		decNumberMultiply(&u, &t0, &B, ctx);
		decNumberSubtract(&v, &t, &u, ctx);
		decNumberCopy(&t0, &t1);
		if (incC) {
			decNumberAdd(&C, &C, &const_1, ctx);
			decNumberDivide(&t1, &v, &C, ctx);
		} else
			decNumberCopy(&t1, &v);
		if (incA)
			decNumberAdd(&A, &A, &dA, ctx);
		if (incB)
			decNumberAdd(&B, &B, small_int(incB), ctx);
	}
	return decNumberCopy(r, &t1);
}

decNumber *decNumberPolyPn(decNumber *r, const decNumber *y, const decNumber *x, decContext *ctx) {
	return ortho_poly(r, NULL, y, x, ctx, ORTHOPOLY_LEGENDRE_PN);
}

decNumber *decNumberPolyTn(decNumber *r, const decNumber *y, const decNumber *x, decContext *ctx) {
	return ortho_poly(r, NULL, y, x, ctx, ORTHOPOLY_CHEBYCHEV_TN);
}

decNumber *decNumberPolyUn(decNumber *r, const decNumber *y, const decNumber *x, decContext *ctx) {
	return ortho_poly(r, NULL, y, x, ctx, ORTHOPOLY_CHEBYCHEV_UN);
}

decNumber *decNumberPolyLn(decNumber *r, const decNumber *y, const decNumber *x, decContext *ctx) {
	return ortho_poly(r, &const_0, y, x, ctx, ORTHOPOLY_GEN_LAGUERRE);
}

decNumber *decNumberPolyLnAlpha(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x, decContext *ctx) {
	return ortho_poly(r, z, y, x, ctx, ORTHOPOLY_GEN_LAGUERRE);
}

decNumber *decNumberPolyHEn(decNumber *r, const decNumber *y, const decNumber *x, decContext *ctx) {
	return ortho_poly(r, NULL, y, x, ctx, ORTHOPOLY_HERMITE_HE);
}

decNumber *decNumberPolyHn(decNumber *r, const decNumber *y, const decNumber *x, decContext *ctx) {
	return ortho_poly(r, NULL, y, x, ctx, ORTHOPOLY_HERMITE_H);
}


#ifdef INCLUDE_BERNOULLI
decNumber *decNumberBernBn(decNumber *r, const decNumber *n, decContext *ctx) {
	decNumber a, b;

	if (decNumberIsZero(n))
		return decNumberCopy(r, &const_1);
	if (! is_int(n, ctx) || decNumberIsNegative(n)) {
		set_NaN(r);
		return r;
	}
	decNumberSubtract(&a, &const_1, n, ctx);
	if (decNumberIsZero(&a))
		return decNumberCopy(r, &const__0_5);
	if (is_even(n)) {
		decNumberZeta(&b, &a, ctx);
		decNumberMultiply(&a, &b, n, ctx);
		return decNumberMinus(r, &a, ctx);
	}
	decNumberZero(r);
	return r;
	
}

decNumber *decNumberBernBnS(decNumber *r, const decNumber *n, decContext *ctx) {
	decNumber a;

	if (decNumberIsZero(n)) {
		set_NaN(r);
		return r;
	}
	decNumberMultiply(&a, n, &const_2, ctx);
	decNumberBernBn(r, &a, ctx);
	if (is_even(n))
		decNumberMinus(r, r, ctx);
	return r;
}
#endif

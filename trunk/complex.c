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
#include "complex.h"
#include "xeq.h"
#include "consts.h"

#if 0
#include <stdio.h>
static FILE *debugf = NULL;

static void open_debug(void) {
	if (debugf == NULL) {
		debugf = fopen("/dev/ttys000", "w");
	}
}
static void dump1(const decNumber *a, const char *msg) {
	char b[100];

	open_debug();
	decNumberToString(a, b);
	fprintf(debugf, "%s: %s\n", msg, b);
}

static void dump2(const decNumber *a, const decNumber *b, const char *msg) {
	char b1[100], b2[100];

	open_debug();
	decNumberToString(a, b1);
	decNumberToString(b, b2);
	fprintf(debugf, "%s: %s / %s\n", msg, b1, b2);
}
#endif

void cmplx_NaN(decNumber *x, decNumber *y) {
	set_NaN(x);
	set_NaN(y);
}

#ifndef TINY_BUILD
static void cmplxCopy(decNumber *rx, decNumber *ry, const decNumber *x, const decNumber *y) {
	decNumberCopy(rx, x);
	decNumberCopy(ry, y);
}

// r - (a + i b) = (r - a) - i b
static void cmplxSubtractFromReal(decNumber *rx, decNumber *ry,
		const decNumber *r,
		const decNumber *a, const decNumber *b) {
	dn_subtract(rx, r, a);
	dn_minus(ry, b);
}

// (a + i b) * r = (a * r) + i (b * r)
static void cmplxMultiplyReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r) {
	dn_multiply(rx, a, r);
	dn_multiply(ry, b, r);
}

// (a + i b) / c = ( a / r ) + i ( b / r)
static void cmplxDivideReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r) {
	dn_divide(rx, a, r);
	dn_divide(ry, b, r);
}

// a / (c + i d) = (a*c) / (c*c + d*d) + i (- a*d) / (c*c + d*d)
static void cmplxDivideRealBy(decNumber *rx, decNumber *ry,
		const decNumber *a,
		const decNumber *c, const decNumber *d) {
	decNumber t1, t2, t3, den;

	dn_multiply(&t1, c, c);
	dn_multiply(&t2, d, d);
	dn_add(&den, &t1, &t2);

	dn_multiply(&t1, a, c);
	dn_divide(rx, &t1, &den);

	dn_multiply(&t2, a, d);
	dn_minus(&t3, &t2);
	dn_divide(ry, &t3, &den);
}
#endif

// (a + i b) + (c + i d) = (a + c) + i (b + d)
void cmplxAdd(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
	dn_add(rx, a, c);
	dn_add(ry, b, d);
}

// (a + i b) - (c + i d) = (a - c) + i (b - d)
void cmplxSubtract(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
	dn_subtract(rx, a, c);
	dn_subtract(ry, b, d);
}

// (a + i b) * (c + i d) = (a * c - b * d) + i (a * d + b * c)
void cmplxMultiply(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
	decNumber t1, t2, u1, u2;

	dn_multiply(&t1, a, c);
	dn_multiply(&t2, b, d);
	dn_multiply(&u1, a, d);
	dn_multiply(&u2, b, c);

	dn_subtract(rx, &t1, &t2);
	dn_add(ry, &u1, &u2);
}

// (a + i b) / (c + i d) = (a*c + b*d) / (c*c + d*d) + i (b*c - a*d) / (c*c + d*d)
void cmplxDivide(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
	decNumber t1, t2, t3, t4, den;

	dn_multiply(&t1, c, c);
	dn_multiply(&t2, d, d);
	dn_add(&den, &t1, &t2);

	dn_multiply(&t3, a, c);
	dn_multiply(&t2, b, d);
	dn_add(&t1, &t3, &t2);

	dn_multiply(&t4, b, c);
	dn_multiply(&t2, a, d);
	dn_subtract(&t3, &t4, &t2);

	dn_divide(rx, &t1, &den);
	dn_divide(ry, &t3, &den);
}

void cmplxArg(decNumber *arg, const decNumber *a, const decNumber *b) {
	do_atan2(arg, b, a);
}

void cmplxR(decNumber *r, const decNumber *a, const decNumber *b) {
	decNumber a2, b2, s;

	dn_multiply(&a2, a, a);
	dn_multiply(&b2, b, b);
	dn_add(&s, &a2, &b2);
	dn_sqrt(r, &s);
}

void cmplxFromPolar(decNumber *x, decNumber *y, const decNumber *r, const decNumber *t) {
	decNumber s, c;

	dn_sincos(t, &s, &c);
	dn_multiply(x, r, &c);
	dn_multiply(y, r, &s);
}

void cmplxToPolar(decNumber *r, decNumber *t, const decNumber *x, const decNumber *y) {
	do_atan2(t, y, x);
	cmplxR(r, y, x);
}



#ifndef TINY_BUILD
// ( a + i * b ) ^ r
static void cmplxPowerReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r) {
	cmplxPower(rx, ry, a, b, r, &const_0);
}
#endif

static void cmplxRealPower(decNumber *rx, decNumber *ry,
		const decNumber *r,
		const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	cmplxPower(rx, ry, r, &const_0, a, b);
#endif
}

// a ^ b = e ^ (b ln(a))
void cmplxPower(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber e1, e2, f1, f2;

	cmplxLn(&e1, &e2, a, b);
	cmplxMultiply(&f1, &f2, &e1, &e2, c, d);
	cmplxExp(rx, ry, &f1, &f2);
#endif
}

#ifdef INCLUDE_XROOT
// (a, b) ^ 1 / (c, d)
void cmplxXRoot(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
	decNumber g, h;

	cmplxRecip(&g, &h, c, d);
	cmplxPower(rx, ry, a, b, &g, &h);
}
#endif


// abs(a + i b) = sqrt(a^2 + b^2)
void cmplxAbs(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplxR(rx, a, b);
	decNumberZero(ry);
}

// sign(a + i b) = (a + i b) / |a + i b|
void cmplxSign(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber z;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b)) {
		if (decNumberIsNaN(a) || decNumberIsNaN(b))
			cmplx_NaN(rx, ry);
		else if (decNumberIsInfinite(a)) {
			if (decNumberIsInfinite(b))
				cmplx_NaN(rx, ry);
			else {
				decNumberSign(rx, a);
				decNumberZero(ry);
			}
		} else {
			decNumberSign(ry, b);
			decNumberZero(rx);
		}
		return;
	}
	if (decNumberIsZero(b)) {
		decNumberSign(rx, a);
		decNumberZero(ry);
	} else if (decNumberIsZero(a)) {
		decNumberZero(rx);
		decNumberSign(ry, b);
	} else {
		cmplxR(&z, a, b);
		cmplxDivideReal(rx, ry, a, b, &z);
	}
#endif
}

// - (a + i b) = - a - i b
void cmplxMinus(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	dn_minus(rx, a);
	dn_minus(ry, b);
}

// conj(a + i b) = a - i b
void cmplxConj(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumberCopy(rx, a);
	dn_minus(ry, b);
}

// 1 / (c + i d) = c / (c*c + d*d) + i (- d) / (c*c + d*d)
void cmplxRecip(decNumber *rx, decNumber *ry, const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber t, u, v, den;

	dn_multiply(&u, c, c);
	dn_multiply(&v, d, d);
	dn_add(&den, &u, &v);
	dn_minus(&t, d);

	dn_divide(rx, c, &den);
	dn_divide(ry, &t, &den);
#endif
}

// (a + ib)^2 = (a^2 - b^2) + i ( 2 * a * b )
void cmplxSqr(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber t, u, v;

	decNumberSquare(&t, a);
	decNumberSquare(&u, b);
	dn_multiply(&v, a, b);

	dn_subtract(rx, &t, &u);
	dn_add(ry, &v, &v);
#endif
}

// sqrt(a + i b) = +- (sqrt(r + a) + i sqrt(r - a) sign(b)) sqrt(2) / 2
//		where r = sqrt(a^2 + b^2)
void cmplxSqrt(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber fac, t1, u, v;

	if (decNumberIsZero(b)) {
		// Detect a purely real input and shortcut the computation
		if (decNumberIsNegative(a)) {
			dn_minus(&t1, a);
			dn_sqrt(ry, &t1);
			decNumberZero(rx);
		} else {
			dn_sqrt(rx, a);
			decNumberZero(ry);
		}
		return;
	} else {
		cmplxR(&fac, a, b);

		dn_subtract(&v, &fac, a);
		dn_sqrt(&u, &v);
		dn_add(&v, &fac, a);
		if (decNumberIsNegative(b)) {
			dn_minus(&t1, &u);
			dn_multiply(ry, &t1, &const_root2on2);
		} else
			dn_multiply(ry, &u, &const_root2on2);

		dn_sqrt(&t1, &v);
		dn_multiply(rx, &t1, &const_root2on2);
	}
#endif
}

// (a + ib)^3 = (a^2 + b^2) + i ( 2 * a * b ) . ( a + i b )
void cmplxCube(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber s1, s2;

	cmplxSqr(&s1, &s2, a, b);
	cmplxMultiply(rx, ry, &s1, &s2, a, b);
#endif
}

// Fairly naive implementation...
void cmplxCubeRoot(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber t;

	decNumberRecip(&t, &const_3);
	cmplxPowerReal(rx, ry, a, b, &t);
#endif
}


// sin(a + i b) = sin(a) cosh(b) + i cos(a) sinh(b)
// cos(a + i b) = cos(a) cosh(b) - i sin(a) sinh(b)
static void cmplx_sincos(const decNumber *a, const decNumber *b, decNumber *sx, decNumber *sy, decNumber *cx, decNumber *cy) {
#ifndef TINY_BUILD
	decNumber sa, ca, sb, cb;

	if (decNumberIsZero(a) && decNumberIsInfinite(b)) {
		if (sx != NULL)	decNumberZero(sx);
		if (sy != NULL)	decNumberCopy(sy, b);
		if (cx != NULL)	set_inf(cx);
		if (cy != NULL)	decNumberZero(cy);
	} else {
		dn_sincos(a, &sa, &ca);
		dn_sinhcosh(b, &sb, &cb);
		if (sx != NULL)	dn_multiply(sx, &sa, &cb);
		if (sy != NULL)	dn_multiply(sy, &ca, &sb);
		if (cx != NULL)	dn_multiply(cx, &ca, &cb);
		if (cy != NULL) {
			dn_multiply(&ca, &sa, &sb);
			dn_minus(cy, &ca);
		}
	}
#endif
}

// sin(a + i b) = sin(a) cosh(b) + i cos(a) sinh(b)
void cmplxSin(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_sincos(a, b, rx, ry, NULL, NULL);
}

// cos(a + i b) = cos(a) cosh(b) - i sin(a) sinh(b)
void cmplxCos(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_sincos(a, b, NULL, NULL, rx, ry);
}

// tan(a + i b) = (sin(a) cosh(b) + i cos(a) sinh(b)) / (cos(a) cosh(b) - i sin(a) sinh(b))
void cmplxTan(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber sb, cb;
	decNumber t1, t2, t3;

	if (decNumberIsZero(a) && decNumberIsInfinite(b)) {
		decNumberZero(rx);
		if (decNumberIsNegative(b))
			decNumberCopy(ry, &const__1);
		else
			decNumberCopy(ry, &const_1);
	} else {
		dn_sincos(a, rx, ry);
		dn_sinhcosh(b, &sb, &cb);

		dn_multiply(&t1, rx, &cb);
		dn_multiply(&t2, ry, &sb);
		dn_multiply(&t3, ry, &cb);
		dn_multiply(&cb, rx, &sb);

		cmplxDivide(rx, ry, &t1, &t2, &t3, &cb);
	}
#endif
}

// Helper for arcsin and arccos.
// alpha = 1/2 sqrt((x+1)^2+y^2) + 1/2 sqrt((x-1)^2+y^2)
// beta  = 1/2 sqrt((x+1)^2+y^2) - 1/2 sqrt((x-1)^2+y^2)
// la = ln(alpha + sqrt(alpha^2-1)
#ifndef TINY_BUILD
static void asinacos_chelper(decNumber *la, decNumber *b, const decNumber *x, const decNumber *y) {
	decNumber y2, x1, r, s, t;

	decNumberSquare(&y2, y);
	dn_add(&x1, x, &const_1);
	decNumberSquare(&t, &x1);
	dn_add(&r, &t, &y2);
	dn_sqrt(&t, &r);
	dn_multiply(&s, &t, &const_0_5);

	dn_subtract(&x1, x, &const_1);
	decNumberSquare(&t, &x1);
	dn_add(&r, &t, &y2);
	dn_sqrt(&t, &r);
	dn_multiply(&r, &t, &const_0_5);

	dn_add(&t, &s, &r);
	dn_subtract(b, &s, &r);

	// Now ln(alpha + sqrt(alpha*alpha-1))
	decNumberSquare(&r, &t);
	dn_subtract(&s, &r, &const_1);
	dn_sqrt(&r, &s);
	dn_add(&s, &r, &t);
	dn_ln(la, &s);
}
#endif

// arcsin(z) = k PI + (-1)^k . asin(beta) + i (-1)^k ln(alpha+sqrt(alpha^2-1))
void cmplxAsin(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber beta;

	asinacos_chelper(ry, &beta, a, b);
	do_asin(rx, &beta);
#endif
}

// arccos(z) = 2k PI +- (acos(beta) - i ln(alpha+sqrt(alpha^2-1)))
void cmplxAcos(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber beta;

	asinacos_chelper(ry, &beta, a, b);
	dn_minus(ry, ry);
	do_acos(rx, &beta);
#endif
}

// atan(z) = k PI + 0.5 atan(2a / (1-a^2-b^2) + i/4 ln((a^2+(b+1)^2)/(a^2+(b-1)^2))
//		z^2 <>-1
void cmplxAtan(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2, u, v1, v2;

	dn_add(&t1, b, &const_1);
	dn_minus(&t2, a);
	cmplxLn(&s1, &s2, &t1, &t2);

	dn_subtract(&u, &const_1, b);
	cmplxLn(&v1, &v2, &u, a);

	dn_subtract(&t1, &s1, &v1);
	dn_subtract(&t2, &s2, &v2);

	dn_multiply(&s1, &const_0_5, &t2);
	dn_minus(rx, &s1);
	dn_multiply(ry, &const_0_5, &t1);
#endif
}

// sinc(a + i b) = sin(a + i b) / (a + i b)
void cmplxSinc(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber s1, s2;

	if (decNumberIsZero(b)) {
		decNumberSinc(rx, a);
		decNumberZero(ry);
	} else {
		cmplxSin(&s1, &s2, a, b);
		cmplxDivide(rx, ry, &s1, &s2, a, b);
	}
#endif
}

// sinh(a + i b) = sinh(a) cos(b) + i cosh(a) sin(b)
// cosh(a + i b) = cosh(a) cos(b) + i sinh(a) sin(b)
static void cmplx_sinhcosh(const decNumber *a, const decNumber *b, decNumber *sx, decNumber *sy, decNumber *cx, decNumber *cy) {
#ifndef TINY_BUILD
	decNumber sa, ca, sb, cb;

	if (decNumberIsZero(b)) {
		if (sx != NULL)	decNumberSinh(sx, a);
		if (sy != NULL)	decNumberZero(sy);
		if (cx != NULL)	decNumberCosh(cx, a);
		if (cy != NULL)	decNumberZero(cy);
	} else {
		dn_sinhcosh(a, &sa, &ca);
		dn_sincos(b, &sb, &cb);
		if (sx != NULL)	dn_multiply(sx, &sa, &cb);
		if (sy != NULL)	dn_multiply(sy, &ca, &sb);
		if (cx != NULL)	dn_multiply(cx, &ca, &cb);
		if (cy != NULL)	dn_multiply(cy, &sa, &sb);
	}
#endif
}

// sinh(a + i b) = sinh(a) cos(b) + i cosh(a) sin(b)
void cmplxSinh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_sinhcosh(a, b, rx, ry, NULL, NULL);
}

// cosh(a + i b) = cosh(a) cos(b) + i sinh(a) sin(b)
void cmplxCosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_sinhcosh(a, b, NULL, NULL, rx, ry);
}

// tanh(a + i b) = (tanh(a) + i tan(b))/(1 + i tanh(a) tan(b))
void cmplxTanh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber ta, tb, t2;

	if (decNumberIsZero(b)) {
		decNumberTanh(rx, a);
		decNumberZero(ry);
	} else {
		decNumberTanh(&ta, a);
		decNumberTan(&tb, b);

		dn_multiply(&t2, &ta, &tb);

		cmplxDivide(rx, ry, &ta, &tb, &const_1, &t2);
	}
#endif
}

static void cmplx_asinhacosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *add) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2;

	cmplxSqr(&s1, &t2, a, b);

	dn_add(&t1, &s1, add);

	cmplxSqrt(&s1, &s2, &t1, &t2);
	dn_add(&t1, &s1, a);
	dn_add(&t2, &s2, b);

	cmplxLn(rx, ry, &t1, &t2);
#endif
}

// arcsinh(a + i b) = ln((a + i b) + sqrt((a + i b) (a + i b) + 1)
void cmplxAsinh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_asinhacosh(rx, ry, a, b, &const_1);
}

// arccosh(a + i b) = ln((a + i b) + sqrt((a + i b) (a + i b) - 1)
void cmplxAcosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_asinhacosh(rx, ry, a, b, &const__1);
}

// arctanh(a + i b) = (1/2)*ln((1 + (a + i b))/(1 - (a + i b)))
void cmplxAtanh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2;

	dn_add(&t1, a, &const_1);
	cmplxSubtractFromReal(&t2, ry, &const_1, a, b);

	cmplxDivide(&s1, &s2, &t1, b, &t2, ry);
	cmplxLn(&t1, &t2, &s1, &s2);

	cmplxMultiplyReal(rx, ry, &t1, &t2, &const_0_5);
#endif
}


void cmplxLn1p(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber t;

	if (decNumberIsZero(b)) {
		dn_abs(&t, a);
		dn_compare(&t, &t, &const_0_0001);
		if (decNumberIsNegative(&t)) {
			decNumberZero(ry);
			decNumberLn1p(rx, a);
			return;
		}
	}
	dn_add(&t, a, &const_1);
	cmplxLn(rx, ry, &t, b);
#endif
}

void cmplxExpm1(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber t;

	if (decNumberIsZero(b)) {
		dn_abs(&t, a);
		dn_compare(&t, &t, &const_0_0001);
		if (decNumberIsNegative(&t)) {
			decNumberZero(ry);
			decNumberExpm1(rx, a);
			return;
		}
	}
	cmplxExp(&t, ry, a, b);
	dn_subtract(rx, &t, &const_1);
#endif
}


void cmplx_do_log(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *base) {
#ifndef TINY_BUILD
	decNumber s1, s2;

	if (decNumberIsInfinite(a) || decNumberIsInfinite(b)) {
		set_inf(rx);
		decNumberZero(ry);
	} else {
		cmplxLn(&s1, &s2, a, b);
		cmplxDivideReal(rx, ry, &s1, &s2, base);
	}
#endif
}

void cmplxLog(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_do_log(rx, ry, a, b, &const_ln10);
}

void cmplx10x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplxRealPower(rx, ry, &const_10, a, b);
}

void cmplxLog2(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_do_log(rx, ry, a, b, &const_ln2);
}

void cmplx2x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplxRealPower(rx, ry, &const_2, a, b);
}

void cmplx_1x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplxRealPower(rx, ry, &const__1, a, b);
}

void cmplxLogxy(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber la, lb, lc, ld;

	cmplxLn(&la, &lb, a, b);
	cmplxLn(&lc, &ld, c, d);
	cmplxDivide(rx, ry, &la, &lb, &lc, &ld);
#endif
}

void cmplxlamW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2, u1, u2, v1, v2, w1, w2;
	int i;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b)) {
		if (decNumberIsNaN(a) || decNumberIsNaN(b))
			cmplx_NaN(rx, ry);
		else {
			set_inf(rx);
			decNumberZero(ry);
		}
		return;
	}

	// Make an initial guess as to the value
	dn_compare(&t1, &const_500, a);
	if (! decNumberIsNegative(&t1)) {
		// x<500, lx1 = ln(x+1); est = .665 * (1 + .0195*lx1) * lx1 + 0.04
		dn_add(&t1, a, &const_1);
		cmplxLn(&w1, &w2, &t1, b);				// ln(1+a,b)
		cmplxMultiplyReal(&t1, &s2, &w1, &w2, &const_0_0195);
		dn_add(&s1, &const_1, &t1);
		cmplxMultiplyReal(&u1, &u2, &s1, &s2, &const_0_665);
		cmplxMultiply(&t1, ry, &u1, &u2, &w1, &w2);
		dn_add(rx, &const_0_04, &t1);
	} else {
		// x>=500, est = ln(x-4) - (1 - 1/ln(x)) * ln(ln(x))
		cmplxLn(&w1, &w2, a, b);				// ln(a,b)
		cmplxLn(&t1, &t2, &w1, &w2);			// ln(ln(a,b))
		cmplxRecip(&u1, &u2, &w1, &w2);			// 1 / ln(a,b)
		cmplxSubtractFromReal(&s1, &s2, &const_1, &u1, &u2);// 1 - 1 / ln(a,b)
		cmplxMultiply(&u1, &u2, &s1, &s2, &t1, &t2);

		dn_subtract(&s1, a, &const_4);
		cmplxLn(&t1, &t2, &s1, b);				// ln(a-4,b)

		cmplxSubtract(rx, ry, &t1, &t2, &u1, &u2);
	}

	for (i=0; i<20; i++) {
		// Now iterate to refine the estimate
		dn_add(&u1, rx, &const_1);			// u = wj + 1	(u1,ry)
		cmplxExp(&t1, &t2, rx, ry);			// t = e^wj
		cmplxMultiply(&s1, &s2, &u1, ry, &t1, &t2);	// (s1,s2) = (wj+1)e^wj

		dn_add(&v1, &u1, &const_1);			// v = wj + 2	(v1,ry)
		dn_add(&w1, &u1, &u1);			// w = 2wj + 2
		dn_add(&w2, ry, ry);
		cmplxDivide(&u1, &u2, &v1, ry, &w1, &w2);		// (u1,u2) = (wj+2)/(2wj+2)
		cmplxMultiply(&w1, &w2, &t1, &t2, rx, ry);		// (w1,w2) = wj e^wj

		// Check for termination w, x, u & s are live here
		cmplxSubtract(&v1, &v2, a, b, &w1, &w2);		// (v1,v2) = x - wj e^wj
		cmplxDivide(&t1, &t2, &v1, &v2, &s1, &s2);
		cmplxR(&t2, &t1, &t2);
		dn_compare(&t1, &t2, &const_1e_32);
		if (decNumberIsNegative(&t1))
			break;

		// Continue the increment update
		cmplxMinus(&v1, &v2, &v1, &v2);
		cmplxSubtract(&v1, &v2, &w1, &w2, a, b);		// (v1,v2) = wj e^wj - x
		cmplxMultiply(&t1, &t2, &v1, &v2, &u1, &u2);	// t = (wj+2).(wj e^wj - x) / (2wj + 2)
		cmplxSubtract(&w1, &w2, &s1, &s2, &t1, &t2);	// w = denominator
		cmplxDivide(&t1, &t2, &v1, &v2, &w1, &w2);
		cmplxSubtract(rx, ry, rx, ry, &t1, &t2);		// wj+1
	}
#endif
}

void cmplxInvW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber t1, t2;

	cmplxExp(&t1, &t2, a, b);
	cmplxMultiply(rx, ry, &t1, &t2, a, b);
#endif
}

// ln(a + i b) = ln(sqrt(a*a + b*b)) + i (2*arctan(signum(b)) - arctan(a/b))
// signum(b) = 1 if b>0, 0 if b=0, -1 if b<0, atan(1) = pi/4
void cmplxLn(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber u;

	if (decNumberIsZero(b)) {
		if (decNumberIsZero(a)) {
			cmplx_NaN(rx, ry);
		} else if (decNumberIsNegative(a)) {
			dn_minus(&u, a);
			dn_ln(rx, &u);
			decNumberPI(ry);
		} else {
			decNumberZero(ry);
			dn_ln(rx, a);
		}
		return;
	}
	if (decNumberIsSpecial(a)  || decNumberIsSpecial(b)) {
		if (decNumberIsNaN(a) || decNumberIsNaN(b)) {
			cmplx_NaN(rx, ry);
		} else {
			if (decNumberIsNegative(b))
				set_neginf(ry);
			else
				set_inf(ry);
			set_inf(rx);
		}
	} else {
		cmplxToPolar(&u, ry, a, b);
		dn_ln(rx, &u);
	}
#endif
}

// e ^ ( a + i b ) = e^a cos(b) + i e^a sin(b)
void cmplxExp(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber e, s, c;

	if (decNumberIsZero(b)) {
		dn_exp(rx, a);
		decNumberZero(ry);
	} else if (decNumberIsSpecial(a) || decNumberIsSpecial(b)) {
		cmplx_NaN(rx, ry);
	} else {
		dn_exp(&e, a);
		dn_sincos(b, &s, &c);
		dn_multiply(rx, &e, &c);
		dn_multiply(ry, &e, &s);
	}
#endif
}

#ifndef TINY_BUILD
static int cmplx_perm_helper(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
	decNumber n, m, s1, s2;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b) || decNumberIsSpecial(c) || decNumberIsSpecial(d)) {
		cmplx_NaN(rx, ry);
		return 0;
	}
		
	dn_add(&n, a, &const_1);		// x+1
	cmplxLnGamma(&s1, &s2, &n, b);		// lnGamma(x+1) = Ln x!

	cmplxSubtract(rx, ry, &n, b, c, d);	// x-y+1
	cmplxLnGamma(&n, &m, rx, ry);		// LnGamma(x-y+1) = Ln (x-y)!
	cmplxSubtract(rx, ry, &s1, &s2, &n, &m);
	return 1;
}
#endif

/* Calculate permutations:
 * C(x, y) = P(x, y) / y! = x! / ( (x-y)! y! )
 */
void cmplxComb(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber r1, r2, n, m, s1, s2;
	const int code = cmplx_perm_helper(&r1, &r2, a, b, c, d);

	if (code) {
		dn_add(&n, c, &const_1);	// y+1
		cmplxLnGamma(&s1, &s2, &n, d);	// LnGamma(y+1) = Ln y!
		cmplxSubtract(&n, &m, &r1, &r2, &s1, &s2);

		cmplxExp(rx, ry, &n, &m);
	} else
		cmplxCopy(rx, ry, &r1, &r2);
#endif
}

/* Calculate permutations:
 * P(x, y) = x! / (x-y)!
 */
void cmplxPerm(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber t1, t2;
	const int code = cmplx_perm_helper(&t1, &t2, a, b, c, d);

	if (code)
		cmplxExp(rx, ry, &t1, &t2);
	else
		cmplxCopy(rx, ry, &t1, &t2);
#endif
}


#ifndef TINY_BUILD
static void c_lg(decNumber *rx, decNumber *ry, const decNumber *x, const decNumber *y) {
	decNumber s1, s2, t1, t2, u1, u2, v1, v2;
	int k;

	decNumberZero(&s1);
	decNumberZero(&s2);
	dn_add(&t1, x, &const_21);		// ( t1, y )
	for (k=20; k>=0; k--) {
		cmplxDivideRealBy(&u1, &u2, gamma_consts[k], &t1, y);
		dn_dec(&t1);
		cmplxAdd(&s1, &s2, &s1, &s2, &u1, &u2);
	}
	dn_add(&t1, &s1, &const_gammaC00);	// (t1, s2)
	cmplxMultiplyReal(&u1, &u2, &t1, &s2, &const_2rootEonPI);
	cmplxLn(&s1, &s2, &u1, &u2);

	dn_add(&t1, x, &const_gammaR);	// (t1, y)
	cmplxDivideReal(&u1, &u2, &t1, y, &const_e);
	cmplxLn(&t1, &t2, &u1, &u2);
	dn_add(&u1, x, &const_0_5);		// (u1, y)
	cmplxMultiply(&v1, &v2, &u1, y, &t1, &t2);
	cmplxAdd(rx, ry, &v1, &v2, &s1, &s2);
}
#endif


void cmplxGamma(decNumber *rx, decNumber *ry, const decNumber *xin, const decNumber *y) {
#ifndef TINY_BUILD
	decNumber x, s1, s2, t1, t2, u1, u2;
	int reflec = 0;

	// Check for special cases
	if (decNumberIsSpecial(xin) || decNumberIsSpecial(y)) {
		if (decNumberIsNaN(xin) || decNumberIsNaN(y))
			cmplx_NaN(rx, ry);
		else {
			if (decNumberIsInfinite(xin)) {
				if (decNumberIsInfinite(y))
					cmplx_NaN(rx, ry);
				else if (decNumberIsNegative(xin))
					cmplx_NaN(rx, ry);
				else {
					set_inf(rx);
					decNumberZero(ry);
				}
			} else {
				decNumberZero(rx);
				decNumberZero(ry);
			}
		}
		return;
	}

	// Correct out argument and begin the inversion if it is negative
	if (decNumberIsNegative(xin)) {
		reflec = 1;
		dn_subtract(&t1, &const_1, xin);
		if (decNumberIsZero(y) && is_int(&t1)) {
			cmplx_NaN(rx, ry);
			return;
		}
		dn_subtract(&x, &t1, &const_1);
	} else
		dn_subtract(&x, xin, &const_1);

	// Sum the series
	c_lg(&s1, &s2, &x, y);
	cmplxExp(rx, ry, &s1, &s2);

	// Finally invert if we started with a negative argument
	if (reflec) {
		cmplxMultiplyReal(&t1, &t2, xin, y, &const_PI);
		cmplxSin(&s1, &s2, &t1, &t2);
		cmplxMultiply(&u1, &u2, &s1, &s2, rx, ry);
		cmplxDivideRealBy(rx, ry, &const_PI, &u1, &u2);
	}
#endif
}

void cmplxLnGamma(decNumber *rx, decNumber *ry, const decNumber *xin, const decNumber *y) {
#ifndef TINY_BUILD
	decNumber x, s1, s2, t1, t2, u1, u2;
	int reflec = 0;

	// Check for special cases
	if (decNumberIsSpecial(xin) || decNumberIsSpecial(y)) {
		if (decNumberIsNaN(xin) || decNumberIsNaN(y))
			cmplx_NaN(rx, ry);
		else {
			if (decNumberIsInfinite(xin)) {
				if (decNumberIsInfinite(y))
					cmplx_NaN(rx, ry);
				else if (decNumberIsNegative(xin))
					cmplx_NaN(rx, ry);
				else {
					set_inf(rx);
					decNumberZero(ry);
				}
			} else {
				decNumberZero(rx);
				decNumberZero(ry);
			}
		}
		return;
	}


	// Correct out argument and begin the inversion if it is negative
	if (decNumberIsNegative(xin)) {
		reflec = 1;
		dn_subtract(&t1, &const_1, xin);
		if (decNumberIsZero(y) && is_int(&t1)) {
			cmplx_NaN(rx, ry);
			return;
		}
		dn_subtract(&x, &t1, &const_1);
	} else
		dn_subtract(&x, xin, &const_1);

	c_lg(rx, ry, &x, y);

	// Finally invert if we started with a negative argument
	if (reflec) {
		cmplxMultiplyReal(&t1, &t2, xin, y, &const_PI);
		cmplxSin(&s1, &s2, &t1, &t2);
		cmplxDivideRealBy(&u1, &u2, &const_PI, &s1, &s2);
		cmplxLn(&t1, &t2, &u1, &u2);
		cmplxSubtract(rx, ry, &t1, &t2, rx, ry);
	}
#endif
}

void cmplxFactorial(decNumber *rx, decNumber *ry, const decNumber *xin, const decNumber *y) {
#ifndef TINY_BUILD
	decNumber x;

	dn_add(&x, xin, &const_1);
	cmplxGamma(rx, ry, &x, y);
#endif
}

#ifdef INCLUDE_DBLFACT
void cmplxDblFactorial(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber t, u1, u2, v1, v2, w1, w2;

	dn_add(&t, a, &const_1);
	cmplx2x(&u1, &u2, &t, b);
	cmplxDivideReal(&v1, &v2, &u1, &u2, &const_PI);
	cmplxSqrt(&u1, &u2, &v1, &v2);
	cmplxMultiplyReal(&v1, &v2, a, b, &const_0_5);
	dn_add(&t, &v1, &const_1);
	cmplxGamma(&w1, &w2, &t, &v2);
	cmplxMultiply(rx, ry, &w1, &w2, &u1, &u2);
#endif
}
#endif

// Beta(a, b) = exp(lngamma(a) + lngamma(b) - lngamma(a+b))
void cmplxLnBeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2, u1, u2;

	cmplxLnGamma(&s1, &s2, a, b);
	busy();
	cmplxLnGamma(&t1, &t2, c, d);
	busy();
	cmplxAdd(&u1, &u2, &s1, &s2, &t1, &t2);
	cmplxAdd(&s1, &s2, a, b, c, d);
	cmplxLnGamma(&t1, &t2, &s1, &s2);
	cmplxSubtract(rx, ry, &u1, &u2, &t1, &t2);
#endif
}

// Beta(a, b) = exp(lngamma(a) + lngamma(b) - lngamma(a+b))
void cmplxBeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber s1, s2;

	cmplxLnBeta(&s1, &s2, a, b, c, d);
	cmplxExp(rx, ry, &s1, &s2);
#endif
}

#ifdef INCLUDE_DIGAMMA
// Digamma function
extern void cmplxPsi(decNumber *rx, decNumber *ry, const decNumber *ain, const decNumber *bin) {
#ifndef TINY_BUILD
	decNumber a, b, t1, t2, r1, r2, x_2x, x_2y;
	int i;

	if (decNumberIsSpecial(ain) || decNumberIsSpecial(bin)) {
		cmplx_NaN(rx, ry);
		return;
	}

	// Reflection if negative real part
	if (dn_le0(ain)) {
		if (decNumberIsZero(bin) && is_int(ain)) {
			cmplx_NaN(rx, ry);
			return;
		}
		cmplxMultiplyReal(&r1, &r2, ain, bin, &const_PI);
		cmplxTan(&t1, &t2, &r1, &r2);
		cmplxDivideRealBy(&r1, &r2, &const_PI, &t1, &t2);
		cmplxSubtractFromReal(&a, &b, &const_1, ain, bin);
		cmplxMinus(rx, ry, &r1, &r2);
	} else {
		decNumberZero(rx);
		decNumberZero(ry);
		decNumberCopy(&a, ain);
		decNumberCopy(&b, bin);
	}

	// Recurrance to push real part > 8
	for (;;) {
		dn_compare(&t1, &const_8, &a);
		if (decNumberIsNegative(&t1))
			break;
		cmplxRecip(&t1, &t2, &a, &b);
		cmplxSubtract(rx, ry, rx, ry, &t1, &t2);
		dn_inc(&a);
	}

	// Series approximation
	cmplxLn(&t1, &t2, &a, &b);
	cmplxAdd(rx, ry, rx, ry, &t1, &t2);
	cmplxMultiplyReal(&r1, &r2, &a, &b, &const__2);
	cmplxRecip(&t1, &t2, &r1, &r2);
	cmplxAdd(rx, ry, rx, ry, &t1, &t2);

	cmplxSqr(&t1, &t2, &a, &b);
	cmplxRecip(&x_2x, &x_2y, &t1, &t2);
	cmplxCopy(&r1, &r2, &x_2x, &x_2y);
	for (i=0; i<10; i++) {
		cmplxMultiplyReal(&t1, &t2, &r1, &r2, digamma_consts[i]);
		cmplxAdd(rx, ry, rx, ry, &t1, &t2);
		cmplxMultiply(&r1, &r2, &r1, &r2, &x_2x, &x_2y);
	}
#endif
}
#endif 


#ifdef INCLUDE_COMPLEX_ZETA
/* Riemann's Zeta function */
#ifndef TINY_BUILD
static void c_zeta_step(decNumber *sx, decNumber *sy,
		const decNumber *x, const decNumber *y,
		const decNumber *dc, decNumber *k) {
	decNumber t1, t2, s1, s2;

	busy();
	dn_inc(k);
	cmplxRealPower(&s1, &s2, k, x, y);
	cmplxDivideRealBy(&t1, &t2, dc, &s1, &s2);
	cmplxAdd(sx, sy, sx, sy, &t1, &t2);
}
#endif

void cmplxZeta(decNumber *rx, decNumber *ry,
		const decNumber *xin, const decNumber *yin) {
#ifndef TINY_BUILD
	decNumber s1, s2, x, y, u1, u2, reflecfac1, reflecfac2, sum1, sum2, t1, t2;
	int reflec, i;

	if (decNumberIsSpecial(xin) || decNumberIsSpecial(yin)) {
		if (decNumberIsNaN(xin) || decNumberIsNegative(xin) || decNumberIsNaN(yin)) {
			cmplx_NaN(rx, ry);
		} else {
			decNumberCopy(rx, &const_1);
			decNumberZero(ry);
		}
		return;
	}

	/* Short cut the real case by using the real version of the function */
#if 0
	if (decNumberIsZero(yin)) {
		decNumberZeta(rx, xin);
		decNumberZero(ry);
		return;
	}
#endif

	dn_compare(&s1, xin, &const_0_5);
	if (decNumberIsNegative(&s1)) {
		/* use reflection formula
		 * zeta(x) = 2^x*Pi^(x-1)*sin(Pi*x/2)*gamma(1-x)*zeta(1-x)
		 */
		reflec = 1;
		cmplxSubtractFromReal(&x, &y, &const_1, xin, yin);
		cmplxMultiplyReal(&u1, &u2, xin, yin, &const_PIon2);
		cmplxSin(&s1, &s2, &u1, &u2);
		cmplxRealPower(rx, ry, &const_2, xin, yin);
		cmplxMultiply(&u1, &u2, rx, ry, &s1, &s2);
		cmplxRealPower(rx, ry, &const_PI, &x, &y);
		cmplxDivide(&s1, &s2, &u1, &u2, rx, ry);
		cmplxGamma(rx, ry, &x, &y);
		cmplxMultiply(&reflecfac1, &reflecfac2, &s1, &s2, rx, ry);
	} else {
		reflec = 0;
		cmplxCopy(&x, &y, xin, yin);
	}

	/* Now calculate zeta(x) where x >= 0.5 */
	decNumberZero(&sum1);
	decNumberZero(&sum2);
	decNumberZero(&t1);
	for (i=0; i<30; i++)
		c_zeta_step(&sum1, &sum2, &x, &y, zeta_consts[i], &t1);

	cmplxSubtractFromReal(&t1, &t2, &const_1, &x, &y);
	cmplxRealPower(&u1, &u2, &const_2, &t1, &t2);
	dn_dec(&u1);
	cmplxMultiplyReal(&t1, &t2, &u1, &u2, &const_zeta_dn);
	cmplxDivide(rx, ry, &sum1, &sum2, &t1, &t2);

	/* Finally, undo the reflection if required */
	if (reflec)
		cmplxMultiply(rx, ry, &reflecfac1, &reflecfac2, rx, ry);
#endif
}
#endif /* INCLUDE_COMPLEX_ZETA */


void cmplxParallel(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber p1, p2, s1, s2;

	cmplxMultiply(&p1, &p2, a, b, c, d);
	cmplxAdd(&s1, &s2, a, b, c, d);
	cmplxDivide(rx, ry, &p1, &p2, &s1, &s2);
#endif
}

void cmplxAGM(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
#ifndef TINY_BUILD
	decNumber x1, x2, y1, y2, t1, t2, u1, u2;
	int n;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b) ||
			decNumberIsSpecial(c) || decNumberIsSpecial(d)) {
			goto nan;
	}
	cmplxCopy(&x1, &x2, a, b);
	cmplxCopy(&y1, &y2, c, d);
	for (n=0; n<1000; n++) {
		cmplxSubtract(&t1, &t2, &x1, &x2, &y1, &y2);
		cmplxR(&u1, &t1, &t2);
		dn_compare(&u2, &u1, &const_1e_32);
		if (decNumberIsNegative(&u2)) {
			cmplxCopy(rx, ry, &x1, &x2);
			return;
		}

		cmplxAdd(&t1, &t2, &x1, &x2, &y1, &y2);
		cmplxMultiplyReal(&u1, &u2, &t1, &t2, &const_0_5);

		cmplxMultiply(&t1, &t2, &x1, &x2, &y1, &y2);
		cmplxSqrt(&y1, &y2, &t1, &t2);

		cmplxCopy(&x1, &x2, &u1, &u2);
	}
nan:	cmplx_NaN(rx, ry);
#endif
}

void cmplxRnd(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumberRnd(rx, a);
	decNumberRnd(ry, b);
}


void cmplxFrac(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumberFrac(rx, a);
	decNumberFrac(ry, b);
}

void cmplxTrunc(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumberTrunc(rx, a);
	decNumberTrunc(ry, b);
}

void cmplxFib(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
#ifndef TINY_BUILD
	decNumber r1, r2, s1, s2, t1, t2;

	cmplxRealPower(&r1, &r2, &const_phi, a, b);
	cmplxMultiplyReal(&t1, &t2, a, b, &const_PI);
	cmplxCos(&s1, &s2, &t1, &t2);
	cmplxDivide(&t1, &t2, &s1, &s2, &r1, &r2);
	cmplxSubtract(&s1, &s2, &r1, &r2, &t1, &t2);
	cmplxMultiplyReal(rx, ry, &s1, &s2, &const_recipsqrt5);
#endif
}

#ifdef INCLUDE_ELLIPTIC
#ifndef TINY_BUILD
static void elliptic_setup(decNumber *r,
		decNumber *snuk, decNumber *cnuk, decNumber *dnuk,
		decNumber *snvki, decNumber *cnvki, decNumber *dnvki,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki) {
	decNumber a;

	dn_elliptic(snuk, cnuk, dnuk, u, k);
	dn_elliptic(snvki, cnvki, dnvki, v, ki);

	dn_multiply(&a, dnuk, snvki);
	decNumberSquare(r, &a);
	dn_subtract(&a, &const_1, r);
	decNumberRecip(r, &a);
}
#endif


// SN(u + i v, k + i ki) = sn(u, k) . dn(v, ki) / denom
//		  + i . cn(u, k) . dn(u, k) . sn(v, ki) . cn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxSN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki) {
#ifndef TINY_BUILD
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki);

	dn_multiply(&a, &snuk, &dnvki);
	dn_multiply(rx, &a, &denom);

	dn_multiply(&a, &cnuk, &dnuk);
	dn_multiply(&b, &a, &snvki);
	dn_multiply(&a, &b, &cnvki);
	dn_multiply(ry, &a, &denom);
#endif
}


// CN(u + i v, k + i ki) = cn(u, k) . cn(v, ki) / denom
//		  - i . sn(u, k) . dn(u, k) . sn(v, ki) . dn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxCN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki) {
#ifndef TINY_BUILD
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki);

	dn_multiply(&a, &cnuk, &cnvki);
	dn_multiply(rx, &a, &denom);

	dn_multiply(&a, &snuk, &dnuk);
	dn_multiply(&b, &a, &snvki);
	dn_multiply(&a, &b, &dnvki);
	dn_multiply(&b, &a, &denom);
	dn_minus(ry, &b);
#endif
}


// DN(a + i b, c + i d) = dn(u, k) . cn(v, ki) . dn(v, ki) / denom
//		  - i . k^2 . sn(u, k) . cn(u, k) . sn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxDN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki) {
#ifndef TINY_BUILD
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki);

	dn_multiply(&a, &dnuk, &cnvki);
	dn_multiply(&b, &a, &dnvki);
	dn_multiply(rx, &b, &denom);

	decNumberSquare(&a, k);
	dn_minus(&b, &a);
	dn_multiply(&a, &b, &snuk);
	dn_multiply(&b, &a, &cnuk);
	dn_multiply(&a, &b, &snvki);
	dn_multiply(ry, &a, &denom);
#endif
}
#endif

#ifdef COMPLEX_BESSEL
#ifndef TINY_BUILD
static void cmplx_bessel(decNumber *rx, decNumber *ry,
			const decNumber *nx, const decNumber *ny,
			const decNumber *xx, const decNumber *xy, const int neg) {
	decNumber x2on4x, x2on4y, k, ux, uy;
	decNumber t1, t2, a1, a2, b;
	int i;

	cmplxMultiplyReal(&t1, &t2, xx, xy, &const_0_5);
	cmplxSqr(&x2on4x, &x2on4y, &t1, &t2);

	cmplxPower(&t1, &t2, &t1, &t2, nx, ny);
	dn_add(&b, nx, &const_1);
	cmplxGamma(&a1, &a2, &b, ny);
	cmplxDivide(&ux, &uy, &t1, &t2, &a1, &a2);

	decNumberCopy(&k, &const_1);
	cmplxCopy(rx, ry, &ux, &uy);

	for (i=0;i<1000;i++) {
		cmplxMultiply(&t1, &t2, &x2on4x, &x2on4y, &ux, &uy);
		if (neg)
			cmplxMinus(&t1, &t2, &t1, &t2);
		dn_add(&b, nx, &k);
		cmplxMultiplyReal(&a1, &a2, &b, ny, &k);
		cmplxDivide(&ux, &uy, &t1, &t2, &a1, &a2);

		cmplxAdd(&a1, &a2, &ux, &uy, rx, ry);
		cmplxSubtract(&t1, &t2, &a1, &a2, rx, ry);
		cmplxR(&b, &t1, &t2);
		if (decNumberIsZero(&b))
			return;
		cmplxCopy(rx, ry, &a1, &a2);
		dn_inc(&k);
	}
	cmplx_NaN(rx, ry);
}
#endif

void cmplxBSJN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy) {
#ifndef TINY_BUILD
	if (decNumberIsZero(xy) && decNumberIsZero(alphay)) {
		decNumberBSJN(rx, alphax, xx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else if (decNumberIsSpecial(alphax) || decNumberIsSpecial(alphay) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else
		cmplx_bessel(rx, ry, alphax, alphay, xx, xy, 1);
#endif
}

void cmplxBSIN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy) {
#ifndef TINY_BUILD
	if (decNumberIsZero(xy) && decNumberIsZero(alphay)) {
		decNumberBSIN(rx, alphax, xx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else if (decNumberIsSpecial(alphax) || decNumberIsSpecial(alphay) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else
		cmplx_bessel(rx, ry, alphax, alphay, xx, xy, 0);
#endif
}


// See A&S page 360 section 9.1.11
#ifndef TINY_BUILD
static void cmplx_bessel2_int_series(decNumber *rx, decNumber *ry, const decNumber *n, const decNumber *x, const decNumber *y, int modified) {
	const decNumber *const factor = modified?&const_0_5:&const__1onPI;
	decNumber xon2x, xon2y, xon2nx, xon2ny, x2on4x, x2on4y;
	decNumber k, npk, tx, ty, ux, uy, vx, vy, sx, sy, px, py, nf, absn;
	int i, in, n_odd, n_neg;

	if (decNumberIsNegative(n)) {
		n = dn_abs(&absn, n);
		n_neg = 1;
	} else	n_neg = 0;
	in = dn_to_int(n);
	n_odd = in & 1;

	cmplxMultiplyReal(&xon2x, &xon2y, x, y, &const_0_5);	// xon2 = x/2
	cmplxPowerReal(&xon2nx, &xon2ny, &xon2x, &xon2y, n);	// xon2n = (x/2)^n
	cmplxSqr(&x2on4x, &x2on4y, &xon2x, &xon2y);		// x2on4 = +/- x^2/4

	if (modified)
		cmplxMinus(&x2on4x, &x2on4y, &x2on4x, &x2on4y);
	if (in > 0) {
		dn_subtract(&vx, n, &const_1);	// v = n-k-1 = n-1
		decNumberZero(&k);
		decNumberGamma(&px, n);		// p = (n-1)!
		decNumberZero(&py);
		decNumberZero(&sy);
		decNumberCopy(&sx, &px);
		dn_multiply(&nf, &px, n);	// nf = n!  (for later)
		for (i=1; i<in; i++) {
			cmplxDivideReal(&tx, &ty, &px, &py, &vx);
			dn_dec(&vx);
			dn_inc(&k);
			cmplxMultiplyReal(&ux, &uy, &tx, &ty, &k);
			cmplxMultiply(&px, &py, &ux, &uy, &x2on4x, &x2on4y);
			cmplxAdd(&sx, &sy, &sx, &sy, &px, &py);
		}
		cmplxMultiplyReal(&tx, &ty, &sx, &sy, factor);
		cmplxDivide(rx, ry, &tx, &ty, &xon2nx, &xon2ny);
	} else {
		decNumberZero(rx);
		decNumberZero(ry);
		decNumberCopy(&nf, &const_1);
	}

	if (modified) {
		cmplxBSIN(&tx, &ty, n, &const_0, x, y);
		if (!n_odd)
			cmplxMinus(&tx, &ty, &tx, &ty);
	} else {
		cmplxBSJN(&ux, &uy, n, &const_0, x, y);
		cmplxDivideReal(&tx, &ty, &ux, &uy, &const_PIon2);
	}
	cmplxLn(&ux, &uy, &xon2x, &xon2y);
	cmplxMultiply(&vx, &vy, &ux, &uy, &tx, &ty);
	cmplxAdd(rx, ry, rx, ry, &vx, &vy);

	cmplxMinus(&x2on4x, &x2on4y, &x2on4x, &x2on4y);
	dn_add(&tx, n, &const_1);			// t = n+1
	decNumberPsi(&ux, &tx);				// u = Psi(n+1)
	dn_subtract(&vx, &ux, &const_egamma);	// v = psi(k+1) + psi(n+k+1)
	decNumberZero(&k);
	decNumberZero(&py);
	decNumberZero(&sy);
	decNumberCopy(&npk, n);
	decNumberRecip(&px, &nf);			// p = (x^2/4)^k/(k!(n+k)!)
	dn_multiply(&sx, &vx, &px);

	for (i=0;i<1000;i++) {
		dn_inc(&k);
		dn_inc(&npk);
		cmplxMultiply(&tx, &ty, &px, &py, &x2on4x, &x2on4y);
		dn_multiply(&ux, &k, &npk);
		cmplxDivideReal(&px, &py, &tx, &ty, &ux);

		decNumberRecip(&tx, &k);
		dn_add(&ux, &vx, &tx);
		decNumberRecip(&tx, &npk);
		dn_add(&vx, &ux, &tx);

		cmplxMultiplyReal(&tx, &ty, &px, &py, &vx);
		cmplxAdd(&ux, &uy, &tx, &ty, &sx, &sy);
		cmplxSubtract(&tx, &ty, &ux, &uy, &sx, &sy);
		cmplxR(&vy, &tx, &ty);
		if (decNumberIsZero(&vy))
			break;
		cmplxCopy(&sx, &sy, &ux, &uy);
	}
	cmplxMultiply(&tx, &ty, &sx, &sy, &xon2nx, &xon2ny);
	if (modified) {
		if (n_odd)
			cmplxMultiplyReal(&ux, &uy, &tx, &ty, &const__0_5);
		else
			cmplxMultiplyReal(&ux, &uy, &tx, &ty, &const_0_5);
	} else
		cmplxMultiplyReal(&ux, &uy, &tx, &ty, &const__1onPI);
	cmplxAdd(rx, ry, rx, ry, &ux, &uy);
	if (!modified && n_neg)
		cmplxMinus(rx, ry, rx, ry);
}
#endif


// Yv(z) = ( Jv(z).cos(v PI) - J-v(z) ) / sin(v PI)
//Y(3.5+4.5i)(1.5+2.5i) = -1.19957042014349569 + 2.34640749171622195334 i
void cmplxBSYN(decNumber *rx, decNumber *ry,
		const decNumber *vx, const decNumber *vy,
		const decNumber *xx, const decNumber *xy) {
#ifndef TINY_BUILD
	decNumber t1, t2, sx, sy, cx, cy, jnx, jny, jmx, jmy;
#if 0
	if (decNumberIsZero(xy) && decNumberIsZero(vy)) {
		decNumberBSYN(rx, vx, xx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else
#endif
	if (decNumberIsSpecial(vx) || decNumberIsSpecial(vy) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else if (decNumberIsZero(vy) && is_int(vx)) {
		cmplx_bessel2_int_series(rx, ry, vx, xx, xy, 0);
	} else {
		cmplx_bessel(&jnx, &jny, vx, vy, xx, xy, 1);
		cmplxMinus(&t1, &t2, vx, vy);
		cmplx_bessel(&jmx, &jmy, &t1, &t2, xx, xy, 1);
		cmplxMultiplyReal(&t1, &t2, vx, vy, &const_PI);
		cmplx_sincos(&t1, &t2, &sx, &sy, &cx, &cy);
		cmplxMultiply(&t1, &t2, &jnx, &jny, &cx, &cy);
		cmplxSubtract(&jnx, &jny, &t1, &t2, &jmx, &jmy);
		cmplxDivide(rx, ry, &jnx, &jny, &sx, &sy);
	}
#endif
}

// Kn(x) = PI/2 . (I-n(x) - In(x)) / sin(n PI)
//K(3.5+4.5i)(1.5+2.5i) = 2.41061613980770099 + 1.0563528861789105228i
void cmplxBSKN(decNumber *rx, decNumber *ry,
		const decNumber *vx, const decNumber *vy,
		const decNumber *xx, const decNumber *xy) {
#ifndef TINY_BUILD
	decNumber t1, t2, inx, iny, imx, imy;

#if 0
	if (decNumberIsZero(xy) && decNumberIsZero(vy)) {
		decNumberBSKN(rx, vx, xx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else
#endif
	if (decNumberIsSpecial(vx) || decNumberIsSpecial(vy) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else if (decNumberIsZero(vy) && is_int(vx)) {
		cmplx_bessel2_int_series(rx, ry, vx, xx, xy, 1);
	} else {
		cmplx_bessel(&inx, &iny, vx, vy, xx, xy, 0);
		cmplxMinus(&t1, &t2, xx, xy);
		cmplx_bessel(&imx, &imy, vx, vy, &t1, &t2, 0);
		cmplxSubtract(&t1, &t2, &imx, &imy, &inx, &iny);
		cmplxMultiplyReal(&imx, &imy, &t1, &t2, &const_PIon2);
		cmplxMultiplyReal(&inx, &iny, vx, vy, &const_PI);
		cmplxSin(&t1, &t2, &inx, &iny);
		cmplxDivide(rx, ry, &imx, &imy, &t1, &t2);
	}
#endif
}

#endif

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
		const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumberSubtract(rx, r, a, ctx);
	decNumberMinus(ry, b, ctx);
}

// (a + i b) * r = (a * r) + i (b * r)
static void cmplxMultiplyReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r, decContext *ctx) {
	decNumberMultiply(rx, a, r, ctx);
	decNumberMultiply(ry, b, r, ctx);
}

// (a + i b) / c = ( a / r ) + i ( b / r)
static void cmplxDivideReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r, decContext *ctx) {
	decNumberDivide(rx, a, r, ctx);
	decNumberDivide(ry, b, r, ctx);
}

// a / (c + i d) = (a*c) / (c*c + d*d) + i (- a*d) / (c*c + d*d)
static void cmplxDivideRealBy(decNumber *rx, decNumber *ry,
		const decNumber *a,
		const decNumber *c, const decNumber *d, decContext *ctx) {
	decNumber t1, t2, t3, den;

	decNumberMultiply(&t1, c, c, ctx);
	decNumberMultiply(&t2, d, d, ctx);
	decNumberAdd(&den, &t1, &t2, ctx);

	decNumberMultiply(&t1, a, c, ctx);
	decNumberDivide(rx, &t1, &den, ctx);

	decNumberMultiply(&t2, a, d, ctx);
	decNumberMinus(&t3, &t2, ctx);
	decNumberDivide(ry, &t3, &den, ctx);
}
#endif

// (a + i b) + (c + i d) = (a + c) + i (b + d)
void cmplxAdd(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx) {
	decNumberAdd(rx, a, c, ctx);
	decNumberAdd(ry, b, d, ctx);
}

// (a + i b) - (c + i d) = (a - c) + i (b - d)
void cmplxSubtract(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx) {
	decNumberSubtract(rx, a, c, ctx);
	decNumberSubtract(ry, b, d, ctx);
}

// (a + i b) * (c + i d) = (a * c - b * d) + i (a * d + b * c)
void cmplxMultiply(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx) {
	decNumber t1, t2, u1, u2;

	decNumberMultiply(&t1, a, c, ctx);
	decNumberMultiply(&t2, b, d, ctx);
	decNumberMultiply(&u1, a, d, ctx);
	decNumberMultiply(&u2, b, c, ctx);

	decNumberSubtract(rx, &t1, &t2, ctx);
	decNumberAdd(ry, &u1, &u2, ctx);
}

// (a + i b) / (c + i d) = (a*c + b*d) / (c*c + d*d) + i (b*c - a*d) / (c*c + d*d)
void cmplxDivide(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx) {
	decNumber t1, t2, t3, t4, den;

	decNumberMultiply(&t1, c, c, ctx);
	decNumberMultiply(&t2, d, d, ctx);
	decNumberAdd(&den, &t1, &t2, ctx);

	decNumberMultiply(&t3, a, c, ctx);
	decNumberMultiply(&t2, b, d, ctx);
	decNumberAdd(&t1, &t3, &t2, ctx);

	decNumberMultiply(&t4, b, c, ctx);
	decNumberMultiply(&t2, a, d, ctx);
	decNumberSubtract(&t3, &t4, &t2, ctx);

	decNumberDivide(rx, &t1, &den, ctx);
	decNumberDivide(ry, &t3, &den, ctx);
}

void cmplxArg(decNumber *arg, const decNumber *a, const decNumber *b, decContext *ctx) {
	do_atan2(arg, b, a, ctx);
}

void cmplxR(decNumber *r, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumber a2, b2, s;

	decNumberMultiply(&a2, a, a, ctx);
	decNumberMultiply(&b2, b, b, ctx);
	decNumberAdd(&s, &a2, &b2, ctx);
	decNumberSquareRoot(r, &s, ctx);
}

void cmplxFromPolar(decNumber *x, decNumber *y, const decNumber *r, const decNumber *t, decContext *ctx) {
	decNumber s, c;

	dn_sincos(t, &s, &c, ctx);
	decNumberMultiply(x, r, &c, ctx);
	decNumberMultiply(y, r, &s, ctx);
}

void cmplxToPolar(decNumber *r, decNumber *t, const decNumber *x, const decNumber *y, decContext *ctx) {
	do_atan2(t, y, x, ctx);
	cmplxR(r, y, x, ctx);
}



#ifndef TINY_BUILD
// ( a + i * b ) ^ r
static void cmplxPowerReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r, decContext *ctx) {
	cmplxPower(rx, ry, a, b, r, &const_0, ctx);
}
#endif

static void cmplxRealPower(decNumber *rx, decNumber *ry,
		const decNumber *r,
		const decNumber *a, const decNumber *b,
		decContext *ctx) {
#ifndef TINY_BUILD
	cmplxPower(rx, ry, r, &const_0, a, b, ctx);
#endif
}

// a ^ b = e ^ (b ln(a))
void cmplxPower(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber e1, e2, f1, f2;

	cmplxLn(&e1, &e2, a, b, ctx);
	cmplxMultiply(&f1, &f2, &e1, &e2, c, d, ctx);
	cmplxExp(rx, ry, &f1, &f2, ctx);
#endif
}


// abs(a + i b) = sqrt(a^2 + b^2)
void cmplxAbs(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplxR(rx, a, b, ctx);
	decNumberZero(ry);
}

// sign(a + i b) = (a + i b) / |a + i b|
void cmplxSign(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber z;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b)) {
		if (decNumberIsNaN(a) || decNumberIsNaN(b))
			cmplx_NaN(rx, ry);
		else if (decNumberIsInfinite(a)) {
			if (decNumberIsInfinite(b))
				cmplx_NaN(rx, ry);
			else {
				decNumberSign(rx, a, ctx);
				decNumberZero(ry);
			}
		} else {
			decNumberSign(ry, b, ctx);
			decNumberZero(rx);
		}
		return;
	}
	if (decNumberIsZero(b)) {
		decNumberSign(rx, a, ctx);
		decNumberZero(ry);
	} else if (decNumberIsZero(a)) {
		decNumberZero(rx);
		decNumberSign(ry, b, ctx);
	} else {
		cmplxR(&z, a, b, ctx);
		cmplxDivideReal(rx, ry, a, b, &z, ctx);
	}
#endif
}

// - (a + i b) = - a - i b
void cmplxMinus(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumberMinus(rx, a, ctx);
	decNumberMinus(ry, b, ctx);
}

// conj(a + i b) = a - i b
void cmplxConj(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumberCopy(rx, a);
	decNumberMinus(ry, b, ctx);
}

// 1 / (c + i d) = c / (c*c + d*d) + i (- d) / (c*c + d*d)
void cmplxRecip(decNumber *rx, decNumber *ry, const decNumber *c, const decNumber *d, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, u, v, den;

	decNumberMultiply(&u, c, c, ctx);
	decNumberMultiply(&v, d, d, ctx);
	decNumberAdd(&den, &u, &v, ctx);
	decNumberMinus(&t, d, ctx);

	decNumberDivide(rx, c, &den, ctx);
	decNumberDivide(ry, &t, &den, ctx);
#endif
}

// (a + ib)^2 = (a^2 - b^2) + i ( 2 * a * b )
void cmplxSqr(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, u, v;

	decNumberSquare(&t, a, ctx);
	decNumberSquare(&u, b, ctx);
	decNumberMultiply(&v, a, b, ctx);

	decNumberSubtract(rx, &t, &u, ctx);
	decNumberAdd(ry, &v, &v, ctx);
#endif
}

// sqrt(a + i b) = +- (sqrt(r + a) + i sqrt(r - a) sign(b)) sqrt(2) / 2
//		where r = sqrt(a^2 + b^2)
void cmplxSqrt(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber fac, t1, u, v;

	if (decNumberIsZero(b)) {
		// Detect a purely real input and shortcut the computation
		if (decNumberIsNegative(a)) {
			decNumberMinus(&t1, a, ctx);
			decNumberSquareRoot(ry, &t1, ctx);
			decNumberZero(rx);
		} else {
			decNumberSquareRoot(rx, a, ctx);
			decNumberZero(ry);
		}
		return;
	} else {
		cmplxR(&fac, a, b, ctx);

		decNumberSubtract(&v, &fac, a, ctx);
		decNumberSquareRoot(&u, &v, ctx);
		decNumberAdd(&v, &fac, a, ctx);
		if (decNumberIsNegative(b)) {
			decNumberMinus(&t1, &u, ctx);
			decNumberMultiply(ry, &t1, &const_root2on2, ctx);
		} else
			decNumberMultiply(ry, &u, &const_root2on2, ctx);

		decNumberSquareRoot(&t1, &v, ctx);
		decNumberMultiply(rx, &t1, &const_root2on2, ctx);
	}
#endif
}

// (a + ib)^3 = (a^2 + b^2) + i ( 2 * a * b ) . ( a + i b )
void cmplxCube(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber s1, s2;

	cmplxSqr(&s1, &s2, a, b, ctx);
	cmplxMultiply(rx, ry, &s1, &s2, a, b, ctx);
#endif
}

// Fairly naive implementation...
void cmplxCubeRoot(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t;

	decNumberRecip(&t, &const_3, ctx);
	cmplxPowerReal(rx, ry, a, b, &t, ctx);
#endif
}


// sin(a + i b) = sin(a) cosh(b) + i cos(a) sinh(b)
// cos(a + i b) = cos(a) cosh(b) - i sin(a) sinh(b)
static void cmplx_sincos(const decNumber *a, const decNumber *b, decNumber *sx, decNumber *sy, decNumber *cx, decNumber *cy, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber sa, ca, sb, cb;

	if (decNumberIsZero(a) && decNumberIsInfinite(b)) {
		if (sx != NULL)	decNumberZero(sx);
		if (sy != NULL)	decNumberCopy(sy, b);
		if (cx != NULL)	set_inf(cx);
		if (cy != NULL)	decNumberZero(cy);
	} else {
		dn_sincos(a, &sa, &ca, ctx);
		dn_sinhcosh(b, &sb, &cb, ctx);
		if (sx != NULL)	decNumberMultiply(sx, &sa, &cb, ctx);
		if (sy != NULL)	decNumberMultiply(sy, &ca, &sb, ctx);
		if (cx != NULL)	decNumberMultiply(cx, &ca, &cb, ctx);
		if (cy != NULL) {
			decNumberMultiply(&ca, &sa, &sb, ctx);
			decNumberMinus(cy, &ca, ctx);
		}
	}
#endif
}

// sin(a + i b) = sin(a) cosh(b) + i cos(a) sinh(b)
void cmplxSin(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplx_sincos(a, b, rx, ry, NULL, NULL, ctx);
}

// cos(a + i b) = cos(a) cosh(b) - i sin(a) sinh(b)
void cmplxCos(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplx_sincos(a, b, NULL, NULL, rx, ry, ctx);
}

// tan(a + i b) = (sin(a) cosh(b) + i cos(a) sinh(b)) / (cos(a) cosh(b) - i sin(a) sinh(b))
void cmplxTan(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
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
		dn_sincos(a, rx, ry, ctx);
		dn_sinhcosh(b, &sb, &cb, ctx);

		decNumberMultiply(&t1, rx, &cb, ctx);
		decNumberMultiply(&t2, ry, &sb, ctx);
		decNumberMultiply(&t3, ry, &cb, ctx);
		decNumberMultiply(&cb, rx, &sb, ctx);

		cmplxDivide(rx, ry, &t1, &t2, &t3, &cb, ctx);
	}
#endif
}

// Helper for arcsin and arccos.
// alpha = 1/2 sqrt((x+1)^2+y^2) + 1/2 sqrt((x-1)^2+y^2)
// beta  = 1/2 sqrt((x+1)^2+y^2) - 1/2 sqrt((x-1)^2+y^2)
// la = ln(alpha + sqrt(alpha^2-1)
#ifndef TINY_BUILD
static void asinacos_chelper(decNumber *la, decNumber *b, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber y2, x1, r, s, t;

	decNumberSquare(&y2, y, ctx);
	decNumberAdd(&x1, x, &const_1, ctx);
	decNumberSquare(&t, &x1, ctx);
	decNumberAdd(&r, &t, &y2, ctx);
	decNumberSquareRoot(&t, &r, ctx);
	decNumberMultiply(&s, &t, &const_0_5, ctx);

	decNumberSubtract(&x1, x, &const_1, ctx);
	decNumberSquare(&t, &x1, ctx);
	decNumberAdd(&r, &t, &y2, ctx);
	decNumberSquareRoot(&t, &r, ctx);
	decNumberMultiply(&r, &t, &const_0_5, ctx);

	decNumberAdd(&t, &s, &r, ctx);
	decNumberSubtract(b, &s, &r, ctx);

	// Now ln(alpha + sqrt(alpha*alpha-1))
	decNumberSquare(&r, &t, ctx);
	decNumberSubtract(&s, &r, &const_1, ctx);
	decNumberSquareRoot(&r, &s, ctx);
	decNumberAdd(&s, &r, &t, ctx);
	decNumberLn(la, &s, ctx);
}
#endif

// arcsin(z) = k PI + (-1)^k . asin(beta) + i (-1)^k ln(alpha+sqrt(alpha^2-1))
void cmplxAsin(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber beta;

	asinacos_chelper(ry, &beta, a, b, ctx);
	do_asin(rx, &beta, ctx);
#endif
}

// arccos(z) = 2k PI +- (acos(beta) - i ln(alpha+sqrt(alpha^2-1)))
void cmplxAcos(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber beta;

	asinacos_chelper(ry, &beta, a, b, ctx);
	decNumberMinus(ry, ry, ctx);
	do_acos(rx, &beta, ctx);
#endif
}

// atan(z) = k PI + 0.5 atan(2a / (1-a^2-b^2) + i/4 ln((a^2+(b+1)^2)/(a^2+(b-1)^2))
//		z^2 <>-1
void cmplxAtan(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2, u, v1, v2;

	decNumberAdd(&t1, b, &const_1, ctx);
	decNumberMinus(&t2, a, ctx);
	cmplxLn(&s1, &s2, &t1, &t2, ctx);

	decNumberSubtract(&u, &const_1, b, ctx);
	cmplxLn(&v1, &v2, &u, a, ctx);

	decNumberSubtract(&t1, &s1, &v1, ctx);
	decNumberSubtract(&t2, &s2, &v2, ctx);

	decNumberMultiply(&s1, &const_0_5, &t2, ctx);
	decNumberMinus(rx, &s1, ctx);
	decNumberMultiply(ry, &const_0_5, &t1, ctx);
#endif
}

// sinc(a + i b) = sin(a + i b) / (a + i b)
void cmplxSinc(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber s1, s2;

	if (decNumberIsZero(b)) {
		decNumberSinc(rx, a, ctx);
		decNumberZero(ry);
	} else {
		cmplxSin(&s1, &s2, a, b, ctx);
		cmplxDivide(rx, ry, &s1, &s2, a, b, ctx);
	}
#endif
}

// sinh(a + i b) = sinh(a) cos(b) + i cosh(a) sin(b)
// cosh(a + i b) = cosh(a) cos(b) + i sinh(a) sin(b)
static void cmplx_sinhcosh(const decNumber *a, const decNumber *b, decNumber *sx, decNumber *sy, decNumber *cx, decNumber *cy, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber sa, ca, sb, cb;

	if (decNumberIsZero(b)) {
		if (sx != NULL)	decNumberSinh(sx, a, ctx);
		if (sy != NULL)	decNumberZero(sy);
		if (cx != NULL)	decNumberCosh(cx, a, ctx);
		if (cy != NULL)	decNumberZero(cy);
	} else {
		dn_sinhcosh(a, &sa, &ca, ctx);
		dn_sincos(b, &sb, &cb, ctx);
		if (sx != NULL)	decNumberMultiply(sx, &sa, &cb, ctx);
		if (sy != NULL)	decNumberMultiply(sy, &ca, &sb, ctx);
		if (cx != NULL)	decNumberMultiply(cx, &ca, &cb, ctx);
		if (cy != NULL)	decNumberMultiply(cy, &sa, &sb, ctx);
	}
#endif
}

// sinh(a + i b) = sinh(a) cos(b) + i cosh(a) sin(b)
void cmplxSinh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplx_sinhcosh(a, b, rx, ry, NULL, NULL, ctx);
}

// cosh(a + i b) = cosh(a) cos(b) + i sinh(a) sin(b)
void cmplxCosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplx_sinhcosh(a, b, NULL, NULL, rx, ry, ctx);
}

// tanh(a + i b) = (tanh(a) + i tan(b))/(1 + i tanh(a) tan(b))
void cmplxTanh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber ta, tb, t2;

	if (decNumberIsZero(b)) {
		decNumberTanh(rx, a, ctx);
		decNumberZero(ry);
	} else {
		decNumberTanh(&ta, a, ctx);
		decNumberTan(&tb, b, ctx);

		decNumberMultiply(&t2, &ta, &tb, ctx);

		cmplxDivide(rx, ry, &ta, &tb, &const_1, &t2, ctx);
	}
#endif
}

static void cmplx_asinhacosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx, const decNumber *add) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2;

	cmplxSqr(&s1, &t2, a, b, ctx);

	decNumberAdd(&t1, &s1, add, ctx);

	cmplxSqrt(&s1, &s2, &t1, &t2, ctx);
	decNumberAdd(&t1, &s1, a, ctx);
	decNumberAdd(&t2, &s2, b, ctx);

	cmplxLn(rx, ry, &t1, &t2, ctx);
#endif
}

// arcsinh(a + i b) = ln((a + i b) + sqrt((a + i b) (a + i b) + 1)
void cmplxAsinh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplx_asinhacosh(rx, ry, a, b, ctx, &const_1);
}

// arccosh(a + i b) = ln((a + i b) + sqrt((a + i b) (a + i b) - 1)
void cmplxAcosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplx_asinhacosh(rx, ry, a, b, ctx, &const__1);
}

// arctanh(a + i b) = (1/2)*ln((1 + (a + i b))/(1 - (a + i b)))
void cmplxAtanh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2;

	decNumberAdd(&t1, a, &const_1, ctx);
	cmplxSubtractFromReal(&t2, ry, &const_1, a, b, ctx);

	cmplxDivide(&s1, &s2, &t1, b, &t2, ry, ctx);
	cmplxLn(&t1, &t2, &s1, &s2, ctx);

	cmplxMultiplyReal(rx, ry, &t1, &t2, &const_0_5, ctx);
#endif
}


void cmplxLn1p(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t;

	if (decNumberIsZero(b)) {
		decNumberAbs(&t, a, ctx);
		decNumberCompare(&t, &t, &const_0_0001, ctx);
		if (decNumberIsNegative(&t)) {
			decNumberZero(ry);
			decNumberLn1p(rx, a, ctx);
			return;
		}
	}
	decNumberAdd(&t, a, &const_1, ctx);
	cmplxLn(rx, ry, &t, b, ctx);
#endif
}

void cmplxExpm1(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t;

	if (decNumberIsZero(b)) {
		decNumberAbs(&t, a, ctx);
		decNumberCompare(&t, &t, &const_0_0001, ctx);
		if (decNumberIsNegative(&t)) {
			decNumberZero(ry);
			decNumberExpm1(rx, a, ctx);
			return;
		}
	}
	cmplxExp(&t, ry, a, b, ctx);
	decNumberSubtract(rx, &t, &const_1, ctx);
#endif
}


void cmplx_do_log(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *base, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber s1, s2;

	if (decNumberIsInfinite(a) || decNumberIsInfinite(b)) {
		set_inf(rx);
		decNumberZero(ry);
	} else {
		cmplxLn(&s1, &s2, a, b, ctx);
		cmplxDivideReal(rx, ry, &s1, &s2, base, ctx);
	}
#endif
}

void cmplxLog(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplx_do_log(rx, ry, a, b, &const_ln10, ctx);
}

void cmplx10x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplxRealPower(rx, ry, &const_10, a, b, ctx);
}

void cmplxLog2(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplx_do_log(rx, ry, a, b, &const_ln2, ctx);
}

void cmplx2x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	cmplxRealPower(rx, ry, &const_2, a, b, ctx);
}

void cmplxLogxy(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber la, lb, lc, ld;

	cmplxLn(&la, &lb, a, b, ctx);
	cmplxLn(&lc, &ld, c, d, ctx);
	cmplxDivide(rx, ry, &la, &lb, &lc, &ld, ctx);
#endif
}

void cmplxlamW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
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
	decNumberCompare(&t1, &const_500, a, ctx);
	if (! decNumberIsNegative(&t1)) {
		// x<500, lx1 = ln(x+1); est = .665 * (1 + .0195*lx1) * lx1 + 0.04
		decNumberAdd(&t1, a, &const_1, ctx);
		cmplxLn(&w1, &w2, &t1, b, ctx);				// ln(1+a,b)
		cmplxMultiplyReal(&t1, &s2, &w1, &w2, &const_0_0195, ctx);
		decNumberAdd(&s1, &const_1, &t1, ctx);
		cmplxMultiplyReal(&u1, &u2, &s1, &s2, &const_0_665, ctx);
		cmplxMultiply(&t1, ry, &u1, &u2, &w1, &w2, ctx);
		decNumberAdd(rx, &const_0_04, &t1, ctx);
	} else {
		// x>=500, est = ln(x-4) - (1 - 1/ln(x)) * ln(ln(x))
		cmplxLn(&w1, &w2, a, b, ctx);				// ln(a,b)
		cmplxLn(&t1, &t2, &w1, &w2, ctx);			// ln(ln(a,b))
		cmplxRecip(&u1, &u2, &w1, &w2, ctx);			// 1 / ln(a,b)
		cmplxSubtractFromReal(&s1, &s2, &const_1, &u1, &u2, ctx);// 1 - 1 / ln(a,b)
		cmplxMultiply(&u1, &u2, &s1, &s2, &t1, &t2, ctx);

		decNumberSubtract(&s1, a, &const_4, ctx);
		cmplxLn(&t1, &t2, &s1, b, ctx);				// ln(a-4,b)

		cmplxSubtract(rx, ry, &t1, &t2, &u1, &u2, ctx);
	}

	for (i=0; i<20; i++) {
		// Now iterate to refine the estimate
		decNumberAdd(&u1, rx, &const_1, ctx);			// u = wj + 1	(u1,ry)
		cmplxExp(&t1, &t2, rx, ry, ctx);			// t = e^wj
		cmplxMultiply(&s1, &s2, &u1, ry, &t1, &t2, ctx);	// (s1,s2) = (wj+1)e^wj

		decNumberAdd(&v1, &u1, &const_1, ctx);			// v = wj + 2	(v1,ry)
		decNumberAdd(&w1, &u1, &u1, ctx);			// w = 2wj + 2
		decNumberAdd(&w2, ry, ry, ctx);
		cmplxDivide(&u1, &u2, &v1, ry, &w1, &w2, ctx);		// (u1,u2) = (wj+2)/(2wj+2)
		cmplxMultiply(&w1, &w2, &t1, &t2, rx, ry, ctx);		// (w1,w2) = wj e^wj

		// Check for termination w, x, u & s are live here
		cmplxSubtract(&v1, &v2, a, b, &w1, &w2, ctx);		// (v1,v2) = x - wj e^wj
		cmplxDivide(&t1, &t2, &v1, &v2, &s1, &s2, ctx);
		cmplxR(&t2, &t1, &t2, ctx);
		decNumberCompare(&t1, &t2, &const_1e_32, ctx);
		if (decNumberIsNegative(&t1))
			break;

		// Continue the increment update
		cmplxMinus(&v1, &v2, &v1, &v2, ctx);
		cmplxSubtract(&v1, &v2, &w1, &w2, a, b, ctx);		// (v1,v2) = wj e^wj - x
		cmplxMultiply(&t1, &t2, &v1, &v2, &u1, &u2, ctx);	// t = (wj+2).(wj e^wj - x) / (2wj + 2)
		cmplxSubtract(&w1, &w2, &s1, &s2, &t1, &t2, ctx);	// w = denominator
		cmplxDivide(&t1, &t2, &v1, &v2, &w1, &w2, ctx);
		cmplxSubtract(rx, ry, rx, ry, &t1, &t2, ctx);		// wj+1
	}
#endif
}

void cmplxInvW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t1, t2;

	cmplxExp(&t1, &t2, a, b, ctx);
	cmplxMultiply(rx, ry, &t1, &t2, a, b, ctx);
#endif
}

// ln(a + i b) = ln(sqrt(a*a + b*b)) + i (2*arctan(signum(b)) - arctan(a/b))
// signum(b) = 1 if b>0, 0 if b=0, -1 if b<0, atan(1) = pi/4
void cmplxLn(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber u;

	if (decNumberIsZero(b)) {
		if (decNumberIsZero(a)) {
			cmplx_NaN(rx, ry);
		} else if (decNumberIsNegative(a)) {
			decNumberMinus(&u, a, ctx);
			decNumberLn(rx, &u, ctx);
			decNumberPI(ry);
		} else {
			decNumberZero(ry);
			decNumberLn(rx, a, ctx);
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
		cmplxToPolar(&u, ry, a, b, ctx);
		decNumberLn(rx, &u, ctx);
	}
#endif
}

// e ^ ( a + i b ) = e^a cos(b) + i e^a sin(b)
void cmplxExp(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber e, s, c;

	if (decNumberIsZero(b)) {
		decNumberExp(rx, a, ctx);
		decNumberZero(ry);
	} else if (decNumberIsSpecial(a) || decNumberIsSpecial(b)) {
		cmplx_NaN(rx, ry);
	} else {
		decNumberExp(&e, a, ctx);
		dn_sincos(b, &s, &c, ctx);
		decNumberMultiply(rx, &e, &c, ctx);
		decNumberMultiply(ry, &e, &s, ctx);
	}
#endif
}

#ifndef TINY_BUILD
static int cmplx_perm_helper(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d, decContext *ctx) {
	decNumber n, m, s1, s2;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b) || decNumberIsSpecial(c) || decNumberIsSpecial(d)) {
		cmplx_NaN(rx, ry);
		return 0;
	}
		
	decNumberAdd(&n, a, &const_1, ctx);		// x+1
	cmplxLnGamma(&s1, &s2, &n, b, ctx);		// lnGamma(x+1) = Ln x!

	cmplxSubtract(rx, ry, &n, b, c, d, ctx);	// x-y+1
	cmplxLnGamma(&n, &m, rx, ry, ctx);		// LnGamma(x-y+1) = Ln (x-y)!
	cmplxSubtract(rx, ry, &s1, &s2, &n, &m, ctx);
	return 1;
}
#endif

/* Calculate permutations:
 * C(x, y) = P(x, y) / y! = x! / ( (x-y)! y! )
 */
void cmplxComb(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber r1, r2, n, m, s1, s2;
	const int code = cmplx_perm_helper(&r1, &r2, a, b, c, d, ctx);

	if (code) {
		decNumberAdd(&n, c, &const_1, ctx);	// y+1
		cmplxLnGamma(&s1, &s2, &n, d, ctx);	// LnGamma(y+1) = Ln y!
		cmplxSubtract(&n, &m, &r1, &r2, &s1, &s2, ctx);

		cmplxExp(rx, ry, &n, &m, ctx);
	} else
		cmplxCopy(rx, ry, &r1, &r2);
#endif
}

/* Calculate permutations:
 * P(x, y) = x! / (x-y)!
 */
void cmplxPerm(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t1, t2;
	const int code = cmplx_perm_helper(&t1, &t2, a, b, c, d, ctx);

	if (code)
		cmplxExp(rx, ry, &t1, &t2, ctx);
	else
		cmplxCopy(rx, ry, &t1, &t2);
#endif
}


#ifndef TINY_BUILD
static void c_lg(decNumber *rx, decNumber *ry, const decNumber *x, const decNumber *y, decContext *ctx) {
	decNumber s1, s2, t1, t2, u1, u2, v1, v2;
	int k;

	decNumberZero(&s1);
	decNumberZero(&s2);
	decNumberAdd(&t1, x, &const_21, ctx);		// ( t1, y )
	for (k=20; k>=0; k--) {
		cmplxDivideRealBy(&u1, &u2, gamma_consts[k], &t1, y, ctx);
		dn_dec(&t1, ctx);
		cmplxAdd(&s1, &s2, &s1, &s2, &u1, &u2, ctx);
	}
	decNumberAdd(&t1, &s1, &const_gammaC00, ctx);	// (t1, s2)
	cmplxMultiplyReal(&u1, &u2, &t1, &s2, &const_2rootEonPI, ctx);
	cmplxLn(&s1, &s2, &u1, &u2, ctx);

	decNumberAdd(&t1, x, &const_gammaR, ctx);	// (t1, y)
	cmplxDivideReal(&u1, &u2, &t1, y, &const_e, ctx);
	cmplxLn(&t1, &t2, &u1, &u2, ctx);
	decNumberAdd(&u1, x, &const_0_5, ctx);		// (u1, y)
	cmplxMultiply(&v1, &v2, &u1, y, &t1, &t2, ctx);
	cmplxAdd(rx, ry, &v1, &v2, &s1, &s2, ctx);
}
#endif


void cmplxGamma(decNumber *rx, decNumber *ry, const decNumber *xin, const decNumber *y, decContext *ctx) {
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
		decNumberSubtract(&t1, &const_1, xin, ctx);
		if (decNumberIsZero(y) && is_int(&t1, ctx)) {
			cmplx_NaN(rx, ry);
			return;
		}
		decNumberSubtract(&x, &t1, &const_1, ctx);
	} else
		decNumberSubtract(&x, xin, &const_1, ctx);

	// Sum the series
	c_lg(&s1, &s2, &x, y, ctx);
	cmplxExp(rx, ry, &s1, &s2, ctx);

	// Finally invert if we started with a negative argument
	if (reflec) {
		cmplxMultiplyReal(&t1, &t2, xin, y, &const_PI, ctx);
		cmplxSin(&s1, &s2, &t1, &t2, ctx);
		cmplxMultiply(&u1, &u2, &s1, &s2, rx, ry, ctx);
		cmplxDivideRealBy(rx, ry, &const_PI, &u1, &u2, ctx);
	}
#endif
}

void cmplxLnGamma(decNumber *rx, decNumber *ry, const decNumber *xin, const decNumber *y, decContext *ctx) {
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
		decNumberSubtract(&t1, &const_1, xin, ctx);
		if (decNumberIsZero(y) && is_int(&t1, ctx)) {
			cmplx_NaN(rx, ry);
			return;
		}
		decNumberSubtract(&x, &t1, &const_1, ctx);
	} else
		decNumberSubtract(&x, xin, &const_1, ctx);

	c_lg(rx, ry, &x, y, ctx);

	// Finally invert if we started with a negative argument
	if (reflec) {
		cmplxMultiplyReal(&t1, &t2, xin, y, &const_PI, ctx);
		cmplxSin(&s1, &s2, &t1, &t2, ctx);
		cmplxDivideRealBy(&u1, &u2, &const_PI, &s1, &s2, ctx);
		cmplxLn(&t1, &t2, &u1, &u2, ctx);
		cmplxSubtract(rx, ry, &t1, &t2, rx, ry, ctx);
	}
#endif
}

void cmplxFactorial(decNumber *rx, decNumber *ry, const decNumber *xin, const decNumber *y, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber x;

	decNumberAdd(&x, xin, &const_1, ctx);
	cmplxGamma(rx, ry, &x, y, ctx);
#endif
}

#ifdef INCLUDE_DBLFACT
void cmplxDblFactorial(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, u1, u2, v1, v2, w1, w2;

	decNumberAdd(&t, a, &const_1, ctx);
	cmplx2x(&u1, &u2, &t, b, ctx);
	cmplxDivideReal(&v1, &v2, &u1, &u2, &const_PI, ctx);
	cmplxSqrt(&u1, &u2, &v1, &v2, ctx);
	cmplxMultiplyReal(&v1, &v2, a, b, &const_0_5, ctx);
	decNumberAdd(&t, &v1, &const_1, ctx);
	cmplxGamma(&w1, &w2, &t, &v2, ctx);
	cmplxMultiply(rx, ry, &w1, &w2, &u1, &u2, ctx);
#endif
}
#endif

// Beta(a, b) = exp(lngamma(a) + lngamma(b) - lngamma(a+b))
void cmplxLnBeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber s1, s2, t1, t2, u1, u2;

	cmplxLnGamma(&s1, &s2, a, b, ctx);
	cmplxLnGamma(&t1, &t2, c, d, ctx);
	cmplxAdd(&u1, &u2, &s1, &s2, &t1, &t2, ctx);
	cmplxAdd(&s1, &s2, a, b, c, d, ctx);
	cmplxLnGamma(&t1, &t2, &s1, &s2, ctx);
	cmplxSubtract(rx, ry, &u1, &u2, &t1, &t2, ctx);
#endif
}

// Beta(a, b) = exp(lngamma(a) + lngamma(b) - lngamma(a+b))
void cmplxBeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber s1, s2;

	cmplxLnBeta(&s1, &s2, a, b, c, d, ctx);
	cmplxExp(rx, ry, &s1, &s2, ctx);
#endif
}

#ifdef INCLUDE_DIGAMMA
// Digamma function
extern void cmplxPsi(decNumber *rx, decNumber *ry, const decNumber *ain, const decNumber *bin, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, t1, t2, r1, r2, x_2x, x_2y;
	int i;

	if (decNumberIsSpecial(ain) || decNumberIsSpecial(bin)) {
		cmplx_NaN(rx, ry);
		return;
	}

	// Reflection if negative real part
	if (dn_le0(ain)) {
		if (decNumberIsZero(bin) && is_int(ain, ctx)) {
			cmplx_NaN(rx, ry);
			return;
		}
		cmplxMultiplyReal(&r1, &r2, ain, bin, &const_PI, ctx);
		cmplxTan(&t1, &t2, &r1, &r2,ctx);
		cmplxDivideRealBy(&r1, &r2, &const_PI, &t1, &t2, ctx);
		cmplxSubtractFromReal(&a, &b, &const_1, ain, bin, ctx);
		cmplxMinus(rx, ry, &r1, &r2, ctx);
	} else {
		decNumberZero(rx);
		decNumberZero(ry);
		decNumberCopy(&a, ain);
		decNumberCopy(&b, bin);
	}

	// Recurrance to push real part > 8
	for (;;) {
		decNumberCompare(&t1, &const_8, &a, ctx);
		if (decNumberIsNegative(&t1))
			break;
		cmplxRecip(&t1, &t2, &a, &b, ctx);
		cmplxSubtract(rx, ry, rx, ry, &t1, &t2, ctx);
		dn_inc(&a, ctx);
	}

	// Series approximation
	cmplxLn(&t1, &t2, &a, &b, ctx);
	cmplxAdd(rx, ry, rx, ry, &t1, &t2, ctx);
	cmplxMultiplyReal(&r1, &r2, &a, &b, &const__2, ctx);
	cmplxRecip(&t1, &t2, &r1, &r2, ctx);
	cmplxAdd(rx, ry, rx, ry, &t1, &t2, ctx);

	cmplxSqr(&t1, &t2, &a, &b, ctx);
	cmplxRecip(&x_2x, &x_2y, &t1, &t2, ctx);
	cmplxCopy(&r1, &r2, &x_2x, &x_2y);
	for (i=0; i<10; i++) {
		cmplxMultiplyReal(&t1, &t2, &r1, &r2, digamma_consts[i], ctx);
		cmplxAdd(rx, ry, rx, ry, &t1, &t2, ctx);
		cmplxMultiply(&r1, &r2, &r1, &r2, &x_2x, &x_2y, ctx);
	}
#endif
}
#endif 


#ifdef INCLUDE_COMPLEX_ZETA
/* Riemann's Zeta function */
#ifndef TINY_BUILD
static void c_zeta_step(decNumber *sx, decNumber *sy,
		const decNumber *x, const decNumber *y,
		const decNumber *dc, decNumber *k, decContext *ctx) {
	decNumber t1, t2, s1, s2;

	dn_inc(k, ctx);
	cmplxRealPower(&s1, &s2, k, x, y, ctx);
	cmplxDivideRealBy(&t1, &t2, dc, &s1, &s2, ctx);
	cmplxAdd(sx, sy, sx, sy, &t1, &t2, ctx);
}
#endif

void cmplxZeta(decNumber *rx, decNumber *ry,
		const decNumber *xin, const decNumber *yin, decContext *ctx) {
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
		decNumberZeta(rx, xin, ctx);
		decNumberZero(ry);
		return;
	}
#endif

	decNumberCompare(&s1, xin, &const_0_5, ctx);
	if (decNumberIsNegative(&s1)) {
		/* use reflection formula
		 * zeta(x) = 2^x*Pi^(x-1)*sin(Pi*x/2)*gamma(1-x)*zeta(1-x)
		 */
		reflec = 1;
		cmplxSubtractFromReal(&x, &y, &const_1, xin, yin, ctx);
		cmplxMultiplyReal(&u1, &u2, xin, yin, &const_PIon2, ctx);
		cmplxSin(&s1, &s2, &u1, &u2, ctx);
		cmplxRealPower(rx, ry, &const_2, xin, yin, ctx);
		cmplxMultiply(&u1, &u2, rx, ry, &s1, &s2, ctx);
		cmplxRealPower(rx, ry, &const_PI, &x, &y, ctx);
		cmplxDivide(&s1, &s2, &u1, &u2, rx, ry, ctx);
		cmplxGamma(rx, ry, &x, &y, ctx);
		cmplxMultiply(&reflecfac1, &reflecfac2, &s1, &s2, rx, ry, ctx);
	} else {
		reflec = 0;
		cmplxCopy(&x, &y, xin, yin);
	}

	/* Now calculate zeta(x) where x >= 0.5 */
	decNumberZero(&sum1);
	decNumberZero(&sum2);
	decNumberZero(&t1);
	for (i=0; i<30; i++)
		c_zeta_step(&sum1, &sum2, &x, &y, zeta_consts[i], &t1, ctx);

	cmplxSubtractFromReal(&t1, &t2, &const_1, &x, &y, ctx);
	cmplxRealPower(&u1, &u2, &const_2, &t1, &t2, ctx);
	dn_dec(&u1, ctx);
	cmplxMultiplyReal(&t1, &t2, &u1, &u2, &const_zeta_dn, ctx);
	cmplxDivide(rx, ry, &sum1, &sum2, &t1, &t2, ctx);

	/* Finally, undo the reflection if required */
	if (reflec)
		cmplxMultiply(rx, ry, &reflecfac1, &reflecfac2, rx, ry, ctx);
#endif
}
#endif /* INCLUDE_COMPLEX_ZETA */


void cmplxParallel(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber p1, p2, s1, s2;

	cmplxMultiply(&p1, &p2, a, b, c, d, ctx);
	cmplxAdd(&s1, &s2, a, b, c, d, ctx);
	cmplxDivide(rx, ry, &p1, &p2, &s1, &s2, ctx);
#endif
}

void cmplxAGM(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx) {
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
		cmplxSubtract(&t1, &t2, &x1, &x2, &y1, &y2, ctx);
		cmplxR(&u1, &t1, &t2, ctx);
		decNumberCompare(&u2, &u1, &const_1e_32, ctx);
		if (decNumberIsNegative(&u2)) {
			cmplxCopy(rx, ry, &x1, &x2);
			return;
		}

		cmplxAdd(&t1, &t2, &x1, &x2, &y1, &y2, ctx);
		cmplxMultiplyReal(&u1, &u2, &t1, &t2, &const_0_5, ctx);

		cmplxMultiply(&t1, &t2, &x1, &x2, &y1, &y2, ctx);
		cmplxSqrt(&y1, &y2, &t1, &t2, ctx);

		cmplxCopy(&x1, &x2, &u1, &u2);
	}
nan:	cmplx_NaN(rx, ry);
#endif
}

void cmplxRnd(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumberRnd(rx, a, ctx);
	decNumberRnd(ry, b, ctx);
}


void cmplxFrac(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumberFrac(rx, a, ctx);
	decNumberFrac(ry, b, ctx);
}

void cmplxTrunc(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
	decNumberTrunc(rx, a, ctx);
	decNumberTrunc(ry, b, ctx);
}

void cmplxFib(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber r1, r2, s1, s2, t1, t2;

	cmplxRealPower(&r1, &r2, &const_phi, a, b, ctx);
	cmplxMultiplyReal(&t1, &t2, a, b, &const_PI, ctx);
	cmplxCos(&s1, &s2, &t1, &t2, ctx);
	cmplxDivide(&t1, &t2, &s1, &s2, &r1, &r2, ctx);
	cmplxSubtract(&s1, &s2, &r1, &r2, &t1, &t2, ctx);
	cmplxMultiplyReal(rx, ry, &s1, &s2, &const_recipsqrt5, ctx);
#endif
}

#ifdef INCLUDE_ELLIPTIC
#ifndef TINY_BUILD
static void elliptic_setup(decNumber *r,
		decNumber *snuk, decNumber *cnuk, decNumber *dnuk,
		decNumber *snvki, decNumber *cnvki, decNumber *dnvki,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki, decContext *ctx) {
	decNumber a;

	dn_elliptic(snuk, cnuk, dnuk, u, k, ctx);
	dn_elliptic(snvki, cnvki, dnvki, v, ki, ctx);

	decNumberMultiply(&a, dnuk, snvki, ctx);
	decNumberSquare(r, &a, ctx);
	decNumberSubtract(&a, &const_1, r, ctx);
	decNumberRecip(r, &a, ctx);
}
#endif


// SN(u + i v, k + i ki) = sn(u, k) . dn(v, ki) / denom
//		  + i . cn(u, k) . dn(u, k) . sn(v, ki) . cn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxSN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki, ctx);

	decNumberMultiply(&a, &snuk, &dnvki, ctx);
	decNumberMultiply(rx, &a, &denom, ctx);

	decNumberMultiply(&a, &cnuk, &dnuk, ctx);
	decNumberMultiply(&b, &a, &snvki, ctx);
	decNumberMultiply(&a, &b, &cnvki, ctx);
	decNumberMultiply(ry, &a, &denom, ctx);
#endif
}


// CN(u + i v, k + i ki) = cn(u, k) . cn(v, ki) / denom
//		  - i . sn(u, k) . dn(u, k) . sn(v, ki) . dn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxCN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki, ctx);

	decNumberMultiply(&a, &cnuk, &cnvki, ctx);
	decNumberMultiply(rx, &a, &denom, ctx);

	decNumberMultiply(&a, &snuk, &dnuk, ctx);
	decNumberMultiply(&b, &a, &snvki, ctx);
	decNumberMultiply(&a, &b, &dnvki, ctx);
	decNumberMultiply(&b, &a, &denom, ctx);
	decNumberMinus(ry, &b, ctx);
#endif
}


// DN(a + i b, c + i d) = dn(u, k) . cn(v, ki) . dn(v, ki) / denom
//		  - i . k^2 . sn(u, k) . cn(u, k) . sn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxDN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki, ctx);

	decNumberMultiply(&a, &dnuk, &cnvki, ctx);
	decNumberMultiply(&b, &a, &dnvki, ctx);
	decNumberMultiply(rx, &b, &denom, ctx);

	decNumberSquare(&a, k, ctx);
	decNumberMinus(&b, &a, ctx);
	decNumberMultiply(&a, &b, &snuk, ctx);
	decNumberMultiply(&b, &a, &cnuk, ctx);
	decNumberMultiply(&a, &b, &snvki, ctx);
	decNumberMultiply(ry, &a, &denom, ctx);
#endif
}
#endif

#ifdef COMPLEX_BESSEL
#ifndef TINY_BUILD
static void cmplx_bessel(decNumber *rx, decNumber *ry,
			const decNumber *nx, const decNumber *ny,
			const decNumber *xx, const decNumber *xy, decContext *ctx, const int neg) {
	decNumber x2on4x, x2on4y, k, ux, uy;
	decNumber t1, t2, a1, a2, b;
	int i;

	cmplxMultiplyReal(&t1, &t2, xx, xy, &const_0_5, ctx);
	cmplxSqr(&x2on4x, &x2on4y, &t1, &t2, ctx);

	cmplxPower(&t1, &t2, &t1, &t2, nx, ny, ctx);
	decNumberAdd(&b, nx, &const_1, ctx);
	cmplxGamma(&a1, &a2, &b, ny, ctx);
	cmplxDivide(&ux, &uy, &t1, &t2, &a1, &a2, ctx);

	decNumberCopy(&k, &const_1);
	cmplxCopy(rx, ry, &ux, &uy);

	for (i=0;i<1000;i++) {
		cmplxMultiply(&t1, &t2, &x2on4x, &x2on4y, &ux, &uy, ctx);
		if (neg)
			cmplxMinus(&t1, &t2, &t1, &t2, ctx);
		decNumberAdd(&b, nx, &k, ctx);
		cmplxMultiplyReal(&a1, &a2, &b, ny, &k, ctx);
		cmplxDivide(&ux, &uy, &t1, &t2, &a1, &a2, ctx);

		cmplxAdd(&a1, &a2, &ux, &uy, rx, ry, ctx);
		cmplxSubtract(&t1, &t2, &a1, &a2, rx, ry, ctx);
		cmplxR(&b, &t1, &t2, ctx);
		if (decNumberIsZero(&b))
			return;
		cmplxCopy(rx, ry, &a1, &a2);
		dn_inc(&k, ctx);
	}
	cmplx_NaN(rx, ry);
}
#endif

void cmplxBSJN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy, decContext *ctx) {
#ifndef TINY_BUILD
	if (decNumberIsZero(xy) && decNumberIsZero(alphay)) {
		decNumberBSJN(rx, alphax, xx, ctx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else if (decNumberIsSpecial(alphax) || decNumberIsSpecial(alphay) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else
		cmplx_bessel(rx, ry, alphax, alphay, xx, xy, ctx, 1);
#endif
}

void cmplxBSIN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy, decContext *ctx) {
#ifndef TINY_BUILD
	if (decNumberIsZero(xy) && decNumberIsZero(alphay)) {
		decNumberBSIN(rx, alphax, xx, ctx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else if (decNumberIsSpecial(alphax) || decNumberIsSpecial(alphay) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else
		cmplx_bessel(rx, ry, alphax, alphay, xx, xy, ctx, 0);
#endif
}


// See A&S page 360 section 9.1.11
#ifndef TINY_BUILD
static void cmplx_bessel2_int_series(decNumber *rx, decNumber *ry, const decNumber *n, const decNumber *x, const decNumber *y, decContext *ctx, int modified) {
	const decNumber *const factor = modified?&const_0_5:&const__1onPI;
	decNumber xon2x, xon2y, xon2nx, xon2ny, x2on4x, x2on4y;
	decNumber k, npk, tx, ty, ux, uy, vx, vy, sx, sy, px, py, nf, absn;
	int i, in, n_odd, n_neg;

	if (decNumberIsNegative(n)) {
		n = decNumberAbs(&absn, n, ctx);
		n_neg = 1;
	} else	n_neg = 0;
	in = dn_to_int(n, ctx);
	n_odd = in & 1;

	cmplxMultiplyReal(&xon2x, &xon2y, x, y, &const_0_5, ctx);	// xon2 = x/2
	cmplxPowerReal(&xon2nx, &xon2ny, &xon2x, &xon2y, n, ctx);	// xon2n = (x/2)^n
	cmplxSqr(&x2on4x, &x2on4y, &xon2x, &xon2y, ctx);		// x2on4 = +/- x^2/4

	if (modified)
		cmplxMinus(&x2on4x, &x2on4y, &x2on4x, &x2on4y, ctx);
	if (in > 0) {
		decNumberSubtract(&vx, n, &const_1, ctx);	// v = n-k-1 = n-1
		decNumberZero(&k);
		decNumberGamma(&px, n, ctx);		// p = (n-1)!
		decNumberZero(&py);
		decNumberZero(&sy);
		decNumberCopy(&sx, &px);
		decNumberMultiply(&nf, &px, n, ctx);	// nf = n!  (for later)
		for (i=1; i<in; i++) {
			cmplxDivideReal(&tx, &ty, &px, &py, &vx, ctx);
			dn_dec(&vx, ctx);
			dn_inc(&k, ctx);
			cmplxMultiplyReal(&ux, &uy, &tx, &ty, &k, ctx);
			cmplxMultiply(&px, &py, &ux, &uy, &x2on4x, &x2on4y, ctx);
			cmplxAdd(&sx, &sy, &sx, &sy, &px, &py, ctx);
		}
		cmplxMultiplyReal(&tx, &ty, &sx, &sy, factor, ctx);
		cmplxDivide(rx, ry, &tx, &ty, &xon2nx, &xon2ny, ctx);
	} else {
		decNumberZero(rx);
		decNumberZero(ry);
		decNumberCopy(&nf, &const_1);
	}

	if (modified) {
		cmplxBSIN(&tx, &ty, n, &const_0, x, y, ctx);
		if (!n_odd)
			cmplxMinus(&tx, &ty, &tx, &ty, ctx);
	} else {
		cmplxBSJN(&ux, &uy, n, &const_0, x, y, ctx);
		cmplxDivideReal(&tx, &ty, &ux, &uy, &const_PIon2, ctx);
	}
	cmplxLn(&ux, &uy, &xon2x, &xon2y, ctx);
	cmplxMultiply(&vx, &vy, &ux, &uy, &tx, &ty, ctx);
	cmplxAdd(rx, ry, rx, ry, &vx, &vy, ctx);

	cmplxMinus(&x2on4x, &x2on4y, &x2on4x, &x2on4y, ctx);
	decNumberAdd(&tx, n, &const_1, ctx);			// t = n+1
	decNumberPsi(&ux, &tx, ctx);				// u = Psi(n+1)
	decNumberSubtract(&vx, &ux, &const_egamma, ctx);	// v = psi(k+1) + psi(n+k+1)
	decNumberZero(&k);
	decNumberZero(&py);
	decNumberZero(&sy);
	decNumberCopy(&npk, n);
	decNumberRecip(&px, &nf, ctx);			// p = (x^2/4)^k/(k!(n+k)!)
	decNumberMultiply(&sx, &vx, &px, ctx);

	for (i=0;i<1000;i++) {
		dn_inc(&k, ctx);
		dn_inc(&npk, ctx);
		cmplxMultiply(&tx, &ty, &px, &py, &x2on4x, &x2on4y, ctx);
		decNumberMultiply(&ux, &k, &npk, ctx);
		cmplxDivideReal(&px, &py, &tx, &ty, &ux, ctx);

		decNumberRecip(&tx, &k, ctx);
		decNumberAdd(&ux, &vx, &tx, ctx);
		decNumberRecip(&tx, &npk, ctx);
		decNumberAdd(&vx, &ux, &tx, ctx);

		cmplxMultiplyReal(&tx, &ty, &px, &py, &vx, ctx);
		cmplxAdd(&ux, &uy, &tx, &ty, &sx, &sy, ctx);
		cmplxSubtract(&tx, &ty, &ux, &uy, &sx, &sy, ctx);
		cmplxR(&vy, &tx, &ty, ctx);
		if (decNumberIsZero(&vy))
			break;
		cmplxCopy(&sx, &sy, &ux, &uy);
	}
	cmplxMultiply(&tx, &ty, &sx, &sy, &xon2nx, &xon2ny, ctx);
	if (modified) {
		if (n_odd)
			cmplxMultiplyReal(&ux, &uy, &tx, &ty, &const__0_5, ctx);
		else
			cmplxMultiplyReal(&ux, &uy, &tx, &ty, &const_0_5, ctx);
	} else
		cmplxMultiplyReal(&ux, &uy, &tx, &ty, &const__1onPI, ctx);
	cmplxAdd(rx, ry, rx, ry, &ux, &uy, ctx);
	if (!modified && n_neg)
		cmplxMinus(rx, ry, rx, ry, ctx);
}
#endif


// Yv(z) = ( Jv(z).cos(v PI) - J-v(z) ) / sin(v PI)
//Y(3.5+4.5i)(1.5+2.5i) = -1.19957042014349569 + 2.34640749171622195334 i
void cmplxBSYN(decNumber *rx, decNumber *ry,
		const decNumber *vx, const decNumber *vy,
		const decNumber *xx, const decNumber *xy, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t1, t2, sx, sy, cx, cy, jnx, jny, jmx, jmy;
#if 0
	if (decNumberIsZero(xy) && decNumberIsZero(vy)) {
		decNumberBSYN(rx, vx, xx, ctx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else
#endif
	if (decNumberIsSpecial(vx) || decNumberIsSpecial(vy) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else if (decNumberIsZero(vy) && is_int(vx, ctx)) {
		cmplx_bessel2_int_series(rx, ry, vx, xx, xy, ctx, 0);
	} else {
		cmplx_bessel(&jnx, &jny, vx, vy, xx, xy, ctx, 1);
		cmplxMinus(&t1, &t2, vx, vy, ctx);
		cmplx_bessel(&jmx, &jmy, &t1, &t2, xx, xy, ctx, 1);
		cmplxMultiplyReal(&t1, &t2, vx, vy, &const_PI, ctx);
		cmplx_sincos(&t1, &t2, &sx, &sy, &cx, &cy, ctx);
		cmplxMultiply(&t1, &t2, &jnx, &jny, &cx, &cy, ctx);
		cmplxSubtract(&jnx, &jny, &t1, &t2, &jmx, &jmy, ctx);
		cmplxDivide(rx, ry, &jnx, &jny, &sx, &sy, ctx);
	}
#endif
}

// Kn(x) = PI/2 . (I-n(x) - In(x)) / sin(n PI)
//K(3.5+4.5i)(1.5+2.5i) = 2.41061613980770099 + 1.0563528861789105228i
void cmplxBSKN(decNumber *rx, decNumber *ry,
		const decNumber *vx, const decNumber *vy,
		const decNumber *xx, const decNumber *xy, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t1, t2, inx, iny, imx, imy;

#if 0
	if (decNumberIsZero(xy) && decNumberIsZero(vy)) {
		decNumberBSKN(rx, vx, xx, ctx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else
#endif
	if (decNumberIsSpecial(vx) || decNumberIsSpecial(vy) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else if (decNumberIsZero(vy) && is_int(vx, ctx)) {
		cmplx_bessel2_int_series(rx, ry, vx, xx, xy, ctx, 1);
	} else {
		cmplx_bessel(&inx, &iny, vx, vy, xx, xy, ctx, 0);
		cmplxMinus(&t1, &t2, xx, xy, ctx);
		cmplx_bessel(&imx, &imy, vx, vy, &t1, &t2, ctx, 0);
		cmplxSubtract(&t1, &t2, &imx, &imy, &inx, &iny, ctx);
		cmplxMultiplyReal(&imx, &imy, &t1, &t2, &const_PIon2, ctx);
		cmplxMultiplyReal(&inx, &iny, vx, vy, &const_PI, ctx);
		cmplxSin(&t1, &t2, &inx, &iny, ctx);
		cmplxDivide(rx, ry, &imx, &imy, &t1, &t2, ctx);
	}
#endif
}

#endif

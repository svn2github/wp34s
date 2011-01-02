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

#ifndef COMPLEX_H
#define COMPLEX_H

/* Dyadic operation */
extern void cmplxAdd(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxSubtract(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxMultiply(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxDivide(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxPower(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxLogxy(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);

/* And some shortcuts to the above for one argument real */
#if 0
extern void cmplxDivideReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r, decContext *ctx);
extern void cmplxDivideRealBy(decNumber *rx, decNumber *ry,
		const decNumber *a,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxMultiplyReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r, decContext *ctx);
extern void cmplxPowerReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r, decContext *ctx);
extern void cmplxPowerReal(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *r, decContext *ctx);
#endif

/* Conversion to and from polar representation */
extern void cmplxArg(decNumber *arg, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxR(decNumber *r, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxFromPolar(decNumber *x, decNumber *y, const decNumber *r, const decNumber *t, decContext *ctx);
extern void cmplxToPolar(decNumber *r, decNumber *t, const decNumber *x, const decNumber *y, decContext *ctx);


/* Monadic operations */
extern void cmplxRnd(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxAbs(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxSign(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxMinus(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxConj(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplxRecip(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplxSqr(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxSqrt(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxCube(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxCubeRoot(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplxLn(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxExp(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxLn1p(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxExpm1(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxLog(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplx10x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxLog2(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplx2x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxlamW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxInvW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplxSin(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxCos(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxTan(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxAsin(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxAcos(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxAtan(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxSinc(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplxSinh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxCosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxTanh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxAsinh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxAcosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxAtanh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplxComb(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxPerm(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxFactorial(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxDblFactorial(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxGamma(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxLnGamma(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxPsi(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxBeta(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxLnBeta(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxZeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplxParallel(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);
extern void cmplxAGM(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d, decContext *ctx);

extern void cmplxFrac(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);
extern void cmplxTrunc(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplxFib(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, decContext *ctx);

extern void cmplx_NaN(decNumber *, decNumber *);

/* Jacobi's elliptical functions */
extern void cmplxSN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki, decContext *ctx);
extern void cmplxCN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki, decContext *ctx);
extern void cmplxDN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki, decContext *ctx);

/* Bessel functions of first and second kinds */
extern void cmplxBSJN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy, decContext *ctx);
extern void cmplxBSIN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy, decContext *ctx);
extern void cmplxBSYN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy, decContext *ctx);
extern void cmplxBSKN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy, decContext *ctx);

#endif

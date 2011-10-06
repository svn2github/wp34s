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

#ifndef __decn_h__
#define __decn_h__

#include "xeq.h"

// true iff the number is a NaN or infinite
#define decNumberIsSpecial(x)	((x)->bits & DECSPECIAL)

extern int dn_gt0(const decNumber *x);
extern int dn_lt0(const decNumber *x);
extern int dn_le0(const decNumber *x);

extern decNumber *dn_add(decNumber *r, const decNumber *a, const decNumber *b);
extern decNumber *dn_subtract(decNumber *r, const decNumber *a, const decNumber *b);
extern decNumber *dn_multiply(decNumber *r, const decNumber *a, const decNumber *b);
extern decNumber *dn_divide(decNumber *r, const decNumber *a, const decNumber *b);
extern decNumber *dn_compare(decNumber *r, const decNumber *a, const decNumber *b);
extern decNumber *dn_min(decNumber *r, const decNumber *a, const decNumber *b);
extern decNumber *dn_max(decNumber *r, const decNumber *a, const decNumber *b);
extern decNumber *dn_abs(decNumber *r, const decNumber *a);
extern decNumber *dn_minus(decNumber *r, const decNumber *a);
extern decNumber *dn_plus(decNumber *r, const decNumber *a);
extern decNumber *dn_sqrt(decNumber *r, const decNumber *a);
extern decNumber *dn_exp(decNumber *r, const decNumber *a);
extern decNumber *dn_power(decNumber *r, const decNumber *a, const decNumber *b);

/*
#define decNumberAdd(r, a, b, ctx)	dn_add(r, a, b)
#define decNumberSubtract(r, a, b, ctx)	dn_subtract(r, a, b)
#define decNumberMultiply(r, a, b, ctx)	dn_multiply(r, a, b)
#define decNumberDivide(r, a, b, ctx)	dn_divide(r, a, b)
#define decNumberAbs(r, a, ctx)		dn_abs(r, a)
*/

extern void decNumberSwap(decNumber *a, decNumber *b);

extern const decNumber *small_int(int i);
extern void int_to_dn(decNumber *, int);
extern int dn_to_int(const decNumber *);
extern void ullint_to_dn(decNumber *, unsigned long long int);
extern unsigned long long int dn_to_ull(const decNumber *, int *);

extern void decNumberPI(decNumber *pi);
extern void decNumberPIon2(decNumber *pion2);
extern int is_int(const decNumber *);

extern decNumber *decNumberMantissa(decNumber *r, const decNumber *a);
extern decNumber *decNumberExponent(decNumber *r, const decNumber *a);

extern decNumber *decNumberXRoot(decNumber *r, const decNumber *a, const decNumber *b);
extern long long int intXRoot(long long int y, long long int x);

extern int relative_error(const decNumber *x, const decNumber *y, const decNumber *tol);
extern int absolute_error(const decNumber *x, const decNumber *y, const decNumber *tol);

extern decNumber *decNumberMAdd(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x);

extern decNumber *decNumberMod(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberBigMod(decNumber *res, const decNumber *x, const decNumber *y);

extern decNumber *decNumberRnd(decNumber *r, const decNumber *x);
extern decNumber *decNumberRecip(decNumber *r, const decNumber *x);

extern decNumber *decNumberFloor(decNumber *r, const decNumber *x);
extern decNumber *decNumberCeil(decNumber *r, const decNumber *x);
extern decNumber *decNumberTrunc(decNumber *r, const decNumber *x);
extern decNumber *decNumberRound(decNumber *r, const decNumber *x);
extern decNumber *decNumberFrac(decNumber *r, const decNumber *x);
extern decNumber *decNumberSign(decNumber *r, const decNumber *x);

extern decNumber *decNumberGCD(decNumber *r, const decNumber *x, const decNumber *y);
extern decNumber *decNumberLCM(decNumber *r, const decNumber *x, const decNumber *y);

extern decNumber *decNumberPow_1(decNumber *r, const decNumber *x);
extern decNumber *decNumberPow2(decNumber *r, const decNumber *x);
extern decNumber *decNumberPow10(decNumber *r, const decNumber *x);
extern decNumber *decNumberLn1p(decNumber *r, const decNumber *x);
extern decNumber *decNumberExpm1(decNumber *r, const decNumber *x);
extern decNumber *dn_log2(decNumber *r, const decNumber *x);
extern decNumber *dn_log10(decNumber *r, const decNumber *x);
extern decNumber *decNumberLogxy(decNumber *r, const decNumber *x, const decNumber *y);
extern decNumber *decNumberLamW(decNumber *r, const decNumber *x);
extern decNumber *decNumberInvW(decNumber *r, const decNumber *x);

extern decNumber *decNumberSquare(decNumber *r, const decNumber *x);
extern decNumber *decNumberCube(decNumber *r, const decNumber *x);
extern decNumber *decNumberCubeRoot(decNumber *r, const decNumber *x);

extern decNumber *decNumber2Deg(decNumber *res, const decNumber *x);
extern decNumber *decNumber2Rad(decNumber *res, const decNumber *x);
extern decNumber *decNumber2Grad(decNumber *res, const decNumber *x);
extern decNumber *decNumberDeg2(decNumber *res, const decNumber *x);
extern decNumber *decNumberRad2(decNumber *res, const decNumber *x);
extern decNumber *decNumberGrad2(decNumber *res, const decNumber *x);

extern decNumber *decNumberSin(decNumber *res, const decNumber *x);
extern decNumber *decNumberCos(decNumber *res, const decNumber *x);
extern decNumber *decNumberTan(decNumber *res, const decNumber *x);
#if 0
extern decNumber *decNumberSec(decNumber *res, const decNumber *x);
extern decNumber *decNumberCosec(decNumber *res, const decNumber *x);
extern decNumber *decNumberCot(decNumber *res, const decNumber *x);
#endif
extern decNumber *decNumberArcSin(decNumber *res, const decNumber *x);
extern decNumber *decNumberArcCos(decNumber *res, const decNumber *x);
extern decNumber *decNumberArcTan(decNumber *res, const decNumber *x);
extern decNumber *decNumberArcTan2(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberSinc(decNumber *res, const decNumber *x);

extern decNumber *do_atan2(decNumber *at, const decNumber *ain, const decNumber *b);


extern void op_r2p(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_p2r(decimal64 *nul1, decimal64 *nul2, enum nilop op);

extern decNumber *decNumberSinh(decNumber *res, const decNumber *x);
extern decNumber *decNumberCosh(decNumber *res, const decNumber *x);
extern decNumber *decNumberTanh(decNumber *res, const decNumber *x);
#if 0
extern decNumber *decNumberSech(decNumber *res, const decNumber *x);
extern decNumber *decNumberCosech(decNumber *res, const decNumber *x);
extern decNumber *decNumberCoth(decNumber *res, const decNumber *x);
#endif
extern decNumber *decNumberArcSinh(decNumber *res, const decNumber *x);
extern decNumber *decNumberArcCosh(decNumber *res, const decNumber *x);
extern decNumber *decNumberArcTanh(decNumber *res, const decNumber *x);

/* Elliptic functions */
extern decNumber *decNumberSN(decNumber *res, const decNumber *k, const decNumber *u);
extern decNumber *decNumberCN(decNumber *res, const decNumber *k, const decNumber *u);
extern decNumber *decNumberDN(decNumber *res, const decNumber *k, const decNumber *u);

/* Bessel functions */
extern decNumber *decNumberBSJN(decNumber *res, const decNumber *, const decNumber *u);
extern decNumber *decNumberBSIN(decNumber *res, const decNumber *, const decNumber *u);
extern decNumber *decNumberBSYN(decNumber *res, const decNumber *, const decNumber *u);
extern decNumber *decNumberBSKN(decNumber *res, const decNumber *, const decNumber *u);

extern decNumber *decNumberFactorial(decNumber *r, const decNumber *xin);
extern decNumber *decNumberDblFactorial(decNumber *r, const decNumber *xin);
extern decNumber *decNumberSubFactorial(decNumber *r, const decNumber *xin);
extern decNumber *decNumberGamma(decNumber *res, const decNumber *x);
extern decNumber *decNumberLnGamma(decNumber *res, const decNumber *x);
extern decNumber *decNumberPsi(decNumber *res, const decNumber *x);
extern decNumber *decNumberBeta(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberLnBeta(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberZeta(decNumber *res, const decNumber *x);

extern const decNumber *const gamma_consts[];
extern const decNumber *const zeta_consts[];
extern const decNumber *const digamma_consts[];

extern decNumber *decNumberERF(decNumber *res, const decNumber *x);
extern decNumber *decNumberERFC(decNumber *res, const decNumber *x);
extern decNumber *decNumberGammap(decNumber *res, const decNumber *a, const decNumber *x);

extern decNumber *decNumberFib(decNumber *res, const decNumber *x);

extern decNumber *decNumberD2G(decNumber *res, const decNumber *x);
extern decNumber *decNumberD2R(decNumber *res, const decNumber *x);
extern decNumber *decNumberG2D(decNumber *res, const decNumber *x);
extern decNumber *decNumberG2R(decNumber *res, const decNumber *x);
extern decNumber *decNumberR2D(decNumber *res, const decNumber *x);
extern decNumber *decNumberR2G(decNumber *res, const decNumber *x);

extern void decNumber2Fraction(decNumber *n, decNumber *d, const decNumber *x);

extern decNumber *decNumberComb(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberPerm(decNumber *res, const decNumber *x, const decNumber *y);

extern decNumber *decNumberPercent(decNumber *res, const decNumber *x);
extern decNumber *decNumberPerchg(decNumber *res, const decNumber *x);
extern decNumber *decNumberPertot(decNumber *res, const decNumber *x);
extern decNumber *decNumberPerMargin(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberMargin(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNemberPerMRR(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x);

extern decNumber *decNumberHMS2HR(decNumber *res, const decNumber *x);
extern decNumber *decNumberHR2HMS(decNumber *res, const decNumber *x);
extern decNumber *decNumberHMSAdd(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberHMSSub(decNumber *res, const decNumber *x, const decNumber *y);

extern decNumber *decNumberParallel(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberAGM(decNumber *res, const decNumber *x, const decNumber *y);

extern decNumber *decNumberNot(decNumber *res, const decNumber *x);
extern decNumber *decNumberAnd(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberOr(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberXor(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberNand(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberNor(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *decNumberNxor(decNumber *res, const decNumber *x, const decNumber *y);


extern void dn_sincos(const decNumber *v, decNumber *sinv, decNumber *cosv);
extern void dn_sinhcosh(const decNumber *v, decNumber *sinhv, decNumber *coshv);
extern void do_asin(decNumber *, const decNumber *);
extern void do_acos(decNumber *, const decNumber *);
extern void do_atan(decNumber *, const decNumber *);

extern void dn_elliptic(decNumber *sn, decNumber *cn, decNumber *dn, const decNumber *u, const decNumber *k);

extern decNumber *set_NaN(decNumber *);
extern decNumber *set_inf(decNumber *);
extern decNumber *set_neginf(decNumber *);

extern decNumber *dn_inc(decNumber *);
extern decNumber *dn_dec(decNumber *);
extern decNumber *dn_p1(decNumber *, const decNumber *);
extern decNumber *dn_m1(decNumber *, const decNumber *);
extern decNumber *dn_1m(decNumber *, const decNumber *);
extern decNumber *dn_1(decNumber *);
extern decNumber *dn__1(decNumber *);
extern decNumber *dn_p2(decNumber *, const decNumber *);
extern decNumber *dn_mul2(decNumber *, const decNumber *);
extern decNumber *dn_div2(decNumber *, const decNumber *);
extern decNumber *dn_mul100(decNumber *, const decNumber *);

extern void solver_init(decNumber *c, decNumber *, decNumber *, decNumber *, decNumber *, unsigned int *);
extern int solver_step(decNumber *, decNumber *, decNumber *, decNumber *, decNumber *, const decNumber *, unsigned int *, int (*)(const decNumber *, const decNumber *, const decNumber *));

extern void solver(unsigned int arg, enum rarg op);

extern decNumber *decNumberPolyPn(decNumber *res, const decNumber *y, const decNumber *x);
extern decNumber *decNumberPolyTn(decNumber *res, const decNumber *y, const decNumber *x);
extern decNumber *decNumberPolyUn(decNumber *res, const decNumber *y, const decNumber *x);
extern decNumber *decNumberPolyLn(decNumber *res, const decNumber *y, const decNumber *x);
extern decNumber *decNumberPolyLnAlpha(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x);
extern decNumber *decNumberPolyHEn(decNumber *res, const decNumber *y, const decNumber *x);
extern decNumber *decNumberPolyHn(decNumber *res, const decNumber *y, const decNumber *x);

extern decNumber *decNumberBernBn(decNumber *res, const decNumber *n);
extern decNumber *decNumberBernBnS(decNumber *res, const decNumber *n);

extern decNumber *decFactor(decNumber *r, const decNumber *x);

extern decNumber *dn_ln(decNumber *r, const decNumber *x);


extern decNumber *decRecv(decNumber *r, const decNumber *x);

#endif

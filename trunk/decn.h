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

extern decContext *get_ctx(decContext *);
extern decContext *get_ctx64(decContext *);

extern int dn_gt0(const decNumber *x);
extern int dn_lt0(const decNumber *x);
extern int dn_le0(const decNumber *x);

extern void decNumberSwap(decNumber *a, decNumber *b);

extern const decNumber *small_int(int i);
extern void int_to_dn(decNumber *, int, decContext *);
extern int dn_to_int(const decNumber *, decContext *);
extern void ullint_to_dn(decNumber *, unsigned long long int, decContext *);
extern unsigned long long int dn_to_ull(const decNumber *, decContext *, int *);

extern void decNumberPI(decNumber *pi);
extern void decNumberPIon2(decNumber *pion2);
extern int is_int(const decNumber *, decContext *);

extern decNumber *decNumberMAdd(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x, decContext *ctx);

extern decNumber *decNumberRnd(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberRecip(decNumber *r, const decNumber *x, decContext *ctx);

extern decNumber *decNumberFloor(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberCeil(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberTrunc(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberRound(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberFrac(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberSign(decNumber *r, const decNumber *x, decContext *ctx);

extern decNumber *decNumberGCD(decNumber *r, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberLCM(decNumber *r, const decNumber *x, const decNumber *y, decContext *ctx);

extern decNumber *decNumberPow_1(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPow2(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPow10(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberLn1p(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberExpm1(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberLog2(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberLog10(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberLogxy(decNumber *r, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberLamW(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberInvW(decNumber *r, const decNumber *x, decContext *ctx);

extern decNumber *decNumberSquare(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberCube(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *decNumberCubeRoot(decNumber *r, const decNumber *x, decContext *ctx);

extern decNumber *decNumber2Deg(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumber2Rad(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumber2Grad(decNumber *res, const decNumber *x, decContext *);

extern decNumber *decNumberSin(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberCos(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberTan(decNumber *res, const decNumber *x, decContext *);
#if 0
extern decNumber *decNumberSec(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberCosec(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberCot(decNumber *res, const decNumber *x, decContext *);
#endif
extern decNumber *decNumberArcSin(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberArcCos(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberArcTan(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberArcTan2(decNumber *res, const decNumber *x, const decNumber *y, decContext *);
extern decNumber *decNumberSinc(decNumber *res, const decNumber *x, decContext *);

extern decNumber *do_atan2(decNumber *at, const decNumber *ain, const decNumber *b, decContext *ctx);


extern void op_r2p(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_p2r(decimal64 *nul1, decimal64 *nul2, decContext *nulc);

extern decNumber *decNumberSinh(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberCosh(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberTanh(decNumber *res, const decNumber *x, decContext *);
#if 0
extern decNumber *decNumberSech(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberCosech(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberCoth(decNumber *res, const decNumber *x, decContext *);
#endif
extern decNumber *decNumberArcSinh(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberArcCosh(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberArcTanh(decNumber *res, const decNumber *x, decContext *);

/* Elliptic functions */
extern decNumber *decNumberSN(decNumber *res, const decNumber *k, const decNumber *u, decContext *);
extern decNumber *decNumberCN(decNumber *res, const decNumber *k, const decNumber *u, decContext *);
extern decNumber *decNumberDN(decNumber *res, const decNumber *k, const decNumber *u, decContext *);

/* Bessel functions */
extern decNumber *decNumberBSJN(decNumber *res, const decNumber *, const decNumber *u, decContext *);
extern decNumber *decNumberBSIN(decNumber *res, const decNumber *, const decNumber *u, decContext *);
extern decNumber *decNumberBSYN(decNumber *res, const decNumber *, const decNumber *u, decContext *);
extern decNumber *decNumberBSKN(decNumber *res, const decNumber *, const decNumber *u, decContext *);

extern decNumber *decNumberFactorial(decNumber *r, const decNumber *xin, decContext *ctx);
extern decNumber *decNumberDblFactorial(decNumber *r, const decNumber *xin, decContext *ctx);
extern decNumber *decNumberSubFactorial(decNumber *r, const decNumber *xin, decContext *ctx);
extern decNumber *decNumberGamma(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberLnGamma(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberPsi(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberBeta(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberLnBeta(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberZeta(decNumber *res, const decNumber *x, decContext *);

extern const decNumber *const gamma_consts[];
extern const decNumber *const zeta_consts[];
extern const decNumber *const digamma_consts[];

extern decNumber *decNumberERF(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberERFC(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberGammap(decNumber *res, const decNumber *a, const decNumber *x, decContext *ctx);

extern decNumber *decNumberFib(decNumber *res, const decNumber *x, decContext *ctx);

extern decNumber *decNumberD2G(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberD2R(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberG2D(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberG2R(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberR2D(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberR2G(decNumber *res, const decNumber *x, decContext *ctx);

extern void decNumber2Fraction(decNumber *n, decNumber *d, const decNumber *x, decContext *ctx);

extern decNumber *decNumberComb(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberPerm(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);

extern decNumber *decNumberPercent(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPerchg(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPertot(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPerMargin(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberMargin(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNemberPerMRR(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x, decContext *ctx);

extern decNumber *decNumberHMS2HR(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberHR2HMS(decNumber *res, const decNumber *x, decContext *ctx);
extern decNumber *decNumberHMSAdd(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberHMSSub(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);

extern decNumber *decNumberParallel(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberAGM(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);

extern decNumber *decNumberNot(decNumber *res, const decNumber *x, decContext *);
extern decNumber *decNumberAnd(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberOr(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberXor(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberNand(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberNor(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);
extern decNumber *decNumberNxor(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx);


extern void dn_sincos(const decNumber *v, decNumber *sinv, decNumber *cosv, decContext *ctx);
extern void dn_sinhcosh(const decNumber *v, decNumber *sinhv, decNumber *coshv, decContext *ctx);
extern void do_asin(decNumber *, const decNumber *, decContext *);
extern void do_acos(decNumber *, const decNumber *, decContext *);
extern void do_atan(decNumber *, const decNumber *, decContext *);

extern void dn_elliptic(decNumber *sn, decNumber *cn, decNumber *dn, const decNumber *u, const decNumber *k, decContext *ctx);

extern decNumber *set_NaN(decNumber *);
extern decNumber *set_inf(decNumber *);
extern decNumber *set_neginf(decNumber *);

extern void dn_inc(decNumber *, decContext *);
extern void dn_dec(decNumber *, decContext *);

extern void solver_init(decNumber *c, decNumber *, decNumber *, decNumber *, decNumber *, decContext *, unsigned int *);
extern int solver_step(decNumber *, decNumber *, decNumber *, decNumber *, decNumber *, const decNumber *, decContext *, unsigned int *);

extern void solver(unsigned int arg, enum rarg op);

extern decNumber *decNumberPolyPn(decNumber *res, const decNumber *y, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPolyTn(decNumber *res, const decNumber *y, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPolyUn(decNumber *res, const decNumber *y, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPolyLn(decNumber *res, const decNumber *y, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPolyLnAlpha(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPolyHEn(decNumber *res, const decNumber *y, const decNumber *x, decContext *ctx);
extern decNumber *decNumberPolyHn(decNumber *res, const decNumber *y, const decNumber *x, decContext *ctx);

extern decNumber *decNumberBernBn(decNumber *res, const decNumber *n, decContext *);
extern decNumber *decNumberBernBnS(decNumber *res, const decNumber *n, decContext *);

extern decNumber *decFactor(decNumber *r, const decNumber *x, decContext *ctx);

#endif

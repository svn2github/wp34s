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
#include "decn.h"
#include "stats.h"
#include "consts.h"
#include "int.h"


#define sigmaXXY	(Regs[86])

#define sigmaX	(Regs[87])
#define sigmaXX	(Regs[88])
#define sigmaY	(Regs[89])
#define sigmaYY	(Regs[90])
#define sigmaXY	(Regs[91])
#define sigmaN	(Regs[92])

#define sigmalnX	(Regs[93])
#define sigmalnXlnX	(Regs[94])
#define sigmalnY	(Regs[95])
#define sigmalnYlnY	(Regs[96])
#define sigmalnXlnY	(Regs[97])
#define sigmaXlnY	(Regs[98])
#define sigmaYlnX	(Regs[99])

//#define DUMP1
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
#endif


static void correlation(decNumber *, const enum sigma_modes);

static int check_number(const decNumber *r, int n) {
	decNumber s;

	if (dn_lt0(decNumberCompare(&s, r, small_int(n), Ctx))) {
		err(ERR_MORE_POINTS);
		return 1;
	}
	return 0;
}

static int check_data(int n) {
	decNumber r;

	decimal64ToNumber(&sigmaN, &r);
	return check_number(&r, n);
}


void stats_mode_expf(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	State.sigma_mode = SIGMA_EXP;
}

void stats_mode_linf(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	State.sigma_mode = SIGMA_LINEAR;
}

void stats_mode_logf(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	State.sigma_mode = SIGMA_LOG;
}

void stats_mode_pwrf(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	State.sigma_mode = SIGMA_POWER;
}

void stats_mode_best(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	State.sigma_mode = SIGMA_BEST;
}

void sigma_clear(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	sigmaN = CONSTANT_INT(OP_ZERO);
	sigmaX = CONSTANT_INT(OP_ZERO);
	sigmaY = CONSTANT_INT(OP_ZERO);
	sigmaXX = CONSTANT_INT(OP_ZERO);
	sigmaYY = CONSTANT_INT(OP_ZERO);
	sigmaXY = CONSTANT_INT(OP_ZERO);

	sigmalnX = CONSTANT_INT(OP_ZERO);
	sigmalnXlnX = CONSTANT_INT(OP_ZERO);
	sigmalnY = CONSTANT_INT(OP_ZERO);
	sigmalnYlnY = CONSTANT_INT(OP_ZERO);
	sigmalnXlnY = CONSTANT_INT(OP_ZERO);
	sigmaXlnY = CONSTANT_INT(OP_ZERO);
	sigmaYlnX = CONSTANT_INT(OP_ZERO);
}


/* Accumulate sigma data.
 */
static void sigop(decimal64 *r, const decNumber *a, decContext *ctx, decNumber *(*op)(decNumber *, const decNumber *, const decNumber *, decContext *)) {
	decNumber t, u;

	decimal64ToNumber(r, &t);
	(*op)(&u, &t, a, ctx);
	decimal64FromNumber(r, &u, Ctx64);
}


/* Multiply a pair of values and accumulate into the sigma data.
 */
static void mulop(decimal64 *r, const decNumber *a, const decNumber *b, decContext *ctx, decNumber *(*op)(decNumber *, const decNumber *, const decNumber *, decContext *)) {
	decNumber t, u, v;

	decNumberMultiply(&t, a, b, ctx);
	decimal64ToNumber(r, &v);
	(*op)(&u, &v, &t, ctx);
	decimal64FromNumber(r, &u, Ctx64);
}


/* Define a helper function to handle sigma+ and sigma-
 */
static void sigma_helper(decContext *ctx, decNumber *(*op)(decNumber *, const decNumber *, const decNumber *, decContext *)) {
	decNumber x, y;
	decNumber lx, ly;

	getXY(&x, &y);

	sigop(&sigmaN, &const_1, ctx, op);
	sigop(&sigmaX, &x, ctx, op);
	sigop(&sigmaY, &y, ctx, op);
	mulop(&sigmaXX, &x, &x, ctx, op);
	mulop(&sigmaYY, &y, &y, ctx, op);
	mulop(&sigmaXY, &x, &y, ctx, op);

	decNumberSquare(&lx, &x, ctx);
	mulop(&sigmaXXY, &lx, &y, ctx, op);

//	if (State.sigma_mode == SIGMA_LINEAR)
//		return;

	decNumberLn(&lx, &x, ctx);
	decNumberLn(&ly, &y, ctx);

	sigop(&sigmalnX, &lx, ctx, op);
	sigop(&sigmalnY, &ly, ctx, op);
	mulop(&sigmalnXlnX, &lx, &lx, ctx, op);
	mulop(&sigmalnYlnY, &ly, &ly, ctx, op);
	mulop(&sigmalnXlnY, &lx, &ly, ctx, op);
	mulop(&sigmaXlnY, &x, &ly, ctx, op);
	mulop(&sigmaYlnX, &y, &lx, ctx, op);
}

void sigma_plus(decContext *ctx) {
	sigma_helper(ctx, &decNumberAdd);
}

void sigma_minus(decContext *ctx) {
	sigma_helper(ctx, &decNumberSubtract);
}


/* Loop through the various modes and work out
 * which has the highest absolute correlation.
 */
static enum sigma_modes determine_best(const decNumber *n) {
	enum sigma_modes m = SIGMA_LINEAR, i;
	decNumber b, c, d;

	decNumberCompare(&b, &const_2, n, Ctx);
	if (dn_lt0(&b)) {
		correlation(&c, SIGMA_LINEAR);
		decNumberAbs(&b, &c, Ctx);
		for (i=SIGMA_LINEAR+1; i<SIGMA_BEST; i++) {
			correlation(&d, i);

			if (! decNumberIsNaN(&d)) {
				decNumberAbs(&c, &d, Ctx);
				decNumberCompare(&d, &b, &c, Ctx);
				if (dn_lt0(&d)) {
					decNumberCopy(&b, &c);
					m = i;
				}
			}
		}
	}
	return m;
}


/* Return the appropriate variables for the specified fit.
 * If the fit is best, call a routine to figure out which has the highest
 * absolute r.  This entails a recursive call back here.
 */
static void get_sigmas(decNumber *N, decNumber *sx, decNumber *sy, decNumber *sxx, decNumber *syy,
			decNumber *sxy, enum sigma_modes mode) {
	decimal64 *xy;
	int lnx, lny;
	decNumber n;

	decimal64ToNumber(&sigmaN, &n);
	if (mode == SIGMA_BEST)
		mode = determine_best(&n);

	switch (mode) {
	default:			// Linear
		DispMsg = "Linear";
	case SIGMA_QUIET_LINEAR:
		xy = &sigmaXY;
		lnx = lny = 0;
		break;

	case SIGMA_LOG:
		DispMsg = "Log";
		xy = &sigmaYlnX;
		lnx = 1;
		lny = 0;
		break;

	case SIGMA_EXP:
		DispMsg = "Exp";
		xy = &sigmaXlnY;
		lnx = 0;
		lny = 1;
		break;

	case SIGMA_POWER:
		DispMsg = "Power";
	case SIGMA_QUIET_POWER:
		xy = &sigmalnXlnY;
		lnx = lny = 1;
		break;
	}

	if (N != NULL)
		decNumberCopy(N, &n);
	if (sx != NULL)
		decimal64ToNumber(lnx?&sigmalnX:&sigmaX, sx);
	if (sy != NULL)
		decimal64ToNumber(lny?&sigmalnY:&sigmaY, sy);
	if (sxx != NULL)
		decimal64ToNumber(lnx?&sigmalnXlnX:&sigmaXX, sxx);
	if (syy != NULL)
		decimal64ToNumber(lny?&sigmalnYlnY:&sigmaYY, syy);
	if (sxy != NULL)
		decimal64ToNumber(xy, sxy);
}


void sigma_N(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaN;
}

void sigma_X(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaX;
}

void sigma_Y(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaY;
}

void sigma_XX(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaXX;
}

void sigma_YY(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaYY;
}

void sigma_XY(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaXY;
}

void sigma_X2Y(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaXXY;
}

void sigma_lnX(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmalnX;
}

void sigma_lnXlnX(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmalnXlnX;
}

void sigma_lnY(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmalnY;
}

void sigma_lnYlnY(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmalnYlnY;
}

void sigma_lnXlnY(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmalnXlnY;
}

void sigma_XlnY(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaXlnY;
}

void sigma_YlnX(decimal64 *x, decimal64 *y, decContext *ctx64) {
	*x = sigmaYlnX;
}

void sigma_sum(decimal64 *x, decimal64 *y, decContext *ctx64) {
	sigma_X(x, NULL, NULL);
	sigma_Y(y, NULL, NULL);
}


static void mean_common(decimal64 *res, const decNumber *x, const decNumber *n, int exp) {
	decNumber t, u, *p = &t;

	decNumberDivide(&t, x, n, Ctx);
	if (exp)
		decNumberExp(p=&u, &t, Ctx);
	decimal64FromNumber(res, p, Ctx64);
}

void stats_mean(decimal64 *x, decimal64 *y, decContext *ctx64) {
	decNumber N;
	decNumber sx, sy;

	if (check_data(1))
		return;
	get_sigmas(&N, &sx, &sy, NULL, NULL, NULL, SIGMA_QUIET_LINEAR);

	mean_common(x, &sx, &N, 0);
	mean_common(y, &sy, &N, 0);
}


// weighted mean sigmaXY / sigmaY
void stats_wmean(decimal64 *x, decimal64 *nul, decContext *ctx64) {
	decNumber xy, y;

	if (check_data(1))
		return;
	get_sigmas(NULL, NULL, &y, NULL, NULL, &xy, SIGMA_QUIET_LINEAR);

	mean_common(x, &xy, &y, 0);
}

// geometric mean e^(sigmaLnX / N)
void stats_gmean(decimal64 *x, decimal64 *y, decContext *ctx64) {
	decNumber N;
	decNumber sx, sy;

	if (check_data(1))
		return;
	get_sigmas(&N, &sx, &sy, NULL, NULL, NULL, SIGMA_QUIET_POWER);

	mean_common(x, &sx, &N, 1);
	mean_common(y, &sy, &N, 1);
}

// Standard deviations and standard errors
static void do_s(decimal64 *s,
		const decNumber *sxx, const decNumber *sx,
		const decNumber *N, const decNumber *denom, 
		int rootn, int exp) {
	decNumber t, u, v, *p;

	decNumberSquare(&t, sx, Ctx);
	decNumberDivide(&u, &t, N, Ctx);
	decNumberSubtract(&t, sxx, &u, Ctx);
	decNumberDivide(&u, &t, denom, Ctx);
	decNumberSquareRoot(p = &t, &u, Ctx);

	if (rootn) {
		decNumberSquareRoot(&u, N, Ctx);
		decNumberDivide(p = &v, &t, &u, Ctx);
	}
	if (exp) {
		decNumberExp(&u, p, Ctx);
		p = &u;
	}
	decimal64FromNumber(s, p, Ctx64);
}

static void S(decimal64 *x, decimal64 *y, int sample, int rootn, int exp) {
	decNumber N, nm1, *n = &N;
	decNumber sx, sxx, sy, syy;

	if (check_data(2))
		return;
	get_sigmas(&N, &sx, &sy, &sxx, &syy, NULL, exp?SIGMA_QUIET_POWER:SIGMA_QUIET_LINEAR);
	if (sample)
		decNumberSubtract(n = &nm1, &N, &const_1, Ctx);
	do_s(x, &sxx, &sx, &N, n, rootn, exp);
	do_s(y, &syy, &sy, &N, n, rootn, exp);
}

// sx = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / (n-1))
void stats_s(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, 1, 0, 0);
}

// [sigma]x = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / n)
void stats_sigma(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, 0, 0, 0);
}

// serr = sx / sqrt(n)
void stats_SErr(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, 1, 1, 0);
}


// sx = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / (n-1))
void stats_gs(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, 1, 0, 1);
}

// [sigma]x = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / n)
void stats_gsigma(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, 0, 0, 1);
}

// serr = sx / sqrt(n)
void stats_gSErr(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, 1, 1, 1);
}


// Weighted standard deviation
void WS(decimal64 *x, int sample, int rootn) {
	decNumber sxxy, sy, sxy, syy;
	decNumber t, u, v, w, *p;

	get_sigmas(NULL, NULL, &sy, NULL, &syy, &sxy, SIGMA_QUIET_LINEAR);
	if (check_number(&sy, 2))
		return;
	decimal64ToNumber(&sigmaXXY, &sxxy);

	decNumberMultiply(&t, &sy, &sxxy, Ctx);
	decNumberSquare(&u, &sxy, Ctx);
	decNumberSubtract(&v, &t, &u, Ctx);
	decNumberSquare(p = &t, &sy, Ctx);
	if (sample)
		decNumberSubtract(p = &u, &t, &syy, Ctx);
	decNumberDivide(&w, &v, p, Ctx);
	decNumberSquareRoot(p = &u, &w, Ctx);
	if (rootn) {
		decNumberSquareRoot(&t, &sy, Ctx);
		decNumberDivide(p = &v, &u, &t, Ctx);
	}
	decimal64FromNumber(x, p, Ctx64);
}

void stats_ws(decimal64 *x, decimal64 *y, decContext *ctx64) {
	WS(x, 1, 0);
}

void stats_wsigma(decimal64 *x, decimal64 *y, decContext *ctx64) {
	WS(x, 0, 0);
}

void stats_wSErr(decimal64 *x, decimal64 *y, decContext *ctx64) {
	WS(x, 1, 1);
}


decNumber *stats_sigper(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber sx, t;

	get_sigmas(NULL, &sx, NULL, NULL, NULL, NULL, SIGMA_QUIET_LINEAR);
	decNumberDivide(&t, x, &sx, ctx);
	return decNumberMultiply(res, &t, &const_100, ctx);
}

/* Calculate the correlation based on the stats data using the
 * specified model.
 */
static void correlation(decNumber *t, const enum sigma_modes m) {
	decNumber N, u, v, w;
	decNumber sx, sy, sxx, syy, sxy;

	get_sigmas(&N, &sx, &sy, &sxx, &syy, &sxy, m);

	decNumberMultiply(t, &N, &sxx, Ctx);
	decNumberSquare(&u, &sx, Ctx);
	decNumberSubtract(&v, t, &u, Ctx);
	decNumberMultiply(t, &N, &syy, Ctx);
	decNumberSquare(&u, &sy, Ctx);
	decNumberSubtract(&w, t, &u, Ctx);
	decNumberMultiply(t, &v, &w, Ctx);
	decNumberSquareRoot(&w, t, Ctx);
	decNumberMultiply(t, &N, &sxy, Ctx);
	decNumberMultiply(&u, &sx, &sy, Ctx);
	decNumberSubtract(&v, t, &u, Ctx);
	decNumberDivide(t, &v, &w, Ctx);

	decNumberCompare(&u, &const_1, t, Ctx);
	if (decNumberIsNegative(&u))
		decNumberCopy(t, &const_1);
	else {
		decNumberCompare(&u, t, &const__1, Ctx);
		if (decNumberIsNegative(&u))
			decNumberCopy(t, &const__1);
	}
}


void stats_correlation(decimal64 *r, decimal64 *nul, decContext *ctx64) {
	decNumber t;

	if (check_data(2))
		return;
	correlation(&t, State.sigma_mode);
	decimal64FromNumber(r, &t, ctx64);
}


static void covariance(decimal64 *r, int sample) {
	decNumber N, t, u, v;
	decNumber sx, sy, sxy;

	if (check_data(2))
		return;
	get_sigmas(&N, &sx, &sy, NULL, NULL, &sxy, State.sigma_mode);
	decNumberMultiply(&t, &sx, &sy, Ctx);
	decNumberDivide(&u, &t, &N, Ctx);
	decNumberSubtract(&t, &sxy, &u, Ctx);
	if (sample) {
		decNumberSubtract(&v, &N, &const_1, Ctx);
		decNumberDivide(&u, &t, &v, Ctx);
	} else
		decNumberDivide(&u, &t, &N, Ctx);
	decimal64FromNumber(r, &u, Ctx64);
}

void stats_COV(decimal64 *r, decimal64 *nul, decContext *ctx64) {
	covariance(r, 0);
}

void stats_Sxy(decimal64 *r, decimal64 *nul, decContext *ctx64) {
	covariance(r, 1);
}

// y = B . x + A
static void do_LR(decNumber *B, decNumber *A, decContext *ctx) {
	decNumber N, u, v, denom;
	decNumber sx, sy, sxx, sxy;

	get_sigmas(&N, &sx, &sy, &sxx, NULL, &sxy, State.sigma_mode);

	decNumberMultiply(B, &N, &sxx, ctx);
	decNumberSquare(&u, &sx, ctx);
	decNumberSubtract(&denom, B, &u, ctx);

	decNumberMultiply(B, &N, &sxy, ctx);
	decNumberMultiply(&u, &sx, &sy, ctx);
	decNumberSubtract(&v, B, &u, ctx);
	decNumberDivide(A, &v, &denom, ctx);

	decNumberMultiply(B, &sxx, &sy, ctx);
	decNumberMultiply(&u, &sx, &sxy, ctx);
	decNumberSubtract(&v, B, &u, ctx);
	decNumberDivide(B, &v, &denom, ctx);
}


void stats_LR(decimal64 *bout, decimal64 *aout, decContext *ctx64) {
	decNumber a, b;

	if (check_data(2))
		return;
	do_LR(&b, &a, Ctx);
	decimal64FromNumber(aout, &a, ctx64);
	decimal64FromNumber(bout, &b, ctx64);
}


decNumber *stats_xhat(decNumber *res, const decNumber *y, decContext *ctx) {
	decNumber a, b, t;

	if (check_data(2))
		return NULL;
	do_LR(&b, &a, ctx);
	decNumberSubtract(&t, y, &b, ctx);
	decNumberDivide(res, &a, &t, ctx);
	return res;
}


decNumber *stats_yhat(decNumber *res, const decNumber *x, decContext *ctx) {
	decNumber a, b, t;

	if (check_data(2))
		return NULL;
	do_LR(&b, &a, ctx);
	decNumberMultiply(&t, x, &a, ctx);
	decNumberAdd(res, &t, &b, ctx);
	return res;
}


/* rng/taus.c from the GNU Scientific Library.
 * The period of this generator is about 2^88.
 */
static unsigned long int taus_get(void) {
#define MASK 0xffffffffUL
#define TAUSWORTHE(s,a,b,c,d) (((s & c) << d) & MASK) ^ ((((s << a) & MASK) ^ s) >> b)

  RandS1 = TAUSWORTHE (RandS1, 13, 19, 4294967294UL, 12);
  RandS2 = TAUSWORTHE (RandS2,  2, 25, 4294967288UL, 4);
  RandS3 = TAUSWORTHE (RandS3,  3, 11, 4294967280UL, 17);

  return RandS1 ^ RandS2 ^ RandS3;
}

static void taus_seed(unsigned long int s) {
	int i;

	if (s == 0)
		s = 1;
#define LCG(n) ((69069 * n) & 0xffffffffUL)
	RandS1 = LCG (s);
	if (RandS1 < 2) RandS1 += 2UL;

	RandS2 = LCG (RandS1);
	if (RandS2 < 8) RandS2 += 8UL;

	RandS3 = LCG (RandS2);
	if (RandS3 < 16) RandS3 += 16UL;
#undef LCG

	for (i=0; i<6; i++)
		taus_get();
}

void stats_random(decimal64 *r, decimal64 *nul, decContext *ctx64) {
	// Start by generating the next in sequence
	unsigned long int s;
	decNumber y, z;

	if (RandS1 == 0 && RandS2 == 0 && RandS3 == 0)
		taus_seed(0);
	s = taus_get();

	// Now build ourselves a number
	if (is_intmode())
		d64fromInt(r, build_value(s, 0));
	else {
		ullint_to_dn(&z, s, Ctx);
		decNumberMultiply(&y, &z, &const_randfac, Ctx);
		decimal64FromNumber(r, &y, ctx64);
	}
}


void stats_sto_random(decimal64 *nul1, decimal64 *nul2, decContext *ctx) {
	unsigned long int s;
	int z;
	decNumber x;

	if (is_intmode()) {
		 s = d64toInt(&regX) & 0xffffffff;
	} else {
		getX(&x);
		s = (unsigned long int) dn_to_ull(&x, Ctx64, &z);
	}
	taus_seed(s);
}


#ifndef TINY_BUILD
static void check_low(decNumber *d, decContext *ctx) {
	decNumber t, u;

	decNumberAbs(&t, d, ctx);
	decNumberCompare(&u, &t, &const_1e_32, ctx);
	if (decNumberIsNegative(&u))
		decNumberCopy(d, &const_1e_32);
}


static void ib_step(decNumber *d, decNumber *c, const decNumber *aa, decContext *ctx) {
	decNumber t, u;

	decNumberMultiply(&t, aa, d, ctx);
	decNumberAdd(&u, &const_1, &t, ctx);	// d = 1+aa*d
	check_low(&u, ctx);
	decNumberRecip(d, &u, ctx);
	decNumberDivide(&t, aa, c, ctx);
	decNumberAdd(c, &const_1, &t, ctx);	// c = 1+aa/c
	check_low(c, ctx);
}


static void betacf(decNumber *r, const decNumber *a, const decNumber *b, const decNumber *x, decContext *ctx) {
	decNumber aa, c, d, apb, am1, ap1, m, m2, oldr;
	int i;
	decNumber t, u, v, w;

	decNumberAdd(&ap1, a, &const_1, ctx);		// ap1 = 1+a
	decNumberSubtract(&am1, a, &const_1, ctx);	// am1 = a-1
	decNumberAdd(&apb, a, b, ctx);			// apb = a+b
	decNumberCopy(&c, &const_1);			// c = 1
	decNumberDivide(&t, x, &ap1, ctx);
	decNumberMultiply(&u, &t, &apb, ctx);
	decNumberSubtract(&t, &const_1, &u, ctx);	// t = 1-apb*x/ap1
	check_low(&t, ctx);
	decNumberRecip(&d, &t, ctx);			// d = 1/t
	decNumberCopy(r, &d);				// res = d
	decNumberZero(&m);
	for (i=0; i<500; i++) {
		decNumberCopy(&oldr, r);
		dn_inc(&m, ctx);			// m = i+1
		decNumberMultiply(&m2, &m, &const_2, ctx);
		decNumberSubtract(&t, b, &m, ctx);
		decNumberMultiply(&u, &t, &m, ctx);
		decNumberMultiply(&t, &u, x, ctx);	// t = m*(b-m)*x
		decNumberAdd(&u, &am1, &m2, ctx);
		decNumberAdd(&v, a, &m2, ctx);
		decNumberMultiply(&w, &u, &v, ctx);	// w = (am1+m2)*(a+m2)
		decNumberDivide(&aa, &t, &w, ctx);	// aa = t/w
		ib_step(&d, &c, &aa, ctx);
		decNumberMultiply(&t, r, &d, ctx);
		decNumberMultiply(r, &t, &c, ctx);	// r = r*d*c
		decNumberAdd(&t, a, &m, ctx);
		decNumberAdd(&u, &apb, &m, ctx);
		decNumberMultiply(&w, &t, &u, ctx);
		decNumberMultiply(&t, &w, x, ctx);
		decNumberMinus(&w, &t, ctx);		// w = -(a+m)*(apb+m)*x
		decNumberAdd(&t, a, &m2, ctx);
		decNumberAdd(&u, &ap1, &m2, ctx);
		decNumberMultiply(&v, &t, &u, ctx);	// v = (a+m2)*(ap1+m2)
		decNumberDivide(&aa, &w, &v, ctx);	// aa = w/v
		ib_step(&d, &c, &aa, ctx);
		decNumberMultiply(&v, &d, &c, ctx);
		decNumberMultiply(r, r, &v, ctx);	// r *= d*c
		decNumberCompare(&u, &oldr, r, ctx);
		if (decNumberIsZero(&u))
			break;
	}
}
#endif

/* Regularised incomplete beta function Ix(a, b)
 */
decNumber *betai(decNumber *r, const decNumber *a, const decNumber *b, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, u, v, w, y;
	int limit = 0;

	decNumberCompare(&t, &const_1, x, ctx);
	if (decNumberIsNegative(x) || decNumberIsNegative(&t)) {
		return set_NaN(r);
	}
	if (decNumberIsZero(x) || decNumberIsZero(&t))
		limit = 1;
	else {
		decNumberLnBeta(&u, a, b, ctx);
		decNumberLn(&v, x, ctx);		// v = ln(x)
		decNumberMultiply(&t, a, &v, ctx);
		decNumberSubtract(&v, &t, &u, ctx);	// v = lng(...)+a.ln(x)
		decNumberSubtract(&y, &const_1, x, ctx);// y = 1-x
		decNumberLn(&u, &y, ctx);		// u = ln(1-x)
		decNumberMultiply(&t, &u, b, ctx);
		decNumberAdd(&u, &t, &v, ctx);		// u = lng(...)+a.ln(x)+b.ln(1-x)
		decNumberExp(&w, &u, ctx);
	}
	decNumberAdd(&v, a, b, ctx);
	decNumberAdd(&u, &v, &const_2, ctx);		// u = a+b+2
	decNumberAdd(&t, a, &const_1, ctx);		// t = a+1
	decNumberDivide(&v, &t, &u, ctx);		// u = (a+1)/(a+b+2)
	decNumberCompare(&t, x, &v, ctx);
	if (decNumberIsNegative(&t)) {
		if (limit)
			return decNumberZero(r);
		betacf(&t, a, b, x, ctx);
		decNumberDivide(&u, &t, a, ctx);
		return decNumberMultiply(r, &w, &u, ctx);
	} else {
		if (limit)
			return decNumberCopy(r, &const_1);
		betacf(&t, b, a, &y, ctx);
		decNumberDivide(&u, &t, b, ctx);
		decNumberMultiply(&t, &w, &u, ctx);
		return decNumberSubtract(r, &const_1, &t, ctx);
	}
#else
	return NULL;
#endif
}


#ifndef TINY_BUILD
static int check_probability(decNumber *r, const decNumber *x, decContext *ctx, int min_zero) {
	decNumber t;

	/* Range check the probability input */
	if (decNumberIsZero(x)) {
	    if (min_zero)
		decNumberCopy(r, &const_0);
	    else
		set_neginf(r);
	    return 1;
	}
	decNumberCompare(&t, &const_1, x, ctx);
	if (decNumberIsZero(&t)) {
	    set_inf(r);
	    return 1;
	}
	if (decNumberIsNegative(&t) || decNumberIsNegative(x) || decNumberIsSpecial(x)) {
	    set_NaN(r);
	    return 1;
	}
	return 0;
}


/* Get parameters for a distribution */
static void dist_one_param(decNumber *a) {
	get_reg_n_as_dn(regJ_idx, a);
}

static void dist_two_param(decNumber *a, decNumber *b) {
	get_reg_n_as_dn(regJ_idx, a);
	get_reg_n_as_dn(regK_idx, b);
}

static int param_verify(decNumber *r, const decNumber *n, int zero, int intg) {
	if (decNumberIsSpecial(n) ||
			dn_le0(n) ||
			(!zero && decNumberIsZero(n)) ||
			(intg && !is_int(n, Ctx))) {
		decNumberZero(r);
		Error = ERR_BAD_PARAM;
		return 1;
	}
	return 0;
}
#define param_positive(r, n)		(param_verify(r, n, 0, 0))
#define param_positive_int(r, n)	(param_verify(r, n, 0, 1))
#define param_nonnegative(r, n)		(param_verify(r, n, 1, 0))
#define param_nonnegative_int(r, n)	(param_verify(r, n, 1, 1))

static int param_range01(decNumber *r, const decNumber *p) {
	decNumber h;

	decNumberCompare(&h, &const_1, p, Ctx);
	if (decNumberIsSpecial(p) || dn_lt0(p) || dn_lt0(&h)) {
		decNumberZero(r);
		err(ERR_BAD_PARAM);
		return 1;
	}
	return 0;
}


// Ln 1 + (cdf(x)-p)/p
static int qf_eval(decNumber *fx, const decNumber *x, const decNumber *p, decContext *ctx,
		decNumber *(*f)(decNumber *, const decNumber *, decContext *)) {
	decNumber a, b;

	(*f)(&a, x, ctx);
	decNumberSubtract(&b, &a, p, ctx);
	if (decNumberIsZero(&b))
		return 0;
	decNumberDivide(&a, &b, p, ctx);
	decNumberLn1p(fx, &a, ctx);
	return 1;
}
#endif

static decNumber *qf_search(decNumber *r,
				const decNumber *x, decContext *ctx, int min_zero,
				const decNumber *samp_low, const decNumber *samp_high,
		decNumber *(*f)(decNumber *, const decNumber *, decContext *)) {
#ifndef TINY_BUILD
	decNumber t, u, v, tv, uv, vv, a, oldv;
	unsigned int flags = 0;

	if (check_probability(r, x, ctx, min_zero))
	    return r;

	// Evaluate the first two points which are given to us.
	decNumberCopy(&t, samp_low);
	if (qf_eval(&tv, &t, x, ctx, f) == 0)
		return decNumberCopy(r, &t);
	if (Error == ERR_BAD_PARAM) {
		decNumberZero(r);
		return r;
	}

	decNumberCopy(&u, samp_high);
	if (qf_eval(&uv, &u, x, ctx, f) == 0)
		return decNumberCopy(r, &u);

	solver_init(&v, &t, &u, &tv, &uv, ctx, &flags);
	set_NaN(&oldv);
	do {
		// If we got below the minimum, do a bisection step instead
		if (min_zero && dn_le0(&v)) {
			decNumberMin(&a, &t, &u, ctx);
			decNumberMultiply(&v, &a, &const_0_5, ctx);
		}
		if (qf_eval(&vv, &v, x, ctx, f) == 0 || decNumberIsNaN(&vv))
			break;
		if (relative_error(&v, &oldv, &const_1e_24, ctx))
			break;
		decNumberCopy(&oldv, &v);
	} while (solver_step(&t, &u, &v, &tv, &uv, &vv, ctx, &flags, &relative_error) == 0);

	return decNumberCopy(r, &v);
#else
	return NULL;
#endif
}


/* Evaluate Ln(1 - x) accurately
 */
decNumber *dn_ln1m(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber a;
	decNumberMinus(&a, x, ctx);
	return decNumberLn1p(r, &a, ctx);
}


// Normal(0,1) PDF
// 1/sqrt(2 PI) . exp(-x^2/2)
decNumber *pdf_Q(decNumber *q, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber r, t;

	decNumberSquare(&t, x, ctx);
	decNumberMultiply(&r, &t, &const_0_5, ctx);
	decNumberMinus(&t, &r, ctx);
	decNumberExp(&r, &t, ctx);
	return decNumberMultiply(q, &r, &const_recipsqrt2PI, ctx);
#else
	return NULL;
#endif
}

// Normal(0,1) CDF function
#ifndef TINY_BUILD
decNumber *cdf_Q_helper(decNumber *q, decNumber *pdf, const decNumber *x, decContext *ctx) {
	decNumber t, u, v, a, x2, d, absx, n;
	int i;

	pdf_Q(pdf, x, ctx);
	decNumberAbs(&absx, x, ctx);
	decNumberCompare(&u, &const_PI, &absx, ctx);	// We need a number about 3.2 and this is close enough
	if (decNumberIsNegative(&u)) {
		decNumberMinus(&x2, &absx, ctx);
		//n = ceil(5 + k / (|x| - 1))
		decNumberSubtract(&v, &absx, &const_1, ctx);
		decNumberDivide(&t, &const_256, &v, ctx);
		decNumberAdd(&u, &t, &const_4, ctx);
		decNumberCeil(&n, &u, ctx);
		decNumberZero(&t);
		do {
			decNumberAdd(&u, x, &t, ctx);
			decNumberDivide(&t, &n, &u, ctx);
			dn_dec(&n, ctx);
		} while (! decNumberIsZero(&n));

		decNumberAdd(&u, &t, x, ctx);
		decNumberDivide(q, pdf, &u, ctx);
		if (! decNumberIsNegative(q))
			decNumberSubtract(q, &const_1, q, ctx);
		if (decNumberIsNegative(x))
			decNumberMinus(q, q, ctx);
		return q;
	} else {
		decNumberSquare(&x2, &absx, ctx);
		decNumberCopy(&t, &absx);
		decNumberCopy(&a, &absx);
		decNumberCopy(&d, &const_3);
		for (i=0;i<500; i++) {
			decNumberMultiply(&u, &t, &x2, ctx);
			decNumberDivide(&t, &u, &d, ctx);
			decNumberAdd(&u, &a, &t, ctx);
			decNumberCompare(&v, &u, &a, ctx);
			if (decNumberIsZero(&v))
				break;
			decNumberCopy(&a, &u);
			decNumberAdd(&d, &d, &const_2, ctx);
		}
		decNumberMultiply(&v, &a, pdf, ctx);
		if (decNumberIsNegative(x))
			return decNumberSubtract(q, &const_0_5, &v, ctx);
		return decNumberAdd(q, &const_0_5, &v, ctx);
	}
}
#endif

decNumber *cdf_Q(decNumber *q, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t;
	return cdf_Q_helper(q, &t, x, ctx);
#else
	return NULL;
#endif
}


#ifndef TINY_BUILD
static void qf_Q_est(decNumber *est, const decNumber *x, const decNumber *x05, decContext *ctx) {
	const int invert = decNumberIsNegative(x05);
	decNumber a, b, u, xc;

	if (invert) {
		decNumberSubtract(&xc, &const_1, x, ctx);
		x = &xc;
	}

	decNumberCompare(&a, x, &const_0_2, ctx);
	if (decNumberIsNegative(&a)) {
		decNumberLn(&a, x, ctx);
		decNumberMultiply(&u, &a, &const__2, ctx);
		decNumberSubtract(&a, &u, &const_1, ctx);
		decNumberSquareRoot(&b, &a, ctx);
		decNumberMultiply(&a, &b, &const_sqrt2PI, ctx);
		decNumberMultiply(&b, &a, x, ctx);
		decNumberLn(&a, &b, ctx);
		decNumberMultiply(&b, &a, &const__2, ctx);
		decNumberSquareRoot(&a, &b, ctx);
		decNumberDivide(&b, &const_0_2, &u, ctx);
		decNumberAdd(est, &a, &b, ctx);
		if (!invert)
			decNumberMinus(est, est, ctx);
	} else {
		decNumberMultiply(&a, &const_sqrt2PI, x05, ctx);
		decNumberCube(&b, &a, ctx);
		decNumberDivide(&u, &b, &const_6, ctx);
		decNumberAdd(est, &u, &a, ctx);
		decNumberMinus(est, est, ctx);
	}
}
#endif

decNumber *qf_Q(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, t, cdf, pdf;
	int i;


	if (check_probability(r, x, ctx, 0))
		return r;
	decNumberSubtract(&b, &const_0_5, x, ctx);
	if (decNumberIsZero(&b)) {
		decNumberZero(r);
		return r;
	}

	qf_Q_est(r, x, &b, ctx);
	for (i=0; i<10; i++) {
		cdf_Q_helper(&cdf, &pdf, r, ctx);
		decNumberSubtract(&a, &cdf, x, ctx);
		decNumberDivide(&t, &a, &pdf, ctx);
		decNumberMultiply(&a, &t, r, ctx);
		decNumberMultiply(&b, &a, &const_0_5, ctx);
		decNumberSubtract(&a, &b, &const_1, ctx);
		decNumberDivide(&b, &t, &a, ctx);
		decNumberAdd(&a, &b, r, ctx);
		if (relative_error(&a, r, &const_1e_32, ctx))
			break;
		decNumberCopy(r, &a);
	}
	return decNumberCopy(r, &a);
#else
	return NULL;
#endif
}


// Pv(x) = (x/2)^(v/2) . exp(-x/2) / Gamma(v/2+1) . (1 + sum(x^k/(v+2)(v+4)..(v+2k))
decNumber *cdf_chi2(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, v;

	dist_one_param(&v);
	if (param_positive_int(r, &v))
		return r;
	if (decNumberIsNaN(x)) {
		return set_NaN(r);
	}
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberMultiply(&a, &v, &const_0_5, ctx);
	decNumberMultiply(&b, x, &const_0_5, ctx);
	return decNumberGammap(r, &a, &b, ctx);
#else
	return NULL;
#endif
}

decNumber *qf_chi2(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, c, q, v;

	dist_one_param(&v);
	if (param_positive_int(r, &v))
		return r;
	if (decNumberIsNaN(x)) {
		return set_NaN(r);
	}
	decNumberMultiply(&a, &v, &const_2, ctx);
	decNumberSubtract(&b, &a, &const_1, ctx);
	decNumberSquareRoot(&a, &b, ctx);
	qf_Q(&q, x, ctx);
	decNumberAdd(&c, &q, &a, ctx);
	decNumberSquare(&b, &c, ctx);
	decNumberMultiply(&a, &b, &const_0_25, ctx);	// lower estimate
	decNumberMultiply(&b, &a, &const_e, ctx);
	return qf_search(r, x, ctx, 1, &a, &b, &cdf_chi2);
#else
	return NULL;
#endif
}

static int t_param(decNumber *r, decNumber *v, const decNumber *x, decContext *ctx) {
	dist_one_param(v);
	if (param_positive(r, v))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}


decNumber *cdf_T(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, u, z, v;
	int invert;

	if (t_param(r, &v, x, ctx))
		return r;
	if (decNumberIsInfinite(x)) {
		if (decNumberIsNegative(x))
			return decNumberZero(r);
		return decNumberCopy(r, &const_1);
	}
	if (decNumberIsInfinite(&v))			// Normal in the limit
		return cdf_Q(r, x, ctx);
	if (decNumberIsZero(x))
		return decNumberCopy(r, &const_0_5);
	invert = ! decNumberIsNegative(x);
	decNumberSquare(&t, x, ctx);
	decNumberAdd(&u, &t, &v, ctx);
	decNumberDivide(&t, &v, &u, ctx);
	decNumberMultiply(&u, &v, &const_0_5, ctx);
	betai(&z, &u, &const_0_5, &t, ctx);
	decNumberMultiply(r, &const_0_5, &z, ctx);
	if (invert)
		decNumberSubtract(r, &const_1, r, ctx);
	return r;
#else
	return NULL;
#endif
}

#ifndef TINY_BUILD
static void qf_T_est(decNumber *r, const decNumber *df, const decNumber *p, const decNumber *p05, decContext *ctx) {
	const int invert = decNumberIsNegative(p05);
	int negate;
	decNumber a, b, u, pc, pc05, x, x2, x3;

	if (invert) {
		decNumberSubtract(&pc, &const_1, p, ctx);
		p = &pc;
		decNumberMinus(&pc05, p05, ctx);
		p05 = &pc05;
	}
	decNumberLn(&a, p, ctx);
	decNumberMinus(&a, &a, ctx);
	decNumberMultiply(&b, df, &const_1_7, ctx);
	if (dn_lt0(decNumberCompare(&u, &a, &b, ctx))) {
		qf_Q_est(&x, p, p05, ctx);
		decNumberSquare(&x2, &x, ctx);
		decNumberMultiply(&x3, &x2, &x, ctx);
		decNumberAdd(&a, &x, &x3, ctx);
		decNumberMultiply(&b, &a, &const_0_25, ctx);
		decNumberDivide(&a, &b, df, ctx);
		decNumberAdd(r, &a, &x, ctx);

		decNumberDivide(&a, &x2, &const_3, ctx);
		dn_inc(&a, ctx);
		decNumberMultiply(&b, &a, &x3, ctx);
		decNumberMultiply(&a, &b, &const_0_25, ctx);
		decNumberSquare(&b, df, ctx);
		decNumberDivide(&u, &a, &b, ctx);
		decNumberAdd(r, r, &u, ctx);
		negate = invert;
	} else {
		decNumberMultiply(&x2, df, &const_2, ctx);
		decNumberSubtract(&b, &x2, &const_1, ctx);
		decNumberDivide(&a, &const_PI, &b, ctx);
		decNumberSquareRoot(&b, &a, ctx);
		decNumberMultiply(&a, &b, &x2, ctx);
		decNumberMultiply(&b, &a, p, ctx);
		decNumberRecip(&a, df, ctx);
		decNumberPower(&u, &b, &a, ctx);
		decNumberSquareRoot(&a, df, ctx);
		decNumberDivide(r, &a, &u, ctx);
		negate = !invert;
	}
	if (negate)
		decNumberMinus(r, r, ctx);
}
#endif

decNumber *qf_T(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, c, d, v;

	if (t_param(r, &v, x, ctx))
		return r;
	decNumberSubtract(&b, &const_0_5, x, ctx);
	if (decNumberIsZero(&b)) {
		return decNumberZero(r);
	}
	if (decNumberIsInfinite(&v))					// Normal in the limit
		return qf_Q(r, x, ctx);

	decNumberCompare(&a, &v, &const_1, ctx);
	if (decNumberIsZero(&a)) {					// special case v = 1
		decNumberMultiply(&a, &b, &const_PI, ctx);
		dn_sincos(&a, &c, &d, ctx);
		decNumberDivide(&a, &c, &d, ctx);			// lower = tan(pi (x - 1/2))
		return decNumberMinus(r, &a, ctx);
	}
	decNumberCompare(&d, &v, &const_2, ctx);			// special case v = 2
	if (decNumberIsZero(&d)) {
		decNumberSubtract(&a, &const_1, x, ctx);
		decNumberMultiply(&c, &a, x, ctx);
		decNumberMultiply(&d, &c, &const_4, ctx);		// alpha = 4p(1-p)

		decNumberDivide(&c, &const_2, &d, ctx);
		decNumberSquareRoot(&a, &c, ctx);
		decNumberMultiply(&c, &a, &b, ctx);
		decNumberMultiply(r, &c, &const__2, ctx);
	}

	// common case v >= 3
	qf_T_est(&c, &v, x, &b, ctx);
	decNumberDivide(&b, &c, &const_0_9, ctx);
	decNumberMultiply(&a, &c, &const_0_9, ctx);

	return qf_search(r, x, ctx, 0, &a, &b, &cdf_T);
#else
	return NULL;
#endif
}
	
decNumber *cdf_F(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, u, w, v1, v2;

	dist_two_param(&v1, &v2);
	if (param_positive(r, &v1) || param_positive(r, &v2))
		return r;
	if (decNumberIsNaN(x)) {
		return set_NaN(r);
	}
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberMultiply(&t, &v1, x, ctx);
	decNumberAdd(&u, &t, &v2, ctx);			// u = v1 * x + v2
	decNumberDivide(&w, &t, &u, ctx);		// w = (v1 * x) / (v1 * x + v2)
	decNumberMultiply(&t, &v1, &const_0_5, ctx);
	decNumberMultiply(&u, &v2, &const_0_5, ctx);
	return betai(r, &t, &u, &w, ctx);
#else
	return NULL;
#endif
}

decNumber *qf_F(decNumber *r, const decNumber *x, decContext *ctx) {
	if (decNumberIsZero(x))
		return decNumberZero(r);
	// MORE: provide reasonable initial estaimtes
	return qf_search(r, x, ctx, 1, &const_1e_10, &const_20, &cdf_F);
}


/* Weibull distribution cdf = 1 - exp(-(x/lambda)^k)
 */
#ifndef TINY_BUILD
static int weibull_param(decNumber *r, decNumber *k, decNumber *lam, const decNumber *x, decContext *ctx) {
	dist_two_param(k, lam);
	if (param_positive(r, k) || param_positive(r, lam))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}
#endif

decNumber *pdf_WB(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber k, lam, t, u, v, q;

	if (weibull_param(r, &k, &lam, x, ctx))
		return r;
	if (dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	decNumberDivide(&q, x, &lam, ctx);
	decNumberPower(&u, &q, &k, ctx);		// (x/lam)^k
	decNumberDivide(&t, &u, &q, ctx);		// (x/lam)^(k-1)
	decNumberExp(&v, &u, ctx);
	decNumberDivide(&q, &t, &v, ctx);
	decNumberDivide(&t, &q, &lam, ctx);
	return decNumberMultiply(r, &t, &k, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_WB(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber k, lam, t;

	if (weibull_param(r, &k, &lam, x, ctx))
		return r;
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberDivide(&t, x, &lam, ctx);
	decNumberPower(&lam, &t, &k, ctx);
	decNumberMinus(&t, &lam, ctx);
	decNumberExpm1(&lam, &t, ctx);
	return decNumberMinus(r, &lam, ctx);
#else
	return NULL;
#endif
}


/* Weibull distribution quantile function:
 *	p = 1 - exp(-(x/lambda)^k)
 *	exp(-(x/lambda)^k) = 1 - p
 *	-(x/lambda)^k = ln(1-p)
 * Thus, the qf is:
 *	x = (-ln(1-p) ^ (1/k)) * lambda
 * So no searching is required.
 */
decNumber *qf_WB(decNumber *r, const decNumber *p, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, u, k, lam;

	if (weibull_param(r, &k, &lam, p, ctx))
		return r;
	if (check_probability(r, p, ctx, 1))
	    return r;
	decNumberSubtract(&t, &const_1, p, ctx);
	if (decNumberIsNaN(p) || decNumberIsSpecial(&lam) || decNumberIsSpecial(&k) ||
			dn_le0(&k) || dn_le0(&lam)) {
		return set_NaN(r);
	}

	decNumberLn(&u, &t, ctx);
	decNumberMinus(&t, &u, ctx);
	decNumberRecip(&u, &k, ctx);
	decNumberPower(&k, &t, &u, ctx);
	return decNumberMultiply(r, &lam, &k, ctx);
#else
	return NULL;
#endif
}


/* Exponential distribution cdf = 1 - exp(-lambda . x)
 */
#ifndef TINY_BUILD
static int exponential_xform(decNumber *r, decNumber *lam, const decNumber *x, decContext *ctx) {
	dist_one_param(lam);
	if (param_positive(r, lam))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}
#endif

decNumber *pdf_EXP(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber lam, t, u;

	if (exponential_xform(r, &lam, x, ctx))
		return r;
	if (dn_lt0(x)) {
		set_NaN(r);
		return r;
	}
	decNumberMultiply(&t, &lam, x, ctx);
	decNumberMinus(&u, &t, ctx);
	decNumberExp(&t, &u, ctx);
	return decNumberMultiply(r, &t, &lam, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_EXP(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber lam, t, u;

	if (exponential_xform(r, &lam, x, ctx))
		return r;
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberMultiply(&t, &lam, x, ctx);
	decNumberMinus(&u, &t, ctx);
	decNumberExpm1(&t, &u, ctx);
	return decNumberMinus(r, &t, ctx);
#else
	return NULL;
#endif
}


/* Exponential distribution quantile function:
 *	p = 1 - exp(-lambda . x)
 *	exp(-lambda . x) = 1 - p
 *	-lambda . x = ln(1 - p)
 * Thus, the quantile function is:
 *	x = ln(1-p)/-lambda
 */
decNumber *qf_EXP(decNumber *r, const decNumber *p, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, u, lam;

	dist_one_param(&lam);
	if (param_positive(r, &lam))
		return r;
	if (check_probability(r, p, ctx, 1))
	    return r;
	if (decNumberIsNaN(p) || decNumberIsSpecial(&lam) || dn_le0(&lam)) {
		return set_NaN(r);
	}

//	decNumberMinus(&t, p, ctx);
//	decNumberLn1p(&u, &t, ctx);
	dn_ln1m(&u, p, ctx);
	decNumberDivide(&t, &u, &lam, ctx);
	return decNumberMinus(r, &t, ctx);
#else
	return NULL;
#endif
}

/* Binomial cdf f(k; n, p) = iBeta(n-floor(k), 1+floor(k); 1-p)
 */
#ifndef TINY_BUILD
static int binomial_param(decNumber *r, decNumber *p, decNumber *n, const decNumber *x, decContext *ctx) {
	dist_two_param(p, n);
	if (param_nonnegative_int(r, n) || param_range01(r, p))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}
#endif

decNumber *cdf_B_helper(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber n, p, t, u, v;

	if (binomial_param(r, &p, &n, x, ctx))
		return r;
	if (dn_lt0(x))
		return decNumberZero(r);
	if (dn_lt0(decNumberCompare(&t, &n, x, ctx)))
		return decNumberCopy(r, &const_1);

	decNumberAdd(&u, x, &const_1, ctx);
	decNumberSubtract(&v, &n, x, ctx);
	decNumberSubtract(&t, &const_1, &p, ctx);
	return betai(r, &v, &u, &t, ctx);
#else
	return NULL;
#endif
}

decNumber *pdf_B(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber n, p, t, u, v;

	if (binomial_param(r, &p, &n, x, ctx))
		return r;
	if (! is_int(x, ctx)) {
		decNumberZero(r);
		return r;
	}

	decNumberSubtract(&u, &n, x, ctx);
	if (dn_lt0(&u) || dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	decNumberSubtract(&t, &const_1, &p, ctx);
	decNumberPower(&v, &t, &u, ctx);
	decNumberComb(&t, &n, x, ctx);
	decNumberMultiply(&u, &t, &v, ctx);
	decNumberPower(&t, &p, x, ctx);
	return decNumberMultiply(r, &t, &u, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_B(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberFloor(&t, x, ctx);
	return cdf_B_helper(r, &t, ctx);
}

decNumber *qf_B(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber p, n;

	if (binomial_param(r, &p, &n, x, ctx))
		return r;
	return qf_search(r, x, ctx, 1, &const_0, &n, &cdf_B_helper);
}


/* Poisson cdf f(k, lam) = 1 - iGamma(floor(k+1), lam) / floor(k)! k>=0
 */
#ifndef TINY_BUILD
static int poisson_param(decNumber *r, decNumber *lambda, const decNumber *x, decContext *ctx) {
	decNumber prob, count;

	dist_two_param(&prob, &count);
	if (param_range01(r, &prob) || param_nonnegative_int(r, &count))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	decNumberMultiply(lambda, &prob, &count, ctx);
	return 0;
}
#endif

// Evaluate via: exp(x Ln(lambda) - lambda - sum(i=1, k, Ln(i)))
decNumber *cdf_P_helper(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber lambda, t, u;

	poisson_param(r, &lambda, x, ctx);
	if (dn_lt0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberAdd(&u, x, &const_1, ctx);
	decNumberGammap(&t, &u, &lambda, ctx);
	return decNumberSubtract(r, &const_1, &t, ctx);
#else
	return NULL;
#endif
}

decNumber *pdf_P(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber lambda, t, u, v;

	if (poisson_param(r, &lambda, x, ctx))
		return r;
	if (! is_int(x, ctx) || dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	decNumberPower(&t, &lambda, x, ctx);
	decNumberFactorial(&u, x, ctx);
	decNumberDivide(&v, &t, &u, ctx);
	decNumberExp(&t, &lambda, ctx);
	return decNumberDivide(r, &v, &t, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_P(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberFloor(&t, x, ctx);
	return cdf_P_helper(r, &t, ctx);
}

decNumber *qf_P(decNumber *r, const decNumber *x, decContext *ctx) {
	// MORE: provide reasonable initial estaimtes
	return qf_search(r, x, ctx, 1, &const_0, &const_20, &cdf_P_helper);
}


/* Geometric cdf
 */
#ifndef TINY_BUILD
static int geometric_param(decNumber *r, decNumber *p, const decNumber *x, decContext *ctx) {
        dist_one_param(p);
        if (param_range01(r, p))
                return 1;
        if (decNumberIsNaN(x)) {
                set_NaN(r);
                return 1;
        }
        return 0;
}
#endif

decNumber *pdf_G(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber p, t, u, v;

	if (geometric_param(r, &p, x, ctx))
		return r;
        if (! is_int(x, ctx) || dn_lt0(x)) {
		decNumberZero(r);
		return r;
        }
	decNumberSubtract(&t, &const_1, &p, ctx);
	decNumberSubtract(&u, x, &const_1, ctx);
	decNumberPower(&v, &t, &u, ctx);
	return decNumberMultiply(r, &v, &p, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_G(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
        decNumber p, t, u, ipx;

        if (geometric_param(r, &p, x, ctx))
                return r;
        if (! is_int(x, ctx)) {
		decNumberFloor(&ipx, x, ctx);
		x = &ipx;
        }
        if (dn_le0(x))
                return decNumberZero(r);
        if (decNumberIsInfinite(x))
                return decNumberCopy(r, &const_1);

//	decNumberMinus(&t, &p, ctx);
//	decNumberLn1p(&u, &t, ctx);
	dn_ln1m(&u, &p, ctx);
	decNumberMultiply(&t, &u, x, ctx);
	decNumberExpm1(&u, &t, ctx);
	return decNumberMinus(r, &u, ctx);
#else
	return NULL;
#endif
}

decNumber *qf_G(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
        decNumber p, t, v;

        if (geometric_param(r, &p, x, ctx))
                return r;
        if (check_probability(r, x, ctx, 1))
                return r;
//	decNumberMinus(&t, x, ctx);
//	decNumberLn1p(&v, &t, ctx);
//	decNumberMinus(&u, &p, ctx);
//	decNumberLn1p(&t, &u, ctx);
	dn_ln1m(&v, x, ctx);
	dn_ln1m(&t, &p, ctx);
        return decNumberDivide(r, &v, &t, ctx);
#else
	return NULL;
#endif
}

/* Normal with specified mean and variance */
#ifndef TINY_BUILD
static int normal_xform(decNumber *r, decNumber *q, const decNumber *x, decNumber *var, decContext *ctx) {
	decNumber a, mu;

	dist_two_param(&mu, var);
	if (param_positive(r, var))
		return 1;
	decNumberSubtract(&a, x, &mu, ctx);
	decNumberDivide(q, &a, var, ctx);
	return 0;
}
#endif

decNumber *pdf_normal(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber q, var, s;

	if (normal_xform(r, &q, x, &var, ctx))
		return r;
	pdf_Q(&s, &q, ctx);
	return decNumberDivide(r, &s, &var, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_normal(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber q, var;

	if (normal_xform(r, &q, x, &var, ctx))
		return r;
	return cdf_Q(r, &q, ctx);
#else
	return NULL;
#endif
}

decNumber *qf_normal(decNumber *r, const decNumber *p, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, mu, var;

	dist_two_param(&mu, &var);
	if (param_positive(r, &var))
		return r;
	qf_Q(&a, p, ctx);
	decNumberMultiply(&b, &a, &var, ctx);
	return decNumberAdd(r, &b, &mu, ctx);
#else
	return NULL;
#endif
}


/* Log normal with specified mean and variance */
decNumber *pdf_lognormal(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber t, lx;

	decNumberLn(&lx, x, ctx);
	pdf_normal(&t, &lx, ctx);
	return decNumberDivide(r, &t, x, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_lognormal(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber lx;

	decNumberLn(&lx, x, ctx);
	return cdf_normal(r, &lx, ctx);
#else
	return NULL;
#endif
}


decNumber *qf_lognormal(decNumber *r, const decNumber *p, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber lr;

	qf_normal(&lr, p, ctx);
	return decNumberExp(r, &lr, ctx);
#else
	return NULL;
#endif
}

/* Logistic with specified mean and spread */
#ifndef TINY_BUILD
static int logistic_xform(decNumber *r, decNumber *c, const decNumber *x, decNumber *s, decContext *ctx) {
	decNumber mu, a, b;
	
	dist_two_param(&mu, s);
	if (param_positive(r, s))
		return 1;
	decNumberSubtract(&a, x, &mu, ctx);
	decNumberDivide(&b, &a, s, ctx);
	decNumberMultiply(c, &b, &const_0_5, ctx);
	return 0;
}
#endif

decNumber *pdf_logistic(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, s;

	if (logistic_xform(r, &a, x, &s, ctx))
		return r;
	decNumberCosh(&b, &a, ctx);
	decNumberSquare(&a, &b, ctx);
	decNumberMultiply(&b, &a, &const_4, ctx);
	decNumberMultiply(&a, &b, &s, ctx);
	return decNumberRecip(r, &a, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_logistic(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, s;

	if (logistic_xform(r, &a, x, &s, ctx))
		return r;
	decNumberTanh(&b, &a, ctx);
	decNumberMultiply(&a, &b, &const_0_5, ctx);
	return decNumberAdd(r, &a, &const_0_5, ctx);
#else
	return NULL;
#endif
}

decNumber *qf_logistic(decNumber *r, const decNumber *p, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, mu, s;

	dist_two_param(&mu, &s);
	if (param_positive(r, &s))
		return r;
	if (check_probability(r, p, ctx, 0))
	    return r;
	decNumberSubtract(&a, p, &const_0_5, ctx);
	decNumberMultiply(&b, &a, &const_2, ctx);
	decNumberArcTanh(&a, &b, ctx);
	decNumberMultiply(&b, &a, &const_2, ctx);
	decNumberMultiply(&a, &b, &s, ctx);
	return decNumberAdd(r, &a, &mu, ctx);
#else
	return NULL;
#endif
}

/* Cauchy distribution */
#ifndef TINY_BUILD
static int cauchy_xform(decNumber *r, decNumber *c, const decNumber *x, decNumber *gamma, decContext *ctx) {
	decNumber a, x0;

	dist_two_param(&x0, gamma);
	if (param_positive(r, gamma))
		return 1;
	decNumberSubtract(&a, x, &x0, ctx);
	decNumberDivide(c, &a, gamma, ctx);
	return 0;
}
#endif

decNumber *pdf_cauchy(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma, ctx))
		return r;
	decNumberSquare(&a, &b, ctx);
	decNumberAdd(&b, &a, &const_1, ctx);
	decNumberMultiply(&a, &b, &const_PI, ctx);
	decNumberMultiply(&b, &a, &gamma, ctx);
	return decNumberRecip(r, &b, ctx);
#else
	return NULL;
#endif
}

decNumber *cdf_cauchy(decNumber *r, const decNumber *x, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma, ctx))
		return r;
	do_atan(&a, &b, ctx);
	decNumberDivide(&b, &a, &const_PI, ctx);
	return decNumberAdd(r, &b, &const_0_5, ctx);
#else
	return NULL;
#endif
}

decNumber *qf_cauchy(decNumber *r, const decNumber *p, decContext *ctx) {
#ifndef TINY_BUILD
	decNumber a, b, x0, gamma;

	dist_two_param(&x0, &gamma);
	if (param_positive(r, &gamma))
		return r;
	if (check_probability(r, p, ctx, 0))
	    return r;
	decNumberSubtract(&a, p, &const_0_5, ctx);
	decNumberMultiply(&b, &a, &const_PI, ctx);
	decNumberTan(&a, &b, ctx);
	decNumberMultiply(&b, &a, &gamma, ctx);
	return decNumberAdd(r, &b, &x0, ctx);
#else
	return NULL;
#endif
}

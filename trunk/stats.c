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

/* Implement statistical functions.
 * The data collection and analysis commands have two different options
 * possible.  The first is to use numerically stable incrementally updating
 * algorithms.  The second is to use higher precisions accumulation variables
 * and hope that they retain sufficient accuracy for the task.
 *
 * I've chosen the latter route since our intermediate variables are well over
 * twice the number of digits and have a much much larger exponent range so
 * we should be safe.
 */

#include "xeq.h"
#include "decn.h"
#include "stats.h"
#include "consts.h"
#include "int.h"


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


static void correlation(decNumber *, const enum sigma_modes);


static int check_data(int n) {
	decNumber r, s;

	decimal64ToNumber(&sigmaN, &r);
	decNumberCompare(&s, &r, small_int(n), Ctx);
	if (decNumberIsNegative(&s) && ! decNumberIsZero(&s)) {
		err(ERR_MORE_POINTS);
		return 1;
	}
	return 0;
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
	if (decNumberIsNegative(&b) && !decNumberIsZero(&b)) {
		correlation(&c, SIGMA_LINEAR);
		decNumberAbs(&b, &c, Ctx);
		for (i=SIGMA_LINEAR+1; i<SIGMA_BEST; i++) {
			correlation(&d, i);

			if (! decNumberIsNaN(&d)) {
				decNumberAbs(&c, &d, Ctx);
				decNumberCompare(&d, &b, &c, Ctx);
				if (decNumberIsNegative(&d) && !decNumberIsZero(&d)) {
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

static void S(decimal64 *x, decimal64 *y, enum sigma_modes mode, int sub1, int rootn, int exp) {
	decNumber N, nm1, *n = &N;
	decNumber sx, sxx, sy, syy;

	if (check_data(2))
		return;
	get_sigmas(&N, &sx, &sy, &sxx, &syy, NULL, mode);
	if (sub1)
		decNumberSubtract(n = &nm1, &N, &const_1, Ctx);
	do_s(x, &sxx, &sx, &N, n, rootn, exp);
	do_s(y, &syy, &sy, &N, n, rootn, exp);
}

// sx = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / (n-1))
void stats_s(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, SIGMA_QUIET_LINEAR, 1, 0, 0);
}

// [sigma]x = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / n)
void stats_sigma(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, SIGMA_QUIET_LINEAR, 0, 0, 0);
}

// serr = sx / sqrt(n)
void stats_SErr(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, SIGMA_QUIET_LINEAR, 1, 1, 0);
}


// sx = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / (n-1))
void stats_gs(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, SIGMA_QUIET_POWER, 1, 0, 0);
}

// [sigma]x = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / n)
void stats_gsigma(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, SIGMA_QUIET_POWER, 0, 0, 0);
}

// serr = sx / sqrt(n)
void stats_gSErr(decimal64 *x, decimal64 *y, decContext *ctx64) {
	S(x, y, SIGMA_QUIET_POWER, 1, 1, 0);
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
	decNumber aa, c, d, apb, am1, ap1, m, m2;
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
	for (i=0; i<100; i++) {
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
		decNumberSubtract(&t, &v, &const_1, ctx);
		decNumberAbs(&u, &t, ctx);
		decNumberCompare(&t, &u, &const_1e_10, ctx);
		if (decNumberIsNegative(&t))
			break;
	}
}

/* Regularised incomplete beta function Ix(a, b)
 */
decNumber *betai(decNumber *r, const decNumber *a, const decNumber *b, const decNumber *x, decContext *ctx) {
	decNumber t, u, v, w, y;
	int limit = 0;

	decNumberCompare(&t, &const_1, x, ctx);
	if (decNumberIsNegative(x) || decNumberIsNegative(&t)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsZero(x) || decNumberIsZero(&t))
		limit = 1;
	else {
		decNumberLnBeta(&u, a, b, ctx);
		decNumberLn(&v, x, ctx);		// v = ln(x)
		decNumberMultiply(&t, a, &v, ctx);
		decNumberSubtract(&v, &t, &u, ctx);		// v = lng(...)+a.ln(x)
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
}


/* Get parameters for a distribution */
static void dist_one_param(decNumber *a) {
	get_reg_n_as_dn(regJ_idx, a);
}

static void dist_two_param(decNumber *a, decNumber *b) {
	get_reg_n_as_dn(regJ_idx, a);
	get_reg_n_as_dn(regK_idx, b);
}


// Normal(0,1) PDF
// 1/sqrt(2 PI) . exp(-x^2/2)
static decNumber *pdf_Q(decNumber *q, const decNumber *x, decContext *ctx) {
	decNumber r, t;

	decNumberSquare(&t, x, ctx);
	decNumberMultiply(&r, &t, &const_0_5, ctx);
	decNumberMinus(&t, &r, ctx);
	decNumberExp(&r, &t, ctx);
	return decNumberMultiply(q, &r, &const_recipsqrt2PI, ctx);
}

// Normal(0,1) CDF function using the Error Function and a transformation
decNumber *cdf_Q(decNumber *q, const decNumber *x, decContext *ctx) {
	decNumber t, u;

	decNumberAdd(&u, x, &const_10, ctx);
	if (decNumberIsNegative(&u)) {
		// For big negative arguments, use a continued fraction expansion
		int n = 21;
		decNumberRecip(&t, small_int(n), ctx);
		while (--n > 0) {
			decNumberAdd(&u, x, &t, ctx);
			decNumberDivide(&t, small_int(n), &u, ctx);
		}
		decNumberAdd(&u, &t, x, ctx);
		pdf_Q(&t, x, ctx);
		decNumberDivide(q, &t, &u, ctx);
		return decNumberMinus(q, q, ctx);
	} else	{
		decNumberMultiply(&t, x, &const_root2on2, ctx);
		decNumberERF(&u, &t, ctx);
		decNumberAdd(&t, &u, &const_1, ctx);
		return decNumberMultiply(q, &t, &const_0_5, ctx);
	}
}


// Pv(x) = (x/2)^(v/2) . exp(-x/2) / Gamma(v/2+1) . (1 + sum(x^k/(v+2)(v+4)..(v+2k))
decNumber *cdf_chi2(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber a, b, v;

	dist_one_param(&v);
	if (decNumberIsNaN(x) || decNumberIsSpecial(&v) ||
			decNumberIsNegative(&v) || decNumberIsZero(&v) ||
			!is_int(&v, Ctx)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberMultiply(&a, &v, &const_0_5, ctx);
	decNumberMultiply(&b, x, &const_0_5, ctx);
	return decNumberGammap(r, &a, &b, ctx);
}

decNumber *cdf_T(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t, u, z, v;

	dist_one_param(&v);
	if (decNumberIsNaN(x) || decNumberIsNaN(&v) ||
			decNumberIsNegative(&v) || decNumberIsZero(&v) ||
			!is_int(&v, Ctx)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsInfinite(x)) {
		if (decNumberIsNegative(x))
			return decNumberZero(r);
		return decNumberCopy(r, &const_1);
	}
	if (decNumberIsInfinite(&v))			// Normal in the limit
		return cdf_Q(r, x, ctx);

	decNumberSquare(&t, x, ctx);
	decNumberAdd(&u, &t, &v, ctx);
	decNumberSquareRoot(&z, &u, ctx);
	decNumberAdd(&t, &z, x, ctx);
	decNumberDivide(&u, &t, &z, ctx);
	decNumberMultiply(&t, &u, &const_0_5, ctx);
	decNumberMultiply(&u, &v, &const_0_5, ctx);
	return betai(r, &u, &u, &t, ctx);
}

decNumber *cdf_F(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t, u, w, v1, v2;

	dist_two_param(&v1, &v2);
	if (decNumberIsNaN(x) || decNumberIsSpecial(&v1) || decNumberIsSpecial(&v2) ||
			decNumberIsNegative(&v1) || decNumberIsZero(&v1) ||
			decNumberIsNegative(&v2) || decNumberIsZero(&v2) ||
			!is_int(&v1, ctx) || !is_int(&v2, ctx)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberMultiply(&t, &v1, x, ctx);
	decNumberAdd(&u, &t, &v2, ctx);			// u = v1 * x + v2
	decNumberDivide(&w, &t, &u, ctx);		// w = (v1 * x) / (v1 * x + v2)
	decNumberMultiply(&t, &v1, &const_0_5, ctx);
	decNumberMultiply(&u, &v2, &const_0_5, ctx);
	return betai(r, &t, &u, &w, ctx);
}

/* Weibull distribution cdf = 1 - exp(-(x/lambda)^k)
 */
decNumber *cdf_WB(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber k, lam, t;

	dist_two_param(&k, &lam);

	if (decNumberIsNaN(x) || decNumberIsSpecial(&k) || decNumberIsSpecial(&lam) ||
			decNumberIsNegative(&k) || decNumberIsZero(&k) ||
			decNumberIsNegative(&lam) || decNumberIsZero(&lam)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberDivide(&t, x, &lam, ctx);
	decNumberPower(&lam, &t, &k, ctx);
	decNumberMinus(&t, &lam, ctx);
	decNumberExp(&lam, &t, ctx);
	return decNumberSubtract(r, &const_1, &lam, ctx);
}

/* Exponential distribution cdf = 1 - exp(-lambda . x)
 */
decNumber *cdf_EXP(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber lam, t, u;

	dist_one_param(&lam);
	if (decNumberIsNaN(x) || decNumberIsSpecial(&lam) ||
			decNumberIsNegative(&lam) || decNumberIsZero(&lam)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberMultiply(&t, &lam, x, ctx);
	decNumberMinus(&u, &t, ctx);
	decNumberExp(&t, &u, ctx);
	return decNumberSubtract(r, &const_1, &t, ctx);
}

/* Binomial cdf f(k; n, p) = iBeta(n-floor(k), 1+floor(k); 1-p)
 */
decNumber *cdf_B_helper(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber n, p, t, u, v;

	dist_two_param(&p, &n);
	decNumberCompare(&t, &const_1, &p, ctx);
	if (decNumberIsNaN(x) || decNumberIsSpecial(&n) || decNumberIsSpecial(&p) ||
			decNumberIsNegative(&p) || decNumberIsZero(&p) ||
			decNumberIsNegative(&t) || decNumberIsZero(&t) ||
			(decNumberIsNegative(&n) && !decNumberIsZero(&n)) ||
			!is_int(&n, ctx)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberAdd(&u, x, &const_1, ctx);
	decNumberSubtract(&v, &n, x, ctx);
	decNumberSubtract(&t, &const_1, &p, ctx);
	return betai(r, &v, &u, &t, ctx);
}

decNumber *cdf_B(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberFloor(&t, x, ctx);
	return cdf_B_helper(&t, x, ctx);
}

/* Poisson cdf f(k, lam) = 1 - iGamma(floor(k+1), lam) / floor(k)! k>=0
 */
decNumber *cdf_P_helper(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber lambda, t, u;

	dist_two_param(&t, &u);		// n
	decNumberMultiply(&lambda, &t, &u, ctx);
	if (decNumberIsNaN(x) || decNumberIsSpecial(&lambda) ||
			(decNumberIsNegative(&lambda) && !decNumberIsZero(&lambda)) ||
			!is_int(&lambda, ctx)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberAdd(&u, x, &const_1, ctx);
	decNumberGammap(&t, &u, &lambda, ctx);
	return decNumberSubtract(r, &const_1, &t, ctx);
}

decNumber *cdf_P(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberFloor(&t, x, ctx);
	return cdf_P_helper(r, &t, ctx);
}

/* Poisson cdf f(k, lam) = 1 - iGamma(floor(k+1), lam) / floor(k)! k>=0
 */
decNumber *cdf_G_helper(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber p, t, u;

	dist_one_param(&p);
	decNumberCompare(&t, &const_1, &p, ctx);
	if (decNumberIsNaN(x) || decNumberIsSpecial(&p) ||
			decNumberIsNegative(&p) || decNumberIsZero(&p) ||
			decNumberIsNegative(&t) || decNumberIsZero(&t)) {
		set_NaN(r);
		return r;
	}
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberSubtract(&t, &const_1, &p, ctx);
	decNumberPower(&u, &t, x, ctx);
	return decNumberSubtract(r, &const_1, &u, ctx);
}

decNumber *cdf_G(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberFloor(&t, x, ctx);
	return cdf_G_helper(r, &t, ctx);
}
static int qf_eval(decNumber *diff, const decNumber *pt, const decNumber *x, decContext *ctx,
		decNumber *(*f)(decNumber *, const decNumber *, decContext *)) {
	decNumber z, prob;

	(*f)(&prob, pt, ctx);
        decNumberLn(&z, &prob, ctx);
	decNumberSubtract(diff, &z, x, ctx);
	if (decNumberIsZero(diff))
		return 0;
	if (decNumberIsNegative(diff))
		return -1;
	return 1;
}

static int check_probability(decNumber *r, const decNumber *x, decContext *ctx, const decNumber *lower) {
	decNumber t;

	/* Range check the probability input */
	if (decNumberIsZero(x)) {
	    if (lower != NULL)
		decNumberCopy(r, lower);
	    else
		set_neginf(r);
	    return 1;
	}
	decNumberCompare(&t, &const_1, x, ctx);
	if (decNumberIsZero(&t)) {
	    set_inf(r);
	    return 1;
	}
	if (decNumberIsNegative(&t) || decNumberIsNegative(x)) {
	    set_NaN(r);
	    return 1;
	}
	return 0;
}

static decNumber *qf_search(decNumber *r,
				const decNumber *xin, decContext *ctx, const decNumber *lower,
				const decNumber *samp_low, const decNumber *samp_high,
		decNumber *(*f)(decNumber *, const decNumber *, decContext *)) {
	decNumber t, u, v, tv, uv, vv, x;
	unsigned int flags = 0;

	if (check_probability(r, xin, ctx, lower))
	    return r;

        decNumberLn(&x, xin, ctx);

	// Evaluate the first two points which are given to us.
	decNumberCopy(&t, samp_low);
	if (qf_eval(&tv, &t, &x, ctx, f) == 0)
		return decNumberCopy(r, &t);

	decNumberCopy(&u, samp_high);
	if (qf_eval(&uv, &u, &x, ctx, f) == 0)
		return decNumberCopy(r, &u);

	solver_init(&v, &t, &u, &tv, &uv, ctx, &flags);
	do
		if (qf_eval(&vv, &v, &x, ctx, f) == 0)
			break;
	while (solver_step(&t, &u, &v, &tv, &uv, &vv, ctx, &flags) == 0);
	return decNumberCopy(r, &v);
}


/* Quantile functions call the above solving code to invert the CDF
 */
decNumber *qf_Q(decNumber *r, const decNumber *x, decContext *ctx) {
	// An initial guess of sqrt(-2*ln x) could be tried.
	return qf_search(r, x, ctx, NULL, &const__4, &const_7, &cdf_Q);
}

decNumber *qf_chi2(decNumber *r, const decNumber *x, decContext *ctx) {
	return qf_search(r, x, ctx, &const_0, &const_0, &const_20, &cdf_chi2);
}

decNumber *qf_T(decNumber *r, const decNumber *x, decContext *ctx) {
	return qf_search(r, x, ctx, NULL, &const__7, &const_13, &cdf_T);
}

decNumber *qf_F(decNumber *r, const decNumber *x, decContext *ctx) {
	return qf_search(r, x, ctx, &const_0, &const_0, &const_20, &cdf_F);
}

/* Inverse discrete distributions */
decNumber *discrete_qf(decNumber *r, const decNumber *x, decContext *ctx,
		decNumber *(*f)(decNumber *, const decNumber *, decContext *)) {
	return qf_search(r, x, ctx, &const_0, &const_0, &const_20, f);
}

decNumber *qf_P(decNumber *r, const decNumber *x, decContext *ctx) {
	return discrete_qf(r, x, ctx, &cdf_P_helper);
}

decNumber *qf_G(decNumber *r, const decNumber *x, decContext *ctx) {
	return discrete_qf(r, x, ctx, &cdf_G_helper);
}

decNumber *qf_B(decNumber *r, const decNumber *x, decContext *ctx) {
	return discrete_qf(r, x, ctx, &cdf_B_helper);
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
	decNumber t, u, k, lam;

	if (check_probability(r, p, ctx, &const_0))
	    return r;
	dist_two_param(&k, &lam);
	decNumberSubtract(&t, &const_1, p, ctx);
	if (decNumberIsNaN(p) || decNumberIsSpecial(&lam) || decNumberIsSpecial(&k) ||
			decNumberIsNegative(&k) || decNumberIsZero(&k) ||
			decNumberIsNegative(&lam) || decNumberIsZero(&lam)) {
		set_NaN(r);
		return r;
	}

	decNumberLn(&u, &t, ctx);
	decNumberMinus(&t, &u, ctx);
	decNumberRecip(&u, &k, ctx);
	decNumberPower(&k, &t, &u, ctx);
	return decNumberMultiply(r, &lam, &k, ctx);
}

/* Exponential distribution quantile function:
 *	p = 1 - exp(-lambda . x)
 *	exp(-lambda . x) = 1 - p
 *	-lambda . x = ln(1 - p)
 * Thus, the quantile function is:
 *	x = ln(1-p)/-lambda
 */
decNumber *qf_EXP(decNumber *r, const decNumber *p, decContext *ctx) {
	decNumber t, u, lam;

	if (check_probability(r, p, ctx, &const_0))
	    return r;
	dist_one_param(&lam);
	decNumberSubtract(&t, &const_1, p, ctx);
	if (decNumberIsNaN(p) || decNumberIsSpecial(&lam) ||
			decNumberIsNegative(&lam) || decNumberIsZero(&lam)) {
		set_NaN(r);
		return r;
	}

	decNumberLn(&u, &t, ctx);
	decNumberDivide(&t, &u, &lam, ctx);
	return decNumberMinus(r, &t, ctx);
}


/* Normal with specified mean and variance */
decNumber *cdf_normal(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber a, b, mu, var;

	dist_two_param(&mu, &var);
	decNumberSubtract(&a, x, &mu, ctx);
	decNumberDivide(&b, &a, &var, ctx);
	return cdf_Q(r, &b, ctx);
}

decNumber *qf_normal(decNumber *r, const decNumber *p, decContext *ctx) {
	decNumber a, b, mu, var;

	qf_Q(&a, p, ctx);
	dist_two_param(&mu, &var);
	decNumberMultiply(&b, &a, &var, ctx);
	return decNumberAdd(r, &b, &mu, ctx);
}


/* Log normal with specified mean and variance */
decNumber *cdf_lognormal(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber lx;

	decNumberLn(&lx, x, ctx);
	return cdf_normal(r, &lx, ctx);
}

decNumber *qf_lognormal(decNumber *r, const decNumber *p, decContext *ctx) {
	decNumber lr;

	qf_normal(&lr, p, ctx);
	return decNumberExp(r, &lr, ctx);
}

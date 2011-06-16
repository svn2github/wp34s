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
#endif


static void correlation(decNumber *, const enum sigma_modes);

static int check_number(const decNumber *r, int n) {
	decNumber s;

	if (dn_lt0(dn_compare(&s, r, small_int(n)))) {
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


void stats_mode_expf(decimal64 *nul1, decimal64 *nul2) {
	State.sigma_mode = SIGMA_EXP;
}

void stats_mode_linf(decimal64 *nul1, decimal64 *nul2) {
	State.sigma_mode = SIGMA_LINEAR;
}

void stats_mode_logf(decimal64 *nul1, decimal64 *nul2) {
	State.sigma_mode = SIGMA_LOG;
}

void stats_mode_pwrf(decimal64 *nul1, decimal64 *nul2) {
	State.sigma_mode = SIGMA_POWER;
}

void stats_mode_best(decimal64 *nul1, decimal64 *nul2) {
	State.sigma_mode = SIGMA_BEST;
}

void sigma_clear(decimal64 *nul1, decimal64 *nul2) {
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
static void sigop(decimal64 *r, const decNumber *a, decNumber *(*op)(decNumber *, const decNumber *, const decNumber *)) {
	decNumber t, u;

	decimal64ToNumber(r, &t);
	(*op)(&u, &t, a);
	packed_from_number(r, &u);
}


/* Multiply a pair of values and accumulate into the sigma data.
 */
static void mulop(decimal64 *r, const decNumber *a, const decNumber *b, decNumber *(*op)(decNumber *, const decNumber *, const decNumber *)) {
	decNumber t, u, v;

	dn_multiply(&t, a, b);
	decimal64ToNumber(r, &v);
	(*op)(&u, &v, &t);
	packed_from_number(r, &u);
}


/* Define a helper function to handle sigma+ and sigma-
 */
static void sigma_helper(decNumber *(*op)(decNumber *, const decNumber *, const decNumber *)) {
	decNumber x, y;
	decNumber lx, ly;

	getXY(&x, &y);

	sigop(&sigmaN, &const_1, op);
	sigop(&sigmaX, &x, op);
	sigop(&sigmaY, &y, op);
	mulop(&sigmaXX, &x, &x, op);
	mulop(&sigmaYY, &y, &y, op);
	mulop(&sigmaXY, &x, &y, op);

	decNumberSquare(&lx, &x);
	mulop(&sigmaXXY, &lx, &y, op);

//	if (State.sigma_mode == SIGMA_LINEAR)
//		return;

	dn_ln(&lx, &x);
	dn_ln(&ly, &y);

	sigop(&sigmalnX, &lx, op);
	sigop(&sigmalnY, &ly, op);
	mulop(&sigmalnXlnX, &lx, &lx, op);
	mulop(&sigmalnYlnY, &ly, &ly, op);
	mulop(&sigmalnXlnY, &lx, &ly, op);
	mulop(&sigmaXlnY, &x, &ly, op);
	mulop(&sigmaYlnX, &y, &lx, op);
}

void sigma_plus() {
	sigma_helper(&dn_add);
}

void sigma_minus() {
	sigma_helper(&dn_subtract);
}


/* Loop through the various modes and work out
 * which has the highest absolute correlation.
 */
static enum sigma_modes determine_best(const decNumber *n) {
	enum sigma_modes m = SIGMA_LINEAR, i;
	decNumber b, c, d;

	dn_compare(&b, &const_2, n);
	if (dn_lt0(&b)) {
		correlation(&c, SIGMA_LINEAR);
		dn_abs(&b, &c);
		for (i=SIGMA_LINEAR+1; i<SIGMA_BEST; i++) {
			correlation(&d, i);

			if (! decNumberIsNaN(&d)) {
				dn_abs(&c, &d);
				dn_compare(&d, &b, &c);
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
static enum sigma_modes get_sigmas(decNumber *N, decNumber *sx, decNumber *sy, decNumber *sxx, decNumber *syy,
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
	return mode;
}


void sigma_N(decimal64 *x, decimal64 *y) {
	*x = sigmaN;
}

void sigma_X(decimal64 *x, decimal64 *y) {
	*x = sigmaX;
}

void sigma_Y(decimal64 *x, decimal64 *y) {
	*x = sigmaY;
}

void sigma_XX(decimal64 *x, decimal64 *y) {
	*x = sigmaXX;
}

void sigma_YY(decimal64 *x, decimal64 *y) {
	*x = sigmaYY;
}

void sigma_XY(decimal64 *x, decimal64 *y) {
	*x = sigmaXY;
}

void sigma_X2Y(decimal64 *x, decimal64 *y) {
	*x = sigmaXXY;
}

void sigma_lnX(decimal64 *x, decimal64 *y) {
	*x = sigmalnX;
}

void sigma_lnXlnX(decimal64 *x, decimal64 *y) {
	*x = sigmalnXlnX;
}

void sigma_lnY(decimal64 *x, decimal64 *y) {
	*x = sigmalnY;
}

void sigma_lnYlnY(decimal64 *x, decimal64 *y) {
	*x = sigmalnYlnY;
}

void sigma_lnXlnY(decimal64 *x, decimal64 *y) {
	*x = sigmalnXlnY;
}

void sigma_XlnY(decimal64 *x, decimal64 *y) {
	*x = sigmaXlnY;
}

void sigma_YlnX(decimal64 *x, decimal64 *y) {
	*x = sigmaYlnX;
}

void sigma_sum(decimal64 *x, decimal64 *y) {
	sigma_X(x, NULL);
	sigma_Y(y, NULL);
}


static void mean_common(decimal64 *res, const decNumber *x, const decNumber *n, int exp) {
	decNumber t, u, *p = &t;

	dn_divide(&t, x, n);
	if (exp)
		dn_exp(p=&u, &t);
	packed_from_number(res, p);
}

void stats_mean(decimal64 *x, decimal64 *y) {
	decNumber N;
	decNumber sx, sy;

	if (check_data(1))
		return;
	get_sigmas(&N, &sx, &sy, NULL, NULL, NULL, SIGMA_QUIET_LINEAR);

	mean_common(x, &sx, &N, 0);
	mean_common(y, &sy, &N, 0);
}


// weighted mean sigmaXY / sigmaY
void stats_wmean(decimal64 *x, decimal64 *nul) {
	decNumber xy, y;

	if (check_data(1))
		return;
	get_sigmas(NULL, NULL, &y, NULL, NULL, &xy, SIGMA_QUIET_LINEAR);

	mean_common(x, &xy, &y, 0);
}

// geometric mean e^(sigmaLnX / N)
void stats_gmean(decimal64 *x, decimal64 *y) {
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

	decNumberSquare(&t, sx);
	dn_divide(&u, &t, N);
	dn_subtract(&t, sxx, &u);
	dn_divide(&u, &t, denom);
	dn_sqrt(p = &t, &u);

	if (rootn) {
		dn_sqrt(&u, N);
		dn_divide(p = &v, &t, &u);
	}
	if (exp) {
		dn_exp(&u, p);
		p = &u;
	}
	packed_from_number(s, p);
}

static void S(decimal64 *x, decimal64 *y, int sample, int rootn, int exp) {
	decNumber N, nm1, *n = &N;
	decNumber sx, sxx, sy, syy;

	if (check_data(2))
		return;
	get_sigmas(&N, &sx, &sy, &sxx, &syy, NULL, exp?SIGMA_QUIET_POWER:SIGMA_QUIET_LINEAR);
	if (sample)
		dn_subtract(n = &nm1, &N, &const_1);
	do_s(x, &sxx, &sx, &N, n, rootn, exp);
	do_s(y, &syy, &sy, &N, n, rootn, exp);
}

// sx = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / (n-1))
void stats_s(decimal64 *x, decimal64 *y) {
	S(x, y, 1, 0, 0);
}

// [sigma]x = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / n)
void stats_sigma(decimal64 *x, decimal64 *y) {
	S(x, y, 0, 0, 0);
}

// serr = sx / sqrt(n)
void stats_SErr(decimal64 *x, decimal64 *y) {
	S(x, y, 1, 1, 0);
}


// sx = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / (n-1))
void stats_gs(decimal64 *x, decimal64 *y) {
	S(x, y, 1, 0, 1);
}

// [sigma]x = sqrt(sigmaX^2 - (sigmaX ^ 2 ) / n)
void stats_gsigma(decimal64 *x, decimal64 *y) {
	S(x, y, 0, 0, 1);
}

// serr = sx / sqrt(n)
void stats_gSErr(decimal64 *x, decimal64 *y) {
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

	dn_multiply(&t, &sy, &sxxy);
	decNumberSquare(&u, &sxy);
	dn_subtract(&v, &t, &u);
	decNumberSquare(p = &t, &sy);
	if (sample)
		dn_subtract(p = &u, &t, &syy);
	dn_divide(&w, &v, p);
	dn_sqrt(p = &u, &w);
	if (rootn) {
		dn_sqrt(&t, &sy);
		dn_divide(p = &v, &u, &t);
	}
	packed_from_number(x, p);
}

void stats_ws(decimal64 *x, decimal64 *y) {
	WS(x, 1, 0);
}

void stats_wsigma(decimal64 *x, decimal64 *y) {
	WS(x, 0, 0);
}

void stats_wSErr(decimal64 *x, decimal64 *y) {
	WS(x, 1, 1);
}


decNumber *stats_sigper(decNumber *res, const decNumber *x) {
	decNumber sx, t;

	get_sigmas(NULL, &sx, NULL, NULL, NULL, NULL, SIGMA_QUIET_LINEAR);
	dn_divide(&t, x, &sx);
	return dn_multiply(res, &t, &const_100);
}

/* Calculate the correlation based on the stats data using the
 * specified model.
 */
static void correlation(decNumber *t, const enum sigma_modes m) {
	decNumber N, u, v, w;
	decNumber sx, sy, sxx, syy, sxy;

	get_sigmas(&N, &sx, &sy, &sxx, &syy, &sxy, m);

	dn_multiply(t, &N, &sxx);
	decNumberSquare(&u, &sx);
	dn_subtract(&v, t, &u);
	dn_multiply(t, &N, &syy);
	decNumberSquare(&u, &sy);
	dn_subtract(&w, t, &u);
	dn_multiply(t, &v, &w);
	dn_sqrt(&w, t);
	dn_multiply(t, &N, &sxy);
	dn_multiply(&u, &sx, &sy);
	dn_subtract(&v, t, &u);
	dn_divide(t, &v, &w);

	dn_compare(&u, &const_1, t);
	if (decNumberIsNegative(&u))
		decNumberCopy(t, &const_1);
	else {
		dn_compare(&u, t, &const__1);
		if (decNumberIsNegative(&u))
			decNumberCopy(t, &const__1);
	}
}


void stats_correlation(decimal64 *r, decimal64 *nul) {
	decNumber t;

	if (check_data(2))
		return;
	correlation(&t, State.sigma_mode);
	packed_from_number(r, &t);
}


static void covariance(decimal64 *r, int sample) {
	decNumber N, t, u, v;
	decNumber sx, sy, sxy;

	if (check_data(2))
		return;
	get_sigmas(&N, &sx, &sy, NULL, NULL, &sxy, State.sigma_mode);
	dn_multiply(&t, &sx, &sy);
	dn_divide(&u, &t, &N);
	dn_subtract(&t, &sxy, &u);
	if (sample) {
		dn_subtract(&v, &N, &const_1);
		dn_divide(&u, &t, &v);
	} else
		dn_divide(&u, &t, &N);
	packed_from_number(r, &u);
}

void stats_COV(decimal64 *r, decimal64 *nul) {
	covariance(r, 0);
}

void stats_Sxy(decimal64 *r, decimal64 *nul) {
	covariance(r, 1);
}

// y = B . x + A
static enum sigma_modes do_LR(decNumber *B, decNumber *A) {
	decNumber N, u, v, denom;
	decNumber sx, sy, sxx, sxy;
	enum sigma_modes m;

	m = get_sigmas(&N, &sx, &sy, &sxx, NULL, &sxy, State.sigma_mode);

	dn_multiply(B, &N, &sxx);
	decNumberSquare(&u, &sx);
	dn_subtract(&denom, B, &u);

	dn_multiply(B, &N, &sxy);
	dn_multiply(&u, &sx, &sy);
	dn_subtract(&v, B, &u);
	dn_divide(A, &v, &denom);

	dn_multiply(B, &sxx, &sy);
	dn_multiply(&u, &sx, &sxy);
	dn_subtract(&v, B, &u);
	dn_divide(B, &v, &denom);

	return m;
}


void stats_LR(decimal64 *bout, decimal64 *aout) {
	decNumber a, b;

	if (check_data(2))
		return;
	do_LR(&b, &a);
	packed_from_number(aout, &a);
	packed_from_number(bout, &b);
}


decNumber *stats_xhat(decNumber *res, const decNumber *y) {
	decNumber a, b, t, u;
	enum sigma_modes m;

	if (check_data(2))
		return NULL;
	m = do_LR(&b, &a);
	switch (m) {
	default:
		dn_subtract(&t, y, &b);
		return dn_divide(res, &a, &t);

	case SIGMA_EXP:
		dn_ln(&t, y);
		dn_subtract(&u, &t, &b);
		return dn_divide(res, &u, &a);

	case SIGMA_LOG:
		dn_subtract(&t, y, &b);
		dn_divide(&b, &t, &a);
		return dn_exp(res, &b);

	case SIGMA_POWER:
	case SIGMA_QUIET_POWER:
		dn_ln(&t, y);
		dn_subtract(&u, &t, &b);
		dn_divide(&t, &u, &a);
		return dn_exp(res, &t);
	}
}


decNumber *stats_yhat(decNumber *res, const decNumber *x) {
	decNumber a, b, t, u;
	enum sigma_modes m;

	if (check_data(2))
		return NULL;
	m = do_LR(&b, &a);
	switch (m) {
	default:
		dn_multiply(&t, x, &a);
		return dn_add(res, &t, &b);

	case SIGMA_EXP:
		dn_multiply(&t, x, &a);
		dn_add(&a, &b, &t);
		return dn_exp(res, &a);

	case SIGMA_LOG:
		dn_ln(&u, x);
		dn_multiply(&t, &u, &a);
		return dn_add(res, &t, &b);

	case SIGMA_POWER:
	case SIGMA_QUIET_POWER:
		dn_ln(&t, x);
		dn_multiply(&u, &t, &a);
		dn_add(&t, &u, &b);
		return dn_exp(res, &t);
	}
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

void stats_random(decimal64 *r, decimal64 *nul) {
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
		ullint_to_dn(&z, s);
		dn_multiply(&y, &z, &const_randfac);
		packed_from_number(r, &y);
	}
}


void stats_sto_random(decimal64 *nul1, decimal64 *nul2) {
	unsigned long int s;
	int z;
	decNumber x;

	if (is_intmode()) {
		 s = d64toInt(&regX) & 0xffffffff;
	} else {
		getX(&x);
		s = (unsigned long int) dn_to_ull(&x, &z);
	}
	taus_seed(s);
}


static void check_low(decNumber *d) {
	decNumber t, u;

	dn_abs(&t, d);
	dn_compare(&u, &t, &const_1e_32);
	if (decNumberIsNegative(&u))
		decNumberCopy(d, &const_1e_32);
}


static void ib_step(decNumber *d, decNumber *c, const decNumber *aa) {
	decNumber t, u;

	dn_multiply(&t, aa, d);
	dn_add(&u, &const_1, &t);	// d = 1+aa*d
	check_low(&u);
	decNumberRecip(d, &u);
	dn_divide(&t, aa, c);
	dn_add(c, &const_1, &t);	// c = 1+aa/c
	check_low(c);
}


static void betacf(decNumber *r, const decNumber *a, const decNumber *b, const decNumber *x) {
	decNumber aa, c, d, apb, am1, ap1, m, m2, oldr;
	int i;
	decNumber t, u, v, w;

	dn_add(&ap1, a, &const_1);		// ap1 = 1+a
	dn_subtract(&am1, a, &const_1);	// am1 = a-1
	dn_add(&apb, a, b);			// apb = a+b
	decNumberCopy(&c, &const_1);			// c = 1
	dn_divide(&t, x, &ap1);
	dn_multiply(&u, &t, &apb);
	dn_subtract(&t, &const_1, &u);	// t = 1-apb*x/ap1
	check_low(&t);
	decNumberRecip(&d, &t);			// d = 1/t
	decNumberCopy(r, &d);				// res = d
	decNumberZero(&m);
	for (i=0; i<500; i++) {
		decNumberCopy(&oldr, r);
		dn_inc(&m);			// m = i+1
		dn_multiply(&m2, &m, &const_2);
		dn_subtract(&t, b, &m);
		dn_multiply(&u, &t, &m);
		dn_multiply(&t, &u, x);	// t = m*(b-m)*x
		dn_add(&u, &am1, &m2);
		dn_add(&v, a, &m2);
		dn_multiply(&w, &u, &v);	// w = (am1+m2)*(a+m2)
		dn_divide(&aa, &t, &w);	// aa = t/w
		ib_step(&d, &c, &aa);
		dn_multiply(&t, r, &d);
		dn_multiply(r, &t, &c);	// r = r*d*c
		dn_add(&t, a, &m);
		dn_add(&u, &apb, &m);
		dn_multiply(&w, &t, &u);
		dn_multiply(&t, &w, x);
		dn_minus(&w, &t);		// w = -(a+m)*(apb+m)*x
		dn_add(&t, a, &m2);
		dn_add(&u, &ap1, &m2);
		dn_multiply(&v, &t, &u);	// v = (a+m2)*(ap1+m2)
		dn_divide(&aa, &w, &v);	// aa = w/v
		ib_step(&d, &c, &aa);
		dn_multiply(&v, &d, &c);
		dn_multiply(r, r, &v);	// r *= d*c
		dn_compare(&u, &oldr, r);
		if (decNumberIsZero(&u))
			break;
	}
}

/* Regularised incomplete beta function Ix(a, b)
 */
decNumber *betai(decNumber *r, const decNumber *a, const decNumber *b, const decNumber *x) {
	decNumber t, u, v, w, y;
	int limit = 0;

	dn_compare(&t, &const_1, x);
	if (decNumberIsNegative(x) || decNumberIsNegative(&t)) {
		return set_NaN(r);
	}
	if (decNumberIsZero(x) || decNumberIsZero(&t))
		limit = 1;
	else {
		decNumberLnBeta(&u, a, b);
		dn_ln(&v, x);		// v = ln(x)
		dn_multiply(&t, a, &v);
		dn_subtract(&v, &t, &u);	// v = lng(...)+a.ln(x)
		dn_subtract(&y, &const_1, x);// y = 1-x
		dn_ln(&u, &y);		// u = ln(1-x)
		dn_multiply(&t, &u, b);
		dn_add(&u, &t, &v);		// u = lng(...)+a.ln(x)+b.ln(1-x)
		dn_exp(&w, &u);
	}
	dn_add(&v, a, b);
	dn_add(&u, &v, &const_2);		// u = a+b+2
	dn_add(&t, a, &const_1);		// t = a+1
	dn_divide(&v, &t, &u);		// u = (a+1)/(a+b+2)
	dn_compare(&t, x, &v);
	if (decNumberIsNegative(&t)) {
		if (limit)
			return decNumberZero(r);
		betacf(&t, a, b, x);
		dn_divide(&u, &t, a);
		return dn_multiply(r, &w, &u);
	} else {
		if (limit)
			return decNumberCopy(r, &const_1);
		betacf(&t, b, a, &y);
		dn_divide(&u, &t, b);
		dn_multiply(&t, &w, &u);
		return dn_subtract(r, &const_1, &t);
	}
}


static int check_probability(decNumber *r, const decNumber *x, int min_zero) {
	decNumber t;

	/* Range check the probability input */
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	if (decNumberIsZero(x)) {
	    if (min_zero)
		decNumberCopy(r, &const_0);
	    else
		set_neginf(r);
	    return 1;
	}
	dn_compare(&t, &const_1, x);
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
			(intg && !is_int(n))) {
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

	dn_compare(&h, &const_1, p);
	if (decNumberIsSpecial(p) || dn_lt0(p) || dn_lt0(&h)) {
		decNumberZero(r);
		err(ERR_BAD_PARAM);
		return 1;
	}
	return 0;
}


/* Evaluate Ln(1 - x) accurately
 */
decNumber *dn_ln1m(decNumber *r, const decNumber *x) {
	decNumber a;
	dn_minus(&a, x);
	return decNumberLn1p(r, &a);
}


// Normal(0,1) PDF
// 1/sqrt(2 PI) . exp(-x^2/2)
decNumber *pdf_Q(decNumber *q, const decNumber *x) {
	decNumber r, t;

	decNumberSquare(&t, x);
	dn_multiply(&r, &t, &const_0_5);
	dn_minus(&t, &r);
	dn_exp(&r, &t);
	return dn_multiply(q, &r, &const_recipsqrt2PI);
}

// Normal(0,1) CDF function
decNumber *cdf_Q_helper(decNumber *q, decNumber *pdf, const decNumber *x) {
	decNumber t, u, v, a, x2, d, absx, n;
	int i;

	pdf_Q(pdf, x);
	dn_abs(&absx, x);
	dn_compare(&u, &const_PI, &absx);	// We need a number about 3.2 and this is close enough
	if (decNumberIsNegative(&u)) {
		dn_minus(&x2, &absx);
		//n = ceil(5 + k / (|x| - 1))
		dn_subtract(&v, &absx, &const_1);
		dn_divide(&t, &const_256, &v);
		dn_add(&u, &t, &const_4);
		decNumberCeil(&n, &u);
		decNumberZero(&t);
		do {
			dn_add(&u, x, &t);
			dn_divide(&t, &n, &u);
			dn_dec(&n);
		} while (! decNumberIsZero(&n));

		dn_add(&u, &t, x);
		dn_divide(q, pdf, &u);
		if (! decNumberIsNegative(q))
			dn_subtract(q, &const_1, q);
		if (decNumberIsNegative(x))
			dn_minus(q, q);
		return q;
	} else {
		decNumberSquare(&x2, &absx);
		decNumberCopy(&t, &absx);
		decNumberCopy(&a, &absx);
		decNumberCopy(&d, &const_3);
		for (i=0;i<500; i++) {
			dn_multiply(&u, &t, &x2);
			dn_divide(&t, &u, &d);
			dn_add(&u, &a, &t);
			dn_compare(&v, &u, &a);
			if (decNumberIsZero(&v))
				break;
			decNumberCopy(&a, &u);
			dn_add(&d, &d, &const_2);
		}
		dn_multiply(&v, &a, pdf);
		if (decNumberIsNegative(x))
			return dn_subtract(q, &const_0_5, &v);
		return dn_add(q, &const_0_5, &v);
	}
}

decNumber *cdf_Q(decNumber *q, const decNumber *x) {
	decNumber t;
	return cdf_Q_helper(q, &t, x);
}


static void qf_Q_est(decNumber *est, const decNumber *x, const decNumber *x05) {
	const int invert = decNumberIsNegative(x05);
	decNumber a, b, u, xc;

	if (invert) {
		dn_subtract(&xc, &const_1, x);
		x = &xc;
	}

	dn_compare(&a, x, &const_0_2);
	if (decNumberIsNegative(&a)) {
		dn_ln(&a, x);
		dn_multiply(&u, &a, &const__2);
		dn_subtract(&a, &u, &const_1);
		dn_sqrt(&b, &a);
		dn_multiply(&a, &b, &const_sqrt2PI);
		dn_multiply(&b, &a, x);
		dn_ln(&a, &b);
		dn_multiply(&b, &a, &const__2);
		dn_sqrt(&a, &b);
		dn_divide(&b, &const_0_2, &u);
		dn_add(est, &a, &b);
		if (!invert)
			dn_minus(est, est);
	} else {
		dn_multiply(&a, &const_sqrt2PI, x05);
		decNumberCube(&b, &a);
		dn_divide(&u, &b, &const_6);
		dn_add(est, &u, &a);
		dn_minus(est, est);
	}
}

decNumber *qf_Q(decNumber *r, const decNumber *x) {
	decNumber a, b, t, cdf, pdf;
	int i;


	if (check_probability(r, x, 0))
		return r;
	dn_subtract(&b, &const_0_5, x);
	if (decNumberIsZero(&b)) {
		decNumberZero(r);
		return r;
	}

	qf_Q_est(r, x, &b);
	for (i=0; i<10; i++) {
		cdf_Q_helper(&cdf, &pdf, r);
		dn_subtract(&a, &cdf, x);
		dn_divide(&t, &a, &pdf);
		dn_multiply(&a, &t, r);
		dn_multiply(&b, &a, &const_0_5);
		dn_subtract(&a, &b, &const_1);
		dn_divide(&b, &t, &a);
		dn_add(&a, &b, r);
		if (relative_error(&a, r, &const_1e_32))
			break;
		decNumberCopy(r, &a);
	}
	return decNumberCopy(r, &a);
}


// Pv(x) = (x/2)^(v/2) . exp(-x/2) / Gamma(v/2+1) . (1 + sum(x^k/(v+2)(v+4)..(v+2k))
static int chi2_param(decNumber *r, decNumber *k, const decNumber *x) {
	dist_one_param(k);
	if (param_positive_int(r, k))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

decNumber *pdf_chi2_helper(decNumber *r, const decNumber *x, const decNumber *k) {
	decNumber k1, k2, t, s, v;

	if (dn_le0(x))
		return decNumberZero(r);

	dn_multiply(&s, x, &const__0_5);		// s = -x/2
	dn_multiply(&k2, k, &const_0_5);		// k2 = k/2
	dn_subtract(&k1, &k2, &const_1);		// k = k/2-1
	dn_ln(&t, x);					// t = ln(x)
	dn_multiply(&v, &k1, &t);			// r = (k/2-1) ln(x)
	dn_add(&k1, &s, &v);
	decNumberLnGamma(&t, &k2);
	dn_subtract(&s, &k1, &t);
	dn_multiply(&t, &k2, &const_ln2);
	dn_subtract(&k1, &s, &t);
	return dn_exp(r, &k1);
}

decNumber *pdf_chi2(decNumber *r, const decNumber *x) {
	decNumber k;

	if (chi2_param(r, &k, x))
		return r;
	return pdf_chi2_helper(r, x, &k);
}

decNumber *cdf_chi2_helper(decNumber *r, const decNumber *x, const decNumber *v) {
	decNumber a, b;

	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	dn_multiply(&a, v, &const_0_5);
	dn_multiply(&b, x, &const_0_5);
	return decNumberGammap(r, &a, &b);
}

decNumber *cdf_chi2(decNumber *r, const decNumber *x) {
	decNumber v;

	if (chi2_param(r, &v, x))
		return r;
	return cdf_chi2_helper(r, x, &v);
}

static void qf_chi2_est(decNumber *guess, const decNumber *n, const decNumber *p) {
	decNumber a, b, d;

	if (decNumberIsNegative(dn_compare(&a, &const_15, n)))
		decNumberCopy(&a, &const_0_85);
	else
		decNumberCopy(&a, &const_1);

	dn_multiply(&b, &a, n);
	dn_minus(&a, &b);
	dn_exp(&b, &a);
	if (decNumberIsNegative(dn_compare(&a, p, &b))) {
		dn_multiply(&a, n, &const_0_5);
		decNumberLnGamma(&b, &a);
		dn_divide(guess, &b, &a);
		dn_exp(&b, guess);
		dn_multiply(guess, p, &a);
		decNumberRecip(&d, &a);
		dn_power(&a, guess, &d);
		dn_multiply(&d, &a, &b);
		dn_multiply(guess, &d, &const_2);
	} else {
		dn_subtract(&b, &const_0_5, p);
		qf_Q_est(&a, p, &b);
		dn_multiply(&b, &a, &const_0_97);
		dn_divide(&a, &const_0_2214, n);
		// c = n * (b * sqrt(a) - a + 1) ^ 3
		dn_sqrt(guess, &a);
		dn_multiply(&d, guess, &b);
		dn_subtract(guess, &d, &a);
		dn_inc(guess);
		decNumberSquare(&d, guess);
		dn_multiply(&a, &d, guess);
		dn_multiply(guess, &a, n);
		dn_multiply(&a, n, &const_6);
		dn_add(&b, &a, &const_16);
		if (decNumberIsNegative(dn_compare(&a, &b, guess))) {
			dn_ln1m(&a, p);
			dn_multiply(&b, &const_150, n);
			dn_divide(&d, &a, &b);
			dn_inc(&d);
			dn_multiply(guess, guess, &d);
		}
	}
	if (dn_le0(dn_compare(&a, guess, &const_1e_400)))
		decNumberZero(guess);
}

static decNumber *newton_qf_chi2(decNumber *r, const decNumber *v, const decNumber *p) {
	decNumber a, b, d, md, prev;
	int i;

	dn_multiply(&md, r, &const_0_04);
	for (i=0; i<100; i++) {
		cdf_chi2_helper(&a, r, v);
		dn_subtract(&b, &a, p);
		dn_abs(&a, &b);
		if (i > 0 && decNumberIsNegative(dn_compare(&d, &prev, &a)))
			return set_NaN(r);
		decNumberCopy(&prev, &a);

		pdf_chi2_helper(&a, r, v);
		dn_divide(&d, &b, &a);			// d = (cdf() - p)/pdf()
		dn_abs(&a, &d);
		dn_compare(&b, &md, &a);
		if (decNumberIsNegative(&b)) {
			if (decNumberIsNegative(&a))
				dn_minus(&d, &md);
			else
				decNumberPlus(&d, &md, &Ctx);
		}
		decNumberCopy(&b, r);
		dn_subtract(r, &b, &d);
		if (relative_error(r, &b, &const_1e_24))
			break;
	}
	return r;
}

decNumber *qf_chi2(decNumber *r, const decNumber *p) {
	decNumber v;

	if (chi2_param(r, &v, p))
		return r;
	qf_chi2_est(r, &v, p);
	if (decNumberIsZero(r))
		return r;
	return newton_qf_chi2(r, &v, p);
}


static int t_param(decNumber *r, decNumber *v, const decNumber *x) {
	dist_one_param(v);
	if (param_positive(r, v))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

decNumber *pdf_T_helper(decNumber *r, const decNumber *x, const decNumber *v) {
	decNumber t, u, w;

	dn_multiply(&t, v, &const_0_5);			// t=v/2
	decNumberLnGamma(&w, &t);			// w = lnGamma(v/2)
	dn_add(&u, &t, &const_0_5);			// u = (v+1)/2
	decNumberLnGamma(&t, &u);			// t = lnGamma((v+1)/2)
	dn_subtract(r, &t, &w);				// r = lnGamma((v+1)/2) - lnGamma(v/2)
	decNumberSquare(&t, x);
	dn_divide(&w, &t, v);				// w = x^2 / v
	decNumberLn1p(&t, &w);				// t = ln (1 + x^2 / v)
	dn_multiply(&w, &t, &u);			// w = ln (1 + x^2 / v) . (v+1)/2
	dn_subtract(&t, r, &w);
	dn_exp(&u, &t);
	dn_multiply(&w, v, &const_PI);
	dn_sqrt(&t, &w);
	return dn_divide(r, &u, &t);
}

decNumber *pdf_T(decNumber *r, const decNumber *x) {
	decNumber v;

	if (t_param(r, &v, x))
		return r;
	return pdf_T_helper(r, x, &v);
}

decNumber *cdf_T_helper(decNumber *r, const decNumber *x, const decNumber *v) {
	decNumber t, u;
	int invert;

	if (decNumberIsInfinite(x)) {
		if (decNumberIsNegative(x))
			return decNumberZero(r);
		return decNumberCopy(r, &const_1);
	}
	if (decNumberIsInfinite(v))			// Normal in the limit
		return cdf_Q(r, x);
	if (decNumberIsZero(x))
		return decNumberCopy(r, &const_0_5);
	invert = ! decNumberIsNegative(x);
	decNumberSquare(&t, x);
	dn_add(r, &t, v);
	dn_divide(&t, v, r);
	dn_multiply(r, v, &const_0_5);
	betai(&u, r, &const_0_5, &t);
	dn_multiply(r, &const_0_5, &u);
	if (invert)
		dn_subtract(r, &const_1, r);
	return r;
}

decNumber *cdf_T(decNumber *r, const decNumber *x) {
	decNumber v;

	if (t_param(r, &v, x))
		return r;
	return cdf_T_helper(r, x, &v);
}

static void qf_T_est(decNumber *r, const decNumber *df, const decNumber *p, const decNumber *p05) {
	const int invert = decNumberIsNegative(p05);
	int negate;
	decNumber a, b, u, pc, pc05, x, x2, x3;

	if (invert) {
		dn_subtract(&pc, &const_1, p);
		p = &pc;
		dn_minus(&pc05, p05);
		p05 = &pc05;
	}
	dn_ln(&a, p);
	dn_minus(&a, &a);
	dn_multiply(&b, df, &const_1_7);
	if (dn_lt0(dn_compare(&u, &a, &b))) {
		qf_Q_est(&x, p, p05);
		decNumberSquare(&x2, &x);
		dn_multiply(&x3, &x2, &x);
		dn_add(&a, &x, &x3);
		dn_multiply(&b, &a, &const_0_25);
		dn_divide(&a, &b, df);
		dn_add(r, &a, &x);

		dn_divide(&a, &x2, &const_3);
		dn_inc(&a);
		dn_multiply(&b, &a, &x3);
		dn_multiply(&a, &b, &const_0_25);
		decNumberSquare(&b, df);
		dn_divide(&u, &a, &b);
		dn_add(r, r, &u);
		negate = invert;
	} else {
		dn_multiply(&x2, df, &const_2);
		dn_subtract(&b, &x2, &const_1);
		dn_divide(&a, &const_PI, &b);
		dn_sqrt(&b, &a);
		dn_multiply(&a, &b, &x2);
		dn_multiply(&b, &a, p);
		decNumberRecip(&a, df);
		dn_power(&u, &b, &a);
		dn_sqrt(&a, df);
		dn_divide(r, &a, &u);
		negate = !invert;
	}
	if (negate)
		dn_minus(r, r);
}

static int qf_T_init(decNumber *r, decNumber *v, const decNumber *x) {
	decNumber a, b, c, d;

	if (t_param(r, v, x))
		return 1;
	dn_subtract(&b, &const_0_5, x);
	if (decNumberIsZero(&b)) {
		decNumberZero(r);
		return 1;
	}
	if (decNumberIsInfinite(v)) {					// Normal in the limit
		qf_Q(r, x);
		return 1;
	}

	dn_compare(&a, v, &const_1);
	if (decNumberIsZero(&a)) {					// special case v = 1
		dn_multiply(&a, &b, &const_PI);
		dn_sincos(&a, &c, &d);
		dn_divide(&a, &c, &d);			// lower = tan(pi (x - 1/2))
		dn_minus(r, &a);
		return 1;
	}
	dn_compare(&d, v, &const_2);			// special case v = 2
	if (decNumberIsZero(&d)) {
		dn_subtract(&a, &const_1, x);
		dn_multiply(&c, &a, x);
		dn_multiply(&d, &c, &const_4);		// alpha = 4p(1-p)

		dn_divide(&c, &const_2, &d);
		dn_sqrt(&a, &c);
		dn_multiply(&c, &a, &b);
		dn_multiply(r, &c, &const__2);
		return 1;
	}

	// common case v >= 3
	qf_T_est(r, v, x, &b);
	return 0;
}

static decNumber *newton_qf_T(decNumber *q, const decNumber *p, const decNumber *ndf) {
	decNumber w, x, z, ndfp1;
	int i;

	dn_add(&ndfp1, ndf, &const_1);
	for (i=0; i<30; i++) {
		// Newton's approximation
		dn_subtract(&z, cdf_T_helper(&x, q, ndf), p);
		if (decNumberIsZero(pdf_T_helper(&x, q, ndf)))
			break;
		dn_divide(&w, &z, &x);
		decNumberCopy(&z, q);
		dn_subtract(q, &z, &w);
		if (relative_error(q, &z, &const_1e_24))
			break;
	}
	return q;
}

decNumber *qf_T(decNumber *r, const decNumber *x) {
	decNumber ndf;

	if (qf_T_init(r, &ndf, x))
		return r;
	newton_qf_T(r, x, &ndf);
	return r;
}

static int f_param(decNumber *r, decNumber *d1, decNumber *d2, const decNumber *x) {
	dist_two_param(d1, d2);
	if (param_positive(r, d1) || param_positive(r, d2))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

decNumber *pdf_F_helper(decNumber *r, const decNumber *x, const decNumber *d1, const decNumber *d2) {
	decNumber a, b, c, s;

	dn_ln(&a, d2);
	dn_multiply(&s, &a, d2);
	dn_multiply(&c, d1, x);
	dn_ln(&b, &c);
	dn_multiply(&a, &b, d1);
	dn_add(&s, &s, &a);
	dn_add(&a, &c, d2);
	dn_ln(&b, &a);
	dn_add(&a, d1, d2);
	dn_multiply(&c, &b, &a);
	dn_subtract(&a, &s, &c);
	dn_multiply(&s, &a, &const_0_5);
	dn_multiply(&a, d1, &const_0_5);
	dn_multiply(&b, d2, &const_0_5);
	decNumberLnBeta(&c, &a, &b);
	dn_subtract(&a, &s, &c);
	dn_exp(&b, &a);
	return dn_divide(r, &b, x);
}

decNumber *pdf_F(decNumber *r, const decNumber *x) {
	decNumber d1, d2;

	if (f_param(r, &d1, &d2, x))
		return r;
	return pdf_F_helper(r, x, &d1, &d2);
}

decNumber *cdf_F_helper(decNumber *r, const decNumber *x, const decNumber *v1, const decNumber *v2) {
	decNumber t, u, w;

	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	dn_multiply(&t, v1, x);
	dn_add(&u, &t, v2);			// u = v1 * x + v2
	dn_divide(&w, &t, &u);			// w = (v1 * x) / (v1 * x + v2)
	dn_multiply(&t, v1, &const_0_5);
	dn_multiply(&u, v2, &const_0_5);
	return betai(r, &t, &u, &w);
}

decNumber *cdf_F(decNumber *r, const decNumber *x) {
	decNumber v1, v2;

	if (f_param(r, &v1, &v2, x))
		return r;
	return cdf_F_helper(r, x, &v1, &v2);
}

static decNumber *newton_qf_F(decNumber *q, const decNumber *p, const decNumber *df1, const decNumber *df2) {
	decNumber w, x, z;
	int i;

	for (i=0; i<100; i++) {
		// Newton's approximation
		dn_subtract(&z, cdf_F_helper(&x, q, df1, df2), p);
		if (decNumberIsZero(pdf_F_helper(&x, q, df1, df2)))
			break;
		dn_divide(&w, &z, &x);
		decNumberCopy(&z, q);
		dn_subtract(q, &z, &w);
		if (relative_error(q, &z, &const_1e_24))
			break;
	}
	return q;
}

decNumber *qf_F(decNumber *r, const decNumber *x) {
	decNumber df1, df2;

	if (f_param(r, &df1, &df2, x))
		return r;
	if (decNumberIsZero(x))
		return decNumberZero(r);

	decNumberCopy(r, &const_5);		// Need better initial estimate
	return newton_qf_F(r, x, &df1, &df2);
}


/* Weibull distribution cdf = 1 - exp(-(x/lambda)^k)
 */
static int weibull_param(decNumber *r, decNumber *k, decNumber *lam, const decNumber *x) {
	dist_two_param(k, lam);
	if (param_positive(r, k) || param_positive(r, lam))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

decNumber *pdf_WB(decNumber *r, const decNumber *x) {
	decNumber k, lam, t, u, v, q;

	if (weibull_param(r, &k, &lam, x))
		return r;
	if (dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	dn_divide(&q, x, &lam);
	dn_power(&u, &q, &k);		// (x/lam)^k
	dn_divide(&t, &u, &q);		// (x/lam)^(k-1)
	dn_exp(&v, &u);
	dn_divide(&q, &t, &v);
	dn_divide(&t, &q, &lam);
	return dn_multiply(r, &t, &k);
}

decNumber *cdf_WB(decNumber *r, const decNumber *x) {
	decNumber k, lam, t;

	if (weibull_param(r, &k, &lam, x))
		return r;
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	dn_divide(&t, x, &lam);
	dn_power(&lam, &t, &k);
	dn_minus(&t, &lam);
	decNumberExpm1(&lam, &t);
	return dn_minus(r, &lam);
}


/* Weibull distribution quantile function:
 *	p = 1 - exp(-(x/lambda)^k)
 *	exp(-(x/lambda)^k) = 1 - p
 *	-(x/lambda)^k = ln(1-p)
 * Thus, the qf is:
 *	x = (-ln(1-p) ^ (1/k)) * lambda
 * So no searching is required.
 */
decNumber *qf_WB(decNumber *r, const decNumber *p) {
	decNumber t, u, k, lam;

	if (weibull_param(r, &k, &lam, p))
		return r;
	if (check_probability(r, p, 1))
	    return r;
	if (decNumberIsSpecial(&lam) || decNumberIsSpecial(&k) ||
			dn_le0(&k) || dn_le0(&lam)) {
		return set_NaN(r);
	}

	dn_ln1m(&u, p);
	dn_minus(&t, &u);
	decNumberRecip(&u, &k);
	dn_power(&k, &t, &u);
	return dn_multiply(r, &lam, &k);
}


/* Exponential distribution cdf = 1 - exp(-lambda . x)
 */
static int exponential_xform(decNumber *r, decNumber *lam, const decNumber *x) {
	dist_one_param(lam);
	if (param_positive(r, lam))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

decNumber *pdf_EXP(decNumber *r, const decNumber *x) {
	decNumber lam, t, u;

	if (exponential_xform(r, &lam, x))
		return r;
	if (dn_lt0(x)) {
		set_NaN(r);
		return r;
	}
	dn_multiply(&t, &lam, x);
	dn_minus(&u, &t);
	dn_exp(&t, &u);
	return dn_multiply(r, &t, &lam);
}

decNumber *cdf_EXP(decNumber *r, const decNumber *x) {
	decNumber lam, t, u;

	if (exponential_xform(r, &lam, x))
		return r;
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	dn_multiply(&t, &lam, x);
	dn_minus(&u, &t);
	decNumberExpm1(&t, &u);
	return dn_minus(r, &t);
}


/* Exponential distribution quantile function:
 *	p = 1 - exp(-lambda . x)
 *	exp(-lambda . x) = 1 - p
 *	-lambda . x = ln(1 - p)
 * Thus, the quantile function is:
 *	x = ln(1-p)/-lambda
 */
decNumber *qf_EXP(decNumber *r, const decNumber *p) {
	decNumber t, u, lam;

	dist_one_param(&lam);
	if (param_positive(r, &lam))
		return r;
	if (check_probability(r, p, 1))
	    return r;
	if (decNumberIsSpecial(&lam) || dn_le0(&lam)) {
		return set_NaN(r);
	}

//	dn_minus(&t, p);
//	decNumberLn1p(&u, &t);
	dn_ln1m(&u, p);
	dn_divide(&t, &u, &lam);
	return dn_minus(r, &t);
}

/* Binomial cdf f(k; n, p) = iBeta(n-floor(k), 1+floor(k); 1-p)
 */
static int binomial_param(decNumber *r, decNumber *p, decNumber *n, const decNumber *x) {
	dist_two_param(p, n);
	if (param_nonnegative_int(r, n) || param_range01(r, p))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

decNumber *pdf_B_helper(decNumber *r, const decNumber *x, const decNumber *p, const decNumber *n) {
	decNumber t, u, v;

	dn_subtract(&u, n, x);
	if (dn_lt0(&u) || dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	dn_ln1m(&v, p);
	dn_multiply(&t, &u, &v);
	dn_exp(&v, &t);
	decNumberComb(&t, n, x);
	dn_multiply(&u, &t, &v);
	dn_power(&t, p, x);
	return dn_multiply(r, &t, &u);
}

decNumber *pdf_B(decNumber *r, const decNumber *x) {
	decNumber n, p;

	if (binomial_param(r, &p, &n, x))
		return r;
	if (! is_int(x)) {
		decNumberZero(r);
		return r;
	}
	return pdf_B_helper(r, x, &p, &n);
}

decNumber *cdf_B_helper(decNumber *r, const decNumber *x, const decNumber *p, const decNumber *n) {
	decNumber t, u, v;

	if (dn_lt0(x))
		return decNumberZero(r);
	if (dn_lt0(dn_compare(&t, n, x)))
		return decNumberCopy(r, &const_1);

	dn_add(&u, x, &const_1);
	dn_subtract(&v, n, x);
	dn_subtract(&t, &const_1, p);
	return betai(r, &v, &u, &t);
}

decNumber *cdf_B(decNumber *r, const decNumber *x) {
	decNumber t, n, p;

	if (binomial_param(r, &p, &n, x))
		return r;
	decNumberFloor(&t, x);
	return cdf_B_helper(r, &t, &p, &n);
}

decNumber *qf_B(decNumber *r, const decNumber *p) {
	decNumber prob, n, w, x, z;
	int i;

	if (binomial_param(r, &prob, &n, p))
		return r;
	if (check_probability(r, p, 1))
		return r;

	decNumberCopy(r, &const_5);	// Better initial estimate required
	for (i=0; i<100; i++) {
		dn_subtract(&z, cdf_B_helper(&x, r, &prob, &n), p);
		if (decNumberIsZero(pdf_B_helper(&x, r, &prob, &n)))
			break;
		dn_divide(&w, &z, &x);
		decNumberCopy(&z, r);
		dn_subtract(r, &z, &w);
		if (relative_error(r, &z, &const_1e_24))
			break;
	}
	return r;
}

/* Poisson cdf f(k, lam) = 1 - iGamma(floor(k+1), lam) / floor(k)! k>=0
 */
static int poisson_param(decNumber *r, decNumber *lambda, const decNumber *x) {
	decNumber prob, count;

	dist_two_param(&prob, &count);
	if (param_range01(r, &prob) || param_nonnegative_int(r, &count))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	dn_multiply(lambda, &prob, &count);
	return 0;
}

decNumber *pdf_P_helper(decNumber *r, const decNumber *x, const decNumber *lambda) {
	decNumber t, u, v;

	dn_power(&t, lambda, x);
	decNumberFactorial(&u, x);
	dn_divide(&v, &t, &u);
	dn_exp(&t, lambda);
	return dn_divide(r, &v, &t);
}

decNumber *pdf_P(decNumber *r, const decNumber *x) {
	decNumber lambda;

	if (poisson_param(r, &lambda, x))
		return r;
	if (! is_int(x) || dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	return pdf_P_helper(r, x, &lambda);
}

decNumber *cdf_P_helper(decNumber *r, const decNumber *x, const decNumber *lambda) {
	decNumber t, u;

	if (dn_lt0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	dn_add(&u, x, &const_1);
	decNumberGammap(&t, &u, lambda);
	return dn_subtract(r, &const_1, &t);
}

decNumber *cdf_P(decNumber *r, const decNumber *x) {
	decNumber t, lambda;

	if (poisson_param(r, &lambda, x))
		return r;
	decNumberFloor(&t, x);
	return cdf_P_helper(r, &t, &lambda);
}

decNumber *qf_P(decNumber *r, const decNumber *p) {
	decNumber lambda, x, z, w;
	int i;

	if (poisson_param(r, &lambda, p))
		return r;
	if (check_probability(r, p, 1))
		return r;

	decNumberCopy(r, &const_5);	// Better initial estimate required
	for (i=0; i<100; i++) {
		dn_subtract(&z, cdf_P_helper(&x, r, &lambda), p);
		if (decNumberIsZero(pdf_P_helper(&x, r, &lambda)))
			break;
		dn_divide(&w, &z, &x);
		decNumberCopy(&z, r);
		dn_subtract(r, &z, &w);
		if (relative_error(r, &z, &const_1e_24))
			break;
	}
	return r;
}

/* Geometric cdf
 */
static int geometric_param(decNumber *r, decNumber *p, const decNumber *x) {
	dist_one_param(p);
	if (param_range01(r, p))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

decNumber *pdf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, u, v;

	if (geometric_param(r, &p, x))
		return r;
	if (! is_int(x) || dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	dn_subtract(&t, &const_1, &p);
	dn_subtract(&u, x, &const_1);
	dn_power(&v, &t, &u);
	return dn_multiply(r, &v, &p);
}

decNumber *cdf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, u, ipx;

	if (geometric_param(r, &p, x))
		return r;
	if (! is_int(x)) {
		decNumberFloor(&ipx, x);
		x = &ipx;
	}
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	dn_ln1m(&u, &p);
	dn_multiply(&t, &u, x);
	decNumberExpm1(&u, &t);
	return dn_minus(r, &u);
}

decNumber *qf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, v;

	if (geometric_param(r, &p, x))
		return r;
	if (check_probability(r, x, 1))
		return r;
	dn_ln1m(&v, x);
	dn_ln1m(&t, &p);
	return dn_divide(r, &v, &t);
}

/* Normal with specified mean and variance */
static int normal_xform(decNumber *r, decNumber *q, const decNumber *x, decNumber *var) {
	decNumber a, mu;

	dist_two_param(&mu, var);
	if (param_positive(r, var))
		return 1;
	dn_subtract(&a, x, &mu);
	dn_divide(q, &a, var);
	return 0;
}

decNumber *pdf_normal(decNumber *r, const decNumber *x) {
	decNumber q, var, s;

	if (normal_xform(r, &q, x, &var))
		return r;
	pdf_Q(&s, &q);
	return dn_divide(r, &s, &var);
}

decNumber *cdf_normal(decNumber *r, const decNumber *x) {
	decNumber q, var;

	if (normal_xform(r, &q, x, &var))
		return r;
	return cdf_Q(r, &q);
}

decNumber *qf_normal(decNumber *r, const decNumber *p) {
	decNumber a, b, mu, var;

	dist_two_param(&mu, &var);
	if (param_positive(r, &var))
		return r;
	qf_Q(&a, p);
	dn_multiply(&b, &a, &var);
	return dn_add(r, &b, &mu);
}


/* Log normal with specified mean and variance */
decNumber *pdf_lognormal(decNumber *r, const decNumber *x) {
	decNumber t, lx;

	dn_ln(&lx, x);
	pdf_normal(&t, &lx);
	return dn_divide(r, &t, x);
}

decNumber *cdf_lognormal(decNumber *r, const decNumber *x) {
	decNumber lx;

	dn_ln(&lx, x);
	return cdf_normal(r, &lx);
}


decNumber *qf_lognormal(decNumber *r, const decNumber *p) {
	decNumber lr;

	qf_normal(&lr, p);
	return dn_exp(r, &lr);
}

/* Logistic with specified mean and spread */
static int logistic_xform(decNumber *r, decNumber *c, const decNumber *x, decNumber *s) {
	decNumber mu, a, b;
	
	dist_two_param(&mu, s);
	if (param_positive(r, s))
		return 1;
	dn_subtract(&a, x, &mu);
	dn_divide(&b, &a, s);
	dn_multiply(c, &b, &const_0_5);
	return 0;
}

decNumber *pdf_logistic(decNumber *r, const decNumber *x) {
	decNumber a, b, s;

	if (logistic_xform(r, &a, x, &s))
		return r;
	decNumberCosh(&b, &a);
	decNumberSquare(&a, &b);
	dn_multiply(&b, &a, &const_4);
	dn_multiply(&a, &b, &s);
	return decNumberRecip(r, &a);
}

decNumber *cdf_logistic(decNumber *r, const decNumber *x) {
	decNumber a, b, s;

	if (logistic_xform(r, &a, x, &s))
		return r;
	decNumberTanh(&b, &a);
	dn_multiply(&a, &b, &const_0_5);
	return dn_add(r, &a, &const_0_5);
}

decNumber *qf_logistic(decNumber *r, const decNumber *p) {
	decNumber a, b, mu, s;

	dist_two_param(&mu, &s);
	if (param_positive(r, &s))
		return r;
	if (check_probability(r, p, 0))
	    return r;
	dn_subtract(&a, p, &const_0_5);
	dn_multiply(&b, &a, &const_2);
	decNumberArcTanh(&a, &b);
	dn_multiply(&b, &a, &const_2);
	dn_multiply(&a, &b, &s);
	return dn_add(r, &a, &mu);
}

/* Cauchy distribution */
static int cauchy_xform(decNumber *r, decNumber *c, const decNumber *x, decNumber *gamma) {
	decNumber a, x0;

	dist_two_param(&x0, gamma);
	if (param_positive(r, gamma))
		return 1;
	dn_subtract(&a, x, &x0);
	dn_divide(c, &a, gamma);
	return 0;
}

decNumber *pdf_cauchy(decNumber *r, const decNumber *x) {
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma))
		return r;
	decNumberSquare(&a, &b);
	dn_add(&b, &a, &const_1);
	dn_multiply(&a, &b, &const_PI);
	dn_multiply(&b, &a, &gamma);
	return decNumberRecip(r, &b);
}

decNumber *cdf_cauchy(decNumber *r, const decNumber *x) {
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma))
		return r;
	do_atan(&a, &b);
	dn_divide(&b, &a, &const_PI);
	return dn_add(r, &b, &const_0_5);
}

decNumber *qf_cauchy(decNumber *r, const decNumber *p) {
	decNumber a, b, x0, gamma;

	dist_two_param(&x0, &gamma);
	if (param_positive(r, &gamma))
		return r;
	if (check_probability(r, p, 0))
	    return r;
	dn_subtract(&a, p, &const_0_5);
	dn_multiply(&b, &a, &const_PI);
	decNumberTan(&a, &b);
	dn_multiply(&b, &a, &gamma);
	return dn_add(r, &b, &x0);

}

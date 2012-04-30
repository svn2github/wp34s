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

/* Distributions:
 *
 * Binomial	570
 * Cauchy	322
 * Chi^2	844
 * Exponential	298
 * F		792
 * Geometric	394
 * LogNormal	96
 * Logistic	258
 * Normal	162
 * Poisson	466
 * Q		856
 * T		1106
 * Weibull	450
 * 
 * extras	190
 * newton solve	694
 */



/* Utility functions */


/* Utility routine to finialise a discrete distribtuion result that might be
 * off by one.  The arugments are the best guess of the reslt, the target
 * probability, the cdf at the best guess, the cdf at the best guess plus one
 * and the best guess plus one.
 *
 * A comparision is made of the best guess and if it is too high, the result
 * must be the best guess minus one.  Likewise a comparision is made to
 * determine if the best guess plus one is below the target.
 */
static decNumber *discrete_final_check(decNumber *r, const decNumber *p, const decNumber *fr, const decNumber *frp1, const decNumber *rp1) {
	if (dn_lt(p, fr))
		dn_dec(r);
	else if (dn_le(frp1, p))
		decNumberCopy(r, rp1);
	return r;
}


/* Optimise an estimate using Newton's method.  For statistical distributions we are fortunate that we've got both the
 * function and its derivative so this is straightforward.
 * Since the different distributions need slightly different flavours of this search, we add a parameter to customise.
 */
#define NEWTON_DISCRETE		0x0001
#define NEWTON_NONNEGATIVE	0x0002

// 694 bytes
static decNumber *newton_qf(decNumber *r, const decNumber *p, const unsigned short int flags,
				decNumber *(*pdf)(decNumber *r, const decNumber *x, const decNumber *a1, const decNumber *a2),
				decNumber *(*cdf)(decNumber *r, const decNumber *x, const decNumber *a1, const decNumber *a2),
				const decNumber *arg1, const decNumber *arg2, const decNumber *maxstep) {
	decNumber v, w, x, z, md, low, high;
	int i;
	const int discrete = (flags & NEWTON_DISCRETE) != 0;
	const int nonnegative = (flags & NEWTON_NONNEGATIVE) != 0;
	int allow_bisect = 1;
	const int max_iterations = 100;
	const decNumber * const cnvg_threshold = convergence_threshold();

	set_inf(&high);
	decNumberCopy(&low, nonnegative ? &const_0 : &const__inf);

	if (maxstep != NULL)
		dn_multiply(&md, r, maxstep);

	for (i=0; i<max_iterations; i++) {
		if (dn_ge(r, &high) || dn_le(r, &low)) {
			if (decNumberIsInfinite(&high))
				dn_mul2(r, &low);
			else if (decNumberIsInfinite(&low))
				dn_div2(r, &high);
			else
bisect:				dn_average(r, &low, &high);
			allow_bisect = 0;
		}
		//{char buf[20]; sprintf(buf, "%03d: est", i);dump1(r, buf);}
		dn_subtract(&z, (*cdf)(&w, r, arg1, arg2), p);

		if (dn_lt0(&z)) {
			if (allow_bisect && dn_lt(r, &low) && ! decNumberIsInfinite(&high))
				goto bisect;
			dn_max(&low, &low, r);
		} else {
			if (allow_bisect && dn_gt(r, &high) && ! decNumberIsInfinite(&low))
				goto bisect;
			dn_min(&high, &high, r);
		}
		allow_bisect = 1;

		if (discrete) {
			// Using the pdf for the slope isn't great for the discrete distributions
			// So we do something more akin to a secant approach
			dn_add(&v, r, &const_0_001);
			(*cdf)(&x, &v, arg1, arg2);
			dn_subtract(&v, &x, &w);
			dn_mulpow10(&x, &v, 3);
		} else
			(*pdf)(&x, r, arg1, arg2);
		if (dn_eq0(&x))
			break;
		dn_divide(&w, &z, &x);

		// Limit the step size if necessary
		if (maxstep != NULL) {
			dn_abs(&x, &w);
			if (dn_lt(&md, &x)) {
				if (decNumberIsNegative(&w))
					dn_minus(&w, &md);
				else
					decNumberPlus(&w, &md, &Ctx);
			}
		}

		// Update estimate
		decNumberCopy(&z, r);
		dn_subtract(r, &z, &w);

		// If this distribution doesn't take negative values, limit outselves to positive ones
		if (nonnegative && decNumberIsNegative(r))
			dn_mulpow10(r, &z, -5);

		// If our upper and lower limits are close enough together we give up searching
		if (! decNumberIsInfinite(&high) && ! decNumberIsInfinite(&low) && relative_error(&high, &low, cnvg_threshold)) {
			dn_average(r, &low, &high);
			break;
		}

		// Check for finished
		if (discrete) {
			if (absolute_error(r, &z, DISCRETE_TOLERANCE))
				break;
		} else if (relative_error(r, &z, cnvg_threshold))
			break;
		busy();
	}
	if (i >= max_iterations)
		return set_NaN(r);

	if (discrete) {
		decNumberFloor(r, r);
		(*cdf)(&x, r, arg1, arg2);
		dn_p1(&w, r);
		(*cdf)(&z, &w, arg1, arg2);
		return discrete_final_check(r, p, &x, &z, &w);
	}
	return r;
}



// 122 bytes
static int check_probability(decNumber *r, const decNumber *x, int min_zero) {
	decNumber t;

	/* Range check the probability input */
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	if (dn_eq0(x)) {
	    if (min_zero)
		decNumberCopy(r, &const_0);
	    else
		set_neginf(r);
	    return 1;
	}
	dn_compare(&t, &const_1, x);
	if (dn_eq0(&t)) {
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
// 14 bytes
static void dist_one_param(decNumber *a) {
	getRegister(a, regJ_idx);
}

static void dist_two_param(decNumber *a, decNumber *b) {
	getRegister(a, regJ_idx);
	getRegister(b, regK_idx);
}

// 80 bytes
static int param_verify(decNumber *r, const decNumber *n, int zero, int intg) {
	if (decNumberIsSpecial(n) ||
			dn_le0(n) ||
			(!zero && dn_eq0(n)) ||
			(intg && !is_int(n))) {
		decNumberZero(r);
		err(ERR_BAD_PARAM);
		return 1;
	}
	return 0;
}
#define param_positive(r, n)		(param_verify(r, n, 0, 0))
#define param_positive_int(r, n)	(param_verify(r, n, 0, 1))
#define param_nonnegative(r, n)		(param_verify(r, n, 1, 0))
#define param_nonnegative_int(r, n)	(param_verify(r, n, 1, 1))

// 60 bytes
static int param_range01(decNumber *r, const decNumber *p) {
	if (decNumberIsSpecial(p) || dn_lt0(p) || dn_gt(p, &const_1)) {
		decNumberZero(r);
		err(ERR_BAD_PARAM);
		return 1;
	}
	return 0;
}


/* Evaluate Ln(1 - x) accurately
 */
// 28 bytes
decNumber *dn_ln1m(decNumber *r, const decNumber *x) {
	decNumber a;
	dn_minus(&a, x);
	return decNumberLn1p(r, &a);
}




/* Binomial
 * Two parameters:
 *	J = probabiliy
 *	K = n, positive integer
 */

// 192 for older version with helper function
decNumber *pdf_B(decNumber *r, const decNumber *x) {
	decNumber n, p, t, u, v;

	if (binomial_param(r, &p, &n, x))
		return r;
	if (!is_int(x))
		return decNumberZero(r);

	dn_subtract(&u, &n, x);
	if (dn_lt0(&u) || dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	dn_ln1m(&v, &p);
	dn_multiply(&t, &u, &v);
	dn_exp(&v, &t);
	decNumberComb(&t, &n, x);
	dn_multiply(&u, &t, &v);
	dn_power(&t, &p, x);
	return dn_multiply(r, &t, &u);
}



/**************************************************************************/
/* Cauchy distribution
 * Two parameters:
 *	J = location (real)
 *	K = scale > 0
 * Formulas:
 *	pdf = (PI K (1 + ((x-J) / K)^2) )^-1
 *	cdf = arctan((x-J) / K) / PI + 1/2
 *	qf = J + K TAN(PI (p - 1/2))
 */

// 70 bytes
static int cauchy_xform(decNumber *r, decNumber *c, const decNumber *x, decNumber *gamma) {
	decNumber a, x0;

	dist_two_param(&x0, gamma);
	if (param_positive(r, gamma))
		return 1;
	dn_subtract(&a, x, &x0);
	dn_divide(c, &a, gamma);
	return 0;
}

// 80 bytes
decNumber *pdf_cauchy(decNumber *r, const decNumber *x) {
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma))
		return r;
	decNumberSquare(&a, &b);
	dn_p1(&b, &a);
	dn_mulPI(&a, &b);
	dn_multiply(&b, &a, &gamma);
	return decNumberRecip(r, &b);
}

// 70 bytes
decNumber *cdf_cauchy(decNumber *r, const decNumber *x) {
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma))
		return r;
	do_atan(&a, &b);
	dn_divide(&b, &a, &const_PI);
	return dn_add(r, &b, &const_0_5);
}

// 102 bytes
decNumber *qf_cauchy(decNumber *r, const decNumber *p) {
	decNumber a, b, x0, gamma;

	dist_two_param(&x0, &gamma);
	if (param_positive(r, &gamma))
		return r;
	if (check_probability(r, p, 0))
	    return r;
	dn_subtract(&a, p, &const_0_5);
	dn_mulPI(&b, &a);
	decNumberTan(&a, &b);
	dn_multiply(&b, &a, &gamma);
	return dn_add(r, &b, &x0);

}


/**************************************************************************/
/* Exponential distribution
 * One parameter:
 *	J = lambda (shape) > 0
 * Formulas:
 *	pdf = lambda exp(-lambda x)
 *	cdf = 1 - exp(-lambda x)
 *	qf = ln(1 - p) / -lambda
 */

// 56 bytes
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

// 88 bytes
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

// 100 bytes
decNumber *cdf_EXP(decNumber *r, const decNumber *x) {
	decNumber lam, t, u;

	if (exponential_xform(r, &lam, x))
		return r;
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return dn_1(r);

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
// 110 bytes
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

/**************************************************************************/
/* Geometric distribution
 * One parameter:
 *	J = probability
 */

// 52 bytes
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

// 98 bytes
decNumber *pdf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, v;

	if (geometric_param(r, &p, x))
		return r;
	if (dn_lt0(x) || !is_int(x)) {
		decNumberZero(r);
		return r;
	}
	dn_ln1m(&t, &p);
	dn_multiply(&v, &t, x);
	dn_exp(&t, &v);
	return dn_multiply(r, &t, &p);
}

// 116 bytes
decNumber *cdf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, u, v;

	if (geometric_param(r, &p, x))
		return r;
	if (dn_lt0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return dn_1(r);

	dn_ln1m(&u, &p);
	decNumberFloor(&t, x);
	dn_p1(&v, &t);
	dn_multiply(&t, &u, &v);
	decNumberExpm1(&u, &t);
	return dn_minus(r, &u);
}

// 150 bytes
decNumber *qf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, v, z;

	if (geometric_param(r, &p, x))
		return r;
	if (check_probability(r, x, 1))
		return r;
	dn_ln1m(&v, x);
	dn_ln1m(&t, &p);
	dn_divide(&p, &v, &t);
	dn_dec(&p);
	decNumberFloor(r, &p);

	/* Not sure this is absolutely necessary but it can't hurt */
	cdf_G(&t, r);
	dn_p1(&v, r);
	cdf_G(&z, &v);
	return discrete_final_check(r, &p, &t, &z, &v);
}

/* Weibull distribution cdf = 1 - exp(-(x/lambda)^k)
 */
// 60 bytes
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

// 122 bytes
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

// 112 bytes
decNumber *cdf_WB(decNumber *r, const decNumber *x) {
	decNumber k, lam, t;

	if (weibull_param(r, &k, &lam, x))
		return r;
	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return dn_1(r);

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
// 140 bytes
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




/**************************************************************************/
/* Logistic distribution
 * Two parameters:
 *	J = mu (location)
 *	K = s (scale) > 0
 * Formulas:
 *	pdf = 1 / ( (cosh( (x-J) / 2K ) )^2 * 4 K)
 *	cdf = 1/2 + tanh( (x-J) / 2K ) / 2 = (1 + tanh( (x-J) / 2K)) / 2
 *	qf = arctanh( (p-1/2) * 2) * 2K + j = archtanh(2p - 1) * 2K + J
 */

/* 78 bytes */
static int logistic_xform(decNumber *r, decNumber *c, const decNumber *x, decNumber *s) {
	decNumber mu, a, b;
	
	dist_two_param(&mu, s);
	if (param_positive(r, s))
		return 1;
	dn_subtract(&a, x, &mu);
	dn_divide(&b, &a, s);
	dn_div2(c, &b);
	return 0;
}

/* 80 bytes */
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

/* 62 bytes */
decNumber *cdf_logistic(decNumber *r, const decNumber *x) {
	decNumber a, b, s;

	if (logistic_xform(r, &a, x, &s))
		return r;
	decNumberTanh(&b, &a);
	dn_div2(&a, &b);
	return dn_add(r, &a, &const_0_5);
}

/* 116 bytes */
decNumber *qf_logistic(decNumber *r, const decNumber *p) {
	decNumber a, b, mu, s;

	dist_two_param(&mu, &s);
	if (param_positive(r, &s))
		return r;
	if (check_probability(r, p, 0))
	    return r;
	dn_subtract(&a, p, &const_0_5);
	dn_mul2(&b, &a);
	decNumberArcTanh(&a, &b);
	dn_mul2(&b, &a);
	dn_multiply(&a, &b, &s);
	return dn_add(r, &a, &mu);
}


/* Standard normal distribution */

// Normal(0,1) PDF
// 1/sqrt(2 PI) . exp(-x^2/2)

// 60 bytes
decNumber *pdf_Q(decNumber *q, const decNumber *x) {
	decNumber r, t;

	decNumberSquare(&t, x);
	dn_div2(&r, &t);
	dn_minus(&t, &r);
	dn_exp(&r, &t);
	return dn_multiply(q, &r, &const_recipsqrt2PI);
}

// Normal(0,1) CDF function
// 356 bytes
decNumber *cdf_Q_helper(decNumber *q, decNumber *pdf, const decNumber *x) {
	decNumber t, u, v, a, x2, d, absx, n;
	int i;

	pdf_Q(pdf, x);
	dn_abs(&absx, x);
	dn_compare(&u, &const_PI, &absx);	// We need a number about 3.2 and this is close enough
	if (decNumberIsNegative(&u)) {
		//dn_minus(&x2, &absx);
		//n = ceil(5 + k / (|x| - 1))
		dn_m1(&v, &absx);
		dn_divide(&t, &const_256, &v);
		dn_add(&u, &t, &const_4);
		decNumberCeil(&n, &u);
		decNumberZero(&t);
		do {
			dn_add(&u, x, &t);
			dn_divide(&t, &n, &u);
			dn_dec(&n);
		} while (! dn_eq0(&n));

		dn_add(&u, &t, x);
		dn_divide(q, pdf, &u);
		if (! decNumberIsNegative(q))
			dn_1m(q, q);
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
			if (dn_eq0(&v))
				break;
			decNumberCopy(&a, &u);
			dn_p2(&d, &d);
		}
		dn_multiply(&v, &a, pdf);
		if (decNumberIsNegative(x))
			return dn_subtract(q, &const_0_5, &v);
		return dn_add(q, &const_0_5, &v);
	}
}

// 18 bytes
decNumber *cdf_Q(decNumber *q, const decNumber *x) {
	decNumber t;
	return cdf_Q_helper(q, &t, x);
}


// 230 bytes
static void qf_Q_est(decNumber *est, const decNumber *x, const decNumber *x05) {
	const int invert = decNumberIsNegative(x05);
	decNumber a, b, u, xc;

	if (invert) {
		dn_1m(&xc, x);
		x = &xc;
	}

	dn_compare(&a, x, &const_0_2);
	if (decNumberIsNegative(&a)) {
		dn_ln(&a, x);
		dn_multiply(&u, &a, &const__2);
		dn_m1(&a, &u);
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

// 192 bytes
decNumber *qf_Q(decNumber *r, const decNumber *x) {
	decNumber a, b, t, cdf, pdf;
	int i;


	if (check_probability(r, x, 0))
		return r;
	dn_subtract(&b, &const_0_5, x);
	if (dn_eq0(&b)) {
		decNumberZero(r);
		return r;
	}

	qf_Q_est(r, x, &b);
	for (i=0; i<10; i++) {
		cdf_Q_helper(&cdf, &pdf, r);
		dn_subtract(&a, &cdf, x);
		dn_divide(&t, &a, &pdf);
		dn_multiply(&a, &t, r);
		dn_div2(&b, &a);
		dn_m1(&a, &b);
		dn_divide(&b, &t, &a);
		dn_add(&a, &b, r);
		if (relative_error(&a, r, &const_1e_32))
			break;
		decNumberCopy(r, &a);
	}
	return decNumberCopy(r, &a);
}



/* Normal with specified mean and variance */

// 70 bytes
static int normal_xform(decNumber *r, decNumber *q, const decNumber *x, decNumber *var) {
	decNumber a, mu;

	dist_two_param(&mu, var);
	if (param_positive(r, var))
		return 1;
	dn_subtract(&a, x, &mu);
	dn_divide(q, &a, var);
	return 0;
}

// 50 bytes
decNumber *pdf_normal(decNumber *r, const decNumber *x) {
	decNumber q, var, s;

	if (normal_xform(r, &q, x, &var))
		return r;
	pdf_Q(&s, &q);
	return dn_divide(r, &s, &var);
}

// 40 bytes
decNumber *cdf_normal(decNumber *r, const decNumber *x) {
	decNumber q, var;

	if (normal_xform(r, &q, x, &var))
		return r;
	return cdf_Q(r, &q);
}

// 72 bytes
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

// 40 bytes
decNumber *pdf_lognormal(decNumber *r, const decNumber *x) {
	decNumber t, lx;

	dn_ln(&lx, x);
	pdf_normal(&t, &lx);
	return dn_divide(r, &t, x);
}

// 28 bytes
decNumber *cdf_lognormal(decNumber *r, const decNumber *x) {
	decNumber lx;

	dn_ln(&lx, x);
	return cdf_normal(r, &lx);
}

// 28 bytes
decNumber *qf_lognormal(decNumber *r, const decNumber *p) {
	decNumber lr;

	qf_normal(&lr, p);
	return dn_exp(r, &lr);
}



/* Poission */
// 150 bytes for older version with a helper function
decNumber *pdf_P(decNumber *r, const decNumber *x) {
	decNumber lambda, t, u, v;

	if (poisson_param(r, &lambda, x))
		return r;
	if (!is_int(x))
		return decNumberZero(r);

	if (dn_lt0(x)) {
		decNumberZero(r);
		return r;
	}
	dn_power(&t, &lambda, x);
	decNumberFactorial(&u, x);
	dn_divide(&v, &t, &u);
	dn_exp(&t, &lambda);
	return dn_divide(r, &v, &t);
}





// Pv(x) = (x/2)^(v/2) . exp(-x/2) / Gamma(v/2+1) . (1 + sum(x^k/(v+2)(v+4)..(v+2k))
// 56 bytes
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

// 144 bytes
decNumber *pdf_chi2_helper(decNumber *r, const decNumber *x, const decNumber *k, const decNumber *null) {
	decNumber k1, k2, t, s, v;

	if (dn_le0(x))
		return decNumberZero(r);

	dn_multiply(&s, x, &const__0_5);		// s = -x/2
	dn_div2(&k2, k);				// k2 = k/2
	dn_m1(&k1, &k2);				// k = k/2-1
	dn_ln(&t, x);					// t = ln(x)
	dn_multiply(&v, &k1, &t);			// r = (k/2-1) ln(x)
	dn_add(&k1, &s, &v);
	decNumberLnGamma(&t, &k2);
	dn_subtract(&s, &k1, &t);
	dn_multiply(&t, &k2, &const_ln2);
	dn_subtract(&k1, &s, &t);
	return dn_exp(r, &k1);
}

// 44 bytes
decNumber *pdf_chi2(decNumber *r, const decNumber *x) {
	decNumber k;

	if (chi2_param(r, &k, x))
		return r;
	return pdf_chi2_helper(r, x, &k, NULL);
}

// 76 bytes
decNumber *cdf_chi2_helper(decNumber *r, const decNumber *x, const decNumber *v, const decNumber *null) {
	decNumber a, b;

	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return dn_1(r);

	dn_div2(&a, v);
	dn_div2(&b, x);
	return decNumberGammap(r, &a, &b);
}

// 44 bytes
decNumber *cdf_chi2(decNumber *r, const decNumber *x) {
	decNumber v;

	if (chi2_param(r, &v, x))
		return r;
	return cdf_chi2_helper(r, x, &v, NULL);
}


static void qf_chi2_est(decNumber *guess, const decNumber *n, const decNumber *p) {
	decNumber a, b, d;

	if (dn_gt(n, &const_15))
		decNumberCopy(&a, &const_0_85);
	else
		dn_1(&a);

	dn_multiply(&b, &a, n);
	dn_minus(&a, &b);
	dn_exp(&b, &a);
	if (dn_lt(p, &b)) {
		dn_div2(&a, n);
		decNumberLnGamma(&b, &a);
		dn_divide(guess, &b, &a);
		dn_exp(&b, guess);
		dn_multiply(guess, p, &a);
		decNumberRecip(&d, &a);
		dn_power(&a, guess, &d);
		dn_multiply(&d, &a, &b);
		dn_mul2(guess, &d);
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
		if (dn_lt(&b, guess)) {
			dn_ln1m(&a, p);
			dn_multiply(&b, &const_150, n);
			dn_divide(&d, &a, &b);
			dn_inc(&d);
			dn_multiply(guess, guess, &d);
		}
	}
	if (dn_le(guess, &const_1e_400))
		decNumberZero(guess);
}

// 464 bytes
decNumber *qf_chi2(decNumber *r, const decNumber *p) {
	decNumber v;

	if (chi2_param(r, &v, p))
		return r;
	qf_chi2_est(r, &v, p);
	if (dn_eq0(r))
		return r;
	return newton_qf(r, p, NEWTON_NONNEGATIVE, &pdf_chi2_helper, &cdf_chi2_helper, &v, NULL, &const_0_04);
}



// 56 bytes
static int t_param(decNumber *r, decNumber *v, const decNumber *x) {
	dist_one_param(v);
	if (param_positive_int(r, v))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

// 154 bytes
decNumber *pdf_T_helper(decNumber *r, const decNumber *x, const decNumber *v, const decNumber *null) {
	decNumber t, u, w;

	dn_div2(&t, v);					// t=v/2
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
	dn_mulPI(&w, v);
	dn_sqrt(&t, &w);
	return dn_divide(r, &u, &t);
}

// 44 bytes
decNumber *pdf_T(decNumber *r, const decNumber *x) {
	decNumber v;

	if (t_param(r, &v, x))
		return r;
	return pdf_T_helper(r, x, &v, NULL);
}

// 150 bytes
decNumber *cdf_T_helper(decNumber *r, const decNumber *x, const decNumber *v, const decNumber *null) {
	decNumber t, u;
	int invert;

	if (decNumberIsInfinite(x)) {
		if (decNumberIsNegative(x))
			return decNumberZero(r);
		return dn_1(r);
	}
#if 0
	if (decNumberIsInfinite(v))			// Normal in the limit
		return cdf_Q(r, x);
#endif
	if (dn_eq0(x))
		return decNumberCopy(r, &const_0_5);
	invert = ! decNumberIsNegative(x);
	decNumberSquare(&t, x);
	dn_add(r, &t, v);
	dn_divide(&t, v, r);
	dn_div2(r, v);
	betai(&u, r, &const_0_5, &t);
	dn_div2(r, &u);
	if (invert)
		dn_1m(r, r);
	return r;
}

// 44 bytes
decNumber *cdf_T(decNumber *r, const decNumber *x) {
	decNumber v;

	if (t_param(r, &v, x))
		return r;
	return cdf_T_helper(r, x, &v, NULL);
}

static void qf_T_est(decNumber *r, const decNumber *df, const decNumber *p, const decNumber *p05) {
	const int invert = decNumberIsNegative(p05);
	int negate;
	decNumber a, b, u, pc, pc05, x, x2, x3;

	if (invert) {
		dn_1m(&pc, p);
		p = &pc;
		dn_minus(&pc05, p05);
		p05 = &pc05;
	}
	dn_ln(&a, p);
	dn_minus(&a, &a);
	dn_multiply(&b, df, &const_1_7);
	if (dn_lt(&a, &b)) {
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
		dn_mul2(&x2, df);
		dn_m1(&b, &x2);
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

// 572 bytes
static int qf_T_init(decNumber *r, decNumber *v, const decNumber *x) {
	decNumber a, b, c, d;

	if (t_param(r, v, x))
		return 1;
	dn_subtract(&b, &const_0_5, x);
	if (dn_eq0(&b)) {
		decNumberZero(r);
		return 1;
	}
#if 0
	if (decNumberIsInfinite(v)) {			// Normal in the limit
		qf_Q(r, x);
		return 1;
	}
#endif

	if (dn_eq1(v)) {				// special case v = 1
		dn_mulPI(&a, &b);
		dn_sincos(&a, &c, &d);
		dn_divide(&a, &c, &d);			// lower = tan(pi (x - 1/2))
		dn_minus(r, &a);
		return 1;
	}
	if (dn_eq(v, &const_2)) {			// special case v = 2
		dn_1m(&a, x);
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

// 64 bytes
decNumber *qf_T(decNumber *r, const decNumber *x) {
	decNumber ndf;

	if (qf_T_init(r, &ndf, x))
		return r;
	return newton_qf(r, x, 0, &pdf_T_helper, &cdf_T_helper, &ndf, NULL, NULL);
}


// 84 bytes
static int f_param(decNumber *r, decNumber *d1, decNumber *d2, const decNumber *x) {
	dist_two_param(d1, d2);
	if (param_positive_int(r, d1) || param_positive_int(r, d2))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

// 186 btyes
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
	dn_div2(&s, &a);
	dn_div2(&a, d1);
	dn_div2(&b, d2);
	decNumberLnBeta(&c, &a, &b);
	dn_subtract(&a, &s, &c);
	dn_exp(&b, &a);
	return dn_divide(r, &b, x);
}

// 48 bytes
decNumber *pdf_F(decNumber *r, const decNumber *x) {
	decNumber d1, d2;

	if (f_param(r, &d1, &d2, x))
		return r;
	return pdf_F_helper(r, x, &d1, &d2);
}

// 110 bytes
decNumber *cdf_F_helper(decNumber *r, const decNumber *x, const decNumber *v1, const decNumber *v2) {
	decNumber t, u, w;

	if (dn_le0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return dn_1(r);

	dn_multiply(&t, v1, x);
	dn_add(&u, &t, v2);			// u = v1 * x + v2
	dn_divide(&w, &t, &u);			// w = (v1 * x) / (v1 * x + v2)
	dn_div2(&t, v1);
	dn_div2(&u, v2);
	return betai(r, &t, &u, &w);
}

// 48 bytes
decNumber *cdf_F(decNumber *r, const decNumber *x) {
	decNumber v1, v2;

	if (f_param(r, &v1, &v2, x))
		return r;
	return cdf_F_helper(r, x, &v1, &v2);
}

// 44 bytes
static void qf_F_recipm1(decNumber *r, const decNumber *n) {
	decNumber t;
	if (dn_eq0(dn_m1(&t, n)))
		dn_1(r);
	else
		decNumberRecip(r, &t);
}

static decNumber *qf_F_est(decNumber *r, const decNumber *n1, const decNumber *n2, const decNumber *pp) {
	decNumber t, u, dr, h, k;

	qf_F_recipm1(&u, n1);
	qf_F_recipm1(&k, n2);
	dn_add(&t, &u, &k);
	dn_divide(&h, &const_2, &t);
	dn_subtract(&dr, &u, &k);

	dn_subtract(&t, &const_0_5, pp);
	qf_Q_est(r, pp, &t);
	decNumberSquare(&k, r);
	dn_subtract(&u, &k, &const_3);
	dn_divide(&k, &u, &const_6);

	dn_divide(&t, &const_2on3, &h);
	dn_subtract(&u, &const_5on6, &t);
	dn_add(&t, &u, &k);
	dn_multiply(&u, &dr, &t);

	dn_add(&dr, &h, &k);
	dn_sqrt(&t, &dr);
	dn_divide(&dr, &t, &h);
	dn_multiply(&t, &dr, r);
	dn_subtract(r, &t, &u);
	dn_mul2(&u, r);
	return dn_exp(r, &u);
}

// 316 bytes
decNumber *qf_F(decNumber *r, const decNumber *x) {
	decNumber df1, df2;

	if (f_param(r, &df1, &df2, x))
		return r;
	if (dn_eq0(x))
		return decNumberZero(r);

	qf_F_est(r, &df1, &df2, x);
	return newton_qf(r, x, NEWTON_NONNEGATIVE, &pdf_F_helper, &cdf_F_helper, &df1, &df2, &const_0_75);
}

/* Binomial cdf f(k; n, p) = iBeta(n-floor(k), 1+floor(k); 1-p)
 */
// 80 bytes
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

// 96 bytes
decNumber *cdf_B_helper(decNumber *r, const decNumber *x, const decNumber *p, const decNumber *n) {
	decNumber t, u, v;

	if (dn_lt0(x))
		return decNumberZero(r);
	if (dn_lt(n, x))
		return dn_1(r);

	dn_p1(&u, x);
	dn_subtract(&v, n, x);
	dn_1m(&t, p);
	return betai(r, &v, &u, &t);
}

// 56 bytes
decNumber *cdf_B(decNumber *r, const decNumber *x) {
	decNumber n, p, fx;

	if (binomial_param(r, &p, &n, x))
		return r;
	decNumberFloor(&fx, x);
	return cdf_B_helper(r, &fx, &p, &n);
}

// 112 bytes
static void normal_approximation_via_moment(decNumber *y, const decNumber *p, const decNumber *mu, const decNumber *sigma) {
	decNumber x, z;

	dn_subtract(&x, &const_0_5, p);
	qf_Q_est(&z, p, &x);
	decNumberSquare(&x, &z);
	dn_dec(&x);
	dn_divide(y, &x, &const_6);
	dn_divide(&x, y, sigma);
	dn_add(y, &x, &z);
	dn_multiply(&x, y, sigma);
	dn_add(y, &x, mu);
}


static void qf_B_est(decNumber *r, const decNumber *p, const decNumber *prob, const decNumber *n) {
	decNumber mu, sigma;

	dn_multiply(&mu, prob, n);
	dn_1m(&sigma, prob);
	dn_multiply(r, &mu, &sigma);
	dn_sqrt(&sigma, r);
	normal_approximation_via_moment(r, p, &mu, &sigma);
}

// 138 bytes
decNumber *qf_B(decNumber *r, const decNumber *p) {
	decNumber prob, n;

	if (binomial_param(r, &prob, &n, p))
		return r;
	if (check_probability(r, p, 1))
		return r;
	qf_B_est(r, p, &prob, &n);
	newton_qf(r, p, NEWTON_DISCRETE | NEWTON_NONNEGATIVE, NULL, &cdf_B_helper, &prob, &n, NULL);
	return dn_min(r, r, &n);
}

/* Poisson cdf f(k, lam) = 1 - iGamma(floor(k+1), lam) / floor(k)! k>=0
 */
// 56 bytes
static int poisson_param(decNumber *r, decNumber *lambda, const decNumber *x) {
	dist_one_param(lambda);
	if (param_nonnegative(r, lambda))
		return 1;
	if (decNumberIsNaN(x)) {
		set_NaN(r);
		return 1;
	}
	return 0;
}

// 76 bytes
decNumber *cdf_P_helper(decNumber *r, const decNumber *x, const decNumber *lambda, const decNumber *null) {
	decNumber t, u;

	if (dn_lt0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return dn_1(r);

	dn_p1(&u, x);
	decNumberGammap(&t, &u, lambda);
	return dn_1m(r, &t);
}

// 54 bytes
decNumber *cdf_P(decNumber *r, const decNumber *x) {
	decNumber lambda, fx;

	if (poisson_param(r, &lambda, x))
		return r;
	decNumberFloor(&fx, x);
	return cdf_P_helper(r, &fx, &lambda, NULL);
}

static void qf_P_est(decNumber *r, const decNumber *p, const decNumber *lambda) {
	decNumber sigma;

	dn_sqrt(&sigma, lambda);
	normal_approximation_via_moment(r, p, lambda, &sigma);
}

// 98 bytes
decNumber *qf_P(decNumber *r, const decNumber *p) {
	decNumber lambda;

	if (poisson_param(r, &lambda, p))
		return r;
	if (check_probability(r, p, 1))
		return r;
	qf_P_est(r, p, &lambda);
	return newton_qf(r, p, NEWTON_DISCRETE | NEWTON_NONNEGATIVE, NULL, &cdf_P_helper, &lambda, NULL, NULL);
}


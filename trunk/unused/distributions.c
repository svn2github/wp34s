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
 */

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

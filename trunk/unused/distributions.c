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



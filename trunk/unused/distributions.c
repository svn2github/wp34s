/* Distributions:
 *
 * Binomial	570
 * Cauchy	322
 * Exponential	298
 * F		792
 * Geometric	394
 * Poisson	466
 * Q		856
 * T		1106
 * Weibull	450
 * Chi^2	844
 * Logistic	258
 * LogNormal	96
 * Normal	162
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
/* Cauchy distribution 322 bytes in total */

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



/* Exponential distribution cdf = 1 - exp(-lambda . x)
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



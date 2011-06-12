decNumber *cdfu_Q(decNumber *q, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberMinus(&t, x, ctx);
	return cdf_Q(q, &t, ctx);
}

decNumber *cdfu_T(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	decNumberMinus(&t, x, ctx);
	return cdf_Q(r, &t, ctx);
}

decNumber *cdfu_WB(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber k, lam, t;

	if (weibull_param(r, &k, &lam, x, ctx))
		return r;
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	decNumberDivide(&t, x, &lam, ctx);
	decNumberPower(&lam, &t, &k, ctx);
	decNumberMinus(&t, &lam, ctx);
	return decNumberExp(r, &t, ctx);
}

decNumber *cdfu_EXP(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber lam, t, u;

	if (exponential_xform(r, &lam, x, ctx))
		return r;
	if (decNumberIsNegative(x) || decNumberIsZero(x))
		return decNumberCopy(r, &const_1);
	if (decNumberIsInfinite(x))
		return decNumberZero(r);

	decNumberMultiply(&t, &lam, x, ctx);
	decNumberMinus(&u, &t, ctx);
	return decNumberExp(r, &u, ctx);
}

decNumber *cdfu_G(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber p, t, ipx, u;

	if (geometric_param(r, &p, x, ctx))
		return r;
	if (! is_int(x, ctx)) {
		decNumberFloor(&ipx, x, ctx);
		x = &ipx;
	}
	if (dn_le0(x))
		return decNumberCopy(r, &const_1);
	if (decNumberIsInfinite(x))
		return decNumberZero(r);

	dn_ln1m(&t, p);
	dn_multiply(&u, x, &t);
	return dn_exp(r, &u);
}

decNumber *cdfu_normal(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber q, var;

	if (normal_xform(r, &q, x, &var, ctx))
		return r;
	return cdfu_Q(r, &q, ctx);
} 

decNumber *cdfu_lognormal(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber lx;

	decNumberLn(&lx, x, ctx);
	return cdfu_normal(r, &lx, ctx);
}

decNumber *cdfu_logistic(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber a, b, s;

	if (logistic_xform(r, &a, x, &s, ctx))
		return r;
	decNumberTanh(&b, &a, ctx);
	decNumberMultiply(&a, &b, &const__0_5, ctx);
	return decNumberAdd(r, &a, &const_0_5, ctx);
}

decNumber *cdfu_cauchy(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma, ctx))
		return r;
	do_atan(&a, &b, ctx);
	decNumberDivide(&b, &a, &const_PI, ctx);
	return decNumberSubtract(r, &const_0_5, &b, ctx);
}



/* The functions below are fairly naive implementations that will
 * have problems at the extreme lower end.
 */

decNumber *cdfu_chi2(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	cdf_chi2(&t, x, ctx);
	return decNumberSubtract(r, &const_1, &t, ctx);
}

decNumber *cdfu_F(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	cdf_F(&t, x, ctx);
	return decNumberSubtract(r, &const_1, &t, ctx);
}

decNumber *cdfu_B(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	cdf_B(&t, x, ctx);
	return decNumberSubtract(r, &const_1, &t, ctx);
}

decNumber *cdfu_P(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber t;

	cdf_P(&t, x, ctx);
	return decNumberSubtract(r, &const_1, &t, ctx);
}

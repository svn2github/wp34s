static void qf_T_est(decNumber *r, const decNumber *df, const decNumber *p, const decNumber *p05, decContext *ctx) {
	const int invert = decNumberIsNegative(p05);
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
	decNumberCompare(&u, &a, &b, ctx);
	if (dn_lt0(&u)) {
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
	} else {
		decNumberMultiply(&a, df, &const_0_5,ctx);
		decNumberSubtract(&b, &a, &const_0_25, ctx);
		decNumberDivide(&a, &const_PI, &b, ctx);
		decNumberSquareRoot(&b, &a, ctx);
		decNumberMultiply(&a, &b, df, ctx);
		decNumberMultiply(&b, &a, p, ctx);
		decNumberRecip(&a, &b, ctx);
		decNumberPower(&u, &b, &a, ctx);
		decNumberSquareRoot(&a, df, ctx);
		decNumberDivide(r, &a, &u, ctx);
	}
	if (invert)
		decNumberMinus(r, r, ctx);
}

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


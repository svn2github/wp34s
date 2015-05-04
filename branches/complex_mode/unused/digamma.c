// digamma function 1156 bytes



extern const decNumber *const digamma_consts[];


	{ DFLT, "digammaC02",		"-0.08333333333333333333333333333333333333333333333333" },
	{ DFLT, "digammaC04",		"0.00833333333333333333333333333333333333333333333333" },
	{ DFLT, "digammaC06",		"-0.00396825396825396825396825396825396825396825396825" },
	{ DFLT, "digammaC08",		"0.00416666666666666666666666666666666666666666666666" },
	{ DFLT, "digammaC10",		"-0.00757575757575757575757575757575757575757575757575" },
	{ DFLT, "digammaC12",		"0.02109279609279609279609279609279609279609279609279" },
	{ DFLT, "digammaC16",		"0.44325980392156862745098039215686274509803921568627" },
	{ DFLT, "digammaC18",		"-3.05395433027011974380395433027011974380395433027011" },
	{ DFLT, "digammaC20",		"26.45621212121212121212121212121212121212121212121212" },





const decNumber *const digamma_consts[10] = {
	&const_digammaC02,	&const_digammaC04,	&const_digammaC06,
	&const_digammaC08,	&const_digammaC10,	&const_digammaC12,
	&const_digammaC02,	&const_digammaC16,	&const_digammaC18,
	&const_digammaC20
};
#endif

decNumber *decNumberPsi(decNumber *res, const decNumber *xin) {
	decNumber x_2, t, r, x;
	int i;

	if (decNumberIsSpecial(xin)) {
		if (decNumberIsNaN(xin) || decNumberIsNegative(xin))
			return set_NaN(res);
		return set_inf(res);
	}

	// Check for reflection
	if (dn_le0(xin)) {
		if (is_int(xin)) {
			return set_NaN(res);
		}
		dn_mulPI(&x_2, xin);
		dn_sincos(&x_2, &t, &r);
		dn_divide(&x_2, &r, &t);		// x_2 = cot(PI.x)
		dn_mulPI(&t, &x_2);
		dn_1m(&x, xin);
		dn_minus(res, &t);
	} else {
		decNumberZero(res);
		decNumberCopy(&x, xin);
	}

	// Use recurrance relation to bring x large enough for our series to converge
	for (;;) {
		if (dn_lt(&const_8, &x))
			break;
		decNumberRecip(&t, &x);
		dn_subtract(res, res, &t);
		dn_inc(&x);
	}

	// Finally the series approximation
	dn_ln(&t, &x);
	dn_add(res, res, &t);
	dn_multiply(&r, &x, &const__2);
	decNumberRecip(&t, &r);
	dn_add(res, res, &t);

	decNumberSquare(&t, &x);
	decNumberRecip(&x_2, &t);
	decNumberCopy(&r, &x_2);
	for (i=0; i<10; i++) {
		dn_multiply(&t, &r, digamma_consts[i]);
		dn_add(res, res, &t);
		dn_multiply(&r, &r, &x_2);
	}
	return res;
}




// Digamma function
void cmplxPsi(decNumber *rx, decNumber *ry, const decNumber *ain, const decNumber *bin) {
	decNumber a, b, t1, t2, r1, r2, x_2x, x_2y;
	int i;

	if (decNumberIsSpecial(ain) || decNumberIsSpecial(bin)) {
		cmplx_NaN(rx, ry);
		return;
	}

	// Reflection if negative real part
	if (dn_le0(ain)) {
		if (dn_eq0(bin) && is_int(ain)) {
			cmplx_NaN(rx, ry);
			return;
		}
		cmplxMultiplyReal(&r1, &r2, ain, bin, &const_PI);
		cmplxTan(&t1, &t2, &r1, &r2);
		cmplxDivideRealBy(&r1, &r2, &const_PI, &t1, &t2);
		cmplxSubtractFromReal(&a, &b, &const_1, ain, bin);
		cmplxMinus(rx, ry, &r1, &r2);
	} else {
		decNumberZero(rx);
		decNumberZero(ry);
		decNumberCopy(&a, ain);
		decNumberCopy(&b, bin);
	}

	// Recurrance to push real part > 8
	for (;;) {
		dn_compare(&t1, &const_8, &a);
		if (decNumberIsNegative(&t1))
			break;
		cmplxRecip(&t1, &t2, &a, &b);
		cmplxSubtract(rx, ry, rx, ry, &t1, &t2);
		dn_inc(&a);
	}

	// Series approximation
	cmplxLn(&t1, &t2, &a, &b);
	cmplxAdd(rx, ry, rx, ry, &t1, &t2);
	cmplxMultiplyReal(&r1, &r2, &a, &b, &const__2);
	cmplxRecip(&t1, &t2, &r1, &r2);
	cmplxAdd(rx, ry, rx, ry, &t1, &t2);

	cmplxSqr(&t1, &t2, &a, &b);
	cmplxRecip(&x_2x, &x_2y, &t1, &t2);
	cmplxCopy(&r1, &r2, &x_2x, &x_2y);
	for (i=0; i<10; i++) {
		cmplxMultiplyReal(&t1, &t2, &r1, &r2, digamma_consts[i]);
		cmplxAdd(rx, ry, rx, ry, &t1, &t2);
		cmplxMultiply(&r1, &r2, &r1, &r2, &x_2x, &x_2y);
	}
}






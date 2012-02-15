// Special flavours of factorial -- double factorial and subfactorial
// 368 bytes for both

decNumber *decNumberDblFactorial(decNumber *r, const decNumber *x) {
	decNumber t, u, v;

	dn_p2(&t, x);				// t = x+2
	decNumberPow2(&u, &t);			// u = 2^(x+1)
	dn_sqrt(&t, &u);
	dn_multiply(&u, &t, &const_recipsqrt2PI);
	dn_div2(&t, x);
	dn_p1(&v, &t);
	decNumberGamma(&t, &v);
	return dn_multiply(r, &u, &t);
}

decNumber *decNumberSubFactorial(decNumber *r, const decNumber *x) {
	decNumber t, u;

	if (is_int(x)) {
		decNumberFactorial(&t, x);
		dn_divide(&u, &t, &const_e);
		dn_add(&t, &u, &const_0_5);
		return decNumberFloor(r, &t);
	}
	return set_NaN(r);
}






static void cmplxRealPower(decNumber *rx, decNumber *ry,
		const decNumber *r,
		const decNumber *a, const decNumber *b) {
	cmplxPower(rx, ry, r, &const_0, a, b);
}

void cmplx2x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplxRealPower(rx, ry, &const_2, a, b);
}

void cmplxDblFactorial(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber t, u1, u2, v1, v2, w1, w2;

	dn_p1(&t, a);
	cmplx2x(&u1, &u2, &t, b);
	cmplxDivideReal(&v1, &v2, &u1, &u2, &const_PI);
	cmplxSqrt(&u1, &u2, &v1, &v2);
	cmplxDiv2(&v1, &v2, a, b);
	dn_p1(&t, &v1;
	cmplxGamma(&w1, &w2, &t, &v2);
	cmplxMultiply(rx, ry, &w1, &w2, &u1, &u2);
}

// real inverse LamW - 32 bytes
decNumber *decNumberInvW(decNumber *r, const decNumber *x) {
	decNumber t;

	dn_exp(&t, x);
	return dn_multiply(r, &t, x);
}


// real LamW - 432 bytes
decNumber *decNumberLamW(decNumber *r, const decNumber *x) {
	decNumber s, t, u, v, w;
	int i;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x) || decNumberIsNegative(x))
			return set_NaN(r);
		return set_inf(r);
	}

	decNumberRecip(&t, &const_e);
	dn_minus(&s, &t);	// -1/e
	dn_compare(&t, &s, x);
	if (! decNumberIsNegative(&t)) {
		return set_NaN(r);
	}

	// Make an initial guess as to the value
	dn_compare(&t, &const_500, x);
	if (! decNumberIsNegative(&t)) {
		// x<500, lx1 = ln(x+1); est = .665 * (1 + .0195*lx1) * lx1 + 0.04
		dn_p1(&t, x);
		dn_ln(&w, &t);

		dn_multiply(&s, &const_0_0195, &w);
		dn_p1(&t, &s);
		dn_multiply(&u, &t, &const_0_665);
		dn_multiply(&t, &u, &w);
		dn_add(r, &const_0_04, &t);
	} else {
		// x>=500, est = ln(x-4) - (1 - 1/ln(x)) * ln(ln(x))
		dn_ln(&w, x);
		dn_ln(&t, &w);
		decNumberRecip(r, &w);
		dn_1m(&s, r);
		dn_multiply(r, &s, &t);

		dn_subtract(&s, x, &const_4);
		dn_ln(&t, &s);

		dn_subtract(r, &t, r);
	}

	for (i=0; i<20; i++) {
		// Now iterate to refine the estimate
		dn_p1(&u, r);			// u = wj + 1
		dn_exp(&t, r);			// t = e^wj
		dn_multiply(&s, &u, &t);	// s = (wj+1)e^wj

		dn_p1(&v, &u);			// v = wj + 2
		dn_mul2(&w, &u);		// w = 2wj + 2
		dn_divide(&u, &v, &w);		// u = (wj+2)/(2wj+2)
		dn_multiply(&w, &t, r);		// w = wj e^wj

		// Check for termination w, x, u & s are live here
		dn_subtract(&v, x, &w);	// v = x - wj e^wj
		dn_divide(&t, &v, &s);
		dn_abs(&t, &t);
		dn_compare(&t, &t, &const_1e_32);
		if (decNumberIsNegative(&t))
			break;

		// Continue the increment update
		dn_minus(&v, &v);		// v = wj e^wj - x
		dn_multiply(&t, &v, &u);	// t = (wj+2).(wj e^wj - x) / (2wj + 2)
		dn_subtract(&w, &s, &t);	// w = denominator
		dn_divide(&t, &v, &w);
		dn_subtract(r, r, &t);	// wj+1
	}
	return r;
}



// complex inverse LamW - 44 bytes
void cmplxInvW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber t1, t2;

	cmplxExp(&t1, &t2, a, b);
	cmplxMultiply(rx, ry, &t1, &t2, a, b);
}


// complex LamW - 624 bytes
void cmplxlamW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber s1, s2, t1, t2, u1, u2, v1, v2, w1, w2;
	int i;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b)) {
		if (decNumberIsNaN(a) || decNumberIsNaN(b))
			cmplx_NaN(rx, ry);
		else {
			set_inf(rx);
			decNumberZero(ry);
		}
		return;
	}

	// Make an initial guess as to the value
	dn_compare(&t1, &const_500, a);
	if (! decNumberIsNegative(&t1)) {
		// x<500, lx1 = ln(x+1); est = .665 * (1 + .0195*lx1) * lx1 + 0.04
		dn_p1(&t1, a);
		cmplxLn(&w1, &w2, &t1, b);				// ln(1+a,b)
		cmplxMultiplyReal(&t1, &s2, &w1, &w2, &const_0_0195);
		dn_p1(&s1, &t1);
		cmplxMultiplyReal(&u1, &u2, &s1, &s2, &const_0_665);
		cmplxMultiply(&t1, ry, &u1, &u2, &w1, &w2);
		dn_add(rx, &const_0_04, &t1);
	} else {
		// x>=500, est = ln(x-4) - (1 - 1/ln(x)) * ln(ln(x))
		cmplxLn(&w1, &w2, a, b);				// ln(a,b)
		cmplxLn(&t1, &t2, &w1, &w2);			// ln(ln(a,b))
		cmplxRecip(&u1, &u2, &w1, &w2);			// 1 / ln(a,b)
		cmplxSubtractFromReal(&s1, &s2, &const_1, &u1, &u2);// 1 - 1 / ln(a,b)
		cmplxMultiply(&u1, &u2, &s1, &s2, &t1, &t2);

		dn_subtract(&s1, a, &const_4);
		cmplxLn(&t1, &t2, &s1, b);				// ln(a-4,b)

		cmplxSubtract(rx, ry, &t1, &t2, &u1, &u2);
	}

	for (i=0; i<20; i++) {
		// Now iterate to refine the estimate
		dn_p1(&u1, rx);					// u = wj + 1	(u1,ry)
		cmplxExp(&t1, &t2, rx, ry);			// t = e^wj
		cmplxMultiply(&s1, &s2, &u1, ry, &t1, &t2);	// (s1,s2) = (wj+1)e^wj

		dn_p1(&v1, &u1);				// v = wj + 2	(v1,ry)
		dn_add(&w1, &u1, &u1);				// w = 2wj + 2
		dn_add(&w2, ry, ry);
		cmplxDivide(&u1, &u2, &v1, ry, &w1, &w2);		// (u1,u2) = (wj+2)/(2wj+2)
		cmplxMultiply(&w1, &w2, &t1, &t2, rx, ry);		// (w1,w2) = wj e^wj

		// Check for termination w, x, u & s are live here
		cmplxSubtract(&v1, &v2, a, b, &w1, &w2);		// (v1,v2) = x - wj e^wj
		cmplxDivide(&t1, &t2, &v1, &v2, &s1, &s2);
		cmplxR(&t2, &t1, &t2);
		dn_compare(&t1, &t2, &const_1e_32);
		if (decNumberIsNegative(&t1))
			break;

		// Continue the increment update
		cmplxMinus(&v1, &v2, &v1, &v2);
		cmplxSubtract(&v1, &v2, &w1, &w2, a, b);		// (v1,v2) = wj e^wj - x
		cmplxMultiply(&t1, &t2, &v1, &v2, &u1, &u2);	// t = (wj+2).(wj e^wj - x) / (2wj + 2)
		cmplxSubtract(&w1, &w2, &s1, &s2, &t1, &t2);	// w = denominator
		cmplxDivide(&t1, &t2, &v1, &v2, &w1, &w2);
		cmplxSubtract(rx, ry, rx, ry, &t1, &t2);		// wj+1
	}
}


#if 0
/* Standard normal support routines */
decNumber *pdf_Q(decNumber *q, const decNumber *x) {
	decNumber r, t;

	decNumberSquare(&t, x);
	dn_div2(&r, &t);
	dn_minus(&t, &r);
	dn_exp(&r, &t);
	return dn_multiply(q, &r, &const_recipsqrt2PI);
}

/* Smart CDF helper routine that return three values */ 
void cdf_Q_helper(enum nilop op) {
	decNumber a, b, t, u, x2, d, absx, x;
	int i;

	getX(&x);
	dn_abs(&absx, &x);
	if (dn_lt(&absx, &const_2_326)) {
		decNumberSquare(&x2, &absx);
		decNumberCopy(&t, &absx);
		decNumberCopy(&a, &absx);
		decNumberCopy(&d, &const_3);
		for (i=0;i<500; i++) {
			dn_multiply(&u, &t, &x2);
			dn_divide(&t, &u, &d);
			dn_add(&u, &a, &t);
			if (dn_eq(&u, &a))
				break;
			decNumberCopy(&a, &u);
			dn_p2(&d, &d);
		}
		decNumberCopy(&b, &const_0_5);
		if (decNumberIsNegative(&x))
			dn_minus(&a, &a);
	} else {
		const decNumber *nom, *extra, *sub;
		//dn_minus(&x2, &absx);
		//n = ceil(extra + nom / (|x| - sub))
		if (is_usrdblmode()) {
			sub = &const_1_5;
			nom = &const_300;
			extra = &const_8;
		}
		else {
			sub = &const_1_3;
			nom = &const_100;
			extra = &const_4;
		}
		dn_subtract(&b, &absx, sub);
		dn_divide(&t, nom, &b);
		dn_add(&u, &t, extra);
		decNumberCeil(&b, &u);
		decNumberZero(&t);
		do {
			dn_add(&u, &x, &t);
			dn_divide(&t, &b, &u);
			dn_dec(&b);
		} while (! dn_eq0(&b));

		dn_add(&u, &t, &x);
		decNumberRecip(&a, &u);

		if (decNumberIsNegative(&a)) {
			dn_minus(&a, &a);
			decNumberZero(&b);
		} else {
			dn_1(&b);
			dn_minus(&a, &a);
		}
	}
	pdf_Q(&t, &x);
	setXYZ(&t, &a, &b);
}
#endif

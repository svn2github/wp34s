

/* Jacobi Elliptical functions */
// 1624 bytes in total

#define ELLIPTIC_N	16

void dn_elliptic(decNumber *sn, decNumber *cn, decNumber *dn, const decNumber *u, const decNumber *m) {
	decNumber a, b, e, f, g;
	decNumber s_n, c_n, d_n;
	decNumber MU[ELLIPTIC_N], NU[ELLIPTIC_N], C[ELLIPTIC_N], D[ELLIPTIC_N];
	decNumber sin_umu, cos_umu, t, r;
	int n = 0;
	
#define mu(n)	(MU + (n))
#define nu(n)	(NU + (n))
#define c(n)	(C + (n))
#define d(n)	(D + (n))

	if (sn == NULL) sn = &s_n;
	if (cn == NULL) cn = &c_n;
	if (dn == NULL) dn = &d_n;

	dn_abs(&a, m);
	if (dn_lt(&const_1, &a)) {
		cmplx_NaN(sn, cn);
		set_NaN(dn);
		return;
	}
	if (dn_lt(&a, &const_1e_32)) {
		dn_sincos(u, sn, cn);
		dn_1(dn);
		return;
	}
	dn_m1(&a, m);
	if (dn_abs_lt(&a, &const_1e_32)) {
		dn_sinhcosh(u, &a, &b);
		decNumberRecip(cn, &b);
		dn_multiply(sn, &a, cn);
		decNumberCopy(dn, cn);
		return;
	}
	dn_1(mu(0));
	dn_1m(&a, m);
	dn_sqrt(nu(0), &a);
	for (;;) {
		dn_add(&g, mu(n), nu(n));
		dn_abs(&a, &g);
		dn_mulpow10(&b, &a, 32);
		dn_mul2(&a, &b);
		dn_subtract(&e, mu(n), nu(n));
		dn_abs(&f, &e);
		if (dn_gt(&a, &f))
			break;
		dn_div2(mu(n+1), &g);
		dn_multiply(&a, mu(n), nu(n));
		dn_sqrt(nu(n+1), &a);
		n++;
		if (n >= ELLIPTIC_N-1)
			break;
	}

	dn_multiply(&a, u, mu(n));
	dn_sincos(&a, &sin_umu, &cos_umu);
	if (dn_abs_lt(&sin_umu, dn_abs(&b, &cos_umu)))
		dn_divide(&t, &sin_umu, &cos_umu);
	else
		dn_divide(&t, &cos_umu, &sin_umu);

	dn_multiply(c(n), mu(n), &t);
	dn_1(d(n));

	while (n > 0) {
		n--;
		dn_multiply(c(n), d(n+1), c(n+1));
		decNumberSquare(&a, c(n+1));
		dn_divide(&r, &a, mu(n+1));
		dn_add(&a, &r, nu(n));
		dn_add(&b, &r, mu(n));
		dn_divide(d(n), &a, &b);
	}
	cmplxAbs(&f, &b, &const_1, c(0));
	if (decNumberIsNegative(&e)) {
		dn_1m(&a, m);
		dn_sqrt(&g, &a);
		dn_divide(dn, &g, d(0));

		dn_divide(cn, dn, &f);
		if (decNumberIsNegative(&cos_umu))
			dn_minus(cn, cn);

		dn_divide(&a, c(0), &g);
		dn_multiply(sn, cn, &a);
	} else {
		decNumberCopy(dn, d(0));

		dn_divide(sn, &const_1, &f);
		if (decNumberIsNegative(&sin_umu))
			dn_minus(sn, sn);
		dn_multiply(cn, c(0), sn);
	}
#undef mu
#undef nu
#undef c
#undef d
}

decNumber *decNumberSN(decNumber *res, const decNumber *k, const decNumber *u) {
	dn_elliptic(res, NULL, NULL, u, k);
	return res;
}

decNumber *decNumberCN(decNumber *res, const decNumber *k, const decNumber *u) {
	dn_elliptic(NULL, res, NULL, u, k);
	return res;
}

decNumber *decNumberDN(decNumber *res, const decNumber *k, const decNumber *u) {
	dn_elliptic(NULL, NULL, res, u, k);
	return res;
}








static void elliptic_setup(decNumber *r,
		decNumber *snuk, decNumber *cnuk, decNumber *dnuk,
		decNumber *snvki, decNumber *cnvki, decNumber *dnvki,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki) {
	decNumber a;

	dn_elliptic(snuk, cnuk, dnuk, u, k);
	dn_elliptic(snvki, cnvki, dnvki, v, ki);

	dn_multiply(&a, dnuk, snvki);
	decNumberSquare(r, &a);
	dn_1m(&a, r);
	decNumberRecip(r, &a);
}


// SN(u + i v, k + i ki) = sn(u, k) . dn(v, ki) / denom
//		  + i . cn(u, k) . dn(u, k) . sn(v, ki) . cn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxSN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki) {
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki);

	dn_multiply(&a, &snuk, &dnvki);
	dn_multiply(rx, &a, &denom);

	dn_multiply(&a, &cnuk, &dnuk);
	dn_multiply(&b, &a, &snvki);
	dn_multiply(&a, &b, &cnvki);
	dn_multiply(ry, &a, &denom);
}


// CN(u + i v, k + i ki) = cn(u, k) . cn(v, ki) / denom
//		  - i . sn(u, k) . dn(u, k) . sn(v, ki) . dn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxCN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki) {
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki);

	dn_multiply(&a, &cnuk, &cnvki);
	dn_multiply(rx, &a, &denom);

	dn_multiply(&a, &snuk, &dnuk);
	dn_multiply(&b, &a, &snvki);
	dn_multiply(&a, &b, &dnvki);
	dn_multiply(&b, &a, &denom);
	dn_minus(ry, &b);
}


// DN(a + i b, c + i d) = dn(u, k) . cn(v, ki) . dn(v, ki) / denom
//		  - i . k^2 . sn(u, k) . cn(u, k) . sn(v, ki) / denom
// where
//	denom = 1 - dn(u, k)^2 * sn(v, ki)^2
void cmplxDN(decNumber *rx, decNumber *ry,
		const decNumber *u, const decNumber *v,
		const decNumber *k, const decNumber *ki) {
	decNumber denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
	decNumber a, b;

	elliptic_setup(&denom, &snuk, &cnuk, &dnuk,
			&snvki, &cnvki, &dnvki,
			u, v, k, ki);

	dn_multiply(&a, &dnuk, &cnvki);
	dn_multiply(&b, &a, &dnvki);
	dn_multiply(rx, &b, &denom);

	decNumberSquare(&a, k);
	dn_minus(&b, &a);
	dn_multiply(&a, &b, &snuk);
	dn_multiply(&b, &a, &cnuk);
	dn_multiply(&a, &b, &snvki);
	dn_multiply(ry, &a, &denom);
}

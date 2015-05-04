// Bessel functions
// 1704 bytes for real, about twice that for complex but the latter don't
// work properly yet.

static decNumber *dn_bessel(decNumber *res, const decNumber *alpha, const decNumber *x, const int neg) {
	decNumber q, r, m;
	decNumber x2on4, term, gfac;
	int n;

	dn_div2(&q, x);				// q = x/2
	decNumberSquare(&x2on4, &q);		// factor each time around
	dn_power(&r, &q, alpha);		// (x/2)^(2m+alpha)

	dn_p1(&gfac, alpha);
	decNumberGamma(&q, &gfac);
	dn_divide(&term, &r, &q);
	decNumberCopy(res, &term);			// first term in series

	decNumberZero(&m);

	for (n=0; n<1000; n++) {
		dn_multiply(&q, &term, &x2on4);
		dn_inc(&m);			// m = m+1
		dn_divide(&r, &q, &m);
		dn_divide(&term, &r, &gfac);
		dn_inc(&gfac);
		if (neg)
			dn_minus(&term, &term);
		dn_add(&q, &term, res);
		if (dn_eq(&q, res))
			return res;
		decNumberCopy(res, &q);
	}
	return set_NaN(res);
}

decNumber *decNumberBSJN(decNumber *res, const decNumber *alpha, const decNumber *x) {
	decNumber a;

	if (decNumberIsNaN(alpha) || decNumberIsSpecial(x))
		return set_NaN(res);
	if (dn_eq0(x)) {
		if (dn_eq0(alpha))
			return dn_1(res);
		return decNumberZero(res);
	}
	if (decNumberIsNegative(alpha) && is_int(alpha)) {
		dn_abs(&a, alpha);
		alpha = &a;
	}
	dn_bessel(res, alpha, x, 1);
	return res;
}

decNumber *decNumberBSIN(decNumber *res, const decNumber *alpha, const decNumber *x) {
	decNumber a;

	if (decNumberIsNaN(alpha) || decNumberIsNaN(x))
		return set_NaN(res);
	else if (decNumberIsInfinite(x))
		return set_inf(res);
	else if (dn_eq0(x)) {
		if (dn_eq0(alpha))
			return dn_1(res);
		else
			return decNumberZero(res);
	} else {
		if (decNumberIsNegative(alpha) && is_int(alpha)) {
			dn_abs(&a, alpha);
			alpha = &a;
		}
		dn_bessel(res, alpha, x, 0);
	}
	return res;
}

// See A&S page 360 section 9.1.11
static void bessel2_int_series(decNumber *res, const decNumber *n, const decNumber *x, int modified) {
	const decNumber *const factor = modified?&const_0_5:&const__1onPI;
	decNumber xon2, xon2n, x2on4;
	decNumber k, npk, t, u, v, s, p, nf, absn;
	int i, in, n_odd, n_neg;

	if (decNumberIsNegative(n)) {
		n = dn_abs(&absn, n);
		n_neg = 1;
	} else	n_neg = 0;
	in = dn_to_int(n);
	n_odd = in & 1;

	dn_div2(&xon2, x);			// xon2 = x/2
	dn_power(&xon2n, &xon2, n);		// xon2n = (x/2)^n
	decNumberSquare(&x2on4, &xon2);		// x2on4 = +/- x^2/4

	if (modified)
		dn_minus(&x2on4, &x2on4);
	if (in > 0) {
		dn_m1(&v, n);			// v = n-k-1 = n-1
		decNumberZero(&k);
		decNumberGamma(&p, n);		// p = (n-1)!
		decNumberCopy(&s, &p);
		dn_multiply(&nf, &p, n);	// nf = n!  (for later)
		for (i=1; i<in; i++) {
			dn_divide(&t, &p, &v);
			dn_dec(&v);
			dn_inc(&k);
			dn_multiply(&u, &t, &k);
			dn_multiply(&p, &u, &x2on4);
			dn_add(&s, &s, &p);
		}
		dn_multiply(&t, &s, factor);
		dn_divide(res, &t, &xon2n);
	} else {
		decNumberZero(res);
		dn_1(&nf);
	}

	if (modified) {
		decNumberBSIN(&t, n, x);
		if (!n_odd)
			dn_minus(&t, &t);
	} else {
		decNumberBSJN(&u, n, x);
		dn_divide(&t, &u, &const_PIon2);
	}
	dn_ln(&u, &xon2);
	dn_multiply(&v, &u, &t);
	dn_add(res, res, &v);

	dn_minus(&x2on4, &x2on4);
	dn_p1(&t, n);				// t = n+1
	decNumberPsi(&u, &t);			// u = Psi(n+1)
	dn_subtract(&v, &u, &const_egamma);	// v = psi(k+1) + psi(n+k+1)
	decNumberZero(&k);
	decNumberCopy(&npk, n);
	decNumberRecip(&p, &nf);			// p = (x^2/4)^k/(k!(n+k)!)
	dn_multiply(&s, &v, &p);

	for (i=0;i<1000;i++) {
		dn_inc(&k);
		dn_inc(&npk);
		dn_multiply(&t, &p, &x2on4);
		dn_multiply(&u, &k, &npk);
		dn_divide(&p, &t, &u);

		decNumberRecip(&t, &k);
		dn_add(&u, &v, &t);
		decNumberRecip(&t, &npk);
		dn_add(&v, &u, &t);

		dn_multiply(&t, &v, &p);
		dn_add(&u, &t, &s);
		if (dn_eq(&u, &s))
			break;
		decNumberCopy(&s, &u);
	}
	dn_multiply(&t, &s, &xon2n);
	if (modified) {
		if (n_odd)
			dn_multiply(&u, &t, &const__0_5);
		else
			dn_div2(&u, &t);
	} else
		dn_multiply(&u, &t, &const__1onPI);
	dn_add(res, res, &u);
	if (!modified && n_neg)
		dn_minus(res, res);
}


decNumber *decNumberBSYN(decNumber *res, const decNumber *alpha, const decNumber *x) {
	decNumber t, u, s, c;

	if (decNumberIsNaN(alpha) || decNumberIsSpecial(x))
		return set_NaN(res);
	else if (dn_eq0(x))
		return set_neginf(res);
	else if (decNumberIsInfinite(alpha) || decNumberIsNegative(x))
		return set_NaN(res);
	else if (!is_int(alpha)) {
		dn_mulPI(&t, alpha);
		dn_sincos(&t, &s, &c);
		dn_bessel(&t, alpha, x, 1);
		dn_multiply(&u, &t, &c);
		dn_minus(&c, alpha);
		dn_bessel(&t, &c, x, 1);
		dn_subtract(&c, &u, &t);
		dn_divide(res, &c, &s);
	} else
		bessel2_int_series(res, alpha, x, 0);
	return res;
}

decNumber *decNumberBSKN(decNumber *res, const decNumber *alpha, const decNumber *x) {
	decNumber t, u, v;

	if (decNumberIsNaN(alpha) || decNumberIsNaN(x))
		return set_NaN(res);
	else if (dn_eq0(x))
		return set_inf(res);
	else if (decNumberIsInfinite(alpha) || decNumberIsNegative(x))
		return set_NaN(res);
	else if (decNumberIsInfinite(x))
		return decNumberZero(res);
	else if (!is_int(alpha)) {
		dn_bessel(&t, alpha, x, 0);
		dn_minus(&u, alpha);
		dn_bessel(&v, &u, x, 0);
		dn_subtract(&u, &v, &t);
		dn_multiply(&v, &u, &const_PIon2);

		dn_mulPI(&t, alpha);
		dn_sincos(&t, &u, NULL);
		dn_divide(res, &v, &u);
	} else
		bessel2_int_series(res, alpha, x, 1);
	return res;
}







static void cmplx_bessel(decNumber *rx, decNumber *ry,
			const decNumber *nx, const decNumber *ny,
			const decNumber *xx, const decNumber *xy, const int neg) {
	decNumber x2on4x, x2on4y, k, ux, uy;
	decNumber t1, t2, a1, a2, b;
	int i;

	cmplxDiv2(&t1, &t2, xx, xy);
	cmplxSqr(&x2on4x, &x2on4y, &t1, &t2);

	cmplxPower(&t1, &t2, &t1, &t2, nx, ny);
	dn_p1(&b, nx);
	cmplxGamma(&a1, &a2, &b, ny);
	cmplxDivide(&ux, &uy, &t1, &t2, &a1, &a2);

	dn_1(&k);
	cmplxCopy(rx, ry, &ux, &uy);

	for (i=0;i<1000;i++) {
		cmplxMultiply(&t1, &t2, &x2on4x, &x2on4y, &ux, &uy);
		if (neg)
			cmplxMinus(&t1, &t2, &t1, &t2);
		dn_add(&b, nx, &k);
		cmplxMultiplyReal(&a1, &a2, &b, ny, &k);
		cmplxDivide(&ux, &uy, &t1, &t2, &a1, &a2);

		cmplxAdd(&a1, &a2, &ux, &uy, rx, ry);
		cmplxSubtract(&t1, &t2, &a1, &a2, rx, ry);
		cmplxR(&b, &t1, &t2);
		if (dn_eq0(&b))
			return;
		cmplxCopy(rx, ry, &a1, &a2);
		dn_inc(&k);
	}
	cmplx_NaN(rx, ry);
}

void cmplxBSJN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy) {
	if (dn_eq0(xy) && dn_eq0(alphay)) {
		decNumberBSJN(rx, alphax, xx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else if (decNumberIsSpecial(alphax) || decNumberIsSpecial(alphay) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else
		cmplx_bessel(rx, ry, alphax, alphay, xx, xy, 1);
}

void cmplxBSIN(decNumber *rx, decNumber *ry,
		const decNumber *alphax, const decNumber *alphay,
		const decNumber *xx, const decNumber *xy) {
	if (dn_eq0(xy) && dn_eq0(alphay)) {
		decNumberBSIN(rx, alphax, xx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else if (decNumberIsSpecial(alphax) || decNumberIsSpecial(alphay) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else
		cmplx_bessel(rx, ry, alphax, alphay, xx, xy, 0);
}


// See A&S page 360 section 9.1.11
static void cmplx_bessel2_int_series(decNumber *rx, decNumber *ry, const decNumber *n, const decNumber *x, const decNumber *y, int modified) {
	const decNumber *const factor = modified?&const_0_5:&const__1onPI;
	decNumber xon2x, xon2y, xon2nx, xon2ny, x2on4x, x2on4y;
	decNumber k, npk, tx, ty, ux, uy, vx, vy, sx, sy, px, py, nf, absn;
	int i, in, n_odd, n_neg;

	if (decNumberIsNegative(n)) {
		n = dn_abs(&absn, n);
		n_neg = 1;
	} else	n_neg = 0;
	in = dn_to_int(n);
	n_odd = in & 1;

	cmplxDiv2(&xon2x, &xon2y, x, y);			// xon2 = x/2
	cmplxPowerReal(&xon2nx, &xon2ny, &xon2x, &xon2y, n);	// xon2n = (x/2)^n
	cmplxSqr(&x2on4x, &x2on4y, &xon2x, &xon2y);		// x2on4 = +/- x^2/4

	if (modified)
		cmplxMinus(&x2on4x, &x2on4y, &x2on4x, &x2on4y);
	if (in > 0) {
		dn_m1(&vx, n);			// v = n-k-1 = n-1
		decNumberZero(&k);
		decNumberGamma(&px, n);		// p = (n-1)!
		decNumberZero(&py);
		decNumberZero(&sy);
		decNumberCopy(&sx, &px);
		dn_multiply(&nf, &px, n);	// nf = n!  (for later)
		for (i=1; i<in; i++) {
			cmplxDivideReal(&tx, &ty, &px, &py, &vx);
			dn_dec(&vx);
			dn_inc(&k);
			cmplxMultiplyReal(&ux, &uy, &tx, &ty, &k);
			cmplxMultiply(&px, &py, &ux, &uy, &x2on4x, &x2on4y);
			cmplxAdd(&sx, &sy, &sx, &sy, &px, &py);
		}
		cmplxMultiplyReal(&tx, &ty, &sx, &sy, factor);
		cmplxDivide(rx, ry, &tx, &ty, &xon2nx, &xon2ny);
	} else {
		decNumberZero(rx);
		decNumberZero(ry);
		dn_1(&nf);
	}

	if (modified) {
		cmplxBSIN(&tx, &ty, n, &const_0, x, y);
		if (!n_odd)
			cmplxMinus(&tx, &ty, &tx, &ty);
	} else {
		cmplxBSJN(&ux, &uy, n, &const_0, x, y);
		cmplxDivideReal(&tx, &ty, &ux, &uy, &const_PIon2);
	}
	cmplxLn(&ux, &uy, &xon2x, &xon2y);
	cmplxMultiply(&vx, &vy, &ux, &uy, &tx, &ty);
	cmplxAdd(rx, ry, rx, ry, &vx, &vy);

	cmplxMinus(&x2on4x, &x2on4y, &x2on4x, &x2on4y);
	dn_p1(&tx, n);					// t = n+1
	decNumberPsi(&ux, &tx);				// u = Psi(n+1)
	dn_subtract(&vx, &ux, &const_egamma);	// v = psi(k+1) + psi(n+k+1)
	decNumberZero(&k);
	decNumberZero(&py);
	decNumberZero(&sy);
	decNumberCopy(&npk, n);
	decNumberRecip(&px, &nf);			// p = (x^2/4)^k/(k!(n+k)!)
	dn_multiply(&sx, &vx, &px);

	for (i=0;i<1000;i++) {
		dn_inc(&k);
		dn_inc(&npk);
		cmplxMultiply(&tx, &ty, &px, &py, &x2on4x, &x2on4y);
		dn_multiply(&ux, &k, &npk);
		cmplxDivideReal(&px, &py, &tx, &ty, &ux);

		decNumberRecip(&tx, &k);
		dn_add(&ux, &vx, &tx);
		decNumberRecip(&tx, &npk);
		dn_add(&vx, &ux, &tx);

		cmplxMultiplyReal(&tx, &ty, &px, &py, &vx);
		cmplxAdd(&ux, &uy, &tx, &ty, &sx, &sy);
		cmplxSubtract(&tx, &ty, &ux, &uy, &sx, &sy);
		cmplxR(&vy, &tx, &ty);
		if (dn_eq0(&vy))
			break;
		cmplxCopy(&sx, &sy, &ux, &uy);
	}
	cmplxMultiply(&tx, &ty, &sx, &sy, &xon2nx, &xon2ny);
	if (modified) {
		if (n_odd)
			cmplxMultiplyReal(&ux, &uy, &tx, &ty, &const__0_5);
		else
			cmplxDiv2(&ux, &uy, &tx, &ty);
	} else
		cmplxMultiplyReal(&ux, &uy, &tx, &ty, &const__1onPI);
	cmplxAdd(rx, ry, rx, ry, &ux, &uy);
	if (!modified && n_neg)
		cmplxMinus(rx, ry, rx, ry);
}


// Yv(z) = ( Jv(z).cos(v PI) - J-v(z) ) / sin(v PI)
//Y(3.5+4.5i)(1.5+2.5i) = -1.19957042014349569 + 2.34640749171622195334 i
void cmplxBSYN(decNumber *rx, decNumber *ry,
		const decNumber *vx, const decNumber *vy,
		const decNumber *xx, const decNumber *xy) {
	decNumber t1, t2, sx, sy, cx, cy, jnx, jny, jmx, jmy;
#if 0
	if (dn_eq0(xy) && dn_eq0(vy)) {
		decNumberBSYN(rx, vx, xx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else
#endif
	if (decNumberIsSpecial(vx) || decNumberIsSpecial(vy) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else if (dn_eq0(vy) && is_int(vx)) {
		cmplx_bessel2_int_series(rx, ry, vx, xx, xy, 0);
	} else {
		cmplx_bessel(&jnx, &jny, vx, vy, xx, xy, 1);
		cmplxMinus(&t1, &t2, vx, vy);
		cmplx_bessel(&jmx, &jmy, &t1, &t2, xx, xy, 1);
		cmplxMultiplyReal(&t1, &t2, vx, vy, &const_PI);
		cmplx_sincos(&t1, &t2, &sx, &sy, &cx, &cy);
		cmplxMultiply(&t1, &t2, &jnx, &jny, &cx, &cy);
		cmplxSubtract(&jnx, &jny, &t1, &t2, &jmx, &jmy);
		cmplxDivide(rx, ry, &jnx, &jny, &sx, &sy);
	}
}

// Kn(x) = PI/2 . (I-n(x) - In(x)) / sin(n PI)
//K(3.5+4.5i)(1.5+2.5i) = 2.41061613980770099 + 1.0563528861789105228i
void cmplxBSKN(decNumber *rx, decNumber *ry,
		const decNumber *vx, const decNumber *vy,
		const decNumber *xx, const decNumber *xy) {
	decNumber t1, t2, inx, iny, imx, imy;

#if 0
	if (dn_eq0(xy) && dn_eq0(vy)) {
		decNumberBSKN(rx, vx, xx);
		if (decNumberIsNaN(rx))
			set_NaN(ry);
		else	decNumberZero(ry);
	} else
#endif
	if (decNumberIsSpecial(vx) || decNumberIsSpecial(vy) ||
			decNumberIsSpecial(xx) || decNumberIsSpecial(xy))
		cmplx_NaN(rx, ry);
	else if (dn_eq0(vy) && is_int(vx)) {
		cmplx_bessel2_int_series(rx, ry, vx, xx, xy, 1);
	} else {
		cmplx_bessel(&inx, &iny, vx, vy, xx, xy, 0);
		cmplxMinus(&t1, &t2, xx, xy);
		cmplx_bessel(&imx, &imy, vx, vy, &t1, &t2, 0);
		cmplxSubtract(&t1, &t2, &imx, &imy, &inx, &iny);
		cmplxMultiplyReal(&imx, &imy, &t1, &t2, &const_PIon2);
		cmplxMultiplyReal(&inx, &iny, vx, vy, &const_PI);
		cmplxSin(&t1, &t2, &inx, &iny);
		cmplxDivide(rx, ry, &imx, &imy, &t1, &t2);
	}
}


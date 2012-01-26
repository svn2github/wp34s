
// conj(a + i b) = a - i b
// 26 bytes
void cmplxConj(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumberCopy(rx, a);
	dn_minus(ry, b);
}

// 26 bytes
void cmplxRnd(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumberRnd(rx, a);
	decNumberRnd(ry, b);
}

// 26 bytes
void cmplxFrac(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumberFrac(rx, a);
	decNumberFrac(ry, b);
}

// 26 bytes
void cmplxTrunc(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumberTrunc(rx, a);
	decNumberTrunc(ry, b);
}

// 38 bytes
void cmplxFactorial(decNumber *rx, decNumber *ry, const decNumber *xin, const decNumber *y) {
	decNumber x;

	dn_p1(&x, xin);
	cmplxGamma(rx, ry, &x, y);
}


// arcsinh(a + i b) = ln((a + i b) + sqrt((a + i b) (a + i b) + 1)
// arcsin(z) = k PI + -i ln (iz + sqrt(1-z^2))
// 46 bytes
void cmplxAsin(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber t;

	dn_minus(&t, a);
	cmplxAsinh(ry, rx, b, &t);
	dn_minus(rx, rx);
}

// arccos(z) = k PI + -i ln(z + sqrt(z^2-1))
// 46 bytes
void cmplxAcos(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber t1, t2;

	cmplxAsin(&t1, &t2, a, b);
	dn_minus(ry, &t2);
	dn_subtract(rx, &const_PIon2, &t1);
}


// atan(z) = i/2 (ln(1 - i z) - ln (1 + i z))
// 116 bytes
void cmplxAtan(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber s1, s2, t1, t2, v1, v2;

	dn_1m(&v1, b);			// 1 + iz = 1 + i(a+ib)	= 1-b + ia
	cmplxLn(&s1, &s2, &v1, a);
	dn_p1(&v1, b);			// 1 - iz = 1 - i(a+ib) = 1+b - ia
	dn_minus(&v2, a);
	cmplxLn(&t1, &t2, &v1, &v2);
	cmplxSubtract(&v1, &v2, &t1, &t2, &s1, &s2);
	cmplxDiv2(ry, &t1, &v1, &v2);
	dn_minus(rx, &t1);
}





// arcsinh(a + i b) = ln((a + i b) + sqrt((a + i b) (a + i b) + 1)
// 126 bytes
void cmplxAsinh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber s1, s2, t1, t2, na, nb;
	int negate;

	negate = decNumberIsNegative(a);
	if (negate) {
		cmplxMinus(&na, &nb, a, b);
		a = &na;
		b = &nb;
	}

	cmplxSqr(&s1, &t2, a, b);

	dn_p1(&t1, &s1);

	cmplxSqrt(&s1, &s2, &t1, &t2);
	dn_add(&t1, &s1, a);
	dn_add(&t2, &s2, b);

	cmplxLn(rx, ry, &t1, &t2);
	if (negate)
		cmplxMinus(rx, ry, rx, ry);
}

// arccosh(a + i b) = ln((a + i b) + sqrt((a + i b) (a + i b) - 1)
// 112 bytes
void cmplxAcosh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber t1, t2, s1, s2, u1, u2;

	dn_p1(&t1, a);
	cmplxSqrt(&s1, &s2, &t1, b);
	dn_m1(&t1, a);
	cmplxSqrt(&u1, &u2, &t1, b);
	cmplxMultiply(&t1, &t2, &s1, &s2, &u1, &u2);
	cmplxAdd(&s1, &s2, a, b, &t1, &t2);
	cmplxLn(rx, ry, &s1, &s2);
}


// arctanh(a + i b) = (1/2)*ln((1 + (a + i b))/(1 - (a + i b)))
// 100 bytes
void cmplxAtanh(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber s1, s2, t1, t2;

	dn_p1(&t1, a);
	cmplxSubtractFromReal(&t2, ry, &const_1, a, b);

	cmplxDivide(&s1, &s2, &t1, b, &t2, ry);
	cmplxLn(&t1, &t2, &s1, &s2);

	cmplxDiv2(rx, ry, &t1, &t2);
}



/* Logarithm related functions */
// 74 bytes
void cmplx_do_log(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *base) {
	decNumber s1, s2;

	if (decNumberIsInfinite(a) || decNumberIsInfinite(b)) {
		set_inf(rx);
		decNumberZero(ry);
	} else {
		cmplxLn(&s1, &s2, a, b);
		cmplxDivideReal(rx, ry, &s1, &s2, base);
	}
}

// 20 bytes
void cmplxLog(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_do_log(rx, ry, a, b, &const_ln10);
}

// 22 bytes
void cmplxLog2(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplx_do_log(rx, ry, a, b, &const_ln2);
}

// 56 bytes
void cmplxLogxy(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
	decNumber la, lb, lc, ld;

	cmplxLn(&la, &lb, a, b);
	cmplxLn(&lc, &ld, c, d);
	cmplxDivide(rx, ry, &la, &lb, &lc, &ld);
}


/* Power functions */
// This was inlined in the latter two
static void cmplxRealPower(decNumber *rx, decNumber *ry,
		const decNumber *r,
		const decNumber *a, const decNumber *b) {
	cmplxPower(rx, ry, r, &const_0, a, b);
}

// 28 bytes
void cmplx10x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplxRealPower(rx, ry, &const_10, a, b);
}

// 28 bytes
void cmplx2x(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	cmplxRealPower(rx, ry, &const_2, a, b);
}

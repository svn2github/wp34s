/* Real beta function -- implemented incorrectly! */
/* 30 bytes */

// Beta(x, y) = exp(lngamma(x) + lngamma(y) - lngamma(x+y))
decNumber *decNumberBeta(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber s;

	decNumberLnBeta(&s, x, y);
	dn_exp(res, &s);
	return res;
}


/* Complex ln beta function */
/* 116 bytes */
// Beta(a, b) = exp(lngamma(a) + lngamma(b) - lngamma(a+b))
void cmplxLnBeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
	decNumber s1, s2, t1, t2, u1, u2;

	cmplxLnGamma(&s1, &s2, a, b);
	busy();
	cmplxLnGamma(&t1, &t2, c, d);
	busy();
	cmplxAdd(&u1, &u2, &s1, &s2, &t1, &t2);
	cmplxAdd(&s1, &s2, a, b, c, d);
	cmplxLnGamma(&t1, &t2, &s1, &s2);
	cmplxSubtract(rx, ry, &u1, &u2, &t1, &t2);
}

/* Complex beta function -- again implemented incorrectly perhaps */
/* 28 bytes */

// Beta(a, b) = exp(lngamma(a) + lngamma(b) - lngamma(a+b))
void cmplxBeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
	decNumber s1, s2;

	cmplxLnBeta(&s1, &s2, a, b, c, d);
	cmplxExp(rx, ry, &s1, &s2);
}


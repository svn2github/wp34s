
/* Real parallel operator -- 46 bytes */
decNumber *decNumberParallel(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber p, s;

	dn_multiply(&p, x, y);
	dn_add(&s, x, y);
	dn_divide(res, &p, &s);
	return res;
}


/* Complex parallel operator -- 78 bytes */
void cmplxParallel(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
	decNumber p1, p2, s1, s2;

	cmplxMultiply(&p1, &p2, a, b, c, d);
	cmplxAdd(&s1, &s2, a, b, c, d);
	cmplxDivide(rx, ry, &p1, &p2, &s1, &s2);
}


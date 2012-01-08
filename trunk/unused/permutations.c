/* Permutations and combinations */



/* Complex flavours */

static void cmplxCopy(decNumber *rx, decNumber *ry, const decNumber *x, const decNumber *y) {
	decNumberCopy(rx, x);
	decNumberCopy(ry, y);
}

// 134 bytes
static int cmplx_perm_helper(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
	decNumber n, m, s1, s2;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b) || decNumberIsSpecial(c) || decNumberIsSpecial(d)) {
		cmplx_NaN(rx, ry);
		return 0;
	}
		
	dn_p1(&n, a);				// x+1
	cmplxLnGamma(&s1, &s2, &n, b);		// lnGamma(x+1) = Ln x!

	cmplxSubtract(rx, ry, &n, b, c, d);	// x-y+1
	cmplxLnGamma(&n, &m, rx, ry);		// LnGamma(x-y+1) = Ln (x-y)!
	cmplxSubtract(rx, ry, &s1, &s2, &n, &m);
	return 1;
}


/* Calculate combinations:
 * C(x, y) = P(x, y) / y! = x! / ( (x-y)! y! )
 */
// 106 bytes
void cmplxComb(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
	decNumber r1, r2, n, m, s1, s2;
	const int code = cmplx_perm_helper(&r1, &r2, a, b, c, d);

	if (code) {
		dn_p1(&n, c);			// y+1
		cmplxLnGamma(&s1, &s2, &n, d);	// LnGamma(y+1) = Ln y!
		cmplxSubtract(&n, &m, &r1, &r2, &s1, &s2);

		cmplxExp(rx, ry, &n, &m);
	} else
		cmplxCopy(rx, ry, &r1, &r2);
}

/* Calculate permutations:
 * P(x, y) = x! / (x-y)!
 */
// 66 bytes
void cmplxPerm(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
	decNumber t1, t2;
	const int code = cmplx_perm_helper(&t1, &t2, a, b, c, d);

	if (code)
		cmplxExp(rx, ry, &t1, &t2);
	else
		cmplxCopy(rx, ry, &t1, &t2);
}



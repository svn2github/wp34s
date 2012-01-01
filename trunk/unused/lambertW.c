// 32 bytes
decNumber *decNumberInvW(decNumber *r, const decNumber *x) {
	decNumber t;

	dn_exp(&t, x);
	return dn_multiply(r, &t, x);
}


// 44 bytes
void cmplxInvW(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber t1, t2;

	cmplxExp(&t1, &t2, a, b);
	cmplxMultiply(rx, ry, &t1, &t2, a, b);
}

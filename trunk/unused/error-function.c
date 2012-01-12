// Complementary error function
// 50 bytes
decNumber *decNumberERFC(decNumber *res, const decNumber *x) {
	decNumber a, b;

	dn_divide(&a, x, &const_root2on2);
	dn_minus(&b, &a);
	cdf_Q(&a, &b);
	return dn_mul2(res, &a);
}


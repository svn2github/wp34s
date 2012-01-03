
/* Cauchy distribution x0, gamma in registers J and K */
/* Cauchy distribution 322 bytes in total */

// 70 bytes
static int cauchy_xform(decNumber *r, decNumber *c, const decNumber *x, decNumber *gamma) {
	decNumber a, x0;

	dist_two_param(&x0, gamma);
	if (param_positive(r, gamma))
		return 1;
	dn_subtract(&a, x, &x0);
	dn_divide(c, &a, gamma);
	return 0;
}

// 80 bytes
decNumber *pdf_cauchy(decNumber *r, const decNumber *x) {
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma))
		return r;
	decNumberSquare(&a, &b);
	dn_p1(&b, &a);
	dn_mulPI(&a, &b);
	dn_multiply(&b, &a, &gamma);
	return decNumberRecip(r, &b);
}

// 70 bytes
decNumber *cdf_cauchy(decNumber *r, const decNumber *x) {
	decNumber a, b, gamma;

	if (cauchy_xform(r, &b, x, &gamma))
		return r;
	do_atan(&a, &b);
	dn_divide(&b, &a, &const_PI);
	return dn_add(r, &b, &const_0_5);
}

// 102 bytes
decNumber *qf_cauchy(decNumber *r, const decNumber *p) {
	decNumber a, b, x0, gamma;

	dist_two_param(&x0, &gamma);
	if (param_positive(r, &gamma))
		return r;
	if (check_probability(r, p, 0))
	    return r;
	dn_subtract(&a, p, &const_0_5);
	dn_mulPI(&b, &a);
	decNumberTan(&a, &b);
	dn_multiply(&b, &a, &gamma);
	return dn_add(r, &b, &x0);

}

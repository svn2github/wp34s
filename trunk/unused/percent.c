/* Percentage functions.
 * 288 bytes in total
 */

// % = x . y / 100
// 44 bytes
decNumber *decNumberPercent(decNumber *res, const decNumber *x) {
	decNumber y, z;

	getY(&y);
	dn_mulpow10(&z, &y, -2);
	return dn_multiply(res, &z, x);
}

// %chg = 100 ( x - y ) / y
// 52 bytes
decNumber *decNumberPerchg(decNumber *res, const decNumber *x) {
	decNumber w, y, z;

	getY(&y);
	dn_subtract(&z, x, &y);
	dn_divide(&w, &z, &y);
	dn_mul100(res, &w);
	return res;
}

// %tot = 100 . x / y
// 42 bytes
decNumber *decNumberPertot(decNumber *res, const decNumber *x) {
	decNumber y, z;

	getY(&y);
	dn_divide(&z, x, &y);
	dn_mul100(res, &z);
	return res;
}

// Markup Margin = y / ( 1 - x / 100 )
// 46 bytes
decNumber *decNumberPerMargin(decNumber *res, const decNumber *y, const decNumber *x) {
	decNumber a, b;

	dn_mulpow10(&a, x, -2);
	dn_1m(&b, &a);
	return dn_divide(res, y, &b);
}

// Margin = 100 (x - y) / x
// 46 bytes
decNumber *decNumberMargin(decNumber *res, const decNumber *y, const decNumber *x) {
	decNumber a, b;

	dn_subtract(&a, x, y);
	dn_mul100(&b, &a);
	return dn_divide(res, &b, x);
}

// PerMRR = ((x/y) ^ 1/z - 1 ) * 100
// 58 bytes
decNumber *decNumberPerMRR(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x) {
	decNumber a, b, c;

	dn_divide(&a, x, y);
	decNumberRecip(&b, z);
	dn_power(&c, &a, &b);
	dn_m1(&a, &c);
	return dn_mul100(r, &a);
}


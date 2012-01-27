/* Unused date functions */

/* Days between dates.
 * Convert to Julian days and subtract.
 */
// 100 bytes
decNumber *dateDelta(decNumber *res, const decNumber *x, const decNumber *y) {
	int d, m, yr, j1, j2;

	if (decNumberIsSpecial(x) || decNumberIsSpecial(y))
err:		set_NaN(res);
	else {
		if (extract_date(x, &yr, &m, &d))
			goto err;
		j1 = JDN(yr, m, d);
		if (extract_date(y, &yr, &m, &d))
			goto err;
		j2 = JDN(yr, m, d);
		int_to_dn(res, j2-j1);
	}
	return res;
}


/* Add or subtract days from a date.
 * Convert to Julian days, do the addition or subtraction and convert back.
 */
// 112 bytes
decNumber *dateAdd(decNumber *res, const decNumber *x, const decNumber *y) {
	int j;
	int yr, m, d;

	if (decNumberIsSpecial(x) || decNumberIsSpecial(y) || ! is_int(y)) {
err:		set_NaN(res);
		return res;
	}
	if (extract_date(x, &yr, &m, &d))
		goto err;
	j = dn_to_int(y) + JDN(yr, m, d);
	if (j < 0)
		goto err;
	JDN2(j, &yr, &m, &d);
	return build_date(res, yr, m, d);
}


// 60 bytes
decNumber *decNumberSign(decNumber *r, const decNumber *x) {
	const decNumber *z;

	if (decNumberIsNaN(x))
		z = x;
	else if (dn_eq0(x))
		z = &const_0;
	else if (decNumberIsNegative(x))
		z = &const__1;
	else
		z = &const_1;
	return decNumberCopy(r, z);
}

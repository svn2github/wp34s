/* This file is part of 34S.
 * 
 * 34S is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * 34S is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with 34S.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Code for the repeated arithmetic/geometric mean */


/* Real AGM - 192 bytes */

decNumber *decNumberAGM(decNumber *res, const decNumber *x, const decNumber *y) {
	int n;
	decNumber a, g, t, u;

	if (decNumberIsNegative(x) || decNumberIsNegative(y))
		goto nan;
	if (decNumberIsSpecial(x) || decNumberIsSpecial(y)) {
		if (decNumberIsNaN(x) || decNumberIsNaN(y))
			goto nan;
		if (dn_eq0(x) || dn_eq0(y))
			goto nan;
		return set_inf(res);
	}
	decNumberCopy(&a, x);
	decNumberCopy(&g, y);
	for (n=0; n<1000; n++) {
		if (relative_error(&a, &g, &const_1e_32))
			return decNumberCopy(res, &a);

		dn_add(&t, &a, &g);
		dn_div2(&u, &t);

		dn_multiply(&t, &a, &g);
		if (dn_eq0(&t))
			return decNumberZero(res);
		dn_sqrt(&g, &t);
		decNumberCopy(&a, &u);
	}
nan:	return set_NaN(res);
}




/* Complex AGM - 228 bytes */

void cmplxAGM(decNumber *rx, decNumber *ry,
		const decNumber *a, const decNumber *b,
		const decNumber *c, const decNumber *d) {
	decNumber x1, x2, y1, y2, t1, t2, u1, u2;
	int n;

	if (decNumberIsSpecial(a) || decNumberIsSpecial(b) ||
			decNumberIsSpecial(c) || decNumberIsSpecial(d)) {
			goto nan;
	}
	cmplxCopy(&x1, &x2, a, b);
	cmplxCopy(&y1, &y2, c, d);
	for (n=0; n<1000; n++) {
		cmplxSubtract(&t1, &t2, &x1, &x2, &y1, &y2);
		cmplxR(&u1, &t1, &t2);
		dn_compare(&u2, &u1, &const_1e_32);
		if (decNumberIsNegative(&u2)) {
			cmplxCopy(rx, ry, &x1, &x2);
			return;
		}

		cmplxAdd(&t1, &t2, &x1, &x2, &y1, &y2);
		cmplxDiv2(&u1, &u2, &t1, &t2);

		cmplxMultiply(&t1, &t2, &x1, &x2, &y1, &y2);
		cmplxSqrt(&y1, &y2, &t1, &t2);

		cmplxCopy(&x1, &x2, &u1, &u2);
	}
nan:	cmplx_NaN(rx, ry);
}



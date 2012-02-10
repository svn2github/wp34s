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


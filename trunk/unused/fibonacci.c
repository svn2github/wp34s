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


// 88 bytes
decNumber *decNumberFib(decNumber *res, const decNumber *x) {
	decNumber r, s, t;

	dn_power(&r, &const_phi, x);
	dn_mulPI(&t, x);
	dn_sincos(&t, NULL, &s);
	dn_divide(&t, &s, &r);
	dn_subtract(&s, &r, &t);
	return dn_multiply(res, &s, &const_recipsqrt5);
}


// 138 bytes
void cmplxFib(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b) {
	decNumber r1, r2, s1, s2, t1, t2;

	cmplxRealPower(&r1, &r2, &const_phi, a, b);
	cmplxMultiplyReal(&t1, &t2, a, b, &const_PI);
	cmplxCos(&s1, &s2, &t1, &t2);
	cmplxDivide(&t1, &t2, &s1, &s2, &r1, &r2);
	cmplxSubtract(&s1, &s2, &r1, &r2, &t1, &t2);
	cmplxMultiplyReal(rx, ry, &s1, &s2, &const_recipsqrt5);
}


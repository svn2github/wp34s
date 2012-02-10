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

/* Real beta function -- implemented incorrectly! */
/* 30 bytes */

// Beta(x, y) = exp(lngamma(x) + lngamma(y) - lngamma(x+y))
decNumber *decNumberBeta(decNumber *res, const decNumber *x, const decNumber *y) {
	decNumber s;

	decNumberLnBeta(&s, x, y);
	dn_exp(res, &s);
	return res;
}


/* Complex ln beta function */
/* 116 bytes */
// Beta(a, b) = exp(lngamma(a) + lngamma(b) - lngamma(a+b))
void cmplxLnBeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
	decNumber s1, s2, t1, t2, u1, u2;

	cmplxLnGamma(&s1, &s2, a, b);
	busy();
	cmplxLnGamma(&t1, &t2, c, d);
	busy();
	cmplxAdd(&u1, &u2, &s1, &s2, &t1, &t2);
	cmplxAdd(&s1, &s2, a, b, c, d);
	cmplxLnGamma(&t1, &t2, &s1, &s2);
	cmplxSubtract(rx, ry, &u1, &u2, &t1, &t2);
}

/* Complex beta function -- again implemented incorrectly perhaps */
/* 28 bytes */

// Beta(a, b) = exp(lngamma(a) + lngamma(b) - lngamma(a+b))
void cmplxBeta(decNumber *rx, decNumber *ry, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *d) {
	decNumber s1, s2;

	cmplxLnBeta(&s1, &s2, a, b, c, d);
	cmplxExp(rx, ry, &s1, &s2);
}


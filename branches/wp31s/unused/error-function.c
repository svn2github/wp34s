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

// Complementary error function
// 50 bytes
decNumber *decNumberERFC(decNumber *res, const decNumber *x) {
	decNumber a, b;

	dn_divide(&a, x, &const_root2on2);
	dn_minus(&b, &a);
	cdf_Q(&a, &b);
	return dn_mul2(res, &a);
}


// Error function
// 96 bytes
decNumber *decNumberERF(decNumber *res, const decNumber *x) {
	decNumber z;

	if (decNumberIsSpecial(x)) {
		if (decNumberIsNaN(x))
			return set_NaN(res);
		if (decNumberIsNegative(x))
			return dn__1(res);
		return dn_1(res);
	}
	decNumberSquare(&z, x);
	decNumberGammap(res, &const_0_5, &z);
	if (decNumberIsNegative(x))
		return dn_minus(res, res);
	return res;
	
}

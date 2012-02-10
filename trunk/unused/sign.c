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

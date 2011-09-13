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

#include "matrix.h"
#include "decn.h"
#include "consts.h"


#include <stdio.h>

#ifdef MATRIX_SUPPORT
static int matrix_decompose(const decNumber *x, int *rows, int *cols, int *up) {
	decNumber ax, y;
	unsigned int n, base;
	int r, c, u;

	u = decNumberIsNegative(x) ? 1 : 0;
	if (u) {
		dn_abs(&ax, x);
		x = &ax;
	}
	dn_multiply(&y, x, &const_10000);
	n = dn_to_int(&y);
	base = (n / 10000) % 100;
	r = n % 100;
	c = (n / 100) % 100;
	if (r == 0)
		r = c;
	if (base + r * c >= 100) {
		err(ERR_RANGE);
		return -1;
	}
	if (r == 0 || c == 0) {
		err(ERR_TOO_LONG);
		return -1;
	}
	if (rows)	*rows = r;
	if (cols)	*cols = c;
	if (up)		*up = u;
	return base;
}

static decNumber *matrix_do_loop(decNumber *r, int low, int high, int step, int up) {
	decNumber z;
	int i;

	if (up) {
		i = (low * 1000 + high) * 100 + step;
	} else {
		i = (high * 1000 + low - 1) * 100 + step;
	}
	int_to_dn(&z, i);
	dn_multiply(r, &z, &const_0_00001);
	return r;
}

decNumber *matrix_all(decNumber *r, const decNumber *x) {
	int rows, cols, base, up;

	base = matrix_decompose(x, &rows, &cols, &up);
	if (base < 0)
		return NULL;
	return matrix_do_loop(r, base, base+rows*cols-1, 1, up);
}

decNumber *matrix_row(decNumber *r, const decNumber *y, const decNumber *x) {
	int rows, cols, base, up, n;

	base = matrix_decompose(x, &rows, &cols, &up);
	if (base < 0)
		return NULL;
	n = dn_to_int(y) - 1;
	if (n < 0 || n >= rows) {
		err(ERR_RANGE);
		return NULL;
	}
	base += n*cols;
	return matrix_do_loop(r, base, base + cols - 1, 1, up);
}

decNumber *matrix_col(decNumber *r, const decNumber *y, const decNumber *x) {
	int rows, cols, base, up, n;

	base = matrix_decompose(x, &rows, &cols, &up);
	if (base < 0)
		return NULL;
	n = dn_to_int(y) - 1;
	if (n < 0 || n >= cols) {
		err(ERR_RANGE);
		return NULL;
	}
	base += n;
	return matrix_do_loop(r, base, base + cols * (rows-1), cols, up);
}

decNumber *matrix_rowq(decNumber *r, const decNumber *x) {
	int rows;

	if (matrix_decompose(x, &rows, NULL, NULL) < 0)
		return NULL;
	int_to_dn(r, rows);
	return r;
}

decNumber *matrix_colq(decNumber *r, const decNumber *x) {
	int cols;

	if (matrix_decompose(x, NULL, &cols, NULL) < 0)
		return NULL;
	int_to_dn(r, cols);
	return r;
}

#endif

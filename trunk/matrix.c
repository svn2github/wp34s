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


#ifdef MATRIX_SUPPORT

static int matrix_idx(int row, int col, int ncols) {
	return col + row * ncols;
}
		
static int matrix_descriptor(decNumber *r, int base, int rows, int cols) {
	decNumber z;

	if (base < 0 || base + rows * cols > 100) {
		err(ERR_RANGE);
		return 0;
	}
	int_to_dn(&z, (base * 100 + cols) * 100 + rows);
	dn_multiply(r, &z, &const_0_0001);
	return 1;
}

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
	if (base + r * c > 100) {
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

static decimal64 *matrix_decomp(const decNumber *x, int *rows, int *cols) {
	const int base = matrix_decompose(x, rows, cols, NULL);

	if (base < 0)
		return NULL;
	return get_reg_n(base);
}

static decNumber *matrix_do_loop(decNumber *r, int low, int high, int step, int up) {
	decNumber z;
	int i;

	if (up) {
		i = (low * 1000 + high) * 100 + step;
	} else {
		if (low == 0)
			err(ERR_RANGE);
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

decNumber *matrix_getreg(decNumber *r, const decNumber *cdn, const decNumber *rdn, const decNumber *m) {
	int h, w, ri, ci;
	int n = matrix_decompose(m, &h, &w, NULL);

	if (n < 0)
		return NULL;
	ri = dn_to_int(rdn) - 1;
	ci = dn_to_int(cdn) - 1;
	if (ri < 0 || ci < 0 || ri >= h || ci >= w) {
		err(ERR_RANGE);
		return NULL;
	}
	n += matrix_idx(ri, ci, w);
	int_to_dn(r, n);
	return r;
}

decNumber *matrix_getrc(decNumber *res, const decNumber *m) {
	decNumber ydn;
	int rows, cols, c, r, pos;
	int n = matrix_decompose(m, &rows, &cols, NULL);

	if (n < 0)
		return NULL;
	getY(&ydn);
	pos = dn_to_int(&ydn);
	pos -= n;
	if (pos < 0 || pos >= rows*cols) {
		err(ERR_RANGE);
		return NULL;
	}
	c = pos % cols + 1;
	r = pos / cols + 1;
	int_to_dn(res, r);
	int_to_dn(&ydn, c);
	setY(&ydn);
	return res;
}

// a = a + b * k -- generalised matrix add and subtract
decNumber *matrix_genadd(decNumber *r, const decNumber *k, const decNumber *b, const decNumber *a) {
	int arows, acols, brows, bcols;
	decNumber s, t, u;
	int i;

	decimal64 *abase = matrix_decomp(a, &arows, &acols);
	decimal64 *bbase = matrix_decomp(b, &brows, &bcols);
	if (abase == NULL || bbase == NULL)
		return NULL;
	if (arows != brows || acols != bcols) {
		err(ERR_RANGE);
		return NULL;
	}
	for (i=0; i<arows*acols; i++) {
		decimal64ToNumber(bbase + i, &s);
		dn_multiply(&t, &s, k);
		decimal64ToNumber(abase + i, &s);
		dn_add(&u, &t, &s);
		packed_from_number(abase + i, &u);
	}
	return decNumberCopy(r, a);
}


// Matrix multiply c = a * b
decNumber *matrix_multiply(decNumber *r, const decNumber *a, const decNumber *b, const decNumber *c) {
	int arows, acols, brows, bcols;
	decNumber sum, s, t, u;
	int creg;
	int i, j, k;

	decimal64 *abase = matrix_decomp(a, &arows, &acols);
	decimal64 *bbase = matrix_decomp(b, &brows, &bcols);
	decimal64 *ap, *bp, *cp;

	if (abase == NULL || bbase == NULL)
		return NULL;
	if (acols != brows) {
		err(ERR_RANGE);
		return NULL;
	}
	creg = dn_to_int(c);
	if (matrix_descriptor(r, creg, arows, bcols) == 0)
		return NULL;
	cp = get_reg_n(creg);

	for (i=0; i<arows; i++)
		for (j=0; j<bcols; j++) {
			decNumberZero(&sum);
			ap = abase + acols * i;
			bp = bbase + j;
			for (k=0; k<acols; k++) {
				decimal64ToNumber(ap++, &s);
				//matrix_get(&s, abase, i, k, acols);
				decimal64ToNumber(bp, &t);
				bp += bcols;
				//matrix_get(&t, bbase, k, j, bcols);
				dn_multiply(&u, &s, &t);
				dn_add(&sum, &sum, &u);
			}
			packed_from_number(cp++, &sum);
		}
	return r;
}

/* In place matrix transpose using minimal extra storage */
decNumber *matrix_transpose(decNumber *r, const decNumber *m) {
	int w, h, start, next, i;
	int n = matrix_decompose(m, &h, &w, NULL);
	decimal64 *base, tmp;
	const int elements = w*h;
	char used[100];

	if (n < 0)
		return NULL;
	base = get_reg_n(n);
	if (base == NULL)
		return NULL;

	xset(used, 0, elements);
	for (start=0; start < elements; start++) {
		if (! used[start]) {
			tmp = base[next = start];
			do {
				i = (next % h) * w + next / h;
				used[i] = 1;
				base[next] = (i == start) ? tmp : base[i];
				next = i;
			} while (next > start);
		}
	}

	matrix_descriptor(r, n, w, h);
	return r;
}

void matrix_rowops(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	decNumber m, ydn, zdn, t;
	decimal64 *base, *r1, *r2;
	int rows, cols;
	int i;

	getXYZT(&m, &ydn, &zdn, &t);
	base = matrix_decomp(&m, &rows, &cols);
	if (base == NULL)
		return;

	i = dn_to_int(&ydn) - 1;
	if (i < 0 || i >= rows) {
badrow:		err(ERR_RANGE);
		return;
	}
	r1 = base + i * cols;

	if (op == OP_MAT_ROW_MUL) {
		for (i=0; i<cols; i++) {
			decimal64ToNumber(r1, &t);
			dn_multiply(&m, &zdn, &t);
			packed_from_number(r1++, &m);
		}
	} else {
		i = dn_to_int(&zdn) - 1;
		if (i < 0 || i >= rows)
			goto badrow;
		r2 = base + i * cols;

		if (op == OP_MAT_ROW_SWAP) {
			for (i=0; i<cols; i++)
				swap_reg(r1++, r2++);
		} else {
			for (i=0; i<cols; i++) {
				decimal64ToNumber(r1, &ydn);
				decimal64ToNumber(r2++, &zdn);
				dn_multiply(&m, &zdn, &t);
				dn_add(&zdn, &ydn, &m);
				packed_from_number(r1++, &zdn);
			}
		}
	}
}

#endif

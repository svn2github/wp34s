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
#include "decNumber/decimal128.h"

#define MAX_DIMENSION	100
#define MAX_SQUARE	10

#ifdef MATRIX_SUPPORT

static int matrix_idx(int row, int col, int ncols) {
	return col + row * ncols;
}
		
static void matrix_get(decNumber *r, const decimal64 *base, int row, int col, int ncols) {
	decimal64ToNumber(base + matrix_idx(row, col, ncols), r);
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

	if (decNumberIsNegative(x)) {
		dn_abs(&ax, x);
		x = &ax;
		u = 0;
	} else
		u = 1;
	if (up)		*up = u;

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
	if (c == 0) {
		err(ERR_BAD_PARAM);
		return -1;
	}
	if (rows)	*rows = r;
	if (cols)	*cols = c;
	return base;
}

static decimal64 *matrix_decomp(const decNumber *x, int *rows, int *cols) {
	const int base = matrix_decompose(x, rows, cols, NULL);

	if (base < 0)
		return NULL;
	return get_reg_n(base);
}

void matrix_is_square(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	int r, c;
	decNumber x;

	getX(&x);
	if (matrix_decompose(&x, &r, &c, NULL) >= 0 && r != c)
		err(ERR_MATRIX_DIM);
}

#ifdef SILLY_MATRIX_SUPPORT
void matrix_create(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	decNumber x;
	int r, c, i, j;
	decimal64 *base;
	const decimal64 *diag, *off;

	getX(&x);
	base = matrix_decomp(&x, &r, &c);
	if (base != NULL) {
		off = &CONSTANT_INT(OP_ZERO);

		if (op == OP_MAT_IDENT) {
			if (r != c) {
				err(ERR_MATRIX_DIM);
				return;
			}
			diag = &CONSTANT_INT(OP_ONE);
		} else
			diag = off;

		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
				*base++ = *((i==j)?diag:off);
	}
}
#endif

static decNumber *matrix_do_loop(decNumber *r, int low, int high, int step, int up) {
	decNumber z;
	int i;

	if (up) {
		i = (low * 1000 + high) * 100 + step;
	} else {
		if (low == 0)
			err(ERR_DOMAIN);
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
		err(ERR_MATRIX_DIM);
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
	decimal64 *cbase;

	if (abase == NULL || bbase == NULL)
		return NULL;
	if (acols != brows) {
		err(ERR_MATRIX_DIM);
		return NULL;
	}
	creg = dn_to_int(c);
	if (matrix_descriptor(r, creg, arows, bcols) == 0)
		return NULL;
	cbase = get_reg_n(creg);

	for (i=0; i<arows; i++)
		for (j=0; j<bcols; j++) {
			decNumberZero(&sum);
			for (k=0; k<acols; k++) {
				matrix_get(&s, abase, i, k, acols);
				matrix_get(&t, bbase, k, j, bcols);
				dn_multiply(&u, &s, &t);
				dn_add(&sum, &sum, &u);
			}
			packed_from_number(cbase++, &sum);
		}
	return r;
}

/* In place matrix transpose using minimal extra storage */
decNumber *matrix_transpose(decNumber *r, const decNumber *m) {
	int w, h, start, next, i;
	int n = matrix_decompose(m, &h, &w, NULL);
	decimal64 *base, tmp;

	if (n < 0)
		return NULL;
	base = get_reg_n(n);
	if (base == NULL)
		return NULL;

	for (start=0; start < w*h; start++) {
		next = start;
		i=0;
		do {
			i++;
			next = (next % h) * w + next / h;
		} while (next > start);
		if (next < start || i == 1)
			continue;

		tmp = base[next = start];
		do {
			i = (next % h) * w + next / h;
			base[next] = (i == start) ? tmp : base[i];
			next = i;
		} while (next > start);
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


static void matrix_get128(decNumber *r, const decimal128 *base, int row, int col, int ncols) {
	decimal128ToNumber(base + matrix_idx(row, col, ncols), r);
}

static void put128(decimal128 *r, const decNumber *x) {
	decContext ctx128;

	decContextDefault(&ctx128, DEC_INIT_DECIMAL128);
	ctx128.traps = 0;
	decimal128FromNumber(r, x, &ctx128);
}

static void matrix_put128(const decNumber *x, decimal128 *base, int row, int col, int ncols) {
	put128(base + matrix_idx(row, col, ncols), x);
}


static int LU_decomposition(decimal128 *A, unsigned char *pivots, const int n) {
	int i, j, k;
	int pvt, spvt = 1;
	decimal128 *p1, *p2;
	decNumber max, t, u;

	for (k=0; k<n; k++) {
		/* Find the pivot row */
		pvt = k;
		matrix_get128(&u, A, k, k, n);
		dn_abs(&max, &u);
		for (j=k+1; j<n; j++) {
			matrix_get128(&t, A, j, k, n);
			dn_abs(&u, &t);
			if (dn_lt0(dn_compare(&t, &max, &u))) {
				decNumberCopy(&max, &u);
				pvt = j;
			}
		}
		if (pivots != NULL)
			*pivots++ = pvt;

		/* pivot if required */
		if (pvt != k) {
			spvt = -spvt;
			p1 = A + (n * k);
			p2 = A + (n * pvt);
			for (j=0; j<n; j++) {
				decimal128 t = *p1;
				*p1 = *p2;
				*p2 = t;
				p1++;
				p2++;
				//swap_reg(p1++, p2++);
			}
		}

		/* Check for singular */
		matrix_get128(&t, A, k, k, n);
		if (decNumberIsZero(&t))
			return 0;

		/* Find the lower triangular elements for column k */
		for (i=k+1; i<n; i++) {
			matrix_get128(&t, A, k, k, n);
			matrix_get128(&u, A, i, k, n);
			dn_divide(&max, &u, &t);
			matrix_put128(&max, A, i, k, n);
		}
		/* Update the upper triangular elements */
		for (i=k+1; i<n; i++)
			for (j=k+1; j<n; j++) {
				matrix_get128(&t, A, i, k, n);
				matrix_get128(&u, A, k, j, n);
				dn_multiply(&max, &t, &u);
				matrix_get128(&t, A, i, j, n);
				dn_subtract(&u, &t, &max);
				matrix_put128(&u, A, i, j, n);
			}
	}
	return spvt;
}

static int matrix_lu_check(const decNumber *m, decimal128 *mat, decimal64 **mbase) {
	int rows, cols;
	decimal64 *base;
	decNumber t;
	int i;

	base = matrix_decomp(m, &rows, &cols);
	if (base == NULL)
		return 0;
	if (rows != cols) {
		err(ERR_MATRIX_DIM);
		return 0;
	}
	if (mat != NULL) {
		for (i=0; i<rows*rows; i++) {
			decimal64ToNumber(base+i, &t);
			put128(mat+i, &t);
		}
	}
	if (mbase != NULL)
		*mbase = base;
	return rows;
}

decNumber *matrix_determinant(decNumber *r, const decNumber *m) {
	int n, i;
	decimal128 mat[MAX_SQUARE*MAX_SQUARE];
	decNumber t;

	n = matrix_lu_check(m, mat, NULL);
	if (n == 0)
		return NULL;

	i = LU_decomposition(mat, NULL, n);

	int_to_dn(r, i);
	for (i=0; i<n; i++) {
		matrix_get128(&t, mat, i, i, n);
		dn_multiply(r, r, &t);
	}
	return r;
}

decNumber *matrix_lu_decomp(decNumber *r, const decNumber *m) {
	unsigned char pivots[MAX_SQUARE];
	int i, sign, n;
	decNumber t, u;
	decimal128 mat[MAX_SQUARE*MAX_SQUARE];
	decimal64 *base;

	n = matrix_lu_check(m, mat, &base);
	if (n == 0)
		return NULL;

	sign = LU_decomposition(mat, pivots, n);
	if (sign == 0) {
		err(ERR_SINGULAR);
		return NULL;
	}

	/* Build the pivot number */
	decNumberZero(r);
	for (i=0; i<n; i++) {
		int_to_dn(&t, pivots[i]);
		dn_multiply(&u, r, &const_10);
		dn_add(r, &u, &t);
	}

	/* Copy the result back over the matrix */
	for (i=0; i<n*n; i++) {
		decimal128ToNumber(mat+i, &t);
		packed_from_number(base+i, &t);
	}
	return r;
}
#endif

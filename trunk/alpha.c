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

#include "alpha.h"
#include "xeq.h"
#include "decn.h"
#include "display.h"
#include "int.h"
#include "consts.h"


/* Append a single character to the alpha register
 */
static char *internal_add_char(char *a, char c) {
	if (a == Alpha+NUMALPHA)
		*scopy(Alpha, Alpha+1) = c;
	else {
		*a++ = c;
		*a = '\0';
	}
	return a;
}

static void add_char(char c) {
	internal_add_char(find_char(Alpha, '\0'), c);
}


/* Append a string to the Alpha register.
 */
void add_string(const char *s) {
	char *a = find_char(Alpha, '\0');

	while (*s != '\0')
		a = internal_add_char(a, *s++);
}


/* Clear the Alpha register
 */
void clralpha(decimal64 *a, decimal64 *b, decContext *nulc) {
	int i;

	for (i=0; i<=NUMALPHA; i++)
		Alpha[i] = '\0';
}


/* Display the Alpha register
 */
void alpha_view(decimal64 *a, decimal64 *b, decContext *nulc) {
	DispMsg = Alpha;
	display();
	DispMsg = Alpha;
}


/* Append the character from the given register or passed arg to Alpha
 */
void cmdalpha(unsigned int arg, enum rarg op) {
	add_char(arg & 0xff);
}


#ifdef MULTI_ALPHA
/* Multiple append to Alpha */
void multialpha(opcode op, enum multiops mopr) {
	char buf[4];

	buf[0] = op & 0xff;
	buf[1] = (op >> 16) & 0xff;
	buf[2] = (op >> 24) & 0xff;
	buf[3] = '\0';
	add_string(buf);
}
#endif


/* Append integer value to Alpha
 */
void alpha_ip(unsigned int arg, enum rarg op) {
	unsigned int n;
	char tbuf[16], *p;
	int sgn;

	if (is_intmode()) {
		n = (unsigned int) extract_value(get_reg_n_as_int(arg), &sgn);
		// should convert this using the current display mode...
	} else {
		decNumber x;
		int z;

		get_reg_n_as_dn(arg, &x);
		z = dn_to_int(&x, Ctx64);
		n = z<0?-z:z;
		sgn = z<0;
	}

	p = tbuf;
	if (sgn)
		*p++ = '-';
	p = num_arg(p, n);
	*p = '\0';
	add_string(tbuf);
}


/* Return length of Alpha register
 */
int alen(void) {
	return find_char(Alpha, '\0') - Alpha;
}

void alpha_length(decimal64 *x, decimal64 *b, decContext *ctx64) {
	put_int(alen(), 0, x);
}

/* Shift or rotate Alpha register arg positions
 */
void alpha_shift_l(unsigned int arg, enum rarg op) {
	unsigned int i;
	const int rot = (op == RARG_ALRL);

	for (i=0; i<arg; i++) {
		const char c = rot?Alpha[0]:'\0';

		*scopy(Alpha, Alpha+1) = c;
	}
}

void alpha_shift_r(unsigned int arg, enum rarg op) {
	unsigned int i;

	for (i=0; i<arg; i++) {
		xcopy(Alpha+1, Alpha, NUMALPHA-1);
		Alpha[0] = ' ';
	}
}

void alpha_rot_r(unsigned int arg, enum rarg op) {
	unsigned int i;
	const int al = alen();

	if (al)
		for (i=0; i<arg; i++) {
			const char c = Alpha[al-1];
			xcopy(Alpha+1, Alpha, al-1);
			Alpha[0] = c;
		}
}

/* Take first character from Alpha and return its code in X.
 * remove the character from Alpha
 */
void alpha_tox(decimal64 *a, decimal64 *b, decContext *ctx64) {
	put_int(Alpha[0] & 0xff, 0, a);
	alpha_shift_l(1, RARG_ALSL);
}

void alpha_fromx(decimal64 *a, decimal64 *b, decContext *ctx64) {
	decNumber x, y;

	getX(&x);
	decNumberAbs(&y, &x, Ctx);
	decNumberRemainder(&x, &y, &const_256, Ctx);
	add_char(dn_to_int(&x, Ctx));
}

/* Recall a register and append to Alpha.
 * This honours the current display mode.
 */
void alpha_reg(unsigned int arg, enum rarg op) {
	char buf[64];

	xset(buf, '\0', sizeof(buf));
	format_reg(get_reg_n(arg), buf);
	add_string(buf);
}


/* Return the number of characters being stored in a register.
 * For reals this is 6 to match the 41 series.  For integers, it is as
 * many as will fit into the current sord size.
 */
static int char_per_reg(void) {
	return is_intmode()?(word_size() / 8):CHARS_IN_REG;
}


/* Sto start of Alpha register into arg
 */
void alpha_sto(unsigned int arg, enum rarg op) {
	unsigned long long int z = 0;
	int i;
	const int n = char_per_reg();

	for (i=0; Alpha[i] != '\0' && i<n; i++)
		z = (z << 8) | (0xff & Alpha[i]);
	reg_put_int(arg, z, 0);
}


/* Alpha recall a register and convert it in to the supplied
 * buffer and return a point to the first character (which won't
 * always be the start of the buffer.
 */
char *alpha_rcl_s(const decimal64 *reg, char buf[12]) {
	int i;
	unsigned long long int z = get_int(reg, &i);
	char *p = buf + 11;
	int n = char_per_reg();

	*p = '\0';
	for (; z != 0 && n>0; n--) {
		*--p = z & 0xff;
		z >>= 8;
	}
	return p;
}

/* RCL register containing alpha characters into Alpha
 */
void alpha_rcl(unsigned int arg, enum rarg op) {
	char buf[12];

	add_string(alpha_rcl_s(get_reg_n(arg), buf));
}

/* Turn alpha mode on and off
 */
void alpha_on(decimal64 *a, decimal64 *b, decContext *ctx64) {
	State.alphas = 1;
}

void alpha_off(decimal64 *a, decimal64 *b, decContext *ctx64) {
	State.alphas = 0;
}

/* Input one character and append to alpha
 */
void alpha_append() {
}

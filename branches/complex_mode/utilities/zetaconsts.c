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

#include <stdio.h>

// Calculate the coefficients for the gamma function approximation we're
// using.  Set the digit count to well above what we're using in the main
// program.

#define DECNUMDIGITS 100

#include "decNumber/decNumber.h"
#include "decNumber/decContext.h"
#include "decNumber/decimal64.h"

void xcopy(void *d, const void *s, int n) {
	char *dp = d;
	const char *sp = s;

	if (sp > dp)
		while (n--)
			*dp++ = *sp++;
	else if (sp < dp)
		while (n--)
			dp[n] = sp[n];
}

void int_to_dn(decNumber *x, int n, decContext *ctx) {
	char buf[12];

	sprintf(buf, "%d", n);
	decNumberFromString(x, buf, ctx);
}
/* Compute a factorial.
 * Currently, only for positive integer arguments.  Needs to be extended
 * to a full gamma function.
 */
decNumber *decNumberFactorial(decNumber *r, const decNumber *x, decContext *ctx) {
	decNumber y, const_1;

	int_to_dn(&const_1, 1, ctx);
	decNumberCopy(&y, x);
	if (!decNumberIsNegative(x) || decNumberIsZero(x)) {
		decNumberCopy(r, &const_1);
		for (;;) {
			if (decNumberIsZero(&y))
				break;
			if (decNumberIsInfinite(r))
				break;
			decNumberMultiply(r, r, &y, ctx);
			decNumberSubtract(&y, &y, &const_1, ctx);
		}
	}
	return r;
}

void zetadk(decNumber *dk, int n, int k, decContext *ctx) {
	int i;
	decNumber t, r, s, v, sum, const_4;

	int_to_dn(&const_4, 4, ctx);
	decNumberZero(&sum);
	for (i=0; i<=k; i++) {
		int_to_dn(&t, n+i-1, ctx);
		decNumberFactorial(&s, &t, ctx);
		int_to_dn(&t, i, ctx);
		decNumberPower(&r, &const_4, &t, ctx);
		decNumberMultiply(&v, &s, &r, ctx);
		int_to_dn(&t, n-i, ctx);
		decNumberFactorial(&s, &t, ctx);
		int_to_dn(&t, 2*i, ctx);
		decNumberFactorial(&r, &t, ctx);
		decNumberMultiply(&t, &r, &s, ctx);
		decNumberDivide(&r, &v, &t, ctx);
		decNumberAdd(&sum, &sum, &r, ctx);
	}
	int_to_dn(&t, n, ctx);
#if 1
	// Don't bother rounding to int, the conversion in compile_consts
	// will do this if required due to the extra precision being carries.
	decNumberMultiply(dk, &t, &sum, ctx);
#else
	// We can round this to integers this way....
	decNumberMultiply(&s, &t, &sum, ctx);
	decNumberToIntegralValue(dk, &s, ctx);
#endif
}

int main(int argc, char *argv[]) {
	int i;
	const int N = 30;	// At 15 the second last term is the same as the last
	decNumber di, dn;
	char s[10000];

	decContext ctx;

	decContextDefault(&ctx, DEC_INIT_BASE);
	// ctx.traps = 0;
	ctx.digits = DECNUMDIGITS;
	ctx.emax=DEC_MAX_MATH;
	ctx.emin=-DEC_MAX_MATH;
	ctx.round = DEC_ROUND_HALF_EVEN;

	zetadk(&dn, N, N, &ctx);
	decNumberToString(&dn, s);
	printf("\t{ DFLT, \"zeta_dn\",\t\t\"%s\"\t},\n", s);
	for (i=0; i<N; i++) {
		zetadk(&di, N, i, &ctx);
		decNumberSubtract(&di, &dn, &di, &ctx);
		decNumberToString(&di, s);
		printf("\t{ DFLT, \"zetaC%02d\",\t\t\"%s\"\t},\n",
				i, s);
	}
	return 0;
}

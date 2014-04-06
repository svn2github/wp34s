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

#include "int.h"
#include "xeq.h"

/* Some utility routines to extract bits of long longs */

unsigned int int_base(void) {
	const unsigned int b = UState.int_base + 1;
	if (b < 2)
		return 10;
	return b;
}

enum arithmetic_modes int_mode(void) {
	return (enum arithmetic_modes) UState.int_mode;
}

unsigned int word_size(void) {
	unsigned int il = UState.int_len;
	if (il >= MAX_WORD_SIZE || il == 0)
		return MAX_WORD_SIZE;
	return il;
}

int get_carry(void) {
	return get_user_flag(CARRY_FLAG);
}

void set_carry(int c) {
#ifndef TINY_BUILD
	if (c)
		set_user_flag(CARRY_FLAG);
	else
		clr_user_flag(CARRY_FLAG);
#endif
}

int get_overflow(void) {
	return get_user_flag(OVERFLOW_FLAG);
}

void set_overflow(int o) {
#ifndef TINY_BUILD
	if (o)
		set_user_flag(OVERFLOW_FLAG);
	else
		clr_user_flag(OVERFLOW_FLAG);
#endif
}

/* Utility routine for trimming a value to the current word size
 */
long long int mask_value(const long long int v) {
#ifndef TINY_BUILD
	const unsigned int ws = word_size();
	long long int mask;

	if (MAX_WORD_SIZE == 64 && ws == 64)
		return v;
	mask = (1LL << ws) - 1;
	return v & mask;
#else
	return v;
#endif
}

#ifndef TINY_BUILD
/* Ulility routine for returning a bit mask to get the topmost (sign)
 * bit from a number.
 */
static long long int topbit_mask(void) {
	const unsigned int ws = word_size();
	long long int bit = 1LL << (ws - 1);
	return bit;
}
#endif

/* Utility routine to convert a binary integer into separate sign and
 * value components.  The sign returned is 1 for negative and 0 for positive.
 */
unsigned long long int extract_value(const long long int val, int *const sign) {
	const enum arithmetic_modes mode = int_mode();
	long long int v = mask_value(val);
	long long int tbm;

	if (mode == MODE_UNSIGNED) {
		*sign = 0;
		return v;
	}

	tbm = topbit_mask();

	if (v & tbm) {
		*sign = 1;
		if (mode == MODE_2COMP)
			v = -v;
		else if (mode == MODE_1COMP)
			v = ~v;
		else // if (mode == MODE_SGNMANT)
			v ^= tbm;
	} else
		*sign = 0;
    return mask_value(v);
}

/* Helper routine to construct a value from the magnitude and sign
 */
long long int build_value(const unsigned long long int x, const int sign) {
#ifndef TINY_BUILD
	const enum arithmetic_modes mode = int_mode();
	long long int v = mask_value(x);

	if (sign == 0 || mode == MODE_UNSIGNED)
		return v;

	if (mode == MODE_2COMP)
		return mask_value(-(signed long long int)v);
	if (mode == MODE_1COMP)
		return mask_value(~v);
	return v | topbit_mask();
#else
	return x;
#endif
}


/* Utility routine to check if a value has overflowed or not */
int check_overflow(long long int x) {
	return mask_value(x) != x ||
		(int_mode() != MODE_UNSIGNED && (x & topbit_mask()) != 0);
}


#ifndef TINY_BUILD
static void breakup(unsigned long long int x, unsigned short xv[4]) {
	xv[0] = x & 0xffff;
	xv[1] = (x >> 16) & 0xffff;
	xv[2] = (x >> 32) & 0xffff;
	xv[3] = (x >> 48) & 0xffff;
}

static unsigned long long int packup(unsigned short int x[4]) {
	return (((unsigned long long int)x[3]) << 48) |
			(((unsigned long long int)x[2]) << 32) |
			(((unsigned long int)x[1]) << 16) |
			x[0];
}
#endif

void intDblMul(enum nilop op) {
#ifndef TINY_BUILD
	const enum arithmetic_modes mode = int_mode();
	unsigned long long int xv, yv;
	int s;	
	unsigned short int xa[4], ya[4];
	unsigned int t[8];
	unsigned short int r[8];
	int i, j;

	{
		long long int xr, yr;
		int sx, sy;

		xr = getX_int();
		yr = get_reg_n_int(regY_idx);

		xv = extract_value(xr, &sx);
		yv = extract_value(yr, &sy);

		s = sx != sy;
	}

	/* Do the multiplication by breaking the values into unsigned shorts
	 * multiplying them all out and accumulating into unsigned ints.
	 * Then perform a second pass over the ints to propogate carry.
	 * Finally, repack into unsigned long long ints.
	 *
	 * This isn't terribly efficient especially for shorter word
	 * sizes but it works.  Special cases for WS <= 16 and/or WS <= 32
	 * might be worthwhile since the CPU supports these multiplications
	 * natively.
	 */
	breakup(xv, xa);
	breakup(yv, ya);

	for (i=0; i<8; i++)
		t[i] = 0;

	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			t[i+j] += xa[i] * ya[j];

	for (i=0; i<8; i++) {
		if (t[i] >= 65536)
			t[i+1] += t[i] >> 16;
		r[i] = t[i];
	}

	yv = packup(r);
	xv = packup(r+4);

	i = word_size();
	if (i != 64)
		xv = (xv << (64-i)) | (yv >> i);

	setlastX();

	if (s != 0) {
		if (mode == MODE_2COMP) {
			yv = mask_value(1 + ~yv);
			xv = ~xv;
			if (yv == 0)
				xv++;
		} else if (mode == MODE_1COMP) {
			yv = ~yv;
			xv = ~xv;
		} else
			xv |= topbit_mask();
	}

	set_reg_n_int(regY_idx, mask_value(yv));
	setX_int(mask_value(xv));
	set_overflow(0);
#endif
}

/* Calculate (a . b) mod c taking care to avoid overflow */
static unsigned long long mulmod(const unsigned long long int a, unsigned long long int b, const unsigned long long int c) {
	unsigned long long int x=0, y=a%c;
	while (b > 0) {
		if ((b & 1))
			x = (x+y)%c;
		y = (y+y)%c;
		b /= 2;
	}
	return x % c;
}

/* Calculate (a ^ b) mod c */
static unsigned long long int expmod(const unsigned long long int a, unsigned long long int b, const unsigned long long int c) {
	unsigned long long int x=1, y=a;
	while (b > 0) {
		if ((b & 1))
			x = mulmod(x, y, c);
		y = mulmod(y, y, c);
		b /= 2;
	}
	return (x % c);
}

/* Test if a number is prime or not using a Miller-Rabin test */
#ifndef TINY_BUILD
static const unsigned char primes[] = {
	2, 3, 5, 7,	11, 13, 17, 19,
	23, 29, 31, 37,	41, 43, 47, 53,
};
#define N_PRIMES	(sizeof(primes) / sizeof(unsigned char))
#define QUICK_CHECK	(59*59-1)
#endif

int isPrime(unsigned long long int p) {
#ifndef TINY_BUILD
	int i;
	unsigned long long int s;
#define PRIME_ITERATION	15

	/* Quick check for p <= 2 and evens */
	if (p < 2)	return 0;
	if (p == 2)	return 1;
	if ((p&1) == 0)	return 0;

	/* We fail for numbers >= 2^63 */
	if ((p & 0x8000000000000000ull) != 0) {
		err(ERR_DOMAIN);
		return 1;
	}

	/* Quick check for divisibility by small primes */
	for (i=1; i<N_PRIMES; i++)
		if (p == primes[i])
			return 1;
		else if ((p % primes[i]) == 0)
			return 0;
	if (p < QUICK_CHECK)
		return 1;

	s = p - 1;
	while ((s&1) == 0)
		s /= 2;

	for(i=0; i<PRIME_ITERATION; i++) {
		unsigned long long int temp = s;
		unsigned long long int mod = expmod(primes[i], temp, p);
		while (temp != p-1 && mod != 1 && mod != p-1) {
			mod = mulmod(mod, mod, p);
			temp += temp;
		}
		if(mod!=p-1 && temp%2==0)
			return 0;
	}
#endif
	return 1;
}

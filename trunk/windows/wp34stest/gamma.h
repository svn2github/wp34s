/**
 * Copyright (c) 2011 voidware ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#define K 18
#define GG 18

typedef __int64 int64_t;

template<class N> N _gammaFactorialSlow(const N& z)
{
    /* calculate gamma(z+1) = z!. using lanczos expansion.
     *
     * This implementation is VERY slow and not useful in practice.
     * However, it can be useful for testing, since it does not need
     * precomputed constants.
     */

    N t1;
    N t2;
    N s;
    N t;
    N fgc[K+1];
    N pks;
    N aa;
    int64_t a[K];
    int64_t b[K+1];
    int64_t tb;
    int i, j;
    N half = N(1)/2;

    a[0] = 1;
    b[0] = -1;
    b[1] = 2;

    fgc[0] = exp(GG+half)/sqrt(GG+half);
    s = fgc[0]*half;

    aa = half;
    t = 1;
    i = 1;
    for (;;) 
    {
        t *= (z+(1-i))/(z + i);

        t1 = i + half;
        t2 = t1 + GG;

        fgc[i] = aa*exp(t2 - log(t2)*t1);
        aa *= t1;

        pks = 0;
        for (j = 0; j <= i; ++j) 
            pks += b[j]*fgc[j];

        s += pks*t;

        if (i == K) break;

        a[i] = 0;
        b[i+1] = 0;

        tb = b[0];
        for (j = 0; j <= i; ++j) 
        {
            a[j] = -a[j] + 2*tb;
            tb = b[j+1];
            b[j+1] = -b[j+1] + 2*a[j];
        }

        b[0] = -b[0];
        ++i;
    }

    t1 = z + half;
    t2 = t1 + GG;
    return 2*exp(t1*log(t2)-t2)*s;
}

template<class N> N gammaFactorialSlow(const N& x)
{
    // NB: x > 0

    N v(1);
    N z(x);
    // reduce arg until less than 2, then use lanczos
    while (z >= 2)
    {
        v *= z;
        z -= 1;
    }
    return v*_gammaFactorialSlow(z);
}

#undef K
#undef GG

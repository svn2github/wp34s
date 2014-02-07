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

#ifdef INCLUDE_ZETA
// Reiman's Zeta function
/*
    \zeta(0) = -1/2,\!

    \zeta(1/2) = approx -1.4603545088095868,\!

    \zeta(1) =  = \infty;

    \zeta(3/2) \approx 2.612;\! 
    \zeta(2) =  \frac{\pi^2}{6} \approx 1.645;\! 

    \zeta(5/2) \approx 1.341.\!

    \zeta(3) =  \approx 1.202;\! 

    \zeta(7/2) \approx 1.127\!

    \zeta(4)  = \frac{\pi^4}{90} \approx 1.0823;\! 

    \zeta(6) =  \frac{\pi^6}{945} \approx 1.0173;\! 
*/
#ifndef TINY_BUILD
const decNumber *const zeta_consts[30] = {
	&const_zetaC00, &const_zetaC01, &const_zetaC02, &const_zetaC03,
	&const_zetaC04, &const_zetaC05, &const_zetaC06, &const_zetaC07,
	&const_zetaC08, &const_zetaC09, &const_zetaC10, &const_zetaC11,
	&const_zetaC12, &const_zetaC13, &const_zetaC14, &const_zetaC15,
	&const_zetaC16, &const_zetaC17, &const_zetaC18, &const_zetaC19,
	&const_zetaC20, &const_zetaC21, &const_zetaC22, &const_zetaC23,
	&const_zetaC24, &const_zetaC25, &const_zetaC26, &const_zetaC27,
	&const_zetaC28, &const_zetaC29
};

static void zeta_step(decNumber *sum, const decNumber *x,
		const decNumber *dc, decNumber *k) {
	decNumber t, s;

	busy();
	dn_inc(k);
	dn_power(&s, k, x);
	dn_divide(&t, dc, &s);
	dn_add(sum, sum, &t);
}
#endif

decNumber *decNumberZeta(decNumber *res, const decNumber *xin) {
#ifndef TINY_BUILD
	decNumber s, x, u, reflecfac, sum, t;
	int reflec, i;

	if (decNumberIsSpecial(xin)) {
		if (decNumberIsNaN(xin) || decNumberIsNegative(xin))
			return set_NaN(res);
		return dn_1(res);
	}
	if (dn_eq0(xin)) {
		return decNumberCopy(res, &const__0_5);
	}
	if (decNumberIsNegative(xin)) {
		dn_div2(&s, xin);
		if (is_int(&s)) {
			return decNumberZero(res);
		}
	}

	dn_compare(&s, xin, &const_0_5);
	if (decNumberIsNegative(&s)) {
		/* use reflection formula
		 * zeta(x) = 2^x*Pi^(x-1)*sin(Pi*x/2)*gamma(1-x)*zeta(1-x)
		 */
		reflec = 1;
		dn_1m(&x, xin);
		// Figure out xin * PI / 2 mod 2PI
		decNumberMod(&s, xin, &const_4);
		dn_multiply(&u, &const_PIon2, &s);
		sincosTaylor(&u, &s, res);
		dn_power(res, &const_2, xin);
		dn_multiply(&u, res, &s);
		dn_power(res, &const_PI, &x);
		dn_divide(&s, &u, res);
		decNumberGamma(res, &x);
		dn_multiply(&reflecfac, &s, res);
	} else {
		reflec = 0;
		decNumberCopy(&x, xin);
	}

	/* Now calculate zeta(x) where x >= 0.5 */
	decNumberZero(&sum);
	decNumberZero(&t);
	for (i=0; i<30; i++)
		zeta_step(&sum, &x, zeta_consts[i], &t);

	dn_1m(&t, &x);
	dn_power(&u, &const_2, &t);
	dn_m1(&t, &u);
	dn_multiply(&u, &t, &const_zeta_dn);
	dn_divide(res, &sum, &u);

	/* Finally, undo the reflection if required */
	if (reflec)
		dn_multiply(res, &reflecfac, res);
	return res;
#else
	return NULL;
#endif
}
#endif /* INCLUDE_ZETA */



/**************************************************************************/
/* From complex.c */


#ifdef INCLUDE_COMPLEX_ZETA
/* Riemann's Zeta function */
#ifndef TINY_BUILD
static void c_zeta_step(decNumber *sx, decNumber *sy,
		const decNumber *x, const decNumber *y,
		const decNumber *dc, decNumber *k) {
	decNumber t1, t2, s1, s2;

	busy();
	dn_inc(k);
	cmplxRealPower(&s1, &s2, k, x, y);
	cmplxDivideRealBy(&t1, &t2, dc, &s1, &s2);
	cmplxAdd(sx, sy, sx, sy, &t1, &t2);
}
#endif

void cmplxZeta(decNumber *rx, decNumber *ry,
		const decNumber *xin, const decNumber *yin) {
#ifndef TINY_BUILD
	decNumber s1, s2, x, y, u1, u2, reflecfac1, reflecfac2, sum1, sum2, t1, t2;
	int reflec, i;

	if (decNumberIsSpecial(xin) || decNumberIsSpecial(yin)) {
		if (decNumberIsNaN(xin) || decNumberIsNegative(xin) || decNumberIsNaN(yin)) {
			cmplx_NaN(rx, ry);
		} else {
			dn_1(rx);
			decNumberZero(ry);
		}
		return;
	}

	/* Short cut the real case by using the real version of the function */
#if 0
	if (dn_eq0(yin)) {
		decNumberZeta(rx, xin);
		decNumberZero(ry);
		return;
	}
#endif

	dn_compare(&s1, xin, &const_0_5);
	if (decNumberIsNegative(&s1)) {
		/* use reflection formula
		 * zeta(x) = 2^x*Pi^(x-1)*sin(Pi*x/2)*gamma(1-x)*zeta(1-x)
		 */
		reflec = 1;
		cmplxSubtractFromReal(&x, &y, &const_1, xin, yin);
		cmplxMultiplyReal(&u1, &u2, xin, yin, &const_PIon2);
		cmplxSin(&s1, &s2, &u1, &u2);
		cmplxRealPower(rx, ry, &const_2, xin, yin);
		cmplxMultiply(&u1, &u2, rx, ry, &s1, &s2);
		cmplxRealPower(rx, ry, &const_PI, &x, &y);
		cmplxDivide(&s1, &s2, &u1, &u2, rx, ry);
		cmplxGamma(rx, ry, &x, &y);
		cmplxMultiply(&reflecfac1, &reflecfac2, &s1, &s2, rx, ry);
	} else {
		reflec = 0;
		cmplxCopy(&x, &y, xin, yin);
	}

	/* Now calculate zeta(x) where x >= 0.5 */
	decNumberZero(&sum1);
	decNumberZero(&sum2);
	decNumberZero(&t1);
	for (i=0; i<30; i++)
		c_zeta_step(&sum1, &sum2, &x, &y, zeta_consts[i], &t1);

	cmplxSubtractFromReal(&t1, &t2, &const_1, &x, &y);
	cmplxRealPower(&u1, &u2, &const_2, &t1, &t2);
	dn_dec(&u1);
	cmplxMultiplyReal(&t1, &t2, &u1, &u2, &const_zeta_dn);
	cmplxDivide(rx, ry, &sum1, &sum2, &t1, &t2);

	/* Finally, undo the reflection if required */
	if (reflec)
		cmplxMultiply(rx, ry, &reflecfac1, &reflecfac2, rx, ry);
#endif
}
#endif /* INCLUDE_COMPLEX_ZETA */



/**************************************************************************/
/* From decn.c */


#ifdef INCLUDE_BERNOULLI
decNumber *decNumberBernBn(decNumber *r, const decNumber *n) {
#ifndef TINY_BUILD
	decNumber a, b;

	if (dn_eq0(n))
		return dn_1(r);
	if (! is_int(n) || decNumberIsNegative(n)) {
		return set_NaN(r);
	}
	dn_1m(&a, n);
	if (dn_eq0(&a))
		return decNumberCopy(r, &const__0_5);
	if (is_even(n)) {
#if 0
		// Values 306 and larger give infinite results, this is shortcut code
		// to save some time -- using 360 as the threshold to reuse a constant.
		if (decNumberIsNegative(dn_compare(&b, &const_360, n))) {
			dn_div2(&b, n);
			if (is_even(&b))
				return set_neginf(r);
			return set_inf(r);
		}
#endif
		decNumberZeta(&b, &a);
		dn_multiply(&a, &b, n);
		return dn_minus(r, &a);
	}
	decNumberZero(r);
	return r;
#else
	return NULL;
#endif
}

decNumber *decNumberBernBnS(decNumber *r, const decNumber *n) {
#ifndef TINY_BUILD
	decNumber a;

	if (dn_eq0(n)) {
		return set_NaN(r);
	}
	dn_mul2(&a, n);
	decNumberBernBn(r, &a);
	return dn_abs(r, r);
#else
	return NULL;
#endif
}
#endif



/**************************************************************************/
/* This goes into compile consts */



#ifdef INCLUDE_ZETA
	{ DFLT, "zeta_dn",		"46292552162781456490000.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC00",		"-46292552162781456489999.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC01",		"46292552162781456488199.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC02",		"-46292552162781455948799.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC03",		"46292552162781391508479.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC04",		"-46292552162777290342399.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC05",		"46292552162616160083967.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC06",		"-46292552158343766867967.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC07",		"46292552077215245139967.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC08",		"-46292550926542378631167.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC09",		"46292538351868961619967.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC10",		"-46292429944947608649727.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC11",		"46291679074496678985727.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC12",		"-46287440465212083273727.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC13",		"46267721150632671838207.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC14",		"-46191452267259392688127.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC15",		"45944586548202893737983.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC16",		"-45272673804803148611583.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC17",		"43730029217461131280383.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC18",		"-40737788446458043695103.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC19",		"35834429458982144835583.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC20",		"-29057735883983402565631.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC21",		"21187011638920984133631.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC22",		"-13549247519505233477631.99999999999999999999999999999999999999999999999999999999999999999999999999999"	},
	{ DFLT, "zetaC23",		"7409518295008707346431.99999999999999999999999999999999999999999999999999999999999999999999999999999"	},
	{ DFLT, "zetaC24",		"-3370795702299113029632.00000000000000000000000000000000000000000000000000000000000000000000000000000" },
	{ DFLT, "zetaC25",		"1234393873665792933888.00000000000000000000000000000000000000000000000000000000000000000000000000000"	},
	{ DFLT, "zetaC26",		"-348254351985305714688.00000000000000000000000000000000000000000000000000000000000000000000000000000"	},
	{ DFLT, "zetaC27",		"70832614939283161088.00000000000000000000000000000000000000000000000000000000000000000000000000001"	},
	{ DFLT, "zetaC28",		"-9223372036854775808.00000000000000000000000000000000000000000000000000000000000000000000000000001"	},
	{ DFLT, "zetaC29",		"576460752303423488.00000000000000000000000000000000000000000000000000000000000000000000000000001"	},
#endif

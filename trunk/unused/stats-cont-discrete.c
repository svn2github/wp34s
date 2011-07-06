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


/* Utility routine to finialise a discrete distribtuion result that might be
 * off by one.  The arugments are the best guess of the reslt, the target
 * probability, the cdf at the best guess, the cdf at the best guess plus one
 * and the best guess plus one.
 *
 * A comparision is made of the best guess and if it is too high, the result
 * must be the best guess minus one.  Likewise a comparision is made to
 * determine if the best guess plus one is below the target.
 */
#ifdef DISCRETE_DISTRIUBITIONS
static decNumber *discrete_final_check(decNumber *r, const decNumber *p, const decNumber *fr, const decNumber *frp1, const decNumber *rp1) {
	decNumber a;

	if (decNumberIsNegative(dn_compare(&a, p, fr)))
		dn_dec(r);
	else if (dn_le0(dn_compare(&a, frp1, p)))
		decNumberCopy(r, rp1);
	return r;
}
#endif


/* Optimise an estimate using Newton's method.  For statistical distributions we are fortunate that we've got both the
 * function and its derivative so this is straightforward.
 * Since the different distributions need slightly different flavours of this search, we add a parameter to customise.
 */
#define NEWTON_WANDERCHECK	0x0001
#define NEWTON_NONNEGATIVE	0x0002
#ifdef DISCRETE_DISTRIUBITIONS
#define NEWTON_DISCRETE		0x0004
#else
#define NEWTON_DISCRETE		0
#endif

static decNumber *newton_qf(decNumber *r, const decNumber *p, const unsigned short int flags,
				decNumber *(*pdf)(decNumber *r, const decNumber *x, const decNumber *a1, const decNumber *a2),
				decNumber *(*cdf)(decNumber *r, const decNumber *x, const decNumber *a1, const decNumber *a2),
				const decNumber *arg1, const decNumber *arg2, const decNumber *maxstep) {
	decNumber w, x, z, md, prev;
	int i;
#ifdef DISCRETE_DISTRIUBITIONS
	decNumber v;
	const int discrete = (flags & NEWTON_DISCRETE) != 0;
#endif
	const int nonnegative = (flags & NEWTON_NONNEGATIVE) != 0;
	const int wandercheck = (flags & NEWTON_WANDERCHECK) != 0;

	if (maxstep != NULL)
		dn_multiply(&md, r, maxstep);

	for (i=0; i<50; i++) {
//dump1(r, "est");
		dn_subtract(&z, (*cdf)(&w, r, arg1, arg2), p);

		// Check if things are getting worse
		if (wandercheck) {
			dn_abs(&x, &z);
			if (i > 0 && decNumberIsNegative(dn_compare(&w, &prev, &x)))
				return set_NaN(r);
			decNumberCopy(&prev, &x);
		}
#ifdef DISCRETE_DISTRIUBITIONS
		if (discrete) {
			// Using the pdf for the slope isn't great for the discrete distributions
			// So we do something more akin to a secant approach
			dn_add(&v, r, &const_0_001);
			(*cdf)(&x, &v, arg1, arg2);
			dn_subtract(&v, &x, &w);
			dn_multiply(&x, &v, &const_1000);
		} else
#endif
			(*pdf)(&x, r, arg1, arg2);
		if (decNumberIsZero(&x))
			break;
		dn_divide(&w, &z, &x);

		// Limit the step size if necessary
		if (maxstep != NULL) {
			dn_abs(&x, &w);
			if (decNumberIsNegative(dn_compare(&z, &md, &x))) {
				if (decNumberIsNegative(&w))
					dn_minus(&w, &md);
				else
					decNumberPlus(&w, &md, &Ctx);
			}
		}

		// Update estimate
		decNumberCopy(&z, r);
		dn_subtract(r, &z, &w);

		// If this distribution doesn't take negative values, limit outselves to positive ones
		if (nonnegative && decNumberIsNegative(r))
			dn_multiply(r, &z, &const_0_00001);

		// Check for finished
#ifdef DISCRETE_DISTRIUBITIONS
		if (discrete) {
			if (absolute_error(r, &z, DISCRETE_TOLERANCE))
				break;
		} else
#endif
			if (relative_error(r, &z, &const_1e_24))
				break;
		busy();
	}
#ifdef DISCRETE_DISTRIUBITIONS
	if (discrete) {
		decNumberFloor(r, r);
		(*cdf)(&x, r, arg1, arg2);
		dn_add(&w, r, &const_1);
		(*cdf)(&z, &w, arg1, arg2);
		return discrete_final_check(r, p, &x, &z, &w);
	}
#endif
	return r;
}



decNumber *pdf_B(decNumber *r, const decNumber *x) {
	decNumber n, p;

	if (binomial_param(r, &p, &n, x))
		return r;
#ifdef DISCRETE_DISTRIUBITIONS
	if (!is_int(x))
		return decNumberZero(r);
#endif
	return pdf_B_helper(r, x, &p, &n);
}
decNumber *cdf_B(decNumber *r, const decNumber *x) {
	decNumber n, p;

	if (binomial_param(r, &p, &n, x))
		return r;
#ifdef DISCRETE_DISTRIUBITIONS
	{ decNumber fx;
		decNumberFloor(&fx, x);
		return cdf_B_helper(r, &fx, &p, &n);
	}
#else
	return cdf_B_helper(r, x, &p, &n);
#endif
}


#ifdef DISCRETE_DISTRIUBITIONS
static void qf_B_est(decNumber *r, const decNumber *p, const decNumber *prob, const decNumber *n) {
	decNumber mu, sigma;

	dn_multiply(&mu, prob, n);
	dn_subtract(&sigma, &const_1, p);
	dn_multiply(r, &mu, &sigma);
	dn_sqrt(&sigma, r);
	normal_approximation_via_moment(r, p, &mu, &sigma);
}
#endif

decNumber *qf_B(decNumber *r, const decNumber *pp) {
	decNumber p, n;

	if (binomial_param(r, &p, &n, pp))
		return r;
	if (check_probability(r, pp, 1))
		return r;
#ifdef DISCRETE_DISTRIUBITIONS
	qf_B_est(r, pp, &p, &n);
	newton_qf(r, pp, NEWTON_DISCRETE | NEWTON_NONNEGATIVE, &pdf_B_helper, &cdf_B_helper, &p, &n, NULL);
	return dn_min(r, r, &n);
#else
	{
		decNumber q, t, u, v, x, pdf, cdf;

		dn_subtract(&q, &const_1, &p);
		dn_power(&t, &p, &n);			// t= p^n
		dn_subtract(&u, &const_1, &t);		// u = 1 - p^n
		if (dn_le0(dn_compare(&v, &u, pp))) {	// direct solution for p close to 1
			dn_subtract(&v, &const_1, pp);
			dn_divide(&u, &v, &t);
			return dn_subtract(r, &n, &u);
		}
		dn_power(&t, &q, &n);
		if (dn_le0(dn_compare(&v, pp, &t))) {
			dn_subtract(&u, &t, &const_1);
			return dn_divide(r, pp, &u);	// direct solution for p close to 0
		}
		dn_subtract(&t, &const_0_5, pp);
		qf_Q_est(&v, pp, &t);
		decNumberSquare(&t, &v);		// Z * Z
		dn_dec(&t);				// Z*Z - 1
		dn_divide(&u, &t, &const_6);		// (Z*Z-1) / 6
		dn_add(&t, &u, &v);			// Z + (Z*Z-1)/6
		dn_multiply(&x, &n, &p);		// n p
		dn_multiply(&u, &x, &q);		// n p (1-p)
		decNumberSquare(&v, &u);
		dn_multiply(&u, &v, &t);
		dn_add(&t, &u, &x);
		decNumberTrunc(&x, &t);
		dn_max(&x, &x, &const_0);
		if (decNumberIsNegative(&x))
			decNumberZero(&x);
		else
			dn_min(&x, &x, dn_subtract(&t, &n, &const_1));
		cdf_B_helper(&cdf, &x, &p, &n);
		pdf_B_helper(&pdf, &x, &p, &n);
		dn_divide(&q, &p, &q);
		dn_compare(&t, pp, &cdf);
		if (decNumberIsZero(&t))
			return decNumberCopy(r, &x);
		if (decNumberIsNegative(&t)) {
			for (;;) {
				decNumberCopy(&v, &pdf);
				dn_subtract(&cdf, &cdf, &v);
				dn_subtract(&t, &n, &x);
				dn_inc(&t);
				dn_multiply(&u, &t, &q);
				dn_divide(&t, &pdf, &u);
				dn_multiply(&pdf, &t, &x);
				dn_dec(&x);
				if (dn_le0(dn_compare(&t, pp, &cdf)))
					break;
			}
		} else {
			for (;;) {
				dn_inc(&x);
				dn_subtract(&t, &n, &x);
				dn_inc(&t);
				dn_multiply(&u, &t, &q);
				dn_multiply(&t, &u, &pdf);
				dn_divide(&pdf, &t, &x);
				decNumberCopy(&v, &pdf);
				dn_add(&cdf, &cdf, &v);
				if (dn_le0(dn_compare(&t, pp, &cdf)))
					break;
			}
		}
#if 0
		dn_subtract(&t, pp, &cdf);
		dn_divide(&u, &t, &v);
		return dn_add(r, &x, &u);
#endif
		return decNumberCopy(r, &x);
	}
#endif
}




decNumber *pdf_P(decNumber *r, const decNumber *x) {
	decNumber lambda;

	if (poisson_param(r, &lambda, x))
		return r;
#ifdef DISCRETE_DISTRIUBITIONS
	if (!is_int(x))
		return decNumberZero(r);
#endif
	return pdf_P_helper(r, x, &lambda, NULL);
}


decNumber *cdf_P(decNumber *r, const decNumber *x) {
	decNumber lambda;

	if (poisson_param(r, &lambda, x))
		return r;
#ifdef DISCRETE_DISTRIUBITIONS
	{ decNumber fx;
		decNumberFloor(&fx, x);
		return cdf_P_helper(r, &fx, &lambda, NULL);
	}
#else
	return cdf_P_helper(r, x, &lambda, NULL);
#endif
}



decNumber *pdf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, v;

	if (geometric_param(r, &p, x))
		return r;
#ifdef DISCRETE_DISTRIUBITIONS
	if (dn_lt0(x) || !is_int(x))
#else
	if (dn_lt0(x))
#endif
	{
		decNumberZero(r);
		return r;
	}
	dn_ln1m(&t, &p);
	dn_multiply(&v, &t, x);
	dn_exp(&t, &v);
	return dn_multiply(r, &t, &p);
}

decNumber *cdf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, u, v;

	if (geometric_param(r, &p, x))
		return r;
	if (dn_lt0(x))
		return decNumberZero(r);
	if (decNumberIsInfinite(x))
		return decNumberCopy(r, &const_1);

	dn_ln1m(&u, &p);
#ifdef DISCRETE_DISTRIUBITIONS
	decNumberFloor(&t, x);
	dn_add(&v, &t, &const_1);
#else
	dn_add(&v, x, &const_1);
#endif
	dn_multiply(&t, &u, &v);
	decNumberExpm1(&u, &t);
	return dn_minus(r, &u);
}

decNumber *qf_G(decNumber *r, const decNumber *x) {
	decNumber p, t, v;

	if (geometric_param(r, &p, x))
		return r;
	if (check_probability(r, x, 1))
		return r;
	dn_ln1m(&v, x);
	dn_ln1m(&t, &p);
#ifdef DISCRETE_DISTRIUBITIONS
	{ decNumber z;
		dn_divide(&p, &v, &t);
		dn_dec(&p);
		decNumberFloor(r, &p);

		/* Not sure this is absolutely necessary but it can't hurt */
		cdf_G(&t, r);
		dn_add(&v, r, &const_1);
		cdf_G(&z, &v);
		return discrete_final_check(r, &p, &t, &z, &v);
	}
#else
	dn_divide(r, &v, &t);
	dn_dec(r);
	return r;
#endif
}

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

/**********************************************************************/
/* Orthogonal polynomial evaluations                                  */
/**********************************************************************/
// 532 bytes in total


// Orthogonal polynomial types
enum eOrthoPolys {
	ORTHOPOLY_LEGENDRE_PN,
	ORTHOPOLY_CHEBYCHEV_TN,
	ORTHOPOLY_CHEBYCHEV_UN,
	ORTHOPOLY_GEN_LAGUERRE,
	ORTHOPOLY_HERMITE_HE,
	ORTHOPOLY_HERMITE_H,
};

// 452 bytes
static decNumber *ortho_poly(decNumber *r, const decNumber *param, const decNumber *rn, const decNumber *x, const enum eOrthoPolys type) {
	decNumber t0, t1, t, u, v, A, B, C, dA;
	unsigned int i, n;
	int incA, incB, incC;

	// Get argument and parameter
	if (decNumberIsSpecial(x) || decNumberIsSpecial(rn) || dn_lt0(rn)) {
error:		return set_NaN(r);
	}
	if (! is_int(rn))
		goto error;
	n = dn_to_int(rn);
	if (n > 1000)
		goto error;
//	if (type == ORTHOPOLY_GEN_LAGUERRE) {
	if (param != NULL) {
		if (decNumberIsSpecial(param))
			goto error;
		dn_p1(&t, param);
		if (dn_le0(&t))
			goto error;
	}
//	} else
//		param = &const_0;

	// Initialise the first two values t0 and t1
	switch (type) {
	default:
		decNumberCopy(&t1, x);
		break;
	case ORTHOPOLY_HERMITE_H:
	case ORTHOPOLY_CHEBYCHEV_UN:
		dn_mul2(&t1, x);
		break;
	case ORTHOPOLY_GEN_LAGUERRE:
		dn_p1(&t, param);
		dn_subtract(&t1, &t, x);
		break;
	}
	dn_1(&t0);

	if (n < 2) {
		if (n == 0)
			decNumberCopy(r, &t0);
		else
			decNumberCopy(r, &t1);
		return r;
	}

	// Prepare for the iteration
	decNumberCopy(&dA, &const_2);
	dn_1(&C);
	dn_1(&B);
	dn_mul2(&A, x);
	incA = incB = incC = 0;
	switch (type) {
	case ORTHOPOLY_LEGENDRE_PN:
		incA = incB = incC = 1;
		dn_add(&A, &A, x);
		dn_mul2(&dA, x);
		break;
	case ORTHOPOLY_CHEBYCHEV_TN:	break;
	case ORTHOPOLY_CHEBYCHEV_UN:	break;
	case ORTHOPOLY_GEN_LAGUERRE:
		dn_add(&B, &B, param);
		incA = incB = incC = 1;
		dn_add(&t, &const_3, param);
		dn_subtract(&A, &t, x);
		break;
	case ORTHOPOLY_HERMITE_HE:
		decNumberCopy(&A, x);
		incB = 1;
		break;
	case ORTHOPOLY_HERMITE_H:
		decNumberCopy(&B, &const_2);
		incB = 2;
		break;
	}

	// Iterate
	for (i=2; i<=n; i++) {
		dn_multiply(&t, &t1, &A);
		dn_multiply(&u, &t0, &B);
		dn_subtract(&v, &t, &u);
		decNumberCopy(&t0, &t1);
		if (incC) {
			dn_inc(&C);
			dn_divide(&t1, &v, &C);
		} else
			decNumberCopy(&t1, &v);
		if (incA)
			dn_add(&A, &A, &dA);
		if (incB)
			dn_add(&B, &B, small_int(incB));
	}
	return decNumberCopy(r, &t1);
}

// 36 bytes
decNumber *decNumberPolyCommon(decNumber *r, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, NULL, y, x, argKIND(XeqOpCode) - OP_LEGENDRE_PN + ORTHOPOLY_LEGENDRE_PN);
}

// 28 bytes
decNumber *decNumberPolyLn(decNumber *r, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, &const_0, y, x, ORTHOPOLY_GEN_LAGUERRE);
}

// 16 bytes
decNumber *decNumberPolyLnAlpha(decNumber *r, const decNumber *z, const decNumber *y, const decNumber *x) {
	return ortho_poly(r, z, y, x, ORTHOPOLY_GEN_LAGUERRE);
}




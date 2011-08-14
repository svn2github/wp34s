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

#include "xrom.h"
#include "xeq.h"
#include "consts.h"

/* Define all the commands we'll be using to simplify coding */
#define st(n)		reg ## n ## _idx
#define ALPHA(c)	RARG(RARG_ALPHA, (c) & 0xff
#define xRARG(op, n)	RARG((RARG_ ## op), ((n) & RARG_MASK)),
#define xCONST(n)	CONST(OP_ ## n),
#define iCONST(n)	CONST_INT(OP_ ## n),
#define iCONSTIND(n)	RARG_IND | xRARG(CONST_INT, n)
#define NILADIC(n)	(OP_NIL | (OP_ ## n)),
#define MONADIC(n)	(OP_MON | (OP_ ## n)),
#define DYADIC(n)	(OP_DYA | (OP_ ## n)),
#define CMONADIC(n)	(OP_CMON | (OP_ ## n)),
#define CDYADIC(n)	(OP_CDYA | (OP_ ## n)),
#define SPECIAL(n)	(OP_SPEC | OP_ ## n),
#define xMULTI(n,a,b,c)	(OP_DBL | ((DBL_ ## n) << DBL_SHIFT) | ((a) & 0xff)), \
			((b) & 0xff) | (((c) << 8) & 0xff00),
#define xCONV(n, d)	RARG(RARG_CONV, (OP_ ## n)*2 + (d)),

// Specials
#define ENTER		SPECIAL(ENTER)
#define specCLX		SPECIAL(CLX)
#define EEX		SPECIAL(EEX)
#define CHS		SPECIAL(CHS)
#define DOT		SPECIAL(DOT)
#define DIG(n)		(OP_SPEC | (OP_0 + (n))),
#define TST0(t)		(OP_SPEC | (OP_X ## t ## 0)),
#define TST1(t)		(OP_SPEC | (OP_X ## t ## 1)),

// Commands that take an argument
#define ERROR(n)	xRARG(ERROR, n)
#define LBL(n)		xRARG(LBL, n)
#define GTO(n)		xRARG(GTO, n)
#define GSB(n)		xRARG(XEQ, n)
#define SKIP(n)		xRARG(SKIP, n)
#define BACK(n)		xRARG(BACK, n)
#define STO(r)		xRARG(STO, r)
#define RCL(r)		xRARG(RCL, r)
#define STO_PL(r)	xRARG(STO_PL, r)
#define RCL_PL(r)	xRARG(RCL_PL, r)
#define STO_MI(r)	xRARG(STO_MI, r)
#define RCL_MI(r)	xRARG(RCL_MI, r)
#define STO_MU(r)	xRARG(STO_MU, r)
#define RCL_MU(r)	xRARG(RCL_MU, r)
#define STO_DV(r)	xRARG(STO_DV, r)
#define RCL_DV(r)	xRARG(RCL_DV, r)
#define STO_MIN(r)	xRARG(STO_MIN, r)
#define RCL_MIN(r)	xRARG(RCL_MIN, r)
#define STO_MAX(r)	xRARG(STO_MAX, r)
#define RCL_MAX(r)	xRARG(RCL_MAX, r)
#define CSTO(r)		xRARG(CSTO, r)
#define CRCL(r)		xRARG(CRCL, r)
#define SWAP(r)		xRARG(SWAP, r)
#define CSWAP(r)	xRARG(CSWAP, r)
#define ISG(r)		xRARG(ISG, r)
#define DSE(r)		xRARG(DSE, r)
#define ISZ(r)		xRARG(ISZ, r)
#define DSZ(r)		xRARG(DSZ, r)
#define INC(r)		xRARG(INC, r)
#define DEC(r)		xRARG(DEC, r)
#define TST(c, r)	xRARG(TEST_ ## c, r)
#define SF(f)		xRARG(SF, f)
#define CF(f)		xRARG(CF, f)
#define FSp(f)		xRARG(FS, f)
#define FCp(f)		xRARG(FC, f)
#define FSpS(f)		xRARG(FSS, f)
#define FSpC(f)		xRARG(FSC, f)
#define FSpF(f)		xRARG(FSF, f)
#define FCpS(f)		xRARG(FCS, f)
#define FCpC(f)		xRARG(FCC, f)
#define FCpF(f)		xRARG(FCF, f)
#define PAUSE(f)	xRARG(PAUSE, f)
#define VIEW(f)		xRARG(VIEW, f)
#define VIEWREG(f)	xRARG(VIEW_REG, f)
#define SR10(f)		xRARG(SRD, f)
#define SL10(f)		xRARG(SLD, f)
#define STOSTK(f)	xRARG(STOSTK, f)
#define RCLSTK(f)	xRARG(RCLSTK, f)

#define SLVI(f)		xRARG(INISOLVE, f)
#define SLVS(f)		xRARG(SOLVESTEP, f)

// Multiword commands
#define DLBLP(a, b, c)	xMULTI(LBLP, a, b, c)
#define DXEQ(a, b, c)	xMULTI(XEQ, a, b, c)
#define DLBL(a, b, c)	xMULTI(LBL, a, b, c)

// Alpha commands
#define alpha1(c)	xRARG(ALPHA, c)
#define alpha2(a, b)	alpha1(a)  alpha1(b)
#ifdef MULTI_ALPHA
#define alpha3(a, b, c)	xMULTI(ALPHA, a, b, c)
#else
#define alpha3(a, b, c)	alpha1(a)  alpha1(b)  alpha1(c)
#endif
#define alpha4(a,b,c,d)	alpha3(a,b,c)  alpha1(d)
#define alpha5(a,b,c,d,e)	alpha3(a,b,c)  alpha2(d,e)
#define alpha6(a,b,c,d,e,f)	alpha3(a,b,c)  alpha3(d,e,f)

// Define the constants we know about
#define PI		xCONST(PI)
#define ZERO		iCONST(ZERO)
#define E		xCONST(CNSTE)
#define NAN		xCONST(NAN)
#define INFINITY	xCONST(INF)

// And the useful niladic routines
#define RTN		NILADIC(RTN)
#define RTNp1		NILADIC(RTNp1)
#define GSBUSER		NILADIC(GSBuser)
#define AVIEW		NILADIC(VIEWALPHA)
#define DECM		NILADIC(FLOAT)
#define CLX		NILADIC(rCLX)
#define CLSTK		NILADIC(CLSTK)
#define CLALPHA		NILADIC(CLRALPHA)
#define STOP		NILADIC(RS)
#define TST_INFINITE	NILADIC(XisInf)
#define TST_NaN		NILADIC(XisNaN)
#define TST_SPECIAL	NILADIC(XisSpecial)
#define TST_EVEN	NILADIC(XisEVEN)
#define TST_ODD		NILADIC(XisODD)
#define TST_INT		NILADIC(XisINT)
#define TST_FRAC	NILADIC(XisFRAC)
#define TST_PRIME	NILADIC(XisPRIME)
#define TST_TOP		NILADIC(TOP)
#define VERS		NILADIC(VERSION)

// Mondaic functions
#define RECIP		MONADIC(RECIP)
#define ABS		MONADIC(ABS)
#define SIGN		MONADIC(SIGN)
#define RND		MONADIC(RND)
#define POW10		MONADIC(POW10)
#define FRAC		MONADIC(FRAC)
#define TRUNC		MONADIC(TRUNC)
#define FLOOR		MONADIC(FLOOR)
#define CEIL		MONADIC(CEIL)
#define SQUARE		MONADIC(SQR)
#define SQRT		MONADIC(SQRT)
#define LN1P		MONADIC(LN1P)
#define EXPM1		MONADIC(EXPM1)

// Dyadic functions
#define PLUS		DYADIC(ADD)
#define MINUS		DYADIC(SUB)
#define TIMES		DYADIC(MUL)
#define DIVISION	DYADIC(DIV)
#define MOD		DYADIC(MOD)
#define POWER		DYADIC(POW)

// Complex monadic
#define CCONJ		CMONADIC(CCONJ)

// Complex dyadic
#define CPLUS		CDYADIC(ADD)
#define CMINUS		CDYADIC(SUB)
#define CTIMES		CDYADIC(MUL)
#define CDIVISION	CDYADIC(DIV)

// Stack manipulation shortcuts
#define SWAPXY		NILADIC(SWAP)
#define CSWAPXY		NILADIC(CSWAP)
#define ROLLD		NILADIC(RDOWN)
#define ROLLU		NILADIC(RUP)
#define FILL		NILADIC(FILL)
#define CFILL		NILADIC(CFILL)
#define CENTER		NILADIC(CENTER)
#define DROP		NILADIC(DROP)

// Other short cuts
#define ENTRY		GSB(XROM_CHECK)
#define EXIT		GTO(XROM_EXIT)
#define EXITp1		GTO(XROM_EXITp1)

/* Now the xrom table itself.
 *
 * Global labels are from 10 and up.
 * Local labels are 0 through 19 and can only be used for forward
 * branches since they'll likely be redefined elsewhere.
 *
 * Banked registers are 0 - 4
 * Banked flags are 0 - 7
 */
#if defined(REALBUILD) && defined(__GNUC__)
// Special section name so that we can shuffle the code around in flash 
// with the linker
__attribute__((section(".xrom")))
#endif
const s_opcode xrom[] = {
	/* Solve code.
	 *
	 * On entry the stack looks like:
	 *	Y	Guess a
	 *	X	Guess b
	 *
	 * On return the stack looks like:
	 *	L	0
	 *	I	unchanged
	 *
	 *	T	0
	 *	Z	f(X)
	 *	Y	previous root estimate
	 *	X	root estimate
	 */

/* Register use */
#define SOLVE_REG_BASE	0
#define A		(0 + SOLVE_REG_BASE)
#define B		(1 + SOLVE_REG_BASE)
#define C		(2 + SOLVE_REG_BASE)
#define FA		(3 + SOLVE_REG_BASE)
#define FB		(4 + SOLVE_REG_BASE)

	LBL(ENTRY_SOLVE)
		ENTRY
		DECM
		TST(APX, st(Y))
			INC(st(Y))
		TST(APX, st(Y))
			SR10(1)
		TST(GT, st(Y))
			SWAPXY
		CSTO(A)
		FILL
		GSBUSER
		STO(FA)
		TST0(apx)
			GTO(2)

		RCL(B)
		FILL
		GSBUSER
		STO(FB)
		TST0(apx)
			GTO(3)
		SLVI(0)
		RCL(C)

//	LBL(SLV_LOOP)
		FILL
		GSBUSER
		FILL
		TST0(apx)
			GTO(1)
		SLVS(0)
		TST0(ne)		// Check for failure to complete
			GTO(5)
		RCL(C)
		TST(APX, A)
			SKIP(3)	//GTO(6)
		TST(APX, B)
			SKIP(1)	//GTO(6)
//		GTO(SLV_LOOP)
		BACK(13)
	//LBL(6)				// Limits are narrow -- either solved or pole
		RCL(st(Z))
		FILL
		TST0(apx)
			SKIP(2)	//GTO(7)
		GSB(8)
		EXIT
	//LBL(7)
		GSB(8)
//		EXITp1
		GTO(4)
	LBL(8)
		ZERO
		STO(st(L))
		RCL(st(T))
		RCL(B)
		RCL(C)
		RTN
	LBL(5)				// Failed
		RCL(FA)
		ABS
		RCL(FB)
		ABS
		TST(LT, st(Y))
			SKIP(7)	//GTO(5)
		RCL(st(T))
		STO(st(L))
		ZERO
		SWAPXY
		RCL(A)
		RCL(C)
//		EXITp1
		GTO(4)
	//LBL(5)
		RCL(st(T))
		STO(st(L))
		ZERO
		SWAPXY
		RCL(B)
		RCL(C)
//		EXITp1
	LBL(4)
		CF(F_XROM)
		TST_TOP
		ERROR(ERR_SOLVE)
		EXITp1

	LBL(1)				// Success
		ZERO
		STO(st(L))
		RCL(st(Z))
		RCL(B)
		RCL(C)
		EXIT
	LBL(3)				// Initial estimate good
		RCL(FB)
		RCL(B)
		SKIP(3)	// GTO(0)
	LBL(2)
		RCL(FA)
		RCL(A)
	//LBL(0)
		RCL(st(X))
		ZERO
		STO(st(L))
		CSWAPXY
		EXIT

#undef A
#undef B
#undef C
#undef FA
#undef FB

/**************************************************************************/

	/* Integrate code
	 * We're using a Gauss-Kronrod quadrature with 10 Guass points
	 * and 21 Kronrod points.  We calculate the Guass quadrature
	 * first so we can give an estimate if an error occurs during the
	 * extra evaluations of the Kronrod quadrature.
	 *
	 * On entry the stack looks like:
	 *	Y	lower limit
	 *	X	upper limit
	 *
	 * On return the stack looks like:
	 *	L	integral (Gauss)
	 *	I	unchanged
	 *
	 *	T	lower limit (Y on input)
	 *	Z	upper limit (X on input)
	 *	Y	error estimate (Gauss - Kronrod)
	 *	X	integral (Kronrod)
	 */

/* Register use */
#define INTG_REG_BASE	0
#define HALF_LEN	(0 + INTG_REG_BASE)
#define CENTRE		(1 + INTG_REG_BASE)
#define GAUSS		(2 + INTG_REG_BASE)
#define KRONROD		(3 + INTG_REG_BASE)
#define I		(4 + INTG_REG_BASE)

/* No flags used */


	LBL(ENTRY_INTEGRATE)
		ENTRY
		DECM
		TST_SPECIAL
			GTO(8)
		SWAPXY
		TST_SPECIAL
			GTO(8)
		TST(EQ, st(Y))		// Check if range is zero
			GTO(9)
		SWAPXY

		/* Compute (x-y)/2 and (x+y)/2 */
		CENTER
		PLUS
		DIG(2)
		DIVISION
		SWAP(st(Z))
		MINUS
		DIG(2)
		DIVISION
		CSTO(HALF_LEN)		// Also stores the midpoint

		/* Initialise the summations */
		ZERO
		STO(GAUSS)
		STO(KRONROD)

		/* Loop through the common points */
		iCONST(GKL)
		STO(I)

		//LBL(GKLOOP)
			GSB(4)
			GSBUSER
			TST_SPECIAL
				GTO(8)
			GSB(7)

			GSB(5)
			GSBUSER
			TST_SPECIAL
				GTO(8)
			GSB(7)
			ISG(I)
			BACK(11)	//GTO(GKLOOP)
		RCL(HALF_LEN)
		STO_MU(GAUSS)

		/* Evaluate at midpoint for the Kronrod estimate */
		RCL(CENTRE)
		FILL
		GSBUSER
		TST_SPECIAL
			GTO(2)
		iCONST(WGK10)
		TIMES
		STO_PL(KRONROD)

		/* Now loop through the Kronrod points */
		iCONST(KL)
		STO(I)

		//LBL(KLOOP)
			GSB(4)
			GSBUSER
			TST_SPECIAL
				GTO(2)
			GSB(6)

			GSB(5)
			GSBUSER
			TST_SPECIAL
				GTO(2)
			GSB(6)
			ISG(I)
			BACK(11)	//GTO(KLOOP)
		RCL(HALF_LEN)
		STO_MU(KRONROD)


		/* Set up the stack for our output */
		GSB(3)
		RCL(KRONROD)
		RCL_MI(GAUSS)		// err, l, u, ?, G
		RCL(KRONROD)		// K, err, l, u, G		
		EXIT

	LBL(4)				// Calculate the first point from the xi
		iCONSTIND(I)
		RCL_MU(HALF_LEN)
		RCL_PL(CENTRE)
		FILL
		RTN
	LBL(5)				// Calculate the second point from the xi
		RCL(CENTRE)
		iCONSTIND(I)
		RCL_MU(HALF_LEN)
		MINUS
		FILL
		RTN

	LBL(6)				// Do a Kronrod accumulation, f(xi) in X
		RCL(I)			// i, f, ?, ?
		INC(st(X))		// i+1, f, ?, ?
		iCONSTIND(st(X))	// ki, i+1, f, ?
		STO(st(L))
		RCL(st(Z))		// f, ki, i+1, f
		FILL			// f, f, f, f
		RCL_MU(st(L))		// ki*f, f, f, f
		STO_PL(KRONROD)
		RTN
	LBL(7)				// Gauus Kronrod accumulation, f(xi) in X
		RCL(I)
		INC(st(X))		// I+1, f, ?, ?
		RCL(st(Y))		// f, I+1, f, ?
		RCL(st(X))		// f, f, I+1, f
		iCONSTIND(st(Z))	// gi, f, f, I+1
		INC(st(T))		// gi, f, f, I+2
		TIMES			// gi*f, f, I+2, I+2
		STO_PL(GAUSS)
		DROP			// f, I+2, I+2, I+2
		iCONSTIND(st(Z))	// ki, f, I+2, I+2
		TIMES			// ki*f, I+2, I+2, I+2
		STO_PL(KRONROD)
		RTN

	LBL(9)				// Initial estimates equal
		GSB(3)
		ZERO
		ENTER
		STO(st(L))
		EXIT
	LBL(8)				// A value or limit is NaN
		GSB(3)
		NAN
		ENTER
		STO(st(L))
		EXIT
	LBL(2)				// A value is NaN after the Gauss estimate
		GSB(3)
		NAN
		RCL(GAUSS)
		STO(st(L))
		EXIT
	LBL(3)				// Restore the integration limits
		RCL(CENTRE)
		RCL_MI(HALF_LEN)
		RCL(CENTRE)
		RCL_PL(HALF_LEN)	// l, u
		RTN
#undef HALF_LEN
#undef CENTRE
#undef GAUSS
#undef KRONROD
#undef I

/**************************************************************************/
/* Sigma and products */

#define SIGMA_REG_BASE	0
#define I	(0 + SIGMA_REG_BASE)
#define PRODSUM	(1 + SIGMA_REG_BASE)
#define C	(2 + SIGMA_REG_BASE)
#define SAV_I	(3 + SIGMA_REG_BASE)

	LBL(ENTRY_SIGMA)
		ENTRY
		DECM
		TST_SPECIAL
			GTO(9)
		STO(I)
		STO(SAV_I)		// Save for LastX
		TRUNC			// First function call is separate
		FILL			// to avoid Kahan summing from zero
		GSBUSER			// six extra instructions save nine
		TST_SPECIAL		// from executing
			GTO(8)
		STO(PRODSUM)
		ZERO
		STO(C)
		SKIP(14)	//GTO(1)
	//LBL(SIGMA_LOOP)
		RCL(I)
		TRUNC
		FILL
		GSBUSER
		TST_SPECIAL
			GTO(8)
		RCL_MI(C)		// Kahan sum y = Xn - c
		ENTER			// y y . .
		RCL_PL(PRODSUM)		// t = sum + y  y . .
		ENTER			// t t y .
		RCL_MI(PRODSUM)		// t-sum  t y .
		RCL_MI(st(Z))		// (t-sum)-y  t y .
		SWAPXY
		CSTO(PRODSUM)
	//LBL(1)
		DSE(I)
		BACK(15)	//GTO(SIGMA_LOOP)
		GTO(7)

// Product code
	LBL(ENTRY_PI)
		ENTRY
		DECM
		TST_SPECIAL
			GTO(9)
		STO(I)
		STO(SAV_I)
		TRUNC			// First function call is separate
		FILL			// to avoid a multiply
		GSBUSER
		TST_SPECIAL
			GTO(8)
		STO(PRODSUM)
		SKIP(7)	//GTO(1)
	//LBL(PI_LOOP)
		RCL(I)
		TRUNC
		FILL
		GSBUSER
		TST_SPECIAL
			GTO(8)
		STO_MU(PRODSUM)
	//LBL(1)
		DSE(I)
		BACK(8)	//GTO(PI_LOOP)
	LBL(7)
		RCL(SAV_I)
		STO(st(L))
		ZERO
		FILL
		RCL(PRODSUM)
		EXIT

	LBL(8)				// NaN result
		RCL(SAV_I)
	LBL(9)				// Exit in error
		STO(st(L))
		ZERO
		FILL
		NAN
		EXIT

#undef I
#undef PRODSUM
#undef C
#undef SAV_I

/**************************************************************************/
/* Numerical differentiation */

#define DERIVATIVE_REG_BASE	0
#define E3	(0 + DERIVATIVE_REG_BASE)
#define E2	(1 + DERIVATIVE_REG_BASE)
#define E1	(2 + DERIVATIVE_REG_BASE)
#define X	(3 + DERIVATIVE_REG_BASE)
#define H	(4 + DERIVATIVE_REG_BASE)

#define F_SECOND	0

	LBL(ENTRY_2DERIV)
		ENTRY
		DECM
		TST_SPECIAL
			GTO(9)
		SF(F_SECOND)
		GSB(5)

		DIG(1)
		GSB(6)			// f(x+h) + f(x-h)
			GTO(9)
		DIG(1)
		DIG(6)
		TIMES
		STO(E2)			// order four estimate
		DIG(4)
		DIG(2)
		EEX
		DIG(3)
		RCL_MU(E3)
		STO(E1)			// order ten estimate

		DIG(0)
		GSB(6)			// f(x)
			GTO(9)
		DIG(3)
		DIG(0)
		TIMES
		STO_MI(E2)
		DIG(7)
		DIG(3)
		DIG(7)
		DIG(6)
		DIG(6)
		RCL_MU(E3)
		STO_MI(E1)

		DIG(2)
		GSB(6)			// f(x+2h) + f(x-2h)
			GTO(9)
		STO_MI(E2)
		DIG(6)
		EEX
		DIG(3)
		TIMES
		STO_MI(E1)

		DIG(3)			// f(x+3h) + f(x-3h)
		GSB(6)
			GTO(3)
		EEX
		DIG(3)
		TIMES
		STO_PL(E1)

		DIG(4)			// f(x+4h) + f(x-4h)
		GSB(6)
			GTO(3)
		DIG(1)
		DIG(2)
		DIG(5)
		TIMES
		STO_MI(E1)

		DIG(5)			// f(x+5h) + f(x-5h)
		GSB(6)
			GTO(3)
		DIG(8)
		TIMES
		RCL_PL(E1)
		DIG(2)
		DIG(5)
		DIG(2)
		SL10(2)			// * 100 = 25200
		RCL_MU(H)
		RCL_MU(H)
		DIVISION
		GTO(4)

	LBL(ENTRY_DERIV)
		ENTRY
		DECM
		TST_SPECIAL
			GTO(9)
		CF(F_SECOND)
		GSB(5)

		DIG(1)
		GSB(6)
			GTO(9)
		STO(E2)			// f(x+h) - f(x-h)

		DIG(2)
		GSB(6)
			GTO(9)
		STO(E1)			// f(x+2h) - f(x-2h)

		// At this point we can do a four point estimate if something goes awry
		DIG(3)
		GSB(6)			// f(x+3h)-f(x-3h)
			GTO(8)

		// At this point we can do the six point estimate - calculate it now
		DIG(4)
		DIG(5)
		RCL_MU(E2)
		DIG(9)
		RCL_MU(E1)
		MINUS
		PLUS
		DIG(6)
		DIG(0)
		RCL_MU(H)
		DIVISION
		SWAP(E2)		// Six point stimate in E2 & start ten point estimate
		DIG(2)
		DIG(1)
		SL10(2)			// * 100 = 2100
		TIMES
		DIG(6)
		SL10(2)			// * 100 = 600
		RCL_MU(E1)
		MINUS
		DIG(1)
		DIG(5)
		DIG(0)
		RCL_MU(E3)
		PLUS
		STO(E1)			// Ten point estimate to end up in E1

		DIG(4)
		GSB(6)			// f(x+4h) - f(x-4h)
			GTO(7)
		DIG(2)
		DIG(5)
		TIMES
		STO_MI(E1)

		DIG(5)
		GSB(6)
			GTO(7)
		RCL_PL(E3)
		RCL_PL(E1)
		DIG(2)
		DIG(5)
		DIG(2)
		DIG(0)
		RCL_MU(H)
		DIVISION
		GTO(4)

	LBL(6)				// Eval f(X + k h) k on stack
		STO(E3)
		RCL_MU(H)
		RCL_PL(X)
		FILL
		GSBUSER			// f(x + k h)
		TST_SPECIAL
			RTN
		SWAP(E3)
		TST0(eq)
			SKIP(10)
		CHS
		RCL_MU(H)
		RCL_PL(X)
		FILL
		GSBUSER			// f(x - k h)
		TST_SPECIAL
			RTN
		FSp(F_SECOND)
			CHS
		STO_MI(E3)
		RCL(E3)			// f(x + k h) - f(x - k h)
		RTNp1

	LBL(5)				// Determine our default h
		STO(X)
		FILL
		DXEQ('\243', 'X', 0)	// User supplied dX
		STO(H)
		RTN

	DLBL('\243', 'X', 0)
		DOT
		DIG(1)			// default h = 0.1
		RTN

	LBL(8)				// Four point estimate from registers
		DIG(8)
		RCL_MU(E2)		// 8 * (f(x+h) - f(x-h))
		RCL_MI(E1)		// 8 * (f(x+h) - f(x-h)) - f(x+2h) + f(x-2h)
		DIG(1)
		DIG(2)
		RCL_MU(H)
		DIVISION
		GTO(4)
	LBL(9)				// Bad input, can't even do a four point estimate
		NAN
		GTO(4)
	LBL(3)				// four point estimate of the second
		DIG(1)			// derivative
		DIG(2)
		RCL_MU(H)
		RCL_MU(H)
		RECIP
		STO_MU(E2)
	LBL(7)				// Six point estimate return
		RCL(E2)
	LBL(4)				// Common return with value on stack
		STO(st(L))
		ZERO
		FILL
		RCL(X)
		SWAP(st(L))
		EXIT
#undef H
#undef X
#undef E1
#undef E2
#undef E3
#undef F_SECOND

/**************************************************************************/
/* A quadratic equation solver.
 * Based on code by Franz.
 */
	LBL(ENTRY_QUAD)
/* Registers:
 *	00	c
 *	01	b^2 - 4ac
 *	02	2a
 *	03	T
 *	04	I
 */
		GSB(XROM_CHECK)
		STOSTK(00)
		RCL(st(I))
		STO(4)
		ROLLD

		RCL_PL(st(X))		// 2c b a .
		SWAP(st(Z))		// a b 2c .
		TST0(eq)
			ERROR(ERR_INVALID)
		RCL_PL(st(X))		// 2a b 2c .
		STO(2)
		RCL(st(Y))		// b 2a b 2c
		CTIMES			// b^2-4ac 2b(a+c) b 2c
		STO(1)
		TST0(lt)
			SKIP(11)	// complex result
		SQRT			// sqrt 2b(a+c) b 2c
		SWAPXY
		ROLLD			// sqrt b 2c .
		TST0(gt)
			CCONJ
		MINUS			// sqrt-b 2c 2c 2c
		TST0(ne)
			STO_DV(st(Y))
		RCL_DV(2)
		GSB(1)
		EXIT

		// complex result
		ABS
		SQRT			// sqrt ? b 2c
		RCL_DV(2)		// sqrt/2a ? b 2c
		RCL(st(Z))		// b sqrt/2a ? b
		RCL_DV(2)		// b/2a sqrt/2a ? b
		CHS
		GSB(1)
		TST_TOP
			SKIP(1)
		EXITp1

		CLALPHA
		alpha1('C')
		VIEWREG(st(X))
		EXIT

	LBL(1)				// Fix the stack up
		RCL(0)
		STO(st(L))		// Push c into Last X
		RCL(4)
		STO(st(I))		// Restore I
		CSWAPXY
		RCL(4)			// Restore T
		RCL(1)			// Preserve b^2 - 4ac
		CSWAPXY
		RTN

/**************************************************************************/

	LBL(XROM_CHECK)			// Set xrom flag and error if it
		FCpF(F_XROM)		// was set already, return if not.
			RTN
		ERROR(ERR_XROM_NEST)

	LBL(XROM_EXIT)			// Clear xrom flag and return
		CF(F_XROM)
		RTN

	LBL(XROM_EXITp1)		// Clear xrom falg and return with skip
		CF(F_XROM)
		RTNp1



/**************************************************************************/
/* Very minimal routine to return the next prime in sequence
 */
	LBL(ENTRY_NEXTPRIME)
		FLOOR
		TST1(le)
			SKIP(9)
		TST_PRIME
			INC(st(X))
		TST_EVEN
			INC(st(X))
		TST_PRIME
			RTN
		INC(st(X))
		BACK(4)
		CLX
		DIG(2)
		RTN

/**************************************************************************/

	DLBL('W', 'H', 'O')
		CLALPHA
		GSB(1)
		alpha2('b', 'y')
		GSB(0)
		alpha5('P', 'a', 'u', 'l', 'i')
		GSB(0)
		alpha6('W', 'a', 'l', 't', 'e', 'r')
		GSB(0)
		alpha6('M', 'a', 'r', 'c', 'u', 's')
		GSB(0)
		alpha4('N', 'e', 'i', 'l')
		GSB(0)
		alpha6('&', ' ', 'm', 'o', 'r', 'e')
		GSB(0)
		GSB(1)
		//VIEW(st(X))
		VERS
		RTN
	LBL(1)
		alpha6('W', 'P', ' ', '3', '4', 'S')
	LBL(0)
		AVIEW
		PAUSE(8)
		CLALPHA
	RTN
};

const unsigned short int xrom_size = sizeof(xrom) / sizeof(const s_opcode);


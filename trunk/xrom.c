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
#define ALPHA(c)	(OP_RARG | (RARG_ALPHA << RARG_OPSHFT) | ((c) & 0xff)),
#define xRARG(op, n)	RARG((RARG_ ## op), ((n) & RARG_MASK)),
#define xCONST(n)	CONST(OP_ ## n),
#define iCONST(n)	CONST_INT(OP_ ## n),
#define iCONSTIND(n)	RARG_IND | xRARG(CONST_INT, n)
#define NILADIC(n)	(OP_NIL | (OP_ ## n)),
#define MONADIC(n)	(OP_MON | (OP_ ## n)),
#define DYADIC(n)	(OP_DYA | (OP_ ## n)),
#define SPECIAL(n)	(OP_SPEC | OP_ ## n),
#define GSBUSER		(OP_NIL | OP_GSBuser),

// Specials
#define ENTER		SPECIAL(ENTER)
#define CLX		SPECIAL(CLX)
#define EEX		SPECIAL(EEX)
#define CHS		SPECIAL(CHS)
#define DOT		SPECIAL(DOT)
#define DIG(n)		(OP_SPEC | (OP_0 + (n))),
#define TST0(t)		(OP_SPEC | (OP_X ## t ## 0)),
#define TST1(t)		(OP_SPEC | (OP_X ## t ## 1)),

// Commands that take an argument
#define ERROR(n)	xRARG(ERROR, n)
#define WARNING(n)	xRARG(WARNING, n)
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

// Define the constants we know about
#define PI		xCONST(PI)
#define ZERO		iCONST(ZERO)
#define ONE		iCONST(ONE)
#define E		xCONST(CNSTE)
#define NAN		xCONST(NAN)
#define INFINITY	xCONST(INF)

// And the useful niladic routines
#define LASTX		NILADIC(LASTX)
#define RTN		NILADIC(RTN)
#define RTNp1		NILADIC(RTNp1)
#define AVIEW		NILADIC(VIEWALPHA)
#define DECM		NILADIC(FLOAT)
#define CLSTK		NILADIC(CLSTK)
#define STOP		NILADIC(RS)
#define PAUSE		NILADIC(PAUSE)
#define SOLVE_STEP	NILADIC(SOLVESTEP)
#define TST_INFINITE	NILADIC(XisInf)
#define TST_NaN		NILADIC(XisNaN)
#define TST_SPECIAL	NILADIC(XisSpecial)
#define init_solve	NILADIC(inisolve)

// Mondaic functions
#define RECIP		MONADIC(RECIP)
#define ABS		MONADIC(ABS)
#define SIGN		MONADIC(SIGN)
#define RND		MONADIC(RND)
#define POW10		MONADIC(POW10)
#define FRAC		MONADIC(FRAC)
#define TRUNC		MONADIC(TRUNC)
#define SQUARE		MONADIC(SQR)
#define solve_step	MONADIC(stpsolve)

// Dyadic functions
#define PLUS		DYADIC(ADD)
#define MINUS		DYADIC(SUB)
#define TIMES		DYADIC(MUL)
#define DIVISION	DYADIC(DIV)
#define MOD		DYADIC(MOD)

// Stack manipulation shortcuts
#define SWAPXY		NILADIC(SWAP)
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

/* Labels - global */
#define XROM_CHECK		40
#define XROM_EXIT		41
#define XROM_EXITp1		42
//#define SLV_LOOP		51
//#define SIGMA_LOOP		52
//#define PI_LOOP			53
//#define GKLOOP			54
//#define KLOOP			55

/* Flags - global */
#define F_XROM			0

/* Now the xrom table itself.
 *
 * Global labels are from 10 and up.
 * Local labels are 0 through 19 and can only be used for forward
 * branches since they'll likely be redefined elsewhere.
 *
 * Banked registers are 0 - 4
 * Banked flags are 0 - 7
 */
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

/* Flag use */
#define S		1
#define T		2


	LBL(ENTRY_SOLVE)
		ENTRY
		DECM
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
		init_solve

//	LBL(SLV_LOOP)
		RCL(C)
		FILL
		GSBUSER
		FILL
		TST0(apx)
			GTO(1)
		solve_step
		TST0(ne)		// Check for failure to complete
			GTO(5)
		RCL(B)
		TST(APX, A)
			GTO(6)
//		GTO(SLV_LOOP)
		BACK(13)
	LBL(6)				// Limits are narrow -- either solved or pole
		RCL(st(Z))
		FILL
		TST0(apx)
			GTO(7)
		GSB(8)
		EXIT
	LBL(7)
		GSB(8)
		EXITp1
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
			GTO(5)
		RCL(st(T))
		STO(st(L))
		ZERO
		SWAPXY
		RCL(A)
		RCL(C)
		EXITp1
	LBL(5)
		RCL(st(T))
		STO(st(L))
		ZERO
		SWAPXY
		RCL(B)
		RCL(C)
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
		GTO(0)
	LBL(2)
		RCL(FA)
		RCL(A)
	LBL(0)
		RCL(st(X))
		ZERO
		STO(st(L))
		ROLLU
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
			BACK(12)	//GTO(GKLOOP)
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
			BACK(12)	//GTO(KLOOP)
		RCL(HALF_LEN)
		STO_MU(KRONROD)


		/* Set up the stack for our output */
		GSB(3)
		RCL(KRONROD)
		RCL(GAUSS)
		MINUS			// err, l, u, ?, G
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
		RCL(I)			// I, f, ?, ?
		ONE			// 1, I, f, ?
		PLUS			// I+1, f, ?, ?
		iCONSTIND(st(X))	// ki, i+1, f, ?
		STO(st(L))
		RCL(st(Z))
		FILL
		LASTX
		TIMES
		STO_PL(KRONROD)
		RTN
	LBL(7)				// Gauus Kronrod accumulation, f(xi) in X
		RCL(I)
		ONE
		PLUS
		RCL(st(Y))
		RCL(st(X))
		iCONSTIND(st(Z))	// gi, f, f, I+1
		TIMES			// gi*f, f, I+1, I+1
		STO_PL(GAUSS)
		ONE			// 1, gi*f, f, I+1
		STO_PL(st(T))		// 1, gi*f, f, I+2
		DROP
		DROP
		iCONSTIND(st(T))	// ki, f, I+2, I+2
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
		SKIP(15)	//GTO(1)
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
		STO(C)
		SWAPXY
		STO(PRODSUM)
	//LBL(1)
		DSE(I)
		BACK(17)	//GTO(SIGMA_LOOP)
		ZERO			// Clean up and exit
		RCL(SAV_I)
		RCL(PRODSUM)
		SWAPXY
		STO(st(L))
		ENTER
		DROP
		DROP
		EXIT

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
		BACK(9)	//GTO(PI_LOOP)
		ZERO
		RCL(SAV_I)
		RCL(PRODSUM)
		SWAPXY
		STO(st(L))
		ENTER
		DROP
		DROP
		EXIT

	LBL(8)
		TST_NaN
			GTO(0)
		RCL(SAV_I)		// Infinite result
		DROP
		STO(st(L))
		DROP
		ZERO
		ZERO
		ZERO
		ROLLU
		EXIT
	LBL(0)				// NaN result
		RCL(SAV_I)
		STO(st(L))
		ZERO
		ZERO
		ZERO
		NAN
		EXIT
	LBL(9)				// Exit in error
		STO(st(L))
		DROP
		NAN
		EXIT

#undef I
#undef PRODSUM
#undef C
#undef SAV_I

/**************************************************************************/
/* Modular arithmetic routines */
#ifdef INCLUDE_MODULAR
	LBL(ENTRY_MADD)
		ENTRY
		MINUS
		STO_PL(st(Y))
		SWAP(st(L))
		MOD
		EXIT

	LBL(ENTRY_MSUB)
		ENTRY
		ROLLD
		MINUS
		ROLLU
		MOD
		EXIT

	LBL(ENTRY_MMUL)
#define R_SAV	0
#define R_N	1
#define R_M	2
		ENTRY
		ROLLU
		STO(R_SAV)
		ROLLD
		STO(R_N)
		ROLLD
		STO(st(Z))
		EEX
		DIG(8)
		MOD
		STO_MI(st(Z))
		STO(R_M)
		SWAPXY
		STO(st(Y))
		LASTX
		MOD
		STO_MI(st(Y))
		STO_MU(st(T))
		SWAP(R_M)
		STO_MU(R_M)
		SWAPXY
		STO_MU(st(Z))
		TIMES
		RCL(R_N)
		MOD
		SWAPXY
		LASTX
		CHS
		MOD
		PLUS
		RCL(R_N)
		MOD
		SWAPXY
		LASTX
		CHS
		MOD
		PLUS
		RCL(R_N)
		MOD
		RCL(R_M)
		LASTX
		CHS
		MOD
		PLUS
		RCL(R_N)
		MOD
		RCL(R_SAV)
		ENTER
		ENTER
		ROLLU
		EXIT
#undef R_M
#undef R_N

	LBL(ENTRY_MSQ)
		ENTRY
		ROLLD
		ROLLD
		CSTO(R_SAV)
		ROLLD
		ROLLD
		RCL(st(Y))
		EEX
		DIG(8)
		MOD
		STO_MI(st(Z))
		STO(st(T))
		SQUARE
		SWAPXY
		MOD
		SWAPXY
		STO_MU(st(Z))
		STO_MU(st(X))
		LASTX
		CHS
		MOD
		STO_PL(st(Y))
		SWAP(st(L))
		CHS
		MOD
		SWAPXY
		LASTX
		MOD
		RCL(st(X))
		LASTX
		MINUS
		STO_PL(st(Y))
		SWAP(st(L))
		CHS
		MOD
		STO_PL(st(Y))
		SWAP(st(L))
		CHS
		MOD
		CRCL(R_SAV)
		RCL(st(Y))
		SWAPXY
		ROLLU
		EXIT

#undef SAV
#endif

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
};

const unsigned short int xrom_size = sizeof(xrom) / sizeof(const s_opcode);


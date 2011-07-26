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

#if COMMANDS_PASS != 2
#include "xeq.h"
#include "decn.h"
#include "complex.h"
#include "stats.h"
#include "int.h"
#include "date.h"
#include "display.h"
#include "consts.h"
#include "alpha.h"
#include "lcd.h"
#include "storage.h"
#include "serial.h"
#endif

#ifdef SHORT_POINTERS
#ifndef COMMANDS_PASS
#define COMMANDS_PASS 1
#else
/*
 *  This is pass 2 of the compile.
 *
 *  Create a dummy segment and store the full blown tables there
 */
#define CMDTAB __attribute__((section(".cmdtab"),used))

/*
 *  Help the post-processor to find the data in the flash image
 */
extern const struct monfunc_cmdtab monfuncs_ct[];
extern const struct dyfunc_cmdtab dyfuncs_ct[];
extern const struct trifunc_cmdtab trifuncs_ct[];
extern const struct niladic_cmdtab niladics_ct[];
extern const struct argcmd_cmdtab argcmds_ct[];
extern const struct multicmd_cmdtab multicmds_ct[];

CMDTAB __attribute__((externally_visible))
const struct _command_info command_info = {
	monfuncs,  monfuncs_ct,
	dyfuncs,   dyfuncs_ct,
	trifuncs,  trifuncs_ct,
	niladics,  niladics_ct,
	argcmds,   argcmds_ct,
	multicmds, multicmds_ct,
};

#endif
#endif

/* Define our table of monadic functions.
 * These must be in the same order as the monadic function enum but we'll
 * validate this only if debugging is enabled.
 */
#ifdef COMPILE_CATALOGUES
#define PTR	((void *)1)
#define FUNC(name, d, c, i, fn) { PTR, PTR, PTR, fn },
#elif DEBUG
#define FUNC(name, d, c, i, fn) { name, d, c, i, fn },
#elif COMMANDS_PASS == 1
#define FUNC(name, d, c, i, fn) { 0xaa55, 0x55aa, 0xa55a, fn },
#else
#define FUNC(name, d, c, i, fn) { d, c, i, fn },
#endif

#if COMMANDS_PASS == 2
CMDTAB const struct monfunc_cmdtab monfuncs_ct[ NUM_MONADIC ] = {
#else
const struct monfunc monfuncs[ NUM_MONADIC ] = {
#endif
	FUNC(OP_FRAC,	&decNumberFrac,		&cmplxFrac,	&intFP,		"FP")
	FUNC(OP_FLOOR,	&decNumberFloor,	NULL,		&intIP,		"FLOOR")
	FUNC(OP_CEIL,	&decNumberCeil,		NULL,		&intIP,		"CEIL")
	FUNC(OP_ROUND,	&decNumberRound,	NULL,		&intIP,		"ROUNDI")
	FUNC(OP_TRUNC,	&decNumberTrunc,	&cmplxTrunc,	&intIP,		"IP")
	FUNC(OP_ABS,	&dn_abs,		&cmplxAbs,	&intAbs,	"ABS")
	FUNC(OP_RND,	&decNumberRnd,		&cmplxRnd,	&intIP,		"ROUND")
	FUNC(OP_SIGN,	&decNumberSign,		&cmplxSign,	&intSign,	"SIGN")
	FUNC(OP_LN,	&dn_ln,			&cmplxLn,	NULL,		"LN")
	FUNC(OP_EXP,	&dn_exp,		&cmplxExp,	NULL,		"e\234")
	FUNC(OP_SQRT,	&dn_sqrt,		&cmplxSqrt,	&intSqrt,	"\003")
	FUNC(OP_RECIP,	&decNumberRecip,	&cmplxRecip,	NULL,		"1/x")
	FUNC(OP__1POW,	&decNumberPow_1,	&cmplx_1x,	&int_1pow,	"(-1)\234")
	FUNC(OP_LOG,	&dn_log10,		&cmplxLog,	&intLog10,	"LOG\271\270")
	FUNC(OP_LG2,	&dn_log2,		&cmplxLog2,	&intLog2,	"LOG\272")
	FUNC(OP_2POWX,	&decNumberPow2,		&cmplx2x,	&int2pow,	"2\234")
	FUNC(OP_10POWX,	&decNumberPow10,	&cmplx10x,	&int10pow,	"10\234")
	FUNC(OP_LN1P,	&decNumberLn1p,		&cmplxLn1p,	NULL,		"LN1+x")
	FUNC(OP_EXPM1,	&decNumberExpm1,	&cmplxExpm1,	NULL,		"e\234-1")
	FUNC(OP_LAMW,	&decNumberLamW,		&cmplxlamW,	NULL,		"W")
	FUNC(OP_INVW,	&decNumberInvW,		&cmplxInvW,	NULL,		"W\235")
	FUNC(OP_SQR,	&decNumberSquare,	&cmplxSqr,	&intSqr,	"x\232")
	FUNC(OP_CUBE,	&decNumberCube,		&cmplxCube,	&intCube,	"CUBE")
	FUNC(OP_CUBERT,	&decNumberCubeRoot,	&cmplxCubeRoot,	&intCubeRoot,	"CUBERT")
	FUNC(OP_FIB,	&decNumberFib,		&cmplxFib,	&intFib,	"FIB")
	FUNC(OP_2DEG,	&decNumber2Deg,		NULL,		NULL,		"\015DEG")
	FUNC(OP_2RAD,	&decNumber2Rad,		NULL,		NULL,		"\015RAD")
	FUNC(OP_2GRAD,	&decNumber2Grad,	NULL,		NULL,		"\015GRAD")
	FUNC(OP_DEG2,	&decNumberDeg2,		NULL,		NULL,		"DEG\015")
	FUNC(OP_RAD2,	&decNumberRad2,		NULL,		NULL,		"RAD\015")
	FUNC(OP_GRAD2,	&decNumberGrad2,	NULL,		NULL,		"GRAD\015")
	FUNC(OP_SIN,	&decNumberSin,		&cmplxSin,	NULL,		"SIN")
	FUNC(OP_COS,	&decNumberCos,		&cmplxCos,	NULL,		"COS")
	FUNC(OP_TAN,	&decNumberTan,		&cmplxTan,	NULL,		"TAN")
	FUNC(OP_ASIN,	&decNumberArcSin,	&cmplxAsin,	NULL,		"ASIN")
	FUNC(OP_ACOS,	&decNumberArcCos,	&cmplxAcos,	NULL,		"ACOS")
	FUNC(OP_ATAN,	&decNumberArcTan,	&cmplxAtan,	NULL,		"ATAN")
	FUNC(OP_SINC,	&decNumberSinc,		&cmplxSinc,	NULL,		"SINC")
	FUNC(OP_SINH,	&decNumberSinh,		&cmplxSinh,	NULL,		"SINH")
	FUNC(OP_COSH,	&decNumberCosh,		&cmplxCosh,	NULL,		"COSH")
	FUNC(OP_TANH,	&decNumberTanh,		&cmplxTanh,	NULL,		"TANH")
	FUNC(OP_ASINH,	&decNumberArcSinh,	&cmplxAsinh,	NULL,		"ASINH")
	FUNC(OP_ACOSH,	&decNumberArcCosh,	&cmplxAcosh,	NULL,		"ACOSH")
	FUNC(OP_ATANH,	&decNumberArcTanh,	&cmplxAtanh,	NULL,		"ATANH")
	FUNC(OP_FACT,	&decNumberFactorial,	&cmplxFactorial,NULL,		"x!")
	FUNC(OP_GAMMA,	&decNumberGamma,	&cmplxGamma,	NULL,		"\202")
	FUNC(OP_LNGAMMA,&decNumberLnGamma,	&cmplxLnGamma,	NULL,		"LN\202")
#ifdef INCLUDE_DIGAMMA
	FUNC(OP_PSI,	&decNumberPsi,		&cmplxPsi,	NULL,		"\226")
#endif
#ifdef INCLUDE_DBLFACT
	FUNC(OP_DBLFACT,&decNumberDblFactorial,	&cmplxDblFactorial,NULL,	"x!!")
#endif
#ifdef INCLUDE_SUBFACT
	FUNC(OP_SUBFACT,&decNumberSubFactorial,	NULL,		NULL,	"!n")
#endif
	FUNC(OP_DEG2RAD,&decNumberD2R,		NULL,		NULL,		"\005\015rad")
	FUNC(OP_RAD2DEG,&decNumberR2D,		NULL,		NULL,		"rad\015\005")
	FUNC(OP_DEG2GRD,&decNumberD2G,		NULL,		NULL,		"\005\015G")
	FUNC(OP_GRD2DEG,&decNumberG2D,		NULL,		NULL,		"G\015\005")
	FUNC(OP_RAD2GRD,&decNumberR2G,		NULL,		NULL,		"rad\015G")
	FUNC(OP_GRD2RAD,&decNumberG2R,		NULL,		NULL,		"G\015rad")
	FUNC(OP_CCHS,	&dn_minus,		&cmplxMinus,	&intChs,	"+/-")
	FUNC(OP_CCONJ,	NULL,			&cmplxConj,	NULL,		"CONJ")
	FUNC(OP_ERF,	&decNumberERF,		NULL,		NULL,		"erf")
	FUNC(OP_ERFC,	&decNumberERFC,		NULL,		NULL,		"erfc")
	FUNC(OP_pdf_Q,	&pdf_Q,			NULL,		NULL,		"\264(x)")
	FUNC(OP_cdf_Q,	&cdf_Q,			NULL,		NULL,		"\224(x)")
	FUNC(OP_qf_Q,	&qf_Q,			NULL,		NULL,		"\224\235(p)")
	FUNC(OP_pdf_chi2, &pdf_chi2,		NULL,		NULL,		"\265\232\276")
	FUNC(OP_cdf_chi2, &cdf_chi2,		NULL,		NULL,		"\265\232")
	FUNC(OP_qf_chi2,  &qf_chi2,		NULL,		NULL,		"\265\232INV")
	FUNC(OP_pdf_T,	&pdf_T,			NULL,		NULL,		"t\276(x)")
	FUNC(OP_cdf_T,	&cdf_T,			NULL,		NULL,		"t(x)")
	FUNC(OP_qf_T,	&qf_T,			NULL,		NULL,		"t\235(p)")
	FUNC(OP_pdf_F,	&pdf_F,			NULL,		NULL,		"F\276(x)")
	FUNC(OP_cdf_F,	&cdf_F,			NULL,		NULL,		"F(x)")
	FUNC(OP_qf_F,	&qf_F,			NULL,		NULL,		"F\235(p)")
	FUNC(OP_pdf_WB,	&pdf_WB,		NULL,		NULL,		"Weibl\276")
	FUNC(OP_cdf_WB,	&cdf_WB,		NULL,		NULL,		"Weibl")
	FUNC(OP_qf_WB,	&qf_WB,			NULL,		NULL,		"Weibl\235")
	FUNC(OP_pdf_EXP,&pdf_EXP,		NULL,		NULL,		"Expon\276")
	FUNC(OP_cdf_EXP,&cdf_EXP,		NULL,		NULL,		"Expon")
	FUNC(OP_qf_EXP,	&qf_EXP,		NULL,		NULL,		"Expon\235")
	FUNC(OP_pdf_B,	&pdf_B,			NULL,		NULL,		"Binom\276")
	FUNC(OP_cdf_B,	&cdf_B,			NULL,		NULL,		"Binom")
	FUNC(OP_qf_B,	&qf_B,			NULL,		NULL,		"Binom\235")
	FUNC(OP_pdf_P,	&pdf_P,			NULL,		NULL,		"Poiss\276")
	FUNC(OP_cdf_P,	&cdf_P,			NULL,		NULL,		"Poiss")
	FUNC(OP_qf_P,	&qf_P,			NULL,		NULL,		"Poiss\235")
	FUNC(OP_pdf_G,	&pdf_G,			NULL,		NULL,		"Geom\276")
	FUNC(OP_cdf_G,	&cdf_G,			NULL,		NULL,		"Geom")
	FUNC(OP_qf_G,	&qf_G,			NULL,		NULL,		"Geom\235")
	FUNC(OP_pdf_N,	&pdf_normal,		NULL,		NULL,		"Norml\276")
	FUNC(OP_cdf_N,	&cdf_normal,		NULL,		NULL,		"Norml")
	FUNC(OP_qf_N,	&qf_normal,		NULL,		NULL,		"Norml\235")
	FUNC(OP_pdf_LN,	&pdf_lognormal,		NULL,		NULL,		"LgNrm\276")
	FUNC(OP_cdf_LN,	&cdf_lognormal,		NULL,		NULL,		"LgNrm")
	FUNC(OP_qf_LN,	&qf_lognormal,		NULL,		NULL,		"LgNrm\235")
	FUNC(OP_pdf_LG,	&pdf_logistic,		NULL,		NULL,		"Logis\276")
	FUNC(OP_cdf_LG,	&cdf_logistic,		NULL,		NULL,		"Logis")
	FUNC(OP_qf_LG,	&qf_logistic,		NULL,		NULL,		"Logis\235")
	FUNC(OP_pdf_C,	&pdf_cauchy,		NULL,		NULL,		"Cauch\276")
	FUNC(OP_cdf_C,	&cdf_cauchy,		NULL,		NULL,		"Cauch")
	FUNC(OP_qf_C,	&qf_cauchy,		NULL,		NULL,		"Cauch\235")
	FUNC(OP_xhat,	&stats_xhat,		NULL,		NULL,		"\031")
	FUNC(OP_yhat,	&stats_yhat,		NULL,		NULL,		"\032")
	FUNC(OP_sigper,	&stats_sigper,		NULL,		NULL,		"%\221")
	FUNC(OP_PERCNT,	&decNumberPercent,	NULL,		NULL,		"%")
	FUNC(OP_PERCHG,	&decNumberPerchg,	NULL,		NULL,		"\203%")
	FUNC(OP_PERTOT,	&decNumberPertot,	NULL,		NULL,		"%T")
	FUNC(OP_HMS2,	&decNumberHMS2HR,	NULL,		NULL,		"\015HR")
	FUNC(OP_2HMS,	&decNumberHR2HMS,	NULL,		NULL,		"\015H.MS")
	FUNC(OP_NOT,	&decNumberNot,		NULL,		&intNot,	"NOT")
	FUNC(OP_BITCNT,	NULL,			NULL,		&intNumBits,	"nBITS")
	FUNC(OP_MIRROR,	NULL,			NULL,		&intMirror,	"MIRROR")
	FUNC(OP_DOWK,	&dateDayOfWeek,		NULL,		NULL,		"WDAY")
	FUNC(OP_D2J,	&dateToJ,		NULL,		NULL,		"D\015J")
	FUNC(OP_J2D,	&dateFromJ,		NULL,		NULL,		"J\015D")
	FUNC(OP_DEGC_F,	&convC2F,		NULL,		NULL,		"\005C\015\005F")
	FUNC(OP_DEGF_C,	&convF2C,		NULL,		NULL,		"\005F\015\005C")
	FUNC(OP_DB_AR,	&convDB2AR,		NULL,		NULL,		"dB\015ar.")
	FUNC(OP_AR_DB,	&convAR2DB,		NULL,		NULL,		"ar.\015dB")
	FUNC(OP_DB_PR,	&convDB2PR,		NULL,		NULL,		"dB\015pr.")
	FUNC(OP_PR_DB,	&convPR2DB,		NULL,		NULL,		"pr.\015dB")
#ifdef INCLUDE_ZETA
#ifdef INCLUDE_COMPLEX_ZETA
	FUNC(OP_ZETA,	&decNumberZeta,		&cmplxZeta,	NULL,		"\245")
#else
	FUNC(OP_ZETA,	&decNumberZeta,		NULL,		NULL,		"\245")
#endif
#endif
#ifdef INCLUDE_BERNOULLI
	FUNC(OP_Bn,	&decNumberBernBn,	NULL,		NULL,		"B\275")
	FUNC(OP_BnS,	&decNumberBernBnS,	NULL,		NULL,		"B\275*")
#endif
#ifdef INCLUDE_EASTER
	FUNC(OP_EASTER,	&dateEaster,		NULL,		NULL,		"EASTER")
#endif
#ifdef INCLUDE_FACTOR
	FUNC(OP_FACTOR,	&decFactor,		NULL,		&intFactor,	"FACTOR")
#endif
	FUNC(OP_DATE_YEAR, &dateYear,		NULL,		NULL,		"YEAR")
	FUNC(OP_DATE_MONTH, &dateMonth,		NULL,		NULL,		"MONTH")
	FUNC(OP_DATE_DAY, &dateDay,		NULL,		NULL,		"DAY")
#ifdef INCLUDE_USER_IO
	FUNC(OP_RECV1,	&decRecv,		NULL,		&intRecv,	"RECV1")
#endif
#undef FUNC
};
#if COMMANDS_PASS != 2
const unsigned short num_monfuncs = sizeof(monfuncs) / sizeof(struct monfunc);
#endif


/* Define our table of dyadic functions.
 * These must be in the same order as the dyadic function enum but we'll
 * validate this only if debugging is enabled.
 */
#ifdef COMPILE_CATALOGUES
#define PTR	((void *)1)
#define FUNC(name, d, c, i, fn) { PTR, PTR, PTR, fn },
#elif DEBUG
#define FUNC(name, d, c, i, fn) { name, d, c, i, fn },
#elif COMMANDS_PASS == 1
#define FUNC(name, d, c, i, fn) { 0xaa55, 0x55aa, 0xa55a, fn },
#else
#define FUNC(name, d, c, i, fn) { d, c, i, fn },
#endif

#if COMMANDS_PASS == 2
CMDTAB const struct dyfunc_cmdtab dyfuncs_ct[ NUM_DYADIC ] = {
#else
const struct dyfunc dyfuncs[ NUM_DYADIC ] = {
#endif
	FUNC(OP_POW,	&dn_power,	&cmplxPower,	&intPower,	"y\234")
	FUNC(OP_ADD,	&dn_add,		&cmplxAdd,	&intAdd,	"+")
	FUNC(OP_SUB,	&dn_subtract,		&cmplxSubtract,	&intSubtract,	"-")
	FUNC(OP_MUL,	&dn_multiply,		&cmplxMultiply,	&intMultiply,	"\034")
	FUNC(OP_DIV,	&dn_divide,		&cmplxDivide,	&intDivide,	"/")
	FUNC(OP_MOD,	&decNumberBigMod,	NULL,		&intMod,	"RMDR")
	FUNC(OP_LOGXY,	&decNumberLogxy,	&cmplxLogxy,	NULL,		"LOGx")
	FUNC(OP_MIN,	&dn_min,		NULL,		&intMin,	"MIN")
	FUNC(OP_MAX,	&dn_max,		NULL,		&intMax,	"MAX")
	FUNC(OP_ATAN2,	&decNumberArcTan2,	NULL,		NULL,		"ANGLE")
	FUNC(OP_BETA,	&decNumberBeta,		&cmplxBeta,	NULL,		"\241")
	FUNC(OP_LNBETA,	&decNumberLnBeta,	&cmplxLnBeta,	NULL,		"LN\241")
	FUNC(OP_GAMMAP,	&decNumberGammap,	NULL,		NULL,		"I\202")
#ifdef INCLUDE_ELLIPTIC
	FUNC(OP_SN,	&decNumberSN,		&cmplxSN,	NULL,		"SN")
	FUNC(OP_CN,	&decNumberCN,		&cmplxCN,	NULL,		"CN")
	FUNC(OP_DN,	&decNumberDN,		&cmplxDN,	NULL,		"DN")
#endif
#ifdef INCLUDE_BESSEL
#ifdef COMPLEX_BESSEL
	FUNC(OP_BSJN,	&decNumberBSJN,		&cmplxBSJN,	NULL,		"J\275")
	FUNC(OP_BSIN,	&decNumberBSIN,		&cmplxBSIN,	NULL,		"I\275")
	FUNC(OP_BSYN,	&decNumberBSYN,		&cmplxBSYN,	NULL,		"Y\275")
	FUNC(OP_BSKN,	&decNumberBSKN,		&cmplxBSKN,	NULL,		"K\275")
#else
	FUNC(OP_BSJN,	&decNumberBSJN,		NULL,		NULL,		"J\275")
	FUNC(OP_BSIN,	&decNumberBSIN,		NULL,		NULL,		"I\275")
	FUNC(OP_BSYN,	&decNumberBSYN,		NULL,		NULL,		"Y\275")
	FUNC(OP_BSKN,	&decNumberBSKN,		NULL,		NULL,		"K\275")
#endif
#endif
	FUNC(OP_COMB,	&decNumberComb,		&cmplxComb,	NULL,		"COMB")
	FUNC(OP_PERM,	&decNumberPerm,		&cmplxPerm,	NULL,		"PERM")
	FUNC(OP_PERMG,	&decNumberPerMargin,	NULL,		NULL,		"%+MG")
	FUNC(OP_MARGIN,	&decNumberMargin,	NULL,		NULL,		"%MG")
	FUNC(OP_PARAL,	&decNumberParallel,	&cmplxParallel,	NULL,		"||")
	FUNC(OP_AGM,	&decNumberAGM,		&cmplxAGM,	NULL,		"AGM")
	FUNC(OP_HMSADD,	&decNumberHMSAdd,	NULL,		NULL,		"H.MS+")
	FUNC(OP_HMSSUB,	&decNumberHMSSub,	NULL,		NULL,		"H.MS-")
	FUNC(OP_GCD,	&decNumberGCD,		NULL,		&intGCD,	"GCD")
	FUNC(OP_LCM,	&decNumberLCM,		NULL,		&intLCM,	"LCM")
	FUNC(OP_LAND,	&decNumberAnd,		NULL,		&intAnd,	"AND")
	FUNC(OP_LOR,	&decNumberOr,		NULL,		&intOr,		"OR")
	FUNC(OP_LXOR,	&decNumberXor,		NULL,		&intXor,	"XOR")
	FUNC(OP_LNAND,	&decNumberNand,		NULL,		&intNand,	"NAND")
	FUNC(OP_LNOR,	&decNumberNor,		NULL,		&intNor,	"NOR")
	FUNC(OP_LXNOR,	&decNumberNxor,		NULL,		&intEquiv,	"XNOR")
	FUNC(OP_DTADD,	&dateAdd,		NULL,		NULL,		"DAYS+")
	FUNC(OP_DTDIF,	&dateDelta,		NULL,		NULL,		"\203DAYS")

	FUNC(OP_LEGENDRE_PN,	&decNumberPolyPn,	NULL,	NULL,		"P\275")
	FUNC(OP_CHEBYCHEV_TN,	&decNumberPolyTn,	NULL,	NULL,		"T\275")
	FUNC(OP_CHEBYCHEV_UN,	&decNumberPolyUn,	NULL,	NULL,		"U\275")
	FUNC(OP_LAGUERRE,	&decNumberPolyLn,	NULL,	NULL,		"L\275")
	FUNC(OP_HERMITE_HE,	&decNumberPolyHEn,	NULL,	NULL,		"H\275")
	FUNC(OP_HERMITE_H,	&decNumberPolyHn,	NULL,	NULL,		"H\275\276")
#undef FUNC
};
#if COMMANDS_PASS != 2
const unsigned short num_dyfuncs = sizeof(dyfuncs) / sizeof(struct dyfunc);
#endif

/* Define our table of triadic functions.
 * These must be in the same order as the triadic function enum but we'll
 * validate this only if debugging is enabled.
 */
#ifdef COMPILE_CATALOGUES
#define PTR	((void *)1)
#define FUNC(name, d, i, fn) { PTR, PTR, fn },
#elif DEBUG
#define FUNC(name, d, i, fn) { name, d, i, fn },
#elif COMMANDS_PASS == 1
#define FUNC(name, d, i, fn) { 0xaa55, 0xa55a, fn },
#else
#define FUNC(name, d, i, fn) { d, i, fn },
#endif

#if COMMANDS_PASS == 2
CMDTAB const struct trifunc_cmdtab trifuncs_ct[ NUM_TRIADIC ] = {
#else
const struct trifunc trifuncs[ NUM_TRIADIC ] = {
#endif
	FUNC(OP_BETAI,		&betai,		NULL,		"I\241")
	FUNC(OP_DBL_DIV, 	NULL,		&intDblDiv,	"DBL/")
	FUNC(OP_DBL_MOD, 	NULL,		&intDblRmdr,	"DBLR")
#ifdef INCLUDE_MULADD
	FUNC(OP_MULADD, 	&decNumberMAdd,	&intMAdd,	"\034+")
#endif
	FUNC(OP_PERMRR,		&decNemberPerMRR, NULL,		"%MRR")
        FUNC(OP_GEN_LAGUERRE,   &decNumberPolyLnAlpha, NULL,    "L\275\240")
#undef FUNC
};

#if COMMANDS_PASS != 2
const unsigned short num_trifuncs = sizeof(trifuncs) / sizeof(struct trifunc);
#endif


#ifdef COMPILE_CATALOGUES
#define FUNC0(name, d, fn) { PTR, 0, fn },
#define FUNC1(name, d, fn) { PTR, 1, fn },
#define FUNC2(name, d, fn) { PTR, 2, fn },
#elif DEBUG
#define FUNC0(name, d, fn) { name, d, 0, fn },
#define FUNC1(name, d, fn) { name, d, 1, fn },
#define FUNC2(name, d, fn) { name, d, 2, fn },
#elif COMMANDS_PASS == 1
#define FUNC0(name, d, fn) { 0xaa55, 0, fn },
#define FUNC1(name, d, fn) { 0xaa55, 1, fn },
#define FUNC2(name, d, fn) { 0xaa55, 2, fn },
#else
#define FUNC0(name, d, fn) { d, 0, fn },
#define FUNC1(name, d, fn) { d, 1, fn },
#define FUNC2(name, d, fn) { d, 2, fn },
#endif

#if COMMANDS_PASS == 2
CMDTAB const struct niladic_cmdtab niladics_ct[ NUM_NILADIC ] = {
#else
const struct niladic niladics[ NUM_NILADIC ] = {
#endif
	FUNC0(OP_NOP,		NULL,			"NOP")
	FUNC0(OP_VERSION,	&version,		"VERS")
	FUNC0(OP_OFF,		&cmd_off,		"OFF")
	FUNC1(OP_STKSIZE,	&get_stack_size,	"SSIZE?")
	FUNC0(OP_STK4,		&set_stack_size,	"SSIZE4")
	FUNC0(OP_STK8,		&set_stack_size,	"SSIZE8")
	FUNC1(OP_INTSIZE,	&get_word_size,		"WSIZE?")
	FUNC0(OP_SWAP,		&swap,			"x\027y")
	FUNC0(OP_CSWAP,		&cpx_swap,		"\024x\027y")
	FUNC0(OP_RDOWN,		&roll_down,		"R\017")
	FUNC0(OP_RUP,		&roll_up,		"R\020")
	FUNC0(OP_CRDOWN,	&cpx_roll_down,		"\024R\017")
	FUNC0(OP_CRUP,		&cpx_roll_up,		"\024R\020")
	FUNC0(OP_CENTER,	&cpx_enter,		"\024ENTER")
	FUNC0(OP_FILL,		&fill,			"FILL")
	FUNC0(OP_CFILL,		&cpx_fill,		"\024FILL")
	FUNC0(OP_DROP,		&drop,			"DROP")
	FUNC0(OP_DROPXY,	&drop,			"\024DROP")
	FUNC1(OP_sigmaX2Y,	&sigma_val,		"\221x\232y")
	FUNC1(OP_sigmaX,	&sigma_val,		"\221x")
	FUNC1(OP_sigmaX2,	&sigma_val,		"\221x\232")
	FUNC1(OP_sigmaY,	&sigma_val,		"\221y")
	FUNC1(OP_sigmaY2,	&sigma_val,		"\221y\232")
	FUNC1(OP_sigma_XY,	&sigma_val,		"\221xy")
	FUNC1(OP_sigmaN,	&sigma_val,		"n\221")
	FUNC1(OP_sigmalnX,	&sigma_val,		"\221lnx")
	FUNC1(OP_sigmalnXlnX,	&sigma_val,		"\221ln\232x")
	FUNC1(OP_sigmalnY,	&sigma_val,		"\221lny")
	FUNC1(OP_sigmalnYlnY,	&sigma_val,		"\221ln\232y")
	FUNC1(OP_sigmalnXlnY,	&sigma_val,		"\221lnxy")
	FUNC1(OP_sigmaXlnY,	&sigma_val,		"\221xlny")
	FUNC1(OP_sigmaYlnX,	&sigma_val,		"\221ylnx")
	FUNC2(OP_statS,		&stats_deviations,	"s")
	FUNC2(OP_statSigma,	&stats_deviations,	"\261")
	FUNC2(OP_statGS,	&stats_deviations,	"\244")
	FUNC2(OP_statGSigma,	&stats_deviations,	"\244\276")
	FUNC1(OP_statWS,	&stats_wdeviations,	"sw")
	FUNC1(OP_statWSigma,	&stats_wdeviations,	"\261w")
	FUNC2(OP_statMEAN,	&stats_mean,		"\001")
	FUNC1(OP_statWMEAN,	&stats_wmean,		"\001w")
	FUNC2(OP_statGMEAN,	&stats_gmean,		"\001g")
	FUNC1(OP_statR,		&stats_correlation,	"CORR")
	FUNC2(OP_statLR,	&stats_LR,		"LR")
	FUNC2(OP_statSErr,	&stats_deviations,	"SERR")
	FUNC2(OP_statGSErr,	&stats_deviations,	"\244m")
	FUNC1(OP_statWSErr,	&stats_wdeviations,	"SERRw")
	FUNC1(OP_statCOV,	&stats_COV,		"COV")
	FUNC1(OP_statSxy,	&stats_COV,		"sxy")
	FUNC0(OP_LINF,		&stats_mode,		"LinF")
	FUNC0(OP_EXPF,		&stats_mode,		"ExpF")
	FUNC0(OP_PWRF,		&stats_mode,		"PowerF")
	FUNC0(OP_LOGF,		&stats_mode,		"LogF")
	FUNC0(OP_BEST,		&stats_mode,		"BestF")
	FUNC1(OP_RANDOM,	&stats_random,		"RAN#")
	FUNC0(OP_STORANDOM,	&stats_sto_random,	"SEED")
	FUNC0(OP_DEG,		&op_trigmode,		"DEG")
	FUNC0(OP_RAD,		&op_trigmode,		"RAD")
	FUNC0(OP_GRAD,		&op_trigmode,		"GRAD")
	FUNC0(OP_RTN,		&op_rtn,		"RTN")
	FUNC0(OP_RTNp1,		&op_rtn,		"RTN+1")
	FUNC0(OP_RS,		&op_rs,			"STOP")
	FUNC0(OP_PROMPT,	&op_prompt,		"PROMPT")
	FUNC0(OP_SIGMACLEAR,	&sigma_clear,		"CL\221")
	FUNC0(OP_CLREG,		&clrreg,		"CLREG")
	FUNC0(OP_rCLX,		&clrx,			"CLx")
	FUNC0(OP_CLSTK,		&clrstk,		"CLSTK")
	FUNC0(OP_CLALL,		NULL,			"CLALL")
	FUNC0(OP_RESET,		NULL,			"RESET")
	FUNC0(OP_CLFLAGS,	&clrflags,		"CLFLAG")
	FUNC0(OP_R2P,		&op_r2p,		"\015POL")
	FUNC0(OP_P2R,		&op_p2r,		"\015REC")
	FUNC0(OP_FRACDENOM,	&op_fracdenom,		"DENMAX")
	FUNC1(OP_2FRAC,		&op_2frac,		"DECOMP")
	FUNC0(OP_DENANY,	&op_denom,		"DENANY")
	FUNC0(OP_DENFIX,	&op_denom,		"DENFIX")
	FUNC0(OP_DENFAC,	&op_denom,		"DENFAC")
	FUNC0(OP_FRACIMPROPER,	&op_fract,		"IMPFRC")
	FUNC0(OP_FRACPROPER,	&op_fract,		"PROFRC")
	FUNC0(OP_RADDOT,	&op_radix,		"RDX.")
	FUNC0(OP_RADCOM,	&op_radix,		"RDX,")
	FUNC0(OP_THOUS_ON,	&op_thousands,		"E3ON")
	FUNC0(OP_THOUS_OFF,	&op_thousands,		"E3OFF")
	FUNC0(OP_FIXSCI,	&op_fixscieng,		"SCIOVR")
	FUNC0(OP_FIXENG,	&op_fixscieng,		"ENGOVR")
	FUNC0(OP_2COMP,		&op_intsign,		"2COMPL")
	FUNC0(OP_1COMP,		&op_intsign,		"1COMPL")
	FUNC0(OP_UNSIGNED,	&op_intsign,		"UNSIGN")
	FUNC0(OP_SIGNMANT,	&op_intsign,		"SIGNMT")
	FUNC0(OP_FLOAT,		&op_float,		"DECM")
	FUNC0(OP_HMS,		&op_float,		"H.MS")
	FUNC0(OP_FRACT,		&op_fract,		"FRACT")
	FUNC0(OP_LEAD0,		&lead0,			"LZON")
	FUNC0(OP_TRIM0,		&lead0,			"LZOFF")
	FUNC1(OP_LJ,		&int_justify,		"LJ")
	FUNC1(OP_RJ,		&int_justify,		"RJ")
	FUNC0(OP_DBL_MUL, 	&intDblMul,		"DBL\034")
	FUNC2(OP_RCLSIGMA,	&sigma_sum,		"SUM")
	FUNC0(OP_DATEDMY,	&op_datemode,		"D.MY")
	FUNC0(OP_DATEYMD,	&op_datemode,		"Y.MD")
	FUNC0(OP_DATEMDY,	&op_datemode,		"M.DY")
	FUNC0(OP_JG1752,	&op_jgchange,		"JG1752")
	FUNC0(OP_JG1582,	&op_jgchange,		"JG1582")
	FUNC0(OP_ISLEAP,	&date_isleap,		"LEAP?")
	FUNC0(OP_ALPHADAY,	&date_alphaday,		"\240DAY")
	FUNC0(OP_ALPHAMONTH,	&date_alphamonth,	"\240MONTH")
	FUNC0(OP_ALPHADATE,	&date_alphadate,	"\240DATE")
	FUNC0(OP_ALPHATIME,	&date_alphatime,	"\240TIME")
	FUNC1(OP_DATE,		&date_date,		"DATE")
	FUNC1(OP_TIME,		&date_time,		"TIME")
	FUNC0(OP_24HR,		&op_timemode,		"24H")
	FUNC0(OP_12HR,		&op_timemode,		"12H")
	FUNC0(OP_SETDATE,	&date_setdate,		"SETDAT")
	FUNC0(OP_SETTIME,	&date_settime,		"SETTIM")
	FUNC0(OP_CLRALPHA,	&clralpha,		"CL\240")
	FUNC0(OP_VIEWALPHA,	&alpha_view,		"\240VIEW")
	FUNC1(OP_ALPHALEN,	&alpha_length,		"\240LENG")
	FUNC1(OP_ALPHATOX,	&alpha_tox,		"\240\015x")
	FUNC0(OP_XTOALPHA,	&alpha_fromx,		"x\015\240")
	FUNC0(OP_ALPHAON,	&alpha_onoff,		"\240ON")
	FUNC0(OP_ALPHAOFF,	&alpha_onoff,		"\240OFF")
	FUNC0(OP_REGCOPY,	&op_regcopy,		"R-COPY")
	FUNC0(OP_REGSWAP,	&op_regswap,		"R-SWAP")
	FUNC0(OP_REGCLR,	&op_regclr,		"R-CLR")
	FUNC0(OP_REGSORT,	&op_regsort,		"R-SORT")
	FUNC0(OP_GSBuser,	&do_usergsb,		"XEQUSR")
	FUNC0(OP_XisInf,	&isInfinite,		"\237?")
	FUNC0(OP_XisNaN,	&isNan,			"NaN?")
	FUNC0(OP_XisSpecial,	&isSpecial,		"SPEC?")
	FUNC0(OP_XisPRIME,	&XisPrime,		"PRIME?")
	FUNC0(OP_XisINT,	&XisInt,		"INT?")
	FUNC0(OP_XisFRAC,	&XisFrac,		"FP?")
	FUNC0(OP_XisEVEN,	&XisEven,		"EVEN?")
	FUNC0(OP_XisODD,	&XisOdd,		"ODD?")
	FUNC0(OP_ENTRYP,	&op_entryp,		"ENTRY?")

	FUNC1(OP_TICKS,		&op_ticks,		"TICKS")
	FUNC1(OP_VOLTAGE,	&op_voltage,		"BATT")

	FUNC0(OP_SETEUR,	&op_locale,		"SETEUR")
	FUNC0(OP_SETUK,		&op_locale,		"SETUK")
	FUNC0(OP_SETUSA,	&op_locale,		"SETUSA")
	FUNC0(OP_SETIND,	&op_locale,		"SETIND")
	FUNC0(OP_SETCHN,	&op_locale,		"SETCHN")

	FUNC0(OP_QUAD,		&xrom_routines,		"SLVQ")
	FUNC0(OP_NEXTPRIME,	&xrom_routines,		"NEXTP")

	FUNC0(OP_XEQALPHA,	&op_gtoalpha,		"XEQ\240")
	FUNC0(OP_GTOALPHA,	&op_gtoalpha,		"GTO\240")
	FUNC0(OP_RLOAD,		&load_registers,	"RCFRG")
	FUNC0(OP_SLOAD,		&load_state,		"RCFST")
	FUNC0(OP_BACKUP,	&flash_backup,		"SAVE")
	FUNC0(OP_RESTORE,	&flash_restore,		"LOAD")

	FUNC1(OP_ROUNDING,	&op_roundingmode,	"RM?")
	FUNC0(OP_SLOW,		&op_setspeed,		"SLOW")
	FUNC0(OP_FAST,		&op_setspeed,		"FAST")

	FUNC0(OP_SENDP,		&send_program,		"SENDP")
	FUNC0(OP_SENDR,		&send_registers,	"SENDR")
	FUNC0(OP_SENDA,		&send_all,		"SENDA")
	FUNC0(OP_RECV,		&recv_any,		"RECV")
#ifdef INCLUDE_USER_IO
	FUNC0(OP_SEND1,		&send_byte,		"SEND1")
	FUNC0(OP_SERIAL_OPEN,	&serial_open,		"SOPEN")
	FUNC0(OP_SERIAL_CLOSE,	&serial_close,		"SCLOSE")
	FUNC0(OP_ALPHASEND,	&send_alpha,		"\240SEND")
	FUNC0(OP_ALPHARECV,	&recv_alpha,		"\240RECV")
#endif
#undef FUNC0
#undef FUNC1
#undef FUNC2
};

#if COMMANDS_PASS != 2
const unsigned short num_niladics = sizeof(niladics) / sizeof(struct niladic);
#endif


#ifdef COMPILE_CATALOGUES
#define allCMD(name, func, limit, nm, ind, stk, cpx)					\
	{ PTR, limit, ind, stk, cpx, nm },
#elif DEBUG
#define allCMD(name, func, limit, nm, ind, stk, cpx)					\
	{ name, func, limit, ind, stk, cpx, nm },
#elif COMMANDS_PASS == 1
#define allCMD(name, func, limit, nm, ind, stk, cpx)					\
	{ 0xaa55, limit, ind, stk, cpx, nm },
#else
#define allCMD(name, func, limit, nm, ind, stk, cpx)					\
	{ func, limit, ind, stk, cpx, nm },
#endif
#define CMD(n, f, lim, nm)	allCMD(n, f, lim, nm, 1, 0, 0)
#define CMDstk(n, f, lim, nm)	allCMD(n, f, lim, nm, 1, 1, 0)
#define CMDcstk(n, f, lim, nm)	allCMD(n, f, lim, nm, 1, 1, 1)
#define CMDnoI(n, f, lim, nm)	allCMD(n, f, lim, nm, 0, 0, 0)

#if COMMANDS_PASS == 2
CMDTAB const struct argcmd_cmdtab argcmds_ct[ NUM_RARG ] = {
#else
const struct argcmd argcmds[ NUM_RARG ] = {
#endif
	CMDnoI(RARG_CONST,	&cmdconst,	NUM_CONSTS,		"CNST")
	CMDnoI(RARG_CONST_CMPLX,&cmdconstcmplx,	NUM_CONSTS,		"\024CNST")
	CMD(RARG_CONST_INT,	&cmdconstint,	NUM_CONSTS_INT,		"iC")
	CMD(RARG_ERROR,		&cmderr,	MAX_ERROR,		"ERR")
	CMDstk(RARG_STO, 	&cmdsto,	NUMREG,			"STO")
	CMDstk(RARG_STO_PL, 	&cmdsto,	NUMREG,			"STO+")
	CMDstk(RARG_STO_MI, 	&cmdsto,	NUMREG,			"STO-")
	CMDstk(RARG_STO_MU, 	&cmdsto,	NUMREG,			"STO\034")
	CMDstk(RARG_STO_DV, 	&cmdsto,	NUMREG,			"STO/")
	CMDstk(RARG_STO_MIN,	&cmdsto,	NUMREG,			"STO\017")
	CMDstk(RARG_STO_MAX,	&cmdsto,	NUMREG,			"STO\020")
	CMDstk(RARG_RCL, 	&cmdrcl,	NUMREG,			"RCL")
	CMDstk(RARG_RCL_PL, 	&cmdrcl,	NUMREG,			"RCL+")
	CMDstk(RARG_RCL_MI, 	&cmdrcl,	NUMREG,			"RCL-")
	CMDstk(RARG_RCL_MU, 	&cmdrcl,	NUMREG,			"RCL\034")
	CMDstk(RARG_RCL_DV, 	&cmdrcl,	NUMREG,			"RCL/")
	CMDstk(RARG_RCL_MIN,	&cmdrcl,	NUMREG,			"RCL\017")
	CMDstk(RARG_RCL_MAX,	&cmdrcl,	NUMREG,			"RCL\020")
	CMDstk(RARG_SWAP,	&cmdswap,	NUMREG,			"x\027")
	CMDcstk(RARG_CSTO, 	&cmdcsto,	NUMREG-1,		"\024STO")
	CMDcstk(RARG_CSTO_PL, 	&cmdcsto,	NUMREG-1,		"\024STO+")
	CMDcstk(RARG_CSTO_MI, 	&cmdcsto,	NUMREG-1,		"\024STO-")
	CMDcstk(RARG_CSTO_MU, 	&cmdcsto,	NUMREG-1,		"\024STO\034")
	CMDcstk(RARG_CSTO_DV, 	&cmdcsto,	NUMREG-1,		"\024STO/")
	CMDcstk(RARG_CRCL, 	&cmdcrcl,	NUMREG-1,		"\024RCL")
	CMDcstk(RARG_CRCL_PL, 	&cmdcrcl,	NUMREG-1,		"\024RCL+")
	CMDcstk(RARG_CRCL_MI, 	&cmdcrcl,	NUMREG-1,		"\024RCL-")
	CMDcstk(RARG_CRCL_MU, 	&cmdcrcl,	NUMREG-1,		"\024RCL\034")
	CMDcstk(RARG_CRCL_DV, 	&cmdcrcl,	NUMREG-1,		"\024RCL/")
	CMDcstk(RARG_CSWAP,	&cmdswap,	NUMREG-1,		"\024x\027")
	CMDstk(RARG_VIEW,	&cmdview,	NUMREG,			"VIEW")
	CMD(RARG_STOSTK,	&cmdstostk,	TOPREALREG-4+1,		"STOS")
	CMD(RARG_RCLSTK,	&cmdrclstk,	TOPREALREG-4+1,		"RCLS")
	CMDnoI(RARG_ALPHA,	&cmdalpha,	0,			"")
	CMDstk(RARG_AREG,	&alpha_reg,	NUMREG,			"\240RC#")
	CMDstk(RARG_ASTO,	&alpha_sto,	NUMREG,			"\240STO")
	CMDstk(RARG_ARCL,	&alpha_rcl,	NUMREG,			"\240RCL")
	CMDstk(RARG_AIP,	&alpha_ip,	NUMREG,			"\240IP")
	CMD(RARG_ALRL,		&alpha_shift_l,	NUMALPHA,		"\240RL")
	CMD(RARG_ALRR,		&alpha_rot_r,	NUMALPHA,		"\240RR")
	CMD(RARG_ALSL,		&alpha_shift_l,	NUMALPHA+1,		"\240SL")
	CMD(RARG_ALSR,		&alpha_shift_r,	NUMALPHA+1,		"\240SR")
	CMDstk(RARG_TEST_EQ,	&cmdtest,	NUMREG,			"x=?")
	CMDstk(RARG_TEST_NE,	&cmdtest,	NUMREG,			"x\013?")
	CMDstk(RARG_TEST_APX,	&cmdtest,	NUMREG,			"x\035?")
	CMDstk(RARG_TEST_LT,	&cmdtest,	NUMREG,			"x<?")
	CMDstk(RARG_TEST_LE,	&cmdtest,	NUMREG,			"x\011?")
	CMDstk(RARG_TEST_GT,	&cmdtest,	NUMREG,			"x>?")
	CMDstk(RARG_TEST_GE,	&cmdtest,	NUMREG,			"x\012?")
	CMDcstk(RARG_TEST_ZEQ,	&cmdztest,	NUMREG-1,		"\024x=?")
	CMDcstk(RARG_TEST_ZNE,	&cmdztest,	NUMREG-1,		"\024x\013?")
//	CMDcstk(RARG_TEST_ZAPX,	&cmdztest,	NUMREG-1,		"\024x~?")
	CMD(RARG_SKIP,		&cmdskip,	100,			"SKIP")
	CMD(RARG_BACK,		&cmdback,	100,			"BACK")
	CMDstk(RARG_DSE,	&cmdloop,	NUMREG,			"DSE")
	CMDstk(RARG_ISG,	&cmdloop,	NUMREG,			"ISG")
	CMDstk(RARG_DSZ,	&cmdloopz,	NUMREG,			"DSZ")
	CMDstk(RARG_ISZ,	&cmdloopz,	NUMREG,			"ISZ")
	CMDstk(RARG_DEC,	&cmdlincdec,	NUMREG,			"DEC")
	CMDstk(RARG_INC,	&cmdlincdec,	NUMREG,			"INC")
	CMDnoI(RARG_LBL,	NULL,		NUMLBL,			"LBL")
	CMD(RARG_LBLP,		&cmdlblp,	NUMLBL,			"LBL?")
	CMD(RARG_XEQ,		&cmdgto,	NUMLBL,			"XEQ")
	CMD(RARG_GTO,		&cmdgto,	NUMLBL,			"GTO")
	CMD(RARG_SUM,		&xromarg,	NUMLBL,			"\221")
	CMD(RARG_PROD,		&xromarg,	NUMLBL,			"\217")
	CMD(RARG_SOLVE,		&xromarg,	NUMLBL,			"SLV")
	CMD(RARG_DERIV,		&xromarg,	NUMLBL,			"f'(x)")
	CMD(RARG_2DERIV,	&xromarg,	NUMLBL,			"f\"(x)")
	CMD(RARG_INTG,		&xromarg,	NUMLBL,			"INT")

	CMD(RARG_STD,		&cmddisp,	DISPLAY_DIGITS,		"ALL")
	CMD(RARG_FIX,		&cmddisp,	DISPLAY_DIGITS,		"FIX")
	CMD(RARG_SCI,		&cmddisp,	DISPLAY_DIGITS,		"SCI")
	CMD(RARG_ENG,		&cmddisp,	DISPLAY_DIGITS,		"ENG")
	CMD(RARG_DISP,		&cmddisp,	DISPLAY_DIGITS,		"DISP")
	CMD(RARG_SF,		&cmdflag,	NUMFLG,			"SF")
	CMD(RARG_CF,		&cmdflag,	NUMFLG,			"CF")
	CMD(RARG_FF,		&cmdflag,	NUMFLG,			"FF")
	CMD(RARG_FS,		&cmdflag,	NUMFLG,			"FS?")
	CMD(RARG_FC,		&cmdflag,	NUMFLG,			"FC?")
	CMD(RARG_FSC,		&cmdflag,	NUMFLG,			"FS?C")
	CMD(RARG_FSS,		&cmdflag,	NUMFLG,			"FS?S")
	CMD(RARG_FSF,		&cmdflag,	NUMFLG,			"FS?F")
	CMD(RARG_FCC,		&cmdflag,	NUMFLG,			"FC?C")
	CMD(RARG_FCS,		&cmdflag,	NUMFLG,			"FC?S")
	CMD(RARG_FCF,		&cmdflag,	NUMFLG,			"FC?F")
	CMD(RARG_WS,		&intws,		MAX_WORD_SIZE+1,	"WSIZE")
	CMD(RARG_RL,		&introt,	MAX_WORD_SIZE,		"RL")
	CMD(RARG_RR,		&introt,	MAX_WORD_SIZE,		"RR")
	CMD(RARG_RLC,		&introt,	MAX_WORD_SIZE+1,	"RLC")
	CMD(RARG_RRC,		&introt,	MAX_WORD_SIZE+1,	"RRC")
	CMD(RARG_SL,		&introt,	MAX_WORD_SIZE+1,	"SL")
	CMD(RARG_SR,		&introt,	MAX_WORD_SIZE+1,	"SR")
	CMD(RARG_ASR,		&introt,	MAX_WORD_SIZE+1,	"ASR")
	CMD(RARG_SB,		&intbits,	MAX_WORD_SIZE,		"SB")
	CMD(RARG_CB,		&intbits,	MAX_WORD_SIZE,		"CB")
	CMD(RARG_FB,		&intbits,	MAX_WORD_SIZE,		"FB")
	CMD(RARG_BS,		&intbits,	MAX_WORD_SIZE,		"BS?")
	CMD(RARG_BC,		&intbits,	MAX_WORD_SIZE,		"BC?")
	CMD(RARG_MASKL,		&intmsks,	MAX_WORD_SIZE+1,	"MASKL")
	CMD(RARG_MASKR,		&intmsks,	MAX_WORD_SIZE+1,	"MASKR")
	CMD(RARG_BASE,		&set_int_base,	17,			"BASE")

	CMDnoI(RARG_CONV,	&cmdconv,	NUM_CONSTS_CONV*2,	"conv")

	CMD(RARG_INISOLVE,	&solver,	TOPREALREG-10+1,	"SLVI")
	CMD(RARG_SOLVESTEP,	&solver,	TOPREALREG-10+1,	"SLVS")

	CMD(RARG_PAUSE,		&op_pause,	100,			"PSE")
	CMDstk(RARG_KEY,	&op_keyp,	NUMREG,			"KEY?")

	CMDstk(RARG_ALPHAXEQ,	&cmdalphagto,	NUMREG,			"\240XEQ")
	CMDstk(RARG_ALPHAGTO,	&cmdalphagto,	NUMREG,			"\240GTO")

	CMD(RARG_PLOAD,		&load_program,	NUMBER_OF_FLASH_REGIONS-1,	"PRCL")
	CMD(RARG_PSAVE,		&save_program,	NUMBER_OF_FLASH_REGIONS-1,	"PSTO")
	CMD(RARG_PSWAP,		&swap_program,	NUMBER_OF_FLASH_REGIONS-1,	"P\027")
	CMDstk(RARG_FLRCL, 	&cmdflashrcl,	NUMREG,			"RCF")
	CMDstk(RARG_FLRCL_PL, 	&cmdflashrcl,	NUMREG,			"RCF+")
	CMDstk(RARG_FLRCL_MI, 	&cmdflashrcl,	NUMREG,			"RCF-")
	CMDstk(RARG_FLRCL_MU, 	&cmdflashrcl,	NUMREG,			"RCF\034")
	CMDstk(RARG_FLRCL_DV, 	&cmdflashrcl,	NUMREG,			"RCF/")
	CMDstk(RARG_FLRCL_MIN,	&cmdflashrcl,	NUMREG,			"RCF\017")
	CMDstk(RARG_FLRCL_MAX,	&cmdflashrcl,	NUMREG,			"RCF\020")
	CMDcstk(RARG_FLCRCL, 	&cmdflashcrcl,	NUMREG-1,		"\024RCF")
	CMDcstk(RARG_FLCRCL_PL,	&cmdflashcrcl,	NUMREG-1,		"\024RCF+")
	CMDcstk(RARG_FLCRCL_MI, &cmdflashcrcl,	NUMREG-1,		"\024RCF-")
	CMDcstk(RARG_FLCRCL_MU, &cmdflashcrcl,	NUMREG-1,		"\024RCF\034")
	CMDcstk(RARG_FLCRCL_DV, &cmdflashcrcl,	NUMREG-1,		"\024RCF/")

	CMD(RARG_SLD,		&op_shift_digit,100,			"S.L")
	CMD(RARG_SRD,		&op_shift_digit,100,			"S.R")

	CMDstk(RARG_VIEW_REG,	&alpha_view_reg,NUMREG,			"VW\240+")
	CMD(RARG_ROUNDING,	&rarg_roundingmode, DEC_ROUND_MAX,	"RM")

#ifdef INCLUDE_USER_MODE
	CMDstk(RARG_SAVEM,	&cmdsavem,	NUMREG,			"STOM")
	CMDstk(RARG_RESTM,	&cmdrestm,	NUMREG,			"RCLM")
#endif

#undef CMDnoI
#undef CMDstk
#undef CMD
#undef allCMD
};

#if COMMANDS_PASS != 2
const unsigned short num_argcmds = sizeof(argcmds) / sizeof(struct argcmd);
#endif


#ifdef COMPILE_CATALOGUES
#define CMD(name, func, nm)			\
	{ PTR, nm },
#elif DEBUG
#define CMD(name, func, nm)			\
	{ name, func, nm },
#elif COMMANDS_PASS == 1
#define CMD(name, func, nm)			\
	{ 0xaa55, nm },
#else
#define CMD(name, func, nm)			\
	{ func, nm },
#endif

#if COMMANDS_PASS == 2
CMDTAB const struct multicmd_cmdtab multicmds_ct[ NUM_MULTI ] = {
#else
const struct multicmd multicmds[ NUM_MULTI ] = {
#endif
	CMD(DBL_LBL,	NULL,		"LBL")
	CMD(DBL_LBLP,	&cmdmultilblp,	"LBL?")
	CMD(DBL_XEQ,	&cmdmultigto,	"XEQ")
	CMD(DBL_GTO,	&cmdmultigto,	"GTO")
	CMD(DBL_SUM,	&multixromarg,	"\221")
	CMD(DBL_PROD,	&multixromarg,	"\217")
	CMD(DBL_SOLVE,	&multixromarg,	"SLV")
	CMD(DBL_DERIV,	&multixromarg,	"f'(x)")
	CMD(DBL_2DERIV,	&multixromarg,	"f\"(x)")
	CMD(DBL_INTG,	&multixromarg,	"INT")
#ifdef MULTI_ALPHA
	CMD(DBL_ALPHA,	&multialpha,	"\240")
#endif
//	CMD(DBL_NUMBER,	NULL,		"#")
#undef CMD
};

#if COMMANDS_PASS != 2
const unsigned short num_multicmds = sizeof(multicmds) / sizeof(struct multicmd);
#endif


/*
 *  We need to include the same file a second time with updated #defines.
 *  This will create the structures in the CMDTAB segment with all pointers filled in.
 */
#if COMMANDS_PASS == 1
#undef COMMANDS_PASS
#define COMMANDS_PASS 2
#include "commands.c"
#undef COMMANDS_PASS
#endif

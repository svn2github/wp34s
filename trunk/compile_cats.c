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

#define COMPILE_CATALOGUES
#undef DEBUG
#define NOCURSES 1
#undef REALBUILD

#include "consts.h"
#include "xeq.h"

#define CST(op, n)		RARG(RARG_CONST, op),
#define CCST(op, n)		RARG(RARG_CONST_CMPLX, op),
#define TRI(op, n)		op | OP_TRI,
#define DYA(op, n)		op | OP_DYA,
#define MON(op, n)		op | OP_MON,
#define CDYA(op, n)		op | OP_CDYA,
#define CMON(op, n)		op | OP_CMON,
#define NILIC(op, n)		op | OP_NIL,
#define SPECIAL(op, n)		op | OP_SPEC,
#define RARGCMD(op, n)		RARG_BASE(op),
#define CONV(n, d, name)	RARG_BASE(RARG_CONV) + (n)*2 + (d),


static s_opcode program_xfcn[] = {
	MON(OP__1POW,		"(-1)^x")
	DYA(OP_PERMG,		"%+MG")
	DYA(OP_MARGIN,		"%MG")
	TRI(OP_PERMRR,		"%MRR")
	MON(OP_PERTOT,		"%T")
	MON(OP_sigper,		"%\221")
	DYA(OP_ATAN2,		"ANGLE")
#ifdef INCLUDE_BERNOULLI
	MON(OP_Bn,		"B[sub-n]")
	MON(OP_BnS,		"B[sub-n]*")
#endif
	MON(OP_CEIL,		"CEIL")
	NILIC(OP_CLALL,		"CLALL")
	NILIC(OP_CLREG,		"CLREG")
	MON(OP_CUBE,		"CUBE")
	MON(OP_CUBERT,		"CUBERT")
	NILIC(OP_DROP,		"DROP")
	MON(OP_D2J,		"D\015J")
	MON(OP_DOWK,		"WDAY")
	DYA(OP_DTADD,		"DAYS+")
	NILIC(OP_2FRAC,		"DECOMP")
	MON(OP_ERF,		"erf")
	MON(OP_ERFC,		"erfc")
	MON(OP_EXPM1,		"e^x-1")
#ifdef INCLUDE_FACTOR
	MON(OP_FACTOR,		"FACTOR")
#endif 
	MON(OP_FIB,		"FIB")
	MON(OP_FLOOR,		"FLOOR")
	DYA(OP_GCD,		"GCD")
	TRI(OP_BETAI,		"I\241")
	DYA(OP_GAMMAP,		"I\202")
	MON(OP_J2D,		"J\015D")
	DYA(OP_LCM,		"LCM")
	MON(OP_LN1P,		"LN1P")
	DYA(OP_LNBETA,		"LN\241")
	MON(OP_LNGAMMA,		"LN\202")
	DYA(OP_MAX,		"MAX")
	DYA(OP_MIN,		"MIN")
	DYA(OP_LNAND,		"NAND")
	DYA(OP_LNOR,		"NOR")
	NILIC(OP_RESET,		"RESET")
	MON(OP_ROUND,		"ROUNDI")
	NILIC(OP_SETDATE,	"SETDAT")
	NILIC(OP_SETTIME,	"SETTIM")
	MON(OP_SIGN,		"SIGN")
	MON(OP_SINC,		"SINC")
	RARGCMD(RARG_SLD,	"SDL")
	RARGCMD(RARG_SRD,	"SDR")
	NILIC(OP_VERSION,	"VERS")
	MON(OP_LAMW,		"W")
	MON(OP_INVW,		"W\235")
	DYA(OP_LXNOR,		"XNOR")
	NILIC(OP_ALPHADATE,	"\240DATE")
	NILIC(OP_ALPHADAY,	"\240DAY")
	RARGCMD(RARG_AIP,	"\240IP")
	NILIC(OP_ALPHALEN,	"\240LENG")
	NILIC(OP_ALPHAMONTH,	"\240MONTH")
	RARGCMD(RARG_AREG,	"\240RC#")
	RARGCMD(RARG_ARCL,	"\240RCL")
	RARGCMD(RARG_ALRL,	"\240RL")
	RARGCMD(RARG_ALRR,	"\240RR")
	RARGCMD(RARG_ALSL,	"\240SL")
	RARGCMD(RARG_ALSR,	"\240SR")
	RARGCMD(RARG_ASTO,	"\240STO")
	NILIC(OP_ALPHATIME,	"\240TIME")
	DYA(OP_BETA,		"\241")
	MON(OP_GAMMA,		"\202")
	DYA(OP_DTDIF,		"\203DAYS")
	DYA(OP_AGM,		"AGM")
	MON(OP_DEG2,		"DED\015")
	MON(OP_RAD2,		"RAD\015")
	MON(OP_GRAD2,		"GRAD\015")
#ifdef INCLUDE_SUBFACT
	MON(OP_SUBFACT,		"!n")
#endif
#ifdef INCLUDE_DBLFACT
	MON(OP_DBLFACT,		"x!!")
#endif
#ifdef INCLUDE_MULADD
	TRI(OP_MULADD,		"\034+")
#endif
#ifdef INCLUDE_ELLIPTIC
	DYA(OP_CN,		"CN")
	DYA(OP_DN,		"DN")
	DYA(OP_SN,		"SN")
#endif
#ifdef INCLUDE_EASTER
	MON(OP_EASTER,		"EASTER")
#endif
#ifdef INCLUDE_ZETA
	MON(OP_ZETA,		"\245")
#endif
#ifdef INCLUDE_DIGAMMA
	MON(OP_PSI,		"\226")
#endif
#ifdef INCLUDE_BESSEL
	DYA(OP_BSIN,		"In")
	DYA(OP_BSJN,		"Jn")
	DYA(OP_BSKN,		"Kn")
	DYA(OP_BSYN,		"Yn")
#endif
	NILIC(OP_QUAD,		"QUAD")
	NILIC(OP_NEXTPRIME,	"NEXTP")

	MON(OP_DATE_DAY,	"DAY")
	MON(OP_DATE_MONTH,	"MONTH")
	MON(OP_DATE_YEAR,	"YEAR")

	DYA(OP_LEGENDRE_PN,   "P\275")
	DYA(OP_CHEBYCHEV_TN,  "T\275")
	DYA(OP_CHEBYCHEV_UN,  "U\275")
	DYA(OP_LAGUERRE,      "L\275")
	TRI(OP_GEN_LAGUERRE,  "L\275\240")
	DYA(OP_HERMITE_HE,    "HE\275")
	DYA(OP_HERMITE_H,     "H\275")
	NILIC(OP_VOLTAGE,     "BATT")
	RARGCMD(RARG_FLRCL, 	"RCF")
#ifdef INCLUDE_MANTISSA
	MON(OP_MANTISSA,	"MANT")
	MON(OP_EXPONENT,	"EXPT")
#endif
#ifdef INCLUDE_XROOT
	DYA(OP_XROOT,		"\234\003y")
#endif


	/* Integer mode commands */
	RARGCMD(RARG_ASR,	"ASR")
	RARGCMD(RARG_CB,	"CB")
	NILIC(OP_CLFLAGS,	"CLFLAG")
	NILIC(OP_DBL_MUL,	"DBL\034")
	TRI(OP_DBL_DIV, 	"DBL/")
	TRI(OP_DBL_MOD, 	"DBLR")
	RARGCMD(RARG_FB,	"FB")
	NILIC(OP_LJ,		"LJ")
	RARGCMD(RARG_MASKL,	"MASKL")
	RARGCMD(RARG_MASKR,	"MASKR")
	MON(OP_MIRROR,		"MIRROR")
	MON(OP_BITCNT,		"nBITS")
	NILIC(OP_RJ,		"RJ")
	RARGCMD(RARG_RL,	"RL")
	RARGCMD(RARG_RLC,	"RLC")
	RARGCMD(RARG_RR,	"RR")
	RARGCMD(RARG_RRC,	"RRC")
	RARGCMD(RARG_SB,	"SB")
	NILIC(OP_STORANDOM,	"SEED")
	MON(OP_SIGN,		"SIGN")
	RARGCMD(RARG_SL,	"SL")
	RARGCMD(RARG_SR,	"SR")
};


static s_opcode catalogue[] = {
	MON(OP__1POW,		"(-1)^x")
	DYA(OP_PERMG,		"%+MG")
	DYA(OP_MARGIN,		"%MG")
	TRI(OP_PERMRR,		"%MRR")
	MON(OP_PERTOT,		"%T")
	MON(OP_sigper,		"%\221")
	DYA(OP_ATAN2,		"ANGLE")
#ifdef INCLUDE_BERNOULLI
	MON(OP_Bn,		"B[sub-n]")
	MON(OP_BnS,		"B[sub-n]*")
#endif
	MON(OP_CEIL,		"CEIL")
	NILIC(OP_CLALL,		"CLALL")
	NILIC(OP_CLREG,		"CLREG")
	MON(OP_CUBE,		"CUBE")
	MON(OP_CUBERT,		"CUBERT")
	NILIC(OP_DROP,		"DROP")
	MON(OP_D2J,		"D\015J")
	MON(OP_DOWK,		"WDAY")
	DYA(OP_DTADD,		"DAYS+")
	NILIC(OP_2FRAC,		"DECOMP")
	MON(OP_ERF,		"erf")
	MON(OP_ERFC,		"erfc")
	MON(OP_EXPM1,		"e^x-1")
#ifdef INCLUDE_FACTOR
	MON(OP_FACTOR,		"FACTOR")
#endif 
	MON(OP_FIB,		"FIB")
	MON(OP_FLOOR,		"FLOOR")
	DYA(OP_GCD,		"GCD")
	TRI(OP_BETAI,		"I\241")
	DYA(OP_GAMMAP,		"I\202")
	MON(OP_J2D,		"J\015D")
	DYA(OP_LCM,		"LCM")
	MON(OP_LN1P,		"LN1P")
	DYA(OP_LNBETA,		"LN\241")
	MON(OP_LNGAMMA,		"LN\202")
	DYA(OP_MAX,		"MAX")
	DYA(OP_MIN,		"MIN")
	DYA(OP_LNAND,		"NAND")
	DYA(OP_LNOR,		"NOR")
	NILIC(OP_RESET,		"RESET")
	MON(OP_ROUND,		"ROUNDI")
	NILIC(OP_SETDATE,	"SETDAT")
	NILIC(OP_SETTIME,	"SETTIM")
	MON(OP_SIGN,		"SIGN")
	MON(OP_SINC,		"SINC")
	RARGCMD(RARG_SLD,	"SDL")
	RARGCMD(RARG_SRD,	"SDR")
	NILIC(OP_VERSION,	"VERS")
	MON(OP_LAMW,		"W")
	MON(OP_INVW,		"W\235")
	DYA(OP_LXNOR,		"XNOR")
	NILIC(OP_ALPHADATE,	"\240DATE")
	NILIC(OP_ALPHADAY,	"\240DAY")
	RARGCMD(RARG_AIP,	"\240IP")
	NILIC(OP_ALPHALEN,	"\240LENG")
	NILIC(OP_ALPHAMONTH,	"\240MONTH")
	RARGCMD(RARG_AREG,	"\240RC#")
	RARGCMD(RARG_ARCL,	"\240RCL")
	RARGCMD(RARG_ALRL,	"\240RL")
	RARGCMD(RARG_ALRR,	"\240RR")
	RARGCMD(RARG_ALSL,	"\240SL")
	RARGCMD(RARG_ALSR,	"\240SR")
	RARGCMD(RARG_ASTO,	"\240STO")
	NILIC(OP_ALPHATIME,	"\240TIME")
	DYA(OP_BETA,		"\241")
	MON(OP_GAMMA,		"\202")
	DYA(OP_DTDIF,		"\203DAYS")
	DYA(OP_AGM,		"AGM")
	MON(OP_DEG2,		"DED\015")
	MON(OP_RAD2,		"RAD\015")
	MON(OP_GRAD2,		"GRAD\015")
#ifdef INCLUDE_SUBFACT
	MON(OP_SUBFACT,		"!n")
#endif
#ifdef INCLUDE_DBLFACT
	MON(OP_DBLFACT,		"x!!")
#endif
#ifdef INCLUDE_MULADD
	TRI(OP_MULADD,		"\034+")
#endif
#ifdef INCLUDE_ELLIPTIC
	DYA(OP_CN,		"CN")
	DYA(OP_DN,		"DN")
	DYA(OP_SN,		"SN")
#endif
#ifdef INCLUDE_EASTER
	MON(OP_EASTER,		"EASTER")
#endif
#ifdef INCLUDE_ZETA
	MON(OP_ZETA,		"\245")
#endif
#ifdef INCLUDE_DIGAMMA
	MON(OP_PSI,		"\226")
#endif
#ifdef INCLUDE_BESSEL
	DYA(OP_BSIN,		"In")
	DYA(OP_BSJN,		"Jn")
	DYA(OP_BSKN,		"Kn")
	DYA(OP_BSYN,		"Yn")
#endif
	NILIC(OP_QUAD,		"QUAD")
	NILIC(OP_NEXTPRIME,	"NEXTP")

	MON(OP_DATE_DAY,	"DAY")
	MON(OP_DATE_MONTH,	"MONTH")
	MON(OP_DATE_YEAR,	"YEAR")

	DYA(OP_LEGENDRE_PN,   "P\275")
	DYA(OP_CHEBYCHEV_TN,  "T\275")
	DYA(OP_CHEBYCHEV_UN,  "U\275")
	DYA(OP_LAGUERRE,      "L\275")
	TRI(OP_GEN_LAGUERRE,  "L\275\240")
	DYA(OP_HERMITE_HE,    "HE\275")
	DYA(OP_HERMITE_H,     "H\275")
	NILIC(OP_VOLTAGE,     "BATT")
	RARGCMD(RARG_FLRCL, 	"RCF")
#ifdef INCLUDE_MANTISSA
	MON(OP_MANTISSA,	"MANT")
	MON(OP_EXPONENT,	"EXPT")
#endif
#ifdef INCLUDE_XROOT
	DYA(OP_XROOT,		"\234\003y")
#endif
};

static s_opcode cplx_catalogue[] = {
	CMON(OP__1POW,		"(-1)^x")
	CMON(OP_CCONJ,		"CONJ")
	CMON(OP_CUBE,		"CUBE")
	CMON(OP_CUBERT,		"CUBERT")
	NILIC(OP_DROPXY,	"DROP")
	CMON(OP_EXPM1,		"e^x-1")
	CMON(OP_FIB,		"FIB")
	CMON(OP_LN1P,		"LN1P")
	CDYA(OP_LNBETA,		"LN\241")
	CMON(OP_LNGAMMA,	"LN\202")
	CMON(OP_SIGN,		"SIGN")
	CMON(OP_SINC,		"SINC")
	CMON(OP_LAMW,		"W")
	CMON(OP_INVW,		"W\235")
	CDYA(OP_BETA,		"\241")
	CMON(OP_GAMMA,		"\202")
	CDYA(OP_AGM,		"AGM")
#ifdef INCLUDE_DBLFACT
	CMON(OP_DBLFACT,	"x!!")
#endif
#ifdef INCLUDE_COMPLEX_ZETA
	CMON(OP_ZETA,		"\245")
#endif
#ifdef INCLUDE_DIGAMMA
	CMON(OP_PSI,		"\226")
#endif
#ifdef COMPLEX_BESSEL
	CDYA(OP_BSIN,		"In")
	CDYA(OP_BSJN,		"Jn")
	CDYA(OP_BSKN,		"Kn")
	CDYA(OP_BSYN,		"Yn")
#endif
#ifdef INCLUDE_ELLIPTIC
	CDYA(OP_CN,		"CN")
	CDYA(OP_DN,		"DN")
	CDYA(OP_SN,		"SN")
#endif
	RARGCMD(RARG_FLCRCL, 	"\024RCF")
#ifdef INCLUDE_XROOT
	CDYA(OP_XROOT,		"\234\003y")
#endif
};

static s_opcode stats_catalogue[] = {
	MON(OP_sigper,		"%\221")
	NILIC(OP_BEST,		"BESTF")
	NILIC(OP_statCOV,	"COV")
	NILIC(OP_statSxy,	"sxy")
	NILIC(OP_EXPF,		"EXPF")
	NILIC(OP_LINF,		"LINF")
	DYA(OP_LNBETA,		"LN\241")
	MON(OP_LNGAMMA,		"LN\202")
	NILIC(OP_LOGF,		"LOGF")
	NILIC(OP_sigmaN,	"n\221")
	NILIC(OP_PWRF,		"PWRF")
	NILIC(OP_STORANDOM,	"SEED")
	NILIC(OP_statSErr,	"SERR")
	NILIC(OP_RCLSIGMA,	"SUM")
	MON(OP_xhat,		"\031")
	NILIC(OP_statWMEAN,	"\001w")
	NILIC(OP_statGMEAN,	"\001g")
	NILIC(OP_statGSErr,	"\244m")
	NILIC(OP_statGS,	"\244")
	NILIC(OP_statGSigma,	"\244\276")
	NILIC(OP_statWSErr,	"SERRw")
	NILIC(OP_statWS,	"sw")
	NILIC(OP_statWSigma,	"\244w")
	DYA(OP_BETA,		"\241")
	MON(OP_GAMMA,		"\202")
	NILIC(OP_statSigma,	"\261")
	NILIC(OP_sigmalnXlnX,	"\221ln\232X")
	NILIC(OP_sigmalnYlnY,	"\221ln\232Y")
	NILIC(OP_sigmalnX,	"\221lnX")
	NILIC(OP_sigmalnXlnY,	"\221lnXY")
	NILIC(OP_sigmalnY,	"\221lnY")
	NILIC(OP_sigmaX,	"\221x")
	NILIC(OP_sigmaX2,	"\221x\232")
	NILIC(OP_sigmaXlnY,	"\221XlnY")
	NILIC(OP_sigma_XY,	"\221xy")
	NILIC(OP_sigmaX2Y,	"\221x\232y")
	NILIC(OP_sigmaY,	"\221y")
	NILIC(OP_sigmaY2,	"\221y\232")
	NILIC(OP_sigmaYlnX,	"\221YlnX")
};

static s_opcode prob_catalogue[] = {
	MON(OP_pdf_B,		"Binom\276")
	MON(OP_cdf_B,		"B(n)")
	MON(OP_qf_B,		"B\235(p)")
	MON(OP_pdf_C,		"Cap(x)")
	MON(OP_cdf_C,		"Ca(x)")
	MON(OP_qf_C,		"Ca\235(p)")
	MON(OP_pdf_EXP,		"Exp(x)")
	MON(OP_cdf_EXP,		"Ex(x)")
	MON(OP_qf_EXP,		"Ex/235(p)")
	MON(OP_pdf_F,		"F\276(x)")
	MON(OP_cdf_F,		"F(x)")
	MON(OP_qf_F,		"F/235(p)")
	MON(OP_pdf_G,		"Geom\276")
	MON(OP_cdf_G,		"Geom")
	MON(OP_qf_G,		"Geom/235")
	MON(OP_pdf_LG,		"Lgp(x)")
	MON(OP_cdf_LG,		"Lg(x)")
	MON(OP_qf_LG,		"Lg\235(p)")
	MON(OP_pdf_LN,		"Lnp(x)")
	MON(OP_cdf_LN,		"Ln(x)")
	MON(OP_qf_LN,		"Ln\235(p)")
	MON(OP_pdf_N,		"Np(x)")
	MON(OP_cdf_N,		"N(x)")
	MON(OP_qf_N,		"N\235(p)")
	MON(OP_pdf_P,		"P\276")
	MON(OP_cdf_P,		"P(n)")
	MON(OP_qf_P,		"P/235(p)")
	MON(OP_pdf_T,		"t\276(x)")
	MON(OP_cdf_T,		"t(x)")
	MON(OP_qf_T,		"t/235(p)")
	MON(OP_pdf_WB,		"WB\276")
	MON(OP_cdf_WB,		"WB(x)")
	MON(OP_qf_WB,		"WB/235(p)")
	MON(OP_pdf_chi2,	"\225\232\276")
	MON(OP_cdf_chi2,	"\225\232")
	MON(OP_qf_chi2,		"\225\232INV")
	MON(OP_pdf_Q,		"\224p(x)")
};

static s_opcode int_catalogue[] = {
	MON(OP__1POW,		"-1^x")
	RARGCMD(RARG_ASR,	"ASR")
	RARGCMD(RARG_CB,	"CB")
	NILIC(OP_CLALL,		"CLALL")
	NILIC(OP_CLFLAGS,	"CLFLAG")
	NILIC(OP_CLREG,		"CLREG")
	MON(OP_CUBE,		"CUBE")
	MON(OP_CUBERT,		"CUBERT")
	NILIC(OP_DBL_MUL,	"DBL\034")
	TRI(OP_DBL_DIV, 	"DBL/")
	TRI(OP_DBL_MOD, 	"DBLR")
	NILIC(OP_DROP,		"DROP")
	RARGCMD(RARG_FB,	"FB")
	MON(OP_FIB,		"FIB")
	DYA(OP_GCD,		"GCD")
	DYA(OP_LCM,		"LCM")
	NILIC(OP_LJ,		"LJ")
	RARGCMD(RARG_MASKL,	"MASKL")
	RARGCMD(RARG_MASKR,	"MASKR")
	DYA(OP_MAX,		"MAX")
	DYA(OP_MIN,		"MIN")
	MON(OP_MIRROR,		"MIRROR")
	DYA(OP_LNAND,		"NAND")
	MON(OP_BITCNT,		"nBITS")
	DYA(OP_LNOR,		"NOR")
#ifdef INCLUDE_FACTOR
	MON(OP_FACTOR,		"FACTOR")
#endif 
	NILIC(OP_NEXTPRIME,	"NEXTP")
	NILIC(OP_RESET,		"RESET")
	NILIC(OP_RJ,		"RJ")
	RARGCMD(RARG_RL,	"RL")
	RARGCMD(RARG_RLC,	"RLC")
	RARGCMD(RARG_RR,	"RR")
	RARGCMD(RARG_RRC,	"RRC")
	RARGCMD(RARG_SB,	"SB")
	NILIC(OP_STORANDOM,	"SEED")
	MON(OP_SIGN,		"SIGN")
	RARGCMD(RARG_SL,	"SL")
	RARGCMD(RARG_SR,	"SR")
	NILIC(OP_VERSION,	"VERS")
	NILIC(OP_VOLTAGE,	"BATT")
	DYA(OP_LXNOR,		"XNOR")
	RARGCMD(RARG_AIP,	"\240IP")
	NILIC(OP_ALPHALEN,	"\240LENG")
	RARGCMD(RARG_AREG,	"\240RC#")
	RARGCMD(RARG_ARCL,	"\240RCL")
	RARGCMD(RARG_ALRL,	"\240RL")
	RARGCMD(RARG_ALRR,	"\240RR")
	RARGCMD(RARG_ALSL,	"\240SL")
	RARGCMD(RARG_ALSR,	"\240SR")
	RARGCMD(RARG_ASTO,	"\240STO")
#ifdef INCLUDE_MULADD
	TRI(OP_MULADD,		"\034+")
#endif
	RARGCMD(RARG_FLRCL, 	"RCF")
#ifdef INCLUDE_XROOT
	DYA(OP_XROOT,		"\234\003y")
#endif
};

static s_opcode test_catalogue[] = {
	RARGCMD(RARG_BC,	"BC?")
	RARGCMD(RARG_BS,	"BS?")
	RARGCMD(RARG_FC,	"FC?")
	RARGCMD(RARG_FCC,	"FC?C")
	RARGCMD(RARG_FCF,	"FC?F")
	RARGCMD(RARG_FCS,	"FC?S")
	RARGCMD(RARG_FS,	"FS?")
	RARGCMD(RARG_FSC,	"FS?C")
	RARGCMD(RARG_FSF,	"FS?F")
	RARGCMD(RARG_FSS,	"FS?S")

	NILIC(OP_XisEVEN,	"EVEN?")
	NILIC(OP_XisFRAC,	"FP?")
	NILIC(OP_XisINT,	"INT?")
	NILIC(OP_ISLEAP,	"LEAP?")
	NILIC(OP_XisNaN,	"NaN?")
	NILIC(OP_XisODD,	"ODD?")
	NILIC(OP_XisPRIME,	"PRIME?")
	NILIC(OP_STKSIZE,	"SSIZE?")
	NILIC(OP_INTSIZE,	"WSIZE?")
	NILIC(OP_GETBASE,	"IBASE?")
	NILIC(OP_GETSIGN,	"SMODE?")
	NILIC(OP_XisInf,	"\237?")
	NILIC(OP_XisSpecial,	"SPEC?")
	NILIC(OP_ROUNDING,	"RM?")

	RARGCMD(RARG_TEST_APX,	"x~?")
	RARGCMD(RARG_TEST_LT,	"x<?")
	RARGCMD(RARG_TEST_LE,	"x<=?")
	RARGCMD(RARG_TEST_GE,	"x>=?")
	RARGCMD(RARG_TEST_GT,	"x>?")
//	RARGCMD(RARG_TEST_ZAPX,	"cx~?")

	RARGCMD(RARG_LBLP,	"LBL?")
	NILIC(OP_ENTRYP,	"ENTRY?")
	RARGCMD(RARG_KEY,	"KEY?")
	RARGCMD(RARG_KEYTYPE,	"KTP?")
	NILIC(OP_TOP,		"TOP?")
	NILIC(OP_ISINT,		"INTM?")
	NILIC(OP_ISFLOAT,	"REALM?")
	NILIC(OP_Xeq_pos0,	"x=+0?")
	NILIC(OP_Xeq_neg0,	"x=-0?")
};

static s_opcode prog_catalogue[] = {
	RARGCMD(RARG_STOSTK,	"\015STK")
	RARGCMD(RARG_RCLSTK,	"\016STK")
	RARGCMD(RARG_CF,	"CF")
	NILIC(OP_CLFLAGS,	"CLFLAG")
	NILIC(OP_CLSTK,		"CLSTK")
	NILIC(OP_DATE,		"DATE")
	RARGCMD(RARG_DEC,	"DEC")
	NILIC(OP_DROP,		"DROP")
	RARGCMD(RARG_DSL,	"DSL")
	RARGCMD(RARG_DSZ,	"DSZ")
	RARGCMD(RARG_FF,	"FF")
	RARGCMD(RARG_DERIV,	"f'(x)")
	RARGCMD(RARG_2DERIV,	"f\"(x)")
	DYA(OP_HMSADD,		"H.MS+")
	DYA(OP_HMSSUB,		"H.MS-")
	RARGCMD(RARG_INC,	"INC")
	RARGCMD(RARG_ISE,	"ISE")
	RARGCMD(RARG_ISZ,	"ISZ")
	NILIC(OP_NOP,		"NOP")
	NILIC(OP_PROMPT,	"PROMPT")
	NILIC(OP_REGCLR,	"R-CLR")
	NILIC(OP_REGCOPY,	"R-COPY")
	NILIC(OP_REGSORT,	"R-SORT")
	NILIC(OP_REGSWAP,	"R-SWAP")
	NILIC(OP_RADCOM,	"RDX,")
	NILIC(OP_RADDOT,	"RDX.")
	RARGCMD(RARG_SF,	"SF")
	NILIC(OP_TICKS,		"TICKS")
	NILIC(OP_TIME,		"TIME")
	NILIC(OP_ALPHAOFF,	"\240OFF")
	NILIC(OP_ALPHAON,	"\240ON")
	RARGCMD(RARG_VIEW_REG,	"VW\240+")
	NILIC(OP_VIEWALPHA,	"\240VIEW")
	RARGCMD(RARG_BACK,	"BACK")
	RARGCMD(RARG_SKIP,	"SKIP")
	NILIC(OP_RTNp1,		"RTN+1")
	RARGCMD(RARG_ERROR,	"ERR")
	NILIC(OP_XEQALPHA,	"XEQ\240")
	NILIC(OP_GTOALPHA,	"GTO\240")
	RARGCMD(RARG_ALPHAXEQ,	"\240XEQ")
	RARGCMD(RARG_ALPHAGTO,	"\240GTO")

	/* Serial commands */
	NILIC(OP_SENDP,		"SENDP")
	NILIC(OP_SENDR,		"SENDR")
	NILIC(OP_SENDA,		"SENDA")
	NILIC(OP_RECV,		"RECV")
	RARGCMD(RARG_SENDL,	"SENDL")

#ifdef INCLUDE_USER_IO
	NILIC(OP_SEND1,		"SEND1")
	MON(OP_RECV1,		"RECV1")
	NILIC(OP_SERIAL_OPEN,	"SOPEN")
	NILIC(OP_SERIAL_CLOSE,	"SCLOSE")
	NILIC(OP_ALPHASEND,	"\240SEND")
	NILIC(OP_ALPHARECV,	"\240RECV")
#endif
#ifdef INCLUDE_MULTI_DELETE
	RARGCMD(RARG_DELPROG,	"DEL[sub-p]")
#endif
	RARGCMD(RARG_PSAVE,	"PSTO")
	RARGCMD(RARG_PLOAD,	"PRCL")
	RARGCMD(RARG_PSWAP,	"P<>")
	NILIC(OP_RLOAD,		"RCF.RG")
	NILIC(OP_SLOAD,		"RCF.ST")
	NILIC(OP_BACKUP,	"SAVE")
	NILIC(OP_RESTORE,	"LOAD")
	RARGCMD(RARG_FLRCL, 	"RCF")
	RARGCMD(RARG_PUTKEY,	"PUTK")
};

#ifdef INCLUDE_INTERNAL_CATALOGUE
static s_opcode internal_catalogue[] = {
	RARGCMD(RARG_CONST_INT,	"iC")
	RARGCMD(RARG_INISOLVE,	"SLVI")
	RARGCMD(RARG_SOLVESTEP,	"SLVS")
	NILIC(OP_GSBuser,	"XEQUSR")
#ifdef MATRIX_SUPPORT
	MON(OP_MAT_ALL,		"M.ALL")
	DYA(OP_MAT_ROW,		"M.ROW")
	DYA(OP_MAT_COL,		"M.COL")
	MON(OP_MAT_RQ,		"M.ROW?")
	MON(OP_MAT_CQ,		"M.COL?")
	MON(OP_MAT_TRN,		"M.TRN")
	TRI(OP_MAT_MUL,		"M.[times]")
	TRI(OP_MAT_GADD,	"M.+[times]")
	TRI(OP_MAT_REG,		"M.REG")
	MON(OP_MAT_IJ,		"M.IJ")
#ifdef #ifdef MATRIX_SUPPORT
	NILIC(OP_MAT_ROW_SWAP,	"M.R<>")
	NILIC(OP_MAT_ROW_MUL,	"M.R*")
	NILIC(OP_MAT_ROW_GADD,	"M.R+*")
#endif
	NILIC(OP_MAT_CHECK_SQUARE, "M.SQR?")
	MON(OP_MAT_DET,		"M.DET")
	MON(OP_MAT_LU,		"M.LU")
#endif
#ifdef SILLY_MATRIX_SUPPORT
	NILIC(OP_MAT_ZERO,	"M.ZERO")
	NILIC(OP_MAT_IDENT,	"M.IDEN")
#endif
};
#endif

static s_opcode mode_catalogue[] = {
	NILIC(OP_RADCOM,	"RDX,")
	NILIC(OP_RADDOT,	"RDX.")
	NILIC(OP_1COMP,		"1COMPL")
	NILIC(OP_2COMP,		"2COMPL")
	NILIC(OP_SIGNMANT,	"SIGNMT")
	NILIC(OP_UNSIGNED,	"UNSIGN")
	RARGCMD(RARG_WS,	"WSIZE")
	NILIC(OP_LEAD0,		"SHOW 0")
	NILIC(OP_TRIM0,		"HIDE 0")
	RARGCMD(RARG_BASE,	"BASE")
	NILIC(OP_DENANY,	"DENANY")
	NILIC(OP_DENFAC,	"DENMFAC")
	NILIC(OP_DENFIX,	"DENFIX")
	NILIC(OP_FRACDENOM,	"DENMAX")
	RARGCMD(RARG_DISP,	"DISP")
	NILIC(OP_THOUS_OFF,	"E3OFF")
	NILIC(OP_THOUS_ON,	"E3ON")
	NILIC(OP_DATEDMY,	"D.MY")
	NILIC(OP_DATEMDY,	"M.DY")
	NILIC(OP_DATEYMD,	"Y.MD")
	NILIC(OP_JG1752,	"JG1752")
	NILIC(OP_JG1582,	"JG1582")
	NILIC(OP_12HR,		"12H")
	NILIC(OP_24HR,		"24H")
	NILIC(OP_STK4,		"SSIZE4")
	NILIC(OP_STK8,		"SSIZE8")
	NILIC(OP_FRACT,		"FRACT")
	RARGCMD(RARG_ROUNDING,	"RM")
	NILIC(OP_SLOW,		"SLOW")
	NILIC(OP_FAST,		"FAST")

	NILIC(OP_SETEUR,	"SETEUR")
	NILIC(OP_SETUK,		"SETUK")
	NILIC(OP_SETUSA,	"SETUSA")
	NILIC(OP_SETIND,	"SETIND")
	NILIC(OP_SETCHN,	"SETCHN")
};

static s_opcode alpha_catalogue[] = {
	NILIC(OP_CLALL,		"CLALL")
	NILIC(OP_CLREG,		"CLREG")
	NILIC(OP_ALPHADATE,	"\240DATE")
	NILIC(OP_ALPHADAY,	"\240DAY")
	RARGCMD(RARG_AIP,	"\240IP")
	NILIC(OP_ALPHALEN,	"\240LENG")
	NILIC(OP_ALPHAMONTH,	"\240MONTH")
	RARGCMD(RARG_AREG,	"\240RC#")
	NILIC(OP_RESET,		"RESET")
	RARGCMD(RARG_ALRL,	"\240RL")
	RARGCMD(RARG_ALRR,	"\240RR")
	RARGCMD(RARG_ALSL,	"\240SL")
	RARGCMD(RARG_ALSR,	"\240SR")
	NILIC(OP_ALPHATIME,	"\240TIME")
	NILIC(OP_VERSION,	"VERS")
};

static s_opcode conv_catalogue[] = {
	CONV(OP_CM_INCH,	0, "cm->inch")
	CONV(OP_CM_INCH,	1, "inch->cm")
	CONV(OP_G_OZ,		0, "g->oz")
	CONV(OP_G_OZ,		1, "oz->g")
	CONV(OP_G_TOZ,		0, "g->tr oz")
	CONV(OP_G_TOZ,		1, "tr oz->g")
	CONV(OP_HA_ACRE,	0, "ha->acre")
	CONV(OP_HA_ACRE,	1, "acre->ha")
	CONV(OP_J_BTU,		0, "J->Btu")
	CONV(OP_J_BTU,		1, "Btu->J")
	CONV(OP_J_CAL,		0, "J->Cal")
	CONV(OP_J_CAL,		1, "Cal->J")
	CONV(OP_J_kWh,		0, "J->kW.h")
	CONV(OP_J_kWh,		1, "kW.h->J")
	CONV(OP_KG_LBM, 	0, "kg->lbm")
	CONV(OP_KG_STONE,	1, "stone->kg")
	CONV(OP_KG_STONE,	0, "kg->stone")
	CONV(OP_KG_LBM, 	1, "lb->kg")
	CONV(OP_KG_CWT,		0, "kg->cwt")
	CONV(OP_KG_CWT,		1, "cwt->kg")
	CONV(OP_KG_SHCWT,	0, "kg->s.cwt")
	CONV(OP_KG_SHCWT,	1, "s.cwt->kg")
	CONV(OP_KM_AU,		0, "km->AU")
	CONV(OP_KM_AU,		1, "AU->km")
	CONV(OP_KM_LY,		0, "km->l.y.")
	CONV(OP_KM_LY,		1, "l.y.->km")
	CONV(OP_KM_MILE,	0, "km->mile")
	CONV(OP_KM_MILE,	1, "mile->km")
	CONV(OP_KM_NMI,		0, "km->nmile")
	CONV(OP_KM_NMI,		1, "nmile->km")
	CONV(OP_KM_PC,		0, "km->pc")
	CONV(OP_KM_PC,		1, "pc->km")
	CONV(OP_L_CUBFT,	0, "L->cft")
	CONV(OP_L_CUBFT,	1, "cft->L")
	CONV(OP_L_GALUK,	0, "L->galUK")
	CONV(OP_L_GALUK,	1, "galUK->L")
	CONV(OP_L_GALUS,	0, "L->galUS")
	CONV(OP_L_GALUS,	1, "galUS->L")
	CONV(OP_ML_FLOZUK,	0, "mL->flozUK")
	CONV(OP_ML_FLOZUK,	1, "flozUK->mL")
	CONV(OP_ML_FLOZUS,	0, "mL->flozUS")
	CONV(OP_ML_FLOZUS,	1, "flozUS->mL")
	CONV(OP_M_FATHOM,	0, "m->fathom")
	CONV(OP_M_FATHOM,	1, "fathom->m")
	CONV(OP_M_FEET,		0, "m->feet")
	CONV(OP_M_FEET,		1, "feet->m")
	CONV(OP_M_YARD,		0, "m->yard")
	CONV(OP_M_YARD,		1, "yard->m")
	CONV(OP_N_LBF,		0, "N->lbf")
	CONV(OP_N_LBF,		1, "lbf->N")
	CONV(OP_Pa_ATM,		0, "Pa->ATM")
	CONV(OP_Pa_ATM,		1, "atm->Pa")
	CONV(OP_Pa_inhg,	0, "Pa->inHg")
	CONV(OP_Pa_inhg,	1, "inhg->Pa")
	CONV(OP_Pa_bar,		0, "Pa->bar")
	CONV(OP_Pa_bar,		1, "bar->Pa")
	CONV(OP_Pa_mmHg,	0, "Pa->mmHg")
	CONV(OP_Pa_mmHg,	1, "mmHg->Pa")
	CONV(OP_Pa_psi,		0, "Pa->psi")
	CONV(OP_Pa_psi,		1, "psi->Pa")
	CONV(OP_Pa_torr,	0, "Pa->torr")
	CONV(OP_Pa_torr,	1, "torr->Pa")
	CONV(OP_T_SHTON,	0, "t->sh.ton")
	CONV(OP_T_SHTON,	1, "sh.ton->t")
	CONV(OP_T_TON,		0, "t->ton")
	CONV(OP_T_TON,		1, "ton->t")
	CONV(OP_W_HP,		0, "W->PS(HP)")
	CONV(OP_W_HP,		1, "PS(HP)->W")
	CONV(OP_W_HPUK,		0, "W->bhp")
	CONV(OP_W_HPUK,		1, "bhp->W")
	CONV(OP_W_HPe,		0, "W->HPe")
	CONV(OP_W_HPe,		1, "HPe->W")
	CONV(OP_W_HP550,	0, "W->hp")
	CONV(OP_W_HP550,	1, "hp->W")
	MON(OP_AR_DB,		   "AR->dB")
	MON(OP_DB_AR,		   "dB->AR")
	MON(OP_DB_PR,		   "dB->PR")
	MON(OP_DEGC_F,		   "degC->degF")
	MON(OP_DEGF_C,		   "degF->degC")
	MON(OP_PR_DB,		   "PR->dB")
	MON(OP_DEG2RAD,		   "D\015R")
	MON(OP_RAD2DEG,		   "R\015D")
	MON(OP_DEG2GRD,		   "D\015G")
	MON(OP_GRD2DEG,		   "G\015D")
	MON(OP_RAD2GRD,		   "R\015G")
	MON(OP_GRD2RAD,		   "G\015R")
//	CONV(OP_M_SQUARE,	0, "m^2->square")
//	CONV(OP_M_SQUARE,	1, "suare->m^2")
//	CONV(OP_M_PERCH,	0, "m^2->perch")
//	CONV(OP_M_PERCH,	1, "perch->m^2")
};


/* The alpha mode menus to access all the weird characters */
static unsigned char alpha_symbols[] = {
	',',	'"',	'#',	'`',	'*',	':',
	';',	'@',	'\'',	'_',	'~',
	'\216',	'\256'				// Sol, Terra
};

static unsigned char alpha_compares[] = {
	'<',	'\011',	'=',	'\012',	'>',
	'[',	']',	'{',	'}',	'\035',
};

static unsigned char alpha_arrows[] = {
	015,	016,				// left arrow, right arrow
	017,	020,				// up arrow, down arrow,
	004,	0177,				// integral, up/down arrow
	'^',	0237,				// ^, infinity
};

static unsigned char alpha_superscripts[] = {
	0235,	0232,				// ^-1, ^2
	0234,					// ^x
	0005,					// degree
	031,	001,				// x-hat, x-bar
	032,	002,				// y-hat, y-bar
	024,					// complex prefix
};

static unsigned char alpha_subscripts[] = {
	0270,	0271,	0272,			// sub-0, sub-1, sub-2
	0327,	0230,	0273,			// sub-A, sub-B, sub-c
	0274,	0367,	0033,			// sub-e, sub-k, sub-n
	0275,	0276,	0277,			// sub-n, sub-p, sub-u
	0231,	0233,				// sub-mu, sub-infinity
};

static unsigned char alpha_letters_upper[] = {
	0300, 0301, 0302, 0303, 0304,		// A
	0305, 0306, 0307,			// C
	0310, 0311, 0312, 0313,			// E
	0314, 0315, 0316, 0317,			// I
	0320,					// N
	0321, 0322, 0323, 0324, 0025,		// O
	0325,					// R
	0326,					// S
	0330, 0331, 0332, 0333, 0334,		// U
	0335, 0336,				// Y
	0337,					// Z
};
static unsigned char alpha_letters_lower[] = {
	0340, 0341, 0342, 0343, 0344,		// A
	0345, 0346, 0347,			// C
	0350, 0351, 0352, 0353,			// E
	0236,					// h-bar
	0354, 0355, 0356, 0357,			// I
	0360,					// N
	0361, 0362, 0363, 0364, 0026,		// O
	0365,					// R
	0366, 0030,				// S
	0370, 0371, 0372, 0373, 0374,		// U
	0375, 0376,				// Y
	0377,					// Z
};


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charmap.c"
#include "commands.c"
#include "string.c"
#include "prt.c"
#include "consts.c"


#if defined(WIN32) && !defined(__GNUC__)
// Visual C under windows will link
#include "decNumber.h"
#include "decimal64.h"
#else
#include "decNumber.c"
#include "decContext.c"
#include "decimal64.c"
#endif


static const char *gpl[] = {
	"This file is part of 34S.",
	"",
	"34S is free software: you can redistribute it and/or modify",
	"it under the terms of the GNU General Public License as published by",
	"the Free Software Foundation, either version 3 of the License, or",
	"(at your option) any later version.",
	"",
	"34S is distributed in the hope that it will be useful,",
	"but WITHOUT ANY WARRANTY; without even the implied warranty of",
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
	"GNU General Public License for more details.",
	"",
	"You should have received a copy of the GNU General Public License",
	"along with 34S.  If not, see <http://www.gnu.org/licenses/>.",
	"",
	"",
	"This file is automatically generated.  Changes will not be preserved.",
	NULL
};

static void gpl_text(const char *start, const char *middle, const char *end) {
	int i;

	for (i=0; gpl[i] != NULL; i++)
		printf("%s%s\n", i==0?start:middle, gpl[i]);
	printf("%s\n\n", end);
}


static void unpack(const char *b, int *u) {
	while (*b != 0 && *b != ' ') {
		*u++ = remap_chars(0xff & *b++);
	}
	*u = -1;
}

static int compare_cat(const void *v1, const void *v2) {
	const s_opcode c1 = *(const s_opcode *)v1;
	const s_opcode c2 = *(const s_opcode *)v2;
	char b1[16], b2[16];
	int u1[16], u2[16];
	const char *p1, *p2;
	int i;
	int p1c = 0, p2c = 0;

	for (i=0; i<16; i++)
		b1[i] = b2[i] = 0;
	p1 = prt(c1, b1);
	p2 = prt(c2, b2);

	if (*p1 == COMPLEX_PREFIX) { p1++; p1c = 1; }
	if (*p2 == COMPLEX_PREFIX) { p2++; p2c = 1; }

	unpack(p1, u1);
	unpack(p2, u2);

	for (i=0; i<16; i++) {
		if (u1[i] < u2[i]) return -1;
		else if (u1[i] > u2[i]) return 1;
		else if (u1[i] == -1) break;
	}
	if (p1c) return 1;
	if (p2c) return -1;
	return 0;
}

static void emit_catalogue(const char *name, s_opcode cat[], int num_cat) {
	int i;

	qsort(cat, num_cat, sizeof(s_opcode), &compare_cat);

	printf("static const s_opcode %s[] = {", name);
	for (i=0; i<num_cat; i++)
		printf("%s0x%04x,", (i%6) == 0?"\n\t":" ", cat[i] & 0xffff);
	printf("\n};\n\n");
}


static void emit_alpha(const char *name, unsigned char cat[], int num_cat) {
	int i, j;
	unsigned int c, c2[1000];

	for (i=0; i<num_cat; i++)
		c2[i] = cat[i];

	for (i=0; i<num_cat; i++) {
		unsigned int min = 0xffffff;
		int mj = -1;
		for (j=0; j<num_cat; j++) {
			if (c2[j] == 0xffffff)
				continue;
			c = remap_chars(c2[j]);
			if (c < min) {
				min = c;
				mj = j;
			}
		}
		cat[i] = c2[mj];
		c2[mj] = 0xffffff;
	}

	//qsort(cat, num_cat, 1, &alpha_compare);

	printf("static const char %s[] = {", name);
	for (i=0; i<num_cat; i++)
		printf("%s0%03o,", (i%8) == 0?"\n\t":" ", cat[i] & 0xff);
	printf("\n};\n\n");
}

#include "pretty.c"

#define CAT(n)		emit_catalogue(#n , n, sizeof(n) / sizeof(s_opcode))
#define ALPHA(n)	emit_alpha(#n , n, sizeof(n))

int main(int argc, char *argv[]) {
	gpl_text("/* ", " * ", " */");

	CAT(catalogue);
	CAT(program_xfcn);
	CAT(cplx_catalogue);
	CAT(stats_catalogue);
	CAT(prob_catalogue);
	CAT(int_catalogue);
	CAT(test_catalogue);
	CAT(prog_catalogue);
	CAT(mode_catalogue);
	CAT(alpha_catalogue);
	CAT(conv_catalogue);
#ifdef INCLUDE_INTERNAL_CATALOGUE
	CAT(internal_catalogue);
#endif

	ALPHA(alpha_symbols);
	ALPHA(alpha_compares);
	ALPHA(alpha_arrows);
	ALPHA(alpha_superscripts);
	ALPHA(alpha_subscripts);
	ALPHA(alpha_letters_upper);
	ALPHA(alpha_letters_lower);

	dump_opcodes(stderr);

	return 0;
}

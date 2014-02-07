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
#define NOCURSES 1
// #undef REALBUILD

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
#define RARGCMD(op, n)		RARG_BASEOP(op),
#define CONV(n, d, name)	RARG_BASEOP(RARG_CONV) + (n)*2 + (d),

static s_opcode program_xfcn[] = {
#ifdef INCLUDE_MOD41
	DYA(OP_MOD41,		"MOD")
#endif
#ifdef INCLUDE_INTEGER_DIVIDE
	DYA(OP_IDIV,		"IDIV")
#endif
	MON(OP__1POW,		"(-1)^x")
	DYA(OP_PERMG,		"%+MG")
	DYA(OP_MARGIN,		"%MG")
	TRI(OP_PERMRR,		"%MRR")
	MON(OP_PERTOT,		"%T")
	MON(OP_sigper,		"%\221")
	DYA(OP_ATAN2,		"ANGLE")
	MON(OP_CEIL,		"CEIL")
	MON(OP_CUBE,		"x[^3]")
	MON(OP_CUBERT,		"[^3][sqrt]")
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
	DYA(OP_HMSADD,		"H.MS+")
	DYA(OP_HMSSUB,		"H.MS-")
	TRI(OP_BETAI,		"I\241")
	DYA(OP_GAMMAg,		"[gamma][sub-x][sub-y]")
	DYA(OP_GAMMAG,		"[GAMMA][sub-x][sub-y]")
	DYA(OP_GAMMAP,		"I[GAMMA][sub-p]")
	DYA(OP_GAMMAQ,		"I[GAMMA][sub-q]")
	MON(OP_J2D,		"J\015D")
	DYA(OP_LCM,		"LCM")
	MON(OP_LN1P,		"LN1P")
	DYA(OP_LNBETA,		"LN\241")
	MON(OP_LNGAMMA,		"LN\202")
	DYA(OP_MAX,		"MAX")
	DYA(OP_MIN,		"MIN")
	DYA(OP_LNAND,		"NAND")
	DYA(OP_LNOR,		"NOR")
	MON(OP_ROUND,		"ROUNDI")
	RARGCMD(RARG_ROUND,	"RSD")
	RARGCMD(RARG_ROUND_DEC,	"RDP")
	MON(OP_SIGN,		"SIGN")
	MON(OP_SINC,		"SINC")
	RARGCMD(RARG_SLD,	"SDL")
	RARGCMD(RARG_SRD,	"SDR")
	NILIC(OP_VERSION,	"VERS")
	NILIC(OP_WHO,		"WHO")
	MON(OP_LAMW,		"W\276")
	MON(OP_INVW,		"W\235")
	MON(OP_LAMW1,		"W\033")
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
	NILIC(OP_XTOALPHA,	"x->\240")
	NILIC(OP_ALPHATOX,	"\240->x")
	DYA(OP_BETA,		"\241")
	MON(OP_GAMMA,		"\202")
	DYA(OP_DTDIF,		"\203DAYS")
	DYA(OP_AGM,		"AGM")
	MON(OP_DEG2,		"DED\015")
	MON(OP_RAD2,		"RAD\015")
	MON(OP_GRAD2,		"GRAD\015")
#ifdef INCLUDE_MULADD
	TRI(OP_MULADD,		"\034+")
#endif
#ifdef INCLUDE_EASTER
	MON(OP_EASTER,		"EASTER")
#endif
	MON(OP_ZETA,		"\245")
	MON(OP_Bn,		"B[sub-n]")
	MON(OP_BnS,		"B[sub-n][super-*]")
	NILIC(OP_QUAD,		"QUAD")
	NILIC(OP_NEXTPRIME,	"NEXTP")

	MON(OP_DATE_DAY,	"DAY")
	MON(OP_DATE_MONTH,	"MONTH")
	MON(OP_DATE_YEAR,	"YEAR")
	TRI(OP_TO_DATE,		"\015DATE")
	NILIC(OP_DATE_TO,	"DATE\015")
	NILIC(OP_DATE,		"DATE")
	NILIC(OP_TIME,		"TIME")
	DYA(OP_LEGENDRE_PN,	"P\275")
	DYA(OP_CHEBYCHEV_TN,	"T\275")
	DYA(OP_CHEBYCHEV_UN,	"U\275")
	DYA(OP_LAGUERRE,	"L\275")
	TRI(OP_GEN_LAGUERRE,	"L\275\240")
	DYA(OP_HERMITE_HE,	"HE\275")
	DYA(OP_HERMITE_H,	"H\275")
	NILIC(OP_VOLTAGE,	"BATT")
#ifdef INCLUDE_FLASH_RECALL
	RARGCMD(RARG_FLRCL, 	"RCF")
#endif
	RARGCMD(RARG_iRCL,	"iRCL")
	RARGCMD(RARG_sRCL,	"sRCL")
#ifndef INCLUDE_INTERNAL_CATALOGUE
	RARGCMD(RARG_dRCL,	"dRCL")
#endif
#ifdef INCLUDE_MANTISSA
	MON(OP_MANTISSA,	"MANT")
	MON(OP_EXPONENT,	"EXPT")
	MON(OP_ULP,		"ULP")
	DYA(OP_NEIGHBOUR,	"NEIGHB")
#endif
#ifdef INCLUDE_XROOT
	DYA(OP_XROOT,		"\234\003y")
#endif
#ifdef INCLUDE_GUDERMANNIAN
	MON(OP_GUDER,		"gd")
	MON(OP_INVGUD,		"gd[^-1]")
#endif
#ifdef INCLUDE_XROM_BESSEL
	DYA(OP_BESJN,		"Jn")
	DYA(OP_BESIN,		"In")
	DYA(OP_BESYN,		"Yn")
	DYA(OP_BESKN,		"Kn")
#endif
#ifdef INCLUDE_XROM_DIGAMMA
	MON(OP_DIGAMMA,		"[PSI]")
#endif


	/* Integer mode commands */
	RARGCMD(RARG_ASR,	"ASR")
	RARGCMD(RARG_CB,	"CB")
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
	RARGCMD(RARG_SL,	"SL")
	RARGCMD(RARG_SR,	"SR")
#ifdef INCLUDE_INT_MODULO_OPS
	TRI(OP_MULMOD,		"mulmod")
	TRI(OP_EXPMOD,		"powmod")
#endif
};


static s_opcode catalogue[] = {
#ifdef INCLUDE_MOD41
	DYA(OP_MOD41,		"MOD")
#endif
#ifdef INCLUDE_INTEGER_DIVIDE
	DYA(OP_IDIV,		"IDIV")
#endif
	MON(OP__1POW,		"(-1)^x")
	DYA(OP_PERMG,		"%+MG")
	DYA(OP_MARGIN,		"%MG")
	TRI(OP_PERMRR,		"%MRR")
	MON(OP_PERTOT,		"%T")
	MON(OP_sigper,		"%\221")
	DYA(OP_ATAN2,		"ANGLE")
	MON(OP_CEIL,		"CEIL")
	MON(OP_CUBE,		"x[^3]")
	MON(OP_CUBERT,		"[^3][sqrt]")
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
	DYA(OP_HMSADD,		"H.MS+")
	DYA(OP_HMSSUB,		"H.MS-")
	TRI(OP_BETAI,		"I\241")
	DYA(OP_GAMMAg,		"[gamma][sub-x][sub-y]")
	DYA(OP_GAMMAG,		"[GAMMA][sub-x][sub-y]")
	DYA(OP_GAMMAP,		"I[GAMMA][sub-p]")
	DYA(OP_GAMMAQ,		"I[GAMMA][sub-q]")
	MON(OP_J2D,		"J\015D")
	DYA(OP_LCM,		"LCM")
	MON(OP_LN1P,		"LN1P")
	DYA(OP_LNBETA,		"LN\241")
	MON(OP_LNGAMMA,		"LN\202")
	DYA(OP_MAX,		"MAX")
	DYA(OP_MIN,		"MIN")
	DYA(OP_LNAND,		"NAND")
	DYA(OP_LNOR,		"NOR")
	MON(OP_ROUND,		"ROUNDI")
	RARGCMD(RARG_ROUND,	"RSD")
	RARGCMD(RARG_ROUND_DEC,	"RDP")
	MON(OP_SIGN,		"SIGN")
	MON(OP_SINC,		"SINC")
	RARGCMD(RARG_SLD,	"SDL")
	RARGCMD(RARG_SRD,	"SDR")
	NILIC(OP_VERSION,	"VERS")
	NILIC(OP_WHO,		"WHO")
	MON(OP_LAMW,		"W\276")
	MON(OP_INVW,		"W\235")
	MON(OP_LAMW1,		"W\033")
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
	NILIC(OP_XTOALPHA,	"x->\240")
	NILIC(OP_ALPHATOX,	"\240->x")
	DYA(OP_BETA,		"\241")
	MON(OP_GAMMA,		"\202")
	DYA(OP_DTDIF,		"\203DAYS")
	DYA(OP_AGM,		"AGM")
	MON(OP_DEG2,		"DED\015")
	MON(OP_RAD2,		"RAD\015")
	MON(OP_GRAD2,		"GRAD\015")
#ifdef INCLUDE_MULADD
	TRI(OP_MULADD,		"\034+")
#endif
#ifdef INCLUDE_EASTER
	MON(OP_EASTER,		"EASTER")
#endif
	MON(OP_ZETA,		"\245")
	MON(OP_Bn,		"B[sub-n]")
	MON(OP_BnS,		"B[sub-n][super-*]")
	NILIC(OP_QUAD,		"QUAD")
	NILIC(OP_NEXTPRIME,	"NEXTP")

	MON(OP_DATE_DAY,	"DAY")
	MON(OP_DATE_MONTH,	"MONTH")
	MON(OP_DATE_YEAR,	"YEAR")
	TRI(OP_TO_DATE,		"\015DATE")
	NILIC(OP_DATE_TO,	"DATE\015")

	DYA(OP_LEGENDRE_PN,	"P\275")
	DYA(OP_CHEBYCHEV_TN,	"T\275")
	DYA(OP_CHEBYCHEV_UN,	"U\275")
	DYA(OP_LAGUERRE,	"L\275")
	TRI(OP_GEN_LAGUERRE,	"L\275\240")
	DYA(OP_HERMITE_HE,	"HE\275")
	DYA(OP_HERMITE_H,	"H\275")
	NILIC(OP_VOLTAGE,	"BATT")
#ifdef INCLUDE_FLASH_RECALL
	RARGCMD(RARG_FLRCL,	"RCF")
#endif
	RARGCMD(RARG_iRCL,	"iRCL")
	RARGCMD(RARG_sRCL,	"sRCL")
#ifndef INCLUDE_INTERNAL_CATALOGUE
	RARGCMD(RARG_dRCL,	"dRCL")
#endif

#ifdef INCLUDE_MANTISSA
	MON(OP_MANTISSA,	"MANT")
	MON(OP_EXPONENT,	"EXPT")
	MON(OP_ULP,		"ULP")
	DYA(OP_NEIGHBOUR,	"NEIGHB")
#endif
#ifdef INCLUDE_XROOT
	DYA(OP_XROOT,		"\234\003y")
#endif
#ifdef INCLUDE_GUDERMANNIAN
	MON(OP_GUDER,		"gd")
	MON(OP_INVGUD,		"gd[^-1]")
#endif
#ifdef INCLUDE_XROM_BESSEL
	DYA(OP_BESJN,		"Jn")
	DYA(OP_BESIN,		"In")
	DYA(OP_BESYN,		"Yn")
	DYA(OP_BESKN,		"Kn")
#endif
#ifdef INCLUDE_XROM_DIGAMMA
	MON(OP_DIGAMMA,		"[PSI]")
#endif

	NILIC(OP_DATE,		"DATE")
	NILIC(OP_TIME,		"TIME")
#ifdef INCLUDE_STOPWATCH
	NILIC(OP_STOPWATCH,	"STOPW")
#endif
};

static s_opcode cplx_catalogue[] = {
#ifdef INCLUDE_INTEGER_DIVIDE
	CDYA(OP_IDIV,		"IDIV")
#endif
	CMON(OP__1POW,		"(-1)^x")
	CMON(OP_CCONJ,		"CONJ")
	CMON(OP_CUBE,		"x[^3]")
	CMON(OP_CUBERT,		"[^3][sqrt]")
	NILIC(OP_DROPXY,	"DROP")
	CMON(OP_EXPM1,		"e^x-1")
	CMON(OP_FIB,		"FIB")
	CMON(OP_LN1P,		"LN1P")
	CDYA(OP_LNBETA,		"LN\241")
	CMON(OP_LNGAMMA,	"LN\202")
	CMON(OP_SIGN,		"SIGN")
	CMON(OP_SINC,		"SINC")
	CMON(OP_LAMW,		"W\276")
	CMON(OP_INVW,		"W\235")
	CDYA(OP_BETA,		"\241")
	CMON(OP_GAMMA,		"\202")
	CDYA(OP_AGM,		"AGM")
#ifdef INCLUDE_FLASH_RECALL
	RARGCMD(RARG_FLCRCL, 	"\024RCF")
#endif
#ifdef INCLUDE_XROOT
	CDYA(OP_XROOT,		"\234\003y")
#endif
#ifdef INCLUDE_GUDERMANNIAN
	CMON(OP_GUDER,		"gd")
	CMON(OP_INVGUD,		"gd[^-1]")
#endif
#ifdef INCLUDE_XROM_BESSEL
	CDYA(OP_BESJN,		"Jn")
	CDYA(OP_BESIN,		"In")
	CDYA(OP_BESYN,		"Yn")
	CDYA(OP_BESKN,		"Kn")
#endif
#ifdef INCLUDE_XROM_DIGAMMA
	CMON(OP_DIGAMMA,	"[PSI]")
#endif
	NILIC(OP_DOTPROD,	"DOT")
	NILIC(OP_CROSSPROD,	"CROSS")
#ifndef INCLUDE_INTERNAL_CATALOGUE
#ifdef INCLUDE_INDIRECT_CONSTS
	RARGCMD(RARG_IND_CONST_CMPLX, "\024CNST")
#endif
#endif
};

static s_opcode sums_catalogue[] = {
	NILIC(OP_sigmaN,	"n\221")
	NILIC(OP_sigmalnXlnX,	"\221ln\232X")
	NILIC(OP_sigmalnYlnY,	"\221ln\232Y")
	NILIC(OP_sigmalnX,	"\221lnX")
	NILIC(OP_sigmalnXlnY,	"\221lnXY")
	NILIC(OP_sigmalnY,	"\221lnY")
	NILIC(OP_sigmaX,	"\221x")
	NILIC(OP_sigmaX2,	"\221x\232")
	NILIC(OP_sigmaXlnY,	"\221XlnY")
	NILIC(OP_sigmaXY,	"\221xy")
	NILIC(OP_sigmaX2Y,	"\221x\232y")
	NILIC(OP_sigmaY,	"\221y")
	NILIC(OP_sigmaY2,	"\221y\232")
	NILIC(OP_sigmaYlnX,	"\221YlnX")
};

static s_opcode stats_catalogue[] = {
	MON(OP_sigper,		"%\221")
	NILIC(OP_STORANDOM,	"SEED")
	NILIC(OP_statSErr,	"SERR")
	NILIC(OP_RCLSIGMA,	"SUM")
	NILIC(OP_statWMEAN,	"\001w")
	NILIC(OP_statGMEAN,	"\001g")
	NILIC(OP_statGSErr,	"\244m")
	NILIC(OP_statGS,	"\244")
	NILIC(OP_statGSigma,	"\244\276")
	NILIC(OP_statWSErr,	"SERRw")
	NILIC(OP_statWS,	"sw")
	NILIC(OP_statWSigma,	"\244w")
	NILIC(OP_statSigma,	"\261")
	NILIC(OP_statCOV,	"COV")
	NILIC(OP_statLR,	"L.R.")
	NILIC(OP_statSxy,	"sxy")
	MON(OP_xhat,		"\031")
};

static s_opcode prob_catalogue[] = {
	MON(OP_pdf_B,		"Binom\276")
	MON(OP_cdf_B,		"B(n)")
	MON(OP_qf_B,		"B\235(p)")
	MON(OP_pdf_C,		"Cap(x)")
	MON(OP_cdf_C,		"Ca(x)")
	MON(OP_qf_C,		"Ca\235(p)")
	MON(OP_pdf_EXP,		"Expon[sub-p]")
	MON(OP_cdf_EXP,		"Expon")
	MON(OP_qf_EXP,		"Expon[^-1]")
	MON(OP_pdf_F,		"F\276(x)")
	MON(OP_cdf_F,		"F(x)")
	MON(OP_qf_F,		"F/235(p)")
	MON(OP_pdf_G,		"Geom\276")
	MON(OP_cdf_G,		"Geom")
	MON(OP_qf_G,		"Geom/235")
	MON(OP_pdf_LG,		"Lgp(x)")
	MON(OP_cdf_LG,		"Lg(x)")
	MON(OP_qf_LG,		"Lg\235(p)")
	MON(OP_pdf_LN,		"LgNrm(x)")
	MON(OP_cdf_LN,		"LgNrm(x)")
	MON(OP_qf_LN,		"LgNrm\235(p)")
	MON(OP_pdf_N,		"Normlp")
	MON(OP_cdf_N,		"Norml")
	MON(OP_qf_N,		"Norml\235")
	MON(OP_pdf_Plam,	"Pois[lambda]\276")
	MON(OP_cdf_Plam,	"Pois[lambda]")
	MON(OP_qf_Plam,		"Pois[lambda]/235")
	MON(OP_pdf_P,		"Poiss\276")
	MON(OP_cdf_P,		"Poiss")
	MON(OP_qf_P,		"Poiss/235")
	MON(OP_pdf_T,		"t\276(x)")
	MON(OP_cdf_T,		"t(x)")
	MON(OP_qf_T,		"t/235(p)")
	MON(OP_pdf_WB,		"Weibl\276")
	MON(OP_cdf_WB,		"Weibl")
	MON(OP_qf_WB,		"Weibl/235")
	MON(OP_pdf_chi2,	"\225\232\276")
	MON(OP_cdf_chi2,	"\225\232")
	MON(OP_qf_chi2,		"\225\232INV")
	MON(OP_pdf_Q,		"\224p(x)")
#ifdef INCLUDE_CDFU
	MON(OP_cdfu_Q,		"\224\277(x)")
	MON(OP_cdfu_chi2,	"\225\232\277")
	MON(OP_cdfu_T,		"t\277(x)")
	MON(OP_cdfu_F,		"F\277(x)")
	MON(OP_cdfu_WB,		"Weibl\277")
	MON(OP_cdfu_EXP,	"Expon\277")
	MON(OP_cdfu_B,		"Binom\277")
	MON(OP_cdfu_Plam,	"Pois\252\277")
	MON(OP_cdfu_P,		"Poiss\277")
	MON(OP_cdfu_G,		"Geom\277")
	MON(OP_cdfu_N,		"Norml\277")
	MON(OP_cdfu_LN,		"LgNrm\277")
	MON(OP_cdfu_LG,		"Logis\277")
	MON(OP_cdfu_C,		"Cauch\277")
#endif
};

static s_opcode int_catalogue[] = {
#ifdef INCLUDE_MOD41
	DYA(OP_MOD41,		"MOD")
#endif
#ifdef INCLUDE_INTEGER_DIVIDE
	DYA(OP_IDIV,		"IDIV")
#endif
	MON(OP_FLOOR,		"FLOOR")
	MON(OP_CEIL,		"CEIL")
	MON(OP_ROUND,		"ROUNDI")
	MON(OP__1POW,		"-1^x")
	RARGCMD(RARG_ASR,	"ASR")
	RARGCMD(RARG_CB,	"CB")
	MON(OP_CUBE,		"x[^3]")
	MON(OP_CUBERT,		"[^3][sqrt]")
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
	NILIC(OP_WHO,		"WHO")
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
	NILIC(OP_XTOALPHA,	"x->\240")
	NILIC(OP_ALPHATOX,	"\240->x")
#ifdef INCLUDE_MULADD
	TRI(OP_MULADD,		"\034+")
#endif
#ifdef INCLUDE_FLASH_RECALL
	RARGCMD(RARG_FLRCL, 	"RCF")
#endif
	RARGCMD(RARG_sRCL,	"sRCL")
#ifdef INCLUDE_XROOT
	DYA(OP_XROOT,		"\234\003y")
#endif
#ifdef INCLUDE_MANTISSA
	MON(OP_ULP,		"ULP")
	DYA(OP_NEIGHBOUR,	"NEIGHB")
#endif
	MON(OP_GAMMA,		"\202")
#ifdef INCLUDE_INT_MODULO_OPS
	TRI(OP_MULMOD,		"mulmod")
	TRI(OP_EXPMOD,		"powmod")
#endif
};

static s_opcode matrix_catalogue[] = {
	MON(OP_MAT_DET,		"DET")
	TRI(OP_MAT_LIN_EQN,	"LINEQS")
	TRI(OP_MAT_GADD,	"M+[times]")
	NILIC(OP_MAT_INVERSE,	"M^-1")
	MON(OP_MAT_ALL,		"M-ALL")
	DYA(OP_MAT_COL,		"M-COL")
	MON(OP_MAT_DIAG,	"M-DIAG")
	DYA(OP_MAT_ROW,		"M-ROW")
	TRI(OP_MAT_MUL,		"M[times]")
	DYA(OP_MAT_COPY,	"M.COPY")
	MON(OP_MAT_IJ,		"M.IJ")
	TRI(OP_MAT_REG,		"M.REG")
	MON(OP_MAT_CQ,		"nCOL")
	MON(OP_MAT_RQ,		"nROW")
	MON(OP_MAT_TRN,		"TRANSP")

#ifdef MATRIX_ROWOPS
	NILIC(OP_MAT_ROW_SWAP,	"MROW<>")
	NILIC(OP_MAT_ROW_MUL,	"MROW*")
	NILIC(OP_MAT_ROW_GADD,	"MROW+*")
#endif
#ifdef MATRIX_LU_DECOMP
	MON(OP_MAT_LU,		"M.LU")
#endif
#ifdef SILLY_MATRIX_SUPPORT
	NILIC(OP_MAT_ZERO,	"M.ZERO")
	NILIC(OP_MAT_IDENT,	"M.IDEN")
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
	NILIC(OP_XisInf,	"\237?")
	NILIC(OP_XisSpecial,	"SPEC?")
	RARGCMD(RARG_TEST_APX,	"x~?")
	RARGCMD(RARG_TEST_LT,	"x<?")
	RARGCMD(RARG_TEST_LE,	"x<=?")
	RARGCMD(RARG_TEST_GE,	"x>=?")
	RARGCMD(RARG_TEST_GT,	"x>?")
//	RARGCMD(RARG_TEST_ZAPX,	"cx~?")

	RARGCMD(RARG_LBLP,	"LBL?")
	NILIC(OP_ENTRYP,	"ENTRY?")
	RARGCMD(RARG_KEY,	"KEY?")
	NILIC(OP_TOP,		"TOP?")
	NILIC(OP_ISINT,		"INTM?")
	NILIC(OP_ISFLOAT,	"REALM?")
	NILIC(OP_Xeq_pos0,	"x=+0?")
	NILIC(OP_Xeq_neg0,	"x=-0?")

	NILIC(OP_MAT_CHECK_SQUARE, "M.SQR?")

	NILIC(OP_QUERY_XTAL,	    "XTAL?")
	NILIC(OP_QUERY_PRINT,	    "PRT?")
#if 0
	NILIC(OP_FLASHQ,	"FLASH?")
	NILIC(OP_GETBASE,	"BASE?")
	RARGCMD(RARG_KEYTYPE,	"KTP?")
	NILIC(OP_LOCRQ,		"LocR?")
	NILIC(OP_MEMQ,		"MEM?")
	NILIC(OP_REGSQ,		"REGS?")
	NILIC(OP_ROUNDING,	"RM?")
	NILIC(OP_GETSIGN,	"SMODE?")
	NILIC(OP_STKSIZE,	"SSIZE?")
	NILIC(OP_INTSIZE,	"WSIZE?")
#endif
#ifdef INCLUDE_PLOTTING
	RARGCMD(RARG_PLOT_ISSET,    "gPIX?")
#endif
#ifndef INCLUDE_INTERNAL_CATALOGUE
	NILIC(OP_ISDBL,		"DBL?")
	RARGCMD(RARG_CONVERGED,	"CNVG?")
#endif
};

static s_opcode prog_catalogue[] = {
#if 1
	NILIC(OP_FLASHQ,	"FLASH?")
	NILIC(OP_GETBASE,	"BASE?")
	RARGCMD(RARG_KEYTYPE,	"KTP?")
	NILIC(OP_LOCRQ,		"LocR?")
	NILIC(OP_MEMQ,		"MEM?")
	NILIC(OP_REGSQ,		"REGS?")
	NILIC(OP_ROUNDING,	"RM?")
	NILIC(OP_GETSIGN,	"SMODE?")
	NILIC(OP_STKSIZE,	"SSIZE?")
	NILIC(OP_INTSIZE,	"WSIZE?")
#endif
	RARGCMD(RARG_SWAPY,	"y[<->]")
	RARGCMD(RARG_SWAPZ,	"z[<->]")
	RARGCMD(RARG_SWAPT,	"t[<->]")
	RARGCMD(RARG_STOSTK,	"\015STK")
	RARGCMD(RARG_RCLSTK,	"\016STK")
	NILIC(OP_CLREG,		"CLREG")
	NILIC(OP_CLFLAGS,	"CFALL")
	NILIC(OP_CLSTK,		"CLSTK")
	NILIC(OP_CLRALPHA,	"CLalpha")
	NILIC(OP_CLALL,		"CLALL")
	NILIC(OP_CLPALL,	"CLPALL")
	NILIC(OP_RESET,		"RESET")
	RARGCMD(RARG_DEC,	"DEC")
	NILIC(OP_DROP,		"DROP")
	RARGCMD(RARG_DSL,	"DSL")
	RARGCMD(RARG_DSZ,	"DSZ")
	RARGCMD(RARG_FF,	"FF")
	RARGCMD(RARG_DERIV,	"f'(x)")
	RARGCMD(RARG_2DERIV,	"f\"(x)")
	RARGCMD(RARG_INC,	"INC")
	RARGCMD(RARG_ISE,	"ISE")
	RARGCMD(RARG_ISZ,	"ISZ")
	NILIC(OP_NOP,		"NOP")
	NILIC(OP_PROMPT,	"PROMPT")
	NILIC(OP_REGCLR,	"R.CLR")
	NILIC(OP_REGCOPY,	"R.COPY")
	NILIC(OP_REGSORT,	"R.SORT")
	NILIC(OP_REGSWAP,	"R.SWAP")
	NILIC(OP_TICKS,		"TICKS")
	NILIC(OP_ALPHAOFF,	"\240OFF")
	NILIC(OP_ALPHAON,	"\240ON")
	RARGCMD(RARG_VIEW_REG,	"VW\240+")
	NILIC(OP_VIEWALPHA,	"\240VIEW")
	NILIC(OP_END,		"END")
	RARGCMD(RARG_ERROR,	"ERR")
	RARGCMD(RARG_MESSAGE,	"MSG")
	NILIC(OP_XEQALPHA,	"XEQ\240")
	NILIC(OP_GTOALPHA,	"GTO\240")
	RARGCMD(RARG_ALPHAXEQ,	"\240XEQ")
	RARGCMD(RARG_ALPHAGTO,	"\240GTO")


	/* Serial commands */
	NILIC(OP_SENDP,		"SENDP")
	NILIC(OP_SENDR,		"SENDR")
	NILIC(OP_SENDsigma,	"SEND\221")
	NILIC(OP_SENDA,		"SENDA")
	NILIC(OP_RECV,		"RECV")

#ifdef INCLUDE_USER_IO
	NILIC(OP_SEND1,		"SEND1")
	MON(OP_RECV1,		"RECV1")
	NILIC(OP_SERIAL_OPEN,	"SOPEN")
	NILIC(OP_SERIAL_CLOSE,	"SCLOSE")
	NILIC(OP_ALPHASEND,	"\240SEND")
	NILIC(OP_ALPHARECV,	"\240RECV")
#endif
	NILIC(OP_SAVE,		"SAVE")
	NILIC(OP_LOAD,		"LOAD")
	NILIC(OP_LOADP,		"LOADP")
	NILIC(OP_LOADR,		"LOADR")
	NILIC(OP_LOADsigma,	"LOAD\221")
	NILIC(OP_LOADST,	"LOADSS")
	NILIC(OP_PSTO,		"PSTO")
	NILIC(OP_PRCL,		"PRCL")
#ifdef INCLUDE_FLASH_RECALL
	RARGCMD(RARG_FLRCL, 	"RCF")
#endif
	RARGCMD(RARG_PUTKEY,	"PUTK")

	RARGCMD(RARG_LOCR,	"LocR")
	NILIC(OP_POPLR,		"PopLR")

	/* INFRARED commands */
	RARGCMD(RARG_PRINT_REG,	    "\222r")
	RARGCMD(RARG_PRINT_CMPLX,   "\222\024r[sub-x][sub-y]")
	RARGCMD(RARG_PRINT_BYTE,    "\222#")
	RARGCMD(RARG_PRINT_CHAR,    "\222CHR")
	RARGCMD(RARG_PRINT_TAB,     "\222TAB")
	NILIC(OP_PRINT_ADV,	    "\222ADV")
	NILIC(OP_PRINT_ALPHA,	    "\222\240")
	NILIC(OP_PRINT_ALPHA_NOADV, "\222\240+")
	NILIC(OP_PRINT_ALPHA_JUST,  "\222+\240")
	NILIC(OP_PRINT_PGM,	    "\222PGM")
	NILIC(OP_PRINT_REGS,	    "\222REGS")
	NILIC(OP_PRINT_SIGMA,	    "\222\221")
	NILIC(OP_PRINT_STACK,	    "\222STK")
	NILIC(OP_PRINT_WIDTH,	    "\222WIDTH")
	/* end of INFRARED commands */
#ifdef INCLUDE_PLOTTING
	RARGCMD(RARG_PLOT_INIT,     "gDIM")
	RARGCMD(RARG_PLOT_DIM,      "gDIM?")
	RARGCMD(RARG_PLOT_SETPIX,   "gSET")
	RARGCMD(RARG_PLOT_CLRPIX,   "gCLR")
	RARGCMD(RARG_PLOT_FLIPPIX,  "gFLP")
	RARGCMD(RARG_PLOT_DISPLAY,  "gPLOT")
	RARGCMD(RARG_PLOT_PRINT,    "\222PLOT")     /* INFRARED command */
#endif

#ifndef INCLUDE_INTERNAL_CATALOGUE
	RARGCMD(RARG_CASE,	"CASE")
	RARGCMD(RARG_BACK,	"BACK")
	RARGCMD(RARG_SKIP,	"SKIP")
	NILIC(OP_RTNp1,		"RTN+1")
	RARGCMD(RARG_SHUFFLE,	"[<->]")
#ifdef INCLUDE_RELATIVE_CALLS
	RARGCMD(RARG_BSF,	"BSRF")
	RARGCMD(RARG_BSB,	"BSRB")
#endif
#ifdef INCLUDE_INDIRECT_BRANCHES
	RARGCMD(RARG_iBSF,	"iBSRF")
	RARGCMD(RARG_iBSB,	"iBSRB")
	RARGCMD(RARG_iBACK,	"iBACK")
#endif

#ifdef INCLUDE_INDIRECT_CONSTS
	RARGCMD(RARG_IND_CONST, "CNST")
#endif
#ifdef _DEBUG
	NILIC(OP_DEBUG,		"DBG")
#endif
#endif
};

#ifdef INCLUDE_INTERNAL_CATALOGUE
static s_opcode internal_catalogue[] = {
#if 0
	RARGCMD(RARG_MODE_SET,	"xMSET")
	RARGCMD(RARG_MODE_CLEAR,"xMCLR")
	NILIC(OP_LOADA2D,	"[->]A..D")
	NILIC(OP_SAVEA2D,	"A..D[->]")
#endif
	RARGCMD(RARG_CASE,	"CASE")
	RARGCMD(RARG_BACK,	"BACK")
	RARGCMD(RARG_SKIP,	"SKIP")
	NILIC(OP_RTNp1,		"RTN+1")
	RARGCMD(RARG_SHUFFLE,	"[<->]")
#ifdef INCLUDE_RELATIVE_CALLS
	RARGCMD(RARG_BSF,	"BSRF")
	RARGCMD(RARG_BSB,	"BSRB")
#endif
#ifdef INCLUDE_INDIRECT_BRANCHES
	RARGCMD(RARG_iBSF,	"iBSRF")
	RARGCMD(RARG_iBSB,	"iBSRB")
	RARGCMD(RARG_iBACK,	"iBACK")
#endif
	NILIC(OP_DBLON,		"DBLON")
	NILIC(OP_DBLOFF,	"DBLOFF")
	NILIC(OP_ISDBL,		"DBL?")
	RARGCMD(RARG_dRCL,	"dRCL")

	RARGCMD(RARG_CONVERGED,	"CNVG?")
#ifdef INCLUDE_INDIRECT_CONSTS
	RARGCMD(RARG_IND_CONST, "CNST")
	RARGCMD(RARG_IND_CONST_CMPLX, "\024CNST")
#endif
#ifdef _DEBUG
	NILIC(OP_DEBUG,		"DBG")
#endif
};
#endif

static s_opcode mode_catalogue[] = {
	NILIC(OP_RADCOM,	"RDX,")
	NILIC(OP_RADDOT,	"RDX.")
	NILIC(OP_BEST,		"BestF")
	NILIC(OP_EXPF,		"ExpF")
	NILIC(OP_LINF,		"LinF")
	NILIC(OP_LOGF,		"LogF")
	NILIC(OP_PWRF,		"PowerF")
	NILIC(OP_1COMP,		"1COMPL")
	NILIC(OP_2COMP,		"2COMPL")
	NILIC(OP_SIGNMANT,	"SIGNMT")
	NILIC(OP_UNSIGNED,	"UNSIGN")
	RARGCMD(RARG_WS,	"WSIZE")
	NILIC(OP_LEAD0,		"LZON")
	NILIC(OP_TRIM0,		"LZOFF")
	RARGCMD(RARG_BASE,	"BASE")
	NILIC(OP_DENANY,	"DENANY")
	NILIC(OP_DENFAC,	"DENMFAC")
	NILIC(OP_DENFIX,	"DENFIX")
	NILIC(OP_FRACDENOM,	"DENMAX")
	RARGCMD(RARG_DISP,	"DISP")
	NILIC(OP_THOUS_OFF,	"E3OFF")
	NILIC(OP_THOUS_ON,	"E3ON")
	NILIC(OP_INTSEP_OFF,	"SEPOFF")
	NILIC(OP_INTSEP_ON,	"SEPON")
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
	RARGCMD(RARG_REGS,	"REGS")
	NILIC(OP_SETEUR,	"SETEUR")
	NILIC(OP_SETUK,		"SETUK")
	NILIC(OP_SETUSA,	"SETUSA")
	NILIC(OP_SETIND,	"SETIND")
	NILIC(OP_SETCHN,	"SETCHN")
	NILIC(OP_SETJPN,	"SETJPN")
	RARGCMD(RARG_STOM,	"STOM")
	RARGCMD(RARG_RCLM,	"RCLM")
	NILIC(OP_SETDATE,	"SETDAT")
	NILIC(OP_SETTIME,	"SETTIM")
	/* INFRARED commands */
	RARGCMD(RARG_PMODE,	"PMODE")
	RARGCMD(RARG_PDELAY,	"PDLAY")
	/* end of INFRARED commands */
#ifndef INCLUDE_INTERNAL_CATALOGUE
	NILIC(OP_DBLON,		"DBLON")
	NILIC(OP_DBLOFF,	"DBLOFF")
#endif
};

static s_opcode alpha_catalogue[] = {
	NILIC(OP_ALPHADATE,	"\240DATE")
	NILIC(OP_ALPHADAY,	"\240DAY")
	RARGCMD(RARG_AIP,	"\240IP")
	NILIC(OP_ALPHALEN,	"\240LENG")
	NILIC(OP_ALPHAMONTH,	"\240MONTH")
	RARGCMD(RARG_AREG,	"\240RC#")
	RARGCMD(RARG_ALRL,	"\240RL")
	RARGCMD(RARG_ALRR,	"\240RR")
	RARGCMD(RARG_ALSL,	"\240SL")
	RARGCMD(RARG_ALSR,	"\240SR")
	NILIC(OP_ALPHATIME,	"\240TIME")
	NILIC(OP_XTOALPHA,	"x->\240")
	NILIC(OP_ALPHATOX,	"\240->x")
	NILIC(OP_VERSION,	"VERS")
};

static s_opcode conv_catalogue[] = {
	CONV(OP_CM_INCH,	0, "cm->inch")
	CONV(OP_CM_INCH,	1, "inch->cm")
	CONV(OP_G_OZ,		0, "g->oz")
	CONV(OP_G_OZ,		1, "oz->g")
	CONV(OP_G_TOZ,		0, "g->tr oz")
	CONV(OP_G_TOZ,		1, "tr oz->g")
	CONV(OP_HA_ACREUK,	0, "ha->acreUK")
	CONV(OP_HA_ACREUK,	1, "acreUK->ha")
	CONV(OP_HA_ACREUS,	0, "ha->acreUS")
	CONV(OP_HA_ACREUS,	1, "acreUS->ha")
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
	CONV(OP_M_FEETUS,	0, "m->feetUS")
	CONV(OP_M_FEETUS,	1, "feetUS->m")
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
//	CONV(OP_M_SQUARE,	1, "square->m^2")
//	CONV(OP_M_PERCH,	0, "m^2->perch")
//	CONV(OP_M_PERCH,	1, "perch->m^2")
};


/* The alpha mode menus to access all the weird characters */
static unsigned char alpha_symbols[] = {
	',',	';',	':',	
	'\'',	'"',	'#',	'`',
	'*',	'%',	'@',	'_',
	'~',
	0222,					// print
	'$',	0014,	0036,	0037,		// currency symbols
	0216,	0256				// Sol, Terra
};

static unsigned char alpha_compares[] = {
	'<',	0011,	'=',	0012,	'>',
	'[',	']',	'{',	'}',	0035
};

static unsigned char alpha_arrows[] = {
	0015,	0016,				// left arrow, right arrow
	0017,	0020,				// up arrow, down arrow,
	0003,	0004,				// root, integral
	0177,					// up/down arrow
	'^',	0237				// ^, infinity
};

#ifndef MERGE_SUPERSCRIPTS
static unsigned char alpha_superscripts[] = {
	0235,	0232,	0200,			// ^-1, ^2, ^3
	0234,					// ^x
	0005,					// degree
	0024,					// complex prefix
	0031,	0001,				// x-hat, x-bar
	0032,	0002,				// y-hat, y-bar
	0220,					// ^*
};
#endif

static unsigned char alpha_subscripts[] = {
	0270,	0271,	0272,			// sub-0, sub-1, sub-2
	0327,	0230,	0273,			// sub-A, sub-B, sub-c
        0206,                                   // sub-d
	0274,	0367,	0033,			// sub-e, sub-k, sub-m
	0275,	0276,	0223,	0277,		// sub-n, sub-p, sub-q, sub-u
	0201,	0213,	0214,			// sub-w, sub-x, sub-y
	0231,	0233,				// sub-mu, sub-infinity
	0235,	0232, 0200,			// ^-1, ^2, ^3
	0234,					// ^x
	0220,					// ^*
	0005,					// degree
	0024					// complex prefix
};

// Next two must match in size and 'meaning'
static unsigned char alpha_letters[] = {
	0300, 0301, 0302, 0303, 0304, 0210,	// A
	0305, 0306, 0307,			// C
	0204,					// D-bar
	0310, 0311, 0312, 0313,			// E
	0236,					// h-bar
	0314, 0315, 0316, 0317,			// I
	0320,					// N
	0321, 0322, 0323, 0324, 0025,		// O
	0325,					// R
	0326, 0030,				// S
	0330, 0331, 0332, 0333, 0334,		// U
	0031, 0001,				// x-hat, x-bar
	0032, 0002,				// y-hat, y-bar
	0335, 0336,				// Y
	0337					// Z
};
static unsigned char alpha_letters_lower[] = {
	0340, 0341, 0342, 0343, 0344, 0211,	// A
	0345, 0346, 0347,			// C
	0205,					// d-bar
	0350, 0351, 0352, 0353,			// E
	0236,					// h-bar
	0354, 0355, 0356, 0357,			// I
	0360,					// N
	0361, 0362, 0363, 0364, 0026,		// O
	0365,					// R
	0366, 0030,				// S
	0370, 0371, 0372, 0373, 0374,		// U
	0031, 0001,				// x-hat, x-bar
	0032, 0002,				// y-hat, y-bar
	0375, 0376,				// Y
	0377					// Z
};


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charmap.c"
#include "commands.c"
#include "string.c"
#include "prt.c"
#include "consts.c"
#include "licence.h"


#if defined(WIN32) && !defined(__GNUC__)
// Visual C under windows will link
#include "decNumber.h"
#include "decimal64.h"
#else
#include "decNumber.c"
#include "decContext.c"
#include "decimal64.c"
#endif

static const unsigned char opcode_breaks[KIND_MAX] = {
	NUM_SPECIAL,		// Number of specials
	NUM_NILADIC,		// Number of niladics
	NUM_MONADIC,		// Number of monadics
	NUM_DYADIC,		// Number of dyadics
	NUM_TRIADIC,		// Number of triadics
	NUM_MONADIC,		// Number of complex monadics
	NUM_DYADIC,		// Number of complex dyadics
};

static int total_cat, total_alpha, total_conv;

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
	unsigned short int x;
	unsigned short int opcode_sums[1 + KIND_MAX];
	unsigned char buffer[10000];
	unsigned char *bp = buffer;

	opcode_sums[0] = 0;
	for (i=1; i<=KIND_MAX; i++)
		opcode_sums[i] = opcode_sums[i-1] + opcode_breaks[i-1];

	qsort(cat, num_cat, sizeof(s_opcode), &compare_cat);

	for (i=0; i<num_cat; i++) {
		const s_opcode op = cat[i] & 0xffff;
		if (isRARG(op))
			x = opcode_sums[KIND_MAX] + RARG_CMD(op);
		else
			x = opcode_sums[(int)opKIND(op)] + argKIND(op);
		if ((x & 0x3ff) != x) {
			fprintf(stderr, "Error: opcode overflow: 0x%04x\n", x);
			exit(1);
		}
		switch (i%4) {
		case 0:
			*bp++ = x >> 2;
			*bp = (x & 0x3) << 6;
			break;
		case 1:
			*bp++ |= x >> 4;
			*bp = (x & 0xf) << 4;
			break;
		case 2:
			*bp++ |= x >> 6;
			*bp = (x & 0x3f) << 2;
			break;
		case 3:
			*bp++ |= x >> 8;
			*bp++ = (x & 0xff);
			break;
		}
	}
	if (num_cat % 4 != 0)
		bp++;

	printf("#define SIZE_%s %d\n", name, num_cat);
	printf("static const unsigned char %s[] = {", name);
	for (i=0; buffer + i != bp; i++) {
		printf("%s0x%02x,", (i%6) == 0?"\n\t":" ", buffer[i]);
	}
	printf("\n};\n\n");
       	total_cat += num_cat;
}


// compact ecncoding for conversions in just on byte per entry
static void emit_conv_catalogue(const char *name, s_opcode cat[], int num_cat) {
	int i;

	qsort(cat, num_cat, sizeof(s_opcode), &compare_cat);

	printf("#define SIZE_%s %d\n", name, num_cat);
	printf("static const unsigned char %s[] = {", name);
	for (i=0; i<num_cat; i++) {
		unsigned char c = (unsigned char) cat[i];
		if (opKIND(cat[i]) == KIND_MON)
			c += num_cat;
		printf("%s0x%02x,", (i%6) == 0?"\n\t":" ", c);
	}
	printf("\n};\n\n");
	total_conv += num_cat;
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

	printf("#define SIZE_%s %d\n", name, num_cat);
	printf("static const char %s[] = {", name);
	for (i=0; i<num_cat; i++)
		printf("%s0%03o,", (i%8) == 0?"\n\t":" ", cat[i] & 0xff);
	printf("\n};\n\n");
        total_alpha += num_cat;
}

#include "pretty.c"

#define CAT(n)		emit_catalogue(#n , n, sizeof(n) / sizeof(s_opcode))
#define CONVERSION(n)	emit_conv_catalogue(#n , n, sizeof(n) / sizeof(s_opcode))
#define ALPHA(n)	emit_alpha(#n , n, sizeof(n))

int main(int argc, char *argv[]) {
	int i;

	license(stdout, "/* ", " * ", " */");

	printf("#ifndef CATALOGUES_H_INCLUDED\n"
		"#define CATALOGUES_H_INCLUDED\n\n");

	printf("static const unsigned char opcode_breaks[KIND_MAX] = {\n\t");
	for (i = 0; i < sizeof(opcode_breaks)/sizeof(opcode_breaks[0]); ++i)
		printf("%d, ", opcode_breaks[i]);
	printf("\n};\n\n");

	CAT(catalogue);
	CAT(program_xfcn);
	CAT(cplx_catalogue);
	CAT(stats_catalogue);
	CAT(sums_catalogue);
	CAT(prob_catalogue);
	CAT(int_catalogue);
	CAT(test_catalogue);
	CAT(prog_catalogue);
	CAT(mode_catalogue);
	CAT(alpha_catalogue);
	CONVERSION(conv_catalogue);
	CAT(matrix_catalogue);
#ifdef INCLUDE_INTERNAL_CATALOGUE
	CAT(internal_catalogue);
#endif

	ALPHA(alpha_symbols);
	ALPHA(alpha_compares);
	ALPHA(alpha_arrows);
#ifndef MERGE_SUPERSCRIPTS
	ALPHA(alpha_superscripts);
#endif
	ALPHA(alpha_subscripts);
	ALPHA(alpha_letters);
	ALPHA(alpha_letters_lower);

	fprintf(stderr, "# $Rev$\n" );
	fprintf(stderr, "version=%c%c\n", VERSION_STRING[0], VERSION_STRING[2]);
        fprintf(stderr, "maxsteps=%d\n", NUMPROG_LIMIT);
        fprintf(stderr, "maxlibsteps=%d\n", NUMPROG_FLASH);
	dump_opcodes(stderr, 0);

	printf( "/* Total number of catalogue entries %d\n"
		" * Total number of conversion entries %d\n"
		" * Total number of alpha entries %d\n"
		" */\n", total_cat, total_conv, total_alpha);
	printf("#endif\n");
	return 0;
}

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


#define CST(op, n)		RARG(RARG_CONST, op),
#define CCST(op, n)		RARG(RARG_CONST_CMPLX, op),
#define TRI(op, n)		op | OP_TRI,
#define DYA(op, n)		op | OP_DYA,
#define MON(op, n)		op | OP_MON,
#define CDYA(op, n)		op | OP_CDYA,
#define CMON(op, n)		op | OP_CMON,
#define NILIC(op, n)		op | OP_NIL,
#define RARGCMD(op, n)		OP_RARG | ((op) << RARG_OPSHFT),
#define CONV(n, d, name)	(OP_RARG | ((RARG_CONV) << RARG_OPSHFT)) + (n)*2 + (d),


static const s_opcode catalogue[] = {
#ifdef INCLUDE_SUBFACT
	MON(OP_SUBFACT,		"!n")
#endif
#ifdef INCLUDE_MULADD
	TRI(OP_MULADD,		"\034+")
#endif
	DYA(OP_PERMG,		"%+MG")
	DYA(OP_MARGIN,		"%MG")
	TRI(OP_PERMRR,		"%MRR")
	MON(OP_PERTOT,		"%T")
	MON(OP_sigper,		"%\221")
#ifdef INCLUDE_AGM
	DYA(OP_AGM,		"AGM")
#endif
	DYA(OP_ATAN2,		"ANGLE")
	MON(OP_CEIL,		"CEIL")
	NILIC(OP_CLALL,		"CLALL")
	NILIC(OP_CLREG,		"CLREG")
	NILIC(OP_CLSTK,		"CLSTK")
#ifdef INCLUDE_ELLIPTIC
	DYA(OP_CN,		"CN")
#endif
#ifdef INCLUDE_CUBES
	MON(OP_CUBE,		"CUBE")
	MON(OP_CUBERT,		"CUBERT")
#endif
	MON(OP_D2J,		"D\015J")
	MON(OP_DEG2RAD,		"D\015R")
	NILIC(OP_DATE,		"DATE")
	MON(OP_DOWK,		"DAY")
	DYA(OP_DTADD,		"DAYS+")
			NILIC(OP_2FRAC,		"DECOMP")
#ifdef INCLUDE_ELLIPTIC
	DYA(OP_DN,		"DN")
#endif
#ifdef INCLUDE_EASTER
	MON(OP_EASTER,		"EASTER")
#endif
	MON(OP_ERF,		"ERF")
	MON(OP_EXPM1,		"e^x-1")
	MON(OP_FIB,		"FIB")
	MON(OP_FLOOR,		"FLOOR")
	NILIC(OP_FRACT,		"FRACT")
	DYA(OP_GCD,		"GCD")
	DYA(OP_HMSADD,		"H.MS+")
	DYA(OP_HMSSUB,		"H.MS-")
#ifdef INCLUDE_BESSEL
	DYA(OP_BSIN,		"In")
#endif
	TRI(OP_BETAI,		"I\241")
	DYA(OP_GAMMAP,		"I\202")
	MON(OP_J2D,		"J\015D")
#ifdef INCLUDE_BESSEL
	DYA(OP_BSJN,		"Jn")
	DYA(OP_BSKN,		"Kn")
#endif
	DYA(OP_LCM,		"LCM")
	MON(OP_LN1P,		"LN1P")
	DYA(OP_LNBETA,		"LN\241")
	MON(OP_LNGAMMA,		"LN\202")
#ifdef INCLUDE_MODULAR
	NILIC(OP_MMULTIPLY,   "M\034")
	NILIC(OP_MPLUS, 	      "M+")
	NILIC(OP_MMINUS,      "M-")
	NILIC(OP_MSQ,		"M^2")
#endif
	DYA(OP_MAX,		"MAX")
	DYA(OP_MIN,		"MIN")
	DYA(OP_LNAND,		"NAND")
	DYA(OP_LNOR,		"NOR")
	MON(OP_RAD2DEG,		"R\015D")
	NILIC(OP_RADCOM,	"RDX,")
	NILIC(OP_RADDOT,	"RDX.")
	NILIC(OP_RESET,		"RESET")
	MON(OP_ROUND,		"RNDINT")
	NILIC(OP_SETDATE,	"SETDAT")
	NILIC(OP_SETTIME,	"SETTIM")
	MON(OP_SIGN,		"SIGN")
	MON(OP_SINC,		"SINC")
#ifdef INCLUDE_ELLIPTIC
	DYA(OP_SN,		"SN")
#endif
	NILIC(OP_TIME,		"TIME")
	NILIC(OP_VERSION,	"VERS")
	RARGCMD(RARG_VIEW,	"VIEW")
	MON(OP_LAMW,		"W")
	MON(OP_INVW,		"W\235")
#ifdef INCLUDE_DBLFACT
	MON(OP_DBLFACT,		"x!!")
#endif
	DYA(OP_LXNOR,		"XNOR")
	NILIC(OP_DATEYMD,	"Y.MD")
#ifdef INCLUDE_BESSEL
	DYA(OP_BSYN,		"Yn")
#endif
//	NILIC(OP_ALPHAAPP,	"\240APP")
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
//	NILIC(OP_AVIEW,		"\240VIEW")
	DYA(OP_BETA,		"\241")
	MON(OP_GAMMA,		"\202")
	DYA(OP_DTDIF,		"\203DAYS")
#ifdef INCLUDE_ZETA
	MON(OP_ZETA,		"\245")
#endif
#ifdef INCLUDE_DIGAMMA
	MON(OP_PSI,		"\226")
#endif
};

static const s_opcode cplx_catalogue[] = {
#ifdef INCLUDE_AGM
	CDYA(OP_AGM,		"AGM")
#endif
#ifdef INCLUDE_ELLIPTIC
	CDYA(OP_CN,		"CN")
#endif
	CMON(OP_CCONJ,		"CONJ")
#ifdef INCLUDE_CUBES
	CMON(OP_CUBE,		"CUBE")
	CMON(OP_CUBERT,		"CUBERT")
#endif
#ifdef INCLUDE_ELLIPTIC
	CDYA(OP_DN,		"DN")
#endif
	CMON(OP_EXPM1,		"e^x-1")
	CMON(OP_FIB,		"FIB")
#ifdef COMPLEX_BESSEL
	CDYA(OP_BSIN,		"In")
	CDYA(OP_BSJN,		"Jn")
	CDYA(OP_BSKN,		"Kn")
#endif
	CMON(OP_LN1P,		"LN1P")
	CDYA(OP_LNBETA,		"LN\241")
	CMON(OP_LNGAMMA,	"LN\202")
	CMON(OP_SIGN,		"SIGN")
	CMON(OP_SINC,		"SINC")
#ifdef INCLUDE_ELLIPTIC
	CDYA(OP_SN,		"SN")
#endif
	CMON(OP_LAMW,		"W")
	CMON(OP_INVW,		"W\235")
#ifdef INCLUDE_DBLFACT
	CMON(OP_DBLFACT,	"x!!")
#endif
#ifdef COMPLEX_BESSEL
	CDYA(OP_BSYN,		"Yn")
#endif
	CDYA(OP_BETA,		"\241")
	CMON(OP_GAMMA,		"\202")
#ifdef INCLUDE_ZETA
	CMON(OP_ZETA,		"\245")
#endif
#ifdef INCLUDE_DIGAMMA
	CMON(OP_PSI,		"\226")
#endif
};

static const s_opcode stats_catalogue[] = {
	MON(OP_sigper,		"%\221")
	NILIC(OP_BEST,		"BESTF")
	MON(OP_ERF,		"ERF")
	NILIC(OP_EXPF,		"EXPF")
	NILIC(OP_LINF,		"LINF")
		DYA(OP_LNBETA,		"LN\241")
		MON(OP_LNGAMMA,		"LN\202")
	NILIC(OP_LOGF,		"LOGF")
	NILIC(OP_sigmaN,	"n\221")
	NILIC(OP_PWRF,		"PWRF")
	NILIC(OP_RANDOM,	"RAND#")
	NILIC(OP_STORANDOM,	"SEED")
	NILIC(OP_statSErr,	"SERR")
	NILIC(OP_RCLSIGMA,	"SUM")
	MON(OP_xhat,		"\031")
	NILIC(OP_statWMEAN,	"x-barw")
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
	NILIC(OP_sigmaY,	"\221y")
	NILIC(OP_sigmaY2,	"\221y\232")
	NILIC(OP_sigmaYlnX,	"\221YlnX")
};

static const s_opcode prob_catalogue[] = {
	MON(OP_cdf_B,		"B(n)")
	MON(OP_qf_B,		"B\235(p)")
	MON(OP_cdf_EXP,		"Ex(x)")
	MON(OP_qf_EXP,		"Ex/235(p)")
	MON(OP_cdf_F,		"F(x)")
	MON(OP_qf_F,		"F/235(p)")
	MON(OP_cdf_G,		"G(n)")
	MON(OP_qf_G,		"G/235(p)")
	MON(OP_cdf_N,		"N(x)")
	MON(OP_qf_N,		"N\235(p)")
	MON(OP_cdf_P,		"P(n)")
	MON(OP_qf_P,		"P/235(p)")
	MON(OP_cdf_T,		"t(x)")
	MON(OP_qf_T,		"t/235(p)")
	MON(OP_cdf_WB,		"WB(x)")
	MON(OP_qf_WB,		"WB/235(p)")
	MON(OP_cdf_chi2,	"\225\232")
	MON(OP_qf_chi2,		"\225\232INV")
};

static const s_opcode int_catalogue[] = {
#ifdef INCLUDE_MULADD
	TRI(OP_MULADD,		"\034+")
#endif
	RARGCMD(RARG_ASR,	"ASR")
	RARGCMD(RARG_CB,	"CB")
	NILIC(OP_CLALL,		"CLALL")
	NILIC(OP_CLFLAGS,	"CLFLAG")
	NILIC(OP_CLREG,		"CLREG")
#ifdef INCLUDE_CUBES
	MON(OP_CUBE,		"CUBE")
	MON(OP_CUBERT,		"CUBERT")
#endif
	NILIC(OP_DBL_MUL,	"DBL\034")
	TRI(OP_DBL_DIV, 	"DBL/")
	TRI(OP_DBL_MOD, 	"DBLR")
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
	NILIC(OP_RANDOM,	"RAND#")
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
	DYA(OP_LXNOR,		"XNOR")
//	NILIC(OP_ALPHAAPP,	"\240APP")
	NILIC(OP_ALPHADATE,	"\240DATE")
	RARGCMD(RARG_AIP,	"\240IP")
	NILIC(OP_ALPHALEN,	"\240LENG")
	RARGCMD(RARG_AREG,	"\240RC#")
	RARGCMD(RARG_ARCL,	"\240RCL")
	RARGCMD(RARG_ALRL,	"\240RL")
	RARGCMD(RARG_ALRR,	"\240RR")
	RARGCMD(RARG_ALSL,	"\240SL")
	RARGCMD(RARG_ALSR,	"\240SR")
	RARGCMD(RARG_ASTO,	"\240STO")
	NILIC(OP_ALPHATIME,	"\240TIME")
//	NILIC(OP_AVIEW,		"\240VIEW")
};

static const s_opcode test_catalogue[] = {
	RARGCMD(RARG_BC,	"BC?")
	RARGCMD(RARG_BS,	"BS?")
	RARGCMD(RARG_FC,	"FC?")
	RARGCMD(RARG_FCC,	"FC?C")
	RARGCMD(RARG_FCF,	"FC?F")
	RARGCMD(RARG_FCS,	"FC?S")
	NILIC(OP_XisFRAC,	"FP?")
	RARGCMD(RARG_FS,	"FS?")
	RARGCMD(RARG_FSC,	"FS?C")
	RARGCMD(RARG_FSF,	"FS?F")
	RARGCMD(RARG_FSS,	"FS?S")
	NILIC(OP_XisINT,	"INT?")
	NILIC(OP_ISLEAP,	"LEAP?")
	NILIC(OP_XisNaN,	"NaN?")
	NILIC(OP_XisPRIME,	"PRIME?")
	NILIC(OP_STKSIZE,	"SSIZE?")
	NILIC(OP_INTSIZE,	"WSIZE?")
	RARGCMD(RARG_TEST_LT,	"x<?")
	RARGCMD(RARG_TEST_LE,	"x<=?")
	RARGCMD(RARG_TEST_GE,	"x>=?")
	RARGCMD(RARG_TEST_GT,	"x>?")
	NILIC(OP_XisInf,	"\237?")
};

static const s_opcode prog_catalogue[] = {
	RARGCMD(RARG_STOSTK,	"\015STK")
	RARGCMD(RARG_RCLSTK,	"\016STK")
	RARGCMD(RARG_BACK,	"BACK")
	RARGCMD(RARG_CF,	"CF")
	NILIC(OP_CLFLAGS,	"CLFLAG")
	NILIC(OP_DROPY,		"DROPY")
	RARGCMD(RARG_DSZ,	"DSZ")
	RARGCMD(RARG_FF,	"FF")
	RARGCMD(RARG_ISZ,	"ISZ")
	NILIC(OP_NOP,		"NOP")
	// OFF
	// ON
	NILIC(OP_PROMPT,	"PROMPT")
	NILIC(OP_REGCLR,	"R-CLR")
	NILIC(OP_REGCOPY,	"R-COPY")
	NILIC(OP_REGSORT,	"R-SORT")
	NILIC(OP_REGSWAP,	"R-SWAP")
	NILIC(OP_RCLFLAG,	"RCLM")
	RARGCMD(RARG_SF,	"SF")
	RARGCMD(RARG_SKIP,	"SKIP")
	NILIC(OP_STOFLAG,	"STOM")
	NILIC(OP_ALPHAOFF,	"\240OFF")
	NILIC(OP_ALPHAON,	"\240ON")
};

static const s_opcode mode_catalogue[] = {
	NILIC(OP_12HR,		"12H")
	NILIC(OP_1COMP,		"1COMPL")
	NILIC(OP_24HR,		"24H")
	NILIC(OP_2COMP,		"2COMPL")
	NILIC(OP_ALL,		"ALL")
	RARGCMD(RARG_BASE,	"BASE")
	NILIC(OP_DATEDMY,	"D.MY")
	NILIC(OP_DENANY,	"DENANY")
	NILIC(OP_DENFAC,	"DENMFAC")
	NILIC(OP_DENFIX,	"DENFIX")
	NILIC(OP_FRACDENOM,	"DENMAX")
	NILIC(OP_THOUS_OFF,	"E3OFF")
	NILIC(OP_THOUS_ON,	"E3ON")
	RARGCMD(RARG_ENG,	"ENG")
	RARGCMD(RARG_FIX,	"FIX")
	NILIC(OP_DATEMDY,	"M.DY")
	RARGCMD(RARG_SCI,	"SCI")
	NILIC(OP_SIGNMANT,	"SIGNMT")
	NILIC(OP_STK4,		"SSIZE4")
	NILIC(OP_STK8,		"SSIZE8")
	NILIC(OP_UNSIGNED,	"UNSIGN")
	RARGCMD(RARG_WS,	"WSIZ")
};

static const s_opcode alpha_catalogue[] = {
//	NILIC(OP_ALPHAAPP,	"\240APP")
	NILIC(OP_CLALL,		"CLALL")
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
//	NILIC(OP_AVIEW,		"\240VIEW")
};

static const s_opcode conv_catalogue[] = {
	MON(OP_DEGC_F,		   "degC->degF")
	MON(OP_DEGF_C,		   "degF->degC")
	CONV(OP_HA_ACRE,	1, "acre->ha")
	CONV(OP_Pa_ATM,		1, "atm->Pa")
	CONV(OP_KM_AU,		1, "au->km")
	CONV(OP_W_HPUK,		1, "bhp->W")
	CONV(OP_J_BTU,		1, "Btu->J")
	CONV(OP_J_CAL,		1, "Cal->J")
	CONV(OP_CM_INCH,	0, "cm->inch")
	CONV(OP_M_FATHOM,	1, "fathom->m")
	CONV(OP_M_FEET,		1, "feet->m")
//	CONV(OP_M_SQFEET,	1, "feet^2->m^2")
	CONV(OP_ML_FLOZUK,	1, "flozUK->mL")
	CONV(OP_ML_FLOZUS,	1, "flozUS->mL")
	CONV(OP_G_OZ,		0, "g->oz")
	CONV(OP_G_TOZ,		0, "g->tr oz")
	CONV(OP_L_GALUK,	1, "galUK->L")
	CONV(OP_L_GALUS,	1, "galUS->L")
	CONV(OP_HA_ACRE,	0, "ha->acre")
	CONV(OP_W_HPe,		1, "HPe->W")
	CONV(OP_CM_INCH,	1, "inch->cm")
	CONV(OP_J_BTU,		0, "J->Btu")
	CONV(OP_J_CAL,		0, "J->Cal")
	CONV(OP_J_kWh,		0, "J->kW.h")
	CONV(OP_KG_LBM, 	0, "kg->lbm")
	CONV(OP_KM_AU,		0, "km->au")
	CONV(OP_KM_LY,		0, "km->l.y.")
	CONV(OP_KM_MILE,	0, "km->mile")
	CONV(OP_KM_NMI,		0, "km->nmile")
	CONV(OP_KM_PC,		0, "km->pc")
	CONV(OP_J_kWh,		1, "kW.h->J")
	CONV(OP_L_GALUK,	0, "L->galUK")
	CONV(OP_L_GALUS,	0, "L->galUS")
	CONV(OP_KM_LY,		1, "l.y.->km")
	CONV(OP_KG_LBM, 	1, "lb->kg")
	CONV(OP_N_LBF,		1, "lbf->N")
	CONV(OP_M_FATHOM,	0, "m->fathom")
	CONV(OP_M_FEET,		0, "m->feet")
	CONV(OP_M_YARD,		0, "m->yard")
//	CONV(OP_M_SQFEET,	0, "m^2->feet^2")
	CONV(OP_Pa_mbar,	1, "mbar->Pa")
	CONV(OP_KM_MILE,	1, "mile->km")
	CONV(OP_ML_FLOZUK,	0, "mL->flozUK")
	CONV(OP_ML_FLOZUS,	0, "mL->flozUS")
	CONV(OP_Pa_mmHg,	1, "mmHg->Pa")
	CONV(OP_N_LBF,		0, "N->lbf")
	CONV(OP_KM_NMI,		1, "nmile->km")
	CONV(OP_G_OZ,		1, "oz->g")
	CONV(OP_Pa_ATM,		0, "Pa->ATM")
	CONV(OP_Pa_mbar,	0, "Pa->bar")
	CONV(OP_Pa_mmHg,	0, "Pa->mmHg")
	CONV(OP_Pa_psi,		0, "Pa->psi")
	CONV(OP_KM_PC,		1, "pc->km")
	CONV(OP_W_HP,		1, "PS(HP)->W")
	CONV(OP_Pa_psi,		1, "psi->Pa")
	CONV(OP_T_SHTON,	1, "sh ton->t")
	CONV(OP_T_SHTON,	0, "t->sh ton")
	CONV(OP_T_TON,		0, "t->ton")
	CONV(OP_T_TON,		1, "ton->t")
	CONV(OP_G_TOZ,		1, "tr oz->g")
	CONV(OP_W_HPUK,		0, "W->bhp")
	CONV(OP_W_HPe,		0, "W->HPe")
	CONV(OP_W_HP,		0, "W->PS(HP)")
	CONV(OP_M_YARD,		1, "yard->m")
};


/* The alpha mode menus to access all the weird characters */
static const char alpha_symbols[] = {
	',',	'"',	'#',	'&',	'\'',	'`',	'*',	':',
	';',	'@',	'\\',	'_',	'|',	'~'
};

static const char alpha_compares[] = {
	'<',	'\011',	'=',	'\013',	'\012',	'>',
	'[',    ']',	'{',	'}'
};

static const char alpha_arrows[] = {
	015,	016,	017,	020,	027,	//  arrows
	003,	004,	005,			// sqrt, integral, degree
	0235,	0232,				// ^-1, ^2
	0236,					// h bar
	0234,					// ^x
	'^',	0237				// ^, infinity
};

static const char alpha_stats[] = {
	031,	001,				// x hat, x bar
	032,	002,				// y hat, y bar
};

static const char alpha_letters_upper[] = {
	0300, 0301, 0302, 0303, 0304,		// A
	0305, 0306, 0307,			// C
	0310, 0311, 0312, 0313,			// E
	0314, 0315, 0316, 0317,			// I
	0320,					// N
	0321, 0322, 0323, 0324,			// O
	0325,					// R
	0326,					// S
	0330, 0331, 0332, 0333, 0334,		// U
	0335, 0336,				// Y
	0337					// Z
};
static const char alpha_letters_lower[] = {
	0340, 0341, 0342, 0343, 0344,		// A
	0345, 0346, 0347,			// C
	0350, 0351, 0352, 0353,			// E
	0354, 0355, 0356, 0357,			// I
	0360,					// N
	0361, 0362, 0363, 0364,			// O
	0365,					// R
	0366, 0030,				// S
	0367, 0370, 0371, 0372, 0373,		// U
	0374, 0375,				// Y
	0376					// Z
};

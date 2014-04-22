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


static s_opcode clear_catalogue[] = {
	NILIC(OP_CLALL,		"CLALL")
	NILIC(OP_CLREG,		"CLREG")
	NILIC(OP_SIGMACLEAR, "CL\221")
	NILIC(OP_RESET,		"RESET")
//	NILIC(OP_CLSTK,		"CLSTK")
};

static s_opcode displ_catalogue[] = {
	RARGCMD(RARG_STD,	"ALL") 
	RARGCMD(RARG_DISP,	"DISP")
	RARGCMD(RARG_ENG,	"ENG")
	RARGCMD(RARG_FIX,	"FIX")
	NILIC(OP_RADCOM,	"RDX,")
	NILIC(OP_RADDOT,	"RDX.")
	RARGCMD(RARG_SCI,	"SCI")
	NILIC(OP_THOUS_OFF,	"TSOFF")
	NILIC(OP_THOUS_ON,	"TSON")
};

static s_opcode more_catalogue[] = {
	DYA(OP_ATAN2,		"ANGLE")
	NILIC(OP_VOLTAGE,   "BATT")
	DYA(OP_DTADD,       "DAYS+")
	MON(OP_FRAC,        "FP")
	DYA(OP_GCD,		"GCD")
	MON(OP_TRUNC,       "IP")
	DYA(OP_LCM,         "LCM")
	NILIC(OP_LOAD,		"LOAD")
	DYA(OP_MOD41,       "MOD")
	NILIC(OP_NEXTPRIME, "NEXTP")
	NILIC(OP_XisPRIME,  "PRIME?")
	DYA(OP_MOD,         "RMDR")
	NILIC(OP_SAVE,		"SAVE")
	NILIC(OP_STKSIZE,   "SSIZE?")
	NILIC(OP_VERSION,   "VERS")
	MON(OP_DOWK,        "WDAY")
	NILIC(OP_WHO,		"WHO")
	DYA(OP_XROOT,		"\234\003y")
	DYA(OP_DTDIF,       "\203DAYS")
	TRI(OP_PERMRR,      "MRR")
    DYA(OP_PARAL,       "||")
	MON(OP_CUBERT,		"[^3][sqrt]")
	NILIC(OP_2FRAC,		"DECOMP")
#ifdef ENABLE_REGISTER_BROWSER
	NILIC(OP_SHOWREGS,	"CATREG")
#endif
};

static s_opcode stats_catalogue[] = {
	DYA(OP_PERM, "PERM")
	DYA(OP_COMB, "COMB")
	NILIC(OP_statSErr,	"SERR")
	NILIC(OP_RCLSIGMA,	"SUM")
	NILIC(OP_statWMEAN,	"\001w")
	NILIC(OP_statWSErr,	"\244w")
	NILIC(OP_statWS,	"sw")
	NILIC(OP_statLR,	"L.R.")
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
//	MON(OP_pdf_LN,		"LgNrm(x)")
//	MON(OP_cdf_LN,		"LgNrm(x)")
//	MON(OP_qf_LN,		"LgNrm\235(p)")
	MON(OP_pdf_N,		"Normlp")
	MON(OP_cdf_N,		"Norml")
	MON(OP_qf_N,		"Norml\235")
//	MON(OP_pdf_Plam,	"Pois[lambda]\276")
//	MON(OP_cdf_Plam,	"Pois[lambda]")
//	MON(OP_qf_Plam,		"Pois[lambda]/235")
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
//	MON(OP_pdf_Q,		"\224p(x)")
#ifdef INCLUDE_CDFU
//	MON(OP_cdfu_Q,		"\224\277(x)")
	MON(OP_cdfu_chi2,	"\225\232\277")
	MON(OP_cdfu_T,		"t\277(x)")
	MON(OP_cdfu_F,		"F\277(x)")
	MON(OP_cdfu_WB,		"Weibl\277")
	MON(OP_cdfu_EXP,	"Expon\277")
	MON(OP_cdfu_B,		"Binom\277")
//	MON(OP_cdfu_Plam,	"Pois\252\277")
	MON(OP_cdfu_P,		"Poiss\277")
	MON(OP_cdfu_G,		"Geom\277")
	MON(OP_cdfu_N,		"Norml\277")
//	MON(OP_cdfu_LN,		"LgNrm\277")
	MON(OP_cdfu_LG,		"Logis\277")
	MON(OP_cdfu_C,		"Cauch\277")
#endif
};


static s_opcode mode_catalogue[] = {
	NILIC(OP_BEST,		"BestF")
	NILIC(OP_DENANY,	"DENANY")
	NILIC(OP_DENFAC,	"DENFAC")
	NILIC(OP_DENFIX,	"DENFIX")
	NILIC(OP_FRACDENOM,	"DMAX")
	NILIC(OP_DATEDMY,	"D.MY")
	NILIC(OP_EXPF,		"ExpF")
	NILIC(OP_FRACIMPROPER,		"IMPFRC")
	NILIC(OP_LINF,		"LinF")
	NILIC(OP_LOGF,		"LogF")
	NILIC(OP_DATEMDY,	"M.DY")
	NILIC(OP_PWRF,		"PowerF")
	NILIC(OP_FRACPROPER,		"PROFRC")
	NILIC(OP_STK4,		"SSIZE4")
	NILIC(OP_STK8,		"SSIZE8")
	NILIC(OP_DATEYMD,	"Y.MD")
	NILIC(OP_SHOWY,		"YDON")
	NILIC(OP_HIDEY,		"YDOFF")
};

static s_opcode alpha_catalogue[] = {
	NILIC(OP_ALPHADATE,	"\240DATE")
/*	NILIC(OP_ALPHADAY,	"\240DAY")
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
	*/
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
//	CONV(OP_M_SQUARE,	0, "m^2->square")
//	CONV(OP_M_SQUARE,	1, "square->m^2")
//	CONV(OP_M_PERCH,	0, "m^2->perch")
//	CONV(OP_M_PERCH,	1, "perch->m^2")
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

	CAT(stats_catalogue);
	CAT(prob_catalogue);
	CAT(mode_catalogue);
	CAT(alpha_catalogue);
	CONVERSION(conv_catalogue);
	CAT(clear_catalogue);
	CAT(displ_catalogue);
	CAT(more_catalogue);

	fprintf(stderr, "# $Rev$\n" );
	fprintf(stderr, "version=%c%c\n", VERSION_STRING[0], VERSION_STRING[2]);
        fprintf(stderr, "maxsteps=%d\n", 1);
        fprintf(stderr, "maxlibsteps=%d\n", 1);
	dump_opcodes(stderr, 0);

	printf( "/* Total number of catalogue entries %d\n"
		" * Total number of conversion entries %d\n"
		" * Total number of alpha entries %d\n"
		" */\n", total_cat, total_conv, total_alpha);
	printf("#endif\n");
	return 0;
}

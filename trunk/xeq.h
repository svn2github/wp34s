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

#ifndef __XEQ_H__
#define __XEQ_H__

/* Version number */
#define VERSION_STRING	"2.3"

/*
 * Optional features are defined in features.h
 */
#include "features.h"

/*
 * Define endianess if not on GCC
 */
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#ifdef WIN32
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

/* Define the length of our extended precision numbers.
 * This must be greater than the length of the compressed reals we store
 * on the stack and in registers (currently 16 digits).
 *
 * To account for the nasty cancellations you get doing complex arithmetic
 * we really want this to be a bit more than double the standard precision.
 * i.e. >32.  There is a nice band from 34 - 39 digits which occupy 36 bytes.
 * 33 digits is the most that occupy 32 bytes but that doesn't leave enough
 * guard digits for my liking.
 */
//#define DECNUMDIGITS 24

// Note: when changing this, consider adjusting DECBUFFER in DecNumberLocal.h
// to size the temp buffer size and avoid malloc/free
#define DECNUMDIGITS 39		/* 32 bytes per real for 28 .. 33, 36 bytes for 34 .. 39 */


#include "decNumber/decNumber.h"
#include "decNumber/decContext.h"
#include "decNumber/decimal64.h"

enum nilop;
enum rarg;
enum multiops;

/* Define some system flag to user flag mappings
 */
#define A_FLAG		100	/* A = annunciator */
#define OVERFLOW_FLAG	101	/* B = excess/exceed */
#define CARRY_FLAG	102	/* C = carry */
#define NAN_FLAG	103	/* D = danger */

#define NAME_LEN	6	/* Length of command names */

typedef unsigned int opcode;
typedef unsigned short int s_opcode;

#ifdef COMPILE_CATALOGUES
#define isNULL(fp) (strcmp(fp, "NOFN") == 0)
#else
#define isNULL(fp) (fp == FNULL)
#endif

#if (defined(REALBUILD) || defined(POST_PROCESSING)) && !defined(COMPILE_CATALOGUES)
#pragma pack(push)
#pragma pack(2)
#define SHORT_POINTERS
#endif

#ifdef SHORT_POINTERS
/*
 *  Pointers are offset to zero and shifted to the right to fit in a short
 *  This macro reverses the operation.
 */
#define CALL(f) (*((void (*)())(0x100001 | ( f << 1 ))))
#define DCALL(f) (*((decNumber *(*)())(0x100001 | ( f << 1 ))))
#define ICALL(f) (*((long long (*)())(0x100001 | ( f << 1 ))))
#define FNULL 0
#ifdef POST_PROCESSING
#define _CONST /**/
#else
#define _CONST const
#endif
extern _CONST struct _command_info {
	_CONST struct monfunc *p_monfuncs;
	_CONST struct monfunc_cmdtab *p_monfuncs_ct;
	_CONST struct dyfunc *p_dyfuncs;
	_CONST struct dyfunc_cmdtab *p_dyfuncs_ct;
	_CONST struct trifunc *p_trifuncs;
	_CONST struct trifunc_cmdtab *p_trifuncs_ct;
	_CONST struct niladic *p_niladics;
	_CONST struct niladic_cmdtab *p_niladics_ct;
	_CONST struct argcmd *p_argcmds;
	_CONST struct argcmd_cmdtab *p_argcmds_ct;
	_CONST struct multicmd *p_multicmds;
	_CONST struct multicmd_cmdtab *p_multicmds_ct;
} command_info;
#else
#define CALL(f) (*f)
#define DCALL(f) (*f)
#define ICALL(f) (*f)
#define FNULL NULL
#endif

/* Table of monadic functions */
#ifdef SHORT_POINTERS
/*
 *  Define a shorter version of the structure.
 *  Will be filled by post-processor.
 */
struct monfunc
{
	unsigned short mondreal, mondcmplx, monint;
	_CONST char fname[NAME_LEN];
};

/*
 *  The full version goes to the .cmdtab segment
 */
struct monfunc_cmdtab
#else
/*
 *  No tricks version
 */
struct monfunc
#endif
{
#ifdef COMPILE_CATALOGUES
	const char mondreal[30], mondcmplx[30], monint[30];
#else
#ifdef DEBUG
	unsigned short n;
#endif
	decNumber *(*mondreal)(decNumber *, const decNumber *);
	void (*mondcmplx)(decNumber *, decNumber *, const decNumber *, const decNumber *);
	long long int (*monint)(long long int);
#endif
	const char fname[NAME_LEN];
};
extern const struct monfunc monfuncs[];
extern const unsigned short num_monfuncs;

/* Table of dyadic functions */
#ifdef SHORT_POINTERS
/*
 *  Define a shorter version of the structure.
 *  Will be filled by post-processor.
 */
struct dyfunc
{
	unsigned short dydreal, dydcmplx, dydint;
	_CONST char fname[NAME_LEN];
};

/*
 *  The full version goes to the .cmdtab segment
 */
struct dyfunc_cmdtab
#else
/*
 *  No tricks version
 */
struct dyfunc
#endif
{
#ifdef COMPILE_CATALOGUES
	const char dydreal[30], dydcmplx[30], dydint[30];
#else
#ifdef DEBUG
	unsigned short n;
#endif
	decNumber *(*dydreal)(decNumber *, const decNumber *, const decNumber *);
	void (*dydcmplx)(decNumber *, decNumber *, const decNumber *, const decNumber*,
				const decNumber *, const decNumber *);
	long long int (*dydint)(long long int, long long int);
#endif
	const char fname[NAME_LEN];
};
extern const struct dyfunc dyfuncs[];
extern const unsigned short num_dyfuncs;

/* Table of triadic functions */
#ifdef SHORT_POINTERS
/*
 *  Define a shorter version of the structure.
 *  Will be filled by post-processor.
 */
struct trifunc
{
	unsigned short trireal, triint;
	_CONST char fname[NAME_LEN];
};

/*
 *  The full version goes to the .cmdtab segment
 */
struct trifunc_cmdtab
#else
/*
 *  No tricks version
 */
struct trifunc
#endif
{
#ifdef DEBUG
	unsigned short n;
#endif
#ifdef COMPILE_CATALOGUES
	const char trireal[30], triint[30];
#else
	decNumber *(*trireal)(decNumber *, const decNumber *, const decNumber *, const decNumber *);
	long long int (*triint)(long long int, long long int, long long int);
#endif
	const char fname[NAME_LEN];
};
extern const struct trifunc trifuncs[];
extern const unsigned short num_trifuncs;


/* Table of niladic functions */
#ifdef SHORT_POINTERS
/*
 *  Define a shorter version of the structure.
 *  Will be filled by post-processor.
 */
struct niladic
{
	unsigned short niladicf;
	unsigned char numresults;
	_CONST char nname[NAME_LEN];
};

/*
 *  The full version goes to the .cmdtab segment
 */
struct niladic_cmdtab
#else
/*
 *  No tricks version
 */
struct niladic
#endif
{
#ifdef DEBUG
	unsigned short n;
#endif
#ifdef COMPILE_CATALOGUES
	const char niladicf[30];
#else
	void (*niladicf)(decimal64 *, decimal64 *, enum nilop);
#endif
	unsigned char numresults;
	const char nname[NAME_LEN];
};
extern const struct niladic niladics[];
extern const unsigned short num_niladics;
#define NILADIC_NOINT		(0x80)
#define NILADIC_NUMRESULTS(n)	((n).numresults & 0x3)
#define NILADIC_NOTINT(n)	((n).numresults & NILADIC_NOINT)


/* Table of argument taking commands */
#ifdef SHORT_POINTERS
/*
 *  Define a shorter version of the structure.
 *  Will be filled by post-processor.
 */
struct argcmd
{
	unsigned short f;
	unsigned char lim;
	unsigned int indirectokay : 1;
	unsigned int stckreg : 1;
	unsigned int local : 1;
	unsigned int cmplx : 1;
	unsigned int label : 1;
	unsigned int flag : 1;
	unsigned int stos : 1;
	_CONST char cmd[NAME_LEN];
};

/*
 *  The full version goes to the .cmdtab segment
 */
struct argcmd_cmdtab
#else
/*
 *  No tricks version
 */
struct argcmd
#endif
{
#ifdef DEBUG
	unsigned short n;
#endif
#ifdef COMPILE_CATALOGUES
	const char f[30];
#else
	void (*f)(unsigned int, enum rarg);
#endif
	unsigned int lim : 8;
	unsigned int indirectokay : 1;
	unsigned int stckreg : 1;
	unsigned int local : 1;
	unsigned int cmplx : 1;
	unsigned int label : 1;
	unsigned int flag : 1;
	unsigned int stos : 1;
	const char cmd[NAME_LEN];
};
extern const struct argcmd argcmds[];
extern const unsigned short num_argcmds;

#ifdef SHORT_POINTERS
/*
 *  Define a shorter version of the structure.
 *  Will be filled by post-processor.
 */
struct multicmd
{
	unsigned short f;
	_CONST char cmd[NAME_LEN];
};

/*
 *  The full version goes to the .cmdtab segment
 */
struct multicmd_cmdtab
#else
/*
 *  No tricks version
 */
struct multicmd
#endif
{
#ifdef DEBUG
	unsigned short n;
#endif
#ifdef COMPILE_CATALOGUES
	const char f[30];
#else
	void (*f)(opcode, enum multiops);
#endif
	const char cmd[NAME_LEN];
};
extern const struct multicmd multicmds[];
extern const unsigned short num_multicmds;

#if defined(REALBUILD) && !defined(COMPILE_CATALOGUES)
#pragma pack(pop)
#endif


/* Return the specified opcode in the position indicated in the current
 * catalogue.
 */
extern opcode current_catalogue(int);
extern int current_catalogue_max(void);


/* Allow the number of registers and the size of the stack to be changed
 * relatively easily.
 */
#define NUMPROG		510	/* Fill 1 KB (including crc and length) */
#define RET_STACK_SIZE	26	/* Minimum depth of return stack, extends into program space */
#define STACK_SIZE	8	/* Maximum depth of RPN stack */
#define EXTRA_REG	4
#define NUMLBL		104	/* Number of program labels */
#define LBLNAMES	"ABCD"
#define NUMFLG		104	/* Number of flags */

#define NUMALPHA	31	/* Number of characters in Alpha */

#define CMDLINELEN	19	/* 12 mantissa + dot + sign + E + sign + 3 exponent = 19 */
#define NUMBANKFLAGS	16

/* Special return stack marker for local registers */
#define LOCAL_MASK      (0x8000u)
#define isLOCAL(s)	((s) & LOCAL_MASK)
#define LOCAL_LEVELS(s)	((s) & 0x1ff)
#define LOCAL_MAXREG(s)	(LOCAL_LEVELS(s) >> 2)

/* Macros to access program ROM */
#define XROM_MASK	(0x4000u)
#define isXROM(pc)	((pc) & XROM_MASK)
#define addrXROM(pc)	((pc) | XROM_MASK)

/* Macros to access flash library space */
#if NUMPROG > 511
#define LIB_MASK	(0x3c00u)
#define LIB_SHIFT	(10)
#else
#define LIB_MASK	(0x3e00u)
#define LIB_SHIFT	(9)
#endif

#define LIB_MAX		(15 - LIB_SHIFT)
#define LIB_ADDR_MASK	((1 << (LIB_SHIFT)) - 1)
#define isLIB(pc)	((pc) & LIB_MASK)
#define nLIB(pc)	(((pc) & LIB_MASK) >> LIB_SHIFT)
#define addrLIB(pc, n)	((pc) | ((n) << LIB_SHIFT))
#define startLIB(pc)	((pc) & ~LIB_ADDR_MASK)
#define offsetLIB(pc)	((pc) & LIB_ADDR_MASK)
#define sizeLIB(n)	(UserFlash.region[NUMBER_OF_FLASH_REGIONS-1-(n)].last_prog-1)

#define isRAM(pc)	(((pc) & (XROM_MASK | LIB_MASK | LOCAL_MASK)) == 0)


/* Stack lives in the register set */
#define NUMREG		(TOPREALREG+STACK_SIZE+EXTRA_REG)/* Number of registers */
#define TOPREALREG	(100)				/* Non-stack last register */

#define REGNAMES	"XYZTABCDLIJK"

#define regX_idx	(TOPREALREG)
#define regY_idx	(TOPREALREG+1)
#define regZ_idx	(TOPREALREG+2)
#define regT_idx	(TOPREALREG+3)
#define regA_idx	(TOPREALREG+4)
#define regB_idx	(TOPREALREG+5)
#define regC_idx	(TOPREALREG+6)
#define regD_idx	(TOPREALREG+7)
#define regL_idx	(TOPREALREG+8)
#define regI_idx	(TOPREALREG+9)
#define regJ_idx	(TOPREALREG+10)
#define regK_idx	(TOPREALREG+11)

#define regX	(Regs[regX_idx])
#define regY	(Regs[regY_idx])
#define regZ	(Regs[regZ_idx])
#define regT	(Regs[regT_idx])
#define regA	(Regs[regA_idx])
#define regB	(Regs[regB_idx])
#define regC	(Regs[regC_idx])
#define regD	(Regs[regD_idx])
#define regL	(Regs[regL_idx])
#define regI	(Regs[regI_idx])
#define regJ	(Regs[regJ_idx])
#define regK	(Regs[regK_idx])


/* Define the operation codes and various masks to simplify access to them all
 */
enum eKind {
	KIND_SPEC=0,
	KIND_NIL,
	KIND_MON,
	KIND_DYA,
	KIND_TRI,
	KIND_CMON,
	KIND_CDYA
};
#define KIND_MAX	(1 + (int)KIND_CDYA)

#define KIND_SHIFT	8
#define DBL_SHIFT	8

#define OP_SPEC	(KIND_SPEC << KIND_SHIFT)		/* Special operations */
#define OP_NIL	(KIND_NIL << KIND_SHIFT)		/* Niladic operations */
#define OP_MON	(KIND_MON << KIND_SHIFT)		/* Monadic operations */
#define OP_DYA	(KIND_DYA << KIND_SHIFT)		/* Dyadic operations */
#define OP_TRI	(KIND_TRI << KIND_SHIFT)		/* Triadic operations */
#define OP_CMON	(KIND_CMON << KIND_SHIFT)		/* Complex Monadic operation */
#define OP_CDYA	(KIND_CDYA << KIND_SHIFT)		/* Complex Dyadic operation */

#define OP_DBL	0xF000			/* Double sized instructions */

#define RARG_OPSHFT	8		/* Don't use outside this header */
#define RARG_OPBASE	0x20		/* Don't use outside this header */

#define RARG_MASK	0x7f
#define RARG_IND	0x80
#define RARG_BASE(op)	(((op) + RARG_OPBASE) << RARG_OPSHFT)
#define RARG(op, n)	(RARG_BASE(op) | (n))
#define RARG_CMD(op)	(((op) >> RARG_OPSHFT) - RARG_OPBASE)

#define opKIND(op)	((enum eKind)((op) >> KIND_SHIFT))
#define argKIND(op)	((op) & ((1 << KIND_SHIFT)-1))
#define isDBL(op)	(((op) & 0xf000) == OP_DBL)
#define opDBL(op)	(((op) >> DBL_SHIFT) & 0xf)

//#define isRARG(op)	(((op) & 0xff00) >= (RARG_OPBASE << RARG_OPSHFT) && ! isDBL(op))
#define isRARG(op)	(((op) & 0xf000) > 0 && ! isDBL(op))

enum tst_op {
	TST_EQ=0,	TST_NE=1,	TST_APX=2,
	TST_LT=3,	TST_LE=4,
	TST_GT=5,	TST_GE=6,
};
#define TST_NONE	7



// Monadic functions
enum {
	OP_FRAC = 0, OP_FLOOR, OP_CEIL, OP_ROUND, OP_TRUNC,
	OP_ABS, OP_RND, OP_SIGN,

	OP_LN, OP_EXP, OP_SQRT, OP_RECIP, OP__1POW,
	OP_LOG, OP_LG2, OP_2POWX, OP_10POWX,
	OP_LN1P, OP_EXPM1,
	OP_LAMW, OP_INVW,
	OP_SQR,
	OP_CUBE, OP_CUBERT,

	OP_FIB,

	OP_2DEG, OP_2RAD, OP_2GRAD, OP_DEG2, OP_RAD2, OP_GRAD2,

	OP_SIN, OP_COS, OP_TAN,
	OP_ASIN, OP_ACOS, OP_ATAN,
	OP_SINC,

	OP_SINH, OP_COSH, OP_TANH,
	OP_ASINH, OP_ACOSH, OP_ATANH,

	OP_FACT, OP_GAMMA, OP_LNGAMMA,
#ifdef INCLUDE_DIGAMMA
	OP_PSI,
#endif
#ifdef INCLUDE_DBLFACT
	OP_DBLFACT,
#endif
#ifdef INCLUDE_SUBFACT
	OP_SUBFACT,
#endif
	OP_DEG2RAD, OP_RAD2DEG, OP_DEG2GRD, OP_GRD2DEG, OP_RAD2GRD, OP_GRD2RAD,
	OP_CCHS, OP_CCONJ,		// CHS and Conjugate
	OP_ERF, OP_ERFC,
	OP_pdf_Q, OP_cdf_Q, OP_qf_Q,
	OP_pdf_chi2, OP_cdf_chi2, OP_qf_chi2,
	OP_pdf_T, OP_cdf_T, OP_qf_T,
	OP_pdf_F,OP_cdf_F, OP_qf_F,
	OP_pdf_WB, OP_cdf_WB, OP_qf_WB,
	OP_pdf_EXP, OP_cdf_EXP, OP_qf_EXP,
	OP_pdf_B, OP_cdf_B, OP_qf_B,
	OP_pdf_P, OP_cdf_P, OP_qf_P,
	OP_pdf_G, OP_cdf_G, OP_qf_G,
	OP_pdf_N, OP_cdf_N, OP_qf_N,
	OP_pdf_LN, OP_cdf_LN, OP_qf_LN,
	OP_pdf_LG, OP_cdf_LG, OP_qf_LG,
	OP_pdf_C, OP_cdf_C, OP_qf_C,
#ifdef INCLUDE_CDFU
	OP_cdfu_Q,  OP_cdfu_chi2, OP_cdfu_T, OP_cdfu_F, OP_cdfu_WB,
	OP_cdfu_EXP, OP_cdfu_B, OP_cdfu_P, OP_cdfu_G, OP_cdfu_N,
	OP_cdfu_LN, OP_cdfu_LG, OP_cdfu_C,
#endif
	OP_xhat, OP_yhat,
	OP_sigper,
	OP_PERCNT, OP_PERCHG, OP_PERTOT,// % operations -- really dyadic but leave the Y register unchanged
	OP_HMS2, OP_2HMS,

	//OP_SEC, OP_COSEC, OP_COT,
	//OP_SECH, OP_COSECH, OP_COTH,

	OP_NOT,
	OP_BITCNT, OP_MIRROR,
	OP_DOWK, OP_D2J, OP_J2D,

	OP_DEGC_F, OP_DEGF_C,
	OP_DB_AR, OP_AR_DB, OP_DB_PR, OP_PR_DB,
#ifdef INCLUDE_ZETA
	OP_ZETA,
#endif
#ifdef INCLUDE_BERNOULLI
	OP_Bn, OP_BnS,
#endif
#ifdef INCLUDE_EASTER
	OP_EASTER,
#endif
#ifdef INCLUDE_FACTOR
	OP_FACTOR,
#endif
	OP_DATE_YEAR, OP_DATE_MONTH, OP_DATE_DAY,
#ifdef INCLUDE_USER_IO
	OP_RECV1,
#endif
#ifdef INCLUDE_MANTISSA
	OP_MANTISSA, OP_EXPONENT,
#endif
#ifdef MATRIX_SUPPORT
	OP_MAT_ALL, OP_MAT_DIAG,
	OP_MAT_TRN,
	OP_MAT_RQ, OP_MAT_CQ, OP_MAT_IJ,
	OP_MAT_DET,
#ifdef MATRIX_LU_DECOMP
	OP_MAT_LU,
#endif
#endif
	NUM_MONADIC	// Last entry defines number of operations
};
    
// Dyadic functions
enum {
	OP_POW = 0,
	OP_ADD, OP_SUB, OP_MUL, OP_DIV,
	OP_MOD,
	OP_LOGXY,
	OP_MIN, OP_MAX,
	OP_ATAN2,
	OP_BETA, OP_LNBETA,
	OP_GAMMAP,
#ifdef INCLUDE_ELLIPTIC
	OP_SN, OP_CN, OP_DN,
#endif
#ifdef INCLUDE_BESSEL
	OP_BSJN, OP_BSIN, OP_BSYN, OP_BSKN,
#endif
	OP_COMB, OP_PERM,
	OP_PERMG, OP_MARGIN,
	OP_PARAL,
	OP_AGM,
	OP_HMSADD, OP_HMSSUB,
	OP_GCD, OP_LCM,
	OP_LAND, OP_LOR, OP_LXOR, OP_LNAND, OP_LNOR, OP_LXNOR,
	OP_DTADD, OP_DTDIF,

	// Orthogonal polynomials -- must be in the same order as the enum below
	OP_LEGENDRE_PN,
	OP_CHEBYCHEV_TN,
	OP_CHEBYCHEV_UN,
	OP_LAGUERRE,
	OP_HERMITE_HE,
	OP_HERMITE_H,

#ifdef INCLUDE_XROOT
	OP_XROOT,
#endif
#ifdef MATRIX_SUPPORT
	OP_MAT_ROW, OP_MAT_COL,
	OP_MAT_COPY,
#endif
	NUM_DYADIC	// Last entry defines number of operations
};

// Triadic functions
enum {
	OP_BETAI=0,
	OP_DBL_DIV, OP_DBL_MOD,
#ifdef INCLUDE_MULADD
	OP_MULADD,
#endif
	OP_PERMRR,
	OP_GEN_LAGUERRE,
#ifdef MATRIX_SUPPORT
	OP_MAT_MUL,
	OP_MAT_GADD,
	OP_MAT_REG,
	OP_MAT_LIN_EQN,
#endif
	NUM_TRIADIC	// Last entry defines number of operations
};  

// Niladic functions
enum nilop {
	OP_NOP=0, OP_VERSION, OP_OFF,
	OP_STKSIZE, OP_STK4, OP_STK8, OP_INTSIZE,
	OP_RDOWN, OP_RUP, OP_CRDOWN, OP_CRUP,
	OP_CENTER, OP_FILL, OP_CFILL, OP_DROP, OP_DROPXY,
	OP_sigmaX2Y, OP_sigmaX, OP_sigmaX2, OP_sigmaY, OP_sigmaY2, OP_sigma_XY,
	OP_sigmaN,
	OP_sigmalnX, OP_sigmalnXlnX, OP_sigmalnY, OP_sigmalnYlnY,
		OP_sigmalnXlnY, OP_sigmaXlnY, OP_sigmaYlnX,
	OP_statS, OP_statSigma, OP_statGS, OP_statGSigma,
		OP_statWS, OP_statWSigma,
		OP_statMEAN, OP_statWMEAN, OP_statGMEAN,
		OP_statR, OP_statLR,
		OP_statSErr, OP_statGSErr, OP_statWSErr,
	OP_statCOV, OP_statSxy,
	OP_LINF, OP_EXPF, OP_PWRF, OP_LOGF, OP_BEST,
	OP_RANDOM, OP_STORANDOM,
	OP_DEG, OP_RAD, OP_GRAD,
	OP_RTN, OP_RTNp1,
	OP_RS, OP_PROMPT,
	OP_SIGMACLEAR, OP_CLREG, OP_rCLX, OP_CLSTK, OP_CLALL, OP_RESET, OP_CLFLAGS,
	OP_R2P, OP_P2R,
	OP_FRACDENOM, OP_2FRAC, OP_DENANY, OP_DENFIX, OP_DENFAC,
	OP_FRACIMPROPER, OP_FRACPROPER,
	OP_RADDOT, OP_RADCOM, OP_THOUS_ON, OP_THOUS_OFF,
	OP_FIXSCI, OP_FIXENG,
	OP_2COMP, OP_1COMP, OP_UNSIGNED, OP_SIGNMANT,
	OP_FLOAT, OP_HMS, OP_FRACT,
	OP_LEAD0, OP_TRIM0,
	OP_LJ, OP_RJ,
	OP_DBL_MUL,
	OP_RCLSIGMA,
	OP_DATEDMY, OP_DATEYMD, OP_DATEMDY, OP_JG1752, OP_JG1582,
	OP_ISLEAP, OP_ALPHADAY, OP_ALPHAMONTH, OP_ALPHADATE, OP_ALPHATIME,
	OP_DATE, OP_TIME,
	OP_24HR, OP_12HR,OP_SETDATE, OP_SETTIME,
	OP_CLRALPHA, OP_VIEWALPHA, OP_ALPHALEN,
	OP_ALPHATOX, OP_XTOALPHA, OP_ALPHAON, OP_ALPHAOFF,
	OP_REGCOPY, OP_REGSWAP, OP_REGCLR, OP_REGSORT,
	OP_GSBuser,
	OP_XisInf, OP_XisNaN, OP_XisSpecial, OP_XisPRIME,
	OP_XisINT, OP_XisFRAC, OP_XisEVEN, OP_XisODD,
	OP_ENTRYP,

	OP_TICKS, OP_VOLTAGE,
	OP_SETEUR, OP_SETUK, OP_SETUSA, OP_SETIND, OP_SETCHN,

	OP_QUAD, OP_NEXTPRIME,
	OP_XEQALPHA, OP_GTOALPHA,
	OP_RLOAD, OP_SLOAD, OP_BACKUP, OP_RESTORE,

	OP_ROUNDING,
	OP_SLOW, OP_FAST,

	OP_SENDP, OP_SENDR, OP_SENDA, OP_RECV,
#ifdef INCLUDE_USER_IO
	OP_SEND1, OP_SERIAL_OPEN, OP_SERIAL_CLOSE,
	OP_ALPHASEND, OP_ALPHARECV,
#endif
	OP_TOP,
	OP_GETBASE, OP_GETSIGN,
	OP_ISINT, OP_ISFLOAT,

	OP_Xeq_pos0, OP_Xeq_neg0,

#ifdef MATRIX_SUPPORT
#ifdef MATRIX_ROWOPS
	OP_MAT_ROW_SWAP, OP_MAT_ROW_MUL, OP_MAT_ROW_GADD,
#endif
	OP_MAT_CHECK_SQUARE,
	OP_MAT_INVERSE,
#endif
#ifdef SILLY_MATRIX_SUPPORT
	OP_MAT_ZERO, OP_MAT_IDENT,
#endif
	OP_MEM,
	OP_POPUSR,
#ifdef INCLUDE_STOPWATCH
	OP_STOPWATCH,
#endif // INCLUDE_STOPWATCH

	NUM_NILADIC	// Last entry defines number of operations
};

#define EMPTY_PROGRAM_OPCODE	RARG(RARG_ERROR, ERR_PROG_BAD)

/* Command that can take an argument */
enum rarg {
	RARG_CONST,		// user visible constants
	RARG_CONST_CMPLX,
	RARG_CONST_INT,		// like the above but for internal constants
	RARG_ERROR,
	/* STO and RCL must be in operator order */
	RARG_STO, RARG_STO_PL, RARG_STO_MI, RARG_STO_MU, RARG_STO_DV,
			RARG_STO_MIN, RARG_STO_MAX,
	RARG_RCL, RARG_RCL_PL, RARG_RCL_MI, RARG_RCL_MU, RARG_RCL_DV,
			RARG_RCL_MIN, RARG_RCL_MAX,
	RARG_SWAPX, RARG_SWAPY, RARG_SWAPZ, RARG_SWAPT,
	RARG_CSTO, RARG_CSTO_PL, RARG_CSTO_MI, RARG_CSTO_MU, RARG_CSTO_DV,
	RARG_CRCL, RARG_CRCL_PL, RARG_CRCL_MI, RARG_CRCL_MU, RARG_CRCL_DV,
	RARG_CSWAPX, RARG_CSWAPZ,
	RARG_VIEW,
	RARG_STOSTK, RARG_RCLSTK,

	RARG_ALPHA, RARG_AREG, RARG_ASTO, RARG_ARCL,
	RARG_AIP, RARG_ALRL, RARG_ALRR, RARG_ALSL, RARG_ALSR,

	RARG_TEST_EQ, RARG_TEST_NE, RARG_TEST_APX,	/* Must be in the same order as enum tst_op */
			RARG_TEST_LT, RARG_TEST_LE,
			RARG_TEST_GT, RARG_TEST_GE,
			
	RARG_TEST_ZEQ, RARG_TEST_ZNE, //RARG_TEST_ZAPX,
	RARG_SKIP, RARG_BACK,
	RARG_DSE, RARG_ISG,
	RARG_DSL, RARG_ISE,
	RARG_DSZ, RARG_ISZ,
	RARG_DEC, RARG_INC,

	/* These 8 must be sequential and in the same order as the DBL_ commands */
	RARG_LBL, RARG_LBLP, RARG_XEQ, RARG_GTO,
	RARG_SUM, RARG_PROD, RARG_SOLVE, RARG_DERIV, RARG_2DERIV,
	RARG_INTG,

	RARG_STD, RARG_FIX, RARG_SCI, RARG_ENG, RARG_DISP,

	RARG_SF, RARG_CF, RARG_FF, RARG_FS, RARG_FC,
	RARG_FSC, RARG_FSS, RARG_FSF, RARG_FCC, RARG_FCS, RARG_FCF,

	RARG_WS,

	RARG_RL, RARG_RR, RARG_RLC, RARG_RRC,
	RARG_SL, RARG_SR, RARG_ASR,
	RARG_SB, RARG_CB, RARG_FB, RARG_BS, RARG_BC,
	RARG_MASKL, RARG_MASKR,

	RARG_BASE,

	RARG_CONV,

	RARG_INISOLVE, RARG_SOLVESTEP,

	RARG_PAUSE, RARG_KEY,
	RARG_ALPHAXEQ, RARG_ALPHAGTO,

	RARG_PLOAD, RARG_PSAVE, RARG_PSWAP,
	RARG_FLRCL, RARG_FLRCL_PL, RARG_FLRCL_MI, RARG_FLRCL_MU, RARG_FLRCL_DV,
			RARG_FLRCL_MIN, RARG_FLRCL_MAX,
	RARG_FLCRCL, RARG_FLCRCL_PL, RARG_FLCRCL_MI, RARG_FLCRCL_MU, RARG_FLCRCL_DV,
	RARG_SLD, RARG_SRD,

	RARG_VIEW_REG,
	RARG_ROUNDING,

#ifdef INCLUDE_USER_MODE
	RARG_SAVEM, RARG_RESTM,
#endif
#ifdef INCLUDE_MULTI_DELETE
	RARG_DELPROG,
#endif
	RARG_SENDL,
	RARG_PUTKEY,
	RARG_KEYTYPE,
	RARG_MESSAGE,

	RARG_LOCAL,

	NUM_RARG	// Last entry defines number of operations
};


// Special functions
enum specials {
	OP_ENTER=0,
	OP_CLX, OP_EEX, OP_CHS, OP_DOT,
	OP_0, OP_1, OP_2, OP_3, OP_4, OP_5, OP_6, OP_7, OP_8, OP_9,
			OP_A, OP_B, OP_C, OP_D, OP_E, OP_F,
	OP_SIGMAPLUS, OP_SIGMAMINUS,
	OP_Xeq0, OP_Xne0, OP_Xapx0, OP_Xlt0, OP_Xle0, OP_Xgt0, OP_Xge0,
	OP_Xeq1, OP_Xne1, OP_Xapx1, OP_Xlt1, OP_Xle1, OP_Xgt1, OP_Xge1,
	OP_Zeq0, OP_Zne0, OP_Zapx0,
	OP_Zeq1, OP_Zne1, OP_Zapx1,

	// These are pseudo ops that don't actually do anything outside the keyboard handler
	OP_SST, OP_BST, OP_BACKSPACE, OP_RUNNING, OP_IGNORE, OP_UNFINISHED,
	NUM_SPECIAL
};

// Double sized instructions
enum multiops {
	DBL_LBL=0, DBL_LBLP, DBL_XEQ, DBL_GTO,
	DBL_SUM, DBL_PROD, DBL_SOLVE, DBL_DERIV, DBL_2DERIV,
	DBL_INTG,
#ifdef MULTI_ALPHA
	DBL_ALPHA,
#endif
	//DBL_NUMBER,
	DBL_DELPROG,
	NUM_MULTI	// Last entry defines number of operations
};



// Error codes
enum errors {
	ERR_NONE = 0,
	ERR_DOMAIN,	ERR_BAD_DATE,	ERR_PROG_BAD,
	ERR_INFINITY,	ERR_MINFINITY,	ERR_NO_LBL,
	ERR_XROM_NEST,	ERR_RANGE,	ERR_DIGIT,
	ERR_TOO_LONG,	ERR_RAM_FULL,	ERR_STK_CLASH,
	ERR_BAD_MODE,	ERR_INT_SIZE,	ERR_MORE_POINTS,
	ERR_BAD_PARAM,  ERR_IO,		ERR_INVALID,
	ERR_READ_ONLY,	ERR_SOLVE,
#ifdef MATRIX_SUPPORT
	ERR_MATRIX_DIM,	ERR_SINGULAR,
#endif
	MAX_ERROR
};


// Integer modes
enum arithmetic_modes {
	MODE_2COMP=0,	MODE_1COMP,	MODE_UNSIGNED,	MODE_SGNMANT
};

enum integer_bases {
	MODE_DEC=0,	MODE_BIN,	MODE_OCT,	MODE_HEX
};

// Display modes
enum display_modes {
	MODE_STD=0,	MODE_FIX,	MODE_SCI,	MODE_ENG
};

// Single action display modes
enum single_disp {
	SDISP_NORMAL=0,
	SDISP_SHOW,
	SDISP_BIN,	SDISP_OCT,	SDISP_DEC,	SDISP_HEX
};

// Trig modes
enum trig_modes {
	TRIG_DEG=0,	TRIG_RAD,	TRIG_GRAD
};
extern enum trig_modes get_trig_mode(void);

// Date modes
enum date_modes {
	DATE_DMY=0,	DATE_YMD,	DATE_MDY
};

// Fraction denominator modes
enum denom_modes {
	DENOM_ANY=0,	DENOM_FIXED,	DENOM_FACTOR
};

enum sigma_modes {
	SIGMA_LINEAR=0,		SIGMA_EXP,
	SIGMA_POWER,		SIGMA_LOG,
	SIGMA_BEST,
	SIGMA_QUIET_LINEAR,	SIGMA_QUIET_POWER
};

enum catalogues 
{
	CATALOGUE_NONE=0,
	CATALOGUE_NORMAL,
	CATALOGUE_COMPLEX,
	CATALOGUE_STATS,
	CATALOGUE_PROB,
	CATALOGUE_INT,
	CATALOGUE_PROG,
	CATALOGUE_PROGXFCN,
	CATALOGUE_TEST,
	CATALOGUE_MODE,
	CATALOGUE_ALPHA,
	CATALOGUE_ALPHA_SYMBOLS,
	CATALOGUE_ALPHA_COMPARES,
	CATALOGUE_ALPHA_ARROWS, 
	CATALOGUE_ALPHA_LETTERS_UPPER,
	// CATALOGUE_ALPHA_LETTERS_LOWER,
	CATALOGUE_ALPHA_SUPERSCRIPTS,
	CATALOGUE_ALPHA_SUBSCRIPTS,
	CATALOGUE_CONST,
	CATALOGUE_COMPLEX_CONST,
	CATALOGUE_CONV,
#ifdef MATRIX_SUPPORT
	CATALOGUE_MATRIX,
#endif
#ifdef INCLUDE_INTERNAL_CATALOGUE
	CATALOGUE_INTERNAL,
#endif
	CATALOGUE_LABELS,
	CATALOGUE_REGISTERS,
	CATALOGUE_STATUS,
};

enum shifts {
	SHIFT_N = 0,
	SHIFT_F, SHIFT_G, SHIFT_H,
	SHIFT_LC_N, SHIFT_LC_G		// Two lower case planes
};


#define K_HEARTBEAT 99			// Pseudo key, "pressed" every 100ms
#define K_RELEASE 98			// Pseudo key, sent on key release

#define	MAX_LOCAL	100		// maximum number of local registers
#define MAX_LOCAL_DIRECT 16		// # of directly addressable local registers

/*
 *  All more or less persistent global data
 */
#include "data.h"

/*
 *  Function prototypes
 */
extern void err(const enum errors);
extern const char *pretty(unsigned char);
extern void prettify(const char *in, char *out);

extern const char *get_cmdline(void);

extern int is_intmode(void);
extern enum shifts cur_shift(void);
extern enum shifts reset_shift(void);

extern void init_state(void);
extern void reset_volatile_state(void);
extern void xeq(opcode);
extern void xeqprog(void);
extern void xeqone(char *);
extern void xeq_init(void);
extern void xeq_init_contexts(void);
extern int init_34s(void);
extern void process_keycode(int);
extern void set_entry(void);

#if 0
extern unsigned int state_pc(void);
#else
#define state_pc() (State.pc)
#endif
extern void set_pc(unsigned int);
extern unsigned int user_pc(void);
extern unsigned int find_user_pc(unsigned int);
extern int check_return_stack_segment(int);
extern int local_levels(void);

extern void clrretstk(int clr_pc);
extern void clrprog(void);
extern void clrall(decimal64 *a, decimal64 *b, enum nilop op);
extern void reset(decimal64 *a, decimal64 *b, enum nilop op);

extern opcode getprog(unsigned int n);
extern void stoprog(opcode);
extern void delprog(void);
extern void del_till_label(unsigned int);
extern void del_till_multi_label(unsigned int);
extern unsigned int inc(const unsigned int);
extern unsigned int dec(unsigned int);
extern int incpc(void);
extern void decpc(void);
extern unsigned int find_label_from(unsigned int, unsigned int, int);
extern void fin_tst(const int);

extern const char *prt(opcode, char *);
extern const char *catcmd(opcode, char *);

extern decNumber *getX(decNumber *x);
extern void getY(decNumber *y);
extern void setX(const decNumber *x);
extern void setY(const decNumber *x);

extern void getXY(decNumber *x, decNumber *y);
extern void getYZ(decNumber *x, decNumber *y);
extern void getXYZ(decNumber *x, decNumber *y, decNumber *z);
extern void getXYZT(decNumber *x, decNumber *y, decNumber *z, decNumber *t);
extern void setXY(const decNumber *x, const decNumber *y);

extern void setlastX(void);

extern int stack_size(void);
extern void lift(void);
extern void process_cmdline_set_lift(void);
extern void process_cmdline(void);

extern unsigned int alen(void);

extern long long int d64toInt(const decimal64 *n);
extern void d64fromInt(decimal64 *n, const long long int z);

extern int get_tag_n(int n);
extern void set_tag_n(int n, int tag);

extern decimal64 *get_reg_n(int);
extern long long int get_reg_n_as_int(int);
extern void put_reg_n_from_int(int, const long long int);
extern void get_reg_n_as_dn(int, decNumber *);
extern void put_reg_n(int, const decNumber *);
extern void swap_reg(decimal64 *, decimal64 *);

extern void reg_put_int(int, unsigned long long int, int);
extern unsigned long long int reg_get_int(int, int *);

extern void put_int(unsigned long long int, int, decimal64 *);
extern unsigned long long int get_int(const decimal64 *, int *);

extern void get_maxdenom(decNumber *);

extern int get_user_flag(int);
extern void set_user_flag(int);
extern void clr_user_flag(int);
extern void put_user_flag(int n, int f);
	
extern void *xcopy(void *, const void *, int);
extern void *xset(void *, const char, int);
extern char *find_char(const char *, const char);
extern char *scopy(char *, const char *);		// copy string return pointer to *end*
extern const char *sncopy(char *, const char *, int);	// copy string return pointer to *start*
extern char *scopy_char(char *, const char *, const char);
extern char *scopy_spc(char *, const char *);
extern char *sncopy_char(char *, const char *, int, const char);
extern char *sncopy_spc(char *, const char *, int);
extern int slen(const char *);

extern char *num_arg(char *, unsigned int);		// number, no leading zeros
extern char *num_arg_0(char *, unsigned int, int);	// n digit number, leading zeros

extern int is_even(const decNumber *x);
extern int s_to_i(const char *);
extern unsigned long long int s_to_ull(const char *, unsigned int);

extern void do_conv(decNumber *, unsigned int, const decNumber *);
extern enum display_modes std_round_fix(const decNumber *);

extern unsigned char remap_chars(unsigned char);
extern unsigned char keycode_to_row_column(const int c);
int row_column_to_keycode(const int c);


/* Control program execution */
extern void xeq_sst_bst(int kind);

/* Command functions */
extern void version(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void cmd_off(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void cmderr(unsigned int arg, enum rarg op);
extern void cmdmsg(unsigned int arg, enum rarg op);
extern void cpx_roll_down(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void cpx_roll_up(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void cpx_enter(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void cpx_fill(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void fill(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void drop(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void cmdconst(unsigned int arg, enum rarg op);
extern void cmdconstcmplx(unsigned int arg, enum rarg op);
extern void cmdconstint(unsigned int arg, enum rarg op);
extern void cmdsto(unsigned int arg, enum rarg op);
extern void cmdrcl(unsigned int arg, enum rarg op);
extern void cmdcsto(unsigned int arg, enum rarg op);
extern void cmdcrcl(unsigned int arg, enum rarg op);
extern void cmdswap(unsigned int arg, enum rarg op);
extern void cmdflashrcl(unsigned int arg, enum rarg op);
extern void cmdflashcrcl(unsigned int arg, enum rarg op);
extern void cmdview(unsigned int arg, enum rarg op);
extern void cmdsavem(unsigned int arg, enum rarg op);
extern void cmdrestm(unsigned int arg, enum rarg op);
extern void set_stack_size(decimal64 *a, decimal64 *nul2, enum nilop op);
extern void get_stack_size(decimal64 *a, decimal64 *nul2, enum nilop op);
extern void get_word_size(decimal64 *a, decimal64 *nul2, enum nilop op);
extern void get_sign_mode(decimal64 *a, decimal64 *nul2, enum nilop op);
extern void get_base(decimal64 *a, decimal64 *nul2, enum nilop op);
extern void get_mem(decimal64 *a, decimal64 *nul2, enum nilop op);
extern void cmdstostk(unsigned int arg, enum rarg op);
extern void cmdrclstk(unsigned int arg, enum rarg op);
extern void cmdgto(unsigned int arg, enum rarg op);
extern void cmdalphagto(unsigned int arg, enum rarg op);
extern void op_gtoalpha(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_xeqalpha(decimal64 *a, decimal64 *b, enum nilop op);
extern void cmdmultigto(const opcode o, enum multiops mopr);
extern void cmdlblp(unsigned int arg, enum rarg op);
extern void cmdmultilblp(const opcode o, enum multiops mopr);
extern void xromarg(unsigned int arg, enum rarg op);
extern void multixromarg(const opcode o, enum multiops mopr);
extern void cmddisp(unsigned int arg, enum rarg op);
extern void cmdskip(unsigned int arg, enum rarg op);
extern void cmdback(unsigned int arg, enum rarg op);
extern void cmdtest(unsigned int arg, enum rarg op);
extern void cmdztest(unsigned int arg, enum rarg op);
extern void cmdlincdec(unsigned int arg, enum rarg op);
extern void cmdloopz(unsigned int arg, enum rarg op);
extern void cmdloop(unsigned int arg, enum rarg op);
extern void cmdflag(unsigned int arg, enum rarg op);
extern void intws(unsigned int arg, enum rarg op);
extern void op_2frac(decimal64 *x, decimal64 *b, enum nilop op);
extern void op_fracdenom(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_denom(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_float(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_fract(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_trigmode(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_radix(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_thousands(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_fixscieng(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_pause(unsigned int arg, enum rarg op);
extern void op_intsign(decimal64 *a, decimal64 *b, enum nilop op);
extern void set_int_base(unsigned int arg, enum rarg op);
extern void op_locale(decimal64 *a, decimal64 *nul, enum nilop op);
extern void op_datemode(decimal64 *a, decimal64 *nul, enum nilop op);
extern void op_timemode(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_rtn(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_popusr(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_rs(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_prompt(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void do_usergsb(decimal64 *a, decimal64 *b, enum nilop op);
extern void do_userclear(decimal64 *a, decimal64 *b, enum nilop op);
extern void isTop(decimal64 *a, decimal64 *b, enum nilop op);
extern void XisInt(decimal64 *a, decimal64 *b, enum nilop op);
extern void XisEvenOrOdd(decimal64 *a, decimal64 *b, enum nilop op);
extern void XisPrime(decimal64 *a, decimal64 *b, enum nilop op);
extern void isSpecial(decimal64 *a, decimal64 *b, enum nilop op);
extern void isNan(decimal64 *a, decimal64 *b, enum nilop op);
extern void isInfinite(decimal64 *a, decimal64 *b, enum nilop op);
extern void check_zero(decimal64 *a, decimal64 *nul2, enum nilop op);
extern void op_entryp(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_regcopy(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_regswap(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_regclr(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_regsort(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void cmdconv(unsigned int arg, enum rarg op);
extern void roll_down(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void roll_up(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void clrx(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void clrstk(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void clrflags(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void clrreg(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void lead0(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void op_ticks(decimal64 *a, decimal64 *b, enum nilop op);
extern void op_voltage(decimal64 *a, decimal64 *b, enum nilop op);
extern void check_mode(decimal64 *a, decimal64 *nul2, enum nilop op);
extern void op_keyp(unsigned int arg, enum rarg op);
extern void op_shift_digit(unsigned int n, enum rarg op);
extern void op_roundingmode(decimal64 *, decimal64 *, enum nilop);
extern void rarg_roundingmode(unsigned int arg, enum rarg op);
extern void op_setspeed(decimal64 *, decimal64 *, enum nilop);
extern void op_putkey(unsigned int arg, enum rarg op);
extern void op_keytype(unsigned int arg, enum rarg op);
extern void op_local(unsigned int arg, enum rarg op);

extern void set_running_off_sst(void);
extern void set_running_on_sst(void);
extern void set_running_off(void);
extern void set_running_on(void);

extern decNumber *convC2F(decNumber *r, const decNumber *x);
extern decNumber *convF2C(decNumber *r, const decNumber *x);
extern decNumber *convDB2AR(decNumber *r, const decNumber *x);
extern decNumber *convAR2DB(decNumber *r, const decNumber *x);
extern decNumber *convDB2PR(decNumber *r, const decNumber *x);
extern decNumber *convPR2DB(decNumber *r, const decNumber *x);

extern void xrom_routines(decimal64 *a, decimal64 *b, enum nilop op);

extern void packed_from_number(decimal64 *r, const decNumber *x);
extern void int_mode_convert(decimal64 *r);

/* system functions */
extern void busy(void);
extern int is_key_pressed(void);
extern enum shifts shift_down(void);
extern int get_key(void);
extern int put_key(int k);
extern void shutdown(void);
#ifdef REALBUILD
extern void lock(void);
extern void unlock(void);
extern void watchdog(void);
extern void update_speed(int full);
extern void idle(void);
extern int is_debug(void);
extern int is_test_mode(void);
#else
#define lock()
#define unlock()
#define watchdog()
#define update_speed(full)
#define idle()
#define is_debug() 0
#define is_test_mode() 0
#endif

#endif

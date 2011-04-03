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
#define VERSION_STRING	"1.17"

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

enum rarg;
enum multiops;

/* Define some system flag to user flag mappings
 */
#define CARRY_FLAG	101	/* C = carry */
#define OVERFLOW_FLAG	100	/* B = excess/exceed */
#define NAN_FLAG	102	/* D = danger */

#define NAME_LEN	6	/* Length of command names */

typedef unsigned int opcode;
typedef unsigned short int s_opcode;

/* Table of monadic functions */
struct monfunc {
#ifdef DEBUG
	unsigned short n;
#endif
	decNumber *(*mondreal)(decNumber *, const decNumber *, decContext *);
	void (*mondcmplx)(decNumber *, decNumber *, const decNumber *, const decNumber *, decContext *);
	long long int (*monint)(long long int);
	const char fname[NAME_LEN];
};
extern const struct monfunc monfuncs[];
extern const unsigned short num_monfuncs;

/* Table of dyadic functions */
struct dyfunc {
#ifdef DEBUG
	unsigned short n;
#endif
	decNumber *(*dydreal)(decNumber *, const decNumber *, const decNumber *, decContext *);
	void (*dydcmplx)(decNumber *, decNumber *, const decNumber *, const decNumber*,
				const decNumber *, const decNumber *, decContext *);
	long long int (*dydint)(long long int, long long int);
	const char fname[NAME_LEN];
};
extern const struct dyfunc dyfuncs[];
extern const unsigned short num_dyfuncs;

/* Table of triadic functions */
struct trifunc {
#ifdef DEBUG
	unsigned short n;
#endif
	decNumber *(*trireal)(decNumber *, const decNumber *, const decNumber *, const decNumber *, decContext *);
	long long int (*triint)(long long int, long long int, long long int);
	const char fname[NAME_LEN];
};
extern const struct trifunc trifuncs[];
extern const unsigned short num_trifuncs;


/* Table of niladic functions */
struct niladic {
#ifdef DEBUG
	unsigned short n;
#endif
	void (*niladicf)(decimal64 *, decimal64 *, decContext *);
	unsigned int numresults : 2;
	const char nname[NAME_LEN];
};
extern const struct niladic niladics[];
extern const unsigned short num_niladics;


/* Table of argument taking commands */
struct argcmd {
#ifdef DEBUG
	unsigned short n;
#endif
	void (*f)(unsigned int, enum rarg);
	unsigned char lim;
	unsigned int indirectokay:1;
	unsigned int stckreg:1;
	unsigned int cmplx:1;
	const char cmd[NAME_LEN];
};
extern const struct argcmd argcmds[];
extern const unsigned short num_argcmds;

struct multicmd {
#ifdef DEBUG
	unsigned short n;
#endif
	void (*f)(opcode, enum multiops);
	const char cmd[NAME_LEN];
};
extern const struct multicmd multicmds[];
extern const unsigned short num_multicmds;


extern decContext *Ctx, *Ctx64;


/* Return the specified opcode in the position indicated in the current
 * catalogue.
 */
extern opcode current_catalogue(int);
extern int current_catalogue_max(void);


/* Allow the number of registers and the size of the stack to be changed
 * relatively easily.
 */
#define STACK_SIZE	8	/* Maximum depth of RPN stack */
#define EXTRA_REG	4
#define RET_STACK_SIZE	8	/* Depth of return stack */
#define NUMPROG		476	/* Number of program steps */
#define NUMLBL		103	/* Number of program labels */
#define NUMFLG		103	/* Number of flags */

#define NUMALPHA	31	/* Number of characters in Alpha */

#define CMDLINELEN	19	/* 12 mantissa + dot + sign + E + sign + 3 exponent = 19 */
#define NUMBANKREGS	5
#define NUMBANKFLAGS	16


/* Macros to access program ROM */
#define XROM_MASK	(0x4000u)
#define isXROM(pc)	((pc) & XROM_MASK)
#define addrXROM(pc)	((pc) | XROM_MASK)


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
	KIND_CDYA,
};

#define KIND_SHIFT	8
#define DBL_SHIFT	8

#define OP_SPEC	(KIND_SPEC << KIND_SHIFT)		/* Special operations */
#define OP_NIL	(KIND_NIL << KIND_SHIFT)		/* Niladic operations */
#define OP_MON	(KIND_MON << KIND_SHIFT)		/* Monadic operations */
#define OP_DYA	(KIND_DYA << KIND_SHIFT)		/* Dyadic operations */
#define OP_TRI	(KIND_TRI << KIND_SHIFT)		/* Dyadic operations */
#define OP_CMON	(KIND_CMON << KIND_SHIFT)		/* Complex Monadic operation */
#define OP_CDYA	(KIND_CDYA << KIND_SHIFT)		/* Complex Dyadic operaion */

#define OP_DBL	0x1000			/* Double sized instructions */

#define OP_RARG	0x8000			/* All operations that have a register like argument */
#define RARG_OPSHFT	8
#define RARG_MASK	0x7f
#define RARG_IND	0x80

#define isRARG(op)	((op) & OP_RARG)

#define opKIND(op)	((enum eKind)((op) >> KIND_SHIFT))
#define argKIND(op)	((op) & ((1 << KIND_SHIFT)-1))
#define isDBL(op)	(((op) & 0xf000) == OP_DBL)
#define opDBL(op)	(((op) >> DBL_SHIFT) & 0xf)

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

	OP_LN, OP_EXP, OP_SQRT, OP_RECIP,
	OP_LOG, OP_LG2, OP_2POWX, OP_10POWX,
	OP_LN1P, OP_EXPM1,
	OP_LAMW, OP_INVW,
	OP_SQR,
#ifdef INCLUDE_CUBES
	OP_CUBE, OP_CUBERT,
#endif

	OP_FIB,

	OP_2DEG, OP_2RAD, OP_2GRAD,

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
	OP_DEG2RAD, OP_RAD2DEG,
	OP_CCHS, OP_CCONJ,		// CHS and Conjugate
	OP_ERF,
	OP_pdf_Q, OP_cdf_Q, OP_qf_Q,
	OP_cdf_chi2, OP_qf_chi2,
	OP_cdf_T, OP_qf_T,
	OP_cdf_F, OP_qf_F,
	OP_cdf_WB, OP_qf_WB,
	OP_pdf_EXP, OP_cdf_EXP, OP_qf_EXP,
	OP_cdf_B, OP_qf_B,
	OP_cdf_P, OP_qf_P,
	OP_cdf_G, OP_qf_G,
	OP_pdf_N, OP_cdf_N, OP_qf_N,
	OP_pdf_LN, OP_cdf_LN, OP_qf_LN,
	OP_pdf_LG, OP_cdf_LG, OP_qf_LG,
	OP_pdf_C, OP_cdf_C, OP_qf_C,
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
#ifdef INCLUDE_EASTER
	OP_EASTER,
#endif
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
	OP_PERAD, OP_PERSB, OP_PERMG, OP_MARGIN,
	OP_PARAL,
#ifdef INCLUDE_AGM
	OP_AGM,
#endif
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
};  

// Niladic functions
enum {
	OP_NOP=0, OP_VERSION, OP_OFF,
	OP_STKSIZE, OP_STK4, OP_STK8, OP_INTSIZE,
	OP_LASTX, OP_LASTXY,
	OP_SWAP, OP_CSWAP, OP_RDOWN, OP_RUP, OP_CRDOWN, OP_CRUP,
	OP_CENTER, OP_FILL, OP_CFILL, OP_DROP, OP_DROPXY,
	OP_sigmaX, OP_sigmaY, OP_sigmaX2, OP_sigmaY2, OP_sigma_XY, OP_sigmaX2Y,
	OP_sigmaN,
	OP_sigmalnX, OP_sigmalnXlnX, OP_sigmalnY, OP_sigmalnYlnY,
		OP_sigmalnXlnY, OP_sigmaXlnY, OP_sigmaYlnX,
	OP_statS, OP_statSigma, OP_statGS, OP_statGSigma,
		OP_statWS, OP_statWSigma,
		OP_statMEAN, OP_statWMEAN, OP_statGMEAN,
		OP_statR, OP_statLR,
		OP_statSErr, OP_statGSErr, OP_statWSErr,
	OP_EXPF, OP_LINF, OP_LOGF, OP_PWRF, OP_BEST,
	OP_RANDOM, OP_STORANDOM,
	OP_DEG, OP_RAD, OP_GRAD,
	OP_ALL, OP_RTN, OP_RTNp1,
	OP_RS, OP_PROMPT,
	OP_SIGMACLEAR, OP_CLREG, OP_CLSTK, OP_CLALL, OP_RESET, OP_CLFLAGS,
	OP_R2P, OP_P2R,
	OP_FRACDENOM, OP_2FRAC, OP_DENFIX, OP_DENFAC, OP_DENANY,
	OP_FRACIMPROPER, OP_FRACPROPER,
	OP_RADDOT, OP_RADCOM, OP_THOUS_ON, OP_THOUS_OFF,
	OP_FIXSCI, OP_FIXENG,
	OP_2COMP, OP_1COMP, OP_UNSIGNED, OP_SIGNMANT,
	OP_FLOAT, OP_FRACT,
	OP_LEAD0, OP_TRIM0,
	OP_LJ, OP_RJ,
	OP_DBL_MUL,
	OP_RCLSIGMA,
	OP_DATEYMD, OP_DATEDMY, OP_DATEMDY, OP_ISLEAP,
	OP_ALPHADAY, OP_ALPHAMONTH, OP_ALPHADATE, OP_ALPHATIME,
	OP_DATE, OP_TIME, OP_24HR, OP_12HR,
	OP_SETDATE, OP_SETTIME,
	OP_CLRALPHA, OP_VIEWALPHA, OP_ALPHALEN,
	OP_ALPHATOX, OP_XTOALPHA, OP_ALPHAON, OP_ALPHAOFF,
	OP_REGCOPY, OP_REGSWAP, OP_REGCLR, OP_REGSORT,
	OP_RCLFLAG, OP_STOFLAG,
	OP_GSBuser,
	OP_XisInf, OP_XisNaN, OP_XisSpecial, OP_XisPRIME,
	OP_XisINT, OP_XisFRAC, OP_XisEVEN, OP_XisODD,

	OP_TVM,
};


/* Command that can take an argument */
enum rarg {
	RARG_CONST,		// user visible constants
	RARG_CONST_CMPLX,
	RARG_CONST_INT,		// like the above but for intenal constants
	RARG_ERROR,
	/* STO and RCL must be in operator order */
	RARG_STO, RARG_STO_PL, RARG_STO_MI, RARG_STO_MU, RARG_STO_DV,
			RARG_STO_MIN, RARG_STO_MAX,
	RARG_RCL, RARG_RCL_PL, RARG_RCL_MI, RARG_RCL_MU, RARG_RCL_DV,
			RARG_RCL_MIN, RARG_RCL_MAX,
	RARG_SWAP,
	RARG_CSTO, RARG_CSTO_PL, RARG_CSTO_MI, RARG_CSTO_MU, RARG_CSTO_DV,
	RARG_CRCL, RARG_CRCL_PL, RARG_CRCL_MI, RARG_CRCL_MU, RARG_CRCL_DV,
	RARG_CSWAP,
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
	RARG_DSZ, RARG_ISZ,
	RARG_DEC, RARG_INC,

	/* These 8 must be sequential and in the same order as the DBL_ commands */
	RARG_LBL, RARG_LBLP, RARG_XEQ, RARG_GTO,
	RARG_SUM, RARG_PROD, RARG_SOLVE, RARG_DERIV, RARG_2DERIV,
	RARG_INTG,

	RARG_FIX, RARG_SCI, RARG_ENG, RARG_DISP,

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

	RARG_PAUSE,
#ifdef REALBUILD
	RARG_CONTRAST,
#endif
};
#define RARG(op, n)	(OP_RARG | ((op) << RARG_OPSHFT) | (n))


// Special functions
enum specials {
	OP_ENTER=0,
	OP_CLX, OP_EEX, OP_CHS, OP_DOT,
	OP_0, OP_1, OP_2, OP_3, OP_4, OP_5, OP_6, OP_7, OP_8, OP_9,
			OP_A, OP_B, OP_C, OP_D, OP_E, OP_F,
	OP_SIGMAPLUS, OP_SIGMAMINUS,
	OP_Xeq0, OP_Xne0, OP_Xapx0, OP_Xlt0, OP_Xgt0, OP_Xle0, OP_Xge0,
	OP_Xeq1, OP_Xne1, OP_Xapx1, OP_Xlt1, OP_Xgt1, OP_Xle1, OP_Xge1,
	OP_Zeq0, OP_Zne0, OP_Zapx0,
	OP_Zeq1, OP_Zne1, OP_Zapx1,
	SPECIAL_MAX
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
};



// Error codes
enum errors {
	ERR_NONE = 0,
	ERR_DOMAIN,	ERR_BAD_DATE,	ERR_PROG_BAD,
	ERR_INFINITY,	ERR_MINFINITY,	ERR_NO_LBL,
	ERR_XROM_NEST,	ERR_RANGE,	ERR_DIGIT,
	ERR_TOO_LONG,	ERR_XEQ_NEST,	ERR_STK_CLASH,
	ERR_BAD_MODE,	ERR_INT_SIZE,	ERR_MORE_POINTS,
	ERR_BAD_PARAM,
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
	SDISP_BIN,	SDISP_OCT,	SDISP_DEC,	SDISP_HEX,
};

// Trig modes
enum trig_modes {
	TRIG_DEG=0,	TRIG_RAD,	TRIG_GRAD
};
extern enum trig_modes get_trig_mode(void);
extern void set_trig_mode(enum trig_modes m);

// Date modes
enum date_modes {
	DATE_YMD=0,	DATE_DMY,	DATE_MDY
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
	CATALOGUE_TEST,
	CATALOGUE_MODE,
	CATALOGUE_ALPHA,
	CATALOGUE_ALPHA_SYMBOLS,
	CATALOGUE_ALPHA_COMPARES,
	CATALOGUE_ALPHA_ARROWS, 
	CATALOGUE_ALPHA_LETTERS_UPPER,
	CATALOGUE_ALPHA_LETTERS_LOWER,
	CATALOGUE_ALPHA_SUPERSCRIPTS,
	CATALOGUE_ALPHA_SUBSCRIPTS,
	CATALOGUE_CONST,
	CATALOGUE_COMPLEX_CONST,
	CATALOGUE_CONV,
#ifdef INCLUDE_INTERNAL_CATALOGUE
	CATALOGUE_INTERNAL,
#endif
};

enum shifts {
	SHIFT_N = 0,
	SHIFT_F, SHIFT_G, SHIFT_H,
	SHIFT_LC_N, SHIFT_LC_G		// Two lower case planes
};


#define K_HEARTBEAT 99			// Pseudo key, "pressed" every 100ms

#define MAGIC_MARKER 0x1357fdb9

#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif

struct _state {
	unsigned long int magic;	// Magic marker to detect failed RAM

// User noticable state
#define SB(f, p)	unsigned int f : p
#include "statebits.h"
#undef SB

	unsigned int base : 8;		// Base value for a command with an argument

	unsigned int digval2 : 8;
	unsigned int digval : 10;
	unsigned int numdigit : 4;
	unsigned int shifts : 2;

	unsigned int denom_mode : 2;	// Fractions denominator mode
	unsigned int denom_max : 14;	// Maximum denominator

	unsigned int last_prog : 9;	// Position of the last program statement
	unsigned int int_len : 6;	// Length of Integers
	unsigned int intm : 1;		// In integer mode

	unsigned int state_pc : 15;	// XEQ internal - don't use
	unsigned int state_lift : 1;	// XEQ internal - don't use

	unsigned int retstk_ptr : 4;	// XEQ internal - don't use
	unsigned int usrpc : 9;		// XEQ internal - don't use
	unsigned int smode : 3;		// Single short display mode

	unsigned int catalogue : 5;	// In catalogue mode
	unsigned int int_base : 4;	// Integer input/output base
	unsigned int test : 3;		// Waiting for a test command entry
	unsigned int int_window : 3;	// Which window to display 0=rightmost
	unsigned int gtodot : 1;	// GTO . sequence met

	unsigned int eol : 5;		// XEQ internal - don't use
	unsigned int cmdlineeex : 5;	// XEQ internal - don't use
	unsigned int cmdlinedot : 2;	// XEQ internal - don't use
	unsigned int state_running : 1;	// XEQ internal - don't use

	unsigned int alphas : 1;	// Alpha shift key pressed
	unsigned int cmplx : 1;		// Complex prefix pressed
	unsigned int wascomplex : 1;	// Previous operation was complex

	unsigned int arrow : 1;		// Conversion in progress
	unsigned int multi : 1;		// Multi-word instruction being entered
	unsigned int alphashift : 1;	// Alpha shifted to lower case
	unsigned int version : 1;	// Version display mode
	unsigned int implicit_rtn : 1;	// End of program is an implicit return
	unsigned int hyp : 1;		// Entering a HYP or HYP-1 operation
	unsigned int confirm : 2;	// Confirmation of operation required

	unsigned int dot : 1;		// misc use
	unsigned int improperfrac : 1;	// proper or improper fraction display
	unsigned int nothousands : 1;	// , or nothing for thousands separator
	unsigned int ind : 1;		// Indirection STO or RCL
	unsigned int arrow_alpha : 1;	// display alpha conversion
	unsigned int rarg : 1;		// In argument accept mode
	unsigned int runmode : 1;	// Program mode or run mode
	unsigned int flags : 1;		// Display state flags

	unsigned int disp_small : 1;	// Display the status message in small font
	unsigned int int_maxw : 3;	// maximum available window

	unsigned int hms : 1;		// H.MS mode
	unsigned int fract : 1;		// Fractions mode
	unsigned int fixeng : 1;	// Fix flips to ENG instead of SCI
	unsigned int leadzero : 1;	// forced display of leading zeros in int mode

#ifndef REALBUILD
	unsigned int trace : 1;
#else
	unsigned int testmode : 1;
#endif
	unsigned int error : 5;		// Did an error occur, if so what code?
	unsigned int status : 4;	// display status screen line

	unsigned int LowPowerCount : 16;

	unsigned int contrast : 4;	// Display contrast
	unsigned int off : 1;
	unsigned int LowPower : 1;	// low power detected

	unsigned int pause : 7;         // count down for programmed pause
	unsigned int busy_blink : 1;    // short blink of PRN annunciator with every key
	unsigned int show_register : 7; // temporary display (not X)

};

#ifdef WIN32
#pragma pack(pop)
#endif

typedef struct _ram {

	/* 
	 * Define storage for the machine's registers.
	 */
	decimal64 _regs[NUMREG];
	decimal64 _bank_regs[NUMBANKREGS];

	/*
	 * Define storage for the machine's program space.
	 */
	s_opcode _prog[NUMPROG];

	/*
	 * Generic state
	 */
	struct _state _state;

	/*
	 * What to display in message area
	 */
	const char *_disp_msg;

	/*
	 * Random number seeds
	 */
	unsigned long int _rand_s1, _rand_s2, _rand_s3;

	/* 
	 * The program return stack 
	 */
	unsigned short int _retstk[RET_STACK_SIZE];

	/* 
	 * Storage space for our user flags 
	 */
	unsigned short _bank_flags;
	unsigned char _user_flags[(NUMFLG+7) >> 3];

	/*
	 *  Alpha register gets its own space 
	 */
	char _alpha[NUMALPHA+1];

	/*
	 *  What the user was just typing in
	 */
	char _cmdline[CMDLINELEN + 1];

	/*
	 *  A ticker, incremented every 100ms
	 *  This should never overflow
	 */
	volatile long long _ticker;

	/*
	 *  Another ticker wich is reset on every keystroke
	 *  In fact, it counts the time between keystrokes
	 */
	volatile unsigned short _keyticks;

} TPersistentRam;

extern TPersistentRam PersistentRam;

#define State		(PersistentRam._state)
#define DispMsg		(PersistentRam._disp_msg)
#define Alpha		(PersistentRam._alpha)
#define Regs		(PersistentRam._regs)
#define BankRegs	(PersistentRam._bank_regs)
#define BankFlags	(PersistentRam._bank_flags)
#define UserFlags	(PersistentRam._user_flags)
#define Cmdline		(PersistentRam._cmdline)
#define RetStk		(PersistentRam._retstk)
#define RandS1		(PersistentRam._rand_s1)
#define RandS2		(PersistentRam._rand_s2)
#define RandS3		(PersistentRam._rand_s3)
#define Ticker      (PersistentRam._ticker)
#define Keyticks    (PersistentRam._keyticks)

extern void err(const enum errors);
extern const char *pretty(unsigned char);

extern const char *get_cmdline(void);

extern int is_intmode(void);
extern enum shifts cur_shift(void);
extern void set_shift(enum shifts);

extern void reset_volatile_state(void);
extern void xeq(opcode);
extern void xeqprog(void);
extern void xeqone(char *);
extern void xeq_init(void);
extern void xeq_init_contexts(void);
extern void init_34s(void);
extern void process_keycode(int);

extern unsigned int state_pc(void);
extern void set_pc(unsigned int);

extern void clrprog(void);
extern void clrall(decimal64 *a, decimal64 *b, decContext *nulc);
extern void reset(decimal64 *a, decimal64 *b, decContext *nulc);

extern opcode getprog(unsigned int n);
extern void stoprog(opcode);
extern void delprog(void);
extern unsigned int inc(const unsigned int);
extern unsigned int dec(unsigned int);
extern int incpc(void);
extern void decpc(void);
extern unsigned int find_label_from(unsigned int, unsigned int, int);
extern void fin_tst(const int);
extern unsigned int checksum_code(void);

extern unsigned int get_bank_flags(void);
extern void set_bank_flags(unsigned int);

extern const char *prt(opcode, char *);
extern const char *catcmd(opcode, char *);

extern void getX(decNumber *x);
extern void getY(decNumber *y);
extern void setX(const decNumber *x);

extern void getXY(decNumber *x, decNumber *y);
extern void getYZ(decNumber *x, decNumber *y);
extern void setXY(const decNumber *x, const decNumber *y);

extern void setlastX(void);

extern int stack_size(void);
extern void lift(void);
extern void process_cmdline_set_lift(void);

extern long long int d64toInt(const decimal64 *n);
extern void d64fromInt(decimal64 *n, const long long int z);

extern decimal64 *get_reg_n(int);
extern long long int get_reg_n_as_int(int);
extern void put_reg_n_from_int(int, const long long int);
extern void get_reg_n_as_dn(int, decNumber *);
extern void put_reg_n(int, const decNumber *);
extern void swap_reg(decimal64 *, decimal64 *);

extern void reg_put_int(int, unsigned long long int, int);
//extern unsigned long long int reg_get_int(int, int *);

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

extern int s_to_i(const char *);
unsigned long long int s_to_ull(const char *, unsigned int);

extern void do_conv(decNumber *, unsigned int, const decNumber *, decContext *);

extern unsigned char remap_chars(unsigned char);

/* Control program execution */
extern int running(void);
extern void xeq_sst(char *tracebuf);
extern void xeq_bst(char *tracebuf);

/* Command functions */
extern void version(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void cmd_off(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void cmderr(unsigned int arg, enum rarg op);
extern void lastX(decimal64 *x, decimal64 *nul, decContext *ctx64);
extern void lastXY(decimal64 *x, decimal64 *nul, decContext *ctx64);
extern void cpx_roll_down(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void cpx_roll_up(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void swap(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void cpx_swap(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void cpx_enter(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void cpx_fill(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void fill(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void drop(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void dropxy(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void cmdconst(unsigned int arg, enum rarg op);
extern void cmdconstcmplx(unsigned int arg, enum rarg op);
extern void cmdconstint(unsigned int arg, enum rarg op);
extern void cmdsto(unsigned int arg, enum rarg op);
extern void cmdrcl(unsigned int arg, enum rarg op);
extern void cmdcsto(unsigned int arg, enum rarg op);
extern void cmdcrcl(unsigned int arg, enum rarg op);
extern void cmdswap(unsigned int arg, enum rarg op);
extern void cmdview(unsigned int arg, enum rarg op);
extern void set_stack_size4(decimal64 *a, decimal64 *nul2, decContext *ctx64);
extern void set_stack_size8(decimal64 *a, decimal64 *nul2, decContext *ctx64);
extern void get_stack_size(decimal64 *a, decimal64 *nul2, decContext *ctx64);
extern void get_word_size(decimal64 *a, decimal64 *nul2, decContext *ctx64);
extern void cmdstostk(unsigned int arg, enum rarg op);
extern void cmdrclstk(unsigned int arg, enum rarg op);
extern void cmdgto(unsigned int arg, enum rarg op);
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
extern void op_2frac(decimal64 *x, decimal64 *b, decContext *ctx64);
extern void op_fracdenom(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_denany(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_denfix(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_denfac(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_float(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_fract(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_fracimp(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_fracpro(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_deg(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_rad(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_grad(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_all(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_radixcom(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_radixdot(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_thousands_off(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_thousands_on(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_fixsci(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_fixeng(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_pause(unsigned int arg, enum rarg op);
extern void op_2comp(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_1comp(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_unsigned(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_signmant(decimal64 *a, decimal64 *b, decContext *nulc);
extern void set_int_base(unsigned int arg, enum rarg op);
extern void date_ymd(decimal64 *a, decimal64 *nul, decContext *ctx);
extern void date_dmy(decimal64 *a, decimal64 *nul, decContext *ctx);
extern void date_mdy(decimal64 *a, decimal64 *nul, decContext *ctx);
extern void time_24(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void time_12(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void op_rclflag(decimal64 *x, decimal64 *b, decContext *ctx64);
extern void op_stoflag(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void op_rtn(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_rtnp1(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_rs(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void op_prompt(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void do_usergsb(decimal64 *a, decimal64 *b, decContext *nulc);
extern void do_userclear(decimal64 *a, decimal64 *b, decContext *nulc);
extern void XisInt(decimal64 *a, decimal64 *b, decContext *nulc);
extern void XisFrac(decimal64 *a, decimal64 *b, decContext *nulc);
extern void XisEven(decimal64 *a, decimal64 *b, decContext *nulc);
extern void XisOdd(decimal64 *a, decimal64 *b, decContext *nulc);
extern void XisPrime(decimal64 *a, decimal64 *b, decContext *nulc);
extern void isSpecial(decimal64 *a, decimal64 *b, decContext *nulc);
extern void isNan(decimal64 *a, decimal64 *b, decContext *nulc);
extern void isInfinite(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_regcopy(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_regswap(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_regclr(decimal64 *a, decimal64 *b, decContext *nulc);
extern void op_regsort(decimal64 *nul1, decimal64 *nul2, decContext *nulc);
extern void cmdconv(unsigned int arg, enum rarg op);
extern void roll_down(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void roll_up(decimal64 *nul1, decimal64 *nul2, decContext *ctx64);
extern void clrstk(decimal64 *nul1, decimal64 *nul2, decContext *ctx);
extern void clrflags(decimal64 *nul1, decimal64 *nul2, decContext *ctx);
extern void clrreg(decimal64 *nul1, decimal64 *nul2, decContext *ctx);
extern void showlead0(decimal64 *nul1, decimal64 *nul2, decContext *ctx);
extern void hidelead0(decimal64 *nul1, decimal64 *nul2, decContext *ctx);

extern decNumber *convC2F(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *convF2C(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *convDB2AR(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *convAR2DB(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *convDB2PR(decNumber *r, const decNumber *x, decContext *ctx);
extern decNumber *convPR2DB(decNumber *r, const decNumber *x, decContext *ctx);

extern void xrom_tvm(decimal64 *a, decimal64 *b, decContext *nulc);

/* system functions */
extern int is_key_pressed(void);
extern void watchdog(void);
extern void shutdown(void);

#ifdef REALBUILD
extern void cmdcontrast(unsigned int arg, enum rarg op);
#endif

#endif

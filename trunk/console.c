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

/*
 *  This is the console emulator part
 */
#include <stdlib.h>
#include <stdio.h>

#include "xeq.h" 
#include "keys.h"
#include "display.h"
#include "lcd.h"
#include "int.h"
#include "consts.h"
#include "storage.h"

#include "catalogues.h"


#define CH_QUIT		'Q'
#define CH_TRACE	'T'
#define CH_FLAGS	'F'
#define CH_REFRESH	12	/* ^L */


static const char SvnRevision[ 12 ] = "$Rev:: 621 $";

/*
 *  PC keys to calculator keys
 */
static int remap(const int c) {
	switch (c) {
	case 'F':	return K_F;
	case 'G':	return K_G;
	case 'H':	return K_H;

	case 'q':	return K00;
	case 'w':	return K01;
	case 'B':	return K01;
	case 'e':	return K02;
	case 'C':	return K02;
	case 'r':	return K03;
	case 'D':	return K03;
	case 't':	return K04;
	case 'y':	return K05;

	case 'a':	return K10;
	case 's':	return K11;
	case 'd':	return K12;
	case 'f':	return K_F;
	case 'g':	return K_G;
	case 'h':	return K_H;

	case 'z':	return K20;
	case 'x':	return K20;
	case '\r':	return K20;
	case '\n':	return K20;
	case 'c':	return K21;
	case 'v':	return K22;
	case 'b':	return K23;
	case 'n':	return K24;	// Backspace
	case 127:	return K24;
	case 8:		return K24;

	case 'u':	return K30;
	case '7':	return K31;
	case '8':	return K32;
	case '9':	return K33;
	case '/':	return K34;

	case 'j':	return K40;
	case '4':	return K41;
	case '5':	return K42;
	case '6':	return K43;
	case '*':	return K44;

	case 'm':	return K50;
	case '1':	return K51;
	case '2':	return K52;
	case '3':	return K53;
	case '-':	return K54;

	case ' ':	return K60;	// ON
	case '0':	return K61;
	case '.':	return K62;
	case ',':	return K63;
	case '+':	return K64;
	}
	return K_UNKNOWN;
}


/* Mappings from our internal character codes to readable strings.
 * The first table is for characters below space and the second for those
 * >=127 (del).
 */
static const char *const map32[32] = {
	NULL,	"x-bar", "y-bar", "sqrt", "integral", "degree", "space", "grad",
	"+/-", "<=", ">=", "!=", "euro", "->", "<-", "v",
	"^", "f-shift", "g-shift", "h-shift", "cmplx", "O-slash", "o-slash", "<->",
	"sz", "x-hat", "y-hat", "sub-m", "times", "approx", "pound", "yen"
};

static const char *const maptop[129] = {
	"del",
	"ALPHA", "BETA", "GAMMA", "DELTA", "EPSILON", "ZETA", "ETA", "THETA",
	"IOTA", "KAPPA", "LAMBDA", "MU", "NU", "XI", "OMICRON", "PI",
	"RHO", "SIGMA", "TAU", "UPSILON", "PHI", "CHI", "PSI", "OMEGA",
	"sub-B", "sub-mu", "^2", "sub-infinity", "^x", "^-1", "h-bar", "infinity",
	"alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta",
	"iota", "kappa", "lambda", "mu", "nu", "xi", "omicron", "pi",
	"rho", "sigma", "tau", "upsilon", "phi", "chi", "psi", "omega",
	"sub-0", "sub-1", "sub-2", "sub-c", "sub-e", "sub-n", "sub-p", "sub-u",
	"A-grave", "A-acute", "A-tilde", "A-umlaut", "A-dot", "C-acute", "C-hook", "C-cedilla",
	"E-grave", "E-acute", "E-filde", "E-trema", "I-grave", "I-acute", "I-tilde", "I-trema",
	"N-tilde", "O-grave", "O-acute", "O-tilde", "O-umlaut", "R-hook", "S-hook", "sub-A",
	"U-grave", "U-acute", "U-tilde", "U-umlaut", "U-dot", "Y-acute", "Y-trema", "Z-hook",
	"a-grave", "a-acute", "a-tilde", "a-umlaut", "a-dot", "c-acute", "c-hook", "c-cedilla",
	"e-grave", "e-acute", "e-tilde", "e-trema", "i-grave", "i-acute", "i-tilde", "i-trema",
	"n-tilde", "o-grave", "o-acute", "o-tilde", "o-umlaut", "r-hook", "s-hook", "sub-k",
	"u-grave", "u-acute", "u-tilde", "u-umlaut", "u-dot", "y-acute", "y-trema", "z-hook"
};

const char *pretty(unsigned char z) {
	if (z < 32)
		return map32[z & 0x1f];
	if (z >= 127)
		return maptop[z - 127];
	return NULL;
}

static void rarg_values(const opcode c, int l) {
	char bf[100];

	if (isRARG(c)) {
		const unsigned int cmd = (c & ~OP_RARG) >> RARG_OPSHFT;
		const unsigned int idx = c & 0x7f;

		if (cmd == RARG_CONV) {
			while (l-- > 0)
				putchar(' ');
			printf("%c %s", (idx & 1)?'*':'/', decimal64ToString(&CONSTANT_CONV(idx/2), bf));
		} else if (cmd == RARG_CONST || cmd == RARG_CONST_CMPLX) {
			while (l-- > 0)
				putchar(' ');
			printf("%s", decimal64ToString(&CONSTANT(c & 0x7f), bf));
		}
	}
}
	
static int dumpop(const opcode c, int pt) {
	char tracebuf[25];
	const char *s, *m;

	xset(tracebuf, '\0', sizeof(tracebuf));
	s = prt(c, tracebuf);
	if (s[0] != '?' || s[1] != '?' || s[2] != '?') {
		char t[100], *q = t;
		int l = 35;
		q += sprintf(t, "%04x  ", (unsigned int)c);
		while (*s != '\0') {
			const unsigned char z = 0xff & *s++;

			m = pretty(z);
			if (m == NULL) {
				*q++ = z;
				l--;
			} else {
				l -= slen(m) + 2;
				q += sprintf(q, "[%s]", m);
			}
		}
		*q++ = '\0';
		if (pt) {
			printf("%s", t);
			rarg_values(c, l);
			putchar('\n');
		}
		return 1;
	}
	return 0;
}


static void dump_menu(const char *name, const char *prefix, const enum catalogues cata) {
	int i;
	char cmd[16];
	const char *p;
	const char *buf;
	const char *m;
	const int oldcata = State2.catalogue;
	int n;

	State2.catalogue = cata;
	n = current_catalogue_max();
	printf("%s catalogue:\n", name);
	for (i=0; i<n; i++) {
		int l = 35 - slen(prefix);
		const opcode cati = current_catalogue(i);
		buf = catcmd(cati, cmd);

		printf("\t%d\t%s", i+1, prefix);
		for (p=buf; *p != '\0'; p++) {
			const unsigned char c = 0xff & *p;
			m = pretty(c);
			if (m == NULL) {
				printf("%c", c);
				l--;
			} else {
				printf("[%s]", m);
				l -= slen(m) + 2;
			}
		}
		rarg_values(cati, l);
		printf("\n");
	}
	printf("\n");
	State2.catalogue = oldcata;
}

#include "xrom.h"
static const struct {
	opcode op;
	const char *const name;
} xrom_labels[] = {
#define X(op, n, s)	{ RARG(RARG_ ## op, (n) & RARG_MASK), s},
#define XL(n, s)	X(LBL, n, s)
	XL(ENTRY_SIGMA, "Entry: SUMMATION")
	XL(ENTRY_PI, "Entry: PRODUCT")
	XL(ENTRY_SOLVE, "Entry: SOLVE")
	XL(ENTRY_INTEGRATE, "Entry: INTEGRATE")
	XL(ENTRY_DERIV, "Entry: DERIVATIVE")
	XL(ENTRY_2DERIV, "Entry: SECOND DERIVATIVE")
	XL(ENTRY_QUAD, "Entry: QUADRATIC SOLVER")
	XL(XROM_CHECK, "Internal: Common entry code")
	XL(XROM_EXIT, "Internal: Normal exit code")
	XL(XROM_EXITp1, "Internal: Abnormal exit code")
#undef XL
#define XG(n, s)	X(GTO, n, s)	X(XEQ, n, s)
	XG(XROM_CHECK, "-> Common entry code")
	XG(XROM_EXIT, "-> Normal exit code")
	XG(XROM_EXITp1, "-> Abnormal exit code")
#undef XG
#define XE(n, s)	X(ERROR, n, s)
	XE(ERR_DOMAIN, "Error: Domain Error")
	XE(ERR_BAD_DATE, "Error: Bad Date Error")
	XE(ERR_PROG_BAD, "Error: Undefined Op-code")
	XE(ERR_INFINITY, "Error: +infinity")
	XE(ERR_MINFINITY, "Error: -infinity")
	XE(ERR_NO_LBL, "Error: no such label")
	XE(ERR_XROM_NEST, "Error: Slv integrate sum product nested")
	XE(ERR_RANGE, "Error: out of range error")
	XE(ERR_DIGIT, "Error: bad digit error")
	XE(ERR_TOO_LONG, "Error: too long error")
	XE(ERR_XEQ_NEST, "Error: >8 levels nested")
	XE(ERR_STK_CLASH, "Error: stack clash")
	XE(ERR_BAD_MODE, "Error: bad mode error")
	XE(ERR_INT_SIZE, "Error: word size too small")
#undef XE
#undef X
};
#define num_xrom_labels		(sizeof(xrom_labels) / sizeof(*xrom_labels))

static void dump_xrom(void) {
	unsigned int pc = addrXROM(0);
	const unsigned int max = addrXROM(xrom_size);
	int dbl = 0, sngl = 0;

	printf("ADDR  MNEMONIC\t\tComment\n\n");
	do {
		char instr[16];
		const opcode op = getprog(pc);
		const char *p = prt(op, instr);
		int i;
		if (isDBL(op)) {
			dbl++;
			printf("%04x: %04x %04x  ", pc, op & 0xffff, (op >> 16)&0xffff);
		} else {
			sngl++;
			printf("%04x: %04x       ", pc, op);
		}
		//printf("%04x: %04x  ", pc, op);
		pc = inc(pc);
		while (*p != '\0') {
			char c = *p++;
			const char *q = pretty(c);
			if (q == NULL) putchar(c);
			else printf("[%s]", q);
		}
		for (i=0; i<num_xrom_labels; i++)
			if (xrom_labels[i].op == op)
				printf("\t\t%s", xrom_labels[i].name);
		putchar('\n');
	} while (pc != addrXROM(0));
	printf("%u XROM words\n%d single word instructions\n%d double word instructions\n", max-pc, sngl, dbl);
}

void shutdown( void )
{
	checksum_all();
	setuptty( 1 );
	save_statefile();
	exit( 0 );
}


/*
 *  Dummies
 */
int is_key_pressed(void) 
{
	return 0;
}

int get_key(void)
{
	return 0;
}

int put_key( int k )
{
	return k;
}

int is_shift_down(int s)
{
	return 0;
}


/*
 *  Main loop
 */
int main(int argc, char *argv[]) {
	int c, n = 0;
	decContext ctx;
	int warm = 0;

	Ctx = &ctx;
	xeq_init_contexts();
	if (argc > 1) {
		if (argc == 2) {
			if (argv[1][0] == 'x' && argv[1][1] == 'r' && argv[1][2] == 'o' && argv[1][3] == 'm' && argv[1][4] == '\0') {
				dump_xrom();
				return 0;
			}
			if (argv[1][0] == 'w' && argv[1][1] == 'a' && argv[1][2] == 'k' && argv[1][3] == 'e' && argv[1][4] == '\0') {
				warm = 1;
				goto skipargs;
			}
			dump_menu("float", "", CATALOGUE_NORMAL);
			dump_menu("complex", "[cmplx]", CATALOGUE_COMPLEX);
			dump_menu("statistics", "", CATALOGUE_STATS);
			dump_menu("probability", "", CATALOGUE_PROB);
			dump_menu("integer", "", CATALOGUE_INT);
			dump_menu("alpha", "", CATALOGUE_ALPHA);
			dump_menu("alpha special letters upper", "", CATALOGUE_ALPHA_LETTERS_UPPER);
			dump_menu("alpha special letters lower", "", CATALOGUE_ALPHA_LETTERS_LOWER);
			dump_menu("alpha superscripts", "", CATALOGUE_ALPHA_SUPERSCRIPTS);
			dump_menu("alpha subscripts", "", CATALOGUE_ALPHA_SUBSCRIPTS);
			dump_menu("alpha symbols", "", CATALOGUE_ALPHA_SYMBOLS);
			dump_menu("alpha compares", "", CATALOGUE_ALPHA_COMPARES);
			dump_menu("alpha arrows", "", CATALOGUE_ALPHA_ARROWS);
			dump_menu("programming", "", CATALOGUE_PROG);
			dump_menu("prog x.fcn", "", CATALOGUE_PROGXFCN);
			dump_menu("modes", "", CATALOGUE_MODE);
			dump_menu("test", "", CATALOGUE_TEST);
			dump_menu("conversions", "", CATALOGUE_CONV);
			dump_menu("constants", "# ", CATALOGUE_CONST);
			dump_menu("complex constants", "[cmplx]# ", CATALOGUE_COMPLEX_CONST);
#ifdef INCLUDE_INTERNAL_CATALOGUE
			dump_menu("internals", "", CATALOGUE_INTERNAL);
#endif
		}
		for (c=0; c<65536; c++) {
			if (isDBL(c) && (c & 0xff))	/* Don't show all multi-word instructions */
				continue;
			n += dumpop(c, argc > 2);
		}
		printf("total number of opcodes %d\n", n);
		printf("\tniladic commands %d\n", NUM_NILADIC);

		for (n=c=0; c<num_monfuncs; c++) {
			if (monfuncs[c].mondreal != NULL) n++;
			if (monfuncs[c].mondcmplx != NULL) n++;
			if (monfuncs[c].monint != NULL) n++;
		}
		printf("\tmonadic commands %d with %d functions\n", NUM_MONADIC, n);

		for (n=c=0; c<num_dyfuncs; c++) {
			if (dyfuncs[c].dydreal != NULL) n++;
			if (dyfuncs[c].dydcmplx != NULL) n++;
			if (dyfuncs[c].dydint != NULL) n++;
		}
		printf("\tdyadic commands %d with %d functions\n", NUM_DYADIC, n);

		for (n=c=0; c<num_trifuncs; c++) {
			if (trifuncs[c].trireal != NULL) n++;
			if (trifuncs[c].triint != NULL) n++;
		}
		printf("\ttriadic commands %d with %d functions\n", NUM_TRIADIC, n);

		printf("\targument commands %d\n", NUM_RARG);
		printf("\tmultiword commands %d\n", NUM_MULTI);
		printf("\tspecial commands %d\n", NUM_SPECIAL);

		return 0;
	}
skipargs:
	load_statefile();
	if (!warm)
		init_34s();
	if (setuptty(0) == 0) {
		display();
		while ((c = GETCHAR()) != GETCHAR_ERR && c != CH_QUIT) {
#ifdef USECURSES
			if (c == CH_TRACE) {
				State2.trace = 1 - State2.trace;
				display();
			} else if (c == CH_FLAGS) {
				State2.flags = 1 - State2.flags;
				display();
			} else if (c == CH_REFRESH) {
				clear();
				display();
			} else
#endif
				process_keycode(remap(c));
		}
		setuptty(1);
	}
	shutdown();
	return 0;
}

/*
 *  Tell the revision number (must not be optimised out!)
 */
const char *get_revision( void )
{
	return SvnRevision + 7;
}

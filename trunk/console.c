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
#include <ctype.h>
#include <string.h>

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


#include "pretty.c"

static void rarg_values(const opcode c, int l) {
	char bf[100];

	if (isRARG(c)) {
		const unsigned int cmd = RARG_CMD(c);
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

	if (c == RARG(RARG_ALPHA, 0))
		return 0;
	xset(tracebuf, '\0', sizeof(tracebuf));
	s = prt(c, tracebuf);
	if (strcmp(s, "???") != 0) {
		char t[100], *q = t;
		int l = 35;

		if (c == RARG(RARG_ALPHA, ' '))
			strcpy(tracebuf+2, "[space]");

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

		if (cati == RARG(RARG_ALPHA, ' '))
			strcpy(cmd+2, "[space]");

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

static void dump_code(unsigned int pc, unsigned int max, int annotate) {
	int dbl = 0, sngl = 0;

	printf("ADDR  OPCODE     MNEMONIC%s\n\n", annotate?"\tComment":"");
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
		if (op == RARG(RARG_ALPHA, ' '))
			strcpy(instr+2, "[space]");

		pc = inc(pc);
		while (*p != '\0') {
			char c = *p++;
			const char *q = pretty(c);
			if (q == NULL) putchar(c);
			else printf("[%s]", q);
		}
		if (annotate)
			for (i=0; i<num_xrom_labels; i++)
				if (xrom_labels[i].op == op)
					printf("\t\t%s", xrom_labels[i].name);
		putchar('\n');
	} while (! PcWrapped);
	if (annotate)
		printf("%u XROM words\n%d single word instructions\n%d double word instructions\n", max-pc, sngl, dbl);
}

static void dump_xrom(void) {
	dump_code(addrXROM(0), addrXROM(xrom_size), 1);
}

static void dump_ram(void) {
	if (LastProg > 1)
		dump_code(1, LastProg, 0);
	else
		printf("no RAM program\n");
}

static void dump_prog(unsigned int n) {
	unsigned int pc;
	if (n > NUMBER_OF_FLASH_REGIONS-1)
		printf("no such program region %u\n", n);
	else if (! is_prog_region(n+1))
		printf("region %u is not a program region\n", n);
	else if (sizeLIB(n+1) == 0)
		printf("region %u empty\n", n);
	else {
		pc = addrLIB(0, n+1);
		dump_code(pc, pc + sizeLIB(n+1), 0);
	}
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

#ifndef WIN32  // Windows uses winserial.c
/*
 *  Open a COM port for transmission
 */
int open_port( int baud, int bits, int parity, int stopbits )
{
	return 0;
}


/*
 *  Close the COM port after transmission is complete
 */
extern void close_port( void )
{
}


/*
 *  Output a single byte to the serial
 */
void put_byte( unsigned char byte )
{
	err( ERR_PROG_BAD );
}


/*
 *  Force buffer flush
 */
void flush_comm( void )
{
}

#endif

/*
 *  Main loop
 */
int main(int argc, char *argv[]) {
	int c, n = 0;
	int warm = 0;

	xeq_init_contexts();
	load_statefile();
	if (argc > 1) {
		if (argc == 2) {
			if (strcmp(argv[1], "xrom") == 0) {
				dump_xrom();
				return 0;
			}
			if (strcmp(argv[1], "ram") == 0) {
				dump_ram();
				return 0;
			}
			if (argv[1][0] == 'p' && argv[1][1] == 'r' && argv[1][2] == 'o' && argv[1][3] == 'g' && isdigit(argv[1][4]) && argv[1][5] == '\0') {
				dump_prog(argv[1][4] - '0');
				return 0;
			}
			if (strcmp(argv[1], "wake") == 0) {
				warm = 1;
				goto skipargs;
			}
			if (strcmp(argv[1], "opcodes") == 0) {
				dump_opcodes(stdout);
				return 0;
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
	if (!warm)
		init_34s();
	State2.flags = 1;
	if (setuptty(0) == 0) {
		display();
		just_displayed = 0;
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

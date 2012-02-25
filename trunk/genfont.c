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

/* Generate the bitmapped fonts */

#include <stdio.h>
#include <stdlib.h>
#include "licence.h"

/* Width 1 */
#define _        0

/* Width 2 */
#define __       0
#define O_       1

/* Width 3 */
#define ___      0
#define O__      1
#define _O_      2
#define OO_      3

/* Width 4 */
#define ____     0
#define O___     1
#define _O__     2
#define OO__     3
#define __O_     4
#define O_O_     5
#define _OO_     6
#define OOO_     7

/* Width 5 */
#define _____    0
#define O____    1
#define _O___    2
#define OO___    3
#define __O__    4
#define O_O__    5
#define _OO__    6
#define OOO__    7
#define ___O_    8
#define O__O_    9
#define _O_O_    10
#define OO_O_    11
#define __OO_    12
#define O_OO_    13
#define _OOO_    14
#define OOOO_    15

/* Width 6 */
#define ______   0
#define O_____   1
#define _O____   2
#define OO____   3
#define __O___   4
#define O_O___   5
#define _OO___   6
#define OOO___   7
#define ___O__   8
#define O__O__   9
#define _O_O__   10
#define OO_O__   11
#define __OO__   12
#define O_OO__   13
#define _OOO__   14
#define OOOO__   15
#define ____O_   16
#define O___O_   17
#define _O__O_   18
#define OO__O_   19
#define __O_O_   20
#define O_O_O_   21
#define _OO_O_   22
#define OOO_O_   23
#define ___OO_   24
#define O__OO_   25
#define _O_OO_   26
#define OO_OO_   27
#define __OOO_   28
#define O_OOO_   29
#define _OOOO_   30
#define OOOOO_   31


static struct s_charset {
	unsigned int octal;
	int width;
	const char *name;
	int sortas;
	int unicode;
	int windows;
	const char *uname;

	int bitrows[6];
} charset[512] = {
#include "font.inc"
};


static void gen_pretty(FILE *f) {
	int i;

	license(f, "/* ", " * ", " */");

	fprintf(f, "/* Mappings from our internal character codes to readable strings.\n"
		" * The first table is for characters below space and the second for those\n"
		" * >=127 (del).\n"
		" */\n"
		"static const char *const map32[32] = {\n\t"
			);
	for (i=0; i<32; i++) {
		if (charset[i].name == NULL) fprintf(f, "NULL");
		else fprintf(f, "\"%s\"", charset[i].name);
		if ((i % 8) == 7) fprintf(f, ",\n\t");
		else fprintf(f, ", ");
	}
	fprintf(f, "};\n\nstatic const char *const maptop[129] = {\n\t");
	for (i=127; i<256; i++) {
		fprintf(f, "\"%s\"", charset[i].name);
		if ((i % 8) == 7) fprintf(f, ",\n\t");
		else fprintf(f, ", ");
	}
	fprintf(f, "};\n\n");
}

static int map_sort(const void *v1, const void *v2) {
	const int i1 = *(const int *)v1;
	const int i2 = *(const int *)v2;

	if (i1 < i2)	return -1;
	if (i1 > i2)	return 1;
	return 0;
}

static void gen_charmap(FILE *f) {
	int i, j, n;
	int idx[256];

	for (i=0; i<256; i++)
		idx[i] = charset[i].sortas;
	qsort(idx, 256, sizeof(int), &map_sort);

	for (i=n=0; i<256; i++)
		if (idx[i] != idx[n])
			idx[++n] = idx[i];

	license(f, "/* ", " * ", " */");
	fprintf(f, "/*\n"
			" * Using a 256 element table to define equivalence classes is smaller\n"
			" * than a switch statement to catch the exception.  Lower case is for\n"
			" * Greek letters and the Roman letter doesn't correspond at all with the Greek.\n"
			" */\n\n"
			"/* Maximum distinguished value is %d */\n\n\n"
			"static const unsigned char character_remapping_order[256] = {\n\t", n);
	for (i=0; i<256; i++) {
		for (j=0; j<=n; j++)
			if (idx[j] == charset[i].sortas)
				goto skip;
		j = -1;
skip:
		fprintf(f, "\t% 4d,", j);
		if ((i%8) == 7)
			fprintf(f, "\t// %04o\n\t", i-7);
	}
	fprintf(f, "};\n\n\n"
			"/* Take a character and remap it into sort order.\n"
			" * Also cater for different cases, diacriticals and some\n"
			" * special symbols being identical from an ordering perspective.\n"
			" */\n"
			"unsigned char remap_chars(unsigned char ch) {\n"
			"	return character_remapping_order[ch & 0xff];\n"
			"}\n");
}

static void emit_bitmap(FILE *f, int beg, int end, const char *name) {
	int i, n, row;
	unsigned char buffer[10000];

	for (n=0, i=beg; i<end; i++)
		n += charset[i].width - 1;
	fprintf(f,	"\tstatic const unsigned char %s[6][%d] = {\n", name, (n+7)/8);
	for (row=0; row<6; row++) {
		int l = 0;
		int base = 0;
		int b;

		fprintf(f,	"\t\t{\n\t\t\t");
		for (i=0; i<10000; i++)
			buffer[i] = 0;
		for (i=beg; i<end; i++)
			for (b=0; b<charset[i].width-1; b++, base++)
				if (charset[i].bitrows[row] & (1 << b))
					buffer[base / 8] |= (1 << (base % 8));
		for (i=0; i< (n+7)/8; i++) {
			fprintf(f,	"%3u,%s", buffer[i], (l++ % 12) == 11 ? "\n\t\t\t" : " ");
		}
		fprintf(f,	"\n\t\t},\n");
	}
	fprintf(f,	"\t};\n");
}

static void gen_charset(FILE *f) {
	int i;
	int n=0, l=0;

	license(f, "/* ", " * ", " */");

	fprintf(f,	"/* Function to return the length of a specific character in pixels\n"
			" */\n\n"
			"unsigned int charlengths(unsigned int c) {\n"
			"\tstatic const unsigned char widths[%d] = {\n\t\t", (512 + 2) / 3);
	for (i=0; i<512; i++) {
		const int w = charset[i].width - 1;
		if (w < 0 || w > 5) {
			fprintf(stderr, "Error with character %d: width is out of range%d\n", i, charset[i].width);
			exit(1);
		}
		switch (i%3) {
		case 0:	n = w;		break;
		case 1:	n += w*6;	break;
		case 2:
			n += w*36;
			fprintf(f, "%3d,%s", n, (l++ % 12) == 11 ? "\n\t\t" : " ");
		}
	}
	fprintf(f, 	"%3d\n\t};\n"
			"\tstatic const unsigned char divs[3] = { 1, 6, 36 };\n"
			"\treturn (widths[c/3] / divs[c%%3]) %% 6 + 1;\n"
			"}\n\n",
		n);

	fprintf(f,	"void findlengths(unsigned short int posns[257], int smallp) {\n"
			"\tconst int mask = smallp ? 256 : 0;\n"
			"\tint i;\n\n"
			"\tposns[0] = 0;\n"
			"\tfor(i=0; i<256; i++)\n"
			"\t\tposns[i+1] = posns[i] + charlengths(i + mask) - 1;\n"
			"}\n\n");

	fprintf(f,	"void unpackchar(unsigned int c, unsigned char d[6], int smallp, const unsigned short int posns[257]) {\n");
	emit_bitmap(f, 0, 256, "bm_large");
	emit_bitmap(f, 256, 512, "bm_small");
	fprintf(f,	"\tunsigned int n = posns[c&255];\n"
			"\tconst unsigned int fin = posns[(c&255)+1];\n"
			"\tunsigned int i, j;\n\n"
			"\tfor(i=0; i<6; i++) d[i] = 0;\n"
			"\tfor (i=0; n < fin; i++,n++) {\n"
			"\t\tfor (j=0; j<6; j++) {\n"
			"\t\t\tconst unsigned char z = smallp ? (bm_small[j][n>>3]) : (bm_large[j][n>>3]);\n"
			"\t\t\tif (z & (1 << (n&7)))\n"
			"\t\t\t\td[j] |= 1 << i;\n"
			"\t\t}\n"
			"\t}\n"
			"}\n\n");
}

static void gen_translate(FILE *f) {
	int i;
	static unsigned char t[256] = {0};

	license(f, "/* ", " * ", " */");

	fprintf(f,	"/* Tables to map to Unicode and Windows CP1252 (sort of)\n"
			" */\n\n"
			"const int unicode[256] = {\n");
	for (i=0; i<256; i++) {
		fprintf( f, "\t0x%04x,\t// %s\n", charset[i].unicode, charset[i].name );
	}
	fprintf(f,	"};\n\n"
			"const unsigned char to_cp1252[256] = {\n");
	for (i=0; i<256; i++) {
		int w = charset[i].windows;
		fprintf( f, "\t0x%02x,\t// %s\n", w, charset[i].name );
		if (t[w] != 0) {
			fprintf( f, "\t// Above is duplicate: %02x\n", t[w] );
		}
		t[w] = (unsigned char) i;
	}
	fprintf(f,	"};\n\n"
			"const unsigned char from_cp1252[256] = {\n");
	for (i=0; i<256; i++) {
		fprintf( f, "\t0x%02x,\t// %s\n", t[i], charset[t[i]].name );
	}
	fprintf(f,	"};\n\n"
			"const char *charnames[256] = {\n");
	for (i=0; i<256; i++) {
		fprintf( f, "\t\"%s\",\n", charset[i].uname ? charset[i].uname : charset[i].name );
	}
	fprintf(f,	"};\n\n");
}

static int sanity_check(void) {
	unsigned int i;
	int err = 0;

	for (i=0; i<512; i++)
		if (charset[i].octal != i) {
			fprintf(stderr, "character %04o is tagged in position %04o\n", i, charset[i].octal);
			err = 1;
		}
	return err;
}

int main(int argc, char *argv[]) {
	FILE * f_pretty = fopen("pretty.h", "w");
	FILE * f_charset = fopen("font.c", "w");
	FILE * f_charmap = fopen("charmap.c", "w");
	FILE * f_translate = fopen("translate.c", "w");

	gen_pretty(f_pretty);
	fclose(f_pretty);

	gen_charset(f_charset);
	fclose(f_charset);

	gen_charmap(f_charmap);
	fclose(f_charmap);

	gen_translate(f_translate);
	fclose(f_translate);

	return sanity_check();
}


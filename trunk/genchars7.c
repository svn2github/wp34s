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

#include <stdio.h>
#include <ctype.h>
#include <string.h>


#define DIG(ch, bits) { ch, # bits }
;
static struct pair {
	unsigned char ch;
	const char * segs;
} digtbl[] = {
	DIG(' ',		0),
	DIG('-',		D_MIDDLE),
	DIG('0',		D_TOP | D_TL | D_TR | D_BL | D_BR | D_BOTTOM),
	DIG('1',		D_TR | D_BR),
	DIG('2',		D_TOP | D_TR | D_MIDDLE | D_BL | D_BOTTOM),
	DIG('3',		D_TOP | D_TR | D_MIDDLE | D_BR | D_BOTTOM),
	DIG('4',		D_TL | D_TR | D_MIDDLE | D_BR),
	DIG('5',		D_TOP | D_TL | D_MIDDLE | D_BR | D_BOTTOM),
	DIG('6',		D_TOP | D_TL | D_MIDDLE | D_BL | D_BR | D_BOTTOM),
	DIG('7',		D_TOP | D_TR | D_BR),
	DIG('8',		D_TOP | D_TL | D_TR | D_MIDDLE | D_BL | D_BR | D_BOTTOM),
	DIG('9',		D_TOP | D_TL | D_TR | D_MIDDLE | D_BR | D_BOTTOM),

	DIG('A',		D_TOP | D_TL | D_TR | D_MIDDLE | D_BL | D_BR),
	DIG('a',		D_TOP | D_TL | D_TR | D_MIDDLE | D_BL | D_BR),
	DIG('B',		D_TL | D_MIDDLE | D_BL | D_BR | D_BOTTOM),
	DIG('b',		D_TL | D_MIDDLE | D_BL | D_BR | D_BOTTOM),
	DIG('C',		D_TOP | D_TL | D_BL | D_BOTTOM),
	DIG('c',		D_MIDDLE | D_BL | D_BOTTOM),
	DIG('D',		D_TR | D_MIDDLE | D_BL | D_BR | D_BOTTOM),
	DIG('d',		D_TR | D_MIDDLE | D_BL | D_BR | D_BOTTOM),
	DIG('E',		D_TOP | D_TL | D_MIDDLE | D_BL | D_BOTTOM),
	DIG('e',		D_TOP | D_TL | D_MIDDLE | D_BL | D_BOTTOM),
	DIG('F',		D_TOP | D_TL | D_MIDDLE | D_BL),
	DIG('f',		D_TOP | D_TL | D_MIDDLE | D_BL),
	DIG('G',		D_TOP | D_TL | D_BL | D_BR | D_BOTTOM),
	DIG('g',		D_TOP | D_TL | D_BL | D_BR | D_BOTTOM),
	DIG('H',		D_TL | D_TR | D_MIDDLE | D_BL | D_BR),
	DIG('h',		D_TL | D_MIDDLE | D_BL | D_BR),
	DIG('I',		D_TL | D_BL),
	DIG('i',		D_BL),
	DIG('J',		D_TR | D_BL | D_BR | D_BOTTOM),
	DIG('j',		D_TR | D_BL | D_BR | D_BOTTOM),
	DIG('L',		D_TL | D_BL | D_BOTTOM),
	DIG('l',		D_TL | D_BL | D_BOTTOM),
	DIG('M',		D_TOP | D_TL | D_TR | D_BL),		// left half of M
	DIG('m',		D_TOP | D_TL | D_TR | D_BR),		// right half of M
	DIG('N',		D_TOP | D_TL | D_TR | D_BL | D_BR),
	DIG('n',		D_MIDDLE | D_BL | D_BR),
	DIG('O',		D_TOP | D_TL | D_TR | D_BL | D_BR | D_BOTTOM),
	DIG('o',		D_MIDDLE | D_BL | D_BR | D_BOTTOM),
	DIG('P',		D_TOP | D_TL | D_TR | D_MIDDLE | D_BL),
	DIG('p',		D_TOP | D_TL | D_TR | D_MIDDLE | D_BL),
	DIG('Q',		D_TOP | D_TL | D_TR | D_MIDDLE | D_BR),
	DIG('q',		D_TOP | D_TL | D_TR | D_MIDDLE | D_BR),
	DIG('R',		D_MIDDLE | D_BL),
	DIG('r',		D_MIDDLE | D_BL),
	DIG('S',		D_TOP | D_TL | D_MIDDLE | D_BR | D_BOTTOM),
	DIG('s',		D_TOP | D_TL | D_MIDDLE | D_BR | D_BOTTOM),
	DIG('T',		D_TL | D_MIDDLE | D_BL | D_BOTTOM),
	DIG('t',		D_TL | D_MIDDLE | D_BL | D_BOTTOM),
	DIG('U',		D_TL | D_TR | D_BL | D_BR | D_BOTTOM),
	DIG('u',		D_BL | D_BR | D_BOTTOM),
	DIG('W',		D_TL | D_BL | D_BR | D_BOTTOM),		// left half of W
	DIG('w',		D_TR | D_BL | D_BR | D_BOTTOM),		// also right half of W
	DIG('Y',		D_TL | D_TR | D_MIDDLE | D_BR | D_BOTTOM),
	DIG('y',		D_TL | D_TR | D_MIDDLE | D_BR | D_BOTTOM),

	DIG('@',		D_TOP | D_TL | D_TR | D_MIDDLE),	// degree
	DIG('\'',		D_TL),					// minute
	DIG('"',		D_TL | D_TR),				// second
	DIG('/',		D_TR | D_MIDDLE | D_BL),		// fraction vinculum
	DIG('<',		D_BL | D_BOTTOM),			// fraction continuation (left arrow)
	DIG('>',		D_BR | D_BOTTOM),			// right arrow
	DIG('_',		D_BOTTOM),
	DIG('=',		D_MIDDLE | D_BOTTOM),
	DIG('[',		D_TOP | D_TL | D_BL | D_BOTTOM),
	DIG(']',		D_TOP | D_TR | D_BR | D_BOTTOM),
	DIG('(',		D_TOP | D_TL | D_BL | D_BOTTOM),
	DIG(')',		D_TOP | D_TR | D_BR | D_BOTTOM),

	DIG(0,			0),					// In status display these are all
	DIG(1,			D_TOP),					// possible combinations of horozintal
	DIG(2,			D_MIDDLE),				// segments in a binary numbering pattern
	DIG(3,			D_TOP | D_MIDDLE),
	DIG(4,			D_BOTTOM),
	DIG(5,			D_TOP | D_BOTTOM),
	DIG(6,			D_MIDDLE | D_BOTTOM),
	DIG(7,			D_TOP | D_MIDDLE | D_BOTTOM),
	DIG(8,			D_TL | D_TR | D_BL | D_BR),		// Status central separator

	DIG(030,		D_MIDDLE | D_BL | D_BR | D_BOTTOM),	// small zero digit
	DIG(031,		D_BR),					// small one digit
};
#define NUMDIGS		(sizeof(digtbl) / sizeof(*digtbl))

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


int main() {
	const char *mapping[256];
	unsigned int i;
	unsigned int max = 0;
	char buf[1000];
	int empty = -1;

	for (i=0; i<255; i++)
		mapping[i] = "0";

	for (i=0; i<NUMDIGS; i++) {
		mapping[digtbl[i].ch] = digtbl[i].segs;
		if (max < digtbl[i].ch)
			max = digtbl[i].ch;
	}

	gpl_text("/* ", " * ", " */");
	printf("static const unsigned char digtbl[%u] = {\n", max+1);
	for (i=0; i<=max; i++) {
		if (isprint(i))
			sprintf(buf, " %c  ", i);
		else
			sprintf(buf, "0%03o", i);
		printf("\t%-60s/* %s */,\n", mapping[i], buf);
		if (strcmp(mapping[i], "0") == 0)
			empty++;
	}
	printf("};\n#define N_DIGTBL	(%u)\n\n", max+1);
	fprintf(stderr, "%d undefined characters of %u total\n", empty, max);
	return 0;
}

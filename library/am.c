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

/* Quick and dirty program that produces the assembly output for the AM11 and AM12 ill conditioned matrices
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXD	100

static const int am8[64] = {
-65, 153, -222, 257, 306, 520, -121, 461,
131, 13, 184, -69, -202, 13, 253, -121,
11, 27, -81, 88, 92, 44, -71, 99,
347, 320, 267, 171, -328, 577, 463, 88,
87, 237, -21, 277, 55, 336, 107, 104,
-354, -223, -337, -563, 548, 333, 63, 323,
208, 306, 73, 243, -115, 563, 196, 215,
165, 243, 19, 112, 61, 566, 453, 109
};

static const int am11[100] = {
  65,	66,	-58,	74,	-3,	-46,	28,	29,	11,	6,
 -19,	33,	67,	6,	56,	-6,	25,	20,	57,	49,
  72,	19,	85,	-20,	46,	14,	39,	-4,	52,	43,
 -52,	-4,	-37,	95,	39,	32,	79,	90,	4,	-4,
 -16,	29,	71,	2,	60,	3,	30,	17,	61,	52,
 -39,	24,	-23,	88,	23,	-12,	63,	57,	4,	-1,
  63,	82,	42,	28,	20,	-71,	57,	49,	-7,	-12,
 -61,	-26,	47,	30,	77,	63,	77,	27,	26,	16,
  26,	-43,	45,	-39,	47,	97,	57,	-29,	43,	32,
  60,	19,	34,	23,	33,	10,	81,	8,	-12,	-20
};

static const int am12[100] = {
 -19,	33,	56,	-6,	-23,	44,	25,	49,	57,	20,
  26,	-43,	47,	97,	-32,	13,	57,	32,	43,	-29,
 -39,	24,	23,	-12,	77,	54,	63,	-1,	4,	57,
  65,	66,	-3,	-46,	97,	39,	28,	6,	11,	29,
 -52,	-4,	39,	32,	84,	47,	79,	-4,	4,	90,
  72,	19,	46,	14,	-33,	52,	39,	43,	52,	-4,
 -61,	-26,	77,	63,	-10,	37,	77,	16,	26,	27,
  63,	82,	20,	-71,	19,	61,	57,	-12,	-7,	49,
 -16,	29,	60,	3,	-26,	45,	30,	52,	61,	17,
  60,	19,	33,	10,	19,	53,	81,	-20,	-12,	8
};

static const int am13[100] = {
34,	33,	195,	-18,	213,	238,	-66,	13,	24,	-56,
39,	148,	-51,	95,	-388,	-11,	28,	31,	-35,	49,
-125,	124,	-129,	130,	86,	-99,	156,	-31,	-53,	181,
248,	-65,	-44,	128,	107,	28,	71,	84,	-119,	-10,
87,	201,	40,	291,	-25,	40,	-68,	268,	176,	31,
28,	-148,	147,	75,	89,	139,	187,	-146,	-156,	-22,
175,	69,	140,	178,	-143,	306,	182,	-51,	-108,	-129,
-113,	101,	-126,	97,	165,	-112,	127,	36,	45,	123,
-127,	202,	148,	356,	277,	224,	258,	24,	63,	98,
52,	-116,	173,	93,	38,	-14,	-188,	132,	76,	19
};

static int pos(int x) {
	if (x < 0)
		return -x;
	return x;
}

static void outmat(const char *label, const int am[], int d) {
	char buf[30];
	int x;
	int i, j, k;
	char *p;
	unsigned char used[MAXD];

	memset(used, 0, sizeof(used));

	printf("\tLBL'%s'\n", label);
	for (i=0; i<d; i++) {
		if (used[i])
			continue;
		x = am[i];
		if (x < 0)
			x = -x;
		sprintf(buf, "%d", x);
		for (p=buf; *p != 0; p++)
			printf("\t%c\n", *p);
		for (j=i; j<d; j++) {
			if (pos(am[i]) == am[j]) {
				used[j] = 1;
				printf("\tSTO %02d\n", j);
			}
		}
		k = 0;
		for (j=i; j<d; j++) {
			if (pos(am[i]) == -am[j]) {
				if (k == 0) {
					printf("\t+/-\n");
					k = 1;
				}
				used[j] = 1;
				printf("\tSTO %02d\n", j);
			}
		}
		
	}
	printf("\tRTN\n\n");
}

int main() {
	//outmat("AM8", am8, 64);
	outmat("A11", am11, 100);
	//outmat("A12", am12, 100);
	//outmat("A13", am13, 100);
	return 0;
}

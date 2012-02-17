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
#include <stdlib.h>

#define NCH	512

#include "../font.c"
#include "../charmap.c"

extern unsigned int charlengths(unsigned int c);
extern void findlengths(unsigned short int posns[257], int smallp);
extern void unpackchar(unsigned int c, unsigned char d[6], int smallp, const unsigned short int posns[257]);

int main() {
	int i, j, k;
	unsigned short int pos_small[257], pos_large[257];
	int total_width = 0;

	findlengths(pos_small, 1);
	findlengths(pos_large, 0);

	for (i=0; i<NCH; i++) {
		int gap[6];
		const int width = charlengths(i);
		unsigned char p[6];
		int small = i > 255;

		for (k=0; k<6; k++)
			gap[k] = 0;

		printf("Character %d  sort position %u\n", i, remap_chars(0xff & i));
		unpackchar(i & 255, p, small, small ? pos_small : pos_large);
		for (j=0; j<6; j++) {
			printf("\t%d\t", j);
			for (k=0; k<width; k++) {
				if (p[j] & (1 << k)) {
					putchar('#');
				} else {
					gap[k]++;
					putchar('.');
				}
			}
			putchar('\n');
		}
		for (k=0; k<width-1; k++) {
			if (gap[k] == 6)
				printf("warning: column %d is blank\n", k+1);
		}
		if (gap[width-1] != 6)
			printf("warning: last column isn't all blanks\n");
		printf("\n");
		total_width += width;
	}
	printf("Total width is %d (%d real columns)\n", total_width, total_width - NCH);
	return 0;
}

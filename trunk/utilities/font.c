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

#define C(w, a,b,c,d,e,f)	{ w, { a,b,c,d,e,f } }

struct ch {
	int width;
	int p[6];
} chars[] = {
#include "../charset.h"
};
#define NCH	(sizeof(chars) / sizeof(struct ch))

#include "../charmap.c"

int main() {
	int i, j, k;
	int total_width = 0;

	for (i=0; i<NCH; i++) {
		int gap[6];

		for (k=0; k<6; k++)
			gap[k] = 0;

		printf("Character %d  sort position %u\n", i, remap_chars(0xff & i));
		for (j=0; j<6; j++) {
			printf("\t%d\t", j);
			for (k=0; k<chars[i].width; k++) {
				if (chars[i].p[j] & (1 << k)) {
					putchar('#');
				} else {
					gap[k]++;
					putchar('.');
				}
			}
			putchar('\n');
		}
		for (k=0; k<chars[i].width-1; k++) {
			if (gap[k] == 6)
				printf("warning: column %d is blank\n", k+1);
		}
		if (gap[chars[i].width-1] != 6)
			printf("warning: last column isn't all blanks\n");
		printf("\n");
		total_width += chars[i].width;
	}
	printf("Total width is %d (%lu real columns)\n", total_width, total_width - NCH);
	return 0;
}

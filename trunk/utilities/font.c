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

int main() {
	int i, j, k;

	for (i=0; i<NCH; i++) {
		int gap[6];

		for (k=0; k<6; k++)
			gap[k] = 0;

		printf("Character %d\n", i);
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
	}
	return 0;
}

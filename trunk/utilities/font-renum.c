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

#include "../charmap.c"

int main() {
	int i;

	printf("/* This file is part of 34S.\n"
		" * \n"
		" * 34S is free software: you can redistribute it and/or modify\n"
		" * it under the terms of the GNU General Public License as published by\n"
		" * the Free Software Foundation, either version 3 of the License, or\n"
		" * (at your option) any later version.\n"
		" * \n"
		" * 34S is distributed in the hope that it will be useful,\n"
		" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		" * GNU General Public License for more details.\n"
		" * \n"
		" * You should have received a copy of the GNU General Public License\n"
		" * along with 34S.  If not, see <http://www.gnu.org/licenses/>.\n"
		" */\n\n\n"
		"/*\n"
		" * Using a 256 element table to define equivalence classes is smaller\n"
		" * than a switch statement to catch the exception.  Lower case is for\n"
		" * Greek letters and the Roman letter doesn't correspond at all with the Greek.\n"
		" */\n\n");

	printf("static const unsigned char character_remapping_order[256] = {\n");
	for (i=0; i<256; i++) {
		unsigned char z = character_remapping_order[i];
		// Modify z as required here

		if ((i%8) == 0)
			printf("\t", i-8);
		printf("%3d,\t", z);
		if ((i % 8) == 7)
			printf("// %04o\n", i-7);
	}
	printf("};\n\n\n"
		"/* Take a character and remap it into sort order.\n"
		" * Also cater for different cases, diacriticals and some\n"
		" * special symbols being identical from an ordering perspective.\n"
		" */\n"
		"unsigned char remap_chars(unsigned char ch) {\n"
		"	return character_remapping_order[ch & 0xff];\n"
		"}\n\n");
	return 0;
}

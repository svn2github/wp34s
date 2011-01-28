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

#include "xeq.h"


/* Take a characer and remap it into sort order.
 * Also cater for different cases, diacriticals and some
 * special symbols being identical from an ordering perspective.
 *
 * Using a 256 element table to define equivalence classes is smaller
 * than a switch statement to catch teh exception.  Lower case is for
 * Greek letters and the Roman letter doesn't correspond at all with the Greek.
 */
unsigned char remap_chars(unsigned char ch) {
	static const unsigned char remapping[256] = {
		0,	'X',	'Y',	022,	023,	024,	' ',	'G',	// 0000
		010,	'<',	'>',	'=',	'$',	015,	016,	017,	// 0010
		020,	'F',	'G',	'H',	'C',	025,	026,	021,	// 0020
		'S',	'X',	'Y',	' ',	034,	' ',	'$',	'$',	// 0030
		' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	// 0040
		'(',	')',	'*',	'+',	036,	'-',	'.',	'/',	// 0050
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	// 0060
		'8',	'9',	':',	';',	'<',	'=',	'>',	'?',	// 0070
		'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',	// 0100
		'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',	// 0110
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	// 0120
		'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',	// 0130
		'\'',	'A',	'B',	'C',	'D',	'E',	'F',	'G',	// 0140
		'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',	// 0150
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	// 0160
		'X',	'Y',	'Z',	'{',	'|',	'}',	'~',	0177,	// 0170
		'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	// 0200
		'i',	'j',	'k',	'l',	'm',	'n',	'o',	'p',	// 0210
		'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	// 0220
		'B',	'l',	'2',	0237,	'X',	'1',	'H',	0237,	// 0230
		'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	// 0240
		'i',	'j',	'k',	'l',	'm',	'n',	'o',	'p',	// 0250
		'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	// 0260
		'0',	'1',	'2',	'C',	'E',	'N',	'P',	'U',	// 0270
		'A',	'A',	'A',	'A',	'A',	'C',	'C',	'C',	// 0300
		'E',	'E',	'E',	'E',	'I',	'I',	'I',	'I',	// 0310
		'N',	'O',	'O',	'O',	'O',	'R',	'S',	'A',	// 0320
		'U',	'U',	'U',	'U',	'U',	'Y',	'Y',	'Z',	// 0330
		'A',	'A',	'A',	'A',	'A',	'C',	'C',	'C',	// 0340
		'E',	'E',	'E',	'E',	'I',	'I',	'I',	'I',	// 0350
		'N',	'O',	'O',	'O',	'O',	'R',	'S',	/*'S',*/	'U',	// 0360
		'U',	'U',	'U',	'U',	'Y',	'Y',	'Z',	'K',	// 0370
	};

	return remapping[ch & 0xff];
}


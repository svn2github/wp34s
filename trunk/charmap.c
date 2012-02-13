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


/* Take a character and remap it into sort order.
 * Also cater for different cases, diacriticals and some
 * special symbols being identical from an ordering perspective.
 *
 * Using a 256 element table to define equivalence classes is smaller
 * than a switch statement to catch the exception.  Lower case is for
 * Greek letters and the Roman letter doesn't correspond at all with the Greek.
 */
unsigned char remap_chars(unsigned char ch) {
	static const unsigned char remapping[256] = {
		0,	74,	78,	170,	171,	10,	0,	179,	// 0000
		126,	155,	159,	158,	163,	145,	146,	148,	// 0010
		147,	1,	2,	3,	27,	57,	57,	151,	// 0020
		63,	75,	80,	49,	124,	157,	164,	165,	// 0030
		0,	129,	134,	255,	162,	161,	175,	133,	// 0040
		120,	121,	136,	122,	127,	123,	128,	125,	// 0050
		10,	11,	12,	13,	14,	15,	16,	17,	// 0060
		18,	19,	131,	132,	154,	156,	160,	130,	// 0070
		137,	20,	26,	27,	31,	32,	37,	38,	// 0100
		39,	41,	46,	47,	48,	49,	50,	52,	// 0110
		58,	59,	60,	62,	65,	66,	72,	73,	// 0120
		74,	77,	82,	180,	176,	181,	177,	138,	// 0130
		140,	20,	26,	27,	31,	32,	37,	38,	// 0140
		39,	41,	46,	47,	48,	49,	50,	52,	// 0150
		58,	59,	60,	62,	65,	66,	72,	73,	// 0160
		74,	77,	82,	182,	178,	183,	139,	190,	// 0170
		13,	73,	92,	93,	0,	0,	0,	97,	// 0200
		25,	25,	100,	0,	0,	103,	173,	105,	// 0210
		0,	107,	0,	0,	110,	0,	112,	113,	// 0220
		26,	101,	12,	172,	74,	123,	40,	172,	// 0230
		90,	91,	92,	93,	94,	95,	96,	97,	// 0240
		98,	99,	100,	101,	102,	103,	174,	105,	// 0250
		106,	107,	108,	109,	110,	111,	112,	113,	// 0260
		10,	11,	12,	27,	32,	50,	58,	65,	// 0270
		21,	22,	23,	24,	26,	28,	29,	30,	// 0300
		33,	34,	35,	36,	42,	43,	44,	45,	// 0310
		51,	53,	54,	55,	56,	61,	63,	20,	// 0320
		67,	68,	69,	70,	71,	78,	80,	83,	// 0330
		21,	22,	23,	24,	26,	28,	29,	30,	// 0340
		33,	34,	35,	36,	42,	43,	44,	45,	// 0350
		51,	53,	54,	55,	56,	61,	63,	47,	// 0360
		67,	68,	69,	70,	71,	78,	80,	83,	// 0370
	};

	return remapping[ch & 0xff];
}


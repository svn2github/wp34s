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
 * Using a 256 element table to define equivalence classes is smaller
 * than a switch statement to catch the exception.  Lower case is for
 * Greek letters and the Roman letter doesn't correspond at all with the Greek.
 */

static const unsigned char character_remapping_order[256] = {
	  0,	 75,	 79,	171,	172,	 10,	  0,	180,	// 0000
	127,	156,	160,	159,	164,	146,	147,	149,	// 0010
	148,	  1,	  2,	  3,	 27,	 58,	 58,	152,	// 0020
	 64,	 76,	 81,	 50,	125,	158,	165,	166,	// 0030
	  0,	130,	135,	255,	163,	162,	176,	134,	// 0040
	121,	122,	137,	123,	128,	124,	129,	126,	// 0050
	 10,	 11,	 12,	 13,	 14,	 15,	 16,	 17,	// 0060
	 18,	 19,	132,	133,	155,	157,	161,	131,	// 0070
	138,	 20,	 26,	 27,	 31,	 33,	 38,	 39,	// 0100
	 40,	 42,	 47,	 48,	 49,	 50,	 51,	 53,	// 0110
	 59,	 60,	 61,	 63,	 66,	 67,	 73,	 74,	// 0120
	 75,	 78,	 83,	181,	177,	182,	178,	139,	// 0130
	141,	 20,	 26,	 27,	 31,	 33,	 38,	 39,	// 0140
	 40,	 42,	 47,	 48,	 49,	 50,	 51,	 53,	// 0150
	 59,	 60,	 61,	 63,	 66,	 67,	 73,	 74,	// 0160
	 75,	 78,	 83,	183,	179,	184,	140,	191,	// 0170
	 13,	 74,	 93,	 94,	 32,	 32,	  0,	 98,	// 0200
	 25,	 25,	101,	  0,	  0,	104,	174,	106,	// 0210
	  0,	108,	  0,	  0,	111,	  0,	113,	114,	// 0220
	 26,	102,	 12,	173,	 75,	124,	 41,	173,	// 0230
	 91,	 92,	 93,	 94,	 95,	 96,	 97,	 98,	// 0240
	 99,	100,	101,	102,	103,	104,	175,	106,	// 0250
	107,	108,	109,	110,	111,	112,	113,	114,	// 0260
	 10,	 11,	 12,	 27,	 33,	 51,	 59,	 66,	// 0270
	 21,	 22,	 23,	 24,	 26,	 28,	 29,	 30,	// 0300
	 34,	 35,	 36,	 37,	 43,	 44,	 45,	 46,	// 0310
	 52,	 54,	 55,	 56,	 57,	 62,	 64,	 20,	// 0320
	 68,	 69,	 70,	 71,	 72,	 79,	 81,	 84,	// 0330
	 21,	 22,	 23,	 24,	 26,	 28,	 29,	 30,	// 0340
	 34,	 35,	 36,	 37,	 43,	 44,	 45,	 46,	// 0350
	 52,	 54,	 55,	 56,	 57,	 62,	 64,	 48,	// 0360
	 68,	 69,	 70,	 71,	 72,	 79,	 81,	 84,	// 0370
};


/* Take a character and remap it into sort order.
 * Also cater for different cases, diacriticals and some
 * special symbols being identical from an ordering perspective.
 */
unsigned char remap_chars(unsigned char ch) {
	return character_remapping_order[ch & 0xff];
}


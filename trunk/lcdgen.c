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

#define NOCURSES
#include "lcd.h"

#define AT91C_SLCDC_MEM 0xFFFB4200

struct addr_map {
	const char *name;
	unsigned int addr;
	unsigned int mask;
};

const struct addr_map annun[] = {
	{ "0",		0xfffb4210,	0x80 }, // digit 0 segment a
	{ "1",		0xfffb4210,	0x40 }, // digit 0 segment b
	{ "2",		0xfffb4218,	0x80 }, // digit 0 segment c
	{ "3",		0xfffb4210,	0x100 }, // digit 0 segment d
	{ "4",		0xfffb4218,	0x40 }, // digit 0 segment e
	{ "5",		0xfffb4220,	0x40 }, // digit 0 segment f
	{ "6",		0xfffb4220,	0x80 }, // digit 0 segment g
	{ "7",		0xfffb4228,	0x100 }, // digit 0 segment h
	{ "8",		0xfffb4228,	0x80 }, // digit 0 segment i
	{ "9",		0xfffb4210,	0x200 }, // digit 1 segment a
	{ "10",		0xfffb4218,	0x100 }, // digit 1 segment b
	{ "11",		0xfffb4218,	0x200 }, // digit 1 segment c
	{ "12",		0xfffb4218,	0x400 }, // digit 1 segment d
	{ "13",		0xfffb4220,	0x100 }, // digit 1 segment e
	{ "14",		0xfffb4220,	0x200 }, // digit 1 segment f
	{ "15",		0xfffb4220,	0x400 }, // digit 1 segment g
	{ "16",		0xfffb4228,	0x400 }, // digit 1 segment h
	{ "17",		0xfffb4228,	0x200 }, // digit 1 segment i
	{ "18",		0xfffb4210,	0x800 }, // digit 2 segment a
	{ "19",		0xfffb4210,	0x400 }, // digit 2 segment b
	{ "20",		0xfffb4218,	0x800 }, // digit 2 segment c
	{ "21",		0xfffb4210,	0x1000 }, // digit 2 segment d
	{ "22",		0xfffb4220,	0x800 }, // digit 2 segment e
	{ "23",		0xfffb4228,	0x800 }, // digit 2 segment f
	{ "24",		0xfffb4218,	0x1000 }, // digit 2 segment g
	{ "25",		0xfffb4220,	0x1000 }, // digit 2 segment h
	{ "26",		0xfffb4228,	0x1000 }, // digit 2 segment i
	{ "27",		0xfffb4210,	0x4000 }, // digit 3 segment a
	{ "28",		0xfffb4210,	0x2000 }, // digit 3 segment b
	{ "29",		0xfffb4218,	0x2000 }, // digit 3 segment c
	{ "30",		0xfffb4218,	0x4000 }, // digit 3 segment d
	{ "31",		0xfffb4220,	0x2000 }, // digit 3 segment e
	{ "32",		0xfffb4228,	0x2000 }, // digit 3 segment f
	{ "33",		0xfffb4220,	0x4000 }, // digit 3 segment g
	{ "34",		0xfffb4220,	0x8000 }, // digit 3 segment h
	{ "35",		0xfffb4228,	0x4000 }, // digit 3 segment i
	{ "36",		0xfffb4210,	0x10000 }, // digit 4 segment a
	{ "37",		0xfffb4210,	0x8000 }, // digit 4 segment b
	{ "38",		0xfffb4218,	0x10000 }, // digit 4 segment c
	{ "39",		0xfffb4210,	0x20000 }, // digit 4 segment d
	{ "40",		0xfffb4218,	0x8000 }, // digit 4 segment e
	{ "41",		0xfffb4228,	0x8000 }, // digit 4 segment f
	{ "42",		0xfffb4220,	0x10000 }, // digit 4 segment g
	{ "43",		0xfffb4228,	0x20000 }, // digit 4 segment h
	{ "44",		0xfffb4228,	0x10000 }, // digit 4 segment i
	{ "45",		0xfffb4210,	0x40000 }, // digit 5 segment a
	{ "46",		0xfffb4218,	0x20000 }, // digit 5 segment b
	{ "47",		0xfffb4218,	0x40000 }, // digit 5 segment c
	{ "48",		0xfffb4218,	0x80000 }, // digit 5 segment d
	{ "49",		0xfffb4220,	0x20000 }, // digit 5 segment e
	{ "50",		0xfffb4220,	0x40000 }, // digit 5 segment f
	{ "51",		0xfffb4220,	0x80000 }, // digit 5 segment g
	{ "52",		0xfffb4228,	0x80000 }, // digit 5 segment h
	{ "53",		0xfffb4228,	0x40000 }, // digit 5 segment i
	{ "54",		0xfffb4210,	0x100000 }, // digit 6 segment a
	{ "55",		0xfffb4210,	0x80000 }, // digit 6 segment b
	{ "56",		0xfffb4218,	0x100000 }, // digit 6 segment c
	{ "57",		0xfffb4210,	0x200000 }, // digit 6 segment d
	{ "58",		0xfffb4220,	0x100000 }, // digit 6 segment e
	{ "59",		0xfffb4228,	0x100000 }, // digit 6 segment f
	{ "60",		0xfffb4218,	0x200000 }, // digit 6 segment g
	{ "61",		0xfffb4220,	0x200000 }, // digit 6 segment h
	{ "62",		0xfffb4228,	0x200000 }, // digit 6 segment i
	{ "63",		0xfffb4210,	0x800000 }, // digit 7 segment a
	{ "64",		0xfffb4210,	0x400000 }, // digit 7 segment b
	{ "65",		0xfffb4218,	0x400000 }, // digit 7 segment c
	{ "66",		0xfffb4218,	0x800000 }, // digit 7 segment d
	{ "67",		0xfffb4220,	0x400000 }, // digit 7 segment e
	{ "68",		0xfffb4228,	0x400000 }, // digit 7 segment f
	{ "69",		0xfffb4220,	0x800000 }, // digit 7 segment g
	{ "70",		0xfffb4220,	0x1000000 }, // digit 7 segment h
	{ "71",		0xfffb4228,	0x800000 }, // digit 7 segment i
	{ "72",		0xfffb4210,	0x2000000 }, // digit 8 segment a
	{ "73",		0xfffb4210,	0x1000000 }, // digit 8 segment b
	{ "74",		0xfffb4218,	0x2000000 }, // digit 8 segment c
	{ "75",		0xfffb4210,	0x4000000 }, // digit 8 segment d
	{ "76",		0xfffb4218,	0x1000000 }, // digit 8 segment e
	{ "77",		0xfffb4228,	0x1000000 }, // digit 8 segment f
	{ "78",		0xfffb4220,	0x2000000 }, // digit 8 segment g
	{ "79",		0xfffb4228,	0x4000000 }, // digit 8 segment h
	{ "80",		0xfffb4228,	0x2000000 }, // digit 8 segment i
	{ "81",		0xfffb4210,	0x8000000 }, // digit 9 segment a
	{ "82",		0xfffb4218,	0x4000000 }, // digit 9 segment b
	{ "83",		0xfffb4218,	0x8000000 }, // digit 9 segment c
	{ "84",		0xfffb4218,	0x10000000 }, // digit 9 segment d
	{ "85",		0xfffb4220,	0x4000000 }, // digit 9 segment e
	{ "86",		0xfffb4220,	0x8000000 }, // digit 9 segment f
	{ "87",		0xfffb4220,	0x10000000 }, // digit 9 segment g
	{ "88",		0xfffb4228,	0x10000000 }, // digit 9 segment h
	{ "89",		0xfffb4228,	0x8000000 }, // digit 9 segment i
	{ "90",		0xfffb4210,	0x20000000 }, // digit 10 segment a
	{ "91",		0xfffb4210,	0x10000000 }, // digit 10 segment b
	{ "92",		0xfffb4218,	0x20000000 }, // digit 10 segment c
	{ "93",		0xfffb4210,	0x40000000 }, // digit 10 segment d
	{ "94",		0xfffb4220,	0x20000000 }, // digit 10 segment e
	{ "95",		0xfffb4228,	0x20000000 }, // digit 10 segment f
	{ "96",		0xfffb4218,	0x40000000 }, // digit 10 segment g
	{ "97",		0xfffb4220,	0x40000000 }, // digit 10 segment h
	{ "98",		0xfffb4228,	0x40000000 }, // digit 10 segment i
	{ "99",		0xfffb4214,	0x1 }, // digit 11 segment a
	{ "100",	0xfffb4210,	0x80000000 }, // digit 11 segment b
	{ "101",	0xfffb4218,	0x80000000 }, // digit 11 segment c
	{ "102",	0xfffb421c,	0x1 }, // digit 11 segment d
	{ "103",	0xfffb4220,	0x80000000 }, // digit 11 segment e
	{ "104",	0xfffb4228,	0x80000000 }, // digit 11 segment f
	{ "105",	0xfffb4224,	0x1 }, // digit 11 segment g
	{ "106",	0xfffb422c,	0x2 }, // digit 11 segment h
	{ "107",	0xfffb422c,	0x1 }, // digit 11 segment i
	{ "108",	0xfffb4214,	0x4 }, // digit 12 segment a
	{ "109",	0xfffb4214,	0x2 }, // digit 12 segment b
	{ "110",	0xfffb4224,	0x4 }, // digit 12 segment c
	{ "111",	0xfffb421c,	0x4 }, // digit 12 segment d
	{ "112",	0xfffb4224,	0x2 }, // digit 12 segment e
	{ "113",	0xfffb422c,	0x4 }, // digit 12 segment f
	{ "114",	0xfffb422c,	0x8 }, // digit 12 segment g
	{ "115",	0xfffb4214,	0x10 }, // digit 13 segment a
	{ "116",	0xfffb4214,	0x8 }, // digit 13 segment b
	{ "117",	0xfffb421c,	0x8 }, // digit 13 segment c
	{ "118",	0xfffb421c,	0x10 }, // digit 13 segment d
	{ "119",	0xfffb4224,	0x8 }, // digit 13 segment e
	{ "120",	0xfffb422c,	0x10 }, // digit 13 segment f
	{ "121",	0xfffb4224,	0x10 }, // digit 13 segment g
	{ "122",	0xfffb421c,	0x40 }, // digit 14 segment a
	{ "123",	0xfffb4214,	0x20 }, // digit 14 segment b
	{ "124",	0xfffb421c,	0x20 }, // digit 14 segment c
	{ "125",	0xfffb4224,	0x40 }, // digit 14 segment d
	{ "126",	0xfffb4224,	0x20 }, // digit 14 segment e
	{ "127",	0xfffb422c,	0x20 }, // digit 14 segment f
	{ "128",	0xfffb422c,	0x40 }, // digit 14 segment g

	{ "MANT_SIGN",	0xfffb4228,	0x40 },
	{ "EXP_SIGN",	0xfffb421c,	0x02 },
	{ "BIG_EQ",	0xfffb420c,	0x80 },
	{ "LIT_EQ",	0xfffb4244,	0x80 },
	{ "DOWN_ARR",	0xfffb4204,	0x80 },
	{ "INPUT",	0xfffb424c,	0x80 },
	{ "BATTERY",	0xfffb423c,	0x80 },
	{ "BEG",	0xfffb4234,	0x80 },
	{ "STO_annun",	0xfffb422c,	0x80 },
	{ "RCL_annun",	0xfffb4224,	0x80 },
	{ "RAD",	0xfffb421c,	0x80 },
	{ "DEG",	0xfffb4214,	0x80 },
	{ "RPN",	0xfffb4214,	0x40 },
	{ NULL,		0,		0 }
};


static int l2(unsigned int mask) {
	int c = 0;

	while ((mask & 1) == 0) {
		mask >>= 1;
		c++;
	}
	return c;
}


static const char *addr_map(unsigned int addr, unsigned int mask) {
	static char buf[100];

	sprintf(buf, "%2d, %2d", addr - AT91C_SLCDC_MEM, l2(mask));
	return buf;
}

int main() {
	int i;

	printf("/* This file is part of 34S.\n"
		" *\n"
		" * 34S is free software: you can redistribute it and/or modify\n"
		" * it under the terms of the GNU General Public License as published by\n"
		" * the Free Software Foundation, either version 3 of the License, or\n"
		" * (at your option) any later version.\n"
		" *\n"
		" * 34S is distributed in the hope that it will be useful,\n"
		" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		" * GNU General Public License for more details.\n"
		" *\n"
		" * You should have received a copy of the GNU General Public License\n"
		" * along with 34S.  If not, see <http://www.gnu.org/licenses/>.\n"
		" *\n"
		" * LCD graphics map - automatically generated do not edit\n"
		" */\n");

	/* Annunicators */
	for (i=0; annun[i].name != NULL; i++)
		printf("\tM(%s,\t%s),\n", annun[i].name, addr_map(annun[i].addr, annun[i].mask));

	return 0;
}


/*

These are the HP82240B font tables for the Roman8 and ECMA94 character set.

The code is part of my HP82240B Printer Simulator available at
http://hp.giesselink.com/hp82240b.htm.

This code is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

There's no licence behind, so use it in any way you like it.

May 2012, Christoph Giesselink

*/

#ifndef FONTDEF_82240B_H_
#define FONTDEF_82240B_H_

#define HP82240B_CHARACTER_WIDTH 5
#define HP82240B_CHARACTER_HEIGHT 8

extern "C"
{

typedef struct
{
	unsigned char byCol[5];
} FONTDEF;

extern const FONTDEF sFontRoman8[];
extern const FONTDEF sFontEcma94[];

}

#endif /* FONTDEF_82240B_H_ */

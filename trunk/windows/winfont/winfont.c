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

/* Create a Windows bitmap font file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../font.c"

extern unsigned int charlengths( unsigned int c );
extern void findlengths( unsigned short int posns[ 257 ], int smallp);
extern void unpackchar(unsigned int c, unsigned char d[ 6 ], int smallp, const unsigned short int posns[ 257 ]);

#define MAX_ROWS 6
#define MAX_COLS 6

typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef char	       CHAR;
typedef unsigned char  BYTE;

#pragma pack(1)

#define DF_VERSION       0x0300
#define DF_COPYRIGHT	 "(c) 2012 The WP 34S Team. Covered by the GPL V3."
#define DF_RESOLUTION	 72
#define DFF_PROPORTIONAL 2
#define DFT_BITMAP       0
#define FW_REGULAR       400
#define DF_CHARSET       0		// what should it be?
#define PW_PROPORTIONAL  0		// proportional
#define FF_SWISS	 ( 2 << 4 )	// no serifs, proportional

typedef struct {
	WORD  dfVersion;
	DWORD dfSize;
	CHAR  dfCopyright[60];
	WORD  dfType;
	WORD  dfPoints;
	WORD  dfVertRes;
	WORD  dfHorizRes;
	WORD  dfAscent;
	WORD  dfInternalLeading;
	WORD  dfExternalLeading;
	BYTE  dfItalic;
	BYTE  dfUnderline;
	BYTE  dfStrikeOut;
	WORD  dfWeight;
	BYTE  dfCharSet;
	WORD  dfPixWidth;
	WORD  dfPixHeight;
	BYTE  dfPitchAndFamily;
	WORD  dfAvgWidth;
	WORD  dfMaxWidth;
	BYTE  dfFirstChar;
	BYTE  dfLastChar;
	BYTE  dfDefaultChar;
	BYTE  dfBreakChar;
	WORD  dfWidthBytes;
	DWORD dfDevice;
	DWORD dfFace;
	DWORD dfBitsPointer;
	DWORD dfBitsOffset;
	BYTE  dfReserved;
	DWORD dfFlags;
	WORD  dfAspace, dfBspace, dfCspace;
	DWORD dfColorPointer;
	BYTE  dfReserved1[ 16 ];
	struct _glyphentry {
		WORD  geWidth;
		DWORD geOffset;
	}     dfCharTable[ 257 ];
	CHAR  szFaceName[ 1 ];
	// CHAR  szDeviceName;
} FONT;

const char *FaceName[ 2 ] = {
	"WP 34S Raster Font Regular",
	"WP 34S Raster Font Small"
};

int main( int argc, char **argv ) 
{
	int i;
	int file_size;
	unsigned short int positions[257];
	int smallfont = 0;
	int scale = 1;
	int gap = 0;
	int bytes_per_row;
	FILE *f;
	FONT *font;
	char *p;

	/*
	 *  Get arguments
	 */
	while ( argc > 2 && *argv[1] == '-' ) {
		if ( strcmp( argv[ 1 ], "-s" ) == 0 ) {
			smallfont = 1;
		}
		++argv;
		--argc;
	}
	if ( argc >= 3 ) {
		scale = atoi( argv[ 2 ] );
		if ( argc >= 4 ) {
			gap = atoi( argv[ 3 ] );
		}
	}

	if ( argc < 2 || scale == 0 || gap >= scale ) {
		printf( "Usage: winfont [-s] <font-file> [<scale> [<gap>]]\n"
			"       -s creates the small font\n"
			"       <font-file> should end in .fnt\n"
			"       <scale> defines the pixel size (including gap)\n"
			"       <gap> defines the space between pixels\n");
		exit( 1 );
	}

	/*
	 *  Open the FNT file
	 */
	printf( "Writing %s font to file %s, scale=%d, gap=%d\n", 
		smallfont ? "small" : "regular", argv[ 1 ], scale, gap );
	f = fopen( argv[ 1 ], "wb" );
	if ( f == NULL ) {
		perror( argv[ 1 ] );
		abort();
	}

	/*
	 *  Allocate buffer (size is estimated)
	 */
	bytes_per_row = ( MAX_COLS * scale + 7 ) / 8;
	file_size = sizeof( FONT ) + strlen( FaceName[ smallfont ] ) + 257 * bytes_per_row * MAX_ROWS * scale;
	font = (FONT *) malloc( file_size );
	if ( font == NULL ) {
		perror( "malloc" );
		abort();
	}
	memset( font, 0, file_size );

	/*
	 *  Fill the header
	 */
	font->dfVersion = DF_VERSION;
	strcpy( font->dfCopyright, DF_COPYRIGHT );
	font->dfType = DFT_BITMAP;
	font->dfPoints = scale * (MAX_ROWS - smallfont);
	font->dfVertRes = DF_RESOLUTION;
	font->dfHorizRes = DF_RESOLUTION;
	font->dfAscent = scale * (MAX_ROWS - smallfont);
	font->dfInternalLeading = 0;
	font->dfExternalLeading = scale;
	font->dfWeight = FW_REGULAR;
	font->dfCharSet = DF_CHARSET;
	font->dfPixHeight = scale * MAX_ROWS;
	font->dfPitchAndFamily = FF_SWISS;
	font->dfAvgWidth = scale * charlengths( 'X' );
	font->dfMaxWidth = scale * MAX_COLS;
	font->dfFirstChar = 0;
	font->dfLastChar = 255;
	font->dfDefaultChar = '.';
	font->dfBreakChar = ' ';
	font->dfWidthBytes = bytes_per_row;
	font->dfFace = font->szFaceName - (char *) font;
	strcpy( font->szFaceName, FaceName[ smallfont ] );
	p = font->szFaceName + strlen( font->szFaceName ) + 1;
	if ( (int) p & 1 ) {
		++p;
	}
	font->dfBitsOffset = p - (char *) font;
	font->dfFlags = DFF_PROPORTIONAL;

	/*
	 *  Create the glyphs
	 */
	findlengths( positions, smallfont );

	for ( i = 0; i < 257; ++i ) {
		const int width = charlengths( ( i & 0xff ) + 256 * smallfont );
		unsigned char bits[ 6 ];
		int j, k, b;

		font->dfCharTable[ i ].geWidth = width * scale;
		font->dfCharTable[ i ].geOffset = p - (char *) font;

		unpackchar( i & 0xff, bits, smallfont, positions );

		for ( j = 0; j < MAX_ROWS; ++j ) {
			unsigned char pattern = bits[ j ];
			int jj;
			for ( jj = 0; jj < scale - gap; ++jj ) {
				for ( k = 0; k < width; k++ ) {
					if ( pattern & ( 1 << k ) ) {
						for ( b = scale * k; b < scale * ( k + 1 ) - gap; ++b ) {
							p[ scale * MAX_ROWS * ( b / 8 ) ] |= ( 0x80 >> ( b % 8 ) );
						}
					}
				}
				++p;
			}
			p += gap;
		}
		p += scale * MAX_ROWS * ( bytes_per_row - 1 );
	}
	
	/*
	 *  Write the font file
	 */
	file_size = p - (char *) font;
	font->dfSize = file_size;
	
	if ( 1 != fwrite( font, file_size, 1, f ) ) {
		perror( "frwite" );
		fclose( f );
		abort();
	}
	fclose( f );
	return 0;
}

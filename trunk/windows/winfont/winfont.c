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
#include <memory.h>
#include <ctype.h>

#include "../../font.c"
#include "../../translate.c"

extern unsigned int charlengths( unsigned int c );
extern void findlengths( unsigned short int posns[ 257 ], int smallp);
extern void unpackchar(unsigned int c, unsigned char d[ 6 ], int smallp, const unsigned short int posns[ 257 ]);

#define MAX_ROWS 6
#define MAX_COLS 6

const char *FaceName[ 2 ] = {
	"Regular",
	"Small"
};

const char *Encodings[ 3 ] = {
	"Original",
	"Windows",
	"UnicodeBmp"
};

const struct _altuni {
	int code;
	int alternate;
} AltUni[] = {
	{ '"',  0x201c },
	{ '"',  0x201d },
	{ '"',  0x201e },
	{ '\'', 0x0060 },
	{ '\'', 0x2018 },
	{ '\'', 0x2019 },
	{ '\'', 0x201a },
	{ 'A',  0x0391 },
	{ 'B',  0x0392 },
	{ 'E',  0x0395 },
	{ 'Z',  0x0396 },
	{ 'H',  0x0397 },
	{ 'I',  0x0399 },
	{ 'K',  0x039a },
	{ 'M',  0x039c },
	{ 'N',  0x039d },
	{ 'O',  0x039f },
	{ 'P',  0x03a1 },
	{ 'T',  0x03a4 },
	{ 'Y',  0x03a5 },
	{ 'X',  0x03a7 },
	{ 'o',  0x03bf },
	{ '|',  0x00A6 },
	{ 0xab, 0x00b5 }, // µ
	{ 0 }
};


int main( int argc, char **argv ) 
{
	int i, j, k, l;
	unsigned short int positions[257];
	int smallfont = 0;
	int encoding = 0;
	int gap = 5;
	FILE *t;
	FILE *f;
	char *p = "Template.sfd";
	char buffer[ 256 ];
	const struct _altuni *alt;

	/*
	 *  Get arguments
	 */
	while ( argc > 2 && *argv[1] == '-' ) {
		if ( strcmp( argv[ 1 ], "-s" ) == 0 ) {
			smallfont = 1;
		}
		if ( strcmp( argv[ 1 ], "-w" ) == 0 ) {
			encoding = 1;
		}
		if ( strcmp( argv[ 1 ], "-u" ) == 0 ) {
			encoding = 2;
		}
		++argv;
		--argc;
	}
	if ( argc >= 3 ) {
		p = argv[ 2 ];
		if ( argc >= 4 ) {
			gap = atoi( argv[ 3 ] );
		}
	}

	/* 
	 *  Open template file
	 */
	t = fopen( p, "r" );

	if ( argc < 2 || t == NULL || gap > 50 || gap < 0 ) {
		printf( "Usage: winfont [-s] [-w|-u] <font-file> [<template> [<gap>]]\n"
			"       -s creates the small font\n"
			"	-w encode as CP1252 (sort of)\n"
			"       -u encode as Unicode\n"
			"       <font-file> should end in .sfd\n"
			"       <scale> defines the pixel size (including gap)\n"
			"       <gap> defines the space between pixels between 0 and 50% \n");
		if ( t == NULL ) {
			perror( p );
			abort();
		}
		exit( 1 );
	}

	/*
	 *  Open the font file
	 */
	fprintf( stderr, "Writing %s font to file %s, template=%s, gap=%d, encoding is %s\n", 
			 smallfont ? "small" : "regular", argv[ 1 ], p, gap, Encodings[encoding] );
	f = fopen( argv[ 1 ], "wb" );
	if ( f == NULL ) {
		perror( argv[ 1 ] );
		abort();
	}

	/*
	 *  Read the template and copy contents to font file
	 *  $1 will be substituted by "Regular" or "Small"
	 *  $$ will contain the font information, starting with 1
	 */
	while ( !feof( t ) ) {

		p = fgets( buffer, sizeof( buffer ), t );
		if ( p == NULL ) {
			if ( errno ) {
				perror( "Read template" );
				fclose( f );
				abort();
			}
			continue;
		}

		p = strstr( buffer, "$1" );
		if ( p != NULL ) {
			/*
			 *  Complete the face name
			 */
			const char *face = FaceName[ smallfont ];
			const int l = strlen( face );
			const int rest = strlen( p + 2 );

			memmove( p + l, p + 2, rest + 1 );
			memcpy( p, face, l );
		}
		p = strstr( buffer, "$2" );
		if ( p != NULL ) {
			/*
			 *  Insert the ascent and decent
			 */
			int ascent = smallfont ? 700 : 800;
			int descent = 1000 - ascent;
			sprintf( buffer, "Ascent: %d\nDescent: %d\n", ascent, descent );
		}
		p = strstr( buffer, "$3" );
		if ( p != NULL ) {
			/*
			 *  Insert the encoding
			 */
			const char *enc = Encodings[ encoding ];
			sprintf( buffer, "Encoding: %s\n", enc );
		}
		if ( strncmp( buffer, "$$", 2 ) != 0 ) {
			/*
			 *  Write the line unchanged
			 */
			if ( EOF == fputs( buffer, f ) ) {
				goto error;
			}
			continue;
		}

		/*
		 *  Create the glyphs
		 */
		findlengths( positions, smallfont );

		for ( i = 1; i < 256; ++i ) {
			const int width = charlengths( i + 256 * smallfont );
			const int c = encoding == 1 ? from_cp1252[ i ] 
			            : encoding == 2 ? unicode[ i ]
				    : i;
			unsigned char bits[ 6 ];
			const char *cp;
			int empty = 1;

			unpackchar( i, bits, smallfont, positions );

			l = fprintf( f, "StartChar: " );
			for ( cp = charnames[ i ]; *cp != '\0'; ++cp ) {
				if ( isalnum( *cp ) ) {
					fputc( *cp, f );
				}
			}
			if ( l <= 0 ) {
				goto error;
			}
			l = fprintf( f, "\nEncoding: %d %d %d\n", c, unicode[ i ], i );
			if ( l <= 0 ) {
				goto error;
			}
			/*
			 *  Alternate code points
			 */
			j = 0;
			for ( alt = AltUni; alt->code; ++alt ) {
				if ( alt->code == i ) {
					if ( j++ == 0 ) {
						fprintf( f, "AltUni2:" );
						if ( l <= 0 ) {
							goto error;
						}
					}
					l = fprintf( f, " %04x.ffffffff.0", alt->alternate );
					if ( l <= 0 ) {
						goto error;
					}
				}
			}
			if ( j ) {
				putc( '\n', f );
			}
			l = fprintf( f, "Width: %d\n", width * 1600 / 13 );
			if ( l <= 0 ) {
				goto error;
			}
			l = fprintf( f, "VWidth: 0\nFlags:\nLayerCount: 2\n" );
			if ( l <= 0 ) {
				goto error;
			}

			/*
			 *  Generate the drawing instructions
			 */
			l = fprintf( f, "Fore\nSplineSet\n" );
			if ( l <= 0 ) {
				goto error;
			}
			for ( j = 0; j < MAX_ROWS; ++j ) {
				unsigned char pattern = bits[ j ];
				int y1 = ( MAX_ROWS - 1 - smallfont - j ) * 1600 / 13;

				for ( k = 0; k < width; k++ ) {
					int x1 = k * 1600 / 13;
					int x2 = x1 + ( 100 - gap ) * 16 / 13 - ( gap == 0 );
					int y2 = y1 + ( 100 - gap ) * 16 / 13 - ( gap == 0 );
					if ( pattern & ( 1 << k ) ) {
						empty = 0;
						l = fprintf( f, " %d %d m 1\n"
							        " %d %d l 1\n"
							        " %d %d l 1\n"
							        " %d %d l 1\n"
							        " %d %d l 1\n",
							        x1, y1,
							        x1, y2,
							        x2, y2, 
							        x2, y1,
							        x1, y1 );
						if ( l <= 0 ) {
							goto error;
						}
					}
				}
			}
			if ( empty ) {
				l = fprintf( f, " -1 801 m 1\n -1 801 l 1\n" );
				if ( l <= 0 ) {
					goto error;
				}
			}
			fprintf( f, "EndSplineSet\nEndChar\n\n" );
			if ( j < 0 ) {
				goto error;
			}
		}
	}
	fclose( f );
	return 0;

error:
	perror( "Write font file" );
	fclose( f );
	abort();
}

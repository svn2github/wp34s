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

int main( int argc, char **argv ) 
{
	int i, j, k, l;
	unsigned short int positions[257];
	int smallfont = 0;
	int cp1252 = 0;
	int gap = 5;
	FILE *t;
	FILE *f;
	char *p = "Template.sfd";
	char buffer[ 256 ];

	/*
	 *  Get arguments
	 */
	while ( argc > 2 && *argv[1] == '-' ) {
		if ( strcmp( argv[ 1 ], "-s" ) == 0 ) {
			smallfont = 1;
		}
		if ( strcmp( argv[ 1 ], "-t" ) == 0 ) {
			cp1252 = 1;
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
		printf( "Usage: winfont [-s] [-t] <font-file> [<template> [<gap>]]\n"
			"       -s creates the small font\n"
			"	-t translates to CP1252 (sort of)\n"
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
	fprintf( stderr, "Writing %s font to file %s, template=%s, gap=%d, cp1252=%d\n", 
			 smallfont ? "small" : "regular", argv[ 1 ], p, gap, cp1252 );
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
			const int c = cp1252 ? from_cp1252[ i ] : i;
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
			l = fprintf( f, "Width: %d\n", width * 100 );
			if ( l <= 0 ) {
				goto error;
			}
			l = fprintf( f, "VWidth: 0\nFlags:\nLayerCount: 2\nFore\nSplineSet\n" );
			if ( l <= 0 ) {
				goto error;
			}

			/*
			 *  Generate the drawing instructions
			 */
			for ( j = 0; j < MAX_ROWS; ++j ) {
				unsigned char pattern = bits[ j ];
				int y1 = ( MAX_ROWS - 1 - j ) * 100;

				for ( k = 0; k < width; k++ ) {
					int x1 = k * 100;
					int x2 = x1 + 100 - gap;
					int y2 = y1 + 100 - gap;
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

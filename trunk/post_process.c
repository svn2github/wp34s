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
 *  Patch the function pointer table
 */

#undef DEBUG
#undef REALBUILD
#define POST_PROCESSING
#include "xeq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char buffer[ ( 64 + 128 ) * 1024 ];
struct _command_info *info = (struct _command_info *) buffer;
#define BUFF(p) (buffer + (int)(p) - 0xf0000)
#define ADJUST(p,type) (info->p = (struct type *) BUFF(info->p))
#define UPDATE(p,cmp, max) \
	if ( info->p != cmp ) { \
		fprintf( stderr, "Data error in line %d, i=%d/%d (%4x)\n", __LINE__, i, max, info->p ); \
		return 2; \
	} else info->p = (unsigned short) ( fp >> 1 )


int main(int argc, char *argv[])
{
	FILE *tmp, *bin;
	int i, fp;
	int bin_size;

	if ( argc != 3 ) {
		fprintf( stderr, "usage: post_process <calc.tmp calc.bin>\n" );
		return 1;
	}
	printf( "Post-processing address tables\n" );

	/*
	 *  Open files and read input into buffer
	 */
	tmp = fopen( argv[ 1 ], "rb" );
	bin = fopen( argv[ 2 ], "wb" );
	if ( tmp == NULL || bin == NULL ) {
		perror( "File open error" );
		return 2;
	}
	bin_size = fread( buffer, 1, sizeof( buffer ), tmp ) - 64 * 1024;
	if ( bin_size <= 0 ) {
		perror( "Read error" );
		return 2;
	}
	fclose( tmp );

	/*
	 *  Adjust the pointers in the info header
	 *  to make them point into our buffer.
	 */
	ADJUST( p_monfuncs,     monfunc );
	ADJUST( p_monfuncs_ct,  monfunc_cmdtab );
	ADJUST( p_dyfuncs,      dyfunc );
	ADJUST( p_dyfuncs_ct,   dyfunc_cmdtab );
	ADJUST( p_trifuncs,     trifunc );
	ADJUST( p_trifuncs_ct,  trifunc_cmdtab );
	ADJUST( p_niladics,     niladic );
	ADJUST( p_niladics_ct,  niladic_cmdtab );
	ADJUST( p_argcmds,      argcmd );
	ADJUST( p_argcmds_ct,   argcmd_cmdtab );
	ADJUST( p_multicmds,    multicmd );
	ADJUST( p_multicmds_ct, multicmd_cmdtab );

	/*
	 *  Now make the pointers fit into a short
	 *  Copy the untouched rest of the structure
	 */
	printf( "Monadic commands:    %3d\n", info->num_monadic );
	for ( i = 0; i < info->num_monadic; ++i ) {
		fp = (int) info->p_monfuncs_ct[ i ].mondreal;
		UPDATE( p_monfuncs[ i ].mondreal, 0xaa55, NUM_MONADIC );
		fp = (int) info->p_monfuncs_ct[ i ].mondcmplx;
		UPDATE( p_monfuncs[ i ].mondcmplx, 0x55aa, NUM_MONADIC );
		fp = (int) info->p_monfuncs_ct[ i ].monint;
		UPDATE( p_monfuncs[ i ].monint, 0xa55a, NUM_MONADIC );
	}
	printf( "Dyadic commands:     %3d\n", info->num_dyadic );
	for ( i = 0; i < info->num_dyadic; ++i ) {
		fp = (int) info->p_dyfuncs_ct[ i ].dydreal;
		UPDATE( p_dyfuncs[ i ].dydreal, 0xaa55, NUM_DYADIC );
		fp = (int) info->p_dyfuncs_ct[ i ].dydcmplx;
		UPDATE( p_dyfuncs[ i ].dydcmplx, 0x55aa, NUM_DYADIC );
		fp = (int) info->p_dyfuncs_ct[ i ].dydint;
		UPDATE( p_dyfuncs[ i ].dydint, 0xa55a, NUM_DYADIC );
	}
	printf( "Triadic commands:    %3d\n", info->num_triadic );
	for ( i = 0; i < info->num_triadic; ++i ) {
		fp = (int) info->p_trifuncs_ct[ i ].trireal;
		UPDATE( p_trifuncs[ i ].trireal, 0xaa55, NUM_TRIADIC );
		fp = (int) info->p_trifuncs_ct[ i ].triint;
		UPDATE( p_trifuncs[ i ].triint, 0xa55a, NUM_TRIADIC );
	}
	printf( "Niladic commands:    %3d\n", info->num_niladic );
	for ( i = 0; i < info->num_niladic; ++i ) {
		fp = (int) info->p_niladics_ct[ i ].niladicf;
		UPDATE( p_niladics[ i ].niladicf, 0xaa55, NUM_NILADIC );
	}
	printf( "Argument commands:   %3d\n", info->num_rarg );
	for ( i = 0; i < info->num_rarg; ++i ) {
		fp = (int) info->p_argcmds_ct[ i ].f;
		UPDATE( p_argcmds[ i ].f, 0xaa55, NUM_RARG );
	}
	printf( "Multi word commands: %3d\n", info->num_multi );
	for ( i = 0; i < info->num_multi; ++i ) {
		fp = (int) info->p_multicmds_ct[ i ].f;
		UPDATE( p_multicmds[ i ].f, 0xaa55, NUM_MULTI );
	}

	/*
	 *  Now write the modified image to the disk
	 */
	if ( bin_size != fwrite( buffer + 64 * 1024, 1, bin_size, bin ) ) {
		perror( "Write error" );
		return 2;
	}
	fclose( bin );
	return 0;
}

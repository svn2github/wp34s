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
 * This module handles all load/save operations in the real build or emulator
 * Module written by MvC
 */
#ifdef REALBUILD
#define BACKUP_SRAM  __attribute__((section(".backup")))
#define SLCDCMEM     __attribute__((section(".slcdcmem")))
#define VOLATILE     __attribute__((section(".volatile")))
#define USER_FLASH   __attribute__((section(".userflash")))
#ifndef NULL
#define NULL 0
#endif
#else
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#define BACKUP_SRAM
#define SLCDCMEM
#define VOLATILE
#define USER_FLASH
#define STATE_FILE "wp34s.dat"
#define REGION_FILE "wp34s-%c.dat"
#define REPAIR_CRC_ON_LOAD
#endif

#include "xeq.h"
#include "storage.h"
#include "display.h"
#include "stats.h"

/*
 *  Setup the persistent RAM
 */
BACKUP_SRAM TPersistentRam PersistentRam;

/*
 *  Data that is saved in the SLCD controller during deep sleep
 */
SLCDCMEM TStateWhileOn StateWhileOn;

/*
 *  A private register area for XROM code in volatile RAM
 *  It replaces the local registers and flags if active.
 */
VOLATILE TXromLocal XromLocal;

/*
 *  The user flash area:
 *  2 regions for storage of programs and/or registers
 *  Same data as in persistent RAM but in flash memory
 */
USER_FLASH TUserFlash UserFlash;


#ifdef REALBUILD
#ifdef NO_RAM_COPY

/*
 *  We do not copy any static data from flash to RAM at startup and
 *  thus can't use code in RAM. In order to program flash use the
 *  IAP feature in ROM instead
 */
#define IAP_FUNC ((int (*)(unsigned int)) (*(int *)0x400008))

/*
 *  Issue a command to the flash controller. Must be done from ROM.
 *  Returns zero if OK or non zero on error.
 */
static int flash_command( unsigned int cmd )
{
	SUPC_SetVoltageOutput( SUPC_VDD_180 );
	return IAP_FUNC( cmd ) >> 1;
}

#else

/*
 *  RAM copy is enabled at startup
 */
#define RAM_FUNCTION __attribute__((section(".ramfunc"),noinline))

/*
 *  Issue a command to the flash controller. Must be done from RAM.
 *  Returns zero if OK or non zero on error.
 */
static RAM_FUNCTION int flash_command( unsigned int cmd )
{
	AT91C_BASE_MC->MC_FCR = cmd;
	while ( ( AT91C_BASE_MC->MC_FSR & 1 ) == 0 ) {
		// wait for flash controller to do its work
	}
	return ( AT91C_BASE_MC->MC_FSR >> 1 );
}
#endif

/*
 *  Program the flash starting at page page_no
 *  Returns 0 if OK or non zero on error.
 *  If length % 256 != 0 the rest is preserved
 *  This is exact to 4 bytes only!
 */
static int program_flash( int page_no, void *buffer, int length )
{
	int *flash = (int *) ( 0x100000 | ( page_no << 8 ) );
	int *ip = (int *) buffer;
	int i;

	lock();  // No interrupts, please!

	length = ( length + 3 ) / 4;
	while ( length > 0 ) {
		/*
		 *  Copy the source to the flash write buffer
		 */
		for ( i = 0; i < PAGE_SIZE / 4; ++i, ++flash, ++ip ) {
			*flash = i < length ? *ip : *flash;
		}

		/*
		 *  Command the controller to erase and write the page.
		 */
		i = flash_command( 0x5A000003 | ( page_no << 8 ) );
		if ( i ) break;

		length -= PAGE_SIZE / 4;
		++page_no;
	}
	unlock();
	return i;
}


/*
 *  Set the boot bit to ROM and turn off the device.
 *  Next power ON goes into SAM-BA mode.
 */
void sam_ba_boot(void)
{
	/*
	 *  Command the controller to clear GPNVM1
	 */
	lock();
	flash_command( 0x5A00010C );
	SUPC_Shutdown();
}


#else

/*
 *  Emulate the flash in a file wp34s-<n>.dat
 */
#ifdef QTGUI
extern char* get_region_path(int region_index);
#else
char region_path[20];
static char* get_region_path(int region_index)
{
	sprintf( region_path, REGION_FILE, region_index == 0 ? 'R' : region_index + '0' - 1 );
	return region_path;
}
#endif

static int program_flash( int page_no, void *buffer, int length )
{
	char *name;
	char *dest;
	int r_last = -1;
	int r;
	FILE *f = NULL;

	/*
	 *  Copy the source to the destination memory
	 */
	page_no -= PAGE_BEGIN;
	dest = (char *) &UserFlash + PAGE_SIZE * page_no;

	memcpy( dest, buffer, length );

	/*
	 *  Update the correct region file(s)
	 */
	while ( length > 0 ) {

		r = NUMBER_OF_FLASH_REGIONS - 1 - page_no / SIZE_REGION;

		if ( r != r_last ) {
			if ( length < 1024 ) {
				length = 1024;
			}
			if ( f != NULL ) {
				fclose( f );
			}
			name = get_region_path( r );

			f = fopen( name, "rb+" );
			if ( f == NULL ) {
				f = fopen( name, "wb+" );
			}
			if ( f == NULL ) {
				return 1;
			}
		}
		fseek( f, ( page_no % SIZE_REGION ) * PAGE_SIZE, SEEK_SET );
		if ( 1 != fwrite( dest, PAGE_SIZE, 1, f ) ) {
			fclose( f );
			return 1;
		}
		r_last = r;
		++page_no;
		dest += PAGE_SIZE;
		length -= PAGE_SIZE;
	}
	fclose( f );
	return 0;
}
#endif

/*
 *  Simple backup / restore
 *  Started with ON+STO or ON+RCL or the SAVE/LOAD commands
 *  The backup area is the last 2KB of flash (pages 504 to 511)
 */
void flash_backup( decimal64 *nul1, decimal64 *nul2, enum nilop op )
{
	if ( not_running() ) {
		process_cmdline_set_lift();
		init_state();
		checksum_all();

		if ( program_flash( PAGE_BACKUP, &PersistentRam, SIZE_BACKUP * PAGE_SIZE ) ) {
			err( ERR_IO );
			DispMsg = "Error";
		}
		else {
			DispMsg = "Saved";
		}
	}
}


void flash_restore(decimal64 *nul1, decimal64 *nul2, enum nilop op)
{
	if ( not_running() ) {
		if ( checksum_backup() ) {
			err( ERR_INVALID );
		}
		else {
			xcopy( &PersistentRam, &( UserFlash.backup ), sizeof( PersistentRam ) );
			init_state();
			DispMsg = "Restored";
		}
	}
}


/*
 *  The total region size is (number of program steps + 2) * 2
 *  The value of last_prog is 1 more then the number of steps 
 */
static int region_length( FLASH_REGION *fr )
{
	return sizeof( short ) * ( fr->last_prog + 1 );
}


/*
 *  Write a region to flash.
 */
static int write_region( int r, FLASH_REGION *fr )
{
	if ( program_flash( region_page( r ), fr, region_length( fr ) ) ) {
		return ERR_IO;
	}
	return 0;
}


/*
 *  Save the user program area to a region.
 *  Returns an error if failed.
 */
static int internal_save_program( unsigned int r, FLASH_REGION *fr )
{
	if ( write_region( r, (FLASH_REGION *) fr ) ) {
		err( ERR_IO );
                return 1;
	}
        return 0;
}


/*
 *  Save the user program area to a region. Called by PSTO.
 */
void save_program( unsigned int r, enum rarg op )
{
	if ( not_running() ) {
		clrretstk_pc();
		checksum_code();
		internal_save_program( r + 1, (FLASH_REGION *) &CrcProg );
	}
}


/*
 *  Load the user program area from a region.
 *  Returns an error if invalid.
 */
static int internal_load_program( unsigned int r )
{
	FLASH_REGION *fr = flash_region( r );

	if ( checksum_region( r ) || fr->last_prog > NUMPROG + 1 ) {
		/*
		 *  Not a valid program region
		 */
		err( ERR_INVALID );
		return 1;
	}
	clrprog();
	xcopy( &CrcProg, fr, region_length( fr ) );
	if (RetStk < Prog + LastProg ) {
		sigmaDeallocate();
		clrretstk();
	}
	return 0;
}


/*
 *  Load the user program area from a region.
 *  Returns an error if invalid.
 */
void load_program( unsigned int r, enum rarg op )
{
	if ( not_running() ) {
		internal_load_program( r + 1 );
	}
}


/*
 *  Swap the user program area with a region.
 *  Returns an error if invalid or error.
 */
void swap_program( unsigned int r, enum rarg op )
{
	FLASH_REGION *fr;
	FLASH_REGION region;
	int l;

	if ( not_running() ) {
		++r;
		fr = flash_region( r );
		l = region_length( fr );

		/*
		 *  Temporary copy of current program
		 */
		checksum_code();
		xcopy( &region, &CrcProg, l );

		/*
		 *  Load program from flash
		 */
		if ( internal_load_program( r ) ) {
			err( ERR_INVALID );
		}

		/*
		 *  Save current program from temporary copy
		 */
		if ( internal_save_program( r, &region ) ) {
			/*
			 *  Flash failure, recover
			 */
			xcopy( &CrcProg, &region, l );
			return;
		}
	}
}


/*
 *  Load registers from backup
 */
void load_registers(decimal64 *nul1, decimal64 *nul2, enum nilop op)
{
	if ( checksum_backup() ) {
		/*
		 *  Not a valid backup region
		 */
		err( ERR_INVALID );
		return;
	}
	clrretstk();
	NumRegs = UserFlash.backup._numregs;
	sigmaDeallocate();
	xcopy( Regs, UserFlash.backup._regs, sizeof( Regs ) );
}


void load_state(decimal64 *nul1, decimal64 *nul2, enum nilop op)
{
	if ( not_running() ) {
		if ( checksum_backup() ) {
			/*
			 *  Not a valid backup region
			 */
			err( ERR_INVALID );
			return;
		}
		xcopy( &RandS1, &UserFlash.backup._rand_s1, (char *) &Crc - (char *) &RandS1 );
		init_state();
		clrretstk_pc();
	}
}


/*
 *  The CCITT 16 bit CRC algorithm (X^16 + X^12 + X^5 + 1)
 */
unsigned short int crc16( const void *base, unsigned int length )
{
	unsigned short int crc = 0x5aa5;
	unsigned char *d = (unsigned char *) base;
	unsigned int i;

	for ( i = 0; i < length; ++i ) {
		crc  = ( (unsigned char)( crc >> 8 ) ) | ( crc << 8 );
		crc ^= *d++;
		crc ^= ( (unsigned char)( crc & 0xff ) ) >> 4;
		crc ^= crc << 12;
		crc ^= ( crc & 0xff ) << 5;
	}
	return crc;
}


/*
 *  Compute a checksum and compare it against the stored sum
 *  Returns non zero value if failure
 */
static int test_checksum( const void *data, unsigned int length, unsigned short oldcrc, unsigned short *pcrc )
{
	unsigned short crc;
	crc = crc16( data, length );
	if ( pcrc != NULL ) {
		*pcrc = crc;
	}
	return crc != oldcrc && oldcrc != MAGIC_MARKER;
}


/*
 *  Checksum the program area.
 *  This always computes and sets the checksum of the program in RAM.
 *  The checksum of any program in flash can simply be read out.
 *  Returns non zero value if failure
 */
int checksum_code( void )
{
	return LastProg < 1 || LastProg > NUMPROG 
		|| test_checksum( Prog, ( LastProg - 1 ) * sizeof( s_opcode ), CrcProg, &CrcProg );
}


/*
 *  Checksum a user flash region
 *  Returns non zero value if failure
 */
int checksum_region( int r )
{
	FLASH_REGION *fr = flash_region( r );
	int l = ( fr->last_prog - 1 ) * sizeof( s_opcode );
	return l < 0 || l > sizeof( fr->prog ) || test_checksum( fr->prog, l, fr->crc, NULL );
}


/*
 *  Checksum the persistent RAM area (registers and state only)
 *  The magic marker is always valid. This eases manipulating state files.
 *  Returns non zero value if failure
 */
int checksum_data( void )
{
	const char *r = (char *) get_reg_n(0);
	return test_checksum( r, (char *) &Crc - r, Crc, &Crc );
}


/*
 *  Checksum all RAM
 *  Returns non zero value if failure
 */
int checksum_all( void )
{
	return checksum_data() + checksum_code();
}

/*
 *  Checksum the backup flash region
 *  Returns non zero value if failure
 */
int checksum_backup( void )
{
	const char *r = (char *) get_flash_reg_n(0);
	return test_checksum( r, (char *) &( UserFlash.backup._crc ) - r, 
		              UserFlash.backup._crc, NULL );
}

/*
 *  Flash region type
 */
extern int is_prog_region( unsigned int r )
{
	FLASH_REGION *fr = flash_region( r );

	return r > 0 && fr->last_prog >= 1 && fr->last_prog <= NUMPROG + 1;
}

extern int is_data_region( unsigned int r )
{
	return r == 0;
}


#if !defined(REALBUILD) && !defined(QTGUI)
/*
 *  Save/Load state to a file (only for emulator(s))
 */
void save_statefile( void )
{
	FILE *f = fopen( STATE_FILE, "wb" );
	if ( f == NULL ) return;
	process_cmdline_set_lift();
	init_state();
	checksum_all();
	fwrite( &PersistentRam, sizeof( PersistentRam ), 1, f );
	fclose( f );
#ifdef DEBUG
	printf( "sizeof struct _state = %d\n", (int)sizeof( struct _state ) );
	printf( "sizeof struct _ustate = %d\n", (int)sizeof( struct _ustate ) );
	printf( "sizeof RAM = %d (%d free)\n", (int)sizeof(PersistentRam), 2048 - (int)sizeof(PersistentRam));
	printf( "sizeof struct _state2 = %d\n", (int)sizeof( struct _state2 ) );
	printf( "sizeof while on = %d (%d free)\n", (int)sizeof(TStateWhileOn), 55 - (int)sizeof(TStateWhileOn));
	printf( "sizeof decNumber = %d\n", (int)sizeof(decNumber));
	printf( "sizeof decContext = %d\n", (int)sizeof(decContext));
#endif
}


/*
 *  Load both the RAM file and the flash emulation images
 */
void load_statefile( void )
{
	char name[ 20 ];
	int i, l;
	char *p;
	FILE *f = fopen( STATE_FILE, "rb" );
	if ( f != NULL ) {
		fread( &PersistentRam, sizeof( PersistentRam ), 1, f );
		fclose( f );
	}
	for ( i = 0; i < NUMBER_OF_FLASH_REGIONS; ++i ) {
		p = (char *) flash_region( i );
		l = SIZE_REGION * PAGE_SIZE;
		memset( p, 0xff, l );
		sprintf( name, REGION_FILE, i == 0 ? 'R' : i + '0' - 1 );
		f = fopen( name, "rb" );
		if ( f == NULL && i == 0 ) {
			// Emulate a backup
			UserFlash.backup = PersistentRam;
		}
		if ( f != NULL ) {
			fread( p, l, 1, f );
			fclose(f);
#ifdef REPAIR_CRC_ON_LOAD
			if ( checksum_region( i ) ) {
				FLASH_REGION *fr = flash_region( i );
				int l;
				for ( l = 0; l < NUMPROG; ++l ) {
					if ( fr->prog[ l ] == 0xffff
					  || fr->prog[ l ] == EMPTY_PROGRAM_OPCODE ) 
					{
						break;
					}
				}
				fr->last_prog = l + 1;
				fr->crc = crc16( fr->prog, 2 * l );
			}
#endif
		}
	}
}
#endif



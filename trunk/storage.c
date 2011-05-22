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
#include "xeq.h"
#include "storage.h"
#include "display.h"

#ifdef REALBUILD
#define BACKUP_SRAM  __attribute__((section(".backup")))
#define SLCDCMEM     __attribute__((section(".slcdcmem")))
#define USER_FLASH   __attribute__((section(".userflash")))
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BACKUP_SRAM
#define SLCDCMEM
#define USER_FLASH
#define STATE_FILE "wp34s.dat"
#define REGION_FILE "wp34s-%d.dat"
#define BACKUP_FILE "wp34s-backup.dat"
#endif

/*
 *  Setup the persistent RAM
 */
BACKUP_SRAM TPersistentRam PersistentRam;

/*
 *  Data that is saved in the SLCD controller during deep sleep
 */
SLCDCMEM TStateWhileOn StateWhileOn;

/*
 *  The user flash area:
 *  2 regions for storage of programs and/or registers
 *  Same data as in persistent RAM but in flash memory
 */
USER_FLASH TUserFlash UserFlash;

/*
 *  Define some pages in flash memory for user access
 */
#define SIZE_REGION   4
#define SIZE_BACKUP   8
#define PAGE_SIZE     256
#define PAGE_END      512
#define PAGE_BACKUP   (PAGE_END - SIZE_BACKUP)
#define PAGE_BEGIN    (PAGE_BACKUP - SIZE_REGION * NUMBER_OF_FLASH_REGIONS)

#ifdef REALBUILD
#define RAM_FUNCTION __attribute__((section(".ramfunc"),noinline))

/*
 *  Issue a command to the flash controller. Must be done from RAM.
 *  Returns zero if OK or non zero on error.
 */
RAM_FUNCTION int flash_command( unsigned int cmd )
{
	AT91C_BASE_MC->MC_FCR = cmd;
	while ( ( AT91C_BASE_MC->MC_FSR & 1 ) == 0 ) {
		// wait for flash controller to do its work
	}
	return ( AT91C_BASE_MC->MC_FSR >> 1 );
}


/*
 *  Program the flash.
 *  Returns 0 if OK or non zero on error.
 */
int program_flash( int page_no, int buffer[ 64 ] )
{
	int *f = (int *) 0x100000;  // anywhere in flash is OK
	int res;

	lock();  // No interrupts, please!

	/*
	 *  Copy the source to the flash write buffer
	 *  If buffer isn't given, assume the copying is already done.
	 */
	while ( buffer != NULL && f != (int *) 0x100100 ) {
		*f++ = *buffer++;
	}

	/*
	 *  Command the controller to erase and write the page.
	 *  Will re-enable interrupts, too.
	 */
	res = flash_command( 0x5A000003 | ( page_no << 8 ) );
	unlock();
	return res;
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
int program_flash( int page_no, int buffer[ 64 ] )
{
	char name[ 20 ];
	int r;
	FILE *f;
	int page;

	page_no -= PAGE_BEGIN;
	if ( page_no < PAGE_BACKUP - PAGE_BEGIN ) {
		r = page_no / SIZE_REGION;
		page = page_no % SIZE_REGION;
		sprintf( name, REGION_FILE, r );
	}
	else {
		r = NUMBER_OF_FLASH_REGIONS;
		page = page_no;
		strcpy( name, BACKUP_FILE );
	}
	f = fopen( name, "rb+" );
	if ( f == NULL ) f = fopen( name, "wb+" );
	if ( f == NULL ) return 1;
	fseek( f, page * PAGE_SIZE, SEEK_SET );
	if ( 1 != fwrite( buffer, PAGE_SIZE, 1, f ) ) {
		return 1;
	}
	fclose( f );

	/*
	 *  Since flash is readable memory we have to emulate this, too
	 */
	memcpy( ((char *) &UserFlash) + PAGE_SIZE * page_no, buffer, PAGE_SIZE );
	return 0;
}

#endif


/*
 *  The CCITT 16 bit CRC algorithm (X^16 + X^12 + X^5 + 1)
 */
static unsigned short int crc16( void *base, unsigned int length )
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
 *  Simple backup / restore
 *  Started with ON+"B" or ON+"R"
 *  The backup area is the last 2KB of flash (pages 504 to 511)
 *  The commands will eventually be added to a menu in the emulator.
 */
void flash_backup(void)
{
	int *p = (int *) &PersistentRam;
	int i, err = 0;

	process_cmdline_set_lift();
	init_state();
	checksum_all();

	for ( i = PAGE_BACKUP; i < PAGE_BACKUP + SIZE_BACKUP && err == 0; ++i ) {
		err = program_flash( i, p );
		p += 64;
	}
	DispMsg = err ? "Error" : "Saved";
}


void flash_restore(void)
{
	if ( checksum_backup() ) {
		DispMsg = "Invalid";
	}
	else {
		xcopy( &PersistentRam, &( UserFlash.backup ), sizeof( PersistentRam ) );
		init_state();
		DispMsg = "Restored";
	}
}


/*
 *  Write a region to flash
 */
static int write_region( int r, FLASH_REGION *fr )
{
	int *ip = (int *) fr;
	int i;
	r *= SIZE_REGION;
	r += PAGE_BEGIN;
	for ( i = 0; i < SIZE_REGION; ++i ) {
		if ( 0 != program_flash( r + i, ip ) ) {
			return ERR_IO;
		}
		ip += 64; // next page
	}
	return 0;
}


static void program_cleanup(void) {
	if (! Running)
		clrretstk();
}

/*
 *  Save the user program area to a region.
 *  Returns an error if failed.
 */
static int internal_save_program(unsigned int r)
{
	int len = State.last_prog * sizeof(unsigned short) - sizeof(unsigned short);
	FLASH_REGION region;

	program_cleanup();
	if (check_return_stack_segment(r)) {
		err( ERR_INVALID );
		return 1;
	}
	xset( &region, 0xff, sizeof( region ) );
	region.type = REGION_TYPE_PROGRAM;
	region.length = len;
	region.crc = checksum_code();
	xcopy( region.data, Prog, len );
	if ( 0 != write_region( r, &region ) ) {
		err( ERR_IO );
                return 1;
	}
        return 0;
}


void save_program( unsigned int r, enum rarg op )
{
	internal_save_program( r );
}


/*
 *  Load the user program area from a region.
 *  Returns an error if invalid.
 */
void load_program( unsigned int r, enum rarg op )
{
	FLASH_REGION *fr = UserFlash.region + r;
	int len = fr->length;

	program_cleanup();
	if ( fr->type != REGION_TYPE_PROGRAM || checksum_region( r ) || check_return_stack_segment(-1)) {
		/*
		 *  Not a valid program region
		 */
		err( ERR_INVALID );
		return;
	}
	clrprog();
	xcopy( Prog, fr->data, len );
	State.last_prog = len / 2 + 1;
	set_running_off();
}


/*
 *  Swap the user program area with a region.
 *  Returns an error if invalid or error.
 *
 *  Attention: This needs a lot of stack space!
 */
void swap_program( unsigned int r, enum rarg op )
{
	FLASH_REGION *fr = UserFlash.region + r;
	FLASH_REGION region;
	int len = fr->length;

	program_cleanup();
	if ( fr->type != REGION_TYPE_PROGRAM || checksum_region( r )  || check_return_stack_segment(-1)) {
		/*
		 *  Not a valid program region
		 */
		err( ERR_INVALID );
		return;
	}
	/*
	 *  Temporary copy
	 */
	xcopy( &region, fr, sizeof( region ) );

	/*
	 *  Save current program
	 */
	if ( internal_save_program( r ) ) {
		return;
	}

	/*
	 *  Restore temporary copy
	 */
	clrprog();
	xcopy( Prog, region.data, len );
	State.last_prog = len / 2 + 1;
	set_running_off();
}


static int internal_save_registers(unsigned int r)
{
	int len = TOPREALREG * sizeof(decimal64);
	FLASH_REGION region;

	xset( &region, 0xff, sizeof( region ) );
	region.type = REGION_TYPE_DATA;
	region.length = len;
	region.crc = crc16(Regs, len);
	xcopy( region.data, Regs, len );
	if ( 0 != write_region( r, &region ) ) {
		err( ERR_IO );
                return 1;
	}
        return 0;
}

void save_registers(unsigned int r, enum rarg op)
{
	internal_save_registers(r);
}

void load_registers(unsigned int r, enum rarg op)
{
	FLASH_REGION *fr = UserFlash.region + r;
	int len = fr->length;

	if ( fr->type != REGION_TYPE_DATA || checksum_region( r ) ) {
		/*
		 *  Not a valid register region
		 */
		err( ERR_INVALID );
		return;
	}
	xcopy( Regs, fr->data, len );
}

void swap_registers(unsigned int r, enum rarg op)
{
	FLASH_REGION *fr = UserFlash.region + r;
	FLASH_REGION region;
	int len = fr->length;

	if ( fr->type != REGION_TYPE_DATA || checksum_region( r ) ) {
		err( ERR_INVALID );
		return;
	}
	/*
	 *  Temporary copy
	 */
	xcopy( &region, fr, sizeof( region ) );

	/*
	 *  Save current program
	 */
	if ( internal_save_registers( r ) ) {
		return;
	}

	/*
	 *  Restore temporary copy
	 */
	xcopy( Regs, region.data, len );
}

/*
 *  Checksum the program area
 */
unsigned short int checksum_code(void)
{
	const unsigned short int pc = state_pc();
	int n;
	if (! isLIB(pc))
		return crc16( PersistentRam._prog, (State.last_prog-1) * sizeof(unsigned short int) );
	n = nLIB(pc) - 1;
	return crc16(UserFlash.region[n].data, UserFlash.region[n].length);
}


/*
 *  Checksum the persistent RAM area.
 *  The magic marker is always valid. This eases manipulating state files.
 */
int checksum_all( void )
{
	unsigned short int oldcrc = PersistentRam._crc;
	PersistentRam._crc =
		crc16( &PersistentRam, (char *) &PersistentRam._crc - (char *) &PersistentRam );
	return oldcrc != PersistentRam._crc && oldcrc != MAGIC_MARKER;
}


/*
 *  Checksum the backup flash region
 */
int checksum_backup( void )
{
	unsigned short int crc =
		crc16( &( UserFlash.backup ),
			(char *) &( UserFlash.backup._crc ) - (char *) &( UserFlash.backup ) );
	return crc != UserFlash.backup._crc && crc != MAGIC_MARKER;
}


/*
 *  Checksum a user flash region
 */
int checksum_region( int r )
{
	FLASH_REGION *fr = UserFlash.region + r;
	unsigned short int crc = crc16( fr->data, fr->length );
	return crc != fr->crc && crc != MAGIC_MARKER;
}


#ifndef REALBUILD
/*
 *  Save/Load state to a file (only for emulator(s)
 */
void save_state( void )
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
	printf( "sizeof RAM = %d (%d free)\n", (int)sizeof(PersistentRam), 2048 - (int)sizeof(PersistentRam));
	printf( "sizeof pointer = %d\n", (int)sizeof( char * ) );
	printf( "sizeof decNumber = %d\n", (int)sizeof(decNumber));
	printf( "sizeof decContext = %d\n", (int)sizeof(decContext));
#endif
}


/*
 *  Load both the RAM file and the flash emulation images
 */
void load_state( void )
{
	char name[ 20 ];
	int i, l;
	char *p;
	FILE *f = fopen( STATE_FILE, "rb" );
	if ( f != NULL ) {
		fread( &PersistentRam, sizeof( PersistentRam ), 1, f );
		fclose( f );
	}
	for ( i = 0; i < NUMBER_OF_FLASH_REGIONS + 1; ++i ) {
		p = ((char *) &UserFlash) + i * SIZE_REGION * PAGE_SIZE;
		if ( i < NUMBER_OF_FLASH_REGIONS ) {
			l = SIZE_REGION * PAGE_SIZE;
			sprintf( name, REGION_FILE, i );
		}
		else {
			l = SIZE_BACKUP * PAGE_SIZE;
			strcpy( name, BACKUP_FILE );
		}
		f = fopen( name, "rb" );
		if ( f == NULL ) {
			memset( p, 0xff, l );
		} else {
			fread( p, l, 1, f );
			fclose(f);
		}
	}
}
#endif



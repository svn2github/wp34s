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
#define PERSISTENT_RAM __attribute__((section(".persistentram")))
#define SLCDCMEM       __attribute__((section(".slcdcmem")))
#define VOLATILE_RAM   __attribute__((section(".volatileram")))
#define BACKUP_FLASH   __attribute__((section(".backupflash")))
#ifndef NULL
#define NULL 0
#endif
#else
// Emulator definitions
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define PERSISTENT_RAM
#define SLCDCMEM
#define VOLATILE_RAM
#define BACKUP_FLASH
#define STATE_FILE "wp31s.dat"
#define BACKUP_FILE "wp31s-backup.dat"


#endif

#include "xeq.h"
#include "storage.h"
#include "display.h"
#include "stats.h"
#include "alpha.h"

#define PAGE_SIZE	 256

/*
 *  Setup the persistent RAM
 */
PERSISTENT_RAM TPersistentRam PersistentRam;
PERSISTENT_RAM TPersistentRam UndoState, Undo2State;

#ifdef QTGUI
/*
 *  We need to define the Library space here.
 *  On the device the linker takes care of this.
 */
FLASH_REGION UserFlash;
#endif

/*
 *  Data that is saved in the SLCD controller during deep sleep
 */
SLCDCMEM TStateWhileOn StateWhileOn;

/*
 *  A private register area for XROM code in volatile RAM
 *  It replaces the local registers and flags if active.
 */
TXromParams XromParams;
VOLATILE_RAM TXromLocal XromLocal;

/* Private space for four registers temporarily
 */
VOLATILE_RAM REGISTER XromA2D[4];

/*
 *  The backup flash area:
 *  2 KB for storage of programs and registers
 *  Same data as in persistent RAM but in flash memory
 */
BACKUP_FLASH TPersistentRam BackupFlash;

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
 *  Checksum the persistent RAM area
 *  Returns non zero value if failure
 */
int checksum_ram( void )
{
	return test_checksum( &PersistentRam, sizeof( PersistentRam ) - sizeof( short ),
			      Crc, &Crc );
}


/*
 *  Checksum the backup flash region
 *  Returns non zero value if failure
 */
int checksum_backup( void )
{
	return test_checksum( &BackupFlash, sizeof( BackupFlash ) - sizeof( short ),
		              BackupFlash._crc, NULL );
}


/*
 *  Clear all - programs and registers
 */
void clrall(void) 
{
	xeq_init_contexts();
	clrreg( OP_CLREG );
	clrstk( OP_CLSTK );
	clralpha( OP_CLRALPHA );
	clrflags( OP_CLFLAGS );

	reset_shift();
	State2.test = TST_NONE;

	DispMsg = NULL;
}


/*
 *  Clear everything
 */
void reset( void ) 
{
	xset( &PersistentRam, 0, sizeof( PersistentRam ) );
	clrall();
	init_state();
	UState.contrast = 6;
#ifdef INFRARED
	State.print_delay = 10;
#endif
#ifdef DEBUG
	State2.trace = 1;
#endif
	DispMsg = "Erased";
}


#ifdef REALBUILD
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

/*
 *  Program the flash starting at destination.
 *  Returns 0 if OK or non zero on error.
 *  count is in pages, destination % PAGE_SIZE needs to be 0.
 */
static int program_flash( void *destination, void *source, int count )
{
	unsigned int *flash = (unsigned int *) destination;
	unsigned short int *sp = (unsigned short int *) source;

	lock();  // No interrupts, please!

	while ( count-- > 0 ) {
		/*
		 *  Setup the command for the controller by computing the page from the address
		 */
		const unsigned int cmd = 0x5A000003 | ( (unsigned int) flash & 0x1ff00 );
		int i;

		/*
		 *  Copy the source to the flash write buffer
		 */
		for ( i = 0; i < PAGE_SIZE / 4; ++i, sp += 2 ) {
			*flash++ = *sp | ( (unsigned int) ( sp[ 1 ] ) << 16 );
		}

		/*
		 *  Command the controller to erase and write the page.
		 */
		if ( flash_command( cmd ) ) {
			err( ERR_IO );
			break;
		}
	}
	unlock();
	return Error != 0;
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
 *  Emulate the flash in a file wp31s-backup.dat
 *  Page numbers are relative to the start of the user flash
 *  count is in pages, destination % PAGE_SIZE needs to be 0.
 */
#if defined(QTGUI) || defined(IOS)
extern char* get_backup_path();
#else
static char* get_backup_path()
{
	return BACKUP_FILE;
}
#endif

static int program_flash( void *destination, void *source, int count )
{
	char *name;
	char *dest = (char *) destination;
	FILE *f = NULL;
	int offset;

	/*
	 *  Copy the source to the destination memory
	 */
	memcpy( dest, source, count * PAGE_SIZE );

	/*
	 *  Update the file
	 */
	name = get_backup_path();
	offset = dest - (char *) &BackupFlash;
	f = fopen( name, "rb+" );
	if ( f == NULL ) {
		f = fopen( name, "wb+" );
	}
	if ( f == NULL ) {
		err( ERR_IO );
		return 1;
	}
	fseek( f, offset, SEEK_SET );
	if ( count != fwrite( dest, PAGE_SIZE, count, f ) ) {
		fclose( f );
		err( ERR_IO );
		return 1;
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
void flash_backup( enum nilop op )
{
	if ( not_running() ) {
		process_cmdline_set_lift();
		init_state();
		checksum_all();

		if ( program_flash( &BackupFlash, &PersistentRam, sizeof( BackupFlash ) / PAGE_SIZE ) ) {
			err( ERR_IO );
			DispMsg = "Error";
		}
		else {
			DispMsg = "Saved";
		}
	}
}


void flash_restore( enum nilop op )
{
	if ( not_running() ) {
		if ( checksum_backup() ) {
			err( ERR_INVALID );
		}
		else {
			xcopy( &PersistentRam, &BackupFlash, sizeof( PersistentRam ) );
			init_state();
			DispMsg = "Restored";
		}
	}
}


/*
 *  Load registers from backup
 */
void load_registers( enum nilop op )
{
	int count;
	if ( checksum_backup() ) {
		/*
		 *  Not a valid backup region
		 */
		err( ERR_INVALID );
		return;
	}
	count = NumRegs;
	if ( is_dblmode() ) {
		// Don't clobber the stack in DP mode
		count -= EXTRA_REG + STACK_SIZE;
	}
	xcopy( get_reg_n(0), get_flash_reg_n(0), count << 3 );
}


/*
 *  Load the statistical summation registers from backup
 */
void load_sigma( enum nilop op )
{
	if ( checksum_backup() ) {
		/*
		 *  Not a valid backup region
		 */
		err( ERR_INVALID );
		return;
	}
	sigmaCopy( &BackupFlash._stat_regs );
}


/*
 *  Load the configuration data from the backup
 */
void load_state( enum nilop op )
{
	if ( not_running() ) {
		if ( checksum_backup() ) {
			/*
			 *  Not a valid backup region
			 */
			err( ERR_INVALID );
			return;
		}
		xcopy( &RandS1, &BackupFlash._rand_s1, (char *) &Crc - (char *) &RandS1 );
		init_state();
		clrretstk();
	}
}


#if !defined(REALBUILD) && !defined(QTGUI) && !defined(IOS)
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
	fwrite( &UndoState, sizeof( UndoState ), 1, f );
	fclose( f );
#ifdef DEBUG
	printf( "sizeof struct _state = %d\n", (int)sizeof( struct _state ) );
	printf( "sizeof struct _ustate = %d\n", (int)sizeof( struct _ustate ) );
	printf( "sizeof RAM = %d (%d free)\n", (int)sizeof(PersistentRam), 2048 - (int)sizeof(PersistentRam));
	printf( "sizeof struct _state2 = %d\n", (int)sizeof( struct _state2 ) );
	printf( "sizeof while on = %d\n", (int)sizeof(TStateWhileOn));
	printf( "sizeof decNumber = %d\n", (int)sizeof(decNumber));
	printf( "sizeof decContext = %d\n", (int)sizeof(decContext));
#endif
}


/*
 *  Load both the RAM file and the flash emulation images
 */
void load_statefile( void )
{
	FILE *f = fopen( STATE_FILE, "rb" );
	if ( f != NULL ) {
		fread( &PersistentRam, sizeof( PersistentRam ), 1, f );
		fread( &UndoState, sizeof( UndoState ), 1, f );
		fclose( f );
	}
	f = fopen( BACKUP_FILE, "rb" );
	if ( f != NULL ) {
		fread( &BackupFlash, sizeof( BackupFlash ), 1, f );
		fclose( f );
	}
	else {
		// Emulate a backup
		BackupFlash = PersistentRam;
	}
#ifdef DEBUG
	State2.trace = 1;
	remove( TRACE_FILE );
#endif

}
#endif



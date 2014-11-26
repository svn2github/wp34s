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

#include "xeq.h"
// #include "stopwatch.h"
#include "display.h"
#include "data.h"
#include "storage.h"
// #include "serial.h"
#include "keys.h"
#include "lcd.h"

#define REGION_BACKUP   2
#define BACKUP_FILE "wp31s-backup.dat"

extern const char SvnRevision[];
extern int is_key_pressed_adapter();
extern int put_key_adapter(int);
extern void add_heartbeat_adapter(int);
extern char* get_region_path_adapter(int);
extern void* shutdown_adapter();
extern void format_display(char *buf);
#if defined(INCLUDE_STOPWATCH) && !defined(CONSOLE)
extern void updateScreen();
#endif

#if INTERRUPT_XROM_TICKS > 0
extern void increment_OnKeyTicks_adapter(void);

extern volatile char OnKeyPressed;
extern volatile unsigned int OnKeyTicks;
#endif

#define SVN_REVISION_SIZE 4
static char SvnRevisionString[SVN_REVISION_SIZE+1]={ 0 };

#define FORMATTED_DISPLAYED_NUMBER_LENGTH 65

// Replacement for memset as importing WP34-s features.h header is not possible for certain C compilers such as gcc-4.6
// as they define their own
static void memfill(void* aPointer, char aValue, int aSize)
{
	char* pointer=(char*) aPointer;
	char* endPointer=pointer+aSize;
	while(pointer!=endPointer)
	{
		*pointer++=aValue;
	}
}

void init_calculator()
{
	DispMsg = NULL;
	init_34s();
	display();
}

int is_key_pressed()
{
	return is_key_pressed_adapter();
}

int put_key(int key)
{
	return put_key_adapter(key);
}

static int fshift_locked=0;
static int gshift_locked=0;
static int hshift_locked=0;

enum shifts shift_down()
{
	if(fshift_locked)
	{
		return SHIFT_F;
	}
	else if(gshift_locked)
	{
		return SHIFT_G;
	}
	else if(hshift_locked)
	{
		return SHIFT_H;
	}
	else
	{
		return SHIFT_N;
	}
}

int is_not_shifted()
{
	return cur_shift()==SHIFT_N;
}

int is_hshifted()
{
	return cur_shift()==SHIFT_H;
}

void set_fshift_locked(int a_fshift_locked)
{
	fshift_locked=a_fshift_locked;
}

void set_gshift_locked(int a_gshift_locked)
{
	gshift_locked=a_gshift_locked;
}

void set_hshift_locked(int an_hshift_locked)
{
	hshift_locked=an_hshift_locked;
}

void add_heartbeat()
{
	++Ticker;
#if INTERRUPT_XROM_TICKS > 0
	increment_OnKeyTicks_adapter();
#endif
	if(Pause)
	{
		--Pause;
	}
	if(++Keyticks>1000)
	{
		Keyticks=1000;
	}
	add_heartbeat_adapter(K_HEARTBEAT);

#if defined(INCLUDE_STOPWATCH) && !defined(CONSOLE)
	if(KeyCallback==NULL && StopWatchRunning && (Ticker % STOPWATCH_BLINK)==0) {
		dot(LIT_EQ, !is_dot(LIT_EQ));
		updateScreen();
	}
#endif
}

void forward_keycode(int key)
{
#ifdef INCLUDE_STOPWATCH
	if(KeyCallback!=NULL)
	{
		key=(*KeyCallback)(key);
	}
	else
	{
#endif
	process_keycode(key);
	if (key != K_HEARTBEAT && key != K_RELEASE ) {
		Keyticks = 0;
	}

#ifdef INCLUDE_STOPWATCH
	}
#endif
}

void forward_key_released()
{
	put_key_adapter(K_RELEASE);
}

void shutdown()
{
	shutdown_adapter();
}

char* get_memory()
{
	return (char*) &PersistentRam;
}

int get_memory_size()
{
	return sizeof(PersistentRam);
}

char* get_backup()
{
	return (char*) &BackupFlash;
}

int get_backup_size()
{
	return sizeof(BackupFlash);
}

char* get_user_flash()
{
	return (char*) &UserFlash;
}

int get_user_flash_size()
{
	return sizeof(UserFlash);
}

char* get_region_path(int region_index)
{
	return get_region_path_adapter(region_index);
}

void prepare_memory_save()
{
	process_cmdline_set_lift();
	init_state();
	checksum_all();
}

void after_state_load()
{
}

void after_backup_load()
{
}

char* get_backup_path()
{
	return BACKUP_FILE;
}


#if 0  /* ??? DELETE? */
void after_library_load()
{
	init_library();
}
#endif

int get_region_backup_index()
{
	return REGION_BACKUP;
}

void reset_wp34s()
{
	memfill(&PersistentRam, 0, sizeof(PersistentRam));
	init_34s();
	display();
}

char* get_version_string()
{
	return VERSION_STRING;
}

char* get_svn_revision_string()
{
	int i;

	if(SvnRevisionString[0]==0)
	{
		for(i=0; i<SVN_REVISION_SIZE; i++)
		{
			SvnRevisionString[i]=SvnRevision[i];
		}
		SvnRevisionString[SVN_REVISION_SIZE]=0;
	}
	return SvnRevisionString;
}


char* get_formatted_register(int anIndex)
{
	static char buffer[FORMATTED_DISPLAYED_NUMBER_LENGTH];

	memfill(buffer, 0, FORMATTED_DISPLAYED_NUMBER_LENGTH);
	format_reg(anIndex, buffer);
	return buffer;
}

char* get_formatted_displayed_number()
{
	static char buffer[FORMATTED_DISPLAYED_NUMBER_LENGTH];

	memfill(buffer, 0, FORMATTED_DISPLAYED_NUMBER_LENGTH);
	format_display(buffer);
	return buffer;
}

#if HAS_SERIAL
int forward_byte_received(short byte)
{
	return byte_received(byte);
}
#endif

#include "../translate.c"


static int fill_displayed_text(char *p, int *buffer)
{
	int* b = buffer;
	int only_blanks = 1;

	memfill( buffer, 0, sizeof( buffer ));
	while(*p != '\0')
	{
		if (*p != ' ' && *p != 006)
		{
			only_blanks = 0;
		}
		*b++ = unicode[*p++ & 0xff];
	}
	return !only_blanks;
}

int* get_displayed_text()
{
	static int buffer[NUMALPHA + 1];
	if(!fill_displayed_text(LastDisplayedText, buffer)) {
		fill_displayed_text(Alpha, buffer);
	}
	return buffer;
}

char* get_last_displayed()
{
	return LastDisplayedText;
}

char* get_last_displayed_number()
{
	return LastDisplayedNumber;
}

char* get_last_displayed_exponent()
{
	return LastDisplayedExponent;
}

int is_small_font(char *p)
{
	return  State2.disp_small || pixel_length(p, 0) > BITMAP_WIDTH+1;
}

char* get_register_names()
{
	return REGNAMES;
}

int get_first_register_index()
{
	return regX_idx;
}

int get_maxnumregs()
{
	return NUMREG;
}

int get_numregs()
{
	return NumRegs+STACK_SIZE+EXTRA_REG;
}

int is_runmode()
{
	return State2.runmode;
}

int is_catalogue_mode()
{
	return State2.catalogue!=CATALOGUE_NONE;
}

int is_complex_mode()
{
	return State2.cmplx;
}

char get_complex_prefix()
{
	return COMPLEX_PREFIX;
}

void execute_catpos(int aCatPos)
{
	const enum catalogues cat = (enum catalogues) State2.catalogue;
	State.catposition[cat]=aCatPos;
	put_key_adapter(K20);
	put_key_adapter(K_RELEASE);
}

int get_catpos()
{
	const enum catalogues cat = (enum catalogues) State2.catalogue;
	return State.catposition[cat];
}

void forward_catalog_selection(int aCatPos)
{
	const enum catalogues cat = (enum catalogues) State2.catalogue;
	State.catposition[cat]=aCatPos;
	display();
}

void close_catalog()
{
	put_key_adapter(K60);
}

int uparrow_code()
{
	return K40;
}

int downarrow_code()
{
	return K50;
}

int backspace_code()
{
	return K24;
}

void forward_set_IO_annunciator()
{
	set_IO_annunciator();
}

char isForcedDispPlot()
{
	return forceDispPlot;
}

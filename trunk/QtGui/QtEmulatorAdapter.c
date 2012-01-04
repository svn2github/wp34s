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
#include "stopwatch.h"
#include "display.h"
#include "data.h"
#include "storage.h"
#include "serial.h"

extern const char SvnRevision[];
extern int is_key_pressed_adapter();
extern int put_key_adapter(int);
extern void add_heartbeat_adapter(int);
extern char* get_region_path_adapter(int);

#define SVN_REVISION_SIZE 4
static char SvnRevisionString[SVN_REVISION_SIZE+1]={ 0 };

#define FORMATTED_DISPLAYED_NUMBER_LENGTH 30

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

static int hshift_locked=0;

enum shifts shift_down()
{
	return hshift_locked?SHIFT_H:SHIFT_N;
}

void set_hshift_locked(int an_hshift_locked)
{
	hshift_locked=an_hshift_locked;
}

void add_heartbeat()
{
	++Ticker;
	if(Pause)
	{
		--Pause;
	}
	if(++Keyticks>1000)
	{
		Keyticks=1000;
	}
	add_heartbeat_adapter(K_HEARTBEAT);
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
}

char* get_memory()
{
	return (char*) &PersistentRam;
}

int get_memory_size()
{
	return sizeof(PersistentRam);
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
	init_34s();
}

void after_backup_load()
{
}


void after_library_load()
{
	init_library();
}

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

char* get_formatted_displayed_number()
{
	static char buffer[FORMATTED_DISPLAYED_NUMBER_LENGTH];

	memfill(buffer, 0, FORMATTED_DISPLAYED_NUMBER_LENGTH);
	format_reg(regX_idx, buffer);
	return buffer;
}

char *get_displayed_text()
{
 	return (char *) (DispMsg == NULL ? Alpha : DispMsg);
}

int forward_byte_received(short byte)
{
	return byte_received(byte);
}

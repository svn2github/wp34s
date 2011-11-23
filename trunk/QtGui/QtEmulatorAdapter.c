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

extern int is_key_pressed_adapter();
extern int put_key_adapter(int);
extern void add_heartbeat_adapter(int);

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

enum shifts shift_down()
{
	return SHIFT_N;
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

extern char* get_memory()
{
	return (char*) &PersistentRam;
}

extern int get_memory_size()
{
	return sizeof(PersistentRam);
}

void prepare_memory_save()
{
	process_cmdline_set_lift();
	init_state();
	checksum_all();
}

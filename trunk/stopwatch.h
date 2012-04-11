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

#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__


/*
 * Optional features are defined in features.h
 */
#include "features.h"

#ifdef INCLUDE_STOPWATCH

extern void stopwatch(enum nilop op);
extern int (*KeyCallback)(int);
typedef struct _stopwatch_status {
	int running:1;
	int display_tenths:1;
	int	show_memory:1;
	int select_memory_mode:1;
	int rcl_mode:1;
} TStopWatchStatus;

extern TStopWatchStatus StopWatchStatus;
#define StopWatchRunning (StopWatchStatus.running)

#endif

#endif

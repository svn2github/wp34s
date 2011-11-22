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

#include "decn.h"
#include "xeq.h"
#include "consts.h"
#include "alpha.h"

#ifdef INCLUDE_STOPWATCH
#ifndef REALBUILD
#if defined(WIN32) && !defined(QTGUI) && !defined(__GNUC__)
#include "win32.h"
#else
#include <sys/time.h>
#endif
#endif // REALBUILD
#include "display.h"
#include "keys.h"
#endif // INCLUDE_STOPWATCH

#ifdef REALBUILD
#include "atmel/rtc.h"
#else
#include <time.h>
#endif


#ifdef INCLUDE_STOPWATCH

#define StopWatchKeyticks         (StateWhileOn._keyticks)
#define STOPWATCH_APD_TICKS 1800 // 3 minutes

struct _stopwatch_status {
	int running:1;
	int display_tenths:1;
	int	 stopwatch_show_memory:1;
	int stopwatch_select_memory_mode:1;
} StopwatchStatus={ 0, 1, 0, 0, };


int (*KeyCallback)(int)=(int (*)(int)) NULL;
long long int FirstTicker=-1;
long long int Stopwatch=0;
signed char StopwatchMemory=0;
signed char StopwatchMemoryFirstDigit=-1;
char* StopWatchMessage;

#define STOPWATCH_RS K63
#define STOPWATCH_EXIT K60
#define STOPWATCH_CLEAR K24
#define STOPWATCH_EEX K23
#define STOPWATCH_STORE K20
#define STOPWATCH_UP K40
#define STOPWATCH_DOWN K50
#define STOPWATCH_SHOW_MEMORY K04

#define MAX_STOPWATCH_MEMORY 100

long long int getTicker() {
	#if defined(WINGUI) || defined(QTGUI) || defined(REALBUILD)
    return Ticker;
#else
    struct timeval tv;
    gettimeofday(&tv,(struct timezone*) NULL);
    return tv.tv_sec * 10 + tv.tv_usec / 100000;
#endif
}

static void fill_exponent(char* exponent)
{
	if(StopwatchStatus.stopwatch_select_memory_mode) {
		if(StopwatchMemoryFirstDigit>=0) {
			exponent[0]='0'+StopwatchMemoryFirstDigit;
		} else {
			exponent[0]='_';
		}
		exponent[1]='_';
	} else {
		num_arg_0(exponent, StopwatchMemory, 2);
	}
	exponent[2]=0;
}

static void display_stopwatch(char* message) {
	char buf[13], *p;
	char exponent[3];
	int tenths, secs, mins, hours;
	
	tenths=Stopwatch%10;	
	secs=(Stopwatch/10)%60;
	mins=(Stopwatch/600)%60;
	hours=(Stopwatch/36000)%99;
	fill_exponent(exponent);
	if(StopwatchStatus.display_tenths) {
		p=buf;
		*p++=' ';
		p=num_arg_0(p, hours, 2);
		*p++='h';
		p=num_arg_0(p, mins, 2);
		*p++='\'';
		p=num_arg_0(p, secs, 2);
		*p++='"';
		*p++=' ';
		p=num_arg_0(p, tenths, 1);
		*p=0;
	} else {
		p=buf;
		*p++=' ';
		p=num_arg_0(p, hours, 2);
		*p++='h';
		p=num_arg_0(p, mins, 2);
		*p++='\'';
		p=num_arg_0(p, secs, 2);
		*p++='"';
		*p=0;
	}
	stopwatch_message(message, buf, -1, StopwatchStatus.stopwatch_show_memory?exponent:(char*)NULL);
}

static void store_stopwatch_in_memory() {
	decimal64 *memory = get_reg_n((StopwatchMemory++)%MAX_STOPWATCH_MEMORY);
	int tenths, secs, mins, hours;
	decNumber t1, t2, s1, s2, m1, m2, h1, h2;
	
	StopwatchStatus.stopwatch_show_memory=1;

	tenths=Stopwatch%10;	
	secs=(Stopwatch/10)%60;
	mins=(Stopwatch/600)%60;
	hours=(Stopwatch/36000)%99;

	int_to_dn(&t1, tenths);
	dn_mulpow10(&t2, &t1, -5);
	int_to_dn(&s1, secs);
	dn_mulpow10(&s2, &s1, -4);
	int_to_dn(&m1, mins);
	dn_mulpow10(&m2, &m1, -2);
	int_to_dn(&h1, hours);
	dn_add(&h2, &h1, &m2);
	dn_add(&h1, &h2, &s2);
	dn_add(&h2, &h1, &t2);
	packed_from_number(memory, &h2);
}

static int get_digit(int key) {
		switch(key)	{
			case K31: return 7;
			case K32: return 8;
			case K33: return 9;
			case K41: return 4;
			case K42: return 5;
			case K43: return 6;
			case K51: return 1;
			case K52: return 2;
			case K53: return 3;
			case K61: return 0;
		}
		return -1;
}

static void toggle_running() {
	StopwatchStatus.running=!StopwatchStatus.running;
	if(StopwatchStatus.running) {
		if(FirstTicker<0) {
			FirstTicker=getTicker();
		}
		else {
			FirstTicker=getTicker()-Stopwatch;
		}
	}
}

static int process_select_memory_key(int key) {
	switch(key)	{
			case STOPWATCH_RS: {
				toggle_running();
				StopwatchStatus.stopwatch_select_memory_mode=0;
				break;
			}
			case STOPWATCH_EXIT: {
				StopwatchStatus.stopwatch_select_memory_mode=0;
				break;
			}
			case STOPWATCH_CLEAR: {
				if(StopwatchMemoryFirstDigit<0) {
					StopwatchStatus.stopwatch_select_memory_mode=0;
				} else {
					StopwatchMemoryFirstDigit=-1;
				}
				break;
			   }
			// Digits
			case K31:
			case K32:
			case K33:
			case K41:
			case K42:
			case K43:
			case K51:
			case K52:
			case K53:
			case K61: {
				if(StopwatchMemoryFirstDigit<0) {
					StopwatchMemoryFirstDigit=get_digit(key);
				} else {
					StopwatchMemory=StopwatchMemoryFirstDigit*10+get_digit(key);
					StopwatchMemoryFirstDigit=-1;
					StopwatchStatus.stopwatch_select_memory_mode=0;
				}
				break;
			  }
	}

	return -1;
}

static int process_stopwatch_key(int key)
{
	switch(key)	{
			case STOPWATCH_RS: {
				toggle_running();
				break;
			}
			case STOPWATCH_EXIT: {
				KeyCallback=(int(*)(int))NULL;
				return STOPWATCH_EXIT;
			}
			case STOPWATCH_CLEAR: {
				Stopwatch=0;
				if(StopwatchStatus.running)	{
					FirstTicker=getTicker();
				} else {
					FirstTicker=-1;
					Stopwatch=0;
				}
				break;
			   }
			case STOPWATCH_EEX: {
				StopwatchStatus.display_tenths=!StopwatchStatus.display_tenths;
				break;
				}
			case STOPWATCH_STORE: {
				store_stopwatch_in_memory();
				break;
				}
			case STOPWATCH_UP: {
				if(StopwatchMemory<MAX_STOPWATCH_MEMORY-1) {
					StopwatchMemory++;
				}
				break;
				}
			case STOPWATCH_DOWN: {
				if(StopwatchMemory>0) {
					StopwatchMemory--;
				}
				break;
				}
			case STOPWATCH_SHOW_MEMORY: {
				StopwatchStatus.stopwatch_show_memory=!StopwatchStatus.stopwatch_show_memory;
				break;
				}
			 //Digits
			case K31:
			case K32:
			case K33:
			case K41:
			case K42:
			case K43:
			case K51:
			case K52:
			case K53:
			case K61: {
				StopwatchStatus.stopwatch_select_memory_mode=1;
				StopwatchMemoryFirstDigit=-1;
				StopwatchStatus.stopwatch_show_memory=1;
				return process_select_memory_key(key);
			  }
			}
	return -1;
}

int stopwatch_callback(int key) {
	// Trying to make things work whether char are unsigned or not
	short shift=256*(StopWatchMessage[11] & 0x7F);
	shift+=(StopWatchMessage[10] & 0x7F)+(StopWatchMessage[10]&0x80?128:0);
	shift*=StopWatchMessage[11]&0x80?-1:1;

		
	if(StopwatchStatus.running)	{
		long long int ticker=getTicker();
		Stopwatch=ticker-FirstTicker+(shift==0?0:(ticker-FirstTicker)/shift);
		StopWatchKeyticks=0;
	} else if(StopWatchKeyticks >= STOPWATCH_APD_TICKS) {
		KeyCallback=(int(*)(int)) NULL;
		return -1;
	}

	if(key!=K_HEARTBEAT && key!=K_RELEASE) {
		StopWatchKeyticks=0;
		if(StopwatchStatus.stopwatch_select_memory_mode) {
			key=process_select_memory_key(key);
		} else {
			key=process_stopwatch_key(key);
		}
	}
	display_stopwatch(StopWatchMessage);

	return key==K_RELEASE?-1:key;
}

void stopwatch(decimal64 *nul1, decimal64 *nul2, enum nilop op) {
	StopWatchMessage="STOPWATCH\0\0\0";
	StopwatchStatus.stopwatch_show_memory=0;
	StopwatchStatus.stopwatch_select_memory_mode=0;
	StopwatchMemory=0;
	StopwatchMemoryFirstDigit=-1;
	KeyCallback=&stopwatch_callback;
}

#endif // INCLUDE_STOPWATCH

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
#include "lcd.h"
#include "keys.h"
#include "stopwatch.h"

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

#define STOPWATCH_MESSAGE "STOPWATCH"
#define StopWatchKeyticks         (StateWhileOn._keyticks)
#define STOPWATCH_APD_TICKS 1800 // 3 minutes

TStopWatchStatus StopWatchStatus; // ={ 0, 1, 0, 0, };

int (*KeyCallback)(int)=(int (*)(int)) NULL;
unsigned long FirstTicker;
unsigned long StopWatch;
unsigned char StopWatchMemory;
signed char StopWatchMemoryFirstDigit; //-1;
signed char RclMemory;
unsigned char RclMemoryRemanentDisplay;

#define STOPWATCH_RS K63
#define STOPWATCH_EXIT K60
#define STOPWATCH_CLEAR K24
#define STOPWATCH_EEX K23
#define STOPWATCH_STORE K20
#define STOPWATCH_STORE_ROUND K62
#define STOPWATCH_UP K40
#define STOPWATCH_DOWN K50
#define STOPWATCH_SHOW_MEMORY K04
#define STOPWATCH_RCL K11

#define RCL_MEMORY_REMANENCE 3

unsigned long getTicker() {
#ifndef CONSOLE
    return Ticker;
#else
    struct timeval tv;
    gettimeofday(&tv,(struct timezone*) NULL);
    return tv.tv_sec * 10 + tv.tv_usec / 100000;
#endif
}

static void fill_exponent(char* exponent) {
	if(StopWatchStatus.select_memory_mode) {
		if(StopWatchMemoryFirstDigit>=0) {
			exponent[0]='0'+StopWatchMemoryFirstDigit;
		} else {
			exponent[0]='_';
		}
		exponent[1]='_';
	} else {
		num_arg_0(exponent, StopWatchMemory, 2);
	}
	exponent[2]=0;
}

static void display_stopwatch() {
	char buf[13], *p;
	char exponent[3];
	int tenths, secs, mins, hours;
	char rclMessage[8];
	char display_rcl_message;
	
	tenths=StopWatch%10;	
	secs=(StopWatch/10)%60;
	mins=(StopWatch/600)%60;
	hours=(StopWatch/36000)%99;
	fill_exponent(exponent);
	if(StopWatchStatus.display_tenths) {
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
	display_rcl_message=StopWatchStatus.rcl_mode || RclMemory>=0;
	if(display_rcl_message) {
		char* rp=scopy(rclMessage, "RCL\006\006");
		if(RclMemory>=0) {
			rp=num_arg_0(rp, RclMemory, 2);
		}
		else {
			if(StopWatchMemoryFirstDigit>=0) {
				*rp++='0'+StopWatchMemoryFirstDigit;
			}
			*rp++='_';
		}
		*rp=0;
	}
	stopwatch_message(display_rcl_message?rclMessage:STOPWATCH_MESSAGE, buf, -1, StopWatchStatus.show_memory?exponent:(char*)NULL);
}

static void store_stopwatch_in_memory() {
	int max_registers=global_regs();
	if(max_registers>0) {
		decNumber s, s2, hms;
		StopWatchStatus.show_memory=1;
		ullint_to_dn(&s, StopWatch);
		dn_divide(&s2, &s, &const_360);
		dn_divide(&s, &s2, &const_100);
		decNumberHR2HMS(&hms, &s);
		setRegister(StopWatchMemory, &hms);
		StopWatchMemory=(StopWatchMemory+1)%max_registers;
	}
}

static int get_digit(int key) {
	int digit=keycode_to_digit_or_register(key);
	return digit>=0 && digit<=9 && global_regs() > 0 ? digit : -1;
}

static void toggle_running() {
	StopWatchStatus.running=!StopWatchStatus.running;
	if(StopWatchStatus.running) {
		if(FirstTicker==0) {
			FirstTicker=getTicker();
		}
		else {
			FirstTicker=getTicker()-StopWatch;
		}
	}
}

static void recall_memory(int index) {
	decNumber memory, s, s2, hms;
	unsigned long previous;
	int sign;

	StopWatchMemoryFirstDigit=-1;
	RclMemory=index;
	StopWatchStatus.rcl_mode=0;
	RclMemoryRemanentDisplay=0;
	getRegister(&memory, index);
	decNumberHMS2HR(&hms, &memory);
	dn_multiply(&s2, &hms, &const_100);
	dn_multiply(&s, &s2, &const_360);
	previous=(unsigned long) dn_to_ull(&s, &sign);

	FirstTicker=getTicker()-previous;
	StopWatch=previous;
}

static void end_memory_selection(int index) {
	if(StopWatchStatus.rcl_mode){
		recall_memory(index);
	}
	else {
		StopWatchMemory=index;
		StopWatchMemoryFirstDigit=-1;
		StopWatchStatus.select_memory_mode=0;
	}
}

static int process_select_memory_key(int key) {
	int digit = get_digit(key);
	switch(key)	{
			case STOPWATCH_RS: {
				toggle_running();
				StopWatchStatus.select_memory_mode=0;
				StopWatchStatus.rcl_mode=0;
				break;
			}
			case STOPWATCH_EXIT: {
				StopWatchStatus.select_memory_mode=0;
				StopWatchStatus.rcl_mode=0;
				break;
			}
			case STOPWATCH_CLEAR: {
				if(StopWatchMemoryFirstDigit<0) {
					StopWatchStatus.select_memory_mode=0;
					StopWatchStatus.rcl_mode=0;
				} else {
					StopWatchMemoryFirstDigit=-1;
				}
				break;
			   }
			case STOPWATCH_STORE: {
				if(StopWatchMemoryFirstDigit>=0) {
					end_memory_selection(StopWatchMemoryFirstDigit);
				}
			}
			default: {
				if (digit >= 0) {
					int max_registers=global_regs();
					// Digits
					if(StopWatchMemoryFirstDigit<0) {
						if(digit<=max_registers/10) {
							StopWatchMemoryFirstDigit=digit;
						} else if(max_registers<=10 && digit<max_registers) {
							end_memory_selection(digit);
						}
					} else {
					int index=StopWatchMemoryFirstDigit*10+digit;
					if(index<max_registers) {
						end_memory_selection(index);
					}
					}
				}
				break;
			  }
	}

	return -1;
}

static int process_stopwatch_key(int key) {
	int max_registers=global_regs();
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
				StopWatch=0;
				if(StopWatchStatus.running)	{
					FirstTicker=getTicker();
				} else {
					FirstTicker=-1;
					StopWatch=0;
				}
				break;
			   }
			case STOPWATCH_EEX: {
				StopWatchStatus.display_tenths=!StopWatchStatus.display_tenths;
				break;
				}
			case STOPWATCH_STORE: {
				store_stopwatch_in_memory();
				break;
				}
			case STOPWATCH_STORE_ROUND: {
				store_stopwatch_in_memory();
				FirstTicker=getTicker();
				break;
				}
			case STOPWATCH_UP: {
				if(StopWatchMemory<max_registers-1) {
					StopWatchMemory++;
				}
				break;
				}
			case STOPWATCH_DOWN: {
				if(StopWatchMemory>0) {
					StopWatchMemory--;
				}
				break;
				}
			case STOPWATCH_SHOW_MEMORY: {
				StopWatchStatus.show_memory=!StopWatchStatus.show_memory;
				break;
				}
			case STOPWATCH_RCL: {
				StopWatchStatus.rcl_mode=max_registers>0;
				StopWatchMemoryFirstDigit=-1;
				break;
				}
			default: {
				if (get_digit(key) >= 0) {
					//Digits
					StopWatchStatus.select_memory_mode=1;
					StopWatchMemoryFirstDigit=-1;
					StopWatchStatus.show_memory=1;
					return process_select_memory_key(key);
				}
			  }
			}
	return -1;
}

int stopwatch_callback(int key) {
	if(StopWatchStatus.running)	{
		StopWatch=getTicker()-FirstTicker;
		StopWatchKeyticks=0;
	} else if(StopWatchKeyticks >= STOPWATCH_APD_TICKS) {
		KeyCallback=(int(*)(int)) NULL;
		return -1;
	}

	if(key!=K_HEARTBEAT && key!=K_RELEASE) {
		StopWatchKeyticks=0;
		if(StopWatchStatus.select_memory_mode || StopWatchStatus.rcl_mode) {
			key=process_select_memory_key(key);
		} else {
			key=process_stopwatch_key(key);
		}
	}
	display_stopwatch();
	if(RclMemory>=0 && RclMemoryRemanentDisplay++ > RCL_MEMORY_REMANENCE) {
		RclMemory=-1;
	}

	return key==K_RELEASE?-1:key;
}

void stopwatch(enum nilop op) {
	if (Running) {
		err(ERR_ILLEGAL);
		return;
	}
	if(!StopWatchRunning) {
		StopWatchStatus.show_memory=0;
		StopWatchStatus.display_tenths=1;
		StopWatchMemory=0;
		RclMemory=-1;
		StopWatch=0;
		FirstTicker=0;
	}
	StopWatchStatus.select_memory_mode=0;
	StopWatchStatus.rcl_mode=0;
	StopWatchMemoryFirstDigit=-1;
	KeyCallback=&stopwatch_callback;
}

#endif // INCLUDE_STOPWATCH

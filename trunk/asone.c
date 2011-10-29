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
 *  This file is compiled "en block" for better space optimisation
 */
#ifdef REALBUILD
#ifdef __GNUC__
__attribute__((externally_visible)) void LowLevelInit(void);
#endif
//#define TINY_BUILD
//#pragma GCC optimize 1
#include "main.c"
//#pragma GCC optimize "s"
#include "atmel/board_lowlevel.c"
#include "atmel/board_memories.c"
#include "atmel/aic.c"
#include "atmel/pmc.c"
#include "atmel/rtc.c"
#include "atmel/slcdc.c"
#include "atmel/supc.c"
#else
#include "console.c"
#endif

#include "charmap.c"
#include "commands.c"
#include "alpha.c"
#include "complex.c"
#include "consts.c"
#include "date.c"
#include "decn.c"
#include "display.c"
#include "int.c"
#include "keys.c"
#include "lcd.c"
#include "prt.c"
#include "serial.h"
#include "stats.c"
#include "storage.c"
#include "string.c"
#include "xeq.c"
#include "serial.c"
#include "matrix.c"
#include "xrom.c"
#include "stopwatch.c"
#if 0
#include "decNumber/decNumber.c"
#include "decNumber/decContext.c"
#include "decNumber/decimal64.c"
#endif

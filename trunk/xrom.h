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

#ifndef __XROM_H__
#define __XROM_H__

#include "xeq.h"


/* Entry points */
#define ENTRY_SIGMA	99	/* Same order: as RARG_SUM, RARG_PROD, RARG_SOLVE, RARG_INTG */
#define ENTRY_PI	98
#define ENTRY_SOLVE	97
#define ENTRY_DERIV	96
#define ENTRY_2DERIV	95
#define ENTRY_INTEGRATE	94

/* Labels - global */
#define XROM_CHECK		40
#define XROM_EXIT		41
#define XROM_EXITp1		42

extern const s_opcode xrom[];
extern const unsigned short int xrom_size;

#endif

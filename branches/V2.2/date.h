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

#ifndef _DATE_H_
#define _DATE_H_

#include "xeq.h"

extern decNumber *dateDayOfWeek(decNumber *res, const decNumber *x);
extern decNumber *dateYear(decNumber *res, const decNumber *x);
extern decNumber *dateMonth(decNumber *res, const decNumber *x);
extern decNumber *dateDay(decNumber *res, const decNumber *x);

extern decNumber *dateAdd(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *dateDelta(decNumber *res, const decNumber *x, const decNumber *y);

extern decNumber *dateToJ(decNumber *res, const decNumber *x);
extern decNumber *dateFromJ(decNumber *res, const decNumber *x);

extern decNumber *dateEaster(decNumber *res, const decNumber *x);
extern void date_isleap(decimal64 *, decimal64 *, enum nilop op);

extern void date_alphaday(decimal64 *, decimal64 *, enum nilop op);
extern void date_alphamonth(decimal64 *, decimal64 *, enum nilop op);
extern void date_alphadate(decimal64 *r, decimal64 *nul, enum nilop op);
extern void date_alphatime(decimal64 *nul1, decimal64 *nul2, enum nilop op);

extern void date_time(decimal64 *, decimal64 *, enum nilop op);
extern void date_date(decimal64 *, decimal64 *, enum nilop op);
extern void date_settime(decimal64 *, decimal64 *, enum nilop op);
extern void date_setdate(decimal64 *, decimal64 *, enum nilop op);
extern void date_24(decimal64 *, decimal64 *, enum nilop op);

extern void op_jgchange(decimal64 *, decimal64 *, enum nilop op);

#endif

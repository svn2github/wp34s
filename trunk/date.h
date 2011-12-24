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
extern decNumber *dateExtraction(decNumber *res, const decNumber *x);

extern decNumber *dateAdd(decNumber *res, const decNumber *x, const decNumber *y);
extern decNumber *dateDelta(decNumber *res, const decNumber *x, const decNumber *y);

extern decNumber *dateToJ(decNumber *res, const decNumber *x);
extern decNumber *dateFromJ(decNumber *res, const decNumber *x);

extern decNumber *dateEaster(decNumber *res, const decNumber *x);
extern void date_isleap(REGISTER *, REGISTER *, enum nilop op);

extern void date_alphaday(REGISTER *, REGISTER *, enum nilop op);
extern void date_alphamonth(REGISTER *, REGISTER *, enum nilop op);
extern void date_alphadate(REGISTER *r, REGISTER *nul, enum nilop op);
extern void date_alphatime(REGISTER *nul1, REGISTER *nul2, enum nilop op);

extern void date_time(REGISTER *, REGISTER *, enum nilop op);
extern void date_date(REGISTER *, REGISTER *, enum nilop op);
extern void date_settime(REGISTER *, REGISTER *, enum nilop op);
extern void date_setdate(REGISTER *, REGISTER *, enum nilop op);
extern void date_24(REGISTER *, REGISTER *, enum nilop op);

extern void op_jgchange(REGISTER *, REGISTER *, enum nilop op);

#endif

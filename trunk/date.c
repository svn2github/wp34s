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

#include "date.h"
#include "decn.h"
#include "xeq.h"
#include "consts.h"
#include "alpha.h"

#ifdef REALBUILD
#include "atmel/rtc.h"
#else
#include <time.h>
#endif

/* Test if a date is Gregorian or Julian
 * We do this naively by assuming the change happened after the
 * 2nd of September 1752 (the next day being the 14th).
 */
static int isGregorian(int year, int month, int day) {
	if (year > 1752)
		return 1;
	if (year < 1752)
		return 0;
	if (month < 9)
		return 0;
	if (month > 9)
		return 1;
	if (day < 14)		// 3 .. 13 don't exist, but call them Julian
		return 0;
	return 1;
}


/* Convert a date to a Julian day number
 */
static int JDN(int year, int month, int day) {
	const int a = (14 - month) / 12;
	const int y = year + 4800 - a;
	const int m = month + 12 * a - 3;
	int jdn;

	if (isGregorian(year, month, day))
		jdn = -y/100 + y/400 - 32045;
	else
		jdn = -32083;
	jdn += day + (153 * m + 2) / 5 + 365*y + y/4;
	return jdn;
}


/* Convert a julian day number to day, month and year.
 */
static void JDN2(int J, int *year, int *month, int *day) {
	int b, c, y, d, e, m;

	if (J >= 2361222) {	// Gregorian
		const int a = J + 32044;
		b = (4*a+3)/146097;
		c = a - (b*146097)/4;
		b *= 100;
	} else {		// Julian
		b = 0;
		c = J + 32082;
	}

	d = (4*c+3)/1461;
	e = c - (1461*d)/4;
	m = (5*e+2)/153;

	*day   = e - (153*m+2)/5 + 1;
	*month = m + 3 - 12*(m/10);

	y = b + d - 4800 + m/10;
//	if (y <= 0)	y--;
	*year = y;
}


static int isleap(int year) {
	if (year % 4 == 0) {		// leap year candidate
		if (isGregorian(year, 1, 1)) {
			// Gregorian is leap if not divisible by 100 or if devisible by 400
			if (year % 100 == 0) {
				if (year % 400 == 0)
					return 1;
				return 0;
			}
			//return 1;
		}
		return 1;
	}
	return 0;
}


/* Given a year and month, return the number of days in that month.
 * This is primarily used to range check days.
 */
static int month_lengths(int year, int month) {
	switch (month) {
	default:				return 31;
	case 4:	case 6:	case 9:	case 11:	return 30;
	case 2:
		return isleap(year)?29:28;
	}
}


/* Validate that a date is in fact within our calculation range.
 */
static int check_date(int year, int month, int day) {
	if (year < -4799 || year > 9999)
		return -1;
	if (month < 1 || month > 12)
		return -1;
	if (day < 1 || day > month_lengths(year, month))
		return -1;
	if (year == 1752 && month == 9) {
		if (day > 2 && day < 14)
			return -1;
	}
	return 0;
}



/* Day of week.
 * Display the day name and return the day as an integer (1 .. 7).
 */
static int day_of_week(int year, int month, int day, const char **msg) {
	int h;
	const char *const days = "\007\016\025\035\047\060\067"
				"Sunday\0Monday\0Tuesday\0Wednesday\0"
				"Thursday\0Friday\0Saturday";
	h = (JDN(year, month, day)+1) % 7;
	if (msg != NULL)
		*msg = days + ((unsigned char *)days)[h];
	return h!=0?h:7;
}


/* Convert a (y, m, d) triple into a decimal number that represents the date.
 * Some care is required for overflow and the like.
 */
static decNumber *build_date(decNumber *res, int year, int month, int day, decContext *ctx) {
	decNumber y, m, d, x;
	int bc;

	if (check_date(year, month, day)) {
		set_NaN(res);
		return res;
	}
	if (year < 0) {
		year = -year;
		bc = 1;
	} else	bc = 0;

	int_to_dn(&y, year, ctx);
	int_to_dn(&m, month, ctx);
	int_to_dn(&d, day, ctx);

	switch (State.date_mode) {
	case DATE_YMD:
		decNumberMultiply(&x, &d, &const_0_01, ctx);
		decNumberAdd(&d, &x, &m, ctx);
		decNumberMultiply(&x, &d, &const_0_01, ctx);
		decNumberAdd(res, &y, &x, ctx);
		break;

	case DATE_DMY:
		decNumberMultiply(&x, &y, &const_0_0001, ctx);
		decNumberAdd(&y, &m, &x, ctx);
		decNumberMultiply(&x, &y, &const_0_01, ctx);
		decNumberAdd(res, &d, &x, ctx);
		break;

	case DATE_MDY:
		decNumberMultiply(&x, &y, &const_0_0001, ctx);
		decNumberAdd(&y, &d, &x, ctx);
		decNumberMultiply(&x, &y, &const_0_01, ctx);
		decNumberAdd(res, &m, &x, ctx);
		break;
	}

	if (bc)
		decNumberMinus(res, res, ctx);
	day_of_week(year, month, day, &DispMsg);
	return res;
}


/* Convert a decimal real to a date.
 * We have to honour the current date mode and make sure that things
 * don't go out of range.
 */
static int extract_date(const decNumber *x, int *year, int *month, int *day, decContext *ctx) {
	int bc, ip, fp, y, m, d;
	decNumber z, a;

	if (is_intmode())
		return 1;

	if (decNumberIsNegative(x)) {
		decNumberMinus(&z, x, ctx);
		bc = 0;
	} else {
		decNumberCopy(&z, x);
		bc = 1;
	}
	decNumberTrunc(&a, &z, ctx);			// a = iii	z = iii.ffrrrr
	ip = dn_to_int(&a, ctx);
	decNumberSubtract(&a, &z, &a, ctx);		// a = .ffrrrr
	decNumberMultiply(&z, &a, &const_100, ctx);	// z = ff.rrrr
	decNumberTrunc(&a, &z, ctx);			// a = ff
	fp = dn_to_int(&a, ctx);
	decNumberSubtract(&z, &z, &a, ctx);		// z = .rrrr
	switch (State.date_mode) {
	default:
	case DATE_YMD:
		y = ip;
		m = fp;
		decNumberMultiply(&a, &z, &const_100, ctx);
		decNumberTrunc(&z, &a, ctx);
		d = dn_to_int(&z, ctx);
		break;

	case DATE_DMY:
		d = ip;
		m = fp;
		goto year;

	case DATE_MDY:
		m = ip;
		d = fp;
year:		decNumberMultiply(&a, &z, &const_10000, ctx);
		decNumberTrunc(&z, &a, ctx);
		y = dn_to_int(&z, ctx);
		break;
	}
	/* Make sense of things
	 */
	if (year != NULL)
		*year = y;
//	else if (y < -4799 || y > 9999)
//		y = 2000;
	if (month != NULL)
		*month = m;
//	else if (m < 1 || m > 12)
//		m = 12;
	if (day != NULL)
		*day = d;
//	else if (d < 1 || d > month_lengths(y, m))
//		d = 1;
	return check_date(y, m, d);
}


/* Given an argument on the stack, attempt to determine the year.
 * If the argument is a plain integer, it is the year.
 * Otherwise, decode it according to the current date mode and
 * return the year.
 */
static int find_year(const decNumber *x, int *year, decContext *ctx) {
	int y;

	if (decNumberIsSpecial(x))
		return -1;
	if (is_int(x, ctx)) {
		y = dn_to_int(x, ctx);
		if (check_date(y, 1, 1))
			return -1;
	} else if (extract_date(x, &y, NULL, NULL, ctx))
		return -1;
	*year = y;
	return 0;
}


#ifdef INCLUDE_EASTER
/* Return Easter dates from the year.
 * We assume the calendar reformation took place in 1752 which means for this
 * year and before, we return the Julian date of Easter Sunday and afterwards
 * the Gregorian version.
 */
static void easter(int year, int *month, int *day) {
	if (year > 1752) {	// Gregorian
		const int a = year % 19;
		const int b = year / 100;
		const int c = year % 100;
		const int d = b / 4;
		const int e = b % 4;
		const int f = (b + 8) / 25;
		const int g = (b - f + 1) / 3;
		const int h = (19 * a + b - d - g + 15) % 30;
		const int i = c / 4;
		const int k = c % 4;
		const int L = (32 + 2 * e + 2 * i - h - k) % 7;
		const int m = (a + 11 * h + 22 * L) / 451;
		*month = (h + L - 7 * m + 114) / 31;		// Jan = 1
		*day = ((h + L - 7 * m + 114) % 31) + 1;
	} else {
		const int a = year % 4;
		const int b = year % 7;
		const int c = year % 19;
		const int d = (19 * c + 15) % 30;
		const int e = (2 * a + 4 * b - d + 34) % 7;
		*month = (d + e + 114) / 31;			// Jan = 1
		*day = ((d + e + 114) % 31) + 1;
	}
}


/* Date of Easter.  Input is the year and output is a date.
 * The input can either be specified as an integer year or as a
 * valid date in the current format.
 */
decNumber *dateEaster(decNumber *res, const decNumber *x, decContext *ctx) {
	int y;

	if (is_intmode())
		set_NaN(res);
	else {
		if (find_year(x, &y, ctx)) {
			set_NaN(res);
			return res;
		} else {
			int m, d;

			easter(y, &m, &d);
			build_date(res, y, m, d, ctx);
		}
	}
	return res;
}
#endif


/* Test if a year is a leap year
 */
void date_isleap(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	int y, t = 0;
	decNumber x;

	if (is_intmode())
		err(ERR_BAD_DATE);
	else {
		getX(&x);
		if (! find_year(&x, &y, Ctx))
			t = isleap(y);
		fin_tst(t);
	}
}


/* Return the day of the week
 */
decNumber *dateDayOfWeek(decNumber *res, const decNumber *x, decContext *ctx) {
	int y, m, d;

	if (decNumberIsSpecial(x) || extract_date(x, &y, &m, &d, ctx))
		set_NaN(res);
	else
		int_to_dn(res, day_of_week(y, m, d, &DispMsg), ctx);
	return res;
}

static void copy3(const char *p) {
	char buf[4];
	int i;

	for (i=0; i<3; i++)
		buf[i] = p[i];
	buf[3] = '\0';
	add_string(buf);
}

void date_alphaday(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	decNumber x;
	int y, m, d, dow;
	if (is_intmode())
		err(ERR_BAD_DATE);
	else {
		getX(&x);
		if (decNumberIsSpecial(&x) || extract_date(&x, &y, &m, &d, Ctx))
			err(ERR_BAD_DATE);
		else {
			dow = day_of_week(y, m, d, NULL);
			copy3("MONTUEWEDTHUFRISATSUN" + 3*(dow-1));
		}
	}
}

void date_alphamonth(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	decNumber x;
	int y, m, d;
	static const char mons[12*3] = "JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";

	if (is_intmode())
		err(ERR_BAD_DATE);
	else {
		getX(&x);
		if (decNumberIsSpecial(&x) || extract_date(&x, &y, &m, &d, Ctx))
			err(ERR_BAD_DATE);
		else {
			copy3(mons + 3*m - 3);
		}
	}
}

/* Add or subtract days from a date.
 * Convert to Julian days, do the addition or subtraction and convert back.
 */
decNumber *dateAdd(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	int j;
	int yr, m, d;

	if (is_intmode()) {
		err(ERR_BAD_DATE);
		return res;
	}
	if (decNumberIsSpecial(x) || decNumberIsSpecial(y) || ! is_int(y, ctx)) {
err:		set_NaN(res);
		return res;
	}
	if (extract_date(x, &yr, &m, &d, ctx))
		goto err;
	j = dn_to_int(y, ctx) + JDN(yr, m, d);
	if (j < 0)
		goto err;
	JDN2(j, &yr, &m, &d);
	return build_date(res, yr, m, d, ctx);
}


/* Days between dates.
 * Convert to Julian days and subtract.
 */
decNumber *dateDelta(decNumber *res, const decNumber *x, const decNumber *y, decContext *ctx) {
	int d, m, yr, j1, j2;

	if (is_intmode())
		err(ERR_BAD_DATE);
	else if (decNumberIsSpecial(x) || decNumberIsSpecial(y))
err:		set_NaN(res);
	else {
		if (extract_date(x, &yr, &m, &d, ctx))
			goto err;
		j1 = JDN(yr, m, d);
		if (extract_date(y, &yr, &m, &d, ctx))
			goto err;
		j2 = JDN(yr, m, d);
		int_to_dn(res, j2-j1, ctx);
	}
	return res;
}


/* Conversion routines from Julian days to and from dates
 */
decNumber *dateToJ(decNumber *res, const decNumber *x, decContext *ctx) {
	if (is_intmode())
		err(ERR_BAD_DATE);
	else if (decNumberIsSpecial(x))
err:		set_NaN(res);
	else {
		int y, m, d;

		if (extract_date(x, &y, &m, &d, ctx))
			goto err;
		int_to_dn(res, JDN(y, m, d), ctx);
	}
	return res;
}

decNumber *dateFromJ(decNumber *res, const decNumber *x, decContext *ctx) {
	if (is_intmode())
		err(ERR_BAD_DATE);
	else if (decNumberIsSpecial(x))
		set_NaN(res);
	else {
		const int j = dn_to_int(x, ctx);
		int y, m, d;

		JDN2(j, &y, &m, &d);
		return build_date(res, y, m, d, ctx);
	}
	return res;
}


/* Date and times to the Alpha register */
void date_alphadate(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	decNumber x;
	int d, m, y;
	char buf[16];
	char *p;

	getX(&x);
	xset(buf, '\0', sizeof(buf));
	if (extract_date(&x, &y, &m, &d, Ctx)) {
		err(ERR_BAD_DATE);
		return;
	}
	switch (State.date_mode) {
	default:
		p = num_arg(buf, y);
		*p++ = '-';
		p = num_arg_0(p, m, 2);
		*p++ = '-';
		num_arg_0(p, d, 2);
		break;

	case DATE_DMY:
		p = num_arg(buf, d);
		*p++ = '.';
		p = num_arg_0(p, m, 2);
		*p++ = '.';
		num_arg_0(p, y, 4);
		break;

	case DATE_MDY:
		p = num_arg(buf, m);
		*p++ = '/';
		p = num_arg_0(p, d, 2);
		*p++ ='/';
		num_arg_0(p, y, 4);
		break;
	}
	add_string(buf);
}

void date_alphatime(decimal64 *nul1, decimal64 *nul2, decContext *ctx64) {
	decNumber x,  y;
	char buf[16], *p;
	int a;
	const char *suffix;

	if (is_intmode())
		err(ERR_BAD_DATE);
	else {
		xset(buf, '\0', sizeof(buf));
		getX(&x);
		decNumberTrunc(&y, &x, Ctx);
		a = dn_to_int(&y, Ctx);
		if (State.t12) {
			if (a >= 12) {
				a -= 12;
				suffix = " PM";
			} else
				suffix = " AM";
			if (a == 0)
				a = 12;
		} else
			suffix = "";
		p = num_arg(buf, a);
		*p++ = ':';
		decNumberFrac(&y, &x, Ctx);
		decNumberMultiply(&x, &y, &const_60, Ctx);
		decNumberTrunc(&y, &x, Ctx);
		p = num_arg_0(p, dn_to_int(&y, Ctx), 2);
		*p++ = ':';
		decNumberFrac(&y, &x, Ctx);
		decNumberMultiply(&x, &y, &const_60, Ctx);
		decNumberRound(&y, &x, Ctx);
		p = num_arg_0(p, dn_to_int(&y, Ctx), 2);
		scopy(p, suffix);
		add_string(buf);
	}
}


/* These routines get the real time and day from the RTC */

static void query_time(unsigned int *s, unsigned int *m, unsigned int *h) {
#ifdef REALBUILD
	unsigned char hour, minute, second;
	
	RTC_GetTime(&hour, &minute, &second);
	*s = second;
	*m = minute;
	*h = hour;
#else
	time_t now = time(NULL);
	struct tm *dt = localtime(&now);

	*h = dt->tm_hour;
	*m = dt->tm_min;
	*s = dt->tm_sec;
#endif
}

static void query_date(unsigned int *d, unsigned int *m, unsigned int *y) {
#ifdef REALBUILD
	unsigned char day, month, dow;
	unsigned short year;
	
	RTC_GetDate( &year, &month, &day, &dow );

	*y = year;
	*m = month;
	*d = day;
#else
	time_t now = time(NULL);
	struct tm *dt = localtime(&now);

	*y = dt->tm_year + 1900;
	*m = dt->tm_mon + 1;
	*d = dt->tm_mday;
#endif
}


void date_date(decimal64 *r, decimal64 *nul, decContext *ctx64) {
	unsigned int d, m, y;
	decNumber z;

	query_date(&d, &m, &y);
	build_date(&z, y, m, d, Ctx);
	decimal64FromNumber(r, &z, ctx64);
}

void date_time(decimal64 *r, decimal64 *nul, decContext *ctx64) {
	unsigned int h, m, s;
	decNumber a, b, c;

	query_time(&s, &m, &h);
	int_to_dn(&a, s, Ctx);
	decNumberDivide(&b, &a, &const_60, Ctx);
	int_to_dn(&a, m, Ctx);
	decNumberAdd(&c, &a, &b, Ctx);
	decNumberDivide(&b, &c, &const_60, Ctx);
	int_to_dn(&a, h, Ctx);
	decNumberAdd(&c, &b, &a, Ctx);
	decimal64FromNumber(r, &c, ctx64);
}

void date_setdate(decimal64 *r, decimal64 *nul, decContext *ctx64) {
	int d, m, y, dow;
	decNumber x;

	getX(&x);
	if (extract_date(&x, &y, &m, &d, Ctx)) {
		err(ERR_BAD_DATE);
		return;
	}
	dow = day_of_week(y, m, d, NULL);
#ifdef REALBUILD
	RTC_SetDate((unsigned short) y, (unsigned char) m,
		    (unsigned char) d, (unsigned char) dow);
#endif
}

void date_settime(decimal64 *r, decimal64 *nul, decContext *ctx64) {
	int s, m, h;
	decNumber x, y;

	getX(&x);
	h = dn_to_int(decNumberTrunc(&y, &x, Ctx), Ctx) & 0x3f;
	decNumberFrac(&y, &x, Ctx);
	decNumberMultiply(&x, &y, &const_100, Ctx);
	m = dn_to_int(decNumberTrunc(&y, &x, Ctx), Ctx) & 0x7f;
	decNumberTrunc(&y, &x, Ctx);
	decNumberMultiply(&x, &y, &const_100, Ctx);
	s = dn_to_int(decNumberRound(&x, &y, Ctx), Ctx) & 0x7f;
#ifdef REALBUILD
	RTC_SetTime((unsigned char) h, (unsigned char) m, (unsigned char) s);
#endif
}

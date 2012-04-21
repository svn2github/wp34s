
/* Solver code from here */
/* Secant iteration */
static void solve_secant(decNumber *s, const decNumber *a, const decNumber *b, const decNumber *fa, const decNumber *fb) {
	decNumber x, y, z;

	dn_subtract(&x, b, a);
	dn_subtract(&y, fb, fa);
	dn_divide(&z, &x, &y);
	dn_multiply(&x, &z, fb);
	dn_subtract(s, b, &x);
}

/* A third of the inverse quadratic interpolation step.
 * Return non-zero is one of the denominators is zero.
 */
static int qstep(decNumber *r, const decNumber *a, const decNumber *fb, const decNumber *fc, const decNumber *fa) {
	decNumber x, y, z;

	dn_subtract(&x, fa, fb);
	if (dn_eq0(&x))
		return -1;
	dn_subtract(&y, fa, fc);
	if (dn_eq0(&y))
		return -1;
	dn_multiply(&z, &x, &y);
	dn_multiply(&x, a, fb);
	dn_multiply(&y, &x, fc);
	dn_divide(r, &y, &z);
	return 0;
}

/* Inverse quadratic interpolation.
 * Return non-zero if interpolation fails due to equal function values
 */
static int solve_quadratic(decNumber *s, const decNumber *a, const decNumber *b, const decNumber *c, const decNumber *fa, const decNumber *fb, const decNumber *fc) {
	decNumber x, y, z;

	if (qstep(&x, a, fb, fc, fa))
		return -1;
	if (qstep(&y, b, fa, fc, fb))
		return -1;
	dn_add(&z, &x, &y);
	qstep(&x, c, fa, fb, fc);
	dn_add(s, &z, &x);
	return 0;
}

#ifdef USE_RIDDERS
/* Ridder's method
 */
static int solve_ridder(decNumber *xnew, const decNumber *x0, const decNumber *x2, const decNumber *x1, const decNumber *y0, const decNumber *y2, const decNumber *y1) {
	decNumber a, b, r, s, t, u, v;

	dn_subtract(&r, y1, y2);
	if (dn_eq0(&r))
		return -1;
	dn_subtract(&s, y0, y1);		// s = y0 - y1
	dn_divide(&a, &s, &r);
	dn_multiply(&t, &a, y1);
	dn_subtract(&r, y0, &t);
	if (dn_eq0(&r))
		return -1;
	dn_divide(&b, &s, &r);
	dn_p1(&r, &b);
	if (dn_eq0(&r))
		return -1;
	dn_m1(&s, &b);
	dn_divide(&u, &s, &r);
	dn_p1(&r, &a);
	if (dn_eq0(&r))
		return -1;
	dn_m1(&s, &a);
	dn_divide(&v, &s, &r);

	decNumberSquare(&r, &v);
	dn_add(&s, &r, &const_3);
	dn_multiply(&t, &s, &v);
	decNumberSquare(&r, &u);
	dn_add(&s, &r, &const_3);
	dn_multiply(&r, &s, &u);
	dn_divide(&s, &r, &t);
	dn_subtract(&r, x1, x0);
	dn_abs(&t, &r);
	dn_multiply(&r, &t, &s);
	dn_add(xnew, &r, x1);
	return 0;
}
#endif


/* perform a bisection step.
 */
#define solve_bisect(s, a, b)	dn_average(s, a, b)


/* Check if the new point is inside the bracketed interval, if not do a bisection
 * step instead.  This means we'll not escape a bracketed interval ever.
 */
static int solve_bracket(decNumber *s, const decNumber *a, const decNumber *b) {
	decNumber x, y;

	dn_subtract(&x, a, b);
	if (decNumberIsNegative(&x)) {		// a<b
		dn_subtract(&y, s, a);
		dn_subtract(&x, b, s);
	} else {				// a>b
		dn_subtract(&y, s, b);
		dn_subtract(&x, a, s);
	}
	/* If out of bracket or the same as a previous, out of bracket */
	return (dn_le0(&y) || dn_le0(&x));
}


/* Limit the distance a new estimate can be to within 100 times the distance
 * between the existing points.
 */
static void limit_jump(decNumber *s, const decNumber *a, const decNumber *b) {
	decNumber x, y;

	dn_subtract(&x, a, b);
	dn_abs(&y, &x);
	dn_mul100(&x, &y);			// 100 |a-b|
	dn_subtract(&y, a, &x);
	if (dn_lt(s, &y)) {
		decNumberCopy(s, &y);
		return;
	}
	dn_add(&y, b, &x);
	if (dn_lt(&y, s))
		decNumberCopy(s, &y);
}

/* Swap two numbers in place
 */
void decNumberSwap(decNumber *a, decNumber *b) {
	decNumber z;

	decNumberCopy(&z, a);
	decNumberCopy(a, b);
	decNumberCopy(b, &z);
}


/* Compare two numbers to see if they are mostly equal.
 * Return non-zero if they are the same.
 */
static int slv_compare(const decNumber *a, const decNumber *b) {
	decNumber ar, br;

	decNumberRnd(&ar, a);
	decNumberRnd(&br, b);
	return dn_eq(&ar, &br);
}

/* Define how with use the flags.
 * The bottom bit is reserved for the xrom code.
 * The second bottom bit indicates if we've bracketed a solution (set if so).
 *
 * If we've bracketed, the upper bits are an iteration counter.
 * If not, we encode more state:
 */

#define _FLAG_BRACKET	(1)
#define _FLAG_CONST	(2)
#ifdef USE_RIDDERS
#define _FLAG_BISECT	(4)
#endif
#define SLV_COUNT(f)		((f) >> 8)
#define SLV_SET_COUNT(f, c)	(((f) & 0xff) | ((c) << 8))

#define IS_BRACKET(f)		((f) & _FLAG_BRACKET)
#define SET_BRACKET(f)		(f) |= _FLAG_BRACKET
#define CLEAR_BRACKET(f)	(f) &= ~_FLAG_BRACKET
#define BRACKET_MAXCOUNT	150

#define IS_CONST(f)		((f) & _FLAG_CONST)
#define SET_CONST(f)		(f) |= _FLAG_CONST
#define CLEAR_CONST(f)		(f) &= ~_FLAG_CONST
#define CONST_MAXCOUNT		20

#define IS_ONESIDE(f)		(((f) & (_FLAG_BRACKET + _FLAG_CONST)) == 0)
#define ONESIDE_MAXCOUNT	100

#ifdef USE_RIDDERS
#define IS_BISECT(f)		((f) & _FLAG_BISECT)
#define CLEAR_BISECT(f)		(f) &= ~_FLAG_BISECT
#define SET_BISECT(f)		(f) |= _FLAG_BISECT
#endif

static int solver_step(decNumber *a, decNumber *b, decNumber *c,
				decNumber *fa, decNumber *fb, const decNumber *fc,
				unsigned int *statep) {
	decNumber q, x, y, z;
	int s1, s2;
	unsigned int slv_state = *statep;
	int count, r;
#ifdef USE_RIDDERS
	const int was_bisect = IS_BISECT(slv_state);
	CLEAR_BISECT(slv_state);
#endif
	if (IS_BRACKET(slv_state)) {
		count = SLV_COUNT(slv_state);
		if (count >= BRACKET_MAXCOUNT)
			goto failed;
		slv_state = SLV_SET_COUNT(slv_state, count+1);
brcket:
#ifdef USE_RIDDERS

		if (was_bisect)
			r = solve_ridder(&q, a, b, c, fa, fb, fc);
		else
#endif
			r = solve_quadratic(&q, a, b, c, fa, fb, fc);
		s1 = decNumberIsNegative(fc);
		s2 = decNumberIsNegative(fb);
		if (s1 != s2) {
			decNumberCopy(a, c);
			decNumberCopy(fa, fc);
			dn_multiply(&y, b, &const_3);
		} else {
			decNumberCopy(b, c);
			decNumberCopy(fb, fc);
			dn_multiply(&y, a, &const_3);
		}
		dn_add(&z, &y, c);
		dn_multiply(&y, &z, &const_0_25);
		if (r)
			solve_secant(&q, a, b, fa, fb);
		if (solve_bracket(&q, &y, c)) {
			solve_bisect(&q, a, b);
#ifdef USE_RIDDERS
			SET_BISECT(slv_state);
#endif
		}
	} else {
		s1 = decNumberIsNegative(fc);
		s2 = decNumberIsNegative(fb);
		if (s1 != s2) {
			SET_BRACKET(slv_state);
			CLEAR_CONST(slv_state);
			r = -1;
			goto brcket;
		}
		if (IS_CONST(slv_state)) {
			count = SLV_COUNT(slv_state);
			if (count >= CONST_MAXCOUNT)
				goto failed;
			if (! dn_eq(fb, fc)) {
				slv_state = SLV_SET_COUNT(slv_state, 0);
				CLEAR_CONST(slv_state);
				r = -1;
				goto nonconst;
			}
			slv_state = SLV_SET_COUNT(slv_state, count+1);
			if (count & 1) {
				decNumberCopy(b, c);
				decNumberCopy(fb, fc);
				if (decNumberIsNegative(a))
					dn_mul2(&x, a);
				else	dn_div2(&x, a);
				dn_subtract(&q, &x, &const_10);
			} else {
				decNumberCopy(a, c);
				decNumberCopy(fa, fc);
				if (decNumberIsNegative(b))
					dn_div2(&x, b);
				else	dn_mul2(&x, b);
				dn_add(&q, &x, &const_10);
			}
		} else {
			count = SLV_COUNT(slv_state);
			if (count >= ONESIDE_MAXCOUNT)
				goto failed;
			slv_state = SLV_SET_COUNT(slv_state, count+1);
nonconst:
			r = solve_quadratic(&q, a, b, c, fa, fb, fc);
			//MORE: need to check if the new point is worse than the old.
			if (dn_abs_lt(fa, dn_abs(&y, fb))) {
				decNumberCopy(a, c);
				decNumberCopy(fa, fc);
			} else {
				decNumberCopy(b, c);
				decNumberCopy(fb, fc);
			}
			if (dn_lt(b, a)) {
				decNumberSwap(a, b);
				decNumberSwap(fa, fb);
			}
			if (r)
				solve_secant(&q, a, b, fa, fb);
			limit_jump(&q, a, b);
		}
	}
	*statep = slv_state;
	if (slv_compare(c, &q)) return 1;
	if (slv_compare(a, &q)) return 1;
	if (slv_compare(b, &q)) return 1;
	decNumberCopy(c, &q);
	return 0;
failed:
	*statep = slv_state;
	return -1;
}

// User code flag numbers
#define _FLAG_BRACKET_N	8
#define _FLAG_CONST_N	9
#ifdef USE_RIDDERS
#define _FLAG_BISECT_N	10
#endif
#define _FLAG_COUNT_N	0	/* 0 - 7, eight flags in all */

// User code interface to the solver
void solver(enum nilop op) {
	decNumber a, b, c, fa, fb, fc;
	unsigned int flags;
	int r;

	getRegister(&a, LOCAL_REG_BASE + 0);
	getRegister(&b, LOCAL_REG_BASE + 1);
	getRegister(&fa, LOCAL_REG_BASE + 3);
	getRegister(&fb, LOCAL_REG_BASE + 4);

	getRegister(&c, LOCAL_REG_BASE + 2);
	flags = 0;
	for (r=0; r<8; r++)
		if (get_user_flag(LOCAL_FLAG_BASE + r + _FLAG_COUNT_N))
			flags |= 1<<r;
	flags = SLV_SET_COUNT(0, flags);

	if (get_user_flag(LOCAL_FLAG_BASE + _FLAG_BRACKET_N))
		SET_BRACKET(flags);
	if (get_user_flag(LOCAL_FLAG_BASE + _FLAG_CONST_N))
		SET_CONST(flags);
#ifdef USE_RIDDERS
	if (get_user_flag(NUMFLG + _FLAG_BISECT_N))
		SET_BISECT(flags);
#endif

	getX(&fc);
	int_to_dn(&fc, solver_step(&a, &b, &c, &fa, &fb, &fc, &flags));
	setX(&fc);

	setRegister(LOCAL_REG_BASE + 0, &a);
	setRegister(LOCAL_REG_BASE + 1, &b);
	setRegister(LOCAL_REG_BASE + 2, &c);
	setRegister(LOCAL_REG_BASE + 3, &fa);
	setRegister(LOCAL_REG_BASE + 4, &fb);

	put_user_flag(LOCAL_FLAG_BASE + _FLAG_BRACKET_N, IS_BRACKET(flags));
	put_user_flag(LOCAL_FLAG_BASE + _FLAG_CONST_N, IS_CONST(flags));
#ifdef USE_RIDDERS
	put_user_flag(LOCAL_FLAG_BASE + _FLAG_BISECT_N, IS_BISECT(flags));
#endif
	flags = SLV_COUNT(flags);
	for (r=0; r<8; r++)
		put_user_flag(LOCAL_FLAG_BASE + r + _FLAG_COUNT_N, flags & (1<<r));
}


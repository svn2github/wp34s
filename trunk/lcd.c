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


#include "lcd.h"
#include "display.h"
#include "xeq.h"

#if defined(REALBUILD)
#include "hp.h"
#endif

#ifdef USECURSES
static unsigned char dots[400];


static void dispreg(const char n, decimal64 *p) {
	char buf[32];
	if (is_intmode())
		sprintf(buf, "%llx", (unsigned long long int)d64toInt(p));
	else
		decimal64ToString(p, buf);
	PRINTF("%c: %s", n, buf);
}


/* Some wrapper routines to set segments of the display */
void set_dot(int n) {
	dots[n] = 1;
}
void clr_dot(int n) {
	dots[n] = 0;
}
#else

static const unsigned char lcd_addr[] = {
#define M(p, a, b)	/* [p] = */ a/4
#include "lcdmap.h"
#undef M
};

static const unsigned char lcd_bit[] = {
#define M(p, a, b)	/* [p] = */ b
#include "lcdmap.h"
#undef M
};

void set_dot(int n) {
	unsigned int m;
	
	if ( n >= 0 && n <= 141 ) {
		m = 1 << lcd_bit[n];
		AT91C_SLCDC_MEM[lcd_addr[n]] |= m;
	}
#ifdef DEBUG
	else {
		*((char *)NULL)=0;
	}
#endif
}

void clr_dot(int n) {
	unsigned int m;
	if ( n >= 0 && n <= 141 ) {
		m = 1 << lcd_bit[n];
		AT91C_SLCDC_MEM[lcd_addr[n]] &= ~m;
	}
#ifdef DEBUG
	else {
		*((char *)NULL)=0;
	}
#endif
}

void set_status_grob(unsigned long long int grob[6]) {
	volatile unsigned long long int *p = (volatile unsigned long long int *)AT91C_SLCDC_MEM;
	int i, j;

	//6*33 horizontal matrix transfert
	p[6] = (p[6]&~0x7fffffffc0LL) | ((grob[0]<<6) & 0x7fffffffc0LL);
	p[7] = (p[7]&~0x7fffffffc0LL) | ((grob[1]<<6) & 0x7fffffffc0LL);
	p[8] = (p[8]&~0x7fffffffc0LL) | ((grob[2]<<6) & 0x7fffffffc0LL);
	p[9] = (p[9]&~0x7fffffffc0LL) | ((grob[3]<<6) & 0x7fffffffc0LL);
	p[0] = (p[0]&~0x7fffffffc0LL) | ((grob[4]<<6) & 0x7fffffffc0LL);
	p[1] = (p[1]&~0x7fffffffc0LL) | ((grob[5]<<6) & 0x7fffffffc0LL);

	//now the verticals...
	for (j=9; j>=0; j--) { 
		unsigned int c = 0;
		const unsigned int t = (j + 2) % 10;

		for (i=0; i<6; i++) {
			c <<= 1;
			if ((grob[i] & (1LL << (j+33)))!=0)
				c++;
		}
		p[t] = (p[t] & ~0x3fLL) | (unsigned long long)c;
	}
}
#endif

int setuptty(int reset) {
#if !defined(REALBUILD) && !defined(WINGUI)
#ifdef USECURSES
	if (reset)
		endwin();
	else {
		initscr();
		cbreak();
		noecho();
		//keypad(stdscr, TRUE);
	}
#else
#ifdef USETERMIO
	static struct termio old;
	struct termio new;

	if (reset) {
		ioctl(0, TCSETA, &old);
	} else  {
		if (ioctl(0, TCGETA, &old) == -1) {
			printf("cannot grab tty modes\n");
			return 1;
		}
		new = old;
		new.c_lflag &= ~(ICANON | ECHO);
		if (ioctl(0, TCSETA, &new) == -1)
			printf("unable to set terminal modes\n");
	}
#else
	static struct termios old;
	struct termios new;

	if (reset) {
		tcsetattr(0, TCSAFLUSH, &old);
	} else {
		if (tcgetattr(0, &old) == -1){
			printf("cannot grab tty modes\n");
			return 1;
		}
		new = old;
		//cfmakeraw(&new);
		new.c_lflag &= ~(ECHO | ICANON | ISIG);
		tcsetattr(0, TCSAFLUSH, &new);
	}
#endif
#endif
#endif
	return 0;
}


void reset_disp(void) {
	int i;

	clr_dot(STO_annun);
	clr_dot(RCL_annun);
	for (i=0; i<=EXP_SIGN; i++)
		clr_dot(i);

#ifdef USECURSES
	erase();
	MOVE(0, 4);
#elif !defined(REALBUILD) && !defined(WINGUI)
	putchar('\r');
	for (i=0; i<70; i++)
		putchar(' ');
	putchar('\r');
	putchar(' ');
#endif
}

void show_disp(void) {
#ifdef USECURSES
	int i, j, p, x;
	const int dig_base = 16;

	/* Segments 0 - 107 are the main digits */
	for (i=0; i<DISPLAY_DIGITS; i++) {
		p = i*SEGS_PER_DIGIT;
		x = 3+5*i;
		if (dots[p]) {
			MOVE(x+1, dig_base);	PRINTF("--");
		}
		if (dots[p+1]) {
			MOVE(x, dig_base+1);	PRINTF("|");
			MOVE(x, dig_base+2);	PRINTF("|");
		}
		if (dots[p+3]) {
			MOVE(x+3, dig_base+1);	PRINTF("|");
			MOVE(x+3, dig_base+2);	PRINTF("|");
		}
		if (dots[p+2]) {
			MOVE(x+1, dig_base+3);	PRINTF("--");
		}
		if (dots[p+4]) {
			MOVE(x, dig_base+4);	PRINTF("|");
			MOVE(x, dig_base+5);	PRINTF("|");
		}
		if (dots[p+6]) {
			MOVE(x+3, dig_base+4);	PRINTF("|");
			MOVE(x+3, dig_base+5);	PRINTF("|");
		}
		if (dots[p+5]) {
			MOVE(x+1, dig_base+6);	PRINTF("--");
		}
		if (dots[p+7]) {
			MOVE(x+4, dig_base+6);	PRINTF(".");
		}
		if (dots[p+8]) {
			MOVE(x+3, dig_base+7);	PRINTF("/");
		}
	}
	/* Segments 108 - 128 are the exponent digits */
	for (i=0; i<3; i++) {
		p = i*7+108;
		x = 66 + i * 4;
		if (dots[p]) {
			MOVE(x+1, dig_base-1);	PRINTF("-");
		}
		if (dots[p+1]) {
			MOVE(x, dig_base);	PRINTF("|");
		}
		if (dots[p+3]) {
			MOVE(x+2, dig_base);	PRINTF("|");
		}
		if (dots[p+2]) {
			MOVE(x+1, dig_base+1);	PRINTF("-");
		}
		if (dots[p+4]) {
			MOVE(x, dig_base+2);	PRINTF("|");
		}
		if (dots[p+6]) {
			MOVE(x+2, dig_base+2);	PRINTF("|");
		}
		if (dots[p+5]) {
			MOVE(x+1, dig_base+3);	PRINTF("-");
		}
	}
	/* Segments 129 & 130 are the signs */
	if (dots[MANT_SIGN]) {
		MOVE(0, dig_base+3);
		PRINTF("--");
	}
	if (dots[EXP_SIGN]) {
		MOVE(64, dig_base+1);
		PRINTF("-");
	}
	if (dots[BIG_EQ]) {
		MOVE(47, 12);	PRINTF("==");
	}
	if (dots[LIT_EQ]) {
		MOVE(64, 10);	PRINTF("=");
	}
	if (dots[DOWN_ARR]) {
		MOVE(52, 10);	PRINTF("v");
	}
	if (dots[INPUT]) {
		MOVE(55, 10);	PRINTF("INPUT");
	}
	if (dots[BATTERY]) {
		MOVE(70, 10);	PRINTF("####-");
	}
	if (dots[BEG]) {
		MOVE(52, 12);	PRINTF("BEG");
	}
	if (dots[STO_annun]) {
		MOVE(62, 12);	PRINTF("STO");
	}
	if (dots[RCL_annun]) {
		MOVE(72, 12);	PRINTF("RCL");
	}
	if (dots[RAD]) {
		MOVE(52, 14);	PRINTF("RAD");
	}
	if (dots[DEG]) {
		MOVE(62, 14);	PRINTF("360");
	}
	if (dots[RPN]) {
		MOVE(72, 14);	PRINTF("RPN");
	}
	/* The graphical bit last */
	for (i=0; i<BITMAP_WIDTH; i++)
		for (j=0; j<6; j++) {
			if (dots[i*6+j+MATRIX_BASE]) {
				MOVE(1+i, 9+j);
				PRINTF("#");
			}
		}
#endif
}

void show_stack(void) {
#ifdef USECURSES
	int i;

	if (!State.flags)
		return;

	// Stack display smashes the stack registers
	for (i=4; i<STACK_SIZE; i++) {
		MOVE(26, 8-i);
		PRINTF("%c ", i<stack_size()?'*':' ');
		dispreg(REGNAMES[i], &Regs[regX_idx+i]);
	}
	MOVE(53, 2);	dispreg(REGNAMES[regJ_idx-regX_idx], &regJ);
	MOVE(53, 1);	dispreg(REGNAMES[regK_idx-regX_idx], &regK);
	for (i=0; i<4; i++) {
		MOVE(0, 4-i);
		dispreg(REGNAMES[i], &Regs[regX_idx+i]);
	}
	MOVE(53, 4);
	dispreg(REGNAMES[regL_idx-regX_idx], &regL);
	MOVE(53, 3);
	dispreg(REGNAMES[regI_idx-regX_idx], &regI);
	MOVE(53, 0);
	PRINTF("stack depth: %d", stack_size());
#endif
}

void show_flags(void) {
#if !defined(REALBUILD) && !defined(WINGUI)
	if (!State.flags)
		return;
	MOVE(0, 0);
	PRINTF("   ");
	MOVE(10, 0);
	switch (cur_shift()) {
	case SHIFT_F:	PRINTF("[f-shift]");	break;
	case SHIFT_G:	PRINTF("[g-shift]");	break;
	case SHIFT_H:	PRINTF("[h-shift]");	break;
	default:				break;
	}
	if (State.hyp) {
		MOVE(40, 0);
		if (State.dot)
			PRINTF("[hyp]");
		else
			PRINTF("[hyp-1]");
	}
	if (State.cmplx) {
		MOVE(20, 0);
		PRINTF("[cmplx]");
	}
	if (!State.runmode) {
		MOVE(30, 0);
		PRINTF("[prog]");
	}
	MOVE(0, 0);

#ifdef USECURSES
#define FLAG_BASE	5
//	if (State.flags) {
		MOVE(10, FLAG_BASE);
		if (State.rarg)
			PRINTF("[rcmd]");
		else if (State.arrow)
			PRINTF("[arr]");
		if (State.dot) {
			MOVE(18, FLAG_BASE);
			PRINTF("[dot]");
		}
		if (State.ind) {
			MOVE(24, FLAG_BASE);
			PRINTF("[ind]");
		}
		if (State.trace) {
			MOVE(30, FLAG_BASE);
			PRINTF("[trace]");
		}
		if (State.cmplx) {
			MOVE(40, FLAG_BASE);
			PRINTF("[cmplx]");
		}
		if (State.catalogue) {
			MOVE(50, FLAG_BASE);
			PRINTF("[cat %03u]", State.catalogue);
		}
		if (State.hms) {
			MOVE(64, FLAG_BASE);
			PRINTF("[H.MS]");
		}
		if (State.fract) {
			MOVE(71, FLAG_BASE);
			PRINTF("[FRACT]");
		}
		if (State.multi) {
			MOVE(71, FLAG_BASE+1);
			PRINTF("[MULTI]");
		}
		MOVE(50, FLAG_BASE+1);
		PRINTF("[lp %03u]", State.last_prog);
  		if (State.state_lift) {
  			MOVE(10, FLAG_BASE+1);
  			PRINTF("[lift]");
  		}
  		if (State.state_running) {
  			MOVE(18, FLAG_BASE+1);
  			PRINTF("[running]");
  		}
		MOVE(70, 5);
		PRINTF("iw = %u/%u", State.int_window, State.int_maxw);
		MOVE(30, FLAG_BASE+1);
		PRINTF("shft = %u", cur_shift());
		MOVE(40, FLAG_BASE+1);
		PRINTF("trig = %u", State.trigmode);
//		MOVE(60, FLAG_BASE+1);
//		PRINTF("apos = %u", State.alpha_pos);
		MOVE(10, FLAG_BASE+2);
		PRINTF("numdig = %u   alpha '%-31s'   bflags = %03o-%03o",
				State.numdigit, Alpha, get_bank_flags() >> 8,
				get_bank_flags() & 0xff);
		MOVE(10, FLAG_BASE+3);
		PRINTF("digval = %u", State.digval);
		MOVE(23, FLAG_BASE+3);
		PRINTF("pc = %03u", state_pc());
		MOVE(45, FLAG_BASE+3);
		PRINTF("cmddot = %u  cmdeex = %u  eol = %u",
				State.cmdlinedot, State.cmdlineeex, State.eol);
#if 0
		MOVE(0, 30);
		PRINTF("RetStk = %u", State.retstk_ptr);
		int i;
		for (i=0; i<RET_STACK_SIZE; i++) {
			MOVE(4, 31+i);
			PRINTF("[%d] = %u", i, RetStk[i]);
		}
#endif
//	}
#endif
#endif
}

void finish_display(void) {
#ifdef USECURSES
	show_disp();
	MOVE(0, 0);
	refresh();
#elif defined(WINGUI)
	void EXPORT UpdateDlgScreen(int force);
	UpdateDlgScreen(1);
#else
	putchar('\r');
#endif
}

#if !defined(REALBUILD) && !defined(WINGUI)
/* Take a string and cleanse all non-printing characters from it.
 * Replace them with the usual [xxx] sequences.
 */
static char *cleanse(const char *s) {
	static char res[50];
	char *p;

	for (p=res; *s != '\0'; s++) {
		unsigned char c = 0xff & *s;
		const char *m = pretty(c);
		if (m == NULL) {
			*p++ = c;
		} else {
			*p++ = '[';
			p = scopy_char(p, m, ']');
		}
	}
	*p = '\0';
	return res;
}
#endif

void show_progtrace(char *buf) {
#if !defined(REALBUILD) && !defined(WINGUI)
	int pc = state_pc();

#ifdef USECURSES
	int i;

	for (i=4; i>0 && pc >= 0; i--) {
		MOVE(0, i);
		if (pc) {
			opcode op = getprog(pc);
			PRINTF("%03d %08x: %s", pc, op, cleanse(prt(op, buf)));
		} else
			PRINTF("000:");
		pc = dec(pc);
	}
#else
	if (pc) {
		opcode op = getprog(pc);
		PRINTF("%03u %08x: %s", pc, op, cleanse(prt(op, buf)));
	} else	PRINTF("000:");
#endif
#endif
}

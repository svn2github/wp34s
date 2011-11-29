
/* This project runs an accuracy test for the DEC lib against
 * another. in this case MAPM, the arbitrart precision math lib in C.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dec.h"
#include "m_apm.h"
#include "gamma.h"

// need these
struct _ram PersistentRam;
TStateWhileOn StateWhileOn;
unsigned long long int instruction_count;
int view_instruction_counter;

extern "C" {
	const char *pretty(unsigned char z) { return 0; }
	void shutdown( void ) { exit( 0 ); }
	int is_key_pressed(void) { return 0; }
	#ifndef watchdog
	void watchdog(void)  {}
	#endif
	const char *get_revision( void ) { return "test"; }
	int is_real_key_pressed(void) { return 0; }
	int get_key(void) {return 0; }
	int put_key( int k ) {return k;}
	int open_port( int baud, int bits, int parity, int stopbits ) {return 0; }
	void close_port( void ) { return; }
	void put_byte( unsigned char byte ) { return; }
	void flush_comm( void ) { return; }
	enum shifts shift_down(void) { return SHIFT_N; }
}
// we have one context
static decContext Ctx;

typedef Dec Bf(const Dec&);
typedef MAPM Mf(const MAPM&);

static const char* toString(const MAPM& m)
{
    static char mBuf[300];   
    m.toString(mBuf,30);
    return mBuf;
}

static void print(const MAPM& m)
{
    printf("%s",toString(m));
}

static void print(const Dec& b)
{
    printf("%s", b.asString());
}


void _runTest(Dec ba, Dec bb, MAPM ma, MAPM mb,
              Bf* bf, Mf* mf, int n = 1000)
{
    Dec bd = (bb - ba)/n;
    MAPM md = (mb - ma)/n;

    MAPM emax = 0;
    MAPM epos;
    MAPM epmval;
    Dec  epbval;
    while (ma <= mb)
    {
        print(ba); printf("\t");

        Dec bv = (*bf)(ba);
        MAPM mv = (*mf)(ma);

        const char* bs = bv.asString();
        //const char* ms = toString(mv);

        MAPM mbv(bs);

        MAPM e;
        if (mv != 0)
            e = fabs(mv - mbv)/fabs(mv);
        else
            e = fabs(mv - mbv);

        if (e > emax)
        {
            emax = e;
            epos = ma;
            epmval = mv;
            epbval = bv;
        }

        print(e);
        printf("\n");

        ba += bd;
        ma += md;
    }
    printf("max error = "); print(emax); printf(" at "); print(epos); printf("\n");
    printf("Dec val  = "); print(epbval); printf("\n");
    printf("MAPM val = "); print(epmval); printf("\n");
}

void runTest(int a, int b, Bf* bf, Mf* mf, int n = 1000)
{
    Dec ba(a), bb(b);
    MAPM ma(a), mb(b);

    ba /= 100; bb /= 100;
    ma /= 100; mb /= 100;

    _runTest(ba, bb, ma, mb, bf, mf, n);
}

static void sqrtTest(int mx)
{
    Bf* bf = sqrt;
    Mf* mf = sqrt;
    runTest(0, mx, bf, mf, 10000);
}

static void logTest()
{
    Bf* bf = ln;
    Mf* mf = log;
    runTest(10, 500, bf, mf);
}

static void logTestNear1()
{
    Bf* bf = ln;
    Mf* mf = log;
    runTest(99, 101, bf, mf);
}

static void logTestVeryNear1()
{
    Bf* bf = ln;
    Mf* mf = log;

    Dec ba = Dec(1)/1000000;
    MAPM ma = MAPM(1)/1000000;
    _runTest(1-ba, 1+ba, 1-ma, 1+ma, bf, mf, 1001);
}

static void expTest()
{
    Bf* bf = exp;
    Mf* mf = exp;
    runTest(0, 101, bf, mf);
}


static void sinTest()
{
    Bf* bf = sin;
    Mf* mf = sin;

    Dec ba = atan(Dec(1))*4;
    MAPM ma = atan(MAPM(1))*4;
    _runTest(-ba, ba, -ma, ma, bf, mf, 1001);
}

static void cosTest()
{
    Bf* bf = cos;
    Mf* mf = cos;
    runTest(-20, 20, bf, mf);
}


static void tanTest()
{
    Bf* bf = cos;
    Mf* mf = cos;

    Dec ba = atan(Dec(1))*4;
    MAPM ma = atan(MAPM(1))*4;
    _runTest(-ba, ba, -ma, ma, bf, mf, 1001);
}

static void asinTest()
{
    Bf* bf = asin;
    Mf* mf = asin;
    runTest(-100, 100, bf, mf);
}

static void acosTest()
{
    Bf* bf = acos;
    Mf* mf = acos;
    runTest(-100, 100, bf, mf);
}

static void atanTest()
{
    Bf* bf = atan;
    Mf* mf = atan;
    runTest(-10000, 10000, bf, mf);
}

static void sinhTest()
{
    Bf* bf = sinh;
    Mf* mf = sinh;

    Dec ba = Dec(1)/1000000;
    MAPM ma = MAPM(1)/1000000;
    _runTest(0, ba, 0, ma, bf, mf, 1001);
}

static void gammaTest()
{
    Bf* bf = factorial;
    Mf* mf = gammaFactorialSlow<MAPM>;
    runTest(1, 6900, bf, mf);
}


int main()
{

    // initialise the dec context
    xeq_init_contexts();
    
    // be sure to be in radians
    UState.trigmode = TRIG_RAD;
    
    // initialise MAPM lib to 100
    m_apm_cpp_precision(100);         /* default is 30 */

    // begin tests. uncomment as necessary
    // more need adding..

    //logTest();
    //logTestNear1();
    //logTestVeryNear1();
    //sinTest();
    //cosTest();
    //tanTest();
    //expTest();
    //asinTest();
    //acosTest();
    //atanTest();
    //sinhTest();
    gammaTest();
 
    return 0;
}



/* This project runs an accuracy test for the DEC lib against
 * another. in this case MAPM, the arbitrart precision math lib in C.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dec.h"
#include "m_apm.h"

// need this
struct _ram PersistentRam;
const char *pretty(unsigned char z) 
{
    return NULL;
}
void shutdown( void )
{
    exit( 0 );
}
int is_key_pressed(void) 
{
    return 0;
}

void watchdog(void)  {}

const char *get_revision( void )
{
    return "test";
}

static decContext* Ctx;

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

void runTest(int a, int b, Bf* bf, Mf* mf, int n = 1000)
{
    Dec ba(a), bb(b);
    MAPM ma(a), mb(b);

    ba /= 100; bb /= 100;
    ma /= 100; mb /= 100;

    Dec bd = (bb - ba)/n;
    MAPM md = (mb - ma)/n;

    MAPM emax = 0;
    MAPM epos;
    MAPM epmval;
    Dec  epbval;
    while (ma < mb)
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
    printf("Dec val = "); print(epbval); printf("\n");
    printf("MAPM val = "); print(epmval); printf("\n");
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
    runTest(90, 110, bf, mf);
}

static void expTest()
{
    Bf* bf = exp;
    Mf* mf = exp;
    runTest(0, 10000, bf, mf);
}


static void sinTest()
{
    Bf* bf = sin;
    Mf* mf = sin;
    runTest(-20, 20, bf, mf);
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
    runTest(-3, 3, bf, mf);
}


int main()
{
    decContext ctx;
    Ctx = &ctx;

    // initialise the dec context
    decContextDefault(Ctx, DEC_INIT_BASE);
    Ctx->traps = 0;
    Ctx->digits = DECNUMDIGITS;
    Ctx->emax=DEC_MAX_MATH;
    Ctx->emin=-DEC_MAX_MATH;
    Ctx->round = DEC_ROUND_HALF_EVEN;
    
    // be sure to be in radians
    State.trigmode = TRIG_RAD;
    
    // initialise MAPM lib to 100
    m_apm_cpp_precision(100);         /* default is 30 */

    // begin tests. uncomment as necessary
    // more need adding..

    //logTest();
    //logTestNear1();
    //sinTest();
    //cosTest();
    //tanTest();
    expTest();
 
    return 0;
}


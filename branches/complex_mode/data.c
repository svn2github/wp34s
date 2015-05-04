#if defined(IOS)

#include "xeq.h"
#include "display.h"
#include "data.h"
#include "lcd.h"

// These methods used to be in the iOS App code but due to a compiler bug,
// they are better here to prevent over-agressive optimization of access to catpos for instance

void set_catpos(int catpos)
{
	State.catpos = catpos;
}

int get_catpos()
{
	return State.catpos;
}

char* get_last_displayed()
{
	return LastDisplayedText;
}

char* get_last_displayed_number()
{
	return LastDisplayedNumber;
}

char* get_last_displayed_exponent()
{
	return LastDisplayedExponent;
}

int is_small_font(char *p)
{
	return  State2.disp_small || pixel_length(p, 0) > BITMAP_WIDTH+1;
}

char* get_register_names()
{
	return REGNAMES;
}

int get_first_register_index()
{
	return regX_idx;
}

int get_maxnumregs()
{
	return NUMREG;
}

int get_numregs()
{
	return NumRegs+STACK_SIZE+EXTRA_REG;
}

int is_runmode()
{
	return State2.runmode;
}

int is_catalogue_mode()
{
	return State2.catalogue!=CATALOGUE_NONE;
}

extern unsigned int catalog_count()
{
    return CATALOGUE_STATUS+1;
}

unsigned char current_catalogue_index()
{
    return State2.catalogue;
}

int is_complex_mode()
{
	return State2.cmplx;
}

char get_complex_prefix()
{
	return COMPLEX_PREFIX;
}

#endif

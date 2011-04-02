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
 * This is the main module for the real hardware
 * Module written by MvC
 */
#include "xeq.h"

#include "atmel/board.h"
#include "atmel/pit.h"
#include "atmel/supc.h"

#define BACKUP_SRAM  __attribute__((section(".backup")))
#define RAM_FUNCTION __attribute__((section(".ramfunc")))

// Heartbeat frequency in ms
#define PIT_PERIOD 50 
#define HEARTBEAT_SHIFT 1   // factor 2 compared to 100ms user heartbeat
#define HEARTBEAT_MASK ~(0xffffffff<<HEARTBEAT_SHIFT) 

/*
 *  CPU speed settings
 */
#define SPEED_NULL   0
#define SPEED_SLOW   1
#define SPEED_MEDIUM 2
#define SPEED_HIGH   3

/// PLL frequency range.
#define CKGR_PLL          AT91C_CKGR_OUT_2
/// PLL startup time (in number of slow clock ticks).
#define PLLCOUNT          AT91C_CKGR_PLLCOUNT
/// PLL MUL value.
#define PLLMUL            999
/// PLL DIV value.
#define PLLDIV            1
// Helper
#define wait_for_clock() while ( ( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) \
				 != AT91C_PMC_MCKRDY )

/*
 *  setup the perstent RAM
 */
TPersistentRam PersistentRam;

/*
 *  Local data
 */
unsigned int ClockSpeed;
unsigned int Heartbeats;
int IdleMode;

/*
 *  Go to idle mode
 *  Runs from SRAM to be able to turn of flash
 */
RAM_FUNCTION void go_idle( void )
{
	IdleMode = 1;

	/*
	 *  Disable flash memory in order to save power
	 */
	SUPC_DisableFlash();

	/*
	 *  Disable the processor clock and go to sleep
	 */
	AT91C_BASE_PMC->PMC_SCDR = AT91C_PMC_PCK;   
	while ( ( AT91C_BASE_PMC->PMC_SCSR & AT91C_PMC_PCK ) != AT91C_PMC_PCK ); 
}


/*
 *  Scan the keyboard
 */
void scan_keyboard( void )
{
}


/*
 *  Set clock speed to one of 4 fixed values:
 *  0 (idle), 32.768 kHz, 2 MHz, 32.768 MHz
 */
void set_speed( int speed )
{
	switch ( speed ) {

	case SPEED_NULL:
		/*
		 *  Turn off the processor clock
		 */
		// fall through

	case SPEED_SLOW:
		/*
		 *  32.768 KHz oscillator
		 */
		AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_CSS_SLOW_CLK;
		wait_for_clock();
		// No wait states for flash read
		AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_0FWS;
		// Turn off unneccesary oscilators

		ClockSpeed = 32768;
		break;

	case SPEED_MEDIUM:
		/*
		 *  2 MHz interal RC clock
		 */
		AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_CSS_MAIN_CLK;
		wait_for_clock();
		// No wait states for flash read
		AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_0FWS;
		ClockSpeed = 2000000;
		break;

	case SPEED_HIGH:
		/*
		 *  32.768 MHz PLL, derived from 32 KHz slow clock
		 */
		// Maximum wait states for flash reads
		AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_3FWS;

		if ( ( AT91C_BASE_PMC->PMC_MCKR & AT91C_PMC_CSS ) 
			== AT91C_PMC_CSS_PLL_CLK ) 
		{
			// Intermediate switch to slow clock
			AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_CSS_SLOW_CLK;
			wait_for_clock();
		}
		// Initialize PLL at 32MHz
		AT91C_BASE_PMC->PMC_PLLR = CKGR_PLL | PLLCOUNT \
					 | ( PLLMUL << 16 ) | PLLDIV;
		while ( ( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK ) == 0 );

		// Switch to PLL
		AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_CSS_PLL_CLK;
		wait_for_clock();
		ClockSpeed = 32768 * ( 1 + PLLMUL );
		break;
	}

	/*
	 *  Now we have to reprogram the PIT
	 */
	PIT_SetPIV( ( ( PIT_PERIOD * ClockSpeed ) / 1000 + 8 ) >> 4 );

	/*
	 *  Go idle if requested
	 */
	if ( speed == SPEED_NULL ) {
		go_idle();
	}
}


/*
 *  The 100ms heartbeat, called from the PIT interrupt
 */
void user_heartbeat( void )
{
	++Ticker;
	if ( State.pause ) {
	        --State.pause;
	}
	if ( ++Keyticks > 1000 ) {
		Keyticks = 1000;
	}
        // AddKey( K_HEARTBEAT, true );  // add only if buffer is empty
}


/*
 *  The PIT interrupt
 *  It runs typically at the slow clock when called in idle mode
 */
RAM_FUNCTION void heartbeat_irq( void )
{
	int count;

	/*
	 * Flash memeory might be disabled, turn it on again
	 */
#if 0
	if ( ClockSpeed == 0 ) {
		SUPC
#endif
	Heartbeats += PIT_GetPIVR() >> 20;
	count = Heartbeats >> HEARTBEAT_SHIFT;
	Heartbeats &= HEARTBEAT_MASK;

	/*
	 *  The keyboard is scanned every 50ms for debounce and repeat
	 */
	scan_keyboard();

	if ( count != 0 ) {
		/*
		 *  Need for speed
		 */
		set_speed( SPEED_MEDIUM );

		/*
		 *  servicee the missung 100ms heart beats
		 */
		while ( count-- ) {
			user_heartbeat();
		}
	}
}


/*************************************************************************
 *  Entry points called from the application
 *************************************************************************/

/*
 *  Check if something is waiting for attention
 */
int is_key_pressed(void)
{
        return 0;
}


/*
 *  Shut down the emulator from the application
 */
void shutdown( void )
{
}


/*
 *  Serve the watch dog
 */
void watchdog( void )
{
	AT91C_BASE_WDTC->WDTC_WDCR=0xA5000001;
}



/*
 *  Main program as seen from C
 */
int main(void)
{
        /*
         * init hardware (LCD, timer, etc.)
	 */




	/*
                forever
                        if key then
                                full_speed
                                while key process_keycode
                        else idle
        */
        process_keycode(0);
        return 0;
}


#ifdef __GNUC__
/*
 *  Get rid of any exception handler code
 */
#define VISIBLE extern __attribute__((externally_visible))
VISIBLE void __aeabi_unwind_cpp_pr0(void) {};
VISIBLE void __aeabi_unwind_cpp_pr1(void) {};
VISIBLE void __aeabi_unwind_cpp_pr2(void) {};
#endif

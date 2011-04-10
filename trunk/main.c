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

#include "board.h"
#include "aic.h"
#include "pit.h"
#include "supc.h"
#include "slcdc.h"

#define BACKUP_SRAM  __attribute__((section(".backup")))
#define RAM_FUNCTION __attribute__((section(".ramfunc")))

/*
 *  Heartbeat frequency in ms
 *  And Auto Power Down threshold
 */
#define PIT_PERIOD 25
#define APD_TICKS 1800 // 3 minutes

/*
 *  CPU speed settings
 */
#define SPEED_IDLE    0
#define SPEED_M_IDLE  1
#define SPEED_MEDIUM  2
#define SPEED_HIGH    3

/// PLL frequency range.
#define CKGR_PLL          AT91C_CKGR_OUT_2
/// PLL startup time (in number of slow clock ticks).
#define PLLCOUNT          AT91C_CKGR_PLLCOUNT
/// PLL MUL value.
#define PLLMUL            999
/// PLL DIV value.
#define PLLDIV            1

/*
 *  Setup the perstent RAM
 */
BACKUP_SRAM TPersistentRam PersistentRam;

/*
 *  Local data
 */
volatile unsigned int ClockSpeed;
volatile int SpeedSetting, DesiredSpeedSetting;
unsigned int Heartbeats;
unsigned int Contrast;
volatile unsigned int InIrq;
volatile unsigned int StartupTicks;
volatile unsigned int PivValue;
volatile int FlashOff;

/*
 *  Definitions for the keyboard scan
 */
#define KEY_ROWS_MASK 0x0000007f
#define KEY_COLS_MASK 0x0400fC00
#define KEY_COLS_SHIFT 11
#define KEY_COL5_SHIFT 10
#define KEY_ON_MASK   0x00000400
#define KEY_REPEAT_MASK 0x0000010100000000LL   // only Up and Down repeat
#define KEY_BUFF_SHIFT 4
#define KEY_BUFF_LEN ( 1 << KEY_BUFF_SHIFT )
#define KEY_BUFF_MASK ( KEY_BUFF_LEN - 1 )

signed char KeyBuffer[ KEY_BUFF_LEN ];
int KbRead, KbWrite, KbCount;
long long KbData, KbDebounce, KbRepeatKey;
int KbRepeatCount;

/*
 *  Short wait to allow output lines to settle
 */
int short_wait( int count )
{
	static volatile int c;
	c = count;
	while ( c-- );
	return c;
}


/*
 *  Scan the keyboard
 */
void scan_keyboard( void )
{
	int i, k;
	unsigned char m;
	union _ll {
		unsigned char c[ 8 ];
		unsigned long long ll;
	} keys;
	long long last_keys;

	/*
	 *  Assume no key is down
	 */
	keys.ll = 0LL;

	/*
	 *  Program the PIO pins in PIOC
	 */
	// Enable clock
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOC;
	// Rows as open drain output
	AT91C_BASE_PIOC->PIO_OER = KEY_ROWS_MASK;
	AT91C_BASE_PIOC->PIO_MDDR = KEY_ROWS_MASK;
	
	/*
	 *  Quick check
	 *  Set all rows to low and test if any key input is low
	 */
	AT91C_BASE_PIOC->PIO_CODR = KEY_ROWS_MASK;
	short_wait( 1 );
	k = ~AT91C_BASE_PIOC->PIO_PDSR;
	AT91C_BASE_PIOC->PIO_SODR = KEY_ROWS_MASK;

	if ( 0 != ( k & KEY_COLS_MASK ) ) {
		/*
		 *  Some keys may be pressed: Assemble the input
		 */
		int r;
		for ( r = 1, i = 0; i < 7; r <<= 1, ++i ) {
			/*
			 *  Set each column to zero and read the row image.
			 *  The input is inverted on read. 1s are pressed keys now.
			 */
			AT91C_BASE_PIOC->PIO_CODR = r;
			short_wait( 1 );
			k = ~AT91C_BASE_PIOC->PIO_PDSR & KEY_COLS_MASK; 
			AT91C_BASE_PIOC->PIO_SODR = r;

			/*
			 *  Adjust the result
			 */
			if ( i == 6 && k & KEY_ON_MASK && StartupTicks > 5 ) {
				/*
				 *  Add ON key to bit image.
				 *  Avoid registering ON directly on power up.
				 */
				k |= 1 << KEY_COLS_SHIFT; 
			}

			/*
			 *  Some bit shuffling required: Columns are on bits 11-15 & 26.
			 *  These are configurable as wakeup pins, a feature we don't use here.
			 */
			k >>= KEY_COLS_SHIFT;
			k = ( k >> KEY_COL5_SHIFT ) | k;
			keys.c[ i ] = (unsigned char) k;
		}
	}

	/*
	 *  Store new images
	 */
	last_keys = KbDebounce;
	KbDebounce = KbData;
	KbData = keys.ll;

	/*
	 *  A key is newly pressed, if
	 *  a) it wasn't pressed last time we checked
	 *  b) it has the same value as the debounce value
	 */
	keys.ll &= ( ~last_keys & KbDebounce );
	
	/*
	 *  Reprogram PIO
	 */
	// All as input
	AT91C_BASE_PIOC->PIO_ODR = KEY_ROWS_MASK | KEY_COLS_MASK;
	// Disable clock
	AT91C_BASE_PMC->PMC_PCDR = 1 << AT91C_ID_PIOC;

	/*
	 *  Handle repeating keys (arrows)
	 */
	if ( keys.ll == 0 && KbData == KbRepeatKey ) {
		/*
		 *  One of the repeating keys is still down
		 */
		++KbRepeatCount;
		if ( KbRepeatCount == 20
		  || ( KbRepeatCount > 20 && ( ( KbRepeatCount - 20 ) & 7 ) == 0 ) )
		{
			/*
			 *  This should repeat after half a second at 5 per second
			 */
			keys.ll = KbRepeatKey;
		}
	}
	else {
		/*
		 *  Restart the repeat for new key
		 */
		KbRepeatCount = 0;
		KbRepeatKey = keys.ll & KEY_REPEAT_MASK;
	}

	if ( keys.ll != 0 ) {
		/*
		 *  Decode
		 */
		k = 0;
		for ( i = 0; i < 7; ++i ) {
			/*
			 *  Handle each row
			 */
			for ( m = 1; m != 0x40; m <<= 1 ) {
				/*
				 *  Handle each column
				 */
				if ( keys.c[ i ] & m ) {
					/*
					 *  First key found exits loop;
					 */
					i = 7;

					/*
					 *  Add key to buffer
					 */
					put_key( k );
					break;
				}
				/*
				 *  Try next code
				 */
				++k;
			}
		}
	}
}


/*
 *  Set the contrast
 */
void set_contrast( unsigned int contrast )
{
	if ( contrast == Contrast ) {
		/*
		 *  No change
		 */
		return;
	}

	/*
	 *  Update supply controller settings
	 */
	SUPC_SetSlcdVoltage( ( contrast - 1 ) & 0x0f );

	Contrast = contrast;
}


/*
 *  Setup the LCD controller
 */
void enable_lcd( void )
{
	if ( Contrast != 0 ) {
		/*
		 *  LCD is running
		 */
		return;
	}

	/*
	 *  Switch LCD on in supply controller and clear the RAM
	 */
	SUPC_EnableSlcd( 1 );
	SLCDC_Clear();

	/*
	 *  Configure it for 10 commons and 40 segments, non blinking
	 */
	SLCDC_Configure( 10, 40, AT91C_SLCDC_BIAS_1_3, AT91C_SLCDC_BUFTIME_8_Tsclk );
	SLCDC_SetFrameFreq( AT91C_SLCDC_PRESC_SCLK_16, AT91C_SLCDC_DIV_2 );
	SLCDC_SetDisplayMode( AT91C_SLCDC_DISPMODE_NORMAL );

	/*
	 *  Set contrast and enable the display
	 */
	set_contrast( State.contrast + 1 );
	SLCDC_Enable();
}


/*
 *  Stop the LCD controller
 */
void disable_lcd( void )
{
	/*
	 *  Disable the controller itself
	 */
	SLCDC_Disable();

	/*
	 *  Turn it off in supply controller
	 */
	SUPC_DisableSlcd();
	Contrast = 0;
}


/*
 *  Go to idle mode
 *  // Runs from SRAM to be able to turn of flash
 */
RAM_FUNCTION void go_idle( void )
{
	/*
	 *  Disable flash memory in order to save power
	 *  ** Disabled for now **
	 */
	if ( 0 && SpeedSetting == SPEED_IDLE ) {
		SUPC_DisableFlash();
		FlashOff = 1;
	}

	/*
	 *  Disable the processor clock and go to sleep
	 */
	AT91C_BASE_PMC->PMC_SCDR = AT91C_PMC_PCK;
	while ( ( AT91C_BASE_PMC->PMC_SCSR & AT91C_PMC_PCK ) != AT91C_PMC_PCK );
}


/*
 *  Helpers for set_speed
 */
#define wait_for_clock() while ( ( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) \
				 != AT91C_PMC_MCKRDY )
#define disable_pll()  ( AT91C_BASE_PMC->PMC_PLLR = 0x8000 )
#define disable_mclk() ( AT91C_BASE_PMC->PMC_MOR = 0x370000 )
#define enable_mclk()  ( AT91C_BASE_PMC->PMC_MOR = 0x370001 )

static void set_mckr( int pres, int css )
{
	int mckr = AT91C_BASE_PMC->PMC_MCKR;
	if ( ( mckr & AT91C_PMC_PRES ) != pres ) {
		AT91C_BASE_PMC->PMC_MCKR = ( mckr & ~AT91C_PMC_PRES ) | pres;
		wait_for_clock();
	}
	if ( ( mckr & AT91C_PMC_CSS ) != css ) {
		AT91C_BASE_PMC->PMC_MCKR = ( mckr & ~AT91C_PMC_CSS ) | css;
		wait_for_clock();
	}
}


/*
 *  Set clock speed to one of 4 fixed values:
 *  SPEED_IDLE, SPEED_M_IDLE, SPEED_MEDIUM, SPEED_HIGH
 *  In the idle modes, the CPU clock will be turned off.
 *
 *  Physical speeds are 2 MHZ / 64, 2 MHz, 32.768 MHz
 *  Using the slow clock doesn't seem to work for unknown reasons.
 *  Therefore, the main clock with a divider is used as slowest clock.
 *
 *  Since the PIT depends on the speed setting, changes work only reliably
 *  while in the interrupt service routine.
 */
void set_speed_hw( unsigned int speed )
{
	int cpiv;
	/*
	 *  Table of supported speeds
	 */
	static int speeds[ SPEED_HIGH + 1 ] = 
		{ 2000000 / 64 , 2000000, 2000000, 32768 * ( 1 + PLLMUL ) };

	/*
	 *  Set new speed, called from timer interrupt
	 */
	SpeedSetting = speed;
	if ( speed > SPEED_HIGH || speeds[ speed ] == ClockSpeed ) {
		/*
		 *  Invalid or no change.
		 *  The PIV may be a little off from the last speed change.
		 */
		PIT_SetPIV( PivValue );
		return;
	}
	ClockSpeed = speeds[ speed ];
	PivValue = ( ( PIT_PERIOD * ClockSpeed ) / 1000 + 8 ) >> 4;

	switch ( speed ) {

	case SPEED_IDLE:
		/*
		 *  Turn the clock almost off
		 *  System will go idle from main loop
		 */
		enable_mclk();
		set_mckr( AT91C_PMC_PRES_CLK_64, AT91C_PMC_CSS_MAIN_CLK );

		// No wait states for flash read
		AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_0FWS;

		// Turn off the unused oscillators
		disable_pll();
		break;

	case SPEED_M_IDLE:
	case SPEED_MEDIUM:
		/*
		 *  2 MHz internal RC clock
		 */
		enable_mclk();
		set_mckr( AT91C_PMC_PRES_CLK, AT91C_PMC_CSS_MAIN_CLK );

		// No wait states needed for flash read
		AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_0FWS;

		// Turn off the PLL
		disable_pll();
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
			// Intermediate switch to main clock
			enable_mclk();
			set_mckr( AT91C_PMC_PRES_CLK, AT91C_PMC_CSS_MAIN_CLK );
		}
		// Initialize PLL at 32MHz
		AT91C_BASE_PMC->PMC_PLLR = CKGR_PLL | PLLCOUNT \
					 | ( PLLMUL << 16 ) | PLLDIV;
		while ( ( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK ) == 0 );

		// Switch to PLL
		set_mckr( AT91C_PMC_PRES_CLK, AT91C_PMC_CSS_PLL_CLK );

		// Turn off main clock
		disable_mclk();
		break;
	}
	/*
	 *  Now we have to reprogram the PIT.
	 *  The next interrupt is scheduled just one period after now.
	 */
	cpiv = PIT_GetPIIR() & AT91C_PITC_CPIV;
	PIT_SetPIV( PivValue + cpiv );
}


/*
 *  Set clock speed to one of 4 fixed values:
 *  SPEED_IDLE, SPEED_M_IDLE, SPEED_MEDIUM, SPEED_HIGH
 *  In the idle modes, the CPU clock gets turned off.
 *
 *  Since the PIT depends on the speed setting, changes work only reliably
 *  while in the interrupt service routine.
 *  This part only sets the desired speed for the hardware speed setting above.
 */
void set_speed( unsigned int speed )
{
	if ( ( is_debug() || StartupTicks < 10 ) && speed < SPEED_MEDIUM ) {
		/*
		 *  Allow JTAG debugging
		 */
		speed = SPEED_MEDIUM;
	}

	/*
	 *  Postpone the speed switch until next interrupt
	 */
	DesiredSpeedSetting = speed;
	if ( speed <= SPEED_M_IDLE ) {
		/*
		 *  Save power
		 */
		go_idle();
	}
}


/*
 *  The 100ms heartbeat, called from the PIT interrupt
 */
void user_heartbeat( void )
{
	/*
	 *  Disallow power saving for one second after reset
	 *  This is for JTAG debugging.
	 *  ON key is disabled while this value is below 5.
	 */
	if ( StartupTicks < 10 ) {
		++StartupTicks;
	}

	/*
	 *  Application ticker in 10th of seconds
	 */
	++Ticker;

	if ( State.pause ) {
		/*
		 *  The PSE handler checks this value
		 */
	        --State.pause;
	}

	/*
	 *  Allow keyboard timeout handling.
	 *  HAndles Auto Power Down (APD)
	 */
	if ( Keyticks < APD_TICKS ) {
		++Keyticks;
	}
	else {
		shutdown();
	}

	/*
	 *  Put a dummy key code in buffer to wake up application
	 */
        put_key( K_HEARTBEAT );
}


/*
 *  The PIT interrupt
 *  It starts at a slow clock when called in idle mode
 */
RAM_FUNCTION void heartbeat_irq( void )
{
	int count;
	
	InIrq = 1;

	/*
	 *  Flash memory might be disabled, turn it on again
	 */
	if ( FlashOff ) {
		SUPC_EnableFlash( 2 );  // ~ 130 microseconds wakeup at 32768 Hz
	}

	/*
	 *  Set speed to 2MHz for all irq handling. This ensures consistent timing
	 */
	set_speed_hw( SPEED_MEDIUM );

	/*
	 *  Since all system peripherals are tied to the same IRQ source 1
	 *  we need to check, if this is really the PIT interrupt
	 */
	if ( !PIT_GetStatus() ) {
		/*
		 *  Add other sources here
		 */
		// ...
	}
	else {
		/*
		 *  The keyboard is scanned every 33ms for debounce and repeat
		 */
		scan_keyboard();

		/*
		 *  Get the number of missed interrupts
		 */
		Heartbeats += PIT_GetPIVR() >> 20;
		count = Heartbeats / ( 100 / PIT_PERIOD );
		Heartbeats -= ( 100 / PIT_PERIOD ) * count;

		if ( count != 0 ) {
			/*
			 *  Service the outstanding 100ms heart beats
			 */
			while ( count-- ) {
				user_heartbeat();
			}
		}
	}
	
	/*
	 *  Set hardware speed to desired speed
	 */
	if ( DesiredSpeedSetting != SpeedSetting ) {
		set_speed_hw( DesiredSpeedSetting );
	}
	InIrq = 0;
}


/*
 *  Enable the PIT to generate interrupts
 *  Do this after setting the speed so that the PIV value is already set.
 */
void enable_heartbeat()
{
	InIrq = 0;

	/*
	 *  Tell the interrupt controller where to go
	 */
	AIC_ConfigureIT( AT91C_ID_SYS, 
		         AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL 
		         | AT91C_AIC_PRIOR_HIGHEST,
		         heartbeat_irq );
	/*
	 *  Enable IRQ 1
	 */
	AIC_EnableIT( AT91C_ID_SYS );

	/*
	 *  Enable the PIT and its interrupt
	 */
	PIT_EnableIT();
	PIT_Enable();
}


/*
 *  Lock / unlock by disabling / enabling the PIT interrupt.
 *  Do nothing if called inside the interrupt handler
 */
void lock( void )
{
	if ( !InIrq ) {
		AIC_DisableIT( AT91C_ID_SYS );
	}
}

void unlock( void )
{
	if ( !InIrq ) {
		AIC_EnableIT( AT91C_ID_SYS );
	}
}


/*************************************************************************
 *  Entry points called from the application
 *************************************************************************/

/*
 *  Check if something is waiting for attention
 *  Returns the number of keys in the buffer
 */
int is_key_pressed( void )
{
        return KbCount;
}


/*
 *  Get a key
 */
int get_key( void )
{
	int k;
	if ( KbCount == 0 ) {
		/*
		 *  No key in buffer
		 */
		return -1;
	}
	lock();
	k =  KeyBuffer[ KbRead ];
	KbRead = ( KbRead + 1 ) & KEY_BUFF_MASK;
	--KbCount;
	unlock();
	return k;
}


/*
 *  Add a key to the buffer
 *  Returns 0 in case of failure or number of keys in buffer
 */
int put_key( int k )
{
	if ( KbCount == KEY_BUFF_LEN ) {
		/*
		 *  Sorry, no room
		 */
		return 0;
	}

	if ( k == K_HEARTBEAT && KbCount != 0 ) {
		/*
		 *  Don't fill the buffer with heartbeats
		 */
		return 0;
	}
	lock();
	KeyBuffer[ KbWrite ] = (unsigned char) k;
	KbWrite = ( KbWrite + 1 ) & KEY_BUFF_MASK;
	++KbCount;
	unlock();
	return KbCount;
}


/*
 *  Turn everything except the backup sram off
 */
void shutdown( void )
{
	/*
	 *  Turn off display gracefully
	 */
	disable_lcd();

	/*
	 *  Off we go...
	 */
	SUPC_DisableVoltageRegulator();
}


/*
 *  Serve the watch dog
 */
void watchdog( void )
{
	AT91C_BASE_WDTC->WDTC_WDCR=0xA5000001;
}


/*
 *  Is debugger active ?
 *  The flag is set via the JTAG probe
 */
#define DEBUG_FLAG ((char *)(&PersistentRam))[ 0x7ff ]

int is_debug( void )
{
	return DEBUG_FLAG == 0xA5;
}



/*
 *  Main program as seen from C
 */
int main(void)
{
        /*
         * Initialize the hardware (clock, backup RAM, LCD, RTC, timer)
	 */
	set_speed_hw( SPEED_MEDIUM );
	SUPC_EnableSram();
	Keyticks = 0;
	enable_lcd();
	SUPC_EnableRtc();
	enable_heartbeat();

	/*
	 *  Force DEBUG mode for JTAG debugging
	 */
	// DEBUG_FLAG = 0xA5;

	/*
	 *  Allow wake up on ON key
	 */
	AT91C_BASE_SUPC->SUPC_WUMR = (AT91C_BASE_SUPC->SUPC_WUMR)& ~(0x7u <<12);
	SUPC_SetWakeUpSources( AT91C_SUPC_FWUPEN );

	/*
	 *  Initialize the software
	 */
	init_34s();

	/*
	 *  Wait for event and execute it
	 */
	while( 1 ) {
                int k;

		while ( !is_key_pressed() ) {
			/*
			 *  Save power if nothing in queue
			 *  Clock is slowed down after 2 seconds of inactivity
			 */
			set_speed( Keyticks < 20 ? SPEED_M_IDLE : SPEED_IDLE );
		}

		/*
		 *  Read out the keyboard queue
		 */
		k = get_key();

		if ( k != K_HEARTBEAT ) {
			/*
			 *  Increase the speed of operation
			 */
			set_speed( SPEED_HIGH );
		}

		/*
		 *  Handle the input
		 */
		process_keycode( k );
		if ( k != K_HEARTBEAT ) {
			Keyticks = 0;
		}

		/*
		 *  Adjust the display contrast if it has been changed
		 */
		if ( State.contrast != Contrast ) {
			/*
			 *  The saved value ranges from 0 to 15.
			 *  We use values 1 to 16 or 0 which means off
			 */
			set_contrast( State.contrast + 1 );
		}
	}
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


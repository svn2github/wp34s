/* This file is part of 34S.
 * 
 * This file is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the WTFPL, Version 2, as
 * published by Sam Hocevar. See http://sam.zoy.org/wtfpl/COPYING
 * for more details.
 *
 * The WFTPL licensing is specific to this file.
 * Other parts of the software may be covered by a different
 * licence, such as the GPL v3 (See http://www.gnu.org/licenses/).
 *
 * Author (this file only): Marcus von Cube, http://www.mvcsys.de
 */
// #define DEBUG_MAIN  // Adds some key sequences for debugging

/*
 * This is the main module for the real hardware
 */
#include "xeq.h"
#include "display.h"
#include "lcd.h"
#include "keys.h"
#include "storage.h"
#include "serial.h"
#ifdef INCLUDE_STOPWATCH
#include "stopwatch.h"
#endif

#ifndef at91sam7l128
#define at91sam7l128 1
#endif
#include "board.h"
#include "aic.h"
#include "supc.h"
#include "slcdc.h"
#include "rtc.h"

extern void ForceReset(void);

/*
 *  Some conditional compilation defines
 */
#define SLOW_SERIAL
//#define SPEEDUP_ON_KEY_WAITING

/*
 *  CPU speed settings
 */
#define SPEED_IDLE     0
#define SPEED_SLOW     1
#define SPEED_MEDIUM   2
#define SPEED_HALF     3
#define SPEED_HIGH     4

#define SLEEP_ANNUNCIATOR LIT_EQ
// #define SPEED_ANNUNCIATOR LIT_EQ
void set_speed( unsigned int speed );

/*
 *  SUPC Voltages
 */
#define SUPC_VDD_155 (0x2 << 9)
#define SUPC_VDD_165 (0x3 << 9)
#define SUPC_VDD_175 (0x4 << 9)
#define SUPC_VDD_180 (0x5 << 9)

/*
 *  BOD detector voltages
 */
#define SUPC_BOD_1_9V 0
#define SUPC_BOD_2_0V 1
#define SUPC_BOD_2_1V 2
#define SUPC_BOD_2_2V 3
#define SUPC_BOD_2_3V 4
#define SUPC_BOD_2_4V 5
#define SUPC_BOD_2_5V 6
#define SUPC_BOD_2_6V 7
#define SUPC_BOD_2_7V 8
#define SUPC_BOD_2_8V 9
#define SUPC_BOD_2_9V 10
#define SUPC_BOD_3_0V 11
#define SUPC_BOD_3_1V 12
#define SUPC_BOD_3_2V 13
#define SUPC_BOD_3_3V 14
#define SUPC_BOD_3_4V 15
#define SUPC_BOD_MAX  15

/// PLL frequency range.
#define CKGR_PLL          AT91C_CKGR_OUT_2
/// PLL startup time (in number of slow clock ticks).
#define PLLCOUNT          AT91C_CKGR_PLLCOUNT
/// PLL MUL values.
#define PLLMUL_HALF       599   // half speed (20 MHz)
#define PLLMUL_HALF_NX    665   // if no crystal installed
#define PLLMUL_FULL       1124  // (37 MHz) 1199 is probably too demanding on the hardware
#define PLLMUL_FULL_NX    1249  // 1333  // no crystal
/// PLL DIV value.
#define PLLDIV            1

/*
 *  Heart beat frequency in ms
 *  And Auto Power Down threshold
 *  Deep sleep and idle settings
 */
#define APD_TICKS 6000 // 10 minutes
#define APD_VOLTAGE SUPC_BOD_2_2V
#define LOW_VOLTAGE SUPC_BOD_2_5V
#define ALLOW_DEEP_SLEEP 1   // undef to disable
#define TICKS_BEFORE_DEEP_SLEEP 20

//#define RAM_FUNCTION __attribute__((section(".ramfunc"),noinline))
//#define NO_INLINE    __attribute__((noinline))
#define NO_RETURN      __attribute__((noreturn))

/*
 *  Local data
 */
volatile unsigned int ClockSpeed;
volatile unsigned short StartupTicks;
volatile unsigned char SpeedSetting;
volatile unsigned char Heartbeats;
unsigned char UserHeartbeatCountDown;
unsigned char Contrast;
volatile unsigned char LcdFadeOut;
volatile unsigned char InIrq;
unsigned char DebugFlag;
#ifdef XTAL
#define Xtal (1)
#else
unsigned char Xtal;
#endif
#ifdef SLEEP_ANNUNCIATOR
unsigned char SleepAnnunciatorOn;
#endif

#ifdef DEBUG_MAIN
int WdDisable;
#endif

/*
 *  Definitions for the keyboard scan
 */
#define DEBOUNCE_ON_LOW 1 // alternate debouncer
#define KEY_ROWS_MASK 0x0000007f
#define KEY_COLS_MASK 0x0400fC00
#define KEY_WAKEUP_MASK 0x1f80
#define KEY_COLS_SHIFT 11
#define KEY_COL5_SHIFT 10
#define KEY_ON_MASK   0x00000400
#define KEY_REPEAT_MASK 0x0000010100000000LL   // only Up and Down repeat
#define KEY_SHIFT_F_MASK 0x0000000000000800LL
#define KEY_SHIFT_G_MASK 0x0000000000001000LL
#define KEY_SHIFT_H_MASK 0x0000000000002000LL
#define KEY_REPEAT_START 25
#define KEY_REPEAT_NEXT 35
#define KEY_BUFF_SHIFT 4
#define KEY_BUFF_LEN ( 1 << KEY_BUFF_SHIFT )
#define KEY_BUFF_MASK ( KEY_BUFF_LEN - 1 )

signed char KeyBuffer[ KEY_BUFF_LEN ];
volatile char KbRead, KbWrite, KbCount;
volatile char OnKeyPressed;
short int KbRepeatCount;
long long KbData, KbDebounce, KbRepeatKey;
short int BodThreshold;
short int BodStopwatch;

/*
 *  DBGU serial I/O definitions
 */
#define USE_SYSTEM_IRQ 1
#define DBGU_PIOC_MASK 0x00030000

/*
 *  Short wait to allow output lines to settle
 */
void short_wait( int count )
{
	if ( SpeedSetting > SPEED_MEDIUM ) {
		count *= 16;
	}
	while ( count-- ) {
		// Exclude from optimisation
		asm("");
	}
}


/*
 *  Lock / unlock by disabling / enabling the periodic interrupt.
 *  Do nothing if called inside the interrupt handler
 */
#undef lock
void lock( void )
{
	if ( !InIrq ) {
		AIC_DisableIT( AT91C_ID_SLCD );
	}
}

#undef unlock
void unlock( void )
{
	if ( !InIrq ) {
		AIC_EnableIT( AT91C_ID_SLCD );
	}
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
	OnKeyPressed = 0;

	/*
	 *  Program the PIO pins in PIOC
	 */
	// Enable clock
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOC;
	// Rows as open drain output
	AT91C_BASE_PIOC->PIO_OER =  KEY_ROWS_MASK;
	AT91C_BASE_PIOC->PIO_MDER = KEY_ROWS_MASK;
	// No pull ups on outputs, only on keyboard inputs
	AT91C_BASE_PIOC->PIO_PPUDR = ~KEY_COLS_MASK;
	AT91C_BASE_PIOC->PIO_PPUER =  KEY_COLS_MASK;
	
	/*
	 *  Quick check
	 *  Set all rows to low and test if any key input is low
	 */
	AT91C_BASE_PIOC->PIO_CODR = KEY_ROWS_MASK;
	short_wait( 2 );
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
			short_wait( 2 );
			k = ~AT91C_BASE_PIOC->PIO_PDSR & KEY_COLS_MASK; 
			AT91C_BASE_PIOC->PIO_SODR = r;

			/*
			 *  Adjust the result
			 */
			OnKeyPressed = 0 != ( k & KEY_ON_MASK );
			if ( i == 6 && OnKeyPressed && StartupTicks > 5 ) {
				/*
				 *  Add ON key to bit image.
				 *  Avoid registering ON directly on power up.
				 */
				k |= 1 << KEY_COLS_SHIFT;
			}

			/*
			 *  Some bit shuffling required: Columns are on bits 11-15 & 26.
			 *  These are configurable as wake up pins, a feature we don't use here.
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

#ifdef DEBOUNCE_ON_LOW
	/*
	 *  A key is newly pressed, if
	 *  a) it wasn't pressed the last two times checked
	 *  b) it is pressed now
	 */
	keys.ll &= ( ~last_keys & ~KbDebounce );
#else
	/*
	 *  A key is newly pressed, if
	 *  a) it wasn't pressed last time we checked
	 *  b) it has the same value as the debounce value
	 */
	keys.ll &= ( ~last_keys & KbDebounce );
#endif
	
	/*
	 *  Program PIO
	 */
	// All as input, no pull-ups
	AT91C_BASE_PIOC->PIO_ODR = KEY_ROWS_MASK | KEY_COLS_MASK;
	AT91C_BASE_PIOC->PIO_PPUDR = KEY_ROWS_MASK | KEY_COLS_MASK;

	// Disable clock
	AT91C_BASE_PMC->PMC_PCDR = 1 << AT91C_ID_PIOC;

	/*
	 *  Handle repeating keys (arrows)
	 *  Repeat is suppressed when a pending operation is waiting for key-up.
	 */
	if ( keys.ll == 0 && KbData == KbRepeatKey && KbCount <= 2 && OpCode == 0 ) {
		/*
		 *  One of the repeating keys is still down
		 */
		++KbRepeatCount;
		if ( KbRepeatCount == KEY_REPEAT_START ) {
			// Start repeat
			keys.ll = KbRepeatKey;
		}
		else if ( KbRepeatCount == KEY_REPEAT_NEXT ) {
			// Continue repeat
			keys.ll = KbRepeatKey;
			KbRepeatCount = KEY_REPEAT_START;
		}
	}
	else {
		/*
		 *  Restart the repeat for new key
		 */
		KbRepeatCount = 0;
		KbRepeatKey = keys.ll & KEY_REPEAT_MASK;
	}

	if ( keys.ll == 0 ) {
		/*
		 *  No new key to decode: test for key-up
		 */
#ifdef DEBOUNCE_ON_LOW
		/*
		 *  Key must have been twice UP for debounce reasons)
		 */
		if ( ( KbDebounce & ~KbData ) != 0 ) {
#else
		/*
		 *  Key must have been twice DOWN for debounce reasons)
		 */
		if ( ( last_keys & KbDebounce & ~KbData ) != 0 ) {
#endif
			/*
			 *  A key has been released
			 */
			put_key( K_RELEASE );
		}
	}
	else {
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
 *  Setup the LCD controller
 */
void enable_lcd( void )
{
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
	//SLCDC_SetDisplayMode( AT91C_SLCDC_DISPMODE_NORMAL );
	SLCDC_SetDisplayMode( AT91C_SLCDC_DISPMODE_LOAD_ONLY );

	/*
	 *  Set contrast and enable the display
	 */
	SUPC_SetSlcdVoltage( Contrast = UState.contrast );
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
 */
void go_idle( void )
{
	if ( is_debug() ) {
		/*
		 *  Idle and  modes cannot be debugged
		 */
		return;
	}

	/*
	 *  Voltage regulator to deep mode
	 */
	SUPC_EnableDeepMode();

	/*
	 *  Disable the processor clock and go to sleep
	 */
	AT91C_BASE_PMC->PMC_SCDR = AT91C_PMC_PCK;
	while ( ( AT91C_BASE_PMC->PMC_SCSR & AT91C_PMC_PCK ) != AT91C_PMC_PCK );
}


/*
 *  Common part of shutdown() and deep_sleep()
 */
NO_RETURN void turn_off( void )
{
	/*
	 *  Disable any unused oscillators and let the rest go slow
	 */
	set_speed( SPEED_SLOW );

	/*
	 *  Turn off the BOD, if it was still on
	 */
	AT91C_BASE_SUPC->SUPC_BOMR = 0;

	/*
	 *  Off we go...
	 */
	SUPC_DisableVoltageRegulator();
}


/*
 *  Turn everything except the backup sram off
 */
NO_RETURN void shutdown( void )
{
	/*
	 *  CmdLine will be lost, process it first
	 */
	process_cmdline_set_lift();
	init_state();

	/*
	 *  Let Interrupt fade out the display
	 */
	LcdFadeOut = UState.contrast;
	message( "Bye...", "" );

	/*
	 *  Ensure the RAM checksum is correct
	 */
	set_speed( SPEED_HALF );
	checksum_all();

	/*
	 *  Wait for ON key release and LCD to go blank
	 */
	while ( OnKeyPressed || LcdFadeOut ) {
		set_speed( SPEED_IDLE );
	}

	/*
	 *  Turn off display gracefully
	 */
	disable_lcd();

	/*
	 *  Disable IRQ 11 in AIC and SLCD
	 */
	AIC_DisableIT( AT91C_ID_SLCD );
	SLCDC_DisableInterrupts( AT91C_SLCDC_ENDFRAME );

	/*
	 *  Allow wake up on ON key only, set debouncer to 0.015625 sec (4096 gives 0.125 sec)
	 */
	AT91C_BASE_SUPC->SUPC_WUMR = AT91C_SUPC_FWUPDBC_512_SLCK | AT91C_SUPC_FWUPEN;
	AT91C_BASE_SUPC->SUPC_WUIR = 0;

	/*
	 *  Turn off
	 */
	turn_off();
}


#ifdef ALLOW_DEEP_SLEEP
/*
 *  We use the LCD display buffer memory (50 bytes) for saving some state.
 *  Only even registers have full 32 bits, odd registers have only 8 bits.
 */
void save_state_to_lcd_memory( int save )
{
	int *ip, *lcd;
	int i;
	const int size = sizeof( StateWhileOn );
	const int max_i = size > 40 ? 10 : ( size + 3 ) >> 2;

	/*
	 *  10 words into even registers
	 */
	ip = (int *) &StateWhileOn;
	lcd = (int *) AT91C_SLCDC_MEM;

	for ( i = 0; i < max_i; ++i ) {
		if ( save ) {
			*lcd = *ip;
		}
		else {
			*ip = *lcd;
			*lcd = 0;
		}
		++ip;
		lcd += 2; // Skip odd registers
	}

	/*
	 *  Up to 10 bytes into odd registers
	 */
	if ( size > 40 ) {

		char *cp = (char *) ip;
		lcd = (int *) AT91C_SLCDC_MEM + 1;

		for ( i = 0; i < size - 40; ++i ) {
			if ( save ) {
				*lcd = *cp;
			}
			else {
				*cp = (char) *lcd;
				*lcd = 0;
			}
			++cp;
			lcd += 2; // Skip even registers
		}
	}
}


/*
 *  Go to deep sleep mode.
 *  Wait for keyboard or RTC to wake up.
 */
void deep_sleep( void )
{
	unsigned char minute, second;
	int wakeup_time;

	if ( Keyticks >= APD_TICKS ) {
		/*
		 *  No point in going to deep sleep if we have reached the APD timeout
		 */
		shutdown();
	}

	lock();
	scan_keyboard();

	if ( WaitForLcd || KbData ) {
		/*
		 *  LCD is still busy or a key is down, ignore the call
		 */
		unlock();
		return;
	}

	Crc = MAGIC_MARKER;
	State.deep_sleep = 1;

	/*
	 *  Disable IRQ 11 in AIC and SLCD
	 */
	AIC_DisableIT( AT91C_ID_SLCD );
	SLCDC_DisableInterrupts( AT91C_SLCDC_ENDFRAME );

	/*
	 *  What time is it?
	 */
	RTC_GetTime( NULL, &minute, &second );
	LastActiveSecond = (unsigned short) minute * 60 + second;

	/*
	 *  Use SLCD memory to save some state
	 */
	save_state_to_lcd_memory( 1 );

	/*
	 *  Set RTC-Alarm for wake up at next event
	 *  At the moment, this is just APD (Auto Power Down)
	 */
	wakeup_time = LastActiveSecond + ( APD_TICKS - Keyticks ) / 10 + 2;
	wakeup_time %= 3600;
	minute = (unsigned char) ( wakeup_time / 60 );
	second = (unsigned char) ( wakeup_time % 60 );

	/*
	 *  Clear all events in the RTC and program the alarm
	 */
	AT91C_BASE_RTC->RTC_SCCR = AT91C_BASE_RTC->RTC_SR;
	RTC_SetTimeAlarm( NULL, &minute, &second );

	/*
	 *  All keyboard rows as open drain output
	 *  All the rest as input
	 */
	AT91C_BASE_PIOC->PIO_ODR = ~KEY_ROWS_MASK;
	AT91C_BASE_PIOC->PIO_OER =  KEY_ROWS_MASK;
	AT91C_BASE_PIOC->PIO_MDER = KEY_ROWS_MASK;

	/*
	 *  No pull ups on outputs, only on keyboard inputs
	 */
	AT91C_BASE_PIOC->PIO_PPUDR = ~KEY_COLS_MASK;
	AT91C_BASE_PIOC->PIO_PPUER =  KEY_COLS_MASK;

	/*
	 *  All outputs low
	 */
	AT91C_BASE_PIOC->PIO_CODR =  KEY_ROWS_MASK;

	/*
	 *  Prepare keyboard wake up
	 */
	AT91C_BASE_SUPC->SUPC_WUIR = KEY_WAKEUP_MASK;

	/*
	 *  Prepare RTC and ON key wake up
	 *  Set debouncers to minimum value
	 */
	AT91C_BASE_SUPC->SUPC_WUMR = AT91C_SUPC_FWUPEN | AT91C_SUPC_RTCEN;

	/*
	 *  Turn off
	 */
	turn_off();
}
#endif


/*
 *  Brown out detection
 */
void detect_voltage( void )
{
	/*
	 *  Test low voltage conditions
	 */
	if ( is_test_mode() ) {
		Voltage = LOW_VOLTAGE - 1;
		return;
	}

	/*
	 *  Don't be active all the time
	 */
	if ( BodStopwatch > 0 ) {
		--BodStopwatch;
		return;
	}

	/*
	 *  Check if brownout state has changed
	 *  while cycling through all possible values in descending order.
	 *  Called every 25 ms.
	 */
	if ( BodThreshold == -1 ) {
		/*
		 *  First call
		 */
		if ( Voltage == 0 ) {
			Voltage = SUPC_BOD_3_0V;
		}
		BodThreshold = Voltage;
	}
	else {
		/*
		 *  Get current status and decrease Threshold value
		 */
		unsigned char brownout =
			0 != ( AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_BROWNOUT );

		if ( !brownout ) {
			/*
			 *  Voltage must be above the current threshold
			 */
			Voltage = BodThreshold;

			/*
			 *  Try next higher value
			 */
			if ( BodThreshold < SUPC_BOD_MAX ) {
				++BodThreshold;
			}

			/*
			 *  Wait some time before next loop with BOD disabled
			 */
			BodStopwatch = 25;
			AT91C_BASE_SUPC->SUPC_BOMR = 0;
		}
		else if ( BodThreshold == 0 ) {
			/*
			 *  Start over with highest threshold
			 */
			BodThreshold = SUPC_BOD_MAX;
		}
		else {
			/*
			 *  Decrease threshold and wait for next measurement
			 */
			--BodThreshold;
		}
	}
	/*
	 *  Set detector to new threshold
	 */
	AT91C_BASE_SUPC->SUPC_BOMR = AT91C_SUPC_BODSMPL_CONTINUOUS | BodThreshold;
}



/*
 *  Helpers for set_speed
 */
#define wait_for_clock() while ( ( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) \
				 != AT91C_PMC_MCKRDY )
#define disable_pll()  ( AT91C_BASE_PMC->PMC_PLLR = 0x8000 )
#define disable_mclk() ( AT91C_BASE_PMC->PMC_MOR = 0x370000 )
#define enable_mclk()  ( AT91C_BASE_PMC->PMC_MOR = 0x370001 )

/*
 *  Set the master clock register in two steps
 */
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
 *  Set clock speed to one of 5 fixed values:
 *  SPEED_IDLE, SPEED_SLOW, SPEED_MEDIUM, SPEED_HALF, SPEED_HIGH
 *  In the idle modes, the CPU clock will be turned off.
 *
 *  Physical speeds are 2 MHZ / 64, 2 MHz, 20 MHz, 37 MHz
 *  Using the slow clock doesn't seem to work for unknown reasons.
 *  Therefore, the main clock with a divider is used as slowest clock.
 *
 *  Speed remains untouched while the serial I/O is running
 */
void set_speed( unsigned int speed )
{
	unsigned int pll;

	/*
	 *  Table of supported speeds
	 */
	static const int speeds[ SPEED_HIGH + 1 ] =
		{ 2000000 / 64 , 2000000 / 64 , 2000000,
		  32768 * ( 1 + PLLMUL_HALF ),
		  32768 * ( 1 + PLLMUL_FULL ) };

	if ( !SerialOn ) {
		/*
		 *  Speed changes not allowed while the serial port is active
		 */
		if ( speed < SPEED_MEDIUM && ( is_debug() || StartupTicks < 10 ) ) {
			/*
			 *  Allow JTAG debugging
			 */
			speed = SPEED_MEDIUM;
		}

		/*
		 *  If low voltage or requested by user reduce maximum speed
		 */
		if ( speed == SPEED_HIGH ) {
			if ( ( UState.slow_speed || Voltage <= LOW_VOLTAGE ) ) {
				speed = SPEED_HALF;
			}
		}
		if ( speed == SpeedSetting ) {
			/*
			 *  No change.
			 */
			goto idle_check;
		}
#ifdef SPEED_ANNUNCIATOR
		if ( speed == SPEED_HIGH || SpeedSetting == SPEED_HIGH ) {
			dot( SPEED_ANNUNCIATOR, speed == SPEED_HIGH );
			finish_display();
		}
#endif
		/*
		 *  Set new speed
		 */
		lock();

		if ( speed == SPEED_HIGH ) {
			/*
			 *  We need full voltage in the core
			 */
			SUPC_SetVoltageOutput( SUPC_VDD_180 );

			/*
			 *  At frequencies beyond 30 MHz, we need wait states for flash reads
			 */
			AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_2FWS;
		}

		switch ( speed ) {

		case SPEED_IDLE:
		case SPEED_SLOW:
			/*
			 *  Turn the clock almost off
			 *  System will go idle from main loop
			 */
			enable_mclk();
			set_mckr( AT91C_PMC_PRES_CLK_64, AT91C_PMC_CSS_MAIN_CLK );

			// Turn off the unused oscillators
			disable_pll();
			break;

		case SPEED_MEDIUM:
			/*
			 *  2 MHz internal RC clock
			 */
			enable_mclk();
			set_mckr( AT91C_PMC_PRES_CLK, AT91C_PMC_CSS_MAIN_CLK );

			// Turn off the PLL
			disable_pll();

			break;

		case SPEED_HALF:
			/*
			 *  20 MHz PLL, used in case of low battery or user request
			 */

		case SPEED_HIGH:
			/*
			 *  37 MHz PLL, full speed
			 */
			if ( ( AT91C_BASE_PMC->PMC_MCKR & AT91C_PMC_CSS ) == AT91C_PMC_CSS_PLL_CLK ) {
				// Intermediate switch to main clock
				enable_mclk();
				set_mckr( AT91C_PMC_PRES_CLK, AT91C_PMC_CSS_MAIN_CLK );
			}
			if ( speed == SPEED_HALF ) {
				// Initialise PLL at 20MHz
				if ( Xtal ) {
					pll = CKGR_PLL | PLLCOUNT | ( PLLMUL_HALF << 16 ) | PLLDIV;
				}
				else {
					pll = CKGR_PLL | PLLCOUNT | ( PLLMUL_HALF_NX << 16 ) | PLLDIV;
				}
			}
			else {
				// Initialise PLL at 37MHz
				if ( Xtal ) {
					pll = CKGR_PLL | PLLCOUNT | ( PLLMUL_FULL << 16 ) | PLLDIV;
				}
				else {
					pll = CKGR_PLL | PLLCOUNT | ( PLLMUL_FULL_NX << 16 ) | PLLDIV;
				}
			}
			AT91C_BASE_PMC->PMC_PLLR = pll;
			while ( ( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK ) == 0 );

			// Switch to PLL
			set_mckr( AT91C_PMC_PRES_CLK, AT91C_PMC_CSS_PLL_CLK );

			// Turn off main clock
			disable_mclk();

			break;
		}

		if ( speed < SPEED_HIGH ) {
			/*
			 *  We can reduce the core voltage to save power
			 */
			SUPC_SetVoltageOutput( SUPC_VDD_155 );

			/*
			 *  No wait states for flash reads needed
			 */
			AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_0FWS;
		}

		/*
		 *  Store new settings
		 */
		SpeedSetting = speed;
		ClockSpeed = speeds[ speed ];
		unlock();
	}

idle_check:
	if ( speed == SPEED_IDLE ) {
		/*
		 *  Save power
		 */
		if ( !Running ) {
			GoFast = 0;
		}
		go_idle();
	}
}


/*
 *  The 100ms heartbeat, called from the SLCDC end frame interrupt
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

	if ( Pause ) {
		/*
		 *  The PSE handler checks this value
		 */
	        if ( --Pause == 0 && Running ) {
			set_speed( SPEED_HIGH );
	        }
	}

	/*
	 *  Allow keyboard timeout handling.
	 *  Handles Auto Power Down (APD)
	 */
	if ( Keyticks < APD_TICKS ) {
		++Keyticks;
	}

	/*
	 *  Put a dummy key code in buffer to wake up application
	 */
        put_key( K_HEARTBEAT );
}


/*
 *  The SLCD ENDFRAME interrupt
 *  Called every 640 slow clocks (about 51 Hz)
 */
void LCD_interrupt( void )
{
	InIrq = 1;
	/*
	 *  Set speed to a minimum of 2 MHz for all irq handling.
	 */
	int speed = SPEED_MEDIUM;

	if ( GoFast ) {
		/*
		 *  We are doing serious work
		 */
		if ( ++GoFast == 5 ) {
			speed = SPEED_HALF;
		}
		else if ( GoFast == 10 ) {
			speed = SPEED_HIGH;
			GoFast = 0;
		}
	}

	if ( SpeedSetting < speed ) {
		set_speed( speed );
	}

	/*
	 *  Wait for LCD controller to copy the user buffer to the display buffer.
	 *  Then turn off the automatic update.
	 *  WaitForLcd is set to 1 by finish_display()
	 */
	if ( WaitForLcd ) {
		if ( ++WaitForLcd == 3 ) {
			SLCDC_SetDisplayMode( AT91C_SLCDC_DISPMODE_LOAD_ONLY );
		}
		else if ( WaitForLcd == 4 ) {
			WaitForLcd = 0;
		}
	}

	/*
	 *  The keyboard is scanned every 20ms for debounce and repeat
	 */
	scan_keyboard();

	/*
	 *  Check if a serial transfer has to be interrupted
	 */
	if ( SerialOn && OnKeyPressed ) {
		byte_received( R_BREAK );
	}

	Heartbeats = ( Heartbeats + 1 ) & 0x7f;
	if ( Heartbeats & 1 ) {
		/*
		 *  Voltage detection state machine
		 */
		detect_voltage();

		/*
		 *  Fade out the display
		 */
		if ( LcdFadeOut ) {
			SUPC_SetSlcdVoltage( --LcdFadeOut );
		}
	}

	/*
	 *  Count down to next 100ms user heart beat
	 */
	if ( --UserHeartbeatCountDown == 0 ) {
		/*
		 *  Service the 100ms user heart beat
		 */
		user_heartbeat();

		if ( Xtal ) {
			/*
			 *  Schedule the next one so that there are 50 calls in 5 seconds
			 *  We need to skip 3 ticks in 128 cycles
			 */
			if ( Heartbeats ==  40 || Heartbeats ==  81 || Heartbeats == 122 ) {
				UserHeartbeatCountDown = 6;
			}
			else {
				UserHeartbeatCountDown = 5;
			}
		}
		else {
			/*
			 *  Without a crystal a less accurate timing is good enough
			 */
			UserHeartbeatCountDown = Heartbeats & 1 ? 4 : 5;
		}
	}

	InIrq = 0;
}


/*
 *  The SLCDC interrupt handler
 */
void SLCD_irq( void )
{
	SUPC_DisableDeepMode();

	if ( SLCDC_GetInterruptStatus() & AT91C_SLCDC_ENDFRAME ) {
		/*
		 *  SLCDC ENDFRAME
		 */
		LCD_interrupt();
	}
}


#ifdef USE_SYSTEM_IRQ
/*
 *  The DBGU interrupt
 */
void DBGU_irq( void )
{
	// Test receiver
	if ( ( AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_RXRDY ) ) {
		int c = AT91C_BASE_DBGU->DBGU_RHR;
		if ( AT91C_BASE_DBGU->DBGU_CSR & 0xE0 ) {
			// receiver error
			c = R_ERROR;
			AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RSTSTA;
		}
		byte_received( c );
	}

	// If transmitter is ready disable its interrupt
	if ( ( AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXRDY ) ) {
		AT91C_BASE_DBGU->DBGU_IDR = AT91C_US_TXRDY;
	}
}


/*
 *  The system interrupt handler
 */
void system_irq( void )
{
	SUPC_DisableDeepMode();

	/*
	 *  Since all system peripherals are tied to the same IRQ source 1
	 *  we need to check, for the source of the interrupt
	 */

	// This part checks for itself
	DBGU_irq();
}
#endif


/*
 *  Enable the interrupt sources
 */
void enable_interrupts()
{
	InIrq = 0;
	int prio = 7;

#ifdef USE_SYSTEM_IRQ
	/*
	 *  Tell the interrupt controller where to go
	 *  This is for all system peripherals
	 */
	AIC_ConfigureIT( AT91C_ID_SYS, 
		         AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | prio,
		         system_irq );
	/*
	 *  Enable IRQ 1
	 */
	AIC_EnableIT( AT91C_ID_SYS );
	--prio;
#endif

	/*
	 *  Tell the interrupt controller where to go
	 *  This is for the SLCDC
	 */
	AIC_ConfigureIT( AT91C_ID_SLCD,
		         AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | prio,
		         SLCD_irq );
	/*
	 *  Enable IRQ 11
	 */
	AIC_EnableIT( AT91C_ID_SLCD );

	/*
	 *  Use the LCD frame rate for speed independent interrupt timing
	 */
	SLCDC_EnableInterrupts( AT91C_SLCDC_ENDFRAME );
	UserHeartbeatCountDown = 5;
	// --prio;
}


/*
 *  Open the DBGU port for transmission.
 *  The DBGU port is restricted in that it only supports 8 bits, 1 stop bit.
 *  7 bits is rejected, the number of stop bits is ignored.
 *  A non zero return indicates an error.
 */
int open_port( int baud, int bits, int parity, int stopbits )
{
	int mode, div;

#ifdef SLOW_SERIAL
	set_speed( SPEED_HALF );
#endif

	// Assign I/O pins to DBGU, disable pull-ups
	AT91C_BASE_PIOC->PIO_PDR   = DBGU_PIOC_MASK;
	AT91C_BASE_PIOC->PIO_ASR   = DBGU_PIOC_MASK;
	AT91C_BASE_PIOC->PIO_PPUDR = DBGU_PIOC_MASK;

	// Reset & disable receiver and transmitter, disable interrupts
	AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RSTSTA;
	AT91C_BASE_DBGU->DBGU_IDR = 0xFFFFFFFF;

	// The port provides only 8bit support
	if ( bits != 8 ) return 1;
	mode = parity == 'E' ? AT91C_US_PAR_EVEN
	     : parity == 'O' ? AT91C_US_PAR_ODD
	     : AT91C_US_PAR_NONE;
	div = ClockSpeed / ( baud * 16 );
	if ( div <= 1 ) return 1;

	// Configure baud rate
	AT91C_BASE_DBGU->DBGU_BRGR = div;

	// Configure mode register
	AT91C_BASE_DBGU->DBGU_MR = mode;

	// Enable RX interrupt
	AT91C_BASE_DBGU->DBGU_IER = AT91C_US_RXRDY;

	// Enable receiver and transmitter
	AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RXEN | AT91C_US_TXEN;

	return 0;
}


/*
 *  Close the DBGU port after transmission is complete
 */
extern void close_port( void )
{
	// Disable receiver and transmitter, disable interrupts
	AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RXDIS | AT91C_US_TXDIS;
	AT91C_BASE_DBGU->DBGU_IDR = 0xFFFFFFFF;
}


/*
 *  Output a single byte to the serial
 */
void put_byte( unsigned char byte )
{
	// Wait for the transmitter to be ready
	while ( ( AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXRDY ) == 0 && !OnKeyPressed ) {
		go_idle();
	}
	if ( OnKeyPressed ) {
		return;
	}

	// Send character
	AT91C_BASE_DBGU->DBGU_THR = byte;

	// Enable transmitter interrupt
	AT91C_BASE_DBGU->DBGU_IER = AT91C_US_TXRDY;
}


/*
 *  Force all remaining characters to be sent out
 */
void flush_comm( void )
{
	while ( ( AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXEMPTY ) == 0 && !OnKeyPressed ) {
		go_idle();
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
 *  Check if a shift key is held down. The result is one of SHIFT_F, _G, _H.
 */
enum shifts shift_down(void)
{
	const int map = (int) KbData;
	return map & ( 0x400 << SHIFT_H ) ? SHIFT_H
	     : map & ( 0x400 << SHIFT_G ) ? SHIFT_G
	     : map & ( 0x400 << SHIFT_F ) ? SHIFT_F
	     : SHIFT_N;
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
	k =  KeyBuffer[ (int) KbRead ];
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
	KeyBuffer[ (int) KbWrite ] = (unsigned char) k;
	KbWrite = ( KbWrite + 1 ) & KEY_BUFF_MASK;
	++KbCount;
	unlock();
	return KbCount;
}


/*
 *  Serve the watch dog
 */
#undef watchdog
void watchdog( void )
{
#ifndef NOWD
#ifdef DEBUG_MAIN
	if ( WdDisable ) {
		return;
	}
#endif
	AT91C_BASE_WDTC->WDTC_WDCR = 0xA5000001;
#endif
}


/*
 *  Update the processor speed.
 *  This will honour the speed setting in UState.
 */
#undef update_speed
void update_speed( int full )
{
	set_speed( full ? SPEED_HIGH : SPEED_HALF );
}


/*
 *  Go idle to save power.
 *  Called from a busy loop waiting for an interrupt to do something.
 *  The original speed is restored.
 */
#undef idle
void idle( void )
{
#if 0
	int old_speed = SpeedSetting;
	set_speed( SPEED_IDLE );
	set_speed( old_speed );
#else
	go_idle();
#endif
}



#ifndef XTAL
/*
 *  Turn on crystal oscillator for better clock accuracy.
 *  Crystal must be installed, of course!
 */
void turn_on_crystal( void )
{
	message( "Wait...", "" );
	AT91C_BASE_SUPC->SUPC_CR = (0xA5 << 24) | AT91C_SUPC_XTALSEL;
	while ( ( AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_OSCSEL ) != AT91C_SUPC_OSCSEL );
	Xtal = 1;
	DispMsg = "OK";
	display();
}
#endif


/*
 *  Is debugger active ?
 *  The flag is set via ON+D
 */
#undef is_debug
int is_debug( void )
{
	return DebugFlag == 0xA5;
}


void toggle_debug( void )
{
	DebugFlag = is_debug() ? 0 : 0xA5;
#ifdef SLEEP_ANNUNCIATOR
	SleepAnnunciatorOn = is_debug() || SerialOn;
	dot( SLEEP_ANNUNCIATOR, SleepAnnunciatorOn );
#endif
	message( is_debug() ? "Debug ON" : "Debug\006OFF", NULL );
}


/*
 *  Is test mode active ?
 *  The flag is set via ON+1
 */
#undef is_test_mode
int is_test_mode( void )
{
	return TestFlag;
}


void toggle_test_mode( void )
{
	TestFlag = !TestFlag;
	message( is_test_mode() ? "Test ON" : "Test OFF", NULL );
}


#if 0
/*
 *  For debugging
 */
void show_keyticks(void)
{
	char buffer[ 6 ];
	int i = 5;
	int d, r;

	r = Keyticks;
	buffer[ i ] = '\0';
	while ( r != 0 ) {
		d = r % 10;
		r = r / 10;
		buffer[ --i ] = d + '0';
	}
	message( buffer + i, NULL );
}
#else
#define show_keyticks()
#endif


/*
 *  Main program as seen from C
 */
NO_RETURN int main(void)
{
	char confirm_counter = 0;
	char last_key_combo = 0;

#ifdef SHORT_POINTERS
	// Dummy access for optimiser
	xcopy( (void *) &command_info, &command_info, 0 );
#endif
#ifndef XTAL
	/*
	 *  Timing is dependent on the presence of a crystal
	 */
	Xtal = ( ( AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_OSCSEL ) == AT91C_SUPC_OSCSEL );
#endif
#ifdef STACK_DEBUG
	/*
	 *  Fill RAM with 0x5A for debugging
	 */
	xset( (void *) 0x200200, 0x5A, 0x800 );
#endif

#ifdef ALLOW_DEEP_SLEEP
	/*
	 *  Initialisation depends on the wake up reason
	 */
	if ( State.deep_sleep && Crc == MAGIC_MARKER ) {
		/*
		 *  Return from deep sleep
		 *  RTC and LCD are still running
		 */
		unsigned char minute, second;
		int elapsed_time;

		State.deep_sleep = 0;

		/*
		 *  We've used SLCD memory to save some state, restore it.
		 */
		save_state_to_lcd_memory( 0 );
		State2.invalid_disp = 1;
		ShowRegister = regX_idx;

		/*
		 *  Setup hardware
		 */
		RTC_SetTimeAlarm( NULL, NULL, NULL );

		/*
		 *  How long have we slept?
		 */
		RTC_GetTime( NULL, &minute, &second );
		elapsed_time = (int) minute * 60 + second - LastActiveSecond;
		if ( elapsed_time < 0 ) {
			elapsed_time += 3600;
		}

		if ( AT91C_BASE_SUPC->SUPC_SR & ( AT91C_SUPC_FWUPS | AT91C_SUPC_WKUPS ) ) {
			/*
			 *  Do an initial keyboard scan since this was a keyboard wake up
			 */
			scan_keyboard();
		}

		/*
		 *  Update Ticker and friends
		 */
		elapsed_time *= 10;
		Ticker += elapsed_time;
		if ( (int) Keyticks + elapsed_time >= APD_TICKS ) {
			/*
			 *  APD timeout
			 */
			Keyticks = APD_TICKS;
		}
		else {
			Keyticks += elapsed_time;
		}
		StartupTicks = 10; // Allow power off and on key detection
		show_keyticks();
	}
	else
#endif
	{
		/*
		 *  Initialise the software on power on.
		 *  CRC checking the RAM is a bit slow so we speed up.
		 *  Idling will slow us down again.
		 */
		InIrq = 1;		// disable lock/unlock
		set_speed( SPEED_HALF );

		/*
		 *  Turn on LCD, RTC and backup SRAM
		 */
		SUPC_EnableRtc();
		enable_lcd();
		SUPC_EnableSram();

		/*
		 *  Initialise software
		 */
		if ( Crc != MAGIC_MARKER ) {
			/*
			 *  Will perform a full init if checksum is bad
			 */
			if ( init_34s() ) {
				if ( checksum_backup() ) {
					// No valid backup, create an empty one
					const char *p = DispMsg;
					flash_backup( OP_SAVE );
					DispMsg = p;
				}
				else {
					// restore recent backup after power on clear
					flash_restore( OP_LOAD );
				}
			}
			init_library();
		}
		else {
			/*
			 *  Assume a reset, try to protect memory
			 */
			init_state();
			DispMsg = "Reset";
		}

		/*
		 *  Initialise APD
		 */
		Keyticks = 0;
	}

	/*
	 *  Minimum initialisation for decNumber library and RAM pointers
	 */
	// xeq_init_contexts();

	/*
	 *  Start periodic interrupts
	 */
	BodThreshold = -1;
	detect_voltage();
	enable_interrupts();

	if ( !State2.invalid_disp ) {
		// This is not a wakeup from deep sleep but a power on
		// We need to refresh the display.
		display();
	}

#ifdef SLEEP_ANNUNCIATOR
	SleepAnnunciatorOn = is_debug();
	dot( SLEEP_ANNUNCIATOR, SleepAnnunciatorOn );
	finish_display();
#endif

#ifdef INCLUDE_STOPWATCH
		StopWatchRunning = 0;
#endif
	/*
	 *  Wait for event and execute it
	 */
	while( 1 ) {
        int k;

		/*
		 *  Adjust the display contrast if it has been changed
		 */
		if ( UState.contrast != Contrast ) {
			SUPC_SetSlcdVoltage( Contrast = UState.contrast );
		}

		while ( !is_key_pressed() ) {
			/*
			 *  Save power if nothing in queue
			 */
#ifdef ALLOW_DEEP_SLEEP
			/*
			 *  Test if we can turn ourself completely off
			 */
			if ( !is_debug() && !Running && !SerialOn
#ifdef INCLUDE_STOPWATCH
			     && KeyCallback == NULL
#endif
			     && KbData == 0LL && Pause == 0 && StartupTicks >= 10
			     && Keyticks >= TICKS_BEFORE_DEEP_SLEEP
			     && Keyticks < APD_TICKS )
			{
				/*
				 *  Yes, goto deep sleep. Will never return.
				 */
#ifdef SLEEP_ANNUNCIATOR
				if ( SleepAnnunciatorOn ) {
					/*
					 *  "Alive" indicator off
					 */
					dot( SLEEP_ANNUNCIATOR, 0 );
					SleepAnnunciatorOn = 0;
				}
#endif
				dot( RPN, ShowRPN != 0 );	// might still be off
				show_keyticks();		// debugging
				finish_display();
				while ( WaitForLcd || KbData != 0 ) {
					/*
					 *  We have to wait for the LCD to update.
					 *  While we wait, the user might have pressed a key.
					 */
					go_idle();
					if ( is_key_pressed() ) {
						goto key_pressed;
					}
				}
#ifdef INCLUDE_STOPWATCH
				if(StopWatchRunning)
				{
					set_speed( SPEED_IDLE );
				}
				else
#endif
				{
					deep_sleep();
				}
			}
#endif
			/*
			 *  Normal idle mode. Each interrupt wakes us up.
			 */
			set_speed( SPEED_IDLE );
		}

		/*
		 *  Read out the keyboard queue
		 */
	key_pressed:
		k = get_key();
#ifdef INCLUDE_STOPWATCH
		if(KeyCallback!=NULL) {
			k=(*KeyCallback)(k);
		}
#ifndef CONSOLE
		else if(StopWatchRunning && (Ticker % 5)==0) {
			dot(LIT_EQ, !is_dot(LIT_EQ));
			finish_display();
		}
#endif
		if ( k !=-1 && k != K_HEARTBEAT ) {
#else
		if ( k != K_HEARTBEAT ) {
#endif
			/*
			 *  A real key was pressed or released
			 */
#ifdef SPEEDUP_ON_KEY_WAITING
			if ( KbCount && SpeedSetting < SPEED_HALF ) {
				set_speed( SPEED_HALF );
			}
#endif
			if ( !OnKeyPressed && confirm_counter == 1 ) {
				// ON key was released
				confirm_counter = 0;
				DispMsg = "Cancelled";
				display();
			}
			if ( OnKeyPressed && k != K60 && !Running ) {
				/*
				 *  Check for special key combinations.
				 *  Critical keys have to be pressed twice.
				 */
				if ( k != K_RELEASE ) {
					if ( k != last_key_combo ) {
						confirm_counter = 1;
						last_key_combo = k;
					}
					else {
						++confirm_counter;
					}
				}
				switch( k ) {

				case K64:
					// ON-"+" Increase contrast
					if ( UState.contrast != 15 ) {
						++UState.contrast;
						message( "+Contrast", NULL );
					}
					confirm_counter = 0;
					break;

				case K54:
					// ON-"-" Decrease contrast
					if ( UState.contrast != 0 ) {
						--UState.contrast;
						message( "-Contrast", NULL );
					}
					confirm_counter = 0;
					break;

				case K24:
					// ON + <-
					if ( confirm_counter == 1 ) {
						message( "Erase?", "ALL" );
					}
					else {
						set_speed( SPEED_HALF );
						Crc = 0;
						init_34s();
						confirm_counter = 0;
						display();
					}
					break;
#if 0
				case K62:
					// ON + . toggle radix mark
					UState.fraccomma = !UState.fraccomma;
					display(); // Update number in display
					message( UState.fraccomma ? "RDX," : "RDX.", NULL );
					confirm_counter = 0;
					break;
#endif
				case K10:
					// ON-STO Backup to flash
					if ( confirm_counter == 1 ) {
						message( "Backup?", "to FLASH" );
					}
					else {
						set_speed( SPEED_HALF );
						flash_backup( OP_SAVE );
						display();
						confirm_counter = 0;
					}
					break;

				case K11:
					// ON-RCL Restore from backup
					if ( confirm_counter == 1 ) {
						message( "Restore?", "FLASH" );
					}
					else {
						set_speed( SPEED_HALF );
						flash_restore( OP_LOAD );
						display();
						confirm_counter = 0;
					}
					break;

				case K03:
					// ON+"D" Toggle Debug flag
					toggle_debug();
					confirm_counter = 0;
					break;

				case K51:
					// ON-1 Toggle test flag
					toggle_test_mode();
					confirm_counter = 0;
					break;

#ifndef XTAL
				case K02:
					// ON+C turn on Crystal
					if ( !Xtal ) {
						if ( confirm_counter == 1 ) {
							message( "Crystal?", "Installed" );
							break;
						}
						else {
							turn_on_crystal();
						}
					}
					confirm_counter = 0;
					break;
#endif

				case K43:
					// ON-"S" SAM-BA boot
					if ( is_debug() ) {
						if ( confirm_counter == 1 ) {
							message( "SAM-BA?", "boot" );
						}
						else {
							sam_ba_boot();
						}
					}
					break;

#ifdef DEBUG_MAIN
				case K42:
					// ON-"R" RESET
					if ( is_debug() ) {
						if ( confirm_counter == 1 ) {
							message( "RESET?", NULL );
						}
						else {
							ForceReset();
						}
					}
					break;

				case K41:
					// ON-"Q" WD-Disable
					if ( is_debug() ) {
						if ( confirm_counter == 1 ) {
							message( "Watchdog?", NULL );
						}
						else {
							WdDisable = 1;
						}
					}
					break;
#endif
		}
				// No further processing
				k = -1;
			}
		}
#if 0
		if ( k == K_RELEASE /* ( k != K_HEARTBEAT && k != -1 ) */ || Running ) {
			/*
			 *  Increase the speed of operation
			 */
			set_speed( SPEED_HIGH );
		}
#endif
		/*
		 *  Take care of the low battery indicator
		 */
		dot( BATTERY, Voltage <= LOW_VOLTAGE );
		if ( SpeedSetting == SPEED_HIGH && Voltage <= LOW_VOLTAGE ) {
			/*
			 *  Reduce speed
			 */
			set_speed( SPEED_HALF );
		}

		/*
		 *  Handle the input
		 */
		if ( k != -1 ) {
			process_keycode( k );
			if ( k != K_HEARTBEAT || JustStopped ) {
				/*
				 *  User has pressed a key or a program has just stopped: Avoid APD.
				 */
				Keyticks = 0;
				confirm_counter = 0;
			}
		}

#ifdef INCLUDE_STOPWATCH
		if ( Voltage <= APD_VOLTAGE || ( !Running && KeyCallback==NULL && Keyticks >= APD_TICKS ) ) {
#else
		if ( Voltage <= APD_VOLTAGE || ( !Running && Keyticks >= APD_TICKS ) ) {
#endif
			/*
			 *  We have a reason to power the device off
			 */
			if ( !is_debug() && StartupTicks >= 10 ) {
				shutdown();
			}
		}
	}
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


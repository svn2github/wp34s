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

/*
 *  setup the perstent RAM
 */
#ifdef __GNUC__
__attribute__((section(".backup")))
#endif
TPersistentRam PersistentRam;


#if 0
/*
 *  The Heartbeat
 */
void HeartbeatThread( void )
{
        while( 1 ) {
                Sleep( 100 );
                ++Ticker;
                if ( State.pause ) {
                        --State.pause;
                }
                if ( ++Keyticks > 1000 ) {
                        Keyticks = 1000;
                }
                AddKey( K_HEARTBEAT, true );  // add only if buffer is empty
        }
}
#endif


// These are called from the application


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
}


/*
 *  Main program as seen from C
 */
int main(void)
{
        /*
                init hardware (LCD, timer, etc.)
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

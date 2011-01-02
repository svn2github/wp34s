#ifndef __HP_KEYBOARD_H__
#define __HP_KEYBOARD_H__
/*
Copyright (c) 2007 Hewlett-Packard development company llc
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the disclaimer below.
//
//  - Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the disclaimer below in the documentation and/or
//  other materials provided with the distribution. Unless a specific license is granted
//  to te licencee
//
//  - This software can only be used on a HP hardware. You are not permited to leaverage
//  any part of this software for products/project that do not work on hardware sold by HP
//
//  HP's name may not be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//  DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY HP "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
//  DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
//  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  ----------------------------------------------------------------------------
file created by Cyrille de Brebisson
*/

//#include "board.h"
//#include "system.h"


/** @defgroup keydriver Keyboard Driver

// the functioning of the keyboard driver is as follow:

// when a keyboard interupt or a cpu starts occurs, scan the keyboard for a key down.
// in this state, normaly NO key was pressed prior (ie: all keys were released).
// at that time, either no key is down in which case the CPU is made to wait for
// a keyboard interupt, or
// a key is pressed. in which case, (after debouncing and ghost key detection)
// the key is placed in the keyboard buffer.
// keyboard interupt is then disabled and a 60ms timer interupt is setup.
// upon that interupt, the keyboard is scanned again and, if there is still
// at least on key down, a new timer interupt is setup, else we go back in the first state.
// the reason for the timer interupt is that once a key is down on one column, we can
// not use a keyboard interupt to detect a new key down on that culumn, so we have
// to resort to pulling.
// hopefully, when detecting that pulling, the main software uses the idle mode
// of the cpu if it has nothing else to do to preserve power...

 * notes: all key codes are 2 decimal digit numbers rc where
 * r is the row and c is the column. therefore 30 is ON,
 * 00 is N and 01 is I....
 * the keyboard driver should allow for as many keys pressed down at the
 * same time as possible to detect (given ghost keys problems)
 * and should allow press one key, press another, release the first and
 * press again "properly" (ie: the sw will see key 1, 2 and 1 again).
 * the driver also handles debouncing...
 *  @{
 */

/*! \fn void Keyboard_Init(void)
    \brief Initializes the keyboard driver for 12c+ hardware
*/
extern void Keyboard_Init(void);


// #defines for interupt and wakup setup...
/*! \def AllKeyboardRows
    \brief A macro that enables all key board rows

    Details
    enables all key board rows
*/
#define AllKeyboardRows 0x0000007f

#define AllKeyboardInputs 0x400fc00 // 0100 0000 0000 1111 1100 0000 0000 

/*! \def AllKeyboardWU
    \brief A macro that takes the argument \a AllKeyboardInputs&0xffff

    Details.
*/
#define AllKeyboardWU 0x1f80 // 1 1111 1000 0000

/** @} */ // end of keydriver group


/** @} */ // end of timer group

extern void cpureset(void); // reset the CPU!

#endif

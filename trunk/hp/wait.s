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


		PROGRAM	?wait
		RSEG	ICODE:CODE:ROOT(2)
		CODE32	

wait:
  SUBS R0, R0, #1
  BNE wait
  BX LR

// clears the stack from begining to current position.
// also returns min(255, freeStackSize/4)
ClearStackAndSize:
  movs r0, #0      // counter of free stack....
  movs r2, #2      // stack pointer, initialized at 0x200000 or begining of stack/ram
  lsls r2, r2, #20
  b csend
csloop:
  ldr r1, [r2]     // read current data...
  cmp r1, #0       // if not 0, found max stack used... go to clear stack until sp...
  bne diff
  adds r0, r0, #1  // increment counter
  adds r2, r2, #4  // increment poiunter
csend:
  cmp r2, sp       // make sure that we are not bottoming out...
  blt csloop       // loop
  b csfinish
diff:              // this is the erase flash to sp loop
  mov r1, #0       // pout 0 in r1
diffloop:  
  str r1, [r2]     // erase current level...
  adds r2, r2, #4  // increment pointer
  cmp r2, sp       // are we at the end?
  blt diffloop     // loop
csfinish:          
  cmp r0, #255    // max to 255...
  bls csreturn
  movs r0, #255
csreturn:
  bx lr            // return

  PUBLIC wait
  PUBLIC ClearStackAndSize
  ENDMOD
  END


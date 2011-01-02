#ifndef __HP_MAIN_H__
#define __HP_MAIN_H__

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

this file is the main "run" file of the project.
it intitialize the calcaultor, wait for keys and calls the
execution engine and displays the results.
it also handle low power modes... and has
conditional test system management..

this file contains:

- subfunction needed by the execution engine for the 12c CPU emulator (nut)

- display update routines...

- key management

- test mode

- the main entry point file and manages the following:
    -# initializing the calcualtor if needed
    -# wait for a key press and go to key execution
    -# manage 5 minute poweroff timer
    -# idle mode
*  @{
*/

extern void DBGUInit(void);

#ifdef VCPP
#define __arm
#define inline __inline
#endif

// defined in dcboperations.s (assembly)
/*! \fn wait(unsigned int time)
 * \brief wait - used for small waits
 * \Details
 * loops time times (ie: 4 cycles per number)
**/
extern /*"C" __arm*/ void wait(unsigned int time);

/** @} */ 
#endif

#ifndef __HP_TIMERS_H__
#define __HP_TIMERS_H__
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

/*! \fn static void StopTimer(int timer)
 *  @ingroup timerdrv
 *\brief  stops a Timer 
 *  Stops Timer 1 by disabling peripheral clock of ATMEL chip
 **/
extern void StopTimer(int timer);

/*! \fn static void EnableTimerInterupt(int time)
 *  @ingroup timerdrv
 *\brief  enable timer0 causing an interupt every time ms calling the TimerInterupt function
    \param time specifies the time duration in milli seconds, for which the timer has to be enabled.
 **/
extern void EnableTimerInterupt(int timer, int time, void(*function)());
extern void DisableTimerInterupt(int timer);
extern void ReprogramTimers(int oldspeed); // reprogram the timers knowing that the CPU speed was changed from oldspeed to currently stroed RealCpuSpeed
extern int AcknoledgeTimerInterupt(int timer); // acknoledge a timer interupt. returns true if the interupt was pending!

#endif

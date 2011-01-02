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

#include "timers.h"
#include "board.h"
#include "main.h"
#include "dbgu.h"

#include "../xeq.h"

#define WAVE 0x8000
#define WAVEUPRCTRIGGER 0x4000
#define TC_CLKS                  0x7
#define TC_CLKS_MCK2             0x0
#define TC_CLKS_MCK8             0x1
#define TC_CLKS_MCK32            0x2
#define TC_CLKS_MCK128           0x3
#define TC_CLKS_MCK1024          0x4

static inline AT91PS_TC GetTimerBase(int a) { return (AT91PS_TC)(((int)AT91C_BASE_TC0)+0x40*a); }
static inline int GetTimerID(int a) { return AT91C_ID_TC0+a; }

/*! \fn static void StopTimer(int timer)
 *  @ingroup timerdrv
 *\brief  stops a Timer 
 *  Stops Timer 1 by disabling peripheral clock of ATMEL chip
 **/
void StopTimer(int timer)
{
  DisableTimerInterupt(timer);
  AT91C_BASE_PMC->PMC_PCDR= 1 << GetTimerID(timer); // disable timer clock
}

// calculate the divider and counter value for a timer assuming that time is in 1000,000th of second
// and taking into account current CPU speed
static void CalculateDividerAndTime(unsigned long long int timer, unsigned int *divider, unsigned int *counter)
{
  *counter= (timer*RealCpuSpeed)>>21;
  // timer is the time in cpu cycles...
  *divider= TC_CLKS_MCK2; if (*counter<65536) return;
  *counter>>= 2; *divider= TC_CLKS_MCK8; if (*counter<65536) return;
  *counter>>= 2; *divider= TC_CLKS_MCK32; if (*counter<65536) return;
  *counter>>= 2; *divider= TC_CLKS_MCK128; if (*counter<65536) return;
  *counter>>= 3; *divider= TC_CLKS_MCK1024; if (*counter>=65536) *counter= 65535; return;
}
/*! \fn static void EnableTimerInterupt(int timer, int time, void(*function)())
 *  @ingroup timerdrv
 *\brief  enable timer causing an interupt every time ms, calling the TimerInterupt function
    \param time specifies the time duration in milli seconds (actually, in 1024th of second, but who is counting), for which the timer has to be enabled.
 **/
void EnableTimerInterupt(int timer, int time, void(*function)())
{
  AT91PS_TC TC_pt= GetTimerBase(timer);
  timer= GetTimerID(timer);
  //* First, enable the clock of the TIMER
  AT91C_BASE_PMC->PMC_PCER= 1 << timer; // enable clock
  //* Disable the clock and the interrupts
  TC_pt->TC_CCR = AT91C_TC_CLKDIS;
  TC_pt->TC_IDR = 0xFFFFFFFF ;
  //* Clear status bit
  int dummy = TC_pt->TC_SR;
  //* Suppress warning variable "dummy" was set but never used
  dummy = dummy;
  //* Set the Mode of the Timer Counter
  //* Open timer in "WAVE mode and will trigger an interrupt on register C
  unsigned int divider, counter;
  CalculateDividerAndTime(time*1024, &divider, &counter);
  TC_pt->TC_CMR = divider | WAVE | WAVEUPRCTRIGGER;
  TC_pt->TC_RC= counter;
  //* configure Timer interrupt
  // Disable the interrupt on the interrupt controller
  AT91C_BASE_AIC->AIC_IDCR = 0x1 <<timer ;
  // Save the interrupt handler routine pointer and the interrupt priority
  AT91C_BASE_AIC->AIC_SVR[timer] = (unsigned int) function ;
  // Store the Source Mode Register
  AT91C_BASE_AIC->AIC_SMR[timer] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | 1  ;
  // Clear the interrupt on the interrupt controller
  AT91C_BASE_AIC->AIC_ICCR = 0x1 << timer;

  TC_pt->TC_IER = AT91C_TC_CPCS;  //  IRQ enable CPC
  AT91C_BASE_AIC->AIC_IECR = 0x1 << timer; // enable IT
  //* Enable the clock and set it to 0
  TC_pt->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
}

void DisableTimerInterupt(int timer)
{
  AT91C_BASE_AIC->AIC_IDCR = AT91C_BASE_AIC->AIC_ICCR = 0x1 << GetTimerID(timer); // disable IT
}

 // reprogram the timers knowing that the CPU speed was changed from old to new
static short const dividertable[5]={2,8,32,128,1024};
static void recalc(AT91PS_TC TC_pt, int old)
{
  if ((TC_pt->TC_SR & AT91C_TC_CLKSTA)==0) return;
  unsigned int divider, counter, val, div;
  val= TC_pt->TC_CV; div=dividertable[TC_pt->TC_CMR&0x7];
  unsigned long long us= (val*div*0x40)/(old/0x800);
  CalculateDividerAndTime(us, &divider, &counter);  
  Printk("reprogram timer\r\n");
  TC_pt->TC_CMR= (TC_pt->TC_CMR&~0x7)|divider;
  AT91C_BASE_TC0->TC_RC= counter;
}
void ReprogramTimers(int oldspeed)
{
  recalc(AT91C_BASE_TC0, oldspeed);
  recalc(AT91C_BASE_TC1, oldspeed);
  recalc(AT91C_BASE_TC2, oldspeed);
}

 // acknoledge a timer interupt. returns true if the interupt was pending!
int AcknoledgeTimerInterupt(int timer)
{
//  unsigned int TimerId= GetTimerID(timer);;
  // acknoledge timer interrupt if needed (even if not!)
  int overflow; //= (AT91C_BASE_AIC->AIC_IPR&(1<<TimerId))!=0;
  overflow= (GetTimerBase(timer)->TC_SR&AT91C_TC_COVFS)!=0;
//  AT91C_BASE_AIC->AIC_ICCR= 1<<TimerId;
  return overflow;
}

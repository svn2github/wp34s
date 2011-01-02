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

// Include Standard files
#include "board.h"
#define RTC_SCR RTC_SCCR

void AT91F_RTC_Open()
{
  // cut power to the RTC to reset it...
//  RTCON_Disable();
//  while( (AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_RTS) ); 
  // now, give power to the RTC!
   AT91C_BASE_SUPC->SUPC_MR = (AT91C_BASE_SUPC->SUPC_MR)|(0xA5UL << 24)|AT91C_SUPC_RTCON;
   while( !(AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_RTS) );
  
  // Set the Real Time Clock Mode Register
   AT91C_BASE_RTC->RTC_CR  &= ~(AT91C_RTC_UPDCAL|AT91C_RTC_UPDTIM|AT91C_RTC_UPDCAL); // flag UPDTIM is active at 0, so we actually enable the clock there and disable the calendar. we do not use the time and cal event selection, so we leave them at a defulat of 0...
   // set 24 hour mode (clear all flags)
   AT91C_BASE_RTC->RTC_MR= 0;
   // Clear status register
   AT91C_BASE_RTC->RTC_SCR = AT91C_RTC_ACKUPD | AT91C_RTC_ALARM | AT91C_RTC_SEC | AT91C_RTC_TIMEV | AT91C_RTC_CALEV;
   // Clear all interrupt
   AT91C_BASE_RTC->RTC_IDR = AT91C_RTC_ACKUPD | AT91C_RTC_ALARM | AT91C_RTC_SEC | AT91C_RTC_TIMEV | AT91C_RTC_CALEV;
}

int AT91F_RTC_AlarmOccured()
{
   int r= (AT91C_BASE_RTC->RTC_SR&AT91C_RTC_ALARM)!=0;
  // Clear status register
   AT91C_BASE_RTC->RTC_SCR = AT91C_RTC_ACKUPD | AT91C_RTC_ALARM | AT91C_RTC_SEC | AT91C_RTC_TIMEV | AT91C_RTC_CALEV;
   // Clear all interrupt
   AT91C_BASE_RTC->RTC_IDR = AT91C_RTC_ACKUPD | AT91C_RTC_ALARM | AT91C_RTC_SEC | AT91C_RTC_TIMEV | AT91C_RTC_CALEV;
   return r;
}

void AT91F_RTC_Close()
{
  AT91C_BASE_SUPC->SUPC_MR = (0xA5UL << 24) | (AT91C_BASE_SUPC->SUPC_MR & ~AT91C_SUPC_RTCON); // BDs
  while ((AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_RTS) == AT91C_SUPC_RTS); // BDs
}

void AT91F_RTC_Set_Time(int sec, int min, int hour)
{
    // Clear status
    AT91C_BASE_RTC->RTC_SCR |= AT91C_RTC_ACKUPD; //Why is it done twice? any reason?
    AT91C_BASE_RTC->RTC_SCR |= AT91C_RTC_ACKUPD;

    // Stop the RTC
    AT91C_BASE_RTC->RTC_CR |= AT91C_RTC_UPDTIM ; //Why is it done twice? any reason?
    AT91C_BASE_RTC->RTC_CR |= AT91C_RTC_UPDTIM ;

    // Bit RTC_ACKUPD
    while((AT91C_BASE_RTC->RTC_SR & AT91C_RTC_ACKUPD) != AT91C_RTC_ACKUPD) {} 

    // Set the Real Time Clock Time Register
    AT91C_BASE_RTC->RTC_TIMR = sec | ((min<<8) & AT91C_RTC_MIN) | ((hour<<16) & AT91C_RTC_HOUR);

    // Restart the RTC
    AT91C_BASE_RTC->RTC_CR &= ~(AT91C_RTC_UPDTIM) ;

}

void AT91F_RTC_Set_Date(int date)
{
    // Clear status
    AT91C_BASE_RTC->RTC_SCR |= AT91C_RTC_ACKUPD; //Why is it done twice? any reason?
    AT91C_BASE_RTC->RTC_SCR |= AT91C_RTC_ACKUPD;

    // Stop the RTC
    AT91C_BASE_RTC->RTC_CR |= AT91C_RTC_UPDCAL ; //Why is it done twice? any reason?
    AT91C_BASE_RTC->RTC_CR |= AT91C_RTC_UPDCAL ;

    // Bit RTC_ACKUPD
    while((AT91C_BASE_RTC->RTC_SR & AT91C_RTC_ACKUPD) != AT91C_RTC_ACKUPD) {} 

    // Set the Real Time Clock Time Register
    AT91C_BASE_RTC->RTC_CALR = date;

    // Restart the RTC
    AT91C_BASE_RTC->RTC_CR &= ~(AT91C_RTC_UPDCAL) ;

}

void AT91F_RTC_Set_Time_Alarm (int sec, int min, int hour,int alarm)
{
     // Set the Real Time Clock Time Alarm Register
    AT91C_BASE_RTC->RTC_TIMALR = sec | ((min<<8) & AT91C_RTC_MIN) | ((hour<<16) & AT91C_RTC_HOUR) | alarm;
}


unsigned int AT91F_RTC_Get_Time(void)
{
    unsigned int time;

    // Return the Real Time Clock Time Register
    time = (AT91C_BASE_RTC->RTC_TIMR);

    while(time != (AT91C_BASE_RTC->RTC_TIMR))
    {
      time = (AT91C_BASE_RTC->RTC_TIMR);
    }

    return time;
}

unsigned int AT91F_RTC_Get_Date(void)
{
    unsigned int d;

    // Return the Real Time Clock Time Register
    d = (AT91C_BASE_RTC->RTC_CALR);

    while(d != (AT91C_BASE_RTC->RTC_CALR))
    {
      d = (AT91C_BASE_RTC->RTC_CALR);
    }

    return d;
}

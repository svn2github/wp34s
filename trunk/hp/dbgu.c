//  ----------------------------------------------------------------------------
//          ATMEL Microcontroller Software Support  -  ROUSSET  -
//  ----------------------------------------------------------------------------
//  Copyright (c) 2008, Atmel Corporation
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the disclaimer below.
//
//  Atmel's name may not be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//  DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
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

// Include Standard files
#include "board.h"

extern unsigned int RealCpuSpeed;
extern "C" __arm void wait(int a);
void DBGUInit(void)
{
  // Open PIO for DBGU (ie: let perif A take control of pins)
    AT91C_BASE_PIOC->PIO_ASR= AT91C_PC16_DRXD | AT91C_PC17_DTXD;
    AT91C_BASE_PIOC->PIO_PDR= AT91C_PC16_DRXD | AT91C_PC17_DTXD; // Set in Periph mode

    // Configure DBGU
    //* Disable interrupts
    AT91C_BASE_DBGU->DBGU_IDR = (unsigned int) -1;
    //* Reset receiver and transmitter
    AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS;
    //* Define the baud rate divisor register
    unsigned int baud_value = ((RealCpuSpeed*16)/(115200 * 16));
    if ((baud_value % 16) >= 8)
      baud_value = (baud_value / 16) + 1;
    else
      baud_value /= 16;
    if (baud_value==0) baud_value= 1;
    AT91C_BASE_DBGU->DBGU_BRGR= baud_value;

    //* Clear Transmit and Receive Counters
    AT91C_BASE_DBGU->DBGU_PTCR = AT91C_PDC_RXTDIS|AT91C_PDC_TXTDIS;
    //* Define the USART mode
#define AT91C_US_ASYNC_MODE ( AT91C_US_USMODE_NORMAL + \
                        AT91C_US_NBSTOP_1_BIT + \
                        AT91C_US_PAR_NONE + \
                        AT91C_US_CHRL_8_BITS + \
                        AT91C_US_CLKS_CLOCK )
    AT91C_BASE_DBGU->DBGU_MR = AT91C_US_ASYNC_MODE;

    // Enable Transmitter
    AT91C_BASE_DBGU->DBGU_CR = AT91C_US_TXEN;
    // reset Receiver
    AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RSTRX;
    //* Re-Enable receiver
    AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RXEN;
    wait(200);
}

void Printk(char const *buffer) // \arg pointer to a string ending by \0
{
  DBGUInit();
  wait(1024);
  while(*buffer) 
  {
    while (!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXRDY));
    AT91C_BASE_DBGU->DBGU_THR = *buffer++;
  }
  while (!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_ENDTX));
  wait(1024);
}
void Printks(char const *buffer, int size) // \arg : pointer to buffer, number of bytes to be transmitted
{
  DBGUInit();
  wait(1024);
  while(size--) 
  {
    while (!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXRDY));
    AT91C_BASE_DBGU->DBGU_THR = *buffer++;
  }
  while (!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_ENDTX));
  wait(1024);
}

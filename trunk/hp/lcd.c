//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : dbgu.c
//* Object              : DBGU routines written in C
//* Creation            : JG   16/Aug/2004
//*----------------------------------------------------------------------------

// Include Standard files
#include "board.h"
#include "lcd.h"
#include <string.h>

// disable the LCD drivers...
void Lcd_Disable()
{
  AT91C_BASE_SLCDC->SLCDC_CR = AT91C_SLCDC_LCDDIS; // BDs
  AT91C_BASE_SUPC->SUPC_MR = (0xA5UL << 24) | (AT91C_BASE_SUPC->SUPC_MR & ~AT91C_SUPC_LCDMODE); // BDs
  while ((AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_LCDS) == AT91C_SUPC_LCDS); // BDs
}

#define COMMON_NUMBER   9<<0   //10 commons
#define SEGMENT_NUMBER  39<<8  //40 segments
void Lcd_Enable(int contrast)
{
  while (1)
  {
    // turn LCD on at selected contrast...
    AT91C_BASE_SUPC->SUPC_MR = (AT91C_BASE_SUPC->SUPC_MR&0xFFFFFFC0)|(0xA5UL << 24)|AT91C_SUPC_LCDMODE_INTERNAL|(contrast&0xf);
    if (AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_LCDS) break;
    memset((void*)AT91C_SLCDC_MEM, 0, 10*2*4); // erase screen if it was OFF before!!!
  }

  if (AT91C_BASE_SLCDC -> SLCDC_SR == AT91C_SLCDC_ENA) return; // if LCD is already turned ON, quick exit...
  AT91C_BASE_SLCDC -> SLCDC_CR = AT91C_SLCDC_SWRST;
  AT91C_BASE_SLCDC -> SLCDC_MR = COMMON_NUMBER | AT91C_SLCDC_BUFTIME_8_Tsclk | AT91C_SLCDC_BIAS_1_3 | SEGMENT_NUMBER;
  AT91C_BASE_SLCDC -> SLCDC_FRR = AT91C_SLCDC_PRESC_SCLK_16 | AT91C_SLCDC_DIV_2;
  AT91C_BASE_SLCDC -> SLCDC_CR = AT91C_SLCDC_LCDEN;
  //while (pSLCDC -> SLCDC_SR != AT91C_SLCDC_ENA); //TODO: put back...
}

void LcdBlink(int blink)
{
  if(blink)
    AT91C_BASE_SLCDC -> SLCDC_DR = AT91C_SLCDC_DISPMODE_BLINK|(64 <<  8);
  else
    AT91C_BASE_SLCDC -> SLCDC_DR = AT91C_SLCDC_DISPMODE_NORMAL|(64 <<  8);
}


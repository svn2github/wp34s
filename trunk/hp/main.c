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

#include "board.h"
#include "main.h"
#include "lcd.h"
#include "keyboard.h"
#include "rtc.h"
//#include "application.h"
//#include "graphics.h"
#include "timers.h"
#include "dbgu.h"

#include "../xeq.h"
#include "../display.h"

#define Wait(a) wait(a*(RealCpuSpeed/1024)/4)

static void SetCPUSpeed(int freq);
static void Watchdog() { AT91C_BASE_WDTC->WDTC_WDCR= 0xA5000001; }
static void UpdateRS232() { }

//static const int SlowClock= 32768; // slow clock speed, for the moment, 11khz, but will move to 32Khz in the new chips
int SlowClock;    // slow clock speed ~32Khz
int CpuSpeed;     // current CPU speed
int BaseCpuSpeed; // internal RC speed, mesured using oscilator
int RealCpuSpeed; // real speed the CPU is set at, canbe different from the "freq" parametter due to discreete frequency range

// add hms times in dcb format with 24 hour modulo
// used to calculate next 5 minute wakup...
static int hmsadd(int h1, int h2)
{
  int r=0;
  // seconds
  r= (h1&0xf) + (h2&0xf);
  if (r >= 0xA) r= r - 0xA + 0x10;
  r+= (h1&0xf0) + (h2&0xf0);
  if (r >= 0x60) r= r - 0x60 + 0x100;
  //minutes
  r+= (h1&0xf00) + (h2&0xf00);
  if (r >= 0xA00) r= r - 0xA00 + 0x1000;
  r+= (h1&0xf000) + (h2&0xf000);
  if (r >= 0x6000) r= r - 0x6000 + 0x10000;
  // hours...
  r+= (h1&0xf0000) + (h2&0xf0000);
  if (r >= 0xA0000) r= r - 0xA0000 + 0x100000;
  r+= (h1&0xf00000) + (h2&0xf00000);
  if (r >= 0x240000) r= r - 0x240000;

  return r;
}

// setup RTC timer 5 minutes past now....
// used for auto power off...
// time is given in hms format encoded in hex: 0x12141 is 1 hour, 21 minutes, 41 seconds...
void SetRTCAlarm(unsigned int time)
{
  AT91F_RTC_Close(); // need else you get issues with the open
  AT91F_RTC_Open(); // setup RTC
  // Program RTC Wake Up at 5 minutes...
  Printk("set RTC alarm in 5mns \r\n");
  unsigned long int t=AT91F_RTC_Get_Time();

  unsigned long int t2= hmsadd(t, time); // add 5 minutes to time... for alarm

  AT91F_RTC_Set_Time_Alarm(t2&0xff, (t2>>8)&0xff, (t2>>16)&0xff, AT91C_RTC_SECEN|AT91C_RTC_MINEN|AT91C_RTC_HOUREN);
}


//************************ low power functions (ie: turn cpu off) **************
// calc in low power mode,
// screen and everything off (except RAM),
// wait for ON key only...
static void TurnCalcOff()
{
  SetCPUSpeed(BaseCpuSpeed);
  AT91C_BASE_AIC->AIC_IDCR= ~0; // disable all interrupts...

  Printk("calc OFF, wait FWUP\r\n");
  // turn off LCD (can be needed in case of very low battery)
  Lcd_Disable();

  int i = 0;
  while (i < 1000) { Watchdog(); if ((AT91C_BASE_SUPC->SUPC_SR & (1 << 12)) == 0) i = 0; else i++; }
  
  // stop BOD
  AT91C_BASE_SUPC->SUPC_BOMR = 0;

  // kill all timers
  StopTimer(0);
  StopTimer(1);
  StopTimer(2);

  // put all io lines inputs and pullups... and stops keyboard from scanning...
//  Printk("config PIOC\r\n");
  Printk("enter off mode\r\n");
//  AT91F_PIO_CfgInput(AT91C_BASE_PIOC, 0xffffffff);
//  AT91F_PIO_CfgPullup(AT91C_BASE_PIOC, 0xffffffff);
  AT91C_BASE_PIOC->PIO_PER= AT91C_BASE_PIOC->PIO_ODR= AT91C_BASE_PIOC->PIO_PPUER= 0xffffffff; // input, pullup no filter
  Wait(1);
  AT91C_BASE_PMC->PMC_PCDR= 1 << AT91C_ID_PIOC; // disable PIOC clock

  // Enable SRAM Backup
  AT91C_BASE_SUPC->SUPC_MR = (AT91C_BASE_SUPC->SUPC_MR)|(0xA5UL << 24)|AT91C_SUPC_SRAMON;

  Printk("RTC off\r\n");
  AT91F_RTC_Close();
  AT91C_BASE_SUPC->SUPC_WUMR = (AT91C_BASE_SUPC->SUPC_WUMR)&(~AT91C_SUPC_RTCEN); // disable RTC wakup

  Printk("Wakup src\r\n");
  // Program Wake-Up Source debouncer
  AT91C_BASE_SUPC->SUPC_WUMR = (AT91C_BASE_SUPC->SUPC_WUMR)& ~(0x7 <<12);
  AT91C_BASE_SUPC->SUPC_WUMR = (AT91C_BASE_SUPC->SUPC_WUMR)& ~(0x7 <<8);
  // enable ON key
  AT91C_BASE_SUPC->SUPC_WUMR = (AT91C_BASE_SUPC->SUPC_WUMR)|AT91C_SUPC_FWUPEN;
  // disable wakup on anything else...
  AT91C_BASE_SUPC->SUPC_WUIR = (AT91C_BASE_SUPC->SUPC_WUIR)&(~0xffff);

  // Enter Backup Mode
  AT91C_BASE_SUPC->SUPC_CR = (0xA5UL << 24)|AT91C_SUPC_VROFF;
  for (;;);
}

// calc in low power mode,
// everything is off except RAM, screen, RTC (for 5 minutes auto power off) and keyboard,
// wait for any key or RTC timer...
void PutCalcIdle()
{
  AT91C_BASE_AIC->AIC_IDCR= ~0; // disable all interrupts...
  Printk("calc idle, wait for kbd or RTC\r\n");
  // stop BOD
  AT91C_BASE_SUPC->SUPC_BOMR = 0;
  // kill all timers
  StopTimer(0);
  StopTimer(1);
  StopTimer(2);

  // Enable SRAM Backup
  AT91C_BASE_SUPC->SUPC_MR = (AT91C_BASE_SUPC->SUPC_MR)|(0xA5UL << 24)|AT91C_SUPC_SRAMON;

  // make sure we wakeup with RTC...
  AT91C_BASE_SUPC->SUPC_WUMR = (AT91C_BASE_SUPC->SUPC_WUMR)|AT91C_SUPC_RTCEN; 

  // Program Wake-Up Source debouncer
  AT91C_BASE_SUPC->SUPC_WUMR = (AT91C_BASE_SUPC->SUPC_WUMR)& ~(0x7 <<12);
  // enable FWUP/ON key
  AT91C_BASE_SUPC->SUPC_WUMR = (AT91C_BASE_SUPC->SUPC_WUMR)|AT91C_SUPC_FWUPEN;
  // enable wakup on high to low on all keys...
  AT91C_BASE_SUPC->SUPC_WUIR = AllKeyboardWU;

  Printk("bye\r\n");

  // Enter Backup Mode
  AT91C_BASE_SUPC->SUPC_CR = (0xA5 << 24)|AT91C_SUPC_VROFF;
  for (;;);
}


static void SetCPUSpeed(int freq) // set the cpu frequency, input is in Mhz and should be <30Mhz...
                           // frequency of <= 0 reset base frequency.
                           // the minimum frequency is around 40Khz
{
  if (State.testmode) freq= 36000000; // if in test system mode stay at high speed all the time...
  if (freq<=0) freq= BaseCpuSpeed;
  if (freq>36000000) freq= 36000000;
  if (freq==CpuSpeed) return; // if no changes, return...
  int oldRealSpeed= RealCpuSpeed;
  CpuSpeed= freq;
  if (freq>BaseCpuSpeed) // do we need to use the PLL?
  { // yes
    Printk("PLL Clock\r\n");
    int presc= 0;
    while (freq<18000000) { freq*=2; presc++; }
    if (presc>6) presc= 6;
    freq= freq/SlowClock-1;
    if (freq>2047) freq= 2047; // maxes out at 2047
    RealCpuSpeed= (freq*SlowClock)>>presc;
    // set flash wait state (1 ws if > 15Mhz)
    AT91C_BASE_MC->MC_FMR= (AT91C_BASE_MC->MC_FMR&~AT91C_MC_FWS) | ((RealCpuSpeed>15000000)?AT91C_MC_FWS_1FWS:AT91C_MC_FWS_0FWS);
    // program the PLL
    AT91C_BASE_PMC->PMC_PLLR = (0<<29) | AT91C_CKGR_OUT_2 | AT91C_CKGR_PLLCOUNT |
                               (AT91C_CKGR_MUL & (freq << 16)) | (AT91C_CKGR_DIV & 1);
    // Wait for PLL stabilization
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK) );
    // Wait until the master clock is established
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
    // first, select pre-scaler
    AT91C_BASE_PMC->PMC_MCKR = (AT91C_BASE_PMC->PMC_MCKR & ~(7<<2)) | (presc<<2);
    // Wait until the master clock is established
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
    // then Select the Processor clock source as PLL (need to do both operations separately)
    AT91C_BASE_PMC->PMC_MCKR = (AT91C_BASE_PMC->PMC_MCKR&~3) | AT91C_PMC_CSS_PLL_CLK ;
    // Wait until the master clock is established
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
    // disable 2Mhz clock
//    AT91C_BASE_PMC->CKGR_MOR=  (AT91C_BASE_PMC->CKGR_MOR&~((1<<24) | (1<<0))) | AT91C_CKGR_KEY;
//    while ((AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MAINSELS) == AT91C_PMC_MAINSELS);
  } else if (freq>SlowClock) {
    // enable 2Mhz clock
//    AT91C_BASE_PMC->CKGR_MOR=  (AT91C_BASE_PMC->CKGR_MOR&~(1<<24)) | (1<<0) | AT91C_CKGR_KEY;
//    while ((AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MAINSELS) == AT91C_PMC_MAINSELS);
    Printk("2Mhz clock\r\n");
    int presc= 0;
    while (freq<BaseCpuSpeed) { freq*=2; presc++; }
    if (presc>6) presc= 6;
    RealCpuSpeed= BaseCpuSpeed>>presc;

    // Wait until the main clock is established
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
    // first Select the Processor clock source
    AT91C_BASE_PMC->PMC_MCKR = (AT91C_BASE_PMC->PMC_MCKR&~0x3) | AT91C_PMC_CSS_MAIN_CLK ;
    // Wait until the master clock is established
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
    // then, select pre-scaler
    AT91C_BASE_PMC->PMC_MCKR = (AT91C_BASE_PMC->PMC_MCKR & ~(7<<2)) | (presc<<2);
    // Wait until the master clock is established
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
    // disable PLL
    AT91C_BASE_PMC->PMC_PLLR = (0<<29) | AT91C_CKGR_OUT_2 | AT91C_CKGR_PLLCOUNT |
                               (AT91C_CKGR_MUL & 0) | (AT91C_CKGR_DIV & 1);
    // set 0 flash wait state
    AT91C_BASE_MC->MC_FMR= (AT91C_BASE_MC->MC_FMR&~AT91C_MC_FWS) | AT91C_MC_FWS_0FWS;
  } else {
    Printk("Slow Clock\r\n");
    int presc= 0;
    while (freq<SlowClock) { freq*=2; presc++; }
    if (presc>6) presc= 6;
    RealCpuSpeed= SlowClock>>presc;

    // first Select the Processor clock source
    AT91C_BASE_PMC->PMC_MCKR = (AT91C_BASE_PMC->PMC_MCKR&~0x3) | AT91C_PMC_CSS_SLOW_CLK ;
    // Wait until the master clock is established
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
    // then, select pre-scaler
    AT91C_BASE_PMC->PMC_MCKR = (AT91C_BASE_PMC->PMC_MCKR & ~(7<<2)) | (presc<<2);
    // Wait until the master clock is established
    while( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
    // disable PLL
    AT91C_BASE_PMC->PMC_PLLR = (0<<29) | AT91C_CKGR_OUT_2 | AT91C_CKGR_PLLCOUNT |
                               (AT91C_CKGR_MUL & 0) | (AT91C_CKGR_DIV & 1);
    // disable 2Mhz clock
//    AT91C_BASE_PMC->CKGR_MOR=  (AT91C_BASE_PMC->CKGR_MOR&~((1<<24) | (1<<0))) | AT91C_CKGR_KEY;
//    while ((AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MAINSELS) == AT91C_PMC_MAINSELS);
    // set 0 flash wait state
    AT91C_BASE_MC->MC_FMR= (AT91C_BASE_MC->MC_FMR&~AT91C_MC_FWS) | AT91C_MC_FWS_0FWS;
  }
//  debugexec(char s[80]; sprintf(s, "Cpu speed: real:%d required:%d\r\n", RealCpuSpeed, CpuSpeed); Printk(s); );
  ReprogramTimers(oldRealSpeed);
}


/* Board hardware description:
0 is for output followed by default value (0 or 1) followed by PA or PB if pin assigned to a periferial
I is for input followed by Pu or /Pu depending if PullUP is needed or not

O0   PC 0-6 7 keyboard lines
IPu  PC 7 unused
IPu  PC 8 unused
IPu  PC 9 unused
IPu  PC 10 unused
IPu  PC 10 ON key
IPu  PC 11-15,26 Keyboard columns (the gap comes from the fact that these lines must be wakup lines for CPU)
IPu  PC 16 Debug Usart RX, 6 pin connector
IPu  PC 17 Debug Usart TX, 6 pin connector
IPu  PC 18 unused
IPu  PA PC unused
IPu  PC 20 unused
IPu  PC 21 unused
IPu  PC 22 unused
IPu  PC 23 unused
IPu  PC 24 Usart RX, extention connector
IPu  PC 25 Usart TX, extention connector
IPu  PC 27 unused
IPu  PC 28 IO7 on external connector
Ipu  PC 29 unused
*/

void IOInit()
{
  AT91C_BASE_PMC->PMC_PCER= (unsigned int) 1 << AT91C_ID_PIOC; // Enable PIOC Clocking this allows to use inputs!
  AT91C_BASE_PIOC->PIO_PPUDR=  0x0000007f; // disable pullup on outputs and some inputs
  AT91C_BASE_PIOC->PIO_PPUER= ~0x0000007f; // enable pullup where needed
  AT91C_BASE_PIOC->PIO_PER= AT91C_BASE_PIOC->PIO_ODR= 0x0400fc00; // Configure Inputs
  AT91C_BASE_PIOC->PIO_CODR= 0x0000007f; // set appropriate output to 0 (do that before seting the pin as output to avoid glitches)
  AT91C_BASE_PIOC->PIO_SODR= 0x00000000; // set appropriate output to 1
  AT91C_BASE_PIOC->PIO_PER= AT91C_BASE_PIOC->PIO_OER= 0x0000007f; // configure outputs as outputs
  AT91C_BASE_PIOC->PIO_ASR= 0x00000000; AT91C_BASE_PIOC->PIO_PDR= 0x00000000; // give control to Periferial A
}


// returns current VBat in deci volts...
// only test up to max voltage
int TestVoltage(int min, int max)
{
  min= min-19;
  max= max-19;
  if (max>16) max=16;
  if (min<0) min= 0;
  for (; min<=max; min++)
  {
    AT91C_BASE_SUPC->SUPC_BOMR = AT91C_SUPC_BODSMPL_CONTINUOUS|(min);
    Wait(1); // 1ms pause...
    if ((AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_BROWNOUT )!=0) // if lower than current BOD value
    {
      AT91C_BASE_SUPC->SUPC_BOMR = 0;  // stop BOD
      return min+19;
    }
  }
  AT91C_BASE_SUPC->SUPC_BOMR = 0;  // stop BOD
  return 35;
}
//************************ Main function ***************************************

void ServiceBOD(void)
{
  if (AT91C_BASE_SUPC->SUPC_BOMR==0) return;
  if ((AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_BROWNOUT )!=0) // if battery lower than current BOD value
  {
    if ((State.LowPowerCount>>5)<=21-19) // if <= than 2.1V turn calc off
      TurnCalcOff(); 
    else { // counts the number of consecutive readings >= 2.4V and <2.4V, if more than 31 in a row, change the low power flag
      if (State.LowPower)
      {
        if ((State.LowPowerCount>>5)>24-19) State.LowPowerCount++; else State.LowPowerCount= 0;
        if ((State.LowPowerCount&31)>30) { State.LowPower = 0; State.LowPowerCount= 0; }
      } else {
        if ((State.LowPowerCount>>5)<=24-19) State.LowPowerCount++; else State.LowPowerCount= 0;
        if ((State.LowPowerCount&31)>30) { State.LowPower = 1; State.LowPowerCount= 0; }
      }
    }
  } else State.LowPowerCount+= 0x20;
  AT91C_BASE_SUPC->SUPC_BOMR = 0;  // stop BOD
}

int GetKeySpeed(int k)
{
  if (k==2 || k==7 || k==8 || k==12 || k==18 || k==24 || k==40 ) return 36000000;
  else return 5000000;
}

static char const * const MainResetCause[6]= {"Power Up", "Normal WakeUp", "Watchdog", "SW Reset", "NRST pin", "Brown Out"};
int main(void)
{
  IOInit(); // initialize all the IOs as needed...
  
  // calculate CPU speed accuracy is low due to the abscence of xtal, so
  // we are using a constant for the moment...
//#define XTAL // uncomment if you are using an external cristal to handle SlowClock
#ifdef XTAL
  AT91C_BASE_SUPC->SUPC_CR = (0xA5 << 24)|AT91C_SUPC_XTALSEL;
  while ((AT91C_BASE_PMC->PMC_MCFR&(1<<16))==0);
  unsigned int mcfr= AT91C_BASE_PMC->PMC_MCFR;
  SlowClock= 32768;
  BaseCpuSpeed= BaseCpuSpeed= CpuSpeed= (mcfr&0xffff)*SlowClock/16;
#else
  RealCpuSpeed= BaseCpuSpeed= CpuSpeed= 1850000;
  SlowClock= 28500;
#endif

  // print reboot information...
  Printk("\r\n\r\n\r\nBoot: ");
  Printk(MainResetCause[(AT91C_BASE_RSTC->RSTC_RSR>>8)&0x7]);
  if (((AT91C_BASE_RSTC->RSTC_RSR>>8)&0x7)!=1)
  {
    DispMsg = MainResetCause[(AT91C_BASE_RSTC->RSTC_RSR>>8)&0x7];
    display();
    Wait(500);
  }

  SetCPUSpeed(CpuSpeed);

  Keyboard_Init();                        

  // init RS232 if  the testsystem is ON
  if (State.testmode) UpdateRS232();
  
  // setup battery level detection
  AT91C_BASE_SUPC->SUPC_BOMR = AT91C_SUPC_BODSMPL_CONTINUOUS|(State.LowPowerCount>>5);

  // reset of calc (if needed...)
  if (((AT91C_BASE_RSTC->RSTC_RSR>>8)&7)==0) State.magic = 0; // if master reset pressed, force full logical reset
  if (State.magic != MAGIC_MARKER)
  {
    init_34s();
    Lcd_Enable(State.contrast);
  } else   // init LCD
    Lcd_Enable(State.contrast);

#if 0 // To be redone
  if (State.testmode)
  {
    disp_msg = "TST RST!";
    display();
    while (KeyBuffGetKey()==-1) Watchdog();
  }
  
  while (1) // loop endlessly looking for 1 of 3 events: key in buffer, 5 minute timer or calculator turned off
            // or in test mode for a "virtual" key press (or a real one)...
  {
    Watchdog();
    CheckCommunication();
    while (!KeyBuffEmpty()) //if key in buffer, execute it...
    { 
      ClearFlag(MyApplication, VirtualKey);
      StopTimer(2);
      char k=KeyBuffGetKey();
      SendChar(k);
      SetCPUSpeed(36000000);
      process_keycode(k);
      display();
      SetRTCAlarm(0x000500);             // setup 5 minutes timer...
      SetCPUSpeed(GetKeySpeed(k));         // reset to internal oscilator 2mhz...
      ServiceBOD();
    }
    //if in test system mode, loop continously waiting for input, do not go in sleep mode...
    if (State.testmode) continue;
    if (AT91F_RTC_AlarmOccured()) State.off = 1;   // 5 minute timer: auto power off
    if (State.off) TurnCalcOff();   
    if (System.KeyboardMap==0LL) PutCalcIdle(); // if no keys are down, and no scrolling idle the calc waiting for keypress or 5 minute timer...
    // go in wait mode, waiting for timers to tick (key press timers 0 and 1 or scrolling timer 2)
    AT91C_BASE_PMC->PMC_SCDR= 1; // stop CPU clock...
  }
#endif    
  return 0;
}

__attribute__((externally_visible)) void abort( void ) {
    for (;;);
}

void wait( unsigned int s ) {
    while(s--);
}

int is_key_pressed( void ) {
	return 0;
}

__attribute__((section(".backup"))) TPersistentRam PersistentRam;

// Ugly hack to get rid of unwinding code: http://embdev.net/topic/201054
__attribute__((externally_visible)) void __aeabi_unwind_cpp_pr0(void) {};
__attribute__((externally_visible)) void __aeabi_unwind_cpp_pr1(void) {};
__attribute__((externally_visible)) void __aeabi_unwind_cpp_pr2(void) {};

// Test Linker

#if 0
__attribute__((externally_visible)) unsigned char empty_bss_hog[ 2000 ];
__attribute__((externally_visible)) const unsigned char full_data_hog[ 25000 ] = "full_data_hog";
#endif
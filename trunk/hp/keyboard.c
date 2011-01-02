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
#include "keyboard.h"
#include "board.h"
#include "timers.h"
#include "main.h"
#include "dbgu.h"

#include "../xeq.h"
#include "../display.h"

// *****************************************************************************
// keyboard (PIOC) stuff

static void EnableKeyboardInterupt();
static int ScanKeyboard();

static void DisableKeyboardInterupt()
{
  AT91C_BASE_AIC->AIC_IDCR = AT91C_BASE_AIC->AIC_ICCR = 0x1 << AT91C_ID_PIOC; // disable IT
  AT91C_BASE_PIOC->PIO_IDR= AllKeyboardInputs;
}

/*! \fn KeyboardInterupt
 *\brief  Handle keyboard interrupt.
 * acknoledge interrupt whether needed or not.
 * handle keyboard interupt -
 *  -# scan keyboard and add keys in buffer using ScanKeyboard()
 *  -# if keys down, then disable keyboard interrupt and use the timer(EnableTimerInterupt()) to do pooling! every 60ms.
 *  -# if no keys down, then re-enable the keyboard interrupts
 *
 **/

static void TimerInterupt2()
{
  Printk("<TC0 irq");
  AcknoledgeTimerInterupt(0);

  if (ScanKeyboard())
  {
    Printk("<Stop TC0 irq>");
    StopTimer(0);
//    StopTimer(1);
    EnableKeyboardInterupt();
  } else
    EnableTimerInterupt(0, 60, TimerInterupt2);
  Printk(">");
}

static void KeyboardInterupt()
{
  Printk("<Kbd IRQ");
  // acknoledge interrupt if needed (even if not!)
  unsigned int dummy= AT91C_BASE_PIOC->PIO_ISR; dummy= dummy;

  // handle keyboard interupt
  if (!ScanKeyboard()) // scan keyboard and add keys in buffer..
  { // if keys down...
    DisableKeyboardInterupt(); // disable keyboard interrupt as
    EnableTimerInterupt(0, 60, TimerInterupt2);
  } else
    EnableKeyboardInterupt();  // if no keys down, then re-enable the keyboard interrupts...
  Printk(">");
}

/*! \fn EnableKeyboardInterupt
 *\brief  enables keyboard interrupt.
 * -# configure PIO C interrupt handler
 * -# enable interupt generation on all inputs from the keyboard
 * -# set all rows to 0
 * -# enable PIOC interrupts in the AIC controler
 **/
static void EnableKeyboardInterupt()
{
  //* configure PIO C interrupt handler
   // Disable the interrupt on the interrupt controller
   AT91C_BASE_AIC->AIC_IDCR = 0x1 <<AT91C_ID_PIOC ;
   // Save the interrupt handler routine pointer and the interrupt priority
   AT91C_BASE_AIC->AIC_SVR[AT91C_ID_PIOC] = (unsigned int) KeyboardInterupt ;
   // Store the Source Mode Register
   AT91C_BASE_AIC->AIC_SMR[AT91C_ID_PIOC] = AT91C_AIC_SRCTYPE_EXT_LOW_LEVEL | 1  ;
   // Clear the interrupt on the interrupt controller
   AT91C_BASE_AIC->AIC_ICCR = 0x1 << AT91C_ID_PIOC;

  // enable interupt generation on all inputs from the keyboard. disable on the rest...
  AT91C_BASE_PIOC->PIO_IDR= ~AllKeyboardInputs; //InterruptDisable
  AT91C_BASE_PIOC->PIO_IER= AllKeyboardInputs; //Interup enable
  // set all rows to 0
  AT91C_BASE_PIOC->PIO_CODR= AllKeyboardRows; // set output to 0
  // enable PIOC interrupts in the AIC controler
  AT91C_BASE_AIC->AIC_IECR = 0x1 << AT91C_ID_PIOC; // enable IT
}

void Keyboard_Init()
{
  initKeyBuffer(); // init keyboard buffer
  KeyboardInterupt(); // if there is a key press, handle it, else setup keyboard interupt...
}

// called after x ms (max bounce time) to disable bounce checking on the last key pressed
static void ResetLastKeyPress()
{
  Printk("<TC1 IRQ");
  AcknoledgeTimerInterupt(1);
  System.LastKeyPress= -1;
  StopTimer(1);
  Printk(">");
}

/*! \fn ScanKeyboard
 *\brief  scans the keyboard
 * -#  update the key map
 * -#  update LastKeyPress if a new key was pressed
 * -#  debounce stuff
 * -#  and place stuff in the key buffer if it's a new key press..
 * -#  a key consumer program should call GetKey() to read a key from the buffer
 * -#  return true if no key is pressed..
 **/
static int ScanKeyboard()
{
  unsigned long long NewKeyMap= ~0LL;

  // read the 7 lines of the keyboard and create a 64 bit structure
  // 6 bit per line to hold the keyup/down information
  // 1 bit per key...
  AT91C_BASE_PIOC->PIO_ODR= AllKeyboardRows; // set as inputs
  unsigned int t;
  for (int line=6; line>=0; line--)
  {
    AT91C_BASE_PIOC->PIO_OER=  1<<line; // set as output
    wait(100);
    t= AT91C_BASE_PIOC->PIO_PDSR; // aquire the keys...
    t= t&AllKeyboardInputs; t= (t>>11); t= (t|(t>>10))&0x3f; // key states bit 0-5 on the WKUP pins...
    NewKeyMap= (NewKeyMap<<6)+(unsigned long long)t;
    AT91C_BASE_PIOC->PIO_ODR= 1<<line; // back to input
  }
  AT91C_BASE_PIOC->PIO_OER=  AllKeyboardRows; // all outputs...
  // invert the result as we are working with inverted logic
  NewKeyMap= ~NewKeyMap;
  if ((AT91C_BASE_SUPC->SUPC_SR & 0x00001000 )==0) // on key pressed if it is detected down for more than 2ms..
  {
#ifdef HP40b
    Wait(2);
    if ((AT91C_BASE_SUPC->SUPC_SR & 0x00001000 )==0) 
#endif
      NewKeyMap|= OnKeyInKeyMap; // ON Key
  }

  // new key press is the list of the keys that are pressed now, but were not
  // pressed last time...
  unsigned long long NewKeyPress= NewKeyMap & ~System.KeyboardMap;

  // key released are the keys that are not pressed now, but were presssed last time!
  // in this case, we are not using that information because the software
  // does not do anything on key release... so it's just commented out
  // unsigned long long KeyReleased= KeyboardMap & ~NewKeyMap;

  // save the current key map as the key map
  System.KeyboardMap= NewKeyMap;
  
  // update shift indicator...
  DispIndicator(DOWN_ARROW, ((System.KeyboardMap&(1LL<<30))!=0) || GetFlag(MyApplication, shift));

  // handle the case of the ON key. if the ON key is down, handle it first and then look at other keys...
  if ((NewKeyPress&OnKeyInKeyMap)!=0LL)
  {
    AddKeyInBuffer(KEYON); // add on key in buffer
    NewKeyPress= NewKeyPress&~OnKeyInKeyMap; // remove on key from list of new key pressed...
  }

  // is there a new key down?
  if (NewKeyPress==0LL)
  {
    if (System.KeyboardMap!=0LL) return false; // no, just return false if there are still keys down
    int opt= System.ONPlusTask;
    System.ONPlusTask =0;
    if (opt==0) return true;     // test if there is a ON+ task to do
    if (opt==2) { AddKeyInBuffer(KEYONTEST); return true; }       // put special key in buffer
    if (opt==3) MyApplication->Marker= 0; // ON+A+F
    if (opt==1) MyApplication->Marker= MagicMarker-1; 
    cpureset(); // last case, ON+C
  }

  // test system and special system keys testing...
  if ((System.KeyboardMap&OnKeyInKeyMap)!=0LL)
  {
    if (NewKeyPress==Key12InKeyMap) { System.ONPlusTask= 0; return false; } 
    if (NewKeyPress==Key13InKeyMap) { System.ONPlusTask= 1; return false; } 
    if (NewKeyPress==Key14InKeyMap) { System.ONPlusTask= 2; return false; }
    if (System.KeyboardMap==(Key11InKeyMap|Key16InKeyMap|OnKeyInKeyMap)) { System.ONPlusTask= 3; return false; }
    if (NewKeyPress==KeyPlusInKeyMap) { AddKeyInBuffer(KEYONPLUS); return false; } 
    if (NewKeyPress==KeyMinusInKeyMap) { AddKeyInBuffer(KEYONMINUS); return false; } 
  }
  
  // isolate the first new key down..
  int key=0;
  while (1) 
  {
    int k= (int)NewKeyPress&1;
    NewKeyPress>>=1; // next key.. this is of course inefficient, but hey... good enough for now... and on an arm it's not too bad anyway...
    if (k!=0) // is that key down?
    {
      if (NewKeyPress==0) // if this is not the only new key down, chances are this is a ghost effect
                          // or the user is mighty good at pressing 2 keys exactly at the same time (give or take
                          // timer delay...) in all cases, if this is a double press, then we can not know which key was pressed first
                          // and should ignore it, and if it's a ghost, we can not know which key is actually pressed and
                          // should also ignore the key press...
      { // ok, so, this is a valid key press... but is it a bounce?
        if (System.LastKeyPress!=key) // is it a bounce? if yes, ignore key...
        {
          System.LastKeyPress= key; // not a bounce, save the key for alter bounce detection!
          AddKeyInBuffer(key);      // send key to the system
          Printk("<Setup TC1 IRQ>");
          EnableTimerInterupt(1, 50, ResetLastKeyPress); // program timer for end of bounce time detection
        }
      }
      return true;  // return, and there is at least one key down...
    }
    key++;
  }
}


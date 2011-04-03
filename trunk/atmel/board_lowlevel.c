/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
/// \unit
///
/// !Purpose
///
/// Provides the low-level initialization function that gets called on chip
/// startup.
///
/// =====> File adapted to wp34s by Marcus von Cube <=====
///
/// !Usage
///
/// LowLevelInit() is called in #board_cstartup.S#.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "board.h"
#include "board_memories.h"

//------------------------------------------------------------------------------
//         Local definitions
//------------------------------------------------------------------------------

// Set this if you have a modified board fitted with a crystal
//#define XTAL

// We do not need the default Interrupt handlers here
#define FIQ_HANDLER resetHandler
#define IRQ_HANDLER resetHandler
#define SPU_HANDLER resetHandler
extern void resetHandler(void);

// Do not initialize the PLL, leave clock at 2 MHz
#define NOPLL  

#ifndef NOPLL
/// PLL frequency range.
#define BOARD_CKGR_PLL          AT91C_CKGR_OUT_2

/// PLL startup time (in number of slow clock ticks).
#define BOARD_PLLCOUNT          AT91C_CKGR_PLLCOUNT

/// PLL MUL value.
#define BOARD_MUL               (914 << 16)

/// PLL DIV value.
#define BOARD_DIV               1

/// Master clock prescaler value.
#define BOARD_PRESCALER         0
#endif

//------------------------------------------------------------------------------
//         Global variables
//------------------------------------------------------------------------------

// SUPC status value read during LowLevelInit(), discarded by read.
unsigned int gLowLevelInitSupcStatus;

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

#ifndef SPU_HANDLER
//------------------------------------------------------------------------------
/// Default spurious interrupt handler. Infinite loop.
//------------------------------------------------------------------------------
void defaultSpuriousHandler(void)
{
    while (1);
}
#define SPU_HANDLER defaultSpuriousHandler
#endif

#ifndef FIQ_HANDLER
//------------------------------------------------------------------------------
/// Default handler for fast interrupt requests. Infinite loop.
//------------------------------------------------------------------------------
void defaultFiqHandler(void)
{
    while (1);
}
#define FIQ_HANDLER defaultFiqHandler
#endif

#ifndef IRQ_HANDLER
//------------------------------------------------------------------------------
/// Default handler for standard interrupt requests. Infinite loop.
//------------------------------------------------------------------------------
void defaultIrqHandler(void)
{
    while (1);
}
#define IRQ_HANDLER defaultIrqHandler
#endif

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Performs the low-level initialization of the chip. This includes EFC, master
/// clock, AIC & watchdog configuration, as well as memory remapping.
//------------------------------------------------------------------------------

void LowLevelInit(void)
{
    volatile unsigned int i;

#ifndef NOPLL
    // Set flash wait states in the EFC
    // 48MHz = 1 wait state
#if defined(at91sam7l64) || defined(at91sam7l128)
    AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_3FWS;
#else
    #error No chip definition ?
#endif
#endif

#ifdef XTAL
    // Enable external oscillator (on first boot only, after that command does nothing)
    gLowLevelInitSupcStatus = AT91C_BASE_SUPC->SUPC_SR;
    if ((gLowLevelInitSupcStatus & AT91C_SUPC_OSCSEL) != AT91C_SUPC_OSCSEL) {
        
        AT91C_BASE_SUPC->SUPC_CR = (0xA5 << 24) | AT91C_SUPC_XTALSEL;
        while ((AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_OSCSEL) != AT91C_SUPC_OSCSEL);
    }
#endif

#ifndef NOPLL // We leave the clock as is here and do the switch in main();

    // Switch to slow clock if necessary (so PLL frequency can be changed)
    if ((AT91C_BASE_PMC->PMC_MCKR & AT91C_PMC_CSS) == AT91C_PMC_CSS_PLL_CLK) {

        AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_CSS_SLOW_CLK;
        while ((AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) != AT91C_PMC_MCKRDY);
    }

    // Initialize PLL at 30MHz
    AT91C_BASE_PMC->PMC_PLLR = BOARD_CKGR_PLL | BOARD_PLLCOUNT | BOARD_MUL | BOARD_DIV;
    while ((AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK) == 0);

    // Switch to PLL
    AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_CSS_PLL_CLK;
    while ((AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) == 0);

#endif // NOPLL

    // Initialize AIC
    AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;
    AT91C_BASE_AIC->AIC_SVR[0] = (unsigned int) FIQ_HANDLER;
    for (i = 1; i < 31; i++) {

        AT91C_BASE_AIC->AIC_SVR[i] = (unsigned int) IRQ_HANDLER;
    }
    AT91C_BASE_AIC->AIC_SPU = (unsigned int) SPU_HANDLER;

    // Unstack nested interrupts
    for (i = 0; i < 8 ; i++) {

        AT91C_BASE_AIC->AIC_EOICR = 0;
    }

    // Enable Debug mode
    AT91C_BASE_AIC->AIC_DCR = AT91C_AIC_DCR_PROT;

#ifdef NOWD
    // Watchdog initialization
    // Can only be changed once after reset
    AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;
#else
    // Settings borrowed from HP20b SDK
    AT91C_BASE_WDTC->WDTC_WDMR = 
        (AT91C_WDTC_WDV/8)    // (WDTC) Watchdog Timer Restart set to 2 second
      |  AT91C_WDTC_WDRSTEN   // (WDTC) Watchdog Reset Enable
      |  AT91C_WDTC_WDD       // (WDTC) Watchdog Delta Value
      |  AT91C_WDTC_WDDBGHLT  // (WDTC) Watchdog Debug Halt
    ;
#endif

    // Remap the internal SRAM at 0x0
    BOARD_RemapRam();
}


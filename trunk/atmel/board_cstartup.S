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

 /* =====> File adapted to wp34s by Marcus von Cube <===== */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#define IRQ_IN_SVC_MODE

#define __ASSEMBLY__
#include "board.h"

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

#ifdef IRQ_IN_SVC_MODE
#define IRQ_STACK_SIZE   8*3*4
#else
// Reserve a larger stack because we run IRQs in IRQ mode, not in SVC mode here
#define IRQ_STACK_SIZE	 256
#endif

#define ARM_MODE_ABT     0x17
#define ARM_MODE_FIQ     0x11
#define ARM_MODE_IRQ     0x12
#define ARM_MODE_SVC     0x13

#define I_BIT            0x80
#define F_BIT            0x40

//------------------------------------------------------------------------------
//      Startup routine
//------------------------------------------------------------------------------

        .align      4
        .arm
        
/* Exception vectors
 *******************/

//        .section    .vectors, "a"
	.section .text

// =====> MvC: All vectors except irq point to reset

resetVector:
        ldr     pc, =resetHandler       /* Reset */
undefVector:
        b       resetVector             /* Undefined instruction */
swiVector:
        b       resetVector             /* Software interrupt */
prefetchAbortVector:
        b       resetVector             /* Prefetch abort */
dataAbortVector:
        b       resetVector             /* Data abort */
reservedVector:
        b       resetVector             /* Reserved for future use */
irqVector:
        b       irqHandler              /* Interrupt */
fiqVector:
                                        /* Fast interrupt */
//------------------------------------------------------------------------------
/// Handles a fast interrupt request by branching to the address defined in the
/// AIC.
// =====> MvC: Reset
//------------------------------------------------------------------------------
fiqHandler:
        b       resetVector
	
//------------------------------------------------------------------------------
/// Handles incoming interrupt requests by branching to the corresponding
/// handler, as defined in the AIC. Supports interrupt nesting.
//------------------------------------------------------------------------------
irqHandler:

/* Save interrupt context on the stack to allow nesting */
        sub     lr, lr, #4
        stmfd   sp!, {lr}
        mrs     lr, SPSR
        stmfd   sp!, {r0, lr}

/* Write in the IVR to support Protect Mode */
        ldr     lr, =AT91C_BASE_AIC
        ldr     r0, [r14, #AIC_IVR]
        str     lr, [r14, #AIC_IVR]

#ifdef IRQ_IN_SVC_MODE
/* Branch to interrupt handler in Supervisor mode */
        msr     CPSR_c, #ARM_MODE_SVC
#else
/* Leave mode as is */
#endif
        stmfd   sp!, {r1-r3, r4, r12, lr}
        mov     lr, pc
        bx      r0
        ldmia   sp!, {r1-r3, r4, r12, lr}
#ifdef IRQ_IN_SVC_MODE
/* Switch back to IRQ mode */
        msr     CPSR_c, #ARM_MODE_IRQ | I_BIT
#endif

/* Acknowledge interrupt */
        ldr     lr, =AT91C_BASE_AIC
        str     lr, [r14, #AIC_EOICR]

/* Restore interrupt context and branch back to calling code */
        ldmia   sp!, {r0, lr}
        msr     SPSR_cxsf, lr
        ldmia   sp!, {pc}^

//------------------------------------------------------------------------------
/// Initializes the chip and branches to the main() function.
//------------------------------------------------------------------------------
        .section    .text
        .global     entry, resetHandler

entry:
resetHandler:

/* Dummy access to the .vectors section so it does not get optimized */
        ldr     r0, =resetVector

/* Set pc to actual code location (i.e. not in remap zone) */
	ldr     pc, =1f

/* Perform low-level initialization of the chip using LowLevelInit() */
1:
        ldr     r4, =_sstack
        mov     sp, r4
        ldr     r0, =LowLevelInit
        mov     lr, pc
        bx      r0

#ifndef NO_BACKUP_INIT
/* Check if we were in backup mode */
        ldr     r0, =AT91C_BASE_RSTC+RSTC_RSR
        ldr     r0, [r0]
        and     r0, r0, #AT91C_RSTC_RSTTYP_WAKEUP
        cmp     r0, #AT91C_RSTC_RSTTYP_WAKEUP
        bne     initialize
/* Check if SRAM was backuped */
        ldr     r0, =AT91C_BASE_SUPC+SUPC_MR
        ldr     r0, [r0]
        and     r0, r0, #AT91C_SUPC_SRAMON
        cmp     r0, #AT91C_SUPC_SRAMON
        beq     relocate

/* Initialize backup data */
initialize:
        ldr     r0, =_efixed
        ldr     r1, =_erelocate
        ldr     r2, =_srelocate
        add     r0, r0, r1
        sub     r0, r0, r2
        ldr     r1, =_spersistent_ram
        ldr     r2, =_epersistent_ram
1:
        cmp     r1, r2
        ldrne   r3, [r0], #4
        strne   r3, [r1], #4
        bne     1b
#endif

#ifndef NO_RAM_COPY
/* Initialize the relocate segment */
relocate:
        ldr     r0, =_efixed
        ldr     r1, =_srelocate
        ldr     r2, =_erelocate
1:
        cmp     r1, r2
        ldrcc   r3, [r0], #4
        strcc   r3, [r1], #4
        bcc     1b
#endif

/* Clear the zero segment */
        ldr     r0, =_szero
        ldr     r1, =_ezero
        mov     r2, #0
1:
        cmp     r0, r1
        strcc   r2, [r0], #4
        bcc     1b

/* Setup stacks
 **************/
/* IRQ mode */
        msr     CPSR_c, #ARM_MODE_IRQ | I_BIT | F_BIT
        mov     sp, r4
        sub     r4, r4, #IRQ_STACK_SIZE

/* Supervisor mode (interrupts enabled) */
        msr     CPSR_c, #ARM_MODE_SVC | F_BIT
        mov     sp, r4

/* Branch to main()
 ******************/
        ldr     r0, =main
        mov     lr, pc
        bx      r0

/* Reset when program is finished */
1:
        b       1b


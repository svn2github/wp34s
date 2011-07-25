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

/*------------------------------------------------------------------------------
 *      Linker script for running in internal Flash on the AT91SAM7L128.
 *
 *      Modified by Marcus von Cube for wp34s
 *----------------------------------------------------------------------------*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(entry)

MEMORY
{
    sram (W!RX) :       ORIGIN = 0x200000, LENGTH = 4K
    backupsram (W!RX) : ORIGIN = 0x300000, LENGTH = 2K
    
    flash (RX) :        ORIGIN = 0x100000, LENGTH = 0x20000 - 10K - 16
    revision (R) :      ORIGIN = 0x120000 - 10K - 16, LENGTH = 16
    userflash (R) :     ORIGIN = 0x120000 - 10K, LENGTH = 10K

    cmdtab (R) :        ORIGIN = 0x0f0000, LENGTH = 0x10000
}

SECTIONS
{
    .cmdtab :
    {
    	*(.cmdtab)
    } >cmdtab 
      
    .fixed : 
    {
        . = ALIGN(4);
        _sfixed = .;
        *(.text*)
        *(.rodata*)
        *(.xrom)          /* remove for fixed XROM */
        . = ALIGN(4);
        _efixed = .;
    } >flash

    .relocate : AT (_efixed)
    {
        . = ALIGN(4);
        _srelocate = .;
        *(.vectors)
        *(.ramfunc)
        *(.data)
        . = ALIGN(4);
        _erelocate = .;
    } >sram

    .revision : 
    {
	*(.revision)
    } >revision
    
    .userflash (NOLOAD) :
    {
	*(.userflash)
    } >userflash

    .bss (NOLOAD) : 
    {
        . = ALIGN(4);
        _szero = .;
        *(.bss)
    } >sram
    
    .slcdcmem (NOLOAD) :
    {
        . = ALIGN(4);
    	*(.slcdcmem)
        . = ALIGN(4);
        _ezero = .;
    } >sram

    .backup (NOLOAD) :
    {
        . = ALIGN(4);
        _sbackup = .;
        *(.backup)
        . = ALIGN(4);
        _ebackup = .;
    } >backupsram

    _sstack = 0x201000;

    /* The following gets rid of any exception handler code */
    /DISCARD/ :
    {
        *(.ARM.extab*)
        *(.ARM.exidx*)
    } 
}
end = .;

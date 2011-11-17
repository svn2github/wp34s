/* This file is part of 34S.
 * 
 * 34S is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * 34S is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with 34S.  If not, see <http://www.gnu.org/licenses/>.
 */

/*------------------------------------------------------------------------------
 *      Linker script for running in internal Flash on the AT91SAM7L128.
 *
 *      Modified by Marcus von Cube for wp34s
 *      Needs to be preprocessed by gcc -E -P -x c wp34s.lds >wp34s_pre.lds
 *----------------------------------------------------------------------------*/

#define REALBUILD 1
#include "features.h"
#define K(x) CONCAT(x, K)
#define CONCAT(x, y) x ## y
#define USER_FLASH_SIZE K(NUMBER_OF_FLASH_REGIONS)

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(entry)

MEMORY
{
    sram (W!RX) :       ORIGIN = 0x200000, LENGTH = 4K
    backupsram (W!RX) : ORIGIN = 0x300000, LENGTH = 2K
    
    flash (RX) :        ORIGIN = 0x100000, LENGTH = 0x20000 - USER_FLASH_SIZE - 4
    revision (R) :      ORIGIN = 0x120000 - USER_FLASH_SIZE - 4, LENGTH = 4
    userflash (R) :     ORIGIN = 0x120000 - USER_FLASH_SIZE, LENGTH = USER_FLASH_SIZE

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

    .volatile (NOLOAD) :
    {
        . = ALIGN(4);
    	*(.volatile)
        . = ALIGN(4);
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

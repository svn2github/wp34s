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

#ifndef __STORAGE_H__
#define __STORAGE_H__


// The actual size will be shorter on the device
#define NUMPROG_FLASH	4094

#ifdef REALBUILD
// Actual size of user flash area, Linker symbol on the device
extern char UserFlashSteps;
#define NUMPROG_FLASH_MAX ((int) &UserFlashSteps)
#else
#define NUMPROG_FLASH_MAX NUMPROG_FLASH
#endif

typedef struct _flash_region {
        unsigned short crc;
        unsigned short last_prog;
        s_opcode prog[ NUMPROG_FLASH ];
} FLASH_REGION;

extern FLASH_REGION UserFlash;
extern TPersistentRam BackupFlash;

extern unsigned short int crc16( const void *base, unsigned int length );
extern int checksum_code(void);
extern int checksum_all(void);
extern int checksum_backup(void);
extern void init_library(void);
extern int append_program(const s_opcode *source, int length);

extern void flash_backup(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void flash_restore(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern int flash_remove( int step_no, int count );
extern void sam_ba_boot(void);
extern void save_program(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void load_program(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void load_registers(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void load_state(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void store_program(decimal64 *nul1, decimal64 *nul2, enum nilop op);
extern void recall_program(decimal64 *nul1, decimal64 *nul2, enum nilop op);

#if !defined(REALBUILD) && !defined (QTGUI)
extern void save_statefile(void);
extern void load_statefile(void);
#endif


#endif

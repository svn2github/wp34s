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

#define REGION_TYPE_PROGRAM 0
#define REGION_TYPE_DATA 1
#define NUMBER_OF_FLASH_REGIONS 2
#define BACKUP_REGION (NUMBER_OF_FLASH_REGIONS + 1)

typedef struct _flash_region {
	unsigned int crc : 16;
	unsigned int type : 1;        // 1 for data, zero for program
	unsigned int length : 15;
	unsigned short data[ (1024 - 2 - 2) / sizeof(unsigned short) ];
} FLASH_REGION;

typedef struct _user_flash {
	FLASH_REGION region[ NUMBER_OF_FLASH_REGIONS ];
	TPersistentRam backup;
} TUserFlash;

extern TUserFlash UserFlash;

extern unsigned short checksum_code(void);
extern int checksum_region(int r);
extern int checksum_all(void);
extern int checksum_backup(void);

extern void flash_backup(void);
extern void flash_restore(void);
extern void sam_ba_boot(void);
extern void save_program(unsigned int region, enum rarg op);
extern void load_program(unsigned int region, enum rarg op);
extern void swap_program(unsigned int region, enum rarg op);
extern void save_registers(unsigned int region, enum rarg op);
extern void load_registers(unsigned int region, enum rarg op);
extern void swap_registers(unsigned int region, enum rarg op);

#ifndef REALBUILD
extern void save_state(void);
extern void load_state(void);
#endif

#endif

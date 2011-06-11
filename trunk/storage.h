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
#define NUMBER_OF_FLASH_REGIONS 5
#define BACKUP_REGION 1

typedef struct _flash_region {
	unsigned short crc;
	unsigned short last_prog;
	s_opcode prog[ 512 - 2 ];
} FLASH_REGION;

typedef struct _user_flash {
	FLASH_REGION region[ NUMBER_OF_FLASH_REGIONS - 2 ];
	TPersistentRam backup;
} TUserFlash;

extern TUserFlash UserFlash;

#define flash_region(n) (UserFlash.region[ NUMBER_OF_FLASH_REGIONS - 1 - n ])

extern int checksum_code(void);
extern int checksum_region(int r);
extern int checksum_all(void);
extern int checksum_backup(void);
extern int is_prog_region(unsigned int region);
extern int is_data_region(unsigned int region);

extern void flash_backup(decimal64 *nul1, decimal64 *nul2);
extern void flash_restore(decimal64 *nul1, decimal64 *nul2);
extern void sam_ba_boot(void);
extern void save_program(unsigned int region, enum rarg op);
extern void load_program(unsigned int region, enum rarg op);
extern void swap_program(unsigned int region, enum rarg op);
extern void load_registers(decimal64 *nul1, decimal64 *nul2);
extern void load_state(decimal64 *nul1, decimal64 *nul2);

#ifndef REALBUILD
extern void save_statefile(void);
extern void load_statefile(void);
#endif

#endif

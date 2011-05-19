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
	unsigned short crc;
	unsigned int type : 1;        // 1 for data, zero for program
	unsigned int length : 15;
	char data[ 1024 - 2 - 2 ];
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

void flash_backup(void);
void flash_restore(void);
void sam_ba_boot(void);
int save_program(int region);
int load_program(int region);
int swap_program(int region);

#ifndef REALBUILD
extern void save_state(void);
extern void load_state(void);
#endif

#endif

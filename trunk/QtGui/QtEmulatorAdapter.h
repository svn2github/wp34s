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

#ifndef QTEMULATOR_ADAPTER_H_
#define QTEMULATOR_ADAPTER_H_

extern "C"
{
extern void init_calculator();
extern void forward_keycode(int);
extern void forward_key_released();
extern char* get_memory();
extern int get_memory_size();
extern void prepare_memory_save();
extern int get_flash_region_size();
extern int get_number_of_flash_regions();
extern char* get_filled_flash_region(int);
extern void fast_backup_to_flash();
}

#endif /* QTEMULATOR_ADAPTER_H_ */

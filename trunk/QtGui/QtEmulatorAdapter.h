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
extern char* get_backup();
extern int get_backup_size();
extern char* get_user_flash();
extern int get_user_flash_size();
extern void prepare_memory_save();
extern void after_state_load();
extern void after_library_load();
extern void after_backup_load();
extern int get_region_backup_index();
extern void reset_wp34s();
extern char* get_version_string();
extern char* get_svn_revision_string();
extern char* get_formatted_displayed_number();
extern int* get_displayed_text();
extern void set_fshift_locked(int);
extern void set_gshift_locked(int);
extern void set_hshift_locked(int);
extern int is_not_shifted();
extern int is_hshifted();
extern int forward_byte_received(short);
extern void shutdown_adapter();

extern char* get_register_names();
extern int get_first_register_index();
extern char* get_formatted_register(int anIndex);

extern int is_runmode();
extern int uparrow_code();
extern int downarrow_code();

}

#endif /* QTEMULATOR_ADAPTER_H_ */

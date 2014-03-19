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
extern char* get_last_displayed();
extern char* get_last_displayed_number();
extern char* get_last_displayed_exponent();
extern int is_small_font(char *p);
extern void set_fshift_locked(int);
extern void set_gshift_locked(int);
extern void set_hshift_locked(int);
extern int is_not_shifted();
extern int is_hshifted();
#if HAS_SERIAL
extern int forward_byte_received(short);
#endif
extern void shutdown_adapter();

extern char* get_register_names();
extern int get_first_register_index();
extern int get_numregs();
extern int get_maxnumregs();
extern char* get_formatted_register(int anIndex);
extern int is_runmode();
extern int is_catalogue_mode();
extern unsigned int current_catalogue(int);
extern const char *catcmd(unsigned int, char *);
extern int is_complex_mode();
extern char get_complex_prefix();
extern void execute_catpos(int aCatPos);
extern int get_catpos();
extern int forward_catalog_selection(int aCatPos);
extern void close_catalog();
extern int current_catalogue_max(void);
extern int find_pos(const char* text);

extern int uparrow_code();
extern int downarrow_code();
extern int backspace_code();

extern int forward_set_IO_annunciator();
extern int getdig(int ch);
extern char isForcedDispPlot();
}

#endif /* QTEMULATOR_ADAPTER_H_ */

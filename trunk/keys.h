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

#ifndef __KEYS_H__
#define __KEYS_H__

typedef unsigned char key_t;

enum {
	KEY_11=0, KEY_12, KEY_13, KEY_14, KEY_15,
		KEY_16, KEY_17, KEY_18, KEY_19, KEY_10,
	KEY_21, KEY_22, KEY_23, KEY_24, KEY_25,
		KEY_26, KEY_27, KEY_28, KEY_29, KEY_20,
	KEY_31, KEY_32, KEY_33, KEY_34, KEY_35,
		KEY_36, KEY_37, KEY_38, KEY_39, KEY_30,
	KEY_41, KEY_42, KEY_43, KEY_44, KEY_45,
		KEY_46, KEY_47, KEY_48, KEY_49, KEY_40,
	KEY_01, KEY_02,

	KEY_SHIFT_null = 0x00,
	KEY_SHIFT_f = 0x40,
	KEY_SHIFT_g = 0x80,
	KEY_SHIFT_m = 0xc0
};


#endif

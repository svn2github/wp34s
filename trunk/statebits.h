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

/* Define the fields and their bit lengths as arguments to the SB macro.
 * This macro takes on a number of different form to populate the state
 * structure and to process it.
 *
 * There is a hard limit of 53 bits of saved state that will fit into a
 * sixteen digit real.
 */

	SB(dispmode, 2);	  // Display mode (ALL, FIX, SCI, ENG)
	SB(dispdigs, 4);	  // Display digits

	SB(trigmode, 2);	  // Trig mode (DEG, RAD, GRAD)
	SB(date_mode, 2);	  // Date input/output format
	SB(sigma_mode, 3);	  // Which sigma regression mode we're using

	SB(t12, 1);		  // 12 hour time mode

	SB(int_mode, 2);	  // Integer sign mode
	SB(stack_depth, 1);	// Stack depth

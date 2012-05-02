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

#include "pretty.h"
#include "xeq.h"		// This helps the syntax checker

const char *pretty(unsigned char z) {
	if (z < 32)
		return map32[z & 0x1f];
	if (z >= 127)
		return maptop[z - 127];
	return NULL;
}


void prettify(const char *in, char *out) {
	const char *p;
	char c;

	while (*in != '\0') {
		c = *in++;
		p = pretty(c);
		if (p == NULL)
			*out++ = c;
		else {
			*out++ = '[';
			while (*p != '\0')
				*out++ = *p++;
			*out++ = ']';
		}
	}
	*out = '\0';
}

void dump_opcodes(FILE *f) {
	int c, d;
	char cmdname[16];
	char cmdpretty[500];
	const char *p;

	for (c=0; c<65536; c++) {
		if (isDBL(c)) {
			const unsigned int cmd = opDBL(c);
			if ((c & 0xff) != 0)
				continue;
			if (cmd >= NUM_MULTI)
				continue;
#ifdef INCLUDE_MULTI_DELETE
			if (cmd == DBL_DELPROG)
				continue;
#endif
			xset(cmdname, '\0', 16);
			xcopy(cmdname, multicmds[cmd].cmd, NAME_LEN);
			prettify(cmdname, cmdpretty);
#ifdef XROM_LONG_BRANCH
			if (cmd == DBL_XBR)
				fprintf(f, "0x%04x\tmult\t%s\txrom\n", c, cmdpretty);
			else
#endif
				fprintf(f, "0x%04x\tmult\t%s\n", c, cmdpretty);
		} 
		else if (isRARG(c)) {
			const unsigned int cmd = RARG_CMD(c);
			unsigned int limit;

			if (cmd >= NUM_RARG)
				continue;
#ifdef INCLUDE_MULTI_DELETE
			if (cmd == RARG_DELPROG)
				continue;
#endif
			limit = argcmds[cmd].lim + 1;
			if (cmd != RARG_ALPHA && cmd != RARG_SHUFFLE && (c & RARG_IND) != 0)
				continue;
			p = catcmd(c, cmdname);
			if (strcmp(p, "???") == 0)
				continue;
			prettify(p, cmdpretty);
			if (cmd == RARG_ALPHA) {
				if ((c & 0xff) == 0)
					continue;
				if ((c & 0xff) == ' ')
					fprintf(f, "0x%04x\tcmd\t[alpha] [space]\n", c);
				else
					fprintf(f, "0x%04x\tcmd\t[alpha] %s\n", c, cmdpretty);
				continue;
			} 
			else if (cmd == RARG_CONST || cmd == RARG_CONST_CMPLX) {
				const int n = c & 0xff;
				if (n == OP_ZERO || n == OP_ONE)
					continue;
				fprintf(f, "0x%04x\tcmd\t%s# %s\n", c, 
					cmd == RARG_CONST_CMPLX ? "[cmplx]" : "", cmdpretty );
				continue;
			} 
			else if (cmd == RARG_CONV) {
				fprintf(f, "0x%04x\tcmd\t%s\n", c, cmdpretty);
				continue;
			}
			else if (cmd == RARG_SHUFFLE) {
				fprintf(f, "0x%04x\tcmd\t%s %c%c%c%c\n", c, cmdpretty,
						REGNAMES[c & 3], REGNAMES[(c >> 2) & 3],
						REGNAMES[(c >> 4) & 3], REGNAMES[(c >> 6) & 3]);
				continue;
			}
			if ((c & 0xff) != 0)
				continue;
			if (argcmds[cmd].indirectokay && limit > RARG_IND)
				limit = RARG_IND;
			fprintf(f, "0x%04x\targ\t%s\tmax=%u", c, cmdpretty, limit);
			if (argcmds[cmd].indirectokay) {
				fprintf(f, ",indirect");
			}
			if (cmd == RARG_STOSTK || cmd == RARG_RCLSTK)
				fprintf(f, ",stostack");
			else if (argcmds[cmd].stckreg)
				fprintf(f, ",stack");
			else if (argcmds[cmd].local)
				fprintf(f, ",local");
			if (argcmds[cmd].cmplx)
				fprintf(f, ",complex");
			if (cmd == RARG_MODE_SET 
			 || cmd == RARG_MODE_CLEAR 
			 || cmd == RARG_XROM_IN 
			 || cmd == RARG_XROM_OUT
#ifndef INCLUDE_INDIRECT_BRANCHES
		       //|| cmd == RARG_iSKIP
#endif
		       //|| cmd == RARG_BSF
		       //|| cmd == RARG_BSB
		       //|| cmd == RARG_INDEX
		       //|| cmd == RARG_CONST_INDIRECT
#ifdef XROM_RARG_COMMANDS
			 || cmd == RARG_XROM_ARG
#endif
			   )
				fprintf(f, ",xrom");
			fprintf(f, "\n");
		}
		else {
			p = catcmd(c, cmdname);
			if (strcmp(p, "???") == 0)
				continue;
			prettify(p, cmdpretty);
			d = argKIND(c);
			switch (opKIND(c)) {
			default:
				break;

			case KIND_MON:
				if (d < NUM_MONADIC && (! isNULL(monfuncs[d].mondreal) || ! isNULL(monfuncs[d].monint)))
					break;
				continue;

			case KIND_DYA:
				if (d < NUM_DYADIC && (! isNULL(dyfuncs[d].dydreal) || ! isNULL(dyfuncs[d].dydint)))
					break;
				continue;

			case KIND_CMON:
				if (d < NUM_MONADIC && ! isNULL(monfuncs[d].mondcmplx)) {
					if (cmdname[0] == COMPLEX_PREFIX)
						break;
					fprintf(f, "0x%04x\tcmd\t[cmplx]%s\n", c, cmdpretty);
				}
				continue;

			case KIND_CDYA:
				if (d < NUM_DYADIC && ! isNULL(dyfuncs[d].dydcmplx)) {
					if (cmdname[0] == COMPLEX_PREFIX)
						break;
					fprintf(f, "0x%04x\tcmd\t[cmplx]%s\n", c, cmdpretty);
				}
				continue;

			case KIND_NIL:
#ifdef INCLUDE_STOPWATCH
				if (d == OP_STOPWATCH)
					continue;
#endif
				if (d >= OP_CLALL && d <= OP_CLPALL) {
					continue;
				}
				if (d == OP_LOADA2D || d == OP_SAVEA2D ||
						d == OP_GSBuser || d == OP_POPUSR) {
					fprintf(f, "0x%04x\tcmd\t%s\txrom\n", c, cmdpretty);
					continue;
				}
			}
			fprintf(f, "0x%04x\tcmd\t%s\n", c, cmdpretty);
		}
	}
}


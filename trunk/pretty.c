/* Mappings from our internal character codes to readable strings.
 * The first table is for characters below space and the second for those
 * >=127 (del).
 */
static const char *const map32[32] = {
	NULL,	"x-bar", "y-bar", "sqrt", "integral", "degree", "narrow-space", "grad",
	"+/-", "<=", ">=", "!=", "euro", "->", "<-", "v",
	"^", "f-shift", "g-shift", "h-shift", "cmplx", "O-slash", "o-slash", "<->",
	"sz", "x-hat", "y-hat", "sub-m", "times", "approx", "pound", "yen"
};

static const char *const maptop[129] = {
	"^v",
	"^3", "sub-w", "GAMMA", "DELTA", "0204", "0205", "0206", "THETA",
	"AE", "ae", "LAMBDA", "0213", "0214", "XI", "sol", "PI",
	"0220", "SIGMA", "0222", "0223", "PHI", "0225", "PSI", "OMEGA",
	"sub-B", "sub-mu", "^2", "sub-infinity", "^x", "^-1", "h-bar", "infinity",
	"alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta",
	"iota", "kappa", "lambda", "mu", "nu", "xi", "terra", "pi",
	"rho", "sigma", "tau", "upsilon", "phi", "chi", "psi", "omega",
	"sub-0", "sub-1", "sub-2", "sub-c", "sub-e", "sub-n", "sub-p", "sub-u",
	"A-grave", "A-acute", "A-tilde", "A-umlaut", "A-dot", "C-acute", "C-hook", "C-cedilla",
	"E-grave", "E-acute", "E-filde", "E-trema", "I-grave", "I-acute", "I-tilde", "I-trema",
	"N-tilde", "O-grave", "O-acute", "O-tilde", "O-umlaut", "R-hook", "S-hook", "sub-A",
	"U-grave", "U-acute", "U-tilde", "U-umlaut", "U-dot", "Y-acute", "Y-trema", "Z-hook",
	"a-grave", "a-acute", "a-tilde", "a-umlaut", "a-dot", "c-acute", "c-hook", "c-cedilla",
	"e-grave", "e-acute", "e-tilde", "e-trema", "i-grave", "i-acute", "i-tilde", "i-trema",
	"n-tilde", "o-grave", "o-acute", "o-tilde", "o-umlaut", "r-hook", "s-hook", "sub-k",
	"u-grave", "u-acute", "u-tilde", "u-umlaut", "u-dot", "y-acute", "y-trema", "z-hook"
};

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
			if (cmd >= num_multicmds)
				continue;
#ifdef INCLUDE_MULTI_DELETE
			if (cmd == DBL_DELPROG)
				continue;
#endif
			xset(cmdname, '\0', 16);
			xcopy(cmdname, multicmds[cmd].cmd, NAME_LEN);
			prettify(cmdname, cmdpretty);
			if (cmd == DBL_XBR)
				fprintf(f, "0x%04x\tmult\t%s\txrom\n", c, cmdpretty);
			else
				fprintf(f, "0x%04x\tmult\t%s\n", c, cmdpretty);
		} else if (isRARG(c)) {
			const unsigned int cmd = RARG_CMD(c);
			unsigned int limit;

			if (cmd >= NUM_RARG)
				continue;
#ifdef INCLUDE_MULTI_DELETE
			if (cmd == RARG_DELPROG)
				continue;
#endif
			limit = argcmds[cmd].lim;
			if (cmd != RARG_ALPHA && (c & RARG_IND) != 0)
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
			} else if (cmd == RARG_CONST || cmd == RARG_CONST_CMPLX) {
				fprintf(f, "0x%04x\tcmd\t%s# %s\n", c, cmd == RARG_CONST_CMPLX?"[cmplx]":"", cmdpretty);
				continue;
			} else if (cmd == RARG_CONV) {
				fprintf(f, "0x%04x\tcmd\t%s\n", c, cmdpretty);
				continue;
			} else if (cmd == RARG_CONST_INT) {
				p = prt(c, cmdname);
				if (strcmp(p, "???") != 0)
					fprintf(f, "0x%04x\tcmd\t%s\txrom\n", c, p);
				if ((c & 0xff) != 0)
					continue;
				limit = 0;
			}
			if ((c & 0xff) != 0)
				continue;
			fprintf(f, "0x%04x\targ\t%s\tmax=%u", c, cmdpretty, limit);
			if (argcmds[cmd].indirectokay)
				fprintf(f, ",indirect");
			if (argcmds[cmd].stos)
				fprintf(f, ",stostack");
			else if (argcmds[cmd].stckreg)
				fprintf(f, ",stack");
			else if (argcmds[cmd].local)
				fprintf(f, ",local");
			if (argcmds[cmd].cmplx)
				fprintf(f, ",complex");
			if (cmd == RARG_MODE_SET || cmd == RARG_MODE_CLEAR ||
					cmd == RARG_XROM_IN || cmd == RARG_XROM_OUT ||
					cmd == RARG_CONST_INT
			   )
				fprintf(f, ",xrom");
			fprintf(f, "\n");
		} else {
			p = catcmd(c, cmdname);
			if (strcmp(p, "???") == 0)
				continue;
			prettify(p, cmdpretty);
			d = argKIND(c);
			switch (opKIND(c)) {
			default:
				break;

			case KIND_MON:
				if (d < num_monfuncs && (! isNULL(monfuncs[d].mondreal) || ! isNULL(monfuncs[d].monint)))
					break;
				continue;

			case KIND_DYA:
				if (d < num_dyfuncs && (! isNULL(dyfuncs[d].dydreal) || ! isNULL(dyfuncs[d].dydint)))
					break;
				continue;

			case KIND_CMON:
				if (d < num_monfuncs && ! isNULL(monfuncs[d].mondcmplx)) {
					if (cmdname[0] == COMPLEX_PREFIX)
						break;
					fprintf(f, "0x%04x\tcmd\t[cmplx]%s\n", c, cmdpretty);
				}
				continue;

			case KIND_CDYA:
				if (d < num_dyfuncs && ! isNULL(dyfuncs[d].dydcmplx)) {
					if (cmdname[0] == COMPLEX_PREFIX)
						break;
					fprintf(f, "0x%04x\tcmd\t[cmplx]%s\n", c, cmdpretty);
				}
				continue;

			case KIND_NIL:
				d = c & 0xff;
				if (d >= OP_CLALL && d <= OP_CLPALL) {
					continue;
				}
				if (d == OP_INISOLVE || d == OP_SOLVESTEP ||
						d == OP_GSBuser || d == OP_POPUSR) {
					fprintf(f, "0x%04x\tcmd\t%s\txrom\n", c, cmdpretty);
					continue;
				}
			}
			fprintf(f, "0x%04x\tcmd\t%s\n", c, cmdpretty);
		}
	}
}


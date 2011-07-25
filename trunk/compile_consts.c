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


#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h> // mkdir
#define mkdir _mkdir
#define chdir _chdir
#define unlink _unlink
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>

#define DECNUMDIGITS	1000
#define DFLT		39

#define xcopy memcpy


#include "charmap.c"


#define NEED_D64FROMSTRING  1

#if defined(WIN32) && !defined(__GNUC__)
// Visual C on Windows will link with decnumber.lib
#include "decNumber.h"
#include "decimal64.h"
#else
#include "decNumber.c"
#include "decContext.c"
#include "decimal64.c"
#endif

#include "features.h"

static FILE *fh;

static char consts_h[ FILENAME_MAX ];
static char consts_c[ FILENAME_MAX ];
static char *libconsts = "";

/* The table of constants we're going to compile.
 *
 * It is okay to put values into this table which are never used, the linker makes
 * sure they're not in the final executable.
 *
 * As for the number of digits, if the number specified requires less than this,
 * we trim it down as required.  Thus space is saved without thinking too hard.
 */
static struct {
	int ndig;
	const char *name;
	const char *value;
} cnsts[] = {
	{ DFLT,  "NaN",			"NaN"		},
	{ DFLT,  "inf",			"inf"		},
	{ DFLT,  "_inf",		"-inf"		},
	{ DFLT,  "_9",			"-9"		},
	{ DFLT,  "_8",			"-8"		},
	{ DFLT,  "_7",			"-7"		},
	{ DFLT,  "_6",			"-6"		},
	{ DFLT,  "_5",			"-5"		},
	{ DFLT,  "_4",			"-4"		},
	{ DFLT,  "_3",			"-3"		},
	{ DFLT,  "_2",			"-2"		},
	{ DFLT,  "_1",			"-1"		},
	{ DFLT,  "_0",			"-0"		},
	{ DFLT,  "0",			"0"		},
	{ DFLT,  "1",			"1"		},
	{ DFLT,  "2",			"2"		},
	{ DFLT,  "3",			"3"		},
	{ DFLT,  "4",			"4"		},
	{ DFLT,  "5",			"5"		},
	{ DFLT,  "6",			"6"		},
	{ DFLT,  "7",			"7"		},
	{ DFLT,  "8",			"8"		},
	{ DFLT,  "9",			"9"		},
	{ DFLT,  "10",			"10"		},
	{ DFLT,  "11",			"11"		},
	{ DFLT,  "12",			"12"		},
	{ DFLT,  "13",			"13"		},
	{ DFLT,  "14",			"14"		},
	{ DFLT,  "15",			"15"		},
	{ DFLT,  "16",			"16"		},
	{ DFLT,  "17",			"17"		},
	{ DFLT,  "18",			"18"		},
	{ DFLT,  "19",			"19"		},
	{ DFLT,  "20",			"20"		},
	{ DFLT,  "21",			"21"		},
	{ DFLT,  "22",			"22"		},
	{ DFLT,  "23",			"23"		},
	{ DFLT,  "24",			"24"		},
	{ DFLT,  "25",			"25"		},
	{ DFLT,  "26",			"26"		},
	{ DFLT,  "27",			"27"		},
	{ DFLT,  "28",			"28"		},
	{ DFLT,  "29",			"29"		},
	{ DFLT,  "30",			"30"		},
	{ DFLT,  "32",			"32"		},
	{ DFLT,  "37",			"37"		},
	{ DFLT,  "60",			"60"		},
	{ DFLT,  "90",			"90"		},
	{ DFLT,  "100",			"100"		},
	{ DFLT,  "150",			"150"		},
	{ DFLT,  "256",			"256"		},
	{ DFLT,  "360",			"360"		},
	{ DFLT,  "400",			"400"		},
	{ DFLT,  "500",			"500"		},
	{ DFLT,  "1000",		"1000"		},
	{ DFLT,  "9000",		"9000"		},
	{ DFLT,  "10000",		"10000"		},
	{ DFLT,  "100000",		"100000"	},
	{ DFLT,  "1e6",			"1e6"		},
	{ DFLT,  "1e12",		"1e12"		},
	{ DFLT,  "1e32",		"1e32"		},
	{ DFLT,  "1e_32",		"1e-32"		},
	{ DFLT,  "1e_400",		"1e-400"	},
	{ DFLT,  "1e_24",		"1e-24"		},
	{ DFLT,  "1e_10",		"1e-10"		},
	{ DFLT,  "0_0000005",		"0.0000005"	},
	{ DFLT,  "0_00001",		"0.00001"	},
	{ DFLT,  "0_0001",		"0.0001"	},
	{ DFLT,  "0_001",		"0.001"		},
	{ DFLT,  "0_01",		"0.01"		},
	{ DFLT,  "0_1",			"0.1"		},
	{ DFLT,  "0_0195",		"0.0195"	},
	{ DFLT,  "0_2",			"0.2"		},
	{ DFLT,  "0_2214",		"0.2214"	},
	{ DFLT,  "0_25",		"0.25"		},
	{ DFLT,  "0_04",		"0.04"		},
	{ DFLT,  "0_05",		"0.05"		},
	{ DFLT,  "0_15",		"0.15"		},
	{ DFLT,  "0_5",			"0.5"		},
	{ DFLT,  "_0_5",		"-0.5"		},
	{ DFLT,  "0_6",			"0.6"		},
	{ DFLT,  "0_65",		"0.65"		},
	{ DFLT,  "0_665",		"0.665"		},
	{ DFLT,  "0_75",		"0.75"		},
	{ DFLT,  "0_85",		"0.85"		},
	{ DFLT,  "0_9",			"0.9"		},
	{ DFLT,  "0_97",		"0.97"		},
	{ DFLT,  "2on9",		"0.222222222222222222222222222222222222222222222222222" },
	{ DFLT,  "2on3",		"0.666666666666666666666666666666666666666666666666666" },
	{ DFLT,  "5on6",		"0.833333333333333333333333333333333333333333333333333" },
	{ DFLT,  "1on60",		".0166666666666666666666666666666666666666666666666666" },
	{ DFLT,  "100on60",		"1.66666666666666666666666666666666666666666666666666"	},
	{ DFLT,  "1_7",			"1.7"		},
	{ DFLT,  "9on5",		"1.8"		},
	{ DFLT,  "root2on2",		"0.70710678118654752440084436210484903928483593768847"	},
	{ DFLT,  "e",			"2.71828182845904523536028747135266249775724709369995"	},
	{ DFLT,  "PI",			"3.14159265358979323846264338327950288419716939937510"	},
	{ DECNUMDIGITS,  "2PI",		"6.28318530717958647692528676655900576839433879875021"
					      "16419498891846156328125724179972560696506842341359"
					      "64296173026564613294187689219101164463450718816256"
					      "96223490056820540387704221111928924589790986076392"
					      "88576219513318668922569512964675735663305424038182"
					      "91297133846920697220908653296426787214520498282547"
					      "44917401321263117634976304184192565850818343072873"
					      "57851807200226610610976409330427682939038830232188"
					      "66114540731519183906184372234763865223586210237096"
/*
					      "14892475992549913470377150544978245587636602389825"
					      "96673467248813132861720427898927904494743814043597"
					      "21887405541078434352586353504769349636935338810264"
					      "00113625429052712165557154268551557921834727435744"
					      "29368818024499068602930991707421015845593785178470"
					      "84039912224258043921728068836319627259549542619921"
					      "03741442269999999674595609990211946346563219263719"
*/
												},
	{ DFLT,  "sqrt2PI",		"2.50662827463100050241576528481104525300698674060994"	},
	{ DFLT,  "recipsqrt2PI",	"0.3989422804014326779399460599343818684758586311649347"	},
	{ DFLT,  "PIon2",		"1.57079632679489661923132169163975144209858469968755"	},
	{ DFLT,  "PIon180",		"0.01745329251994329576923690768488612713442871888541"	},
	{ DFLT,  "PIon200",		"0.015707963267948966192313216916397514420985846996876" },
	{ DFLT,  "ln2",			"0.6931471805599453094172321214581765680755001343602553"	},
	{ DFLT,  "ln10",		"2.30258509299404568401799145468436420760110148862877"	},
	{ DFLT,  "phi",			"1.61803398874989484820458683436563811772030917980576" },
	{ DFLT,  "recipsqrt5",		"0.4472135954999579392818347337462552470881236719223" },
	{ DFLT,  "egamma",		"0.5772156649015328606065120900824024310421593359399235988" },
	{ DFLT,  "_1onPI",		"-0.31830988618379067153776752674502872406891929148091" },
	{ DFLT,  "2pow60",		"1152921504606846976" },
	{ DFLT,  "60ln2",		"41.588830833596718565033927287490594084530008061615315247240"	      },

	// randfac = 2^-32  This converts a 32 integer into a [0, 1) interval
	{ DFLT, "randfac",		"0.00000000023283064365386962890625" },

	// Gamma estimate constants
	{ DFLT, "2rootEonPI",		"1.86038273420526571733624924726666311205942184140857745289" },
	{ DFLT, "gammaR",		"23.118910" },
	{ DFLT, "gammaC00",		"2.0240434640140357514731512432760E-10" },
	{ DFLT, "gammaC01",		"1.5333183020199267370932516012553" },
	{ DFLT, "gammaC02",		"-1.1640274608858812982567477805332E1" },
	{ DFLT, "gammaC03",		"4.0053698000222503376927701573076E1" },
	{ DFLT, "gammaC04",		"-8.2667863469173479039227422723581E1" },
	{ DFLT, "gammaC05",		"1.1414465885256804336106748692495E2" },
	{ DFLT, "gammaC06",		"-1.1135645608449754488425056563075E2" },
	{ DFLT, "gammaC07",		"7.9037451549298877731413453151252E1" },
	{ DFLT, "gammaC08",		"-4.1415428804507353801947558814560E1" },
	{ DFLT, "gammaC09",		"1.6094742170165161102085734210327E1" },
	{ DFLT, "gammaC10",		"-4.6223809979028638614212851576524" },
	{ DFLT, "gammaC11",		"9.7030884294357827423006360746167E-1" },
	{ DFLT, "gammaC12",		"-1.4607332380456449418243363858893E-1" },
	{ DFLT, "gammaC13",		"1.5330325530769204955496334450658E-2" },
	{ DFLT, "gammaC14",		"-1.0773862404547660506042948153734E-3" },
	{ DFLT, "gammaC15",		"4.7911128916072940196391032755132E-5" },
	{ DFLT, "gammaC16",		"-1.2437781042887028450811158692678E-6" },
	{ DFLT, "gammaC17",		"1.6751019107496606112103160490729E-8" },
	{ DFLT, "gammaC18",		"-9.7674656970897286097939311684868E-11" },
	{ DFLT, "gammaC19",		"1.8326577220560509759575892664132E-13" },
	{ DFLT, "gammaC20",		"-6.4508377189118502115673823719605E-17" },
	{ DFLT, "gammaC21",		"1.3382662604773700632782310392171E-21" },

	{ DFLT, "digammaC02",		"-0.08333333333333333333333333333333333333333333333333" },
	{ DFLT, "digammaC04",		"0.00833333333333333333333333333333333333333333333333" },
	{ DFLT, "digammaC06",		"-0.00396825396825396825396825396825396825396825396825" },
	{ DFLT, "digammaC08",		"0.00416666666666666666666666666666666666666666666666" },
	{ DFLT, "digammaC10",		"-0.00757575757575757575757575757575757575757575757575" },
	{ DFLT, "digammaC12",		"0.02109279609279609279609279609279609279609279609279" },
	{ DFLT, "digammaC16",		"0.44325980392156862745098039215686274509803921568627" },
	{ DFLT, "digammaC18",		"-3.05395433027011974380395433027011974380395433027011" },
	{ DFLT, "digammaC20",		"26.45621212121212121212121212121212121212121212121212" },

	{ DFLT, "zeta_dn",		"46292552162781456490000.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC00",		"-46292552162781456489999.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC01",		"46292552162781456488199.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC02",		"-46292552162781455948799.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC03",		"46292552162781391508479.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC04",		"-46292552162777290342399.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC05",		"46292552162616160083967.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC06",		"-46292552158343766867967.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC07",		"46292552077215245139967.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC08",		"-46292550926542378631167.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC09",		"46292538351868961619967.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC10",		"-46292429944947608649727.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC11",		"46291679074496678985727.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC12",		"-46287440465212083273727.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC13",		"46267721150632671838207.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC14",		"-46191452267259392688127.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC15",		"45944586548202893737983.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC16",		"-45272673804803148611583.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC17",		"43730029217461131280383.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC18",		"-40737788446458043695103.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC19",		"35834429458982144835583.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC20",		"-29057735883983402565631.99999999999999999999999999999999999999999999999999999999999999999999999999998"	},
	{ DFLT, "zetaC21",		"21187011638920984133631.99999999999999999999999999999999999999999999999999999999999999999999999999998" },
	{ DFLT, "zetaC22",		"-13549247519505233477631.99999999999999999999999999999999999999999999999999999999999999999999999999999"	},
	{ DFLT, "zetaC23",		"7409518295008707346431.99999999999999999999999999999999999999999999999999999999999999999999999999999"	},
	{ DFLT, "zetaC24",		"-3370795702299113029632.00000000000000000000000000000000000000000000000000000000000000000000000000000" },
	{ DFLT, "zetaC25",		"1234393873665792933888.00000000000000000000000000000000000000000000000000000000000000000000000000000"	},
	{ DFLT, "zetaC26",		"-348254351985305714688.00000000000000000000000000000000000000000000000000000000000000000000000000000"	},
	{ DFLT, "zetaC27",		"70832614939283161088.00000000000000000000000000000000000000000000000000000000000000000000000000001"	},
	{ DFLT, "zetaC28",		"-9223372036854775808.00000000000000000000000000000000000000000000000000000000000000000000000000001"	},
	{ DFLT, "zetaC29",		"576460752303423488.00000000000000000000000000000000000000000000000000000000000000000000000000001"	},

	{ -1,  NULL,	  NULL		  }
};

struct constsml {
	const char *name;
	const char *op;
	const char *val;
	const char *n2;
};
#define CONSTANT(n, op, val)	{ n, op, val, "" }
#define CONV(n1, n2, op, val)	{ n1, op, val, n2 }

struct constsml constsml[] = {
	CONSTANT("a",		"PC_a",		"365.2425"),		// Days in a year
	CONSTANT("a\270",	"PC_a0",	"5.291772083E-11"),
	CONSTANT("c",		"PC_C",		"299792458"),		// Speed of light in a vacuum
	CONSTANT("c\271",	"PC_C1",	"3.74177153E-16"),
	CONSTANT("c\272",	"PC_C2",	"1.438770E-2"),
	CONSTANT("e",		"PC_eV",	"1.602176565E-19"),
	CONSTANT("F",		"PC_F",		"96485.3365"),
	CONSTANT("g",		"PC_g",		"9.80665"),
	CONSTANT("G",		"PC_G",		"6.67384E-11"),		// NIST 2010, IAU 2009 gives 6.67428E-11
	CONSTANT("G\270",	"PC_Go",	"7.7480917346E-5"),
	CONSTANT("g\274",	"PC_Ge",	"-2.00231930436153"),
	CONSTANT("H\270",	"PC_Hubble",	"70.1"),		// Hubble constant
	CONSTANT("k",		"PC_k",		"1.3806488E-23"),
	CONSTANT("m\274",	"PC_me",	"9.10938291E-31"),
	CONSTANT("m\275",	"PC_mn",	"1.674927351E-27"),
	CONSTANT("m\276",	"PC_mp",	"1.672621777E-27"),
	CONSTANT("m\277",	"PC_mu",	"1.660538921E-27"),
	CONSTANT("m\231",	"PC_mMu",	"1.883531475E-28"),
	CONSTANT("N\327",	"PC_Na",	"6.02214129E23"),
	CONSTANT("p\270",	"PC_atm",	"101325"),
	CONSTANT("R",		"PC_R",		"8.3144621"),
	CONSTANT("r\274",	"PC_Re",	"2.8179403267E-15"),
	//CONSTANT("R\367",	"PC_Rk",	"25812.8074434"),	// von Klitzing constant
	CONSTANT("R\233",	"PC_Rinf",	"10973731.568539"),
	CONSTANT("T\270",	"PC_t",		"273.15"),
	CONSTANT("t\276",	"PC_tp",	"5.39106E-44"),		// Base Planck unit of time
	CONSTANT("V\033",	"PC_Vm",	"22.413968E-3"),
	CONSTANT("Z\270",	"PC_Zo",	"376.730313461"),
	CONSTANT("\240",	"PC_alpha",	"7.2973525698E-3"),
	CONSTANT("\242\276",	"PC_gamP",	"2.675222005E8"),
	CONSTANT("\244\270",	"PC_eps0",	"8.854187817E-12"),
	CONSTANT("\252\273",	"PC_lamC",	"2.4263102389E-12"),
	CONSTANT("\252\273\275","PC_lamCn",	"1.3195909068E-15"),
	CONSTANT("\252\273\276","PC_lamCp",	"1.32140985623E-15"),
	CONSTANT("\253\270",	"PC_mu0",	"12.566370614E-7"),
	CONSTANT("\253\230",	"PC_muB",	"927.400968E-26"),
	CONSTANT("\253\274",	"PC_muE",	"-928.476430E-26"),
	CONSTANT("\253\275",	"PC_mun",	"-0.96623647E-26"),
	CONSTANT("\253\276",	"PC_muP",	"1.410606743E-26"),
	CONSTANT("\253\277",	"PC_mu_u",	"5.05078353E-27"),
	CONSTANT("\253\231",	"PC_mumu",	"-4.49044807E-26"),
	CONSTANT("\261\230",	"PC_sigma",	"5.670373E-8"),
	CONSTANT("\224\270",	"PC_phi0",	"2.067833758E-15"),

	/* Plank related constants */
	CONSTANT("h",		"PC_PLANK",	"6.62606957E-34"),
	CONSTANT("\236",	"PC_hon2PI",	"1.054571726E-34"),
	CONSTANT("l\276",	"PC_PlanckL",	"1.616199E-35"),	// Base Planck unit of length
	CONSTANT("M\276",	"PC_PlanckM",	"2.17651E-8"),		// Base Planck unit of mass
/**/	CONSTANT("q\276",	"PC_PlanckQ",	"1.875545870e-18"),	// Base Planck unit of charge
	CONSTANT("T\276",	"PC_PlanckTh",	"1.416833E32"),		// Base Planck unit of temperature

	/* Mathematical constants */
	CONSTANT("NaN",		"NAN",		"NaN"),
	CONSTANT("\237",	"INF",		"inf"),
	CONSTANT("\242EM",	"EULER",	"0.5772156649015328606065120900824024310421593359399235988"),
	CONSTANT("\224",	"PHI",		"1.61803398874989484820458683436563811772030917980576"),
	CONSTANT("CG",		"PC_catalan",	"0.915965594177219"),	// Catalan's constant
	CONSTANT("eE",		"CNSTE",	"2.71828182845904523536028747135266249775724709369995"),
	CONSTANT("F\243",	"PC_F_delta",	"4.66920160910299067185320382"),
	CONSTANT("F\240",	"PC_F_alpha",	"2.502907875095892822283902873218"),
	CONSTANT("\257",	"PI",		"3.14159265358979323846264338327950288419716939937510"),
	//CONSTANT("\207",	"PC_MILLS",	"1.3063778838630806904686144926026057129167845851567136443680537599664340537668265988215014037011973957"),
	//CONSTANT("\252",	"PC_lam",	"0.62432998854355087099293638310083724417964262018"),
	//CONSTANT("\217\272",	"PI2",		"0.660161815846869573927812110014555778432623336"),
	//CONSTANT("Z",		"PC_apery",	"1.20205690315959"),	// Apery's constant = zeta(3)
	//CONSTANT("Gaus",	"PC_gauss",	"0.834626841674043"),	// Gauss's constant = 1 / AGM(1, sqrt(2))

	/* WGS constants */
	CONSTANT("Sa",		"PC_WGS_A",	"6378137.0"),		// WGS84 standard
	CONSTANT("Sb",		"PC_WGS_B",	"6356752.3142"),	// WGS84 standard
	CONSTANT("Se\232",	"PC_WGS_E2",	"6.69437999014E-3"),	// WGS84 standard
	CONSTANT("Se'\232",	"PC_WGS_ES2",	"6.73949674228E-3"),	// WGS84 standard
	CONSTANT("Sf\235",	"PC_WGS_F",	"298.257223563"),	// WGS84 standard
	CONSTANT("GM",		"PC_WGS_GM",	"3986004.418E8"),	// WGS84 standard
	CONSTANT("\267",	"PC_WGS_OMEGA",	"7292115.0E-11"),	// WGS84 standard

	/* Astronomical constants */
	CONSTANT("M\033",	"PC_M_luna",	"7.349e22"),		// Mass of Moon NASA Horizons
	CONSTANT("M\216",	"PC_M_sol",	"1.9891e30"),		// Mass of sun NASA Horizons
	CONSTANT("M\256",	"PC_M_terra",	"5.9736e24"),		// Mass of Earth NASA Horizons
	CONSTANT("R\033",	"PC_R_luna",	"1737.53E3"),		// Moon mean radius NASA Horizons
	CONSTANT("R\216",	"PC_R_sol",	"6.960E8"),		// Sun mean radius NASA Horizons
	CONSTANT("R\256",	"PC_R_terra",	"6371.01E3"),		// Earth mean radius NASA Horizons
	CONSTANT("SM\033",	"PC_SM_luna",	"384400E3"),		// Semi-major axis Moon NASA Horizons
	CONSTANT("SM\256",	"PC_SM_terra",	"149.60E9"),		// Semi-major axis Earth NASA Earth fact sheet

	CONSTANT(NULL, NULL, NULL)
};

struct constsml constsint[] = {
	CONSTANT("zero",	"ZERO",		"0"),
	CONSTANT("one",		"ONE",		"1"),

	CONSTANT("kloop",	"KL",		"5.01402"),
	CONSTANT("gkloop",	"GKL",		"15.02903"),

	CONSTANT("wgk10",	"WGK10",	"0.149445554002916905664936468389821"),

	CONSTANT("xgk00",	"XGK00",	"0.995657163025808080735527280689003"),
	CONSTANT("wgk00",	"WGK00",	"0.011694638867371874278064396062192"),
	CONSTANT("xgk02",	"XGK02",	"0.930157491355708226001207180059508"),
	CONSTANT("wgk02",	"WGK02",	"0.054755896574351996031381300244580"),
	CONSTANT("xgk04",	"XGK04",	"0.780817726586416897063717578345042"),
	CONSTANT("wgk04",	"WGK04",	"0.093125454583697605535065465083366"),
	CONSTANT("xgk06",	"XGK06",	"0.562757134668604683339000099272694"),
	CONSTANT("wgk06",	"WGK06",	"0.123491976262065851077958109831074"),
	CONSTANT("xgk08",	"XGK08",	"0.294392862701460198131126603103866"),
	CONSTANT("wgk08",	"WGK08",	"0.142775938577060080797094273138717"),

	CONSTANT("xgk01",	"XGK01",	"0.973906528517171720077964012084452"),
	CONSTANT("wg00",	"WG00",		"0.066671344308688137593568809893332"),
	CONSTANT("wk01",	"WK01",		"0.032558162307964727478818972459390"),

	CONSTANT("xgk03",	"XGK03",	"0.865063366688984510732096688423493"),
	CONSTANT("wg01",	"WG01",		"0.149451349150580593145776339657697"),
	CONSTANT("wk03",	"WK03",		"0.075039674810919952767043140916190"),

	CONSTANT("xgk05",	"XGK05",	"0.679409568299024406234327365114874"),
	CONSTANT("wg02",	"WG02",		"0.219086362515982043995534934228163"),
	CONSTANT("wk05",	"WK05",		"0.109387158802297641899210590325805"),

	CONSTANT("xgk07",	"XGK07",	"0.433395394129247190799265943165784"),
	CONSTANT("wg03",	"WG03",		"0.269266719309996355091226921569469"),
	CONSTANT("wk07",	"WK07",		"0.134709217311473325928054001771707"),

	CONSTANT("xgk09",	"XGK09",	"0.148874338981631210884826001129720"),
	CONSTANT("wg04",	"WG04",		"0.295524224714752870173892994651338"),
	CONSTANT("wk09",	"WK09",		"0.147739104901338491374841515972068"),

	CONSTANT(NULL, NULL, NULL)
};

/* Imperial/metric conversions.
 * Data taken from http://physics.nist.gov/Pubs/SP811/appenB9.html
 * In general, the values are rounded to 6 or 7 digits even though
 * more accurate values are known for many of these.
 */
struct constsml conversions[] = {
	CONV("kg",	"lb",		"KG_LBM",	"0.4535924"),		// source: NIST
	CONV("kg",	"stone",	"KG_STONE",	"6.3502936"),		// derived: 14 lbs to a stone
	CONV("kg",	"cwt",		"KG_CWT",	"50.8023488"),		// derived: 112lb to a long cwt
	CONV("kg",	"s.cwt",	"KG_SHCWT",	"45.35924"),		// source: NIST hundredweight, short 100lb
	CONV("g",	"oz",		"G_OZ",		"28.34952"),		// source: NIST
	CONV("g",	"tr.oz",	"G_TOZ",	"31.10348"),		// source: NIST
	CONV("l",	"galUK",	"L_GALUK",	"4.54609"),		// source: NIST
	CONV("l",	"galUS",	"L_GALUS",	"3.785412"),		// source: NIST
	CONV("l",	"cft",		"L_CUBFT",	"28.31685"),		// source: NIST
	CONV("ml",	"flozUK",	"ML_FLOZUK",	"28.41306"),		// source: NIST oz UK fluid
	CONV("ml",	"flozUS",	"ML_FLOZUS",	"29.57353"),		// source: NIST oz US fluid
	CONV("cm",	"inches",	"CM_INCH",	"2.54"),		// source: NIST
	CONV("m",	"fathom",	"M_FATHOM",	"1.828804"),		// derived: 6 feet
	CONV("m",	"feet",		"M_FEET",	"0.3048"),		// source: NIST
	CONV("m",	"yards",	"M_YARD",	"0.9144"),		// source: NIST
	CONV("km",	"miles",	"KM_MILE",	"1.609344"),		// source: NIST
	CONV("km",	"l.y.",		"KM_LY",	"9.46073E12"),		// source: NIST
	CONV("km",	"pc",		"KM_PC",	"3.085678E13"),		// source: NIST
	CONV("km",	"AU",		"KM_AU",	"149597900"),		// source: NIST, IAU 2009 gives 1.49597870700E11
	CONV("km",	"nmi",		"KM_NMI",	"1.852"),		// source: NIST
//	CONV("m\232",	"square",	"M_SQUARE",	"9.290304"),		// derived: 
//	CONV("m\232",	"perch",	"M_PERCH",	"25.29285264"),		// derived: 
	CONV("ha",	"acres",	"HA_ACRE",	"0.4046873"),		// source: NIST
	CONV("N",	"lbf",		"N_LBF",	"4.448222"),		// source: NIST
	CONV("J",	"Btu",		"J_BTU",	"1055.056"),		// source: NIST BTUit
	CONV("J",	"cal",		"J_CAL",	"4.1868"),		// source: NIST calorie it
	CONV("J",	"kWh",		"J_kWh",	"3600000"),		// source: NIST
	CONV("Pa",	"atm",		"Pa_ATM",	"101325"),		// source: NIST atmosphere standard
	CONV("Pa",	"bar",		"Pa_bar",	"100000"),		// source: NIST
	CONV("Pa",	"mmHg",		"Pa_mmHg",	"133.3224"),		// source: NIST cm mercury conventional
	CONV("Pa",	"psi",		"Pa_psi",	"6894.757"),		// source: NIST pound-force per square inch
	CONV("Pa",	"inHg",		"Pa_inhg",	"3386.389"),		// source: NIST inch of mercury conventional
	CONV("Pa",	"torr",		"Pa_torr",	"133.3224"),		// source: NIST
	CONV("W",	"hpUK",		"W_HPUK",	"745.70"),		// source: NIST horsepower UK
	CONV("W",	"hp",		"W_HP550",	"745.6999"),		// source: NIST horsepower 550 ft . lb / s
	CONV("W",	"PS(hp)",	"W_HP",		"735.4988"),		// source: NIST horsepower metric
	CONV("W",	"HP\274",	"W_HPe",	"746"),			// source: NIST horsepower electric
	CONV("t",	"tons",		"T_TON",	"1.016047"),		// source: NIST ton, long 2240lb
	CONV("t",	"s.tons",	"T_SHTON",	"0.9071847"),		// source: NIST ton, short 2000lb

	CONV(NULL, NULL, NULL, NULL)
};


static const char *gpl[] = {
	"This file is part of 34S.",
	"",
	"34S is free software: you can redistribute it and/or modify",
	"it under the terms of the GNU General Public License as published by",
	"the Free Software Foundation, either version 3 of the License, or",
	"(at your option) any later version.",
	"",
	"34S is distributed in the hope that it will be useful,",
	"but WITHOUT ANY WARRANTY; without even the implied warranty of",
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
	"GNU General Public License for more details.",
	"",
	"You should have received a copy of the GNU General Public License",
	"along with 34S.  If not, see <http://www.gnu.org/licenses/>.",
	"",
	"",
	"This file is automatically generated.	Changes will not be preserved.",
	NULL
};

static void gpl_text(FILE *f, const char *start, const char *middle, const char *end) {
	int i;

	for (i=0; gpl[i] != NULL; i++)
		fprintf(f, "%s%s\n", i==0?start:middle, gpl[i]);
	fprintf(f, "%s\n\n", end);
}


/* Compare two files and return non-zero if they are different in any way.
 */
static int compare_files(const char *const fn1, const char *const fn2) {
	FILE *f1, *f2;
	int c1, c2;

	if ((f1 = fopen(fn1, "r")) == NULL)
		return 1;
	if ((f2 = fopen(fn2, "r")) == NULL) {
		fclose(f1);
		return 1;
	}
	do
		if ((c1 = getc(f1)) != (c2 = getc(f2))) {
			fclose(f1);
			fclose(f2);
			return 1;
		}
	while (c1 != EOF);
	fclose(f1);
	fclose(f2);
	return 0;
}


static void output(FILE *fm, const char *name, const decNumber *d) {
	int i;
	int num;
	FILE *fc;
	char fname[1000];
	char tmpname[1000];
	static int body = 0;

	fprintf(fh, "extern const decNumber const_%s;\n", name);

	sprintf(fname, "const_%s.c", name);
	sprintf(tmpname, "tmp_const_%s.c", name);
	fc = fopen(tmpname, "w");
	gpl_text(fc, "/* ", " * ", " */");
	num = ((d->digits+DECDPUN-1)/DECDPUN);
	fprintf(fc,	"#include \"decNumber/decNumber.h\"\n"
			"\n"
			"const struct {\n"
			"\tint32_t digits;\n"
			"\tint32_t exponent;\n"
			"\tuint8_t bits;\n"
			"\tdecNumberUnit lsu[%d];\n"
			"} "
			"const_%s = {\n"
			"\t%d,\n"
			"\t%d,\n"
			"\t%u,\n"
			"\t{ ",
		num, name, d->digits, d->exponent, d->bits);
	for (i=0; i<num; i++) {
		if (i != 0)
			fprintf(fc, ", ");
		fprintf(fc, "%lu", (unsigned long int)d->lsu[i]);
	}
	fprintf(fc, " }\n};\n\n");
	fclose(fc);
	if (compare_files(tmpname, fname)) {
		unlink(fname);
		rename(tmpname, fname);
	} else
		unlink(tmpname);

	if (body)
		fprintf(fm, " \t\\\n");
	else	body = 1;
	fprintf(fm, "\tconst_%s.o", name);
}

static void const_big(void) {
	int n;
	decNumber x, y;
	decContext ctx;
	FILE *fm;

	mkdir("consts"
#ifndef WIN32
	,0755
#endif
	);
	if (chdir("consts") == -1)
		exit(1);
	fm = fopen("Makefile", "w");
	gpl_text(fm, "# ", "# ", "#\n");

	fprintf(fm, "OBJS=");

	decContextDefault(&ctx, DEC_INIT_BASE);
	ctx.traps = 0;
	ctx.digits = DECNUMDIGITS;
	ctx.emax=DEC_MAX_MATH;
	ctx.emin=-DEC_MAX_MATH;
	ctx.round = DEC_ROUND_HALF_EVEN;

	for (n=0; cnsts[n].name != NULL; n++) {
		ctx.digits = cnsts[n].ndig;
		decNumberFromString(&x, cnsts[n].value, &ctx);
		decNumberNormalize(&y, &x, &ctx);

		output(fm, cnsts[n].name, &y);
	}
	fprintf(fm, "\n\n.SILENT: $(OBJS)\n\n"
			"all: $(OBJS)\n"
			"\t@rm -f ../%slibconsts.a\n"
			"\t$(AR) q ../%slibconsts.a $(OBJS)\n"
			"\t$(RANLIB) ../%slibconsts.a\n\n",
			libconsts, libconsts, libconsts);
	fclose(fm);
	// And if the header is unchanged, don't touch that either
	if (chdir("..") == -1)
		exit(1);
}

static void put_name(FILE *f, const char *name) {
	while (*name != '\0') {
		const char ch = *name;
		if ((ch & 0x80) == 0 && isprint(ch))
			putc(ch, f);
		else
			fprintf(f, "\\%03o", 0xff & ch);
		name++;
	}
}

static void const_small_tbl(FILE *f, const int incname, const struct constsml ctbl[],
		const char *tname, const char *num_name, const char *macro_name,
		const char *value_name, const char *rarg_name, const char *rarg_complex,
		const char *comment) {
	int i, j;
	unsigned char *p;
	decimal64 d;
	decContext ctx;

	decContextDefault(&ctx, DEC_INIT_DECIMAL64);
	for (i=0; ctbl[i].val != NULL; i++);
	fprintf(fh, "\nstruct %s {\n"
			"\tdecimal64 x;\n", tname);
	if (incname > 0)
		fprintf(fh, "\tconst char cname[CONST_NAMELEN];\n");
	else if (incname < 0)
		fprintf(fh, "\tconst char metric[METRIC_NAMELEN];\n"
				"\tconst char imperial[IMPERIAL_NAMELEN];\n");
	fprintf(fh, "};\n\n");

	fprintf(fh,	"/* %s */\n"
			"extern const struct %s %s[];\n"
			"#define %s %d\n"
			"#define %s(n)	(%s[n].x)\n\n"
			"enum {\n", comment,
				tname, tname, num_name, i, macro_name, tname);
	fprintf(f, "/* %s\n */\nconst struct %s %s[] = {\n", comment, tname, tname);
	if (incname > 0)
		fprintf(f, "#define CNST(n, b1,b2,b3,b4,b5,b6,b7,b8, fn, nul) { B(b1,b2,b3,b4,b5,b6,b7,b8), fn },\n");
	else if (incname < 0)
		fprintf(f, "#define CNST(n, b1,b2,b3,b4,b5,b6,b7,b8, fm, fi) { B(b1,b2,b3,b4,b5,b6,b7,b8), fm, fi },\n");
	else
		fprintf(f, "#define CNST(n, b1,b2,b3,b4,b5,b6,b7,b8, fn, nul) { B(b1,b2,b3,b4,b5,b6,b7,b8) },\n");
	for (i=0; ctbl[i].val != NULL; i++) {
		fprintf(fh, "\tOP_%s", ctbl[i].op);
		if (i == 0)
			fprintf(fh, " = 0");
		fprintf(fh, ",\n");

		fprintf(f, "\tCNST(OP_%s, ", ctbl[i].op);
		decimal64FromString(&d, ctbl[i].val, &ctx);
		p = (unsigned char *)&d;
		for (j=0; j<8; j++)
			fprintf(f, "0x%02x, ", p[7-j]);
		fprintf(f, "\"");
		put_name(f, ctbl[i].name);
		fprintf(f, "\", \"");
		put_name(f, ctbl[i].n2);
		fprintf(f, "\")\n");
	}
	fprintf(fh, "};\n"
			"#define %s(n) RARG(%s, n)\n",
			value_name, rarg_name);
	if (rarg_complex != NULL)
		fprintf(fh, "#define %s_CMPLX(n) RARG(%s, n)\n", value_name, rarg_complex);
	fprintf(fh, "\n");
	fprintf(f, "#undef CNST\n"
			"};\n\n");
}


static void unpack(const char *b, int *u) {
	while (*b != 0 && *b != ' ') {
		*u++ = remap_chars(0xff & *b++);
	}
	*u = -1;
}

static int const_small_compare(const void *v1, const void *v2) {
	const struct constsml *cs1 = (const struct constsml *)v1;
	const struct constsml *cs2 = (const struct constsml *)v2;
	int u1[16], u2[16];
	int i;

	for (i=0; i<16; i++)
		u1[i] = u2[i] = 0;

	unpack(cs1->name, u1);
	unpack(cs2->name, u2);

	for (i=0; i<16; i++) {
		if (u1[i] < u2[i]) return -1;
		else if (u1[i] > u2[i]) return 1;
		else if (u1[i] == -1) break;
	}
	return strcmp(cs1->name, cs2->name);
}

static void const_small_sort(struct constsml ctbl[]) {
	int n;

	for (n=0; ctbl[n].val != NULL; n++);
	qsort(ctbl, n, sizeof(struct constsml), &const_small_compare);
}


static const unsigned char charlengths[512+5] = {
#define C(len, a, b, c, x, y, z)	len
#include "charset.h"
#undef C
	0, 0, 0, 0, 0
};

static void const_small(FILE *fh) {
	FILE *f;
	int i, n=1;

	f = fopen(consts_c, "w");
	if (f == NULL)
		exit(1);
	gpl_text(f, "/* ", " * ", " */");
	fprintf(f,	"#include \"consts.h\"\n\n"
			"#if BYTE_ORDER == BIG_ENDIAN\n"
			"#define B(b1,b2,b3,b4,b5,b6,b7,b8) {{ b1,b2,b3,b4,b5,b6,b7,b8 }}\n"
			"#else\n"
			"#define B(b1,b2,b3,b4,b5,b6,b7,b8) {{ b8,b7,b6,b5,b4,b3,b2,b1 }}\n"
			"#endif\n\n"
		);
	const_small_sort(constsml);
	const_small_tbl(f, 1, constsml, "cnsts", "NUM_CONSTS",
				"CONSTANT", "CONST", "RARG_CONST", "RARG_CONST_CMPLX",
				"Table of user visible constants");
	const_small_tbl(f, 0, constsint, "cnsts_int", "NUM_CONSTS_INT",
				"CONSTANT_INT", "CONST_INT", "RARG_CONST_INT", NULL,
				"Table of intenally used constants");
	const_small_tbl(f, -1, conversions, "cnsts_conv", "NUM_CONSTS_CONV",
				"CONSTANT_CONV", "CONST_CONV", "RARG_CONST_CONV", NULL,
				"Table of metric/imperial conversion constants");
	fprintf(f, "\n#undef B\n\n");

	fprintf(f, "const unsigned short int charlengthtbl[%d] = {\n\t", (512 + 4) / 5);
	for (i=0; i<512; i+=5) {
		unsigned short val = 0;
		int j;
		for (j=4; j>=0; j--)
			val = (val << 3) + (7 & charlengths[i+j]);
		fprintf(f, "%5u,%s", val, (n%8)==0?"\n\t":" ");
		n++;
	}
	fprintf(f, "\n};\n\n");
	fclose(f);
}

int main(int argc, char *argv[]) 
{
	char tmp[ FILENAME_MAX ] = "";
	if ( argc > 1 ) {
		// Pathname given
		// Acts as a prefix so be careful to supply the path delimiter
		strcpy( consts_h, argv[1] );
		strcpy( consts_c, argv[1] );
		strcpy( tmp, argv[1] );
	}
	strcat( consts_h, "consts.h" );
	strcat( consts_c, "consts.c" );
	strcat( tmp, "tmp_consts.h" );
	if ( argc > 2 ) {
		// Path for libconsts.a in makefile
		libconsts = argv[2];
	}
	fh = fopen(consts_h, "w");
	gpl_text(fh, "/* ", " * ", " */");
	fprintf(fh,	"#ifndef __CONSTS_H__\n"
			"#define __CONSTS_H__\n"
			"\n"
			"#include \"xeq.h\"\n"
			"\n"
			"#define CONST_NAMELEN 4\n"
			"#define METRIC_NAMELEN 2\n"
			"#define IMPERIAL_NAMELEN 6\n"
			"\n\n");
	fprintf(fh, "extern const unsigned short int charlengthtbl[];\n\n");
	const_small(fh);
	fprintf(fh, "\n\n");
	const_big();
	fprintf(fh,	"\n#endif\n");
	fclose(fh);
	return 0;
}

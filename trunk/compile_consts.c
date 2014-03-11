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

#define CONST_NAMELEN		4
#define METRIC_NAMELEN		2
#define IMPERIAL_NAMELEN	6

#define xcopy memcpy


#include "licence.h"
#include "charmap.c"

#include "features.h"

#define NEED_D64FROMSTRING  1
// #define NEED_D128FROMSTRING  1

#include "decNumber.h"
#include "decimal64.h"
#include "decimal128.h"

static FILE *fh, *fu;

static char consts_h[ FILENAME_MAX ];
static char consts_c[ FILENAME_MAX ];
static char user_consts_h[ FILENAME_MAX ];
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
	{ DFLT,  "15",			"15"		},
	{ DFLT,  "16",			"16"		},
	{ DFLT,  "20",			"20"		},
	{ DFLT,  "21",			"21"		},
	{ DFLT,  "32",			"32"		},
	{ DFLT,  "60",			"60"		},
	{ DFLT,  "90",			"90"		},
	{ DFLT,  "100",			"100"		},
	{ DFLT,  "150",			"150"		},
	{ DFLT,  "256",			"256"		},
	{ DFLT,  "300",			"300"		},
	{ DFLT,  "360",			"360"		},
	{ DFLT,  "400",			"400"		},
	{ DFLT,  "500",			"500"		},
	{ DFLT,  "9000",		"9000"		},
	{ DFLT,  "36000",		"36000"		},
	{ DFLT,  "100000",		"100000"	},
	{ DFLT,  "1e_32",		"1e-32"		},
	{ DFLT,  "1e_37",		"1e-37"		},
	{ DFLT,  "1e_24",		"1e-24"		},
	{ DFLT,  "1e_14",		"1e-14"		},
	{ DFLT,  "hms_threshold",	"0.000001388888888888888888888888888888888888888888888"	},
	{ DFLT,  "0_0001",		"0.0001"	},
	{ DFLT,  "0_001",		"0.001"		},
	{ DFLT,  "0_1",			"0.1"		},
	{ DFLT,  "0_0195",		"0.0195"	},
	{ DFLT,  "0_2",			"0.2"		},
	{ DFLT,  "0_2214",		"0.2214"	},
	{ DFLT,  "0_25",		"0.25"		},
	{ DFLT,  "0_04",		"0.04"		},
	{ DFLT,  "0_05",		"0.05"		},
	{ DFLT,  "0_4",			"0.4"		},
	{ DFLT,  "0_5",			"0.5"		},
	{ DFLT,  "_0_5",		"-0.5"		},
	{ DFLT,  "0_6",			"0.6"		},
	{ DFLT,  "0_665",		"0.665"		},
	{ DFLT,  "0_75",		"0.75"		},
	{ DFLT,  "0_85",		"0.85"		},
	{ DFLT,  "0_9",			"0.9"		},
	{ DFLT,  "0_97",		"0.97"		},
	{ DFLT,  "2on3",		"0.666666666666666666666666666666666666666666666666666" },
	{ DFLT,  "5on6",		"0.833333333333333333333333333333333333333333333333333" },
	{ DFLT,  "1on60",		".0166666666666666666666666666666666666666666666666666" },
	{ DFLT,  "1_3",			"1.3"		},
	{ DFLT,  "1_5",			"1.5"		},
	{ DFLT,  "1_7",			"1.7"		},
	{ DFLT,  "9on5",		"1.8"		},
	{ DFLT,  "2_326",		"2.326"		},
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
	{ DFLT,  "egamma",		"0.5772156649015328606065120900824024310421593359399235988" },
	{ DFLT,  "_1onPI",		"-0.31830988618379067153776752674502872406891929148091" },
	{ DFLT,  "2pow64",		"18446744073709551616" },

	// randfac = 2^-32  This converts a 32 integer into a [0, 1) interval
	{ DFLT, "randfac",		"0.00000000023283064365386962890625" },

	// Gamma estimate constants
	{ DFLT, "gammaR",		"23.118910" },
	{ DFLT, "gammaC00",		"2.5066282746310005024157652848102462181924349228522"},
	{ DFLT, "gammaC01",		"18989014209.359348921215164214894448711686095466265"},
	{ DFLT, "gammaC02",		"-144156200090.5355882360184024174589398958958098464"},
	{ DFLT, "gammaC03",		"496035454257.38281370045894537511022614317130604617"},
	{ DFLT, "gammaC04",		"-1023780406198.473219243634817725018768614756637869"},
	{ DFLT, "gammaC05",		"1413597258976.513273633654064270590550203826819201"},
	{ DFLT, "gammaC06",		"-1379067427882.9183979359216084734041061844225060064"},
	{ DFLT, "gammaC07",		"978820437063.87767271855507604210992850805734680106"},
	{ DFLT, "gammaC08",		"-512899484092.42962331637341597762729862866182241859"},
	{ DFLT, "gammaC09",		"199321489453.70740208055366897907579104334149619727"},
	{ DFLT, "gammaC10",		"-57244773205.028519346365854633088208532750313858846"},
	{ DFLT, "gammaC11",		"12016558063.547581575347021769705235401261600637635"},
	{ DFLT, "gammaC12",		"-1809010182.4775432310136016527059786748432390309824"},
	{ DFLT, "gammaC13",		"189854754.19838668942471060061968602268245845778493"},
	{ DFLT, "gammaC14",		"-13342632.512774849543094834160342947898371410759393"},
	{ DFLT, "gammaC15",		"593343.93033412917147656845656655196428754313318006"},
	{ DFLT, "gammaC16",		"-15403.272800249452392387706711012361262554747388558"},
	{ DFLT, "gammaC17",		"207.44899440283941314233039147731732032900399915969"},
	{ DFLT, "gammaC18",		"-1.2096284552733173049067753842722246474652246301493"},
	{ DFLT, "gammaC19",		".0022696111746121940912427376548970713227810419455318"},
	{ DFLT, "gammaC20",		"-.00000079888858662627061894258490790700823308816322084001"},
	{ DFLT, "gammaC21",		".000000000016573444251958462210600022758402017645596303687465"},

	{ -1,  NULL,	  NULL		  }
};

struct _constsml {
	const char *name;
	const char *op;
	const char *val;
	const char *n2;
};
#define CONSTANT(n, op, val)		{ n, op, val, "" }
#define SYSCONST(n, op, val)		{ n, op, val, "-" }
#define CONV(n1, n2, op, val)		{ n1, op, val, n2 }

struct _constsml constsml[] = {
	CONSTANT("#",		"ZERO",		"0"),			// Zero & placeholder for small integers
	CONSTANT("a",		"PC_a",		"365.2425"),		// Days in a Gregorian year
	CONSTANT("a\270",	"PC_a0",	"5.2917721092E-11"),	// Bohr radius
	CONSTANT("c",		"PC_C",		"299792458"),		// Speed of light in a vacuum
	CONSTANT("c\271",	"PC_C1",	"3.74177153E-16"),	// First radiation constant
	CONSTANT("c\272",	"PC_C2",	"1.438770E-2"),		// Second radiation constant
	CONSTANT("F",		"PC_F",		"96485.3365"),		// Faraday's constant
	CONSTANT("g",		"PC_g",		"9.80665"),		// Standard Earth acceleration
	CONSTANT("G",		"PC_G",		"6.67384E-11"),		// NIST 2010, IAU 2009 gives 6.67428E-11
	CONSTANT("G\270",	"PC_Go",	"7.7480917346E-5"),	// Conductace quantum
	CONSTANT("g\274",	"PC_Ge",	"-2.00231930436153"),	// Lande's electon g-factor
	CONSTANT("k",		"PC_k",		"1.3806488E-23"),	// Boltzmann constant
	CONSTANT("Kj",		"PC_Jk",	"483597.870E9"),	// Josephson constant
	CONSTANT("N\327",	"PC_Na",	"6.02214129E23"),	// Avogadro's number
	CONSTANT("p\270",	"PC_atm",	"101325"),		// Standard atmospheric pressure
	CONSTANT("R",		"PC_R",		"8.3144621"),		// Molar gas constant
	CONSTANT("R\367",	"PC_Rk",	"25812.8074434"),	// von Klitzing's constant
	CONSTANT("R\233",	"PC_Rinf",	"10973731.568539"),	// Rydberg constant
	CONSTANT("T\270",	"PC_t",		"273.15"),		// 0 degree celcus, standard temperature
	CONSTANT("V\033",	"PC_Vm",	"22.413968E-3"),	// Molar volume of an ideal gas at STP
	CONSTANT("Z\270",	"PC_Zo",	"376.73031346177065546819840042031930826862350835241864672"),	// Characteristic impedance of vacuum
	CONSTANT("\240",	"PC_alpha",	"7.2973525698E-3"),	// Fine-structure constant
	CONSTANT("\242\276",	"PC_gamP",	"2.675222005E8"),	// Proton gyromagnetic ratio
	CONSTANT("\244\270",	"PC_eps0",	"8.8541878176203898505365630317107502606083701665994498E-12"),	// Electric constant, vacuum permittivity
	CONSTANT("\253\270",	"PC_mu0",	"12.566370614359172953850573533118011536788677597500423E-7"),	// Magnetic constant
	CONSTANT("\253\230",	"PC_muB",	"927.400968E-26"),	// Bohr's magneton
	CONSTANT("\253\277",	"PC_mu_u",	"5.05078353E-27"),	// Nuclear magneton
	CONSTANT("\261\230",	"PC_sigma",	"5.670373E-8"),		// Stefan Boltzmann constant
	CONSTANT("\224\270",	"PC_phi0",	"2.067833758E-15"),	// Magnetic flux quantum

	/* Plank related constants */
	CONSTANT("h",		"PC_PLANK",	"6.62606957E-34"),	// Planck's constant
	CONSTANT("\236",	"PC_hon2PI",	"1.054571726E-34"),	// Planck's constant over 2 pi.
	CONSTANT("l\276",	"PC_PlanckL",	"1.616199E-35"),	// Base Planck unit of length
	CONSTANT("M\276",	"PC_PlanckM",	"2.17651E-8"),		// Base Planck unit of mass
/**/	CONSTANT("q\276",	"PC_PlanckQ",	"1.8755459e-18"),	// Base Planck unit of charge
	CONSTANT("T\276",	"PC_PlanckTh",	"1.416833E32"),		// Base Planck unit of temperature
	CONSTANT("t\276",	"PC_tp",	"5.39106E-44"),		// Base Planck unit of time

	/* Atomic constants */
	CONSTANT("e",		"PC_eV",	"1.602176565E-19"),	// Electron Charge
	CONSTANT("r\274",	"PC_Re",	"2.8179403267E-15"),	// Classical electron radius
	CONSTANT("m\274",	"PC_me",	"9.10938291E-31"),	// Electron mass
	CONSTANT("m\275",	"PC_mn",	"1.674927351E-27"),	// Neutron mass
	CONSTANT("m\276",	"PC_mp",	"1.672621777E-27"),	// Proton mass
	CONSTANT("m\277",	"PC_mu",	"1.660538921E-27"),	// Atomic mass unit
	CONSTANT("m\231",	"PC_mMu",	"1.883531475E-28"),	// Muon mass
//	CONSTANT("m\274c\232",	"PC_mec2",	"8.18710506E-14"),	// Electron mass by c^2
//	CONSTANT("m\275c\232",	"PC_mnc2",	"1.505349631E-10"),	// Neutron mass by c^2
//	CONSTANT("m\276c\232",	"PC_mpc2",	"1.503277484E-10"),	// Proton mass by c^2
	CONSTANT("m\277c\232",	"PC_muc2",	"1.492417954E-10"),	// Atomic mass unit by c^2
//	CONSTANT("m\231c\232",	"PC_mMuc2",	"1.692833667E-11"),	// Muon mass by c^2
	CONSTANT("\252\273",	"PC_lamC",	"2.4263102389E-12"),	// Compton wavelength of the electron
	CONSTANT("\252\273\275","PC_lamCn",	"1.3195909068E-15"),	// Compton wavelength of the neutron
	CONSTANT("\252\273\276","PC_lamCp",	"1.32140985623E-15"),	// Compton wavelength of the proton
	CONSTANT("\253\274",	"PC_muE",	"-928.476430E-26"),	// Electron magnetic moment
	CONSTANT("\253\275",	"PC_mun",	"-0.96623647E-26"),	// Neutron magnetic moment
	CONSTANT("\253\276",	"PC_muP",	"1.410606743E-26"),	// Proton magnetic moment
	CONSTANT("\253\231",	"PC_mumu",	"-4.49044807E-26"),	// Muon magnetic moment

	/* Mathematical constants */
	CONSTANT("NaN",		"NAN",		"NaN"),
	CONSTANT("\237",	"INF",		"inf"),
	CONSTANT("-\237",	"NEGINF",	"-inf"),
	CONSTANT("\242EM",	"EULER",	"0.5772156649015328606065120900824024310421593359399235988"),
	CONSTANT("\224",	"PHI",		"1.61803398874989484820458683436563811772030917980576"),
	CONSTANT("G\273",	"PC_catalan",	"0.915965594177219015054603514932384110774149374281672134266498"),
	CONSTANT("eE",		"CNSTE",	"2.71828182845904523536028747135266249775724709369995"),
	CONSTANT("F\243",	"PC_F_delta",	"4.669201609102990671853203820466201617258185577475768632745651"),
	CONSTANT("F\240",	"PC_F_alpha",	"2.502907875095892822283902873218215786381271376727149977336192"),
	//CONSTANT("\207",	"PC_MILLS",	"1.3063778838630806904686144926026057129167845851567136443680537"),
	//CONSTANT("\252",	"PC_lam",	"0.62432998854355087099293638310083724417964262018"),
	//CONSTANT("\217\272",	"PI2",		"0.660161815846869573927812110014555778432623336"),
	//CONSTANT("Z",		"PC_apery",	"1.202056903159594285399738161511449990764986292"),	// Apery's constant = zeta(3)
	//CONSTANT("Gaus",	"PC_gauss",	"0.83462684167407318628142973279904680899399301349"),	// Gauss's constant = 1 / AGM(1, sqrt(2))
	//CONSTANT("K",		"PC_khinchin",	"2.685452001065306445309714835481795693820382293994462953051152345557218859537152"),
	CONSTANT("1/2",		"HALF",		"0.5"),

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
	CONSTANT("a\033",	"PC_SM_luna",	"384400E3"),		// Semi-major axis Moon NASA Horizons
	CONSTANT("a\256",	"PC_SM_terra",	"149.5979E9"),		// Semi-major axis Earth NASA Earth fact sheet
//	CONSTANT("H\270",	"PC_Hubble",	"70.1"),		// Hubble constant

	/* These are used by internal routines */
	SYSCONST("1",		"ONE",		"1"),			// One
	SYSCONST("\257",	"PI",		"3.14159265358979323846264338327950288419716939937510"),
	SYSCONST("1/\003""5",	"RECIP_SQRT5",	"0.4472135954999579392818347337462552470881236719223"),
	SYSCONST("1/\003\257",	"RECIP_SQRTPI",	"0.564189583547756286948079451560772585844050629329"),
	SYSCONST("\003""2\257",	"SQRT_2_PI",	"2.50662827463100050241576528481104525300698674060994"),
	SYSCONST("\004RgB",	"INT_R_BOUNDS",	"122.134"),
	SYSCONST("\257/2",	"PIon2",	"1.57079632679489661923132169163975144209858469968755"),
	SYSCONST("LN2",		"LN2",		"0.6931471805599453094172321214581765680755001343602553"),
	SYSCONST("LN2\235",	"RECIPLN2",	"1.4426950408889634073599246810018921374266459541529859341354"),
	SYSCONST("L10\235",	"RECIPLN10",	"0.4342944819032518276511289189166050822943970058036665661144"),
	SYSCONST("Chi2",	"POINT_2214",	".2214"),
	SYSCONST("1/eH",	"HIGH_RECIP_E",	"0.36787944117144232159552377016146"),
	SYSCONST("1/eL",	"LOW_RECIP_E",	"8.674458111310317678345078368016975E-34"),

#ifdef INCLUDE_XROM_DIGAMMA
	SYSCONST("DG02",	"DG02",		"-12"),
	SYSCONST("DG04",	"DG04",		"120"),
	SYSCONST("DG06",	"DG06",		"-252"),
	SYSCONST("DG08",	"DG08",		"240"),
	SYSCONST("DG10",	"DG10",		"-132"),
	SYSCONST("DG12",	"DG12",		"47.40955137481910274963820549927641099855282199710564"),
	SYSCONST("DG14",	"DG14",		"-12"),
	SYSCONST("DG16",	"DG16",		"2.25601327066629803704727674868675698092341719657174"),
	SYSCONST("DG18",	"DG18",		"-0.32744432033191237148653885608771969817858526910889"),
	SYSCONST("DG20",	"DG20",		"0.03779830594865157407036211922502018773158621163615"),
#ifdef XROM_DIGAMMA_DOUBLE_PRECISION
	SYSCONST("DG22",	"DG22",		"-0.00355290089208707181751477157164373157576303695789"),
	SYSCONST("DG24",	"DG24",		"0.00027719946681748709451809242885375511629640900063"),
	SYSCONST("DG26",	"DG26",		"-0.000018238994666613976237629781846424625074665884416450965222"),
	SYSCONST("DG28",	"DG28",		"0.000001025707487435377285588673305803802205519095192937062420"),
	SYSCONST("DG30",	"DG30",		"-0.000000049868606702005666912020069073686577470166427786528917"),
	SYSCONST("DG32",	"DG32",		"0.0000000021169179377466567237109532498135231238180401887003845789189798"),
	SYSCONST("DG34",	"DG34",		"-0.0000000000791406916620372916230853867700251482720064744876389233715498"),
#endif
#endif
	CONSTANT(NULL, NULL, NULL)
};

/* Imperial/metric conversions.
 * Data taken from http://physics.nist.gov/Pubs/SP811/appenB9.html
 * In general, the values are rounded to 6 or 7 digits even though
 * more accurate values are known for many of these.
 */
struct _constsml conversions[] = {
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
	CONV("m",	"fathom",	"M_FATHOM",	"1.8288"),		// derived: 6 feet
	CONV("m",	"feet",		"M_FEET",	"0.3048"),		// source: NIST
	CONV("m",	"feetUS",	"M_FEETUS",	"0.3048006096"),		// source: Wikipedia etc
	CONV("m",	"yards",	"M_YARD",	"0.9144"),		// source: NIST
	CONV("km",	"miles",	"KM_MILE",	"1.609344"),		// source: NIST
	CONV("km",	"l.y.",		"KM_LY",	"9.46073E12"),		// source: NIST
	CONV("km",	"pc",		"KM_PC",	"3.085678E13"),		// source: NIST
	CONV("km",	"AU",		"KM_AU",	"149597900"),		// source: NIST, IAU 2009 gives 1.49597870700E11
	CONV("km",	"nmi",		"KM_NMI",	"1.852"),		// source: NIST
//	CONV("m\232",	"square",	"M_SQUARE",	"9.290304"),		// derived: 
//	CONV("m\232",	"perch",	"M_PERCH",	"25.29285264"),		// derived: 
	CONV("ha",	"acres",	"HA_ACREUK",	"0.40468564224"),	// derived: 43560 square feet
	CONV("ha",	"acreUS",	"HA_ACREUS",	"0.4046873"),		// source: NIST
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
	license(fc, "/* ", " * ", " */");
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
	license(fm, "# ", "# ", "#\n");

	fprintf(fm, "OBJS=");

	decContextDefault(&ctx, DEC_INIT_BASE);
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
		if ((ch & 0x80) == 0 && isprint((unsigned char) ch))
			putc(ch, f);
		else
			fprintf(f, "\\%03o", 0xff & ch);
		name++;
	}
}

/*
 	const_small_tbl(f, 
				"CONSTANT", "CONST", "RARG_CONST", "RARG_CONST_CMPLX",
*/
static void const_small_tbl(FILE *f) {
	int i, j, s_index, d_index;
	unsigned char *p;
	decimal64  s_tab[128];
	decimal128 d_tab[128];
	decContext ctx, ctx64, ctx128;
	decNumber x, y;

	decContextDefault(&ctx64,  DEC_INIT_DECIMAL64);
	decContextDefault(&ctx128, DEC_INIT_DECIMAL128);
	decContextDefault(&ctx, DEC_INIT_BASE);
	ctx.digits = DECIMAL128_Pmax;
	ctx.emax=DEC_MAX_MATH;
	ctx.emin=-DEC_MAX_MATH;
	ctx.round = DEC_ROUND_HALF_EVEN;

	for (i=0; constsml[i].val != NULL && constsml[i].n2[0] == '\0'; i++);	// user visible
	for (j=i; constsml[j].val != NULL; j++);				// system	
	fprintf(fh,	"\nstruct cnsts {\n"
			"\tunsigned char index;\n"
			"\tconst char cname[CONST_NAMELEN];\n"
			"#if ! defined(REALBUILD) || defined(COMPILE_CATALOGUES)\n"
			"\tconst char *alias;\n"
			"#endif\n"
			"};\n\n");

	fprintf(fh,	"/* Table of user visible constants */\n"
			"extern const struct cnsts cnsts[];\n"
			"#define NUM_CONSTS_CAT  %d\n"
			"#define NUM_CONSTS      %d\n"
			"#define CONSTANT(n)     ((decimal64 *)  get_const(n, 0))\n"
			"#define CONSTANT_DBL(n) ((decimal128 *) get_const(n, 1))\n\n"
			"#define CONST(n)        RARG(RARG_CONST, n)\n"
			"#define CONST_CMPLX(n)  RARG(RARG_CONST_CMPLX, n)\n\n"
			"enum {\n",
		i, j);

	fprintf(f,	"/* Table of user visible constants\n */\nconst struct cnsts cnsts[] = {\n");
	fprintf(f,	"#if ! defined(REALBUILD) || defined(COMPILE_CATALOGUES)\n"
			"#define CNST(n, index, fn, alias) { index, fn, alias },\n"
			"#else\n"
			"#define CNST(n, index, fn, alias) { index, fn },\n"
			"#endif\n\n");

	s_index = 0; d_index = 0;
	for (i = 0; constsml[i].val != NULL; i++) {
		const char *op = constsml[i].op;
		const char *alias = op;
		if (strncmp(alias, "PC_", 3) == 0)
			alias += 3;
		if (strcmp(alias, constsml[i].name) == 0)
			alias = NULL;

		fprintf(fh, "\tOP_%s", op);
		if (i == 0)
			fprintf(fh, " = 0");
		fprintf(fh, ",\n");

		fprintf(fu, "#define CONST_%s (%d)\n", op, i);

		decNumberFromString(&x, constsml[i].val, &ctx);
		decNumberNormalize(&y, &x, &ctx);
		if (y.digits > DECIMAL64_Pmax || i <= 1) {
			decimal128FromNumber(d_tab + d_index, &y, &ctx128);
			j = d_index | 0x80;
			d_index += 1;
		}
		if (y.digits <= DECIMAL64_Pmax || i <= 1) {
			decimal64FromNumber(s_tab + s_index, &y, &ctx128);
			j = s_index;
			s_index += 1;
		}
		if ( s_index >= 128 || d_index >= 128 ) {
			fprintf(stderr, "Too many small constants defined\n");
			abort();
		}
		fprintf(f, "\tCNST(OP_%s, 0x%02x, \"", op, j);
		put_name(f, constsml[i].name);
		if(alias==NULL) {
			fprintf(f, "\", CNULL)\n" );
		} else {
			fprintf(f, "\", \"%s\")\n", alias);
		}
	}
	fprintf(fh, "};\n\n");			// enum
	fprintf(f, "#undef CNST\n};\n\n");	// array of structs

	fprintf(fh, "extern const decimal64 cnsts_d64[];\n");
	fprintf(f, "const decimal64 cnsts_d64[] = {\n");
	for (i = 0; i < s_index; i++) {
		fprintf(f, "\tB(");
		p = (unsigned char *) (s_tab + i);
		for (j = 0; j < 8; j++)
			fprintf(f, "0x%02x%c", p[ 7 - j ], j == 7 ? ')' : ',');
		fprintf(f, ",\n");
	}
	fprintf(f, "};\n\n");			// array of decial64

	fprintf(fh, "extern const decimal128 cnsts_d128[];\n");
	fprintf(f, "const decimal128 cnsts_d128[] = {\n");
	for (i = 0; i < d_index; i++) {
		fprintf(f, "\tD(");
		p = (unsigned char *) (d_tab + i);
		for (j = 0; j < 16; j++)
			fprintf(f, "0x%02x%c", p[ 15 - j ], j == 15 ? ')' : ',');
		fprintf(f, ",\n");
	}
	fprintf(f, "};\n\n");			// array of decimal128
}


static void const_conv_tbl(FILE *f) {
	int i, j;
	unsigned char *p;
	decimal64 d;
	decContext ctx;

	decContextDefault(&ctx, DEC_INIT_DECIMAL64);
	for (i=0; conversions[i].val != NULL; i++);
	fprintf(fh, "\nstruct cnsts_conv {\n"
			"\tdecimal64 x;\n"
			"\tconst char metric[METRIC_NAMELEN];\n"
			"\tconst char imperial[IMPERIAL_NAMELEN];\n"
		    "};\n\n");

	fprintf(fh,	"/* Table of metric/imperial conversion constants */\n"
			"extern const struct cnsts_conv cnsts_conv[];\n"
			"#define NUM_CONSTS_CONV %d\n"
			"#define CONSTANT_CONV(n)\t(cnsts_conv[n].x)\n\n"
			"enum {\n",
		i);
	fprintf(f,	"/* Table of metric/imperial conversion constants\n */\n"
			"const struct cnsts_conv cnsts_conv[] = {\n");
	fprintf(f, "#define CNST(n, b1,b2,b3,b4,b5,b6,b7,b8, fm, fi) { B(b1,b2,b3,b4,b5,b6,b7,b8), fm, fi },\n");

	for (i=0; conversions[i].val != NULL; i++) {
		fprintf(fh, "\tOP_%s", conversions[i].op);
		if (i == 0)
			fprintf(fh, " = 0");
		fprintf(fh, ",\n");

		fprintf(f, "\tCNST(OP_%s, ", conversions[i].op);
		decimal64FromString(&d, conversions[i].val, &ctx);
		p = (unsigned char *)&d;
		for (j=0; j<8; j++)
			fprintf(f, "0x%02x, ", p[7-j]);
		fprintf(f, "\"");
		put_name(f, conversions[i].name);
		fprintf(f, "\", \"");
		put_name(f, conversions[i].n2);
		fprintf(f, "\")\n");
	}
	fprintf(fh, "};\n#define CONST_CONV(n) RARG(RARG_CONST_CONV, n)\n\n");
	fprintf(f, "#undef CNST\n};\n\n");
}

static void unpack(const char *b, int *u) {
	while (*b != 0 && *b != ' ') {
		*u++ = remap_chars(0xff & *b++);
	}
	*u = -1;
}

static int const_small_compare(const void *v1, const void *v2) {
	const struct _constsml *cs1 = (const struct _constsml *)v1;
	const struct _constsml *cs2 = (const struct _constsml *)v2;
	int u1[16], u2[16];
	int i;

	if (cs1->n2[0] != cs2->n2[0])
		return cs1->n2[0] - cs2->n2[0];		// System constants moved to high index

	for (i=0; i<16; i++)
		u1[i] = u2[i] = 0;

	unpack(cs1->name, u1);
	unpack(cs2->name, u2);

	for (i=0; i<16; i++) {
		if (u1[i] < u2[i]) return -1;
		else if (u1[i] > u2[i]) return 1;
		else if (u1[i] == -1) break;
	}
	return - strcmp(cs1->name, cs2->name);
}

static void const_small_sort(struct _constsml ctbl[]) {
	int n;

	for (n=0; ctbl[n].val != NULL; n++);
	qsort(ctbl, n, sizeof(struct _constsml), &const_small_compare);
}


static void const_small(FILE *fh) {
	FILE *f;

	f = fopen(consts_c, "w");
	if (f == NULL)
		exit(1);
	license(f, "/* ", " * ", " */");
	fprintf(f,	"#include \"consts.h\"\n\n"
			"#if BYTE_ORDER == BIG_ENDIAN\n"
			"#define B(b1,b2,b3,b4,b5,b6,b7,b8) {{ b1,b2,b3,b4,b5,b6,b7,b8 }}\n"
			"#else\n"
			"#define B(b1,b2,b3,b4,b5,b6,b7,b8) {{ b8,b7,b6,b5,b4,b3,b2,b1 }}\n"
			"#endif\n\n"
		);
	fprintf(f,	"#if BYTE_ORDER == BIG_ENDIAN\n"
			"#define D(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16) \\\n"
			"       {{ b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16 }}\n"
			"#else\n"
			"#define D(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16) \\\n"
			"       {{ b16,b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1 }}\n"
			"#endif\n\n"
		);
	const_small_sort(constsml);
	const_small_tbl(f);
	const_conv_tbl(f);
	fprintf(f, "\n#undef B\n");
	fprintf(f, "#undef D\n\n");
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
		strcpy( user_consts_h, argv[1] );
		strcpy( tmp, argv[1] );
	}
	strcat( consts_h, "consts.h" );
	strcat( consts_c, "consts.c" );
	strcat( user_consts_h, "user_consts.h" );
	strcat( tmp, "tmp_consts.h" );
	if ( argc > 2 ) {
		// Path for libconsts.a in makefile
		libconsts = argv[2];
	}

	fh = fopen(consts_h, "w");
	license(fh, "/* ", " * ", " */");
	fprintf(fh,	"#ifndef __CONSTS_H__\n"
			"#define __CONSTS_H__\n"
			"\n"
			"#include \"xeq.h\"\n"
			"\n"
			"#define CONST_NAMELEN %d\n"
			"#define METRIC_NAMELEN %d\n"
			"#define IMPERIAL_NAMELEN %d\n"
			"\n\n", CONST_NAMELEN, METRIC_NAMELEN, IMPERIAL_NAMELEN);
	fu = fopen(user_consts_h, "w");
	if (fu == NULL) {
		perror(user_consts_h);
		abort();
	}
	license(fu, "/* ", " * ", " */");
	fprintf(fu,	"/* This file is for compiling user code */\n\n"
			"#ifndef __USER_CONSTS_H__\n"
			"#define __USER_CONSTS_H__\n"
			"\n");
	const_small(fh);
	fprintf(fu,	"\n#endif\n");
	fclose(fu);
	fprintf(fh, "\n\n");
	const_big();
	fprintf(fh,	"\n#endif\n");
	fclose(fh);
	return 0;
}

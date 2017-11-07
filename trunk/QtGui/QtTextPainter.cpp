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

#include "QtTextPainter.h"
#include "QtNormalTextPainter.h"
#include "QtAccentedTextPainter.h"
#include "QtSubscriptTextPainter.h"
#include "QtSuperscriptTextPainter.h"
#include "QtBoldTextPainter.h"
#include "QtPrinterSymbolTextPainter.h"

#define C(c) new QtNormalTextPainter(c)
#define C2(c1, c2) new QtNormalTextPainter(c1, c2)
#define A(c, a) new QtAccentedTextPainter(c, a)
#define s(c) new QtSubscriptTextPainter(c)
#define S(c) new QtSuperscriptTextPainter(c)
#define B(c) new QtBoldTextPainter(c)

static struct _specialChars {
	unsigned char code;
	QtTextPainter *painter;
} specialChars[] = {
		// 0033, sub-m does not exists
		{ 0001, A('x', 0X02C9) }, // x-bar
		{ 0002, A('y', 0X02C9) }, // y-bar
		{ 0003, C(0x221A) }, // Square Root
		{ 0004, C(0x222B) }, // Integral
		{ 0005, C(0x00B0) }, // Degree
		{ 0006, C(0x2009) }, // Thin Space
		{ 0007, B(0x24BC) }, // grad
		{ 0010, C(0x00B1) }, // Plus/Minus
		{ 0011, C(0x2264) }, // Less or equal
		{ 0012, C(0x2265) }, // Greater or equal
		{ 0013, C(0x2260) }, // Not equal
		{ 0014, C(0x20AC) }, // Euro
		{ 0015, C(0x2192) }, // Right arrow
		{ 0016, C(0x2190) }, // Left arrow
		{ 0017, C(0x2193) }, // Down arrow
		{ 0020, C(0x2191) }, // Up arrow
		{ 0021, B(0x24D5) }, // f-shift
		{ 0022, B(0x24D6) }, // g-shift
		{ 0023, B(0x24D7) }, // h-shift
		{ 0024, C(0x00A9) }, // complex
		{ 0025, C(0x00D8) }, // O-slash
		{ 0026, C(0x00F8) }, // o-slash
		{ 0027, C(0x21C6) }, // exchange
		{ 0030, C(0x00DF) }, // sz
		{ 0031, A('x', 0X02C6) }, // x-hat
		{ 0032, A('y', 0X02C6) }, // y-hat
		{ 0033, s('m') }, // sub-m
		{ 0034, C(0x00D7) }, // times
		{ 0035, C(0x2248) }, // approx
		{ 0036, C(0x00A3) }, // Pound
		{ 0037, C(0x00A5) }, // Yen
		{ 0177, C(0x2B0D) }, // ^3
		{ 0200, C(0x00b3) }, // up-down
		{ 0201, s('w') }, // sub-w
		{ 0202, C(0x0393) }, // GAMMA
		{ 0203, C(0x0394) }, // DELTA
		{ 0204, C(0x00d0) }, // D-bar
		{ 0205, C(0x00f0) }, // d-bar
		{ 0206, s('d') }, // sub-d
		{ 0207, C(0x0398) }, // THETA
		{ 0210, C(0x00c6) }, // AE
		{ 0211, C(0x00e6) }, // ae
		{ 0212, C(0x039b) }, // LAMBDA
		{ 0213, s('x') }, // sub-x
		{ 0214, s('y') }, // sub-y
		{ 0215, C(0x039e) }, // XI
		{ 0216, C(0x2609) }, // sol
		{ 0217, C(0x03a0) }, // PI
		{ 0220, S('*') }, // super-star
		{ 0221, C(0x03a3) }, // SIGMA
//		{ 0222, B(0x2399) }, // print
		{ 0222, new QtPrinterSymbolTextPainter }, // print
		{ 0223, s('q') }, // sub-q
		{ 0224, C(0x03a6) }, // PHI
		{ 0225, C(0x00ac) }, // not
		{ 0226, C(0x03a8) }, // PSI
		{ 0227, C(0x03a9) }, // OMEGA
		{ 0230, s('B') }, // sub-B
		{ 0231, s(0x03BC) }, // sub-mu
		{ 0232, C(0x00b2) }, // ^2
		{ 0233, s(0x221e) }, // sub-infinity
		{ 0234, S('x') }, // ^x
		{ 0235, C2(0x207B, 0x00B9) }, // ^-1
		{ 0236, C(0x0127) }, // h-bar
		{ 0237, C(0x221e) }, // infinity
		{ 0240, C(0x03b1) }, // alpha
		{ 0241, C(0x03b2) }, // beta
		{ 0242, C(0x03b3) }, // gamma
		{ 0243, C(0x03b4) }, // delta
		{ 0244, C(0x03b5) }, // epsilon
		{ 0245, C(0x03b6) }, // zeta
		{ 0246, C(0x03b7) }, // eta
		{ 0247, C(0x03b8) }, // theta
		{ 0250, C(0x03b9) }, // iota
		{ 0251, C(0x03ba) }, // kappa
		{ 0252, C(0x03bb) }, // lambda
		{ 0253, C(0x03bc) }, // mu
		{ 0254, C(0x03bd) }, // nu
		{ 0255, C(0x03be) }, // xi
		{ 0256, C(0x2A01) }, // terra, 0x2641 but to be close, selected + in circle
		{ 0257, C(0x03c0) }, // pi
		{ 0260, C(0x03c1) }, // rho
		{ 0261, C(0x03c3) }, // sigma
		{ 0262, C(0x03c4) }, // tau
		{ 0263, C(0x03c5) }, // upsilon
		{ 0264, C(0x03c6) }, // phi
		{ 0265, C(0x03c7) }, // chi
		{ 0266, C(0x03c8) }, // psi
		{ 0267, C(0x03c9) }, // omega
		{ 0270, s('0') }, // sub-0
		{ 0271, s('1') }, // sub-1
		{ 0272, s('2') }, // sub-2
		{ 0273, s('c') }, // sub-c
		{ 0274, s('e') }, // sub-e
		{ 0275, s('n') }, // sub-n
		{ 0276, s('p') }, // sub-p
		{ 0277, s('u') }, // sub-u
		{ 0300, C(0x00c0) }, // A-grave
		{ 0301, C(0x00c1) }, // A-acute
		{ 0302, C(0x00c2) }, // A-circumflex
		{ 0303, C(0x00c4) }, // A-umlaut
		{ 0304, C(0x00c5) }, // A-dot
		{ 0305, C(0x0106) }, // C-acute
		{ 0306, C(0x010c) }, // C-hook
		{ 0307, C(0x00c7) }, // C-cedilla
		{ 0310, C(0x00c8) }, // E-grave
		{ 0311, C(0x00c9) }, // E-acute
		{ 0312, C(0x00ca) }, // E-circumflex
		{ 0313, C(0x00cb) }, // E-trema
		{ 0314, C(0x00cc) }, // I-grave
		{ 0315, C(0x00cd) }, // I-acute
		{ 0316, C(0x00ce) }, // I-circumflex
		{ 0317, C(0x00cf) }, // I-trema
		{ 0320, C(0x00d1) }, // N-tilde
		{ 0321, C(0x00d2) }, // O-grave
		{ 0322, C(0x00d3) }, // O-acute
		{ 0323, C(0x00d4) }, // O-circumflex
		{ 0324, C(0x00d6) }, // O-umlaut
		{ 0325, C(0x0158) }, // R-hook
		{ 0326, C(0x0160) }, // S-hook
		{ 0327, s('A') }, // sub-A
		{ 0330, C(0x00d9) }, // U-grave
		{ 0331, C(0x00da) }, // U-acute
		{ 0332, C(0x00db) }, // U-circumflex
		{ 0333, C(0x00dc) }, // U-umlaut
		{ 0334, C(0x016e) }, // U-dot
		{ 0335, C(0x00dd) }, // Y-acute
		{ 0336, C(0x0178) }, // Y-trema
		{ 0337, C(0x017d) }, // Z-hook
		{ 0340, C(0x00e0) }, // a-grave
		{ 0341, C(0x00e1) }, // a-acute
		{ 0342, C(0x00e2) }, // a-circumflex
		{ 0343, C(0x00e4) }, // a-umlaut
		{ 0344, C(0x00e5) }, // a-dot
		{ 0345, C(0x0107) }, // c-acute
		{ 0346, C(0x010d) }, // c-hook
		{ 0347, C(0x00e7) }, // c-cedilla
		{ 0350, C(0x00e8) }, // e-grave
		{ 0351, C(0x00e9) }, // e-acute
		{ 0352, C(0x00ea) }, // e-circumflex
		{ 0353, C(0x00eb) }, // e-trema
		{ 0354, C(0x00ec) }, // i-grave
		{ 0355, C(0x00ed) }, // i-acute
		{ 0356, C(0x00ee) }, // i-circumflex
		{ 0357, C(0x00ef) }, // i-trema
		{ 0360, C(0x00f1) }, // n-tilde
		{ 0361, C(0x00f2) }, // o-grave
		{ 0362, C(0x00f3) }, // o-acute
		{ 0363, C(0x00f4) }, // o-circumflex
		{ 0364, C(0x00f6) }, // o-umlaut
		{ 0365, C(0x0159) }, // r-hook
		{ 0366, C(0x0161) }, // s-hook
		{ 0367, s('k') }, // sub-k
		{ 0370, C(0x00f9) }, // u-grave
		{ 0371, C(0x00fa) }, // u-acute
		{ 0372, C(0x00fb) }, // u-circumflex
		{ 0373, C(0x00fc) }, // u-umlaut
		{ 0374, C(0x016f) }, // u-dot
		{ 0375, C(0x00fd) }, // y-acute
		{ 0376, C(0x00ff) }, // y-trema
		{ 0377, C(0x017e) }, // z-hook
};

#undef C
#undef C2
#undef A
#undef s
#undef S

QHash<char, QtTextPainter*>* QtTextPainter::textPainters=NULL;

QtTextPainter::~QtTextPainter()
{
}

QtTextPainter* QtTextPainter::getTextPainter(char c)
{
	if(textPainters==NULL)
	{
		buildPainters();
	}
	QtTextPainter* textPainter=textPainters->value(c);
	if(textPainter==NULL)
	{
		textPainter=new QtNormalTextPainter(c);
		textPainters->insert(c, textPainter);

	}
	return textPainter;
}

void QtTextPainter::buildPainters()
{
	textPainters=new QHash<char, QtTextPainter*>;
	for(int i=0; i<(int) (sizeof(specialChars)/sizeof(_specialChars)); i++)
	{
		textPainters->insert(specialChars[i].code, specialChars[i].painter);
	}
}



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

	C( 1, 0, 0, 0, 0, 0, 0 ),		/* non-character */
	C( 4, 7, 0, 5, 2, 2, 5 ),		/* xbar */
	C( 5, 15, 0, 9, 10, 4, 3 ),		/* ybar */
	C( 6, 28, 4, 4, 5, 6, 4 ),		/* sqrt */
	C( 6, 8, 20, 4, 4, 5, 2 ),		/* integral */
	C( 4, 7, 5, 7, 0, 0, 0 ),		/* degree */
	C( 1, 0, 0, 0, 0, 0, 0 ),		/* single pixel space */
	C( 6, 0, 14, 1, 13, 9, 14 ),		/* Gradians */
	C( 6, 0, 4, 31, 4, 0, 31 ),		/* +/- = 0010 */
	C( 5, 8, 4, 2, 15, 0, 15 ),		/* <= */
	C( 5, 1, 2, 4, 15, 0, 15 ),		/* >= */
	C( 6, 0, 8, 31, 4, 31, 2 ),		/* not equal */
	C( 6, 28, 2, 14, 7, 2, 28 ),		/* Euro */
	C( 6, 0, 4, 8, 31, 8, 4 ),		/* right arrow */
	C( 6, 0, 4, 2, 31, 2, 4 ),		/* left arrow */
	C( 6, 0, 4, 4, 21, 14, 4 ),		/* down arrow */
	C( 6, 0, 4, 14, 21, 4, 4 ),		/* up arrow = 0020 */
	C( 5, 12, 2, 7, 2, 2, 0 ),		/* shift f */
	C( 5, 14, 9, 14, 8, 7, 0 ),		/* shift g */
	C( 5, 1, 1, 7, 9, 9, 0 ),		/* shift h */
	C( 3, 3, 1, 3, 0, 0, 0 ), 		/* complex prefix */
	C( 6, 0, 0, 0, 8, 31, 8 ),		/* -> */
	C( 6, 2, 31, 2, 0, 0, 0 ),		/* <- */
	C( 6, 2, 31, 2, 8, 31, 8 ),		/* <-> */
	C( 5, 6, 9, 7, 9, 15, 1 ),		/* sz = 0030 */
	C( 4, 2, 5, 0, 5, 2, 5 ),		/* xhat */
	C( 4, 2, 5, 0, 5, 2, 2 ),		/* yhat */
	C( 6, 0, 0, 0, 15, 21, 21 ),		/* sub-m */
	C( 4, 0, 0, 5, 2, 5, 0 ),		/* times */
	C( 5, 0, 10, 5, 0, 10, 5 ),		/* approximately equal */
	C( 5, 4, 10, 2, 7, 2, 15 ),		/* Pound */
	C( 6, 17, 10, 14, 4, 14, 4 ),		/* Yen, Yuan */
	C( 5, 0, 0, 0, 0, 0, 0 ),		/* space = 040 */
	C( 2, 1, 1, 1, 1, 0, 1 ),		/* ! */
	C( 4, 5, 5, 5, 0, 0, 0 ),		/* " */
	C( 6, 0, 10, 31, 10, 31, 10 ),		/* # */
	C( 6, 4, 30, 5, 14, 20, 15 ),		/* $, Peso */
	C( 6, 0, 19, 11, 4, 26, 25 ),		/* % */
	C( 6, 6, 9, 6, 25, 9, 22 ),		/* & */
	C( 2, 1, 1, 1, 0, 0, 0 ),		/* ' */
	C( 3, 2, 1, 1, 1, 1, 2 ),		/* ( = 050*/
	C( 3, 1, 2, 2, 2, 2, 1 ),		/* ) */
	C( 6, 0, 10, 4, 31, 4, 10 ),		/* * */
	C( 6, 0, 4, 4, 31, 4, 4 ),		/* + */
	C( 3, 0, 0, 0, 3, 2, 1 ),		/* , */
	C( 6, 0, 0, 0, 31, 0, 0 ),		/* - */
	C( 3, 0, 0, 0, 0, 3, 3 ),		/* . */
	C( 4, 4, 4, 2, 2, 1, 1 ),		/* / */
	C( 5, 6, 9, 13, 11, 9, 6 ),		/* 0 = 060 */
	C( 3, 2, 3, 2, 2, 2, 2 ),		/* 1 */
	C( 5, 6, 9, 8, 4, 2, 15 ),		/* 2 */
	C( 5, 15, 8, 4, 8, 9, 6 ),		/* 3 */
	C( 5, 5, 5, 5, 15, 4, 4 ), 		/* 4 */
	C( 5, 15, 1, 7, 8, 8, 7 ), 		/* 5 */
	C( 5, 6, 1, 7, 9, 9, 6 ), 		/* 6 */
	C( 5, 15, 8, 4, 4, 2, 2 ), 		/* 7 */
	C( 5, 6, 9, 6, 9, 9, 6 ),		/* 8 = 0070*/
	C( 5, 6, 9, 9, 14, 8, 6 ),		/* 9 */
	C( 3, 0, 3, 3, 0, 3, 3 ),		/* : */
	C( 3, 0, 3, 3, 0, 3, 2 ),		/* ; */
	C( 5, 0, 8, 4, 2, 15, 0 ),		/* < */
	C( 6, 0, 0, 31, 0, 31, 0 ),		/* = */
	C( 5, 0, 1, 2, 4, 15, 0 ),		/* > */
	C( 6, 14, 17, 8, 4, 0, 4 ),		/* ? */
	C( 6, 14, 17, 21, 29, 1, 14 ),		/* @ = 0100 */
	C( 6, 14, 17, 17, 31, 17, 17 ),		/* A = 0101 */
	C( 6, 15, 17, 15, 17, 17, 15 ),		/* B */
	C( 6, 14, 17, 1, 1, 17, 14 ),		/* C */
	C( 6, 15, 18, 18, 18, 18, 15 ),		/* D */
	C( 6, 31, 1, 15, 1, 1, 31 ),		/* E */
	C( 6, 31, 1, 15, 1, 1, 1 ),		/* F */
	C( 6, 14, 17, 1, 25, 17, 14 ),		/* G */
	C( 6, 17, 17, 31, 17, 17, 17 ),		/* H = 0110 */
	C( 4, 7, 2, 2, 2, 2, 7 ),		/* I */
	C( 5, 8, 8, 8, 8, 9, 6 ),		/* J */
	C( 6, 17, 9, 7, 5, 9, 17 ),		/* K */
	C( 5, 1, 1, 1, 1, 1, 15 ),		/* L */
	C( 6, 17, 27, 21, 21, 17, 17 ),		/* M */
	C( 6, 17, 19, 21, 25, 17, 17 ),		/* N */
	C( 6, 14, 17, 17, 17, 17, 14 ),		/* O */
	C( 6, 15, 17, 17, 15, 1, 1 ),		/* P = 0120 */
	C( 6, 14, 17, 17, 21, 9, 22 ),		/* Q */
	C( 6, 15, 17, 17, 15, 9, 17 ),		/* R */
	C( 5, 6, 9, 2, 4, 9, 6 ),		/* S */
	C( 6, 31, 4, 4, 4, 4, 4 ),		/* T */
	C( 6, 17, 17, 17, 17, 17, 14 ),		/* U */
	C( 6, 17, 17, 10, 10, 4, 4 ),		/* V */
	C( 6, 17, 17, 21, 21, 27, 17 ),		/* W */
	C( 6, 17, 10, 4, 4, 10, 17 ),		/* X = 0130 */
	C( 6, 17, 17, 10, 4, 4, 4 ),		/* Y */
	C( 5, 15, 8, 4, 2, 1, 15),		/* Z */
	C( 4, 7, 1, 1, 1, 1, 7 ),		/* [ */
	C( 4, 1, 1, 2, 2, 4, 4 ),		/* \ */
	C( 4, 7, 4, 4, 4, 4, 7 ),		/* ] */
	C( 6, 4, 10, 17, 0, 0, 0 ),		/* ^ */
	C( 6, 0, 0, 0, 0, 0, 31 ),		/* _ */
	C( 3, 1, 1, 2, 0, 0, 0 ),		/* ` = 0140 */
	C( 5, 0, 0, 14, 9, 9, 14 ),		/* a = 0141 */
	C( 5, 1, 1, 7, 9, 9, 7 ),		/* b */
	C( 5, 0, 0, 6, 1, 1, 14 ),		/* c */
	C( 5, 8, 8, 14, 9, 9, 14 ),		/* d */
	C( 5, 0, 6, 9, 15, 1, 6 ),		/* e */
	C( 5, 12, 2, 2, 7, 2, 2 ),		/* f */
	C( 5, 0, 14, 9, 14, 8, 7 ),		/* g */
	C( 5, 1, 1, 7, 9, 9, 9 ),		/* h = 0150 */
	C( 4, 2, 0, 3, 2, 2, 7 ),		/* i */
	C( 4, 4, 0, 6, 4, 4, 3 ),		/* j */
	C( 5, 1, 1, 9, 7, 5, 9),		/* k */
	C( 4, 3, 2, 2, 2, 2, 7 ),		/* l */
	C( 6, 0, 0, 15, 21, 21, 21 ),		/* m */
	C( 5, 0, 0, 7, 9, 9, 9 ),		/* n */
	C( 5, 0, 0, 6, 9, 9, 6 ),		/* o */
	C( 5, 0, 0, 7, 9, 7, 1 ),		/* p = 0160 */
	C( 5, 0, 0, 14, 9, 14, 8 ),		/* q */
	C( 5, 0, 0, 13, 3, 1, 1 ),		/* r */
	C( 5, 0, 0, 14, 2, 4, 7 ),		/* s */
	C( 5, 2, 2, 15, 2, 2, 12 ),		/* t */
	C( 5, 0, 0, 9, 9, 9, 14 ),		/* u */
	C( 6, 0, 0, 17, 17, 10, 4 ),		/* v */
	C( 6, 0, 0, 17, 21, 21, 10 ),		/* w */
	C( 4, 0, 0, 5, 2, 2, 5 ),		/* x = 0170 */
	C( 5, 0, 0, 9, 10, 4, 3),		/* y */
	C( 5, 0, 0, 7, 4, 2, 14),		/* z */
	C( 4, 6, 2, 1, 1, 2, 6 ),		/* { */
	C( 2, 1, 1, 1, 1, 1, 1 ),		/* | */
	C( 4, 3, 2, 4, 4, 2, 3 ),		/* } */
	C( 6, 0, 2, 21, 8, 0, 0 ),		/* ~ */
	C( 6, 21, 10, 21, 10, 21, 10 ),		/* del */
	C( 6, 14, 17, 17, 31, 17, 17 ),		/* ALPHA = 0200 */
	C( 6, 15, 17, 15, 17, 17, 15 ),		/* BETA */
	C( 6, 31, 17, 1, 1, 1, 1 ),		/* GAMMA */
	C( 6, 4, 4, 10, 10, 17, 31 ),		/* DELTA */
	C( 6, 31, 1, 15, 1, 1, 31 ),		/* EPSILON */
	C( 5, 15, 8,   4, 2, 1, 15),		/* ZETA */
	C( 6, 17, 17, 31, 17, 17, 17 ),		/* ETA */
	C( 6, 14, 17, 17, 31, 17, 14 ),		/* THETA */
	C( 4, 7, 2, 2, 2, 2, 7),		/* IOTA = 0210 */
	C( 6, 17, 9, 7, 5, 9, 17 ),		/* KAPPA  */
	C( 6, 4, 4, 10, 10, 17, 17 ),		/* LAMBDA  */
	C( 6, 17, 27, 21, 21, 17, 17 ),		/* MU  */
	C( 6, 17, 19, 21, 25, 17, 17 ),		/* NU  */
	C( 6, 31, 0, 14, 0, 0, 31),		/* XI  */
	C( 6, 14, 17, 17, 17, 17, 14 ),		/* OMICRON  */
	C( 6, 31, 10, 10, 10, 10, 10 ),		/* PI  */
	C( 6, 15, 17, 17, 15, 1, 1 ),		/* RHO = 0220 */
	C( 6, 31, 2, 4, 4, 2, 31 ),		/* SIGMA  */
	C( 6, 31, 4, 4, 4, 4, 4 ),		/* TAU	*/
	C( 6, 17, 17, 10, 4, 4, 4 ),		/* UPSILON */
	C( 6, 4, 14, 21, 21, 14, 4),		/* PHI	*/
	C( 6, 17, 10, 4,  4, 10, 17 ),		/* CHI */
	C( 6, 4, 21, 21, 14, 4, 4),		/* PSI */
	C( 6, 14, 17, 17, 17, 10, 27 ),		/* OMEGA */
	C( 4, 0, 0, 0, 3, 7, 7 ),		/* sub B = 0230 */
	C( 4, 0, 0, 0, 5, 7, 1 ),		/* sub mu */
	C( 4, 7, 4, 2, 7, 0, 0 ),		/* super 2 */
	C( 6, 0, 0, 0, 10, 21, 10 ),		/* sub infinity */
	C( 4, 5, 2, 5, 0, 0, 0 ),		/* super x */
	C( 6, 16, 24, 19, 16, 0, 0 ),		/* super -1 */
	C(6, 2, 15, 2, 14, 18, 18 ),		/* hbar */
	C( 6, 0, 0, 10, 21, 21, 10 ),		/* infinity */
	C( 6, 0, 0, 22, 9, 9, 22 ),		/* alpha = 0240 */
	C( 5, 2, 5, 7, 9, 7, 1 ),		/* beta */
	C( 5, 0, 10, 5, 4, 4, 4),		/* gamma */
	C( 5, 4, 2, 4, 14, 9, 6 ),		/* delta */
	C( 5, 0, 6, 1, 2, 9, 6 ),		/* epsilon */
	C( 5, 15, 8, 4, 2, 14, 8 ),		/* zeta */
	C( 6, 0, 0, 13, 18, 18, 16 ),		/* eta */
	C( 6, 6, 9, 9, 30, 9, 6 ),		/* theta */
	C( 3, 0, 0, 1, 1, 1, 3 ),		/* iota = 0250 */
	C( 5, 0, 0, 13, 3, 5, 9 ),		/* kappa  */
	C( 5, 1, 2, 4, 4, 10, 9 ),		/* lambda  */
	C( 5, 0, 0, 9, 9, 15, 1 ),		/* mu  */
	C( 5, 0, 0, 9, 9, 5, 3 ),		/* nu  */
	C( 5, 15, 4, 14, 4, 14, 8 ),		/* xi  */
	C( 5, 0, 0, 6, 9, 9, 6 ),		/* omicron  */
	C( 6, 0, 0, 31, 10, 10, 26 ),		/* pi  */
	C( 6, 0, 0, 28, 18, 14, 3 ),		/* rho = 0260 */
	C( 6, 0, 0, 30, 9, 9, 6 ),		/* sigma  */
	C( 5, 0, 0, 15, 2, 2, 6 ),		/* tau	*/
	C( 6, 0, 0, 19, 18, 18, 14 ),		/* upsilon */
	C( 6, 0, 14, 21, 21, 14, 4 ),		/* phi	*/
	C( 6, 0, 22, 12, 4, 6, 13 ),		/* chi */
	C( 6, 0, 0, 21, 21, 14, 4 ),		/* psi */
	C( 6, 0, 0, 10, 17, 21, 10 ),		/* omega */
	C( 4, 0, 0, 0, 7, 5, 7 ),		/* sub 0 = 0270 */
	C( 3, 0, 0, 2, 3, 2, 2 ),		/* sub 1 */
	C( 4, 0, 0, 7, 4, 2, 7 ),		/* sub 2 */
	C( 4, 0, 0, 0, 7, 1, 6 ),		/* sub c */
	C( 4, 0, 0, 6, 5, 3, 6 ),		/* sub e  */
	C( 4, 0, 0, 0, 3, 5, 5 ),		/* sub n  */
	C( 4, 0, 0, 0, 7, 7, 1 ),		/* sub p */
	C( 4, 0, 0, 0, 5, 5, 6 ),		/* sub u */
	C( 6,  3, 0, 14, 17, 31, 17 ),		/* A grave = 0300 */
	C( 6, 24, 0, 14, 17, 31, 17 ),		/* A acute  */
	C( 6, 14, 0, 14, 17, 31, 17 ),		/* A tilde */
	C( 6, 10, 0, 14, 17, 31, 17 ),		/* A umlaut  */
	C( 6,  4, 0, 14, 17, 31, 17 ),		/* A dot */
	C( 6, 24, 0, 30, 1, 1, 30 ),		/* C acute */
	C( 6, 14, 0, 30, 1, 1, 30 ),		/* C hook */
	C( 6, 14, 17, 1, 1, 30, 3 ),		/* C cedilla */
	C( 6,  3, 0, 31, 15, 1, 31 ),		/* E grave = 0310 */
	C( 6, 24, 0, 31, 15, 1, 31 ),		/* E acute */
	C( 6, 14, 0, 31, 15, 1, 31 ),		/* E trema */
	C( 6, 10, 0, 31, 15, 1, 31 ),		/* E umlaut */
	C( 4, 3, 0, 7, 2, 2, 7 ),		/* I grave */
	C( 4, 6, 0, 7, 2, 2, 7 ),		/* I acute */
	C( 4, 7, 0, 7, 2, 2, 7 ),		/* I tilde */
	C( 4, 5, 0, 7, 2, 2, 7 ),		/* I trema */
	C( 6, 14, 0, 19, 21, 25, 17 ),		/* N tilde = 0320 */
	C( 6,  3, 0, 14, 17, 17, 14 ),		/* O grave */
	C( 6, 24, 0, 14, 17, 17, 14 ),		/* O acute */
	C( 6, 14, 0, 14, 17, 17, 14 ),		/* O tilde */
	C( 6, 10, 0, 14, 17, 17, 14 ),		/* O umlaut */
	C( 6, 14, 0, 15, 17, 15, 17 ),		/* R hook */
	C( 5, 14, 0, 14, 3, 12, 7 ),		/* S hook */
	C( 4, 0, 0, 0, 7, 7, 5 ),		/* sub A */
	C( 6,  3, 0, 17, 17, 17, 14 ),		/* U grave = 0330 */
	C( 6, 24, 0, 17, 17, 17, 14 ),		/* U acute */
	C( 6, 14, 0, 17, 17, 17, 14 ),		/* U tilde */
	C( 6, 10, 0, 17, 17, 17, 14 ),		/* U umlaut */
	C( 6,  4, 0, 17, 17, 17, 14 ),		/* U dot */
	C( 6, 24, 0, 17, 10, 4, 4 ),		/* Y acute */
	C( 6, 10, 0, 17, 10, 4, 4 ),		/* Y trema */
	C( 5, 15, 0, 15, 4, 2, 15),		/* Z hacek */
	C( 5,  3, 0, 14, 9, 9, 14 ),		/* a grave = 0340  */
	C( 5, 12, 0, 14, 9, 9, 14 ),		/* a acute  */
	C( 5, 14, 0, 14, 9, 9, 14 ),		/* a tilde */
	C( 5, 10, 0, 14, 9, 9, 14 ),		/* a umlaut */
	C( 5,  4, 0, 14, 9, 9, 14 ),		/* a dot */
	C( 5, 12, 0, 6, 1, 1, 14 ),		/* c acute */
	C( 5, 14, 0, 6, 1, 1, 14 ),		/* c hook */
	C( 5, 0, 6, 1, 1, 14, 3 ),		/* c cedilla */
	C( 4, 3, 0, 6, 5, 3, 6 ),		/* e grave = 0350 */
	C( 4, 6, 0, 6, 5, 3, 6 ),		/* e acute */
	C( 4, 7, 0, 6, 5, 3, 6 ),		/* e tilde */
	C( 4, 5, 0, 6, 5, 3, 6 ),		/* e trema */
	C( 4, 3, 0, 3, 2, 2, 7 ),		/* i grave */
	C( 4, 6, 0, 3, 2, 2, 7 ),		/* i acute */
	C( 4, 7, 0, 3, 2, 2, 7 ),		/* i tilde */
	C( 4, 5, 0, 3, 2, 2, 7 ),		/* i trema */
	C( 5, 7, 0, 7, 9, 9, 9 ),		/* n tilde = 0360 */
	C( 5, 3, 0, 6, 9, 9, 6 ),		/* o grave */
	C( 5, 12, 0, 6, 9, 9, 6 ),		/* o acute */
	C( 5, 15, 0, 6, 9, 9, 6 ),		/* o tilde */
	C( 5, 9, 0, 6, 9, 9, 6 ),		/* o umlaut */
	C( 5, 7, 0, 13, 3, 1, 1 ),		/* r hook */
	C( 5, 14, 0, 14, 2, 4, 7 ),		/* s hook */
	C( 4, 0, 0, 0, 5, 3, 5 ),		/* sub k */
	C( 5, 3, 0, 9, 9, 9, 14 ),		/* u grave = 0370*/
	C( 5, 12, 0, 9, 9, 9, 14 ),		/* u acute */
	C( 5, 15, 0, 9, 9, 9, 14 ),		/* u tilde */
	C( 5, 9, 0, 9, 9, 9, 14 ),		/* u umlaut */
	C( 5, 6, 0, 9, 9, 9, 14 ),		/* u dot */
	C( 5, 12, 0, 9, 10, 4, 3 ),		/* y acute */
	C( 5, 9, 0, 9, 10, 4, 3 ),		/* y trema */
	C( 5, 7, 0, 7, 4, 2, 14),		/* z hacek */

	/* That's the first half done, now repeat the characters
	 * in a narrower/smaller format.
	 */
	C( 1, 0, 0, 0, 0, 0, 0 ),		/* small non character = 0400 */
	C( 4, 7, 0, 5, 2, 5, 0 ),		/* small xbar */
	C( 4, 7, 0, 5, 7, 4, 3 ),		/* small ybar */
	C( 5, 14, 2, 2, 3, 2, 0 ),		/* small sqrt */
	C( 4, 6, 2, 2, 2, 2, 3 ),		/* small integral */
	C( 4, 7, 5, 7, 0, 0, 0 ),		/* small degree */
	C( 1, 0, 0, 0, 0, 0, 0 ),		/* small single pixel space */
	C( 5, 14, 1, 13, 9, 14, 0 ),		/* small Gradians */
	C( 4, 2, 7, 2, 0, 7, 0 ),		/* small +/- = 0410 */
	C( 4, 4, 2, 7, 0, 7, 0 ),		/* small <= */
	C( 4, 1, 2, 7, 0, 7, 0 ),		/* small >= */
	C( 4, 2, 7, 2, 7, 2, 0 ),		/* small not equal */
	C( 5, 14, 1, 7, 1, 14, 0 ),		/* small Euro */
	C( 5, 2, 4, 15, 4, 2, 0 ),		/* small right arrow */
	C( 5, 4, 2, 15, 2, 4, 0 ),		/* small left arrow */
	C( 4, 2, 2, 2, 7, 2, 0 ),		/* small down arrow */
	C( 4, 2, 7, 2, 2, 2, 0 ),		/* small up arrow = 0420 */
	C( 4, 6, 2, 7, 2, 2, 0 ),		/* small shift f */
	C( 4, 6, 5, 7, 4, 3, 0 ),		/* small shift g */
	C( 4, 1, 1, 7, 5, 5, 0 ),		/* small shift h */
	C( 3, 3, 1, 3, 0, 0, 0 ),		/* small complex prefix */
	C( 6, 0, 0, 0, 8, 31, 8 ),		/* small -> */
	C( 6, 2, 31, 2, 0, 0, 0 ),		/* small <- */
	C( 6, 2, 31, 2, 8, 31, 8 ),		/* small <->? */
	C( 4, 3, 5, 3, 5, 7, 1 ),		/* small sz = 0430 */
	C( 4, 2, 5, 0, 5, 2, 5 ),		/* small xhat */
	C( 4, 2, 5, 0, 5, 2, 2 ),		/* small yhat */
	C( 6, 0, 0, 0, 15, 21, 21 ),		/* small sub-m */
	C( 4, 0, 5, 2, 5, 0, 0 ),		/* small times */
	C( 5, 10, 5, 0, 10, 5, 0 ),		/* small approximately equal*/
	C( 5, 6, 2, 7, 2, 15, 0 ),		/* small Pound */
	C( 6, 17, 14, 4, 14, 4, 0 ),		/* small Yen, Yuan */
	C( 2, 0, 0, 0, 0, 0, 0 ),		/* small space = 0440 */
	C( 2, 1, 1, 1, 0, 1, 0 ),		/* small ! */
	C( 4, 5, 5, 0, 0, 0, 0 ),		/* small " */
	C( 6, 10, 31, 10, 31, 10, 0 ),		/* small # */
	C( 6, 30, 5, 14, 20, 15, 0 ),		/* small $, Peso */
	C( 4, 1, 4, 2, 1, 4, 0 ),		/* small % */
	C( 5, 7, 5, 2, 13, 7, 8 ),		/* small & */
	C( 2, 1, 1, 0, 0, 0, 0 ),		/* small ' */
	C( 3, 2, 1, 1, 1, 2, 0 ),		/* small ( */
	C( 3, 1, 2, 2, 2, 1, 0 ),		/* small ) */
	C( 6, 10, 4, 31, 4, 10, 0 ),		/* small * */
	C( 4, 0, 2, 7, 2, 0, 0 ),		/* small + */
	C( 3, 0, 0, 0, 0, 2, 1 ),		/* small , */
	C( 4, 0, 0, 7, 0, 0, 0 ),		/* small - */
	C( 2, 0, 0, 0, 0, 1, 0 ),		/* small . */
	C( 4, 4, 4, 2, 2, 1, 1 ),		/* small / */
	C( 4, 6, 5, 5, 5, 3, 0 ),		/* small 0 = 0460 */
	C( 3, 2, 3, 2, 2, 2, 0 ),		/* small 1 */
	C( 4, 7, 4, 6, 1, 7, 0 ),		/* small 2 */
	C( 4, 7, 4, 6, 4, 7, 0 ),		/* small 3 */
	C( 4, 5, 5, 7, 4, 4, 0 ),		/* small 4 */
	C( 4, 7, 1, 7, 4, 7, 0 ),		/* small 5 */
	C( 4, 6, 1, 7, 5, 7, 0 ),		/* small 6 */
	C( 4, 7, 4, 4, 2, 2, 0 ),		/* small 7 */
	C( 4, 7, 5, 7, 5, 7, 0 ),		/* small 8 = 0470*/
	C( 4, 7, 5, 7, 4, 3, 0 ),		/* small 9 */
	C( 2, 0, 1, 0, 1, 0, 0 ),		/* small : */
	C( 3, 0, 0, 2, 0, 2, 1 ),		/* small ; */
	C( 4, 4, 2, 1, 2, 4, 0 ),		/* small < */
	C( 4, 0, 7, 0, 7, 0, 0 ),		/* small = */
	C( 4, 1, 2, 4, 2, 1, 0 ),		/* small > */
	C( 4, 7, 4, 6, 0, 2, 0 ),		/* small ? */
	C( 5, 6, 9, 13, 1, 14, 0 ),		/* small @ = 0500 */
	C( 4, 2, 5, 7, 5, 5, 0 ),		/* small A = 0501 */
	C( 4, 3, 5, 3, 5, 3, 0 ),		/* small B */
	C( 4, 6, 1, 1, 1, 6, 0 ),		/* small C */
	C( 4, 3, 5, 5, 5, 3, 0 ),		/* small D */
	C( 4, 7, 1, 3, 1, 7, 0 ),		/* small E */
	C( 4, 7, 1, 3, 1, 1, 0 ),		/* small F */
	C( 4, 6, 1, 5, 5, 7, 0 ),		/* small G */
	C( 4, 5, 5, 7, 5, 5, 0 ),		/* small H = 0510 */
	C( 2, 1, 1, 1, 1, 1, 0 ),		/* small I */
	C( 4, 4, 4, 4, 5, 7, 0 ),		/* small J */
	C( 5, 9, 5, 3, 5, 9, 0 ),		/* small K */
	C( 4, 1, 1, 1, 1, 7, 0 ),		/* small L */
	C( 6, 17, 27, 21, 17, 17, 0 ),		/* small M */
	C( 5, 9, 11, 13, 9, 9, 0 ),		/* small N */
	C( 4, 7, 5, 5, 5, 7, 0 ),		/* small O */
	C( 4, 7, 5, 7, 1, 1, 0 ),		/* small P = 0520 */
	C( 5, 6, 9, 9, 5, 10, 0 ),		/* small Q */
	C( 4, 3, 5, 3, 5, 5, 0 ),		/* small R */
	C( 4, 6, 1, 2, 4, 3, 0 ),		/* small S */
	C( 4, 7, 2, 2, 2, 2, 0 ),		/* small T */
	C( 4, 5, 5, 5, 5, 7, 0 ),		/* small U */
	C( 4, 5, 5, 5, 2, 2, 0 ),		/* small V */
	C( 6, 17, 17, 21, 27, 17, 0 ),		/* small W */
	C( 4, 5, 5, 2, 5, 5, 0 ),		/* small X = 0530 */
	C( 4, 5, 5, 2, 2, 2, 0 ),		/* small Y */
	C( 4, 7, 4, 2, 1, 7, 0 ),		/* small Z */
	C( 3, 3, 1, 1, 1, 3, 0 ),		/* small [ */
	C( 4, 1, 1, 2, 2, 4, 4 ),		/* small \ */
	C( 3, 3, 2, 2, 2, 3, 0 ),		/* small ] */
	C( 4, 2, 5, 0, 0, 0, 0 ),		/* small ^ */
	C( 4, 0, 0, 0, 0, 0, 7 ),		/* small _ */
	C( 3, 1, 2, 0, 0, 0, 0 ),		/* small ` = 0540 */
	C( 4, 0, 6, 5, 5, 6, 0 ),		/* small a = 0541 */
	C( 4, 1, 3, 5, 5, 3, 0 ),		/* small b */
	C( 4, 0, 6, 1, 1, 6, 0 ),		/* small c */
	C( 4, 4, 6, 5, 5, 6, 0 ),		/* small d */
	C( 4, 0, 6, 5, 3, 6, 0 ),		/* small e */
	C( 4, 6, 2, 7, 2, 2, 0 ),		/* small f */
	C( 4, 0, 6, 5, 7, 4, 3 ),		/* small g */
	C( 4, 1, 3, 5, 5, 5, 0 ),		/* small h = 0550 */
	C( 3, 1, 0, 1, 1, 3, 0 ),		/* small i */
	C( 3, 2, 0, 2, 2, 2, 3 ),		/* small j */
	C( 4, 1, 5, 3, 5, 5, 0 ),		/* small k */
	C( 3, 1, 1, 1, 1, 3, 0 ),		/* small l */
	C( 6, 0, 15, 21, 21, 21, 0 ),		/* small m */
	C( 4, 0, 3, 5, 5, 5, 0 ),		/* small n */
	C( 4, 0, 3, 5, 5, 6, 0 ),		/* small o */
	C( 4, 0, 3, 5, 5, 3, 1 ),		/* small p = 0560 */
	C( 4, 0, 6, 5, 5, 6, 4 ),		/* small q */
	C( 4, 0, 6, 1, 1, 1, 0 ),		/* small r */
	C( 4, 0, 6, 2, 4, 7, 0 ),		/* small s */
	C( 4, 2, 7, 2, 2, 6, 0 ),		/* small t */
	C( 4, 0, 5, 5, 5, 6, 0 ),		/* small u */
	C( 4, 0, 5, 5, 7, 2, 0 ),		/* small v */
	C( 6, 0, 17, 21, 21, 10, 0 ),		/* small w */
	C( 4, 0, 5, 2, 2, 5, 0 ),		/* small x = 0570 */
	C( 4, 0, 5, 5, 6, 4, 3 ),		/* small y */
	C( 4, 0, 7, 4, 1, 7, 0 ),		/* small z */
	C( 4, 6, 2, 3, 2, 6, 0 ),		/* small { */
	C( 2, 1, 1, 1, 1, 1, 1 ),		/* small | */
	C( 4, 3, 2, 6, 2, 3, 0 ),		/* small } */
	C( 5, 0, 10, 5, 0, 0, 0 ),		/* small ~ */
	C( 5, 5, 10, 5, 10, 5, 0 ),		/* small del */
	C( 4, 2, 5, 7, 5, 5, 0 ),		/* small ALPHA = 0600 */
	C( 4, 3, 5, 3, 5, 3, 0 ),		/* small BETA */
	C( 4, 7, 1, 1, 1, 1, 0 ),		/* small GAMMA */
	C( 5, 8, 12, 10, 9, 15, 0 ),		/* small DELTA */
	C( 4, 7, 1, 3, 1, 7, 0 ),		/* small EPSILON */
	C( 4, 7, 4, 2, 1, 7, 0 ),		/* small ZETA */
	C( 4, 5, 5, 7, 5, 5, 0 ),		/* small ETA */
	C( 5, 6, 9, 15, 9, 6, 0 ),		/* small THETA */
	C( 2, 1, 1, 1, 1, 1, 0 ),		/* small IOTA = 0610 */
	C( 5, 9, 5, 3, 5, 9, 0 ),		/* small KAPPA  */
	C( 4, 2, 2, 5, 5, 5, 0),		/* small LAMBDA  */
	C( 6, 17, 27, 21, 17, 17, 0 ),		/* small MU  */
	C( 5, 9, 11, 13, 9, 9, 0 ),		/* small NU  */
	C( 5, 15, 0, 6, 0, 15, 0 ),		/* small XI  */
	C( 4, 7, 5, 5, 5, 7, 0 ),		/* small OMICRON  */
	C( 4, 7, 5, 5, 5, 5, 0 ),		/* small PI  */
	C( 4, 7, 5, 7, 1, 1, 0 ),		/* small RHO = 0620 */
	C( 5, 15, 2, 4, 2, 15, 0 ),		/* small SIGMA  */
	C( 4, 7, 2, 2, 2, 2, 0 ),		/* small TAU	*/
	C( 4, 5, 5, 2, 2, 2, 0 ),		/* small UPSILON */
	C( 6, 4, 31, 21, 31, 4, 0),		/* small PHI	*/
	C( 4, 5, 5, 2, 5, 5, 0 ),		/* small CHI */
	C( 6, 21, 21, 14, 4, 4, 0),		/* small PSI */
	C( 5, 6, 9, 9, 6, 15, 0),		/* small OMEGA */
	C( 4, 0, 0, 0, 3, 7, 7 ),		/* small sub B = 0630 */
	C( 4, 0, 0, 0, 5, 7, 1 ),		/* small sub mu */
	C( 3, 3, 2, 1, 3, 0, 0 ),		/* small super 2 */
	C( 6, 0, 0, 0, 10, 21, 10 ),		/* small sub infinity */
	C( 4, 5, 2, 5, 0, 0, 0 ),		/* small super x */
	C( 5, 8, 11, 8, 0, 0, 0 ),		/* small super -1 */
	C( 5, 2, 7, 2, 14, 10, 0 ),		/* small hbar */
	C( 6, 0, 10, 21, 21, 10, 0 ),		/* small infinity */
	C( 5, 0, 10, 5, 5, 10, 0 ),		/* small alpha = 0640 */
	C( 4, 2, 5, 3, 5, 3, 1 ),		/* small beta */
	C( 4, 0, 5, 2, 2, 2, 2),		/* small gamma */
	C( 4, 2, 1, 2, 5, 7, 0 ),		/* small delta */
	C( 4, 6, 1, 6, 1, 6, 0 ),		/* small epsilon */
	C( 4, 7, 4, 2, 1, 7, 4 ),		/* small zeta */
	C( 5, 0, 7, 10, 10, 10, 8 ),		/* small eta */
	C( 5, 7, 5, 14, 5, 7, 0 ),		/* small theta */
	C( 3, 0, 1, 1, 1, 3, 0 ),		/* small iota = 0650 */
	C( 4, 0, 5, 3, 5, 5, 0 ),		/* small kappa  */
	C( 4, 3, 4, 4, 6, 5, 0 ),		/* small lambda  */
	C( 4, 0, 5, 5, 5, 7, 1 ),		/* small mu  */
	C( 4, 0, 5, 5, 3, 3, 0 ),		/* small nu  */
	C( 4, 7, 2, 7, 2, 7, 4 ),		/* small xi  */
	C( 4, 0, 3, 5, 5, 6, 0 ),		/* small omicron  */
	C( 5, 0, 15, 5, 5, 13, 0 ),		/* small pi  */
	C( 5, 0, 12, 10, 14, 2, 3 ),		/* small rho = 0660 */
	C( 5, 0, 14, 5, 5, 2, 0 ),		/* small sigma  */
	C( 4, 0, 7, 2, 2, 6, 0 ),		/* small tau	*/
	C( 5, 0, 11, 10, 10, 6, 0 ),		/* small upsilon */
	C( 6, 0, 14, 21, 21, 14, 4 ),		/* small phi	*/
	C( 4, 0, 5, 5, 2, 5, 5 ),		/* small chi */
	C( 6, 0, 21, 21, 14, 4, 4 ),		/* small psi */
	C( 6, 0, 10, 17, 21, 10, 0 ),		/* small omega */
	C( 4, 0, 0, 0, 7, 5, 7 ),		/* small sub 0 = 0670 */
	C( 3, 0, 0, 2, 3, 2, 2 ),		/* small sub 1 */
	C( 3, 0, 0, 3, 2, 1, 3 ),		/* small sub 2 */
	C( 4, 0, 0, 0, 7, 1, 6 ),		/* small sub c */
	C( 4, 0, 0, 6, 5, 3, 6 ),		/* small sub e  */
	C( 4, 0, 0, 0, 3, 5, 5 ),		/* small sub n  */
	C( 4, 0, 0, 0, 7, 7, 1 ),		/* small sub p */
	C( 4, 0, 0, 0, 5, 5, 6 ),		/* small sub u */
	C( 4, 3, 0, 7, 7, 5, 0 ),		/* small A grave = 0700 */
	C( 4, 6, 0, 7, 7, 5, 0 ),		/* small A acute  */
	C( 4, 7, 0, 7, 7, 5, 0 ),		/* small A tilde */
	C( 4, 5, 0, 7, 7, 5, 0 ),		/* small A umlaut  */
	C( 4, 2, 0, 7, 7, 5, 0 ),		/* small A dot */
	C( 4, 6, 0, 7, 1, 7, 0 ),		/* small C acute */
	C( 4, 7, 0, 7, 1, 7, 0 ),		/* small C tilde */
	C( 4, 6, 1, 1, 1, 6, 3 ),		/* small C cedilla */
	C( 4, 3, 0, 7, 3, 7, 0 ),		/* small E grave = 0710 */
	C( 4, 6, 0, 7, 3, 7, 0 ),		/* small E acute */
	C( 4, 7, 0, 7, 3, 7, 0 ),		/* small E tilde */
	C( 4, 5, 0, 7, 3, 7, 0 ),		/* small E umlaut */
	C( 3, 3, 0, 2, 2, 2, 0 ),		/* small I grave */
	C( 3, 3, 0, 1, 1, 1, 0 ),		/* small I acute */
	C( 4, 7, 0, 2, 2, 2, 0 ),		/* small I tilde */
	C( 4, 5, 0, 2, 2, 2, 0 ),		/* small I umlaut */
	C( 5, 15, 0, 11, 13, 9, 0 ),		/* small N tilde = 0720 */
	C( 4, 3, 0, 7, 5, 7, 0 ),		/* small O grave */
	C( 4, 6, 0, 7, 5, 7, 0 ),		/* small O acute */
	C( 4, 7, 0, 7, 5, 7, 0 ),		/* small O tilde */
	C( 4, 5, 0, 7, 5, 7, 0 ),		/* small O umlaut */
	C( 4, 7, 0, 3, 3, 5, 0 ),		/* small R tilde */
	C( 4, 7, 0, 6, 2, 3, 0 ),		/* small S tilde */
	C( 4, 0, 0, 0, 7, 7, 5 ),		/* small sub A  */
	C( 4, 3, 0, 5, 5, 7, 0 ),		/* small U grave = 0730 */
	C( 4, 6, 0, 5, 5, 7, 0 ),		/* small U acute */
	C( 4, 7, 0, 5, 5, 7, 0 ),		/* small U tilde */
	C( 4, 5, 0, 5, 5, 7, 0 ),		/* small U umlaut */
	C( 4, 2, 0, 5, 5, 7, 0 ),		/* small U dot */
	C( 4, 6, 0, 5, 2, 2, 0 ),		/* small Y acute */
	C( 4, 5, 0, 5, 2, 2, 0 ),		/* small Y umlaut */
	C( 4, 7, 0, 3, 2, 6, 0 ),		/* small Z tilde */
	C( 4, 3, 0, 6, 5, 6, 0 ),		/* small a grave = 0740 */
	C( 4, 6, 0, 6, 5, 6, 0 ),		/* small a acute  */
	C( 4, 7, 0, 6, 5, 6, 0 ),		/* small a tilde */
	C( 4, 5, 0, 6, 5, 6, 0 ),		/* small a umlaut  */
	C( 4, 2, 0, 6, 5, 6, 0 ),		/* small a dot */
	C( 4, 6, 0, 7, 1, 6, 0 ),		/* small c acute */
	C( 4, 7, 0, 7, 1, 6, 0 ),		/* small c tilde */
	C( 4, 0, 6, 1, 1, 6, 3 ),		/* small c cedilla */
	C( 4, 3, 0, 7, 3, 6, 0 ),		/* small e grave = 0750 */
	C( 4, 6, 0, 7, 3, 6, 0 ),		/* small e acute */
	C( 4, 7, 0, 7, 3, 6, 0 ),		/* small e tilde */
	C( 4, 5, 0, 7, 3, 6, 0 ),		/* small e umlaut */
	C( 4, 3, 0, 2, 2, 6, 0 ),		/* small i grave */
	C( 3, 3, 0, 1, 1, 3, 0 ),		/* small i acute */
	C( 4, 7, 0, 2, 2, 6, 0 ),		/* small i tilde */
	C( 4, 5, 0, 2, 2, 6, 0 ),		/* small i umlaut */
	C( 4, 7, 0, 3, 5, 5, 0 ),		/* n tilde = 0760 */
	C( 4, 3, 0, 3, 5, 6, 0 ),		/* small o grave */
	C( 4, 6, 0, 3, 5, 6, 0 ),		/* small o acute */
	C( 4, 7, 0, 3, 5, 6, 0 ),		/* small o tilde */
	C( 4, 5, 0, 3, 5, 6, 0 ),		/* small o umlaut */
	C( 4, 7, 0, 6, 1, 1, 0 ),		/* small r tilde */
	C( 4, 7, 0, 6, 2, 3, 0 ),		/* small s tilde */
	C( 4, 0, 0, 0, 5, 3, 5 ),		/* small sub k  */
	C( 4, 3, 0, 5, 5, 6, 0 ),		/* small u grave = 0770 */
	C( 4, 6, 0, 5, 5, 6, 0 ),		/* small u acute */
	C( 4, 7, 0, 5, 5, 6, 0 ),		/* small u tilde */
	C( 4, 5, 0, 5, 5, 6, 0 ),		/* small u umlaut */
	C( 4, 2, 0, 5, 5, 6, 0 ),		/* small u dot */
	C( 4, 6, 0, 5, 7, 4, 3 ),		/* small y acute */
	C( 4, 5, 0, 5, 7, 4, 3 ),		/* small y umlaut */
	C( 4, 7, 0, 3, 2, 6, 0 ),		/* small z tilde */

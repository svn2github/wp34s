I (Nigel Dowrick) have created a number of small patches for the WP-34S firmware. These aren't bug fixes in any sense; rather, they are small pieces of code that change the behaviour of the calculator in various ways.

Here is a description of the patches. These can be enabled by uncommenting the relevant #define statement in the trunk\features.h file and rebuilding the firmware. 

1. “Casio style” exponent key behaviour.
========================================
#define INCLUDE_EEX_PI

On older Casio calculators pressing the exponent key when a number is expected - e.g., after an arithmetic operator - enters PI, without the need to use the shift key. Doing this on the WP-34S enters “1 EEX”; this behaviour is standard on HP machines. I prefer the Casio behaviour and so this patch emulates it on the WP-34S.

Note: in program mode, this changed behaviour persists. This means that this patch will break code (written by you or a third party) that depends on the standard HP behaviour.

Note 2: in program mode the key still displays as EEX even though it acts as PI.

2. New “Casio style” fraction separator.
========================================
#define INCLUDE_CASIO_SEPARATOR

This is simple: on old 7-segment Casio calculators fractions were displayed with _| as the separator. I remember this from my childhood, and this patch duplicates this on the WP-34S.

3. Double-dot fraction entry.
=============================
#define INCLUDE_DOUBLEDOT_FRACTIONS

On the HP-32SII, pressing 3..7 enters the fraction 3/7. On the WP-34S, the same key sequence enters 3 0/7. The WP-34S behaviour is logical, but the HP behaviour is (to me) more convenient. This patch implements the double-dot entry on the WP-34S.

Note: when the second dot is pressed, the dot already entered changes in the display to the lower half of a comma to indicate the double press.

Note 2: once again, code that depends on the standard behaviour will be broken by this patch. Such code is unlikely to be common.

4. A new display mode.
======================
#define INCLUDE_SIGFIG_MODE

The new mode (which takes over from the ALL display mode) formats numbers to a number of significant figures chosen by the user. Unlike SCI or ENG mode, exponent notation is not used unless the number is outside the range 10^(-3) to 10^(9), and by default trailing zeros are removed. So entering PI and successively multiplying it by 10 would display (in ALL 5 mode): 3.14159; 31.4159; 314.159; 3,141.59; 31,415.9; 314,159; 3,141,590; 31,415,900; 314,159,000; 3.14159e9; and repeated division by 10 would give: 0.314159; 0.0314159; 0.00314159; 3.14159e-4.

The advantages of this mode are that it doesn't drown you in significant figures, extra zeros, or exponent notation. It might seem strange (on a calculator with a double precision mode!) to deliberately hide significant figures, but I like it. I often use ALL 3 (4 significant figures) when doing physics calculations; I rarely need to see more digits than this.

If you would prefer trailing zeros, setting user flag K will give them. So in mode ALL 2 the number “3” would display either as 3 or as 3.00 depending on the state of flag K.

Selecting ALL 10 or ALL 11 turns off the effect of flag K on the display.

Note: SHOW and RND still work as expected.

5. y-register displayed in dot matrix portion of the display.
=============================================================
#define INCLUDE_YREG_CODE

This patch inspired the inclusion of the new complex display mode on the WP-34S, and the improved code from that has been fed back into this. Briefly, it does what it says: instead of just displaying the contents of the y-register after a complex operation, it displays them at all times.

Note: Alpha-mode display and messages are not affected.

Note 2: In integer mode the y-register is not displayed; mode and bit number (e.g., 2c64) are shown as normal.

Note 3: the y-register contents are displayed as a decimal number even when the main display is in fraction or HMS mode.

Note 4: Double precision mode is handled with four digits of exponent display, if needed, but for four digit exponents the “e” is replaced by a colon for positive exponents and removed completely for negative exponents.

Note 5: the y-register display replaces certain information normally displayed in that area - grad angle mode and date format mode. These annunciators are still visible when a function shift key is pressed.

6. Right-justify the exponent
=============================
#define INCLUDE_RIGHT_EXP

This patch right-justifies the seven-segment exponent display, including leading zeroes. For some reason I prefer this.

7. Smaller hyphens
==================
#define SMALLER_HYPHENS

This patch makes the dot matrix hyphens a little shorter. I think they look better like this; more importantly, making the hyphens smaller allows more digits to be displayed on the dot-matrix display.

This branch contains the code and compiled firmware for the WP-34s with complex lock mode.

The .bin firmware files are in realbuild.
At present the files with added library routines do not all fit into the calculator memory; this may change later.

The makeflash.cmd batch file in branches/complex_mode can be used to build the firmware files.

To save space the files are currently compiled with the following options:

* Pixel plotting commands disabled
* Bit's "Universal Dispatch" code enabled; although still labelled experimental, it seems to work fine
* Gudermannian functions disabled

At present the number of program steps that will fit into free flash memory is:
* calc.bin - 3326 steps
* calc_xtal.bin - 2556 steps
* calc_ir.bin - 892 steps
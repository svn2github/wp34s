March 14, 2012

WP 34S Version 3.0
==================

This is beta firmware for the WP 34S calculator. This is for HP's 20b
and 30b hardware. Being beta software, there will be problems and
rough edges. Please show a bit of tolerance and patience in this
regard. We provide ROM files for use on the real hardware and
emulation software running on the PC (see below).

Programs can be written on the PC and translated to binary form with
the tools contained in the tools directory. A separate manual is
provided. Software and data can be transfered to the device with the
same cable that is used to update the firmware.


Highlights of this release
--------------------------

* Dynamic allocation of memory to registers, program and subroutine 
  levels,

* Local registers and flags,

* Much improved handling of libraries in flash memory,

* Matrices with up to 100 elements,

* Separate statistical accumulators some of which stored in higher
  precision for greater accuracy,

* Increased accuracy for some mathematical functions,

* Double precision mode, allows user programs to implement fully
  accurate functions,

* Even more functions.

You will need a special programming cable and the MySamba software
to reflash an HP 20b or 30b. See the folder FlashTool on our SourceForge 
pages where you found this file. For the time being, access to a PC
running Windows is required for flashing.


Some issues
-----------

The classic emulator needs at least Windows(r) XP. Its PC keyboard to
calculator keyboard mapping isn't entirely obvious.

The newer Qt based emulators work on various platforms, including W2K. They
will eventually replace the one contained in the emulator directory in the
distribution archive. They also have a revised keyboard mapping.

The displayed revision numbers in the VERS screen may differ between the
manual, the flash image and the emulator(s).


Contacting the team members
---------------------------

Before asking questions or posting bug reports, please read the
included manual. Not all of the many features of this powerful
device are entirely obvious but they are documented and they are
intended to be internally consistent.

Bug reports, suggestions and problems should be reported to:
	
    Walter Bonin <walter.bonin@web.de>,
    Paul Dale <ppzgs1@gmail.com>, (hence the name 'WP')
    Marcus von Cube <marcus@mvcsys.de>

The Qt based multi platform emulators are maintained by:

    Pascal Meheut <pascal.meheut@density-tech.com>

The programming tools are provided by:

    Neil Hamilton <nfhwp34s.tbw9z@ncf.ca>

The programming cable is available for the costs of shipping from

    Gene Wright <genewright143@hotmail.com>

Keyboard overlays in professional quality are provided by

    Eric Rechlin <eric@hpcalc.org>

An introductory WiKi can be found on

    http://www.wiki4hp.com/doku.php?id=34s:start

Vivid discussions can be found on the Museum of HP Calculators:

    http://www.hpmuseum.org/cgi-sys/cgiwrap/hpmuseum/forum.cgi


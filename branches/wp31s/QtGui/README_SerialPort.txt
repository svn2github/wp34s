---------------------- USAGE ----------------------
To use the serial port, simply enter the preferences dialog and select one. If it does not appear in the list, you can simply enter its name

---------------------- RECOMPILING THE SERIAL LIBRARIES (Optional) ----------------------

The serial port communication is optional. To compile without it, simply type 'make qt_gui_no_serial' in the wp34s trunk directory

This module uses the open-source project qextserialport to provide serial-port
communication with real WP-34s units. 

I have compiled the librairies and added them to the WP-34s project so you should not have to recompile them but here are the instructions do to so.

See http://code.google.com/p/qextserialport/ for more information about this project.

It comes with precompiled libraries for Windows, Mac OSX & Linux. However, as these libraries where compiled
on specific platforms (Windows 7/MinGW, OSX Lion, Ubuntu 11.10/gcc-4.6), they may not work on your platform and are not suitable
because of a bug.


First, you'll have to make sure that the Mercurial, free source control management tool, is installed on your system.

Go to http://mercurial.selenic.com/ to download a version.


On Mac OSX, Linux & Windows, get the sources:

hg clone https://code.google.com/p/qextserialport

Or if you want to be compile the exact same version I used:

hg clone https://code.google.com/p/qextserialport/ -r 0d369327759f

Then edit the config.pri file and make sure that "QEXTSERIALPORT_LIBRARY = yes" & "QEXTSERIALPORT_STATIC = yes" are uncommented.
Go to buildlib, run qmake then make.

On Mac OSX, copy the lib/libqextserialport-1.2.a into the QtGui/serial/lib/Darwin directory and rename it to libqextserialport.a
On Linux, copy the lib/libqextserialport-1.2.a into the QtGui/serial/lib/Linux directory and rename it to libqextserialport.a
On Windows, copy the lib/libqextserialport-1.2.a into the QtGui/serial/lib/windows32 directory and rename it to libqextserialport.a


WARNING
-------
On Windows, qextserialport may not compile because of an invalid file in the QtSDK distribution. Here are the workaround from qextserialport website:

QtSDK does provide a "private/qwineventnofifier_p.h", but contents of it is broken.

 #include "../../../src/corelib/kernel/qwineventnotifier_p.h"

So this should be bug of QtSDK installer and updater.


IMO, there are several workarounds can be used.

1. If Qt's source code can be found in your hardware, fix the contents of this broken file. For example, change to

  #include "c:\QtSDK\QtSources\4.8.0/src/corelib/kernel/qwineventnotifier_p.h"

or you can simply replace the file "include/QtCore/private/qwineventnotifier_p.h" with "src/corelib/kernel/qwineventnotifier_p.h"

2. If Qt's source code does not exist in your hardware, you can download this file from http://qt.gitorious.org/qt/qt/blobs/4.8/src/corelib/kernel/qwineventnotifier_p.h

then put it to $QTDIR/include/QtCore/private/qwineventnotifier_p.h

3. Remove the broken file

$QTDIR/include/QtCore/private/qwineventnotifier_p.h

then QextSerialPort will auto select qextwineventnotifier_p.h
 
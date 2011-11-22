This project is a work in progress to implement a WP34s multi-platform emulator. It is written in C++ using the Qt Framework.
It compiles and run at least under MacOSX Lion (10.7.2), Windows XP SP2, Windows Seven and Ubuntu 11.10. 
This list is not exclusive and it should work as well under any OS with a recent version of Qt available including other Linuxes, Vista,
MacOSX Leopard and Snow Leopard, maybe Tiger...

To compile it, you will need:

1) the Qt framework. You'll find informations here: http://qt.nokia.com/ 
  and you can download it for your(s) platform(s) here: http://qt.nokia.com/downloads
  Installation is straightforward and you can (and probably should) keep the defaults options including the installation directory
  I'm using Qt version 4.7.4, the last stable one at the time I'm writing this but it should work with 4.8 Beta and maybe 4.7.x.
 
2) a C and a C++ compiler, usually gcc/gcc++. On MacOSX, you'll will probably have to get XCode by registering freely to their
developpers site; on Linux, to use your favorite package manager. On Windows, the MinGW version comes with Qt. 
  
3) Under Windows, at least the MSYS part of MINGW, i.e. the Unix-like commands rm, uname... If you already have a Cygwin installation, 
MSYS may not be needed

4) to define the QMAKESPEC environment variable. This is needed by Qt to know which platform/compiler are used. 
  - On MacOSX I use: QMAKESPEC=/Applications/QtSDK/Desktop/Qt/474/gcc/mkspecs/macx-g++
  - On Linux I use: QMAKESPEC=/opt/QtSDK/Desktop/Qt/474/gcc/mkspecs/linux-g++
  - Ob Windows I use: QMAKESPEC=C:\QtSDK\Desktop\Qt\4.7.4\mingw\mkspecs\win32-g++

This variable must of course be exported (or passed to make as an argument)

5) to add a few entries to the PATH
  - on MacOSX, add /Applications/QtSDK/Desktop/Qt/474/gcc/bin
  - on Linux, add /opt/QtSDK/Desktop/Qt/474/gcc/bin
  - on Windows, you'll have to add 3 entries: the one for qmake, the one for Qt version of MinGW and the one for MSYS. 
    so add C:\QtSDK\mingw\bin;C:\QtSDK\Desktop\Qt\4.7.4\mingw\bin;C:\mingw\msys\1.0\bin to your PATH
    
    
Once everything is installed, you can check that commands like make, qmake and uname work for you shell/CMD window.

To compile:

Either enter the QtGui subdirectory of the wp34s project (where you should have found this file) and simply launch make.
Or, in the wp34s directory, type: make qt_qui

If the compilation ends with no error, you can then launch the emulator but only from the QtGui directory yet.

  - on MacOSX, launch ./Darwin/QtGui.app/Contents/MacOS/QtGui
  - on Linux, launch ./Linux/QtGui
  - on Windows, launch .\windows32\QtGui.exe
  

The project is standard C/C++ so any good IDE can be used to edit and debug it. I'm using Eclipse with CDT as well as Emacs and Gdb.  
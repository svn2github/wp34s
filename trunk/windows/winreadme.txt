* This is where the windows port lives.

The Windows version is build using MSVC++ 2005. load the SLN file and build. i
havent tried it on later versions.

some notes about the build.

* CONSTS

`compile_consts.c' is built and a post-build script runs to generate the
constants. these wind up in the top-level source directory. you get consts.h
and consts.c but also a huge directory of constants called `consts'.

because there are many files in this directory (which might increase), the project file does not include all these individually. instead a post-build script on consts concatendates these all together into allconsts.c

like this,

cd "$(ProjectDir)\..\.."
$(OutDir)\consts.exe
cd $(ProjectDir)\..\..\consts
del allconsts.c
type *.c > allconsts.out
move /Y allconsts.out allconsts.c

* CATALOGUES

in a similar way to consts, `compile_cats.c' generates a source code header for
the catalogues. a post build script is used to generate `catalogues.h' like
this,

del catalogues.h
$(OutDir)\catalogs.exe > catalogues.h

note that here the output `catalogues.h' file does not live in the regular source directory, but as part of the catalogs project. this is ok, the main source builds know to look there.

* CURSES

the console build navigates the screen with curses. this is fine for linux, but
windows doesnt have it. to provide this i have chucked in a version of
`pdcurses'.

for details on pdcurses, see:
http://pdcurses.sourceforge.net/

* GENERAL

some compilation warnings remain



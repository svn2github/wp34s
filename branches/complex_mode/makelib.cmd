@echo off
setlocal
cd library
set DEBUG=0
rem set WP34S_LIB_ASM_OPTIONS=-d %DEBUG%
rem set WP34S_LIB_DISASM_OPTIONS=-d %DEBUG%
set TOOLS=..\tools
set ASM=%TOOLS%\wp34s_asm.pl
set LIB=%TOOLS%\wp34s_lib.pl -d %DEBUG%
set PP=%TOOLS%\wp34s_pp.pl

set DAT=wp34c-lib.dat

echo on
%LIB% -pp matrix.wp34s matrixedit.wp34s vectors.wp34s digamma.wp34s invgamma.wp34s coordinates.wp34s modified-AGM.wp34s elliptic.wp34s -olib %DAT%
@if errorlevel 1 goto exit
:
%LIB% TVM.wp34s -ilib %DAT% -olib %DAT%
@if errorlevel 1 goto exit
:
%LIB% -pp TRIGON.wp34s PF.wp34s -ilib %DAT% -olib %DAT%
@if errorlevel 1 goto exit
:
%LIB% -cat -ilib %DAT% >library.cat
copy %DAT% ..\windows\wp34sgui
copy %DAT% ..\realbuild
@setlocal
cd ..\realbuild
copy/b calc.bin+%DAT% calc_full.bin
copy/b calc_xtal.bin+%DAT% calc_xtal_full.bin
copy/b calc_ir.bin+%DAT% calc_ir_full.bin
if exist calc_noxtal.bin copy/b calc_noxtal.bin+%DAT% calc_noxtal_full.bin
@endlocal
@goto exit

REM unused so far

set SRC=8queens.wp34s code_breaker_pp.wp34s 
set SRC=%SRC% primesieve_pp.wp34s quaternions.wp34s
set SRC=%SRC% HHC2010_Challenge.wp34s savage.wp34s

%ASM% -pp %SRC% -o %DAT%
if errorlevel 1 goto exit
copy %DAT% ..\windows\wp34sgui
copy %DAT% ..\realbuild

%ASM% matrixtestAM01.wp34s matrixtestAM02.wp34s matrixtestAM03.wp34s -o %DAT%
if errorlevel 1 goto exit
copy %DAT% ..\windows\wp34sgui
copy %DAT% ..\realbuild

:exit

@echo off
setlocal
cd library
set TOOLS=..\trunk\tools
set ASM=%TOOLS%\wp34s_asm.pl -op %TOOLS%\wp34s.op -fill 0xffff
set PP=%TOOLS%\wp34s_pp.pl

set DAT=wp34s-1.dat
%ASM% -pp matrixedit.wp34s vectors_pp.wp34s -o %DAT%
if errorlevel 1 goto exit
copy %DAT% ..\trunk\windows\wp34sgui

set DAT=wp34s-2.dat
%ASM% TVM.wp34s -o %DAT%
if errorlevel 1 goto exit
copy %DAT% ..\trunk\windows\wp34sgui

set DAT=wp34s-3.dat
%ASM% -pp TRIGON.wp34s PF.wp34s -o %DAT%
if errorlevel 1 goto exit
copy %DAT% ..\trunk\windows\wp34sgui

set DAT=wp34s-4.dat
set SRC=8queens.wp34s code_breaker_pp.wp34s 
set SRC=%SRC% primesieve_pp.wp34s quaternions.wp34s
set SRC=%SRC% HHC2010_Challenge.wp34s savage.wp34s
%ASM% -pp %SRC% -o %DAT%
if errorlevel 1 goto exit
copy %DAT% ..\trunk\windows\wp34sgui

set DAT=wp34s-5.dat
%ASM% -pp matrixtest.wp34s -o %DAT%
if errorlevel 1 goto exit
copy %DAT% ..\trunk\windows\wp34sgui

:exit

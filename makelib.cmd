@echo off
setlocal
cd library
set TOOLS=..\trunk\tools
set ASM=%TOOLS%\wp34s_asm.pl -op %TOOLS%\wp34s.op -fill 0xffff
%ASM% TVM.wp34s vectors.wp34s -o wp34s-1.dat
if errorlevel 1 goto exit
copy wp34s-1.dat ..\trunk\windows\wp34sgui
set SRC=8queens.wp34s code_breaker_pp.wp34s primesieve_pp.wp34s quaternions.wp34s
set SRC=%SRC% HHC2010_Challenge.wp34s savage.wp34s
set SRC=%SRC% PF.wp34s
rem set SRC=%SRC% TRIGON.wp34s 
%ASM% -pp %SRC% -o wp34s-2.dat
if errorlevel 1 goto exit
copy wp34s-2.dat ..\trunk\windows\wp34sgui
set SRC=TRIGON.wp34s 
%ASM% -pp %SRC% -o wp34s-3.dat
if errorlevel 1 goto exit
copy wp34s-3.dat ..\trunk\windows\wp34sgui
:exit

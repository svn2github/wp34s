@echo off
setlocal
cd library
set TOOLS=..\trunk\tools
rem %TOOLS%\wp34s_asm.exe -op %TOOLS%\wp34s.op TVM.wp34s WHO.wp34s 8queens.wp34s code_breaker.wp34s primesieve.wp34s quaternions.wp34s vectors.wp34s -o wp34s-1.dat
%TOOLS%\wp34s_asm.exe -op %TOOLS%\wp34s.op TVM.wp34s vectors.wp34s WHO.wp34s -o wp34s-1.dat
if errorlevel 1 goto exit
copy wp34s-1.dat ..\trunk\windows\wp34sgui
%TOOLS%\wp34s_asm.exe -op %TOOLS%\wp34s.op 8queens.wp34s code_breaker.wp34s primesieve.wp34s quaternions.wp34s -o wp34s-2.dat
if errorlevel 1 goto exit
copy wp34s-2.dat ..\trunk\windows\wp34sgui
:exit
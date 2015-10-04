rem @echo off
setlocal
rem PATH C:\APPS\netX-ARM-GCC\bin;%PATH%
rem PATH C:\APPS\yagarto-4.5.2\bin;%PATH%
PATH C:\APPS\yagarto-4.6.0\bin;%PATH%
rem PATH C:\APPS\yagarto-4.7.2\bin;%PATH%
rem PATH C:\APPS\CodeSourcery\bin;%PATH%
cd trunk
rem touch features.h
make REALBUILD=1 version all > ..\flash-build.log 2>&1
if errorlevel 1 goto exit
if "%1" == "X" goto exit

rem touch features.h
make REALBUILD=1 XTAL=1 all >> ..\flash-build.log 2>&1
if errorlevel 1 goto exit

rem touch features.h
make REALBUILD=1 XTAL=1 INFRARED=1 all >> ..\flash-build.log 2>&1
if errorlevel 1 goto exit
endlocal

call makelib.cmd
@echo off
echo NORMAL:
cat trunk/realbuild/summary.txt
echo XTAL:
cat trunk/realbuild/summary_xtal.txt
echo IR:
cat trunk/realbuild/summary_ir.txt
:exit
chcp 850 >nul

@echo off
setlocal
rem PATH C:\APPS\netX-ARM-GCC\bin;%PATH%
rem PATH C:\APPS\yagarto-4.5.2\bin;%PATH%
PATH C:\APPS\yagarto-4.6.0\bin;%PATH%
rem PATH C:\APPS\yagarto-4.7.2\bin;%PATH%
rem PATH C:\APPS\CodeSourcery\bin;%PATH%
rem touch features.h
make REALBUILD=1 version all > flash-build.log 2>&1
if errorlevel 1 goto exit
if "%1" == "X" goto exit
@echo off
cat realbuild/summary.txt
:exit
chcp 850 >nul

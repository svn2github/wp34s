@echo off
setlocal
rem PATH C:\APPS\netX-ARM-GCC\bin;%PATH%
rem PATH C:\APPS\yagarto-4.5.2\bin;%PATH%
rem PATH C:\APPS\yagarto-4.6.0\bin;%PATH%
rem PATH C:\APPS\CodeSourcery\bin;%PATH%
cd trunk
rem touch features.h
make REALBUILD=1 version all > ..\flash-build.log 2>&1
rem touch features.h
make REALBUILD=1 XTAL=1 all >> ..\flash-build.log 2>&1
chcp 850 >nul
endlocal
makelib.cmd 


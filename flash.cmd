@echo off
setlocal
rem PATH C:\APPS\netX-ARM-GCC\bin;%PATH%
rem PATH C:\APPS\yagarto-4.5.2\bin;%PATH%
rem PATH C:\APPS\yagarto-4.6.0\bin;%PATH%
rem PATH C:\APPS\CodeSourcery\bin;%PATH%
cd trunk
make REALBUILD=1 version all
make REALBUILD=1 XTAL=1 all
chcp 850 >nul
call makelib.cmd
endlocal
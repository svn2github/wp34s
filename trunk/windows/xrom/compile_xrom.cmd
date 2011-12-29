@setlocal
call "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
@cd ..\..
cl /EP /C /Ixrom xrom.wp34s > xrom_pre.wp34s
if errorlevel 1 goto exit
tools\wp34s_asm.pl -pp -c -op tools\wp34s.op -o xrom.c xrom_pre.wp34s
touch xrom_labels.h
:exit
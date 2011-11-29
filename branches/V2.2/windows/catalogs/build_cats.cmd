@setlocal
@cd ..\..
echo # $Rev$ > tools\wp34s.op
%1\catalogs.exe > catalogues.h 2>> tools\wp34s.op
tools\wp34s_asm.pl -pp -c -op tools\wp34s.op -o xrom.c xrom.wp34s

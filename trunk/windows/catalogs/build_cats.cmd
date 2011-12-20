@setlocal
@cd ..\..
%1\catalogs.exe > catalogues.h 2> tools\wp34s.op
tools\wp34s_asm.pl -pp -c -op tools\wp34s.op -o xrom.c xrom.wp34s

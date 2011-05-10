@setlocal
rem PATH C:\APPS\netX-ARM-GCC\bin;%PATH%
PATH C:\APPS\yagarto\bin;%PATH%
rem PATH C:\APPS\CodeSourcery\bin;%PATH%
@cd trunk
make REALBUILD=1 && type realbuild\summary.txt
@chcp 850
@endlocal
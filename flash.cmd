@setlocal
@cd trunk
make REALBUILD=1 && type realbuild\summary.txt
@endlocal
setlocal
cd ..
start /min "OpenOCD" make server
pause
start telnet localhost 4444
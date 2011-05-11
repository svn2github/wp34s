@setlocal
@cd trunk\windows
call "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
MSBuild wp34s_Express.sln /property:Configuration=Release > build.log
@find "Warning" < build.log
@find "Error" < build.log
@endlocal
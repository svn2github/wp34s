@setlocal
del catalogues.h xrom.c
@cd windows
call "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
@echo on
@set log=..\win-build.log
MSBuild wp31s_Express.sln /property:Configuration=Release > %log%
@find "Warning" < %log%
@find "Warnung" < %log%
@find "Error"   < %log%
@find "Fehler"  < %log%
@endlocal
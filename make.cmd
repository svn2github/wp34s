@setlocal
@cd trunk\windows
MSBuild wp34s_Express.sln /property:Configuration=Release > build.log
@find "Warning" < build.log
@find "Error" < build.log
@endlocal
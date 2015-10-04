@echo off
:
:  Force an update of revision.txt in project root for SVN
:  Create the SDK file builddate.h
:  Create the revision.h file
:
setlocal
set lang=en
set TZ=UTC
set dest=..\..\revision.txt
set config=%1
if "%config%"=="" set config=Release
copy revision.txt.template %dest%
echo Last Windows compile >> %dest%
_date.exe >> %dest%

del builddate.h
CreateDate.exe builddate.h xeq.h
echo #define SVN_REVISION "$Rev::       $">>builddate.h

..\create_revision\%config%\create_revision.exe >..\..\revision.h

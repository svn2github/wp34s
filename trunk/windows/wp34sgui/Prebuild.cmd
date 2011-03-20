@echo off
:
:  Create the SDK file builddate.h
:  Force an update of revision.txt in project root for SVN
:
setlocal
set lang=en
set TZ=UTC
set dest=..\..\..\revision.txt
copy revision.txt.template %dest%
echo Last Windows compile >> %dest%
_date.exe >> %dest%

del builddate.h
CreateDate.exe builddate.h xeq.h
echo #define SVN_REVISION "$Rev::       $">>builddate.h
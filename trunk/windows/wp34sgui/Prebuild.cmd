touch ..\..\..\revision.txt
CreateDate.exe builddate.h xeq.h
echo #define SVN_REVISION "$Rev::     $">>builddate.h
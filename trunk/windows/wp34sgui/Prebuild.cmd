touch ..\..\..\revision.txt
del builddate.h
CreateDate.exe builddate.h xeq.h
echo #define SVN_REVISION "$Rev::       $">>builddate.h
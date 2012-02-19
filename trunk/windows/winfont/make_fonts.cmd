@echo off
setlocal
set bindir=Release
if not "%1"=="" set bindir=%1

goto 48
%bindir%\winfont wp34s_regular_6.fnt 1 0
%bindir%\winfont wp34s_regular_12.fnt 2 0
%bindir%\winfont wp34s_regular_18.fnt 3 0
%bindir%\winfont wp34s_regular_24.fnt 4 1
%bindir%\winfont wp34s_regular_30.fnt 5 1
%bindir%\winfont wp34s_regular_36.fnt 6 1
%bindir%\winfont wp34s_regular_42.fnt 7 2
:48
%bindir%\winfont wp34s_regular_48.fnt 8 2

goto 40
%bindir%\winfont -s wp34s_small_5.fnt 1 0
%bindir%\winfont -s wp34s_small_10.fnt 2 0
%bindir%\winfont -s wp34s_small_15.fnt 3 0
%bindir%\winfont -s wp34s_small_20.fnt 4 1
%bindir%\winfont -s wp34s_small_25.fnt 5 1
%bindir%\winfont -s wp34s_small_30.fnt 6 1
%bindir%\winfont -s wp34s_small_35.fnt 7 2
:40
%bindir%\winfont -s wp34s_small_40.fnt 8 2

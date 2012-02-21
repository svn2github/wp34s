@echo off
setlocal
set bindir=Release
if not "%1"=="" set bindir=%1

%bindir%\winfont WP34SRasterFontRegular.sfd  Template.sfd 20
%bindir%\winfont -s WP34SRAsterFontSmall.sfd Template.sfd 20

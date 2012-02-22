@echo off
setlocal
set bindir=Release
if not "%1"=="" set bindir=%1

%bindir%\winfont -u WP34SRasterFontRegular.sfd  Template.sfd 20
%bindir%\winfont -u -s WP34SRasterFontSmall.sfd Template.sfd 20

%bindir%\winfont -u WP34SSolidFontRegular.sfd  TemplateSolid.sfd 0
%bindir%\winfont -u -s WP34SSolidFontSmall.sfd TemplateSolid.sfd 0

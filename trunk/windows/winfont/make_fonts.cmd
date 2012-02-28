@echo off
setlocal
set bindir=Release
if not "%1"=="" set bindir=%1

%bindir%\winfont -u WP34SRasterFontRegular.sfd  Template.sfd 20
%bindir%\winfont -u -s WP34SRasterFontSmall.sfd Template.sfd 20

%bindir%\winfont -u WP34SSolidFontRegular.sfd  TemplateSolid.sfd 0
%bindir%\winfont -u -s WP34SSolidFontSmall.sfd TemplateSolid.sfd 0

%bindir%\winfont -7 WP34SSegmentFont.sfd TemplateSegment.sfd

set FF=C:\APPS\MinGW\fontforge\fontforge.bat
if not exist %FF% goto exit
set FF=%FF% -c "Open($1);Generate($2);"
call %FF% WP34SRasterFontRegular.sfd WP34SRasterFontRegular.otf
call %FF% WP34SRasterFontSmall.sfd   WP34SRasterFontSmall.otf
call %FF% WP34SSolidFontRegular.sfd  WP34SSolidFontRegular.otf
call %FF% WP34SSolidFontSmall.sfd    WP34SSolidFontSmall.otf
call %FF% WP34SSegmentFont.sfd       WP34SSegmentFont.otf

:exit
endlocal
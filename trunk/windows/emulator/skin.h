//Copyright (c) 2009 Hewlett-PAcked development company L.L.P

/* file created by Cyrille de Brébisson as part of emulator skin management...

A skin is a set of 2 files. one text file which is a kind of script (see format bellow) 
and a bmp file which contains the graphic to use.

the format of the text file is as follow:

#comments start with #
#the filename for the picture In order to allow for long filename, the name starts after the =
#and ends at the end of the line
picture=file
#the size of the visible part of the picture when the window border is visible
size=w*h
#the list of points that bounds the visible part of the picture when the window border is NOT visible
border=x1,y1,x2,y2...xn,yn # polygon for borderless mode
#the position and size of the screen in the picture
screen=x,y,w,h
#screen foreground and background colors (6 digit RGB color)
screenfore=ab12dc
screenback=001241
#for each calcualtor key, have a key entry.
#the first parameter is the code that is used by the calculator firmware
#after that, you have the x/y coordinates of the key on the picture
#followed by a list of PC keyboard keycode that will activate the key
#and optionally a location of a 'pressed' key picture (in the picture file) that is used when the key is pressed
#note that the number of keycodes associated with a key is limited to 10
#keycode is the scan key number. note that if shift key is pressed down, 1000 is added to that number
#so that % (shift 5) is 1053
key=code,x1,y1,x2,y2,{keycode1[,keycode2...,keycoden]}[,sx,sy]
#these last 2 items are used to assign a key to the mouse right and middle buttons.
#they will usually be assigned to the shift keys...
mright=code
mmiddle=code
#the next part are the screen polygon definitions... they should be one for each pixel of the screen
#the uppercase version if for high resolution copy and paste. SCREEN is the size of the screen
#for high resolution
#if the pixel number is NEGATIVE (ie, starts with a -, even if it's -0),  then the syntax is:
#poly=-pixel,x,y,w,h,sx,sy where x/y/w/h is the destination position and size and sx/sy is the source location
#in the backgroud image used to draw...
poly=pixel,{x1, y1, x2, y2....,xn,yn}[{next polygon}]+
POLY=pixel,{x1, y1, x2, y2....,xn,yn}
SCREEN=w*h
*/
#include "stdafx.h"
typedef bool TSkinListCallBack(_TCHAR *fullfilename, _TCHAR *filename, void *p);
typedef struct {
  LONG code;
  RECT r;
  bool hasSource;
  POINT s;
#define NbKeyCodeInKey 10
  LONG keycodes[NbKeyCodeInKey];
} TKey;
typedef struct { LONG NbPoints; POINT *points; } TPoly;
typedef struct { LONG NbPoly; TPoly *poly; } TPolys;
typedef struct { LONG NbPoly; LONG *p; } TSourceGraphic;
class CSkin
{
public:
	
	CSkin();
	 ~CSkin();

	HRGN hpGetKeyRegion(CPoint *pPtMouse, LONG *pKeyPadNum);	
	HRGN hpGetKeyRegion(LONG nKeyPadNum, bool ShiftDown, LONG *code);
  bool SkinLoad(_TCHAR *filename);
  void SkinDelete();
  void SkinList(TSkinListCallBack *SkinListCallBack, void *p);
  bool GetNumber(char **b, DWORD *s, bool hex, LONG *numb, bool *NEG=NULL);
  bool GetPoly(char **f, DWORD *s, int *nbpoly, TPolys **p);

  _TCHAR *filename;
  LONG factor;
  POINT size;
  RECT screen;
  POINT HighResScreen; // size of the high res screen
  int regionsize;
  POINT *region;
  HBITMAP bitmap, b2;
  HDC dc;
  LONG screenfore, screenback;
  LONG mright;
  LONG mmiddle;
  int NbKeys;
  TKey *Keys;
  int error; // error code, 0 if no loading error...
  int NbLowResPoly, NbHighResPoly;
  TPolys *lowres, *highres;
};

extern CSkin Skin;

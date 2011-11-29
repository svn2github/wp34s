//  Copyright (c) 2007 Hewlett-Packard development company llc
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the disclaimer below.
//
//  - Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the disclaimer below in the documentation and/or
//  other materials provided with the distribution. Unless a specific license is granted
//  to te licencee
//
//  HP's name may not be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//  DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY HP "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
//  DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
//  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  ----------------------------------------------------------------------------
/*
* file created by Anuj Garg
*/

/*! \VirtualLCD.cpp HP20b LCD emulator
* 
*
*	Implementation of CVirtualLCD class
*	
*
*/

#include "stdafx.h"
#include "HP20b_c.h"
#include "application.h"
#include "VirtualLCD.h"
#include "Skin.h"
#include "graphics.h"

unsigned int SavedLcdData[20];

CVirtualLCD m_VirtualLCD;

IMPLEMENT_DYNAMIC(CVirtualLCD, CStatic)

CVirtualLCD::CVirtualLCD()
{
  m_hBitmap=NULL;
  m_hMyBitmap=NULL;
  m_hMemDC=NULL;
  m_hMyDC=NULL;
  m_hBrush=NULL;
  m_hBrushRepaint=NULL;
	hpInitBitmap();
	m_nDefTimerScrollLines = m_nDefTimerScrollLines  = TIMER_DISABLED;
}

CVirtualLCD::~CVirtualLCD()
{
  if (m_hBitmap!=0) DeleteObject(m_hBitmap);
  if (m_hMyBitmap!=0) DeleteObject(m_hMyBitmap);
  if (m_hMemDC!=0) DeleteObject(m_hMemDC);
  if (m_hMyDC!=0) DeleteObject(m_hMyDC);
  if (m_hBrush!=0) DeleteObject(m_hBrush);
  if (m_hBrushRepaint!=0) DeleteObject(m_hBrushRepaint);
  m_hBitmap=NULL;
  m_hMyBitmap=NULL;
  m_hMemDC=NULL;
  m_hMyDC=NULL;
  m_hBrush=NULL;
  m_hBrushRepaint=NULL;
}


BEGIN_MESSAGE_MAP(CVirtualLCD, CStatic)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


/***************************************************************
** 
** Responsible to create the compatible bitmap of resource bitmap. 
** It is also responsible to create memory device context and brush 
** to paint he bitmap.  
**
***************************************************************/
//
void CVirtualLCD::hpInitBitmap(void)
{	
  if (m_hBitmap!=0) DeleteObject(m_hBitmap);
  if (m_hMyBitmap!=0) DeleteObject(m_hMyBitmap);
  if (m_hMemDC!=0) DeleteObject(m_hMemDC);
  if (m_hMyDC!=0) DeleteObject(m_hMyDC);
  if (m_hBrush!=0) DeleteObject(m_hBrush);
  if (m_hBrushRepaint!=0) DeleteObject(m_hBrushRepaint);

  HDC hdc= ::GetDC(m_hWnd);
  m_hBitmap= CreateCompatibleBitmap(hdc, Skin.HighResScreen.x, Skin.HighResScreen.y);
  m_hMyBitmap= CreateCompatibleBitmap(hdc, Skin.screen.right,Skin.screen.bottom);
  ::ReleaseDC(m_hWnd, hdc);
	
  if(!m_hBitmap)return ;
  if (!m_hMyBitmap)	return ;

  m_hMemDC	= CreateCompatibleDC (::GetDC(m_hWnd)) ;
  if(!m_hMemDC) return;

  m_hMyDC	= CreateCompatibleDC (::GetDC(m_hWnd)) ;
  if(!m_hMyDC) return;

  SelectObject (m_hMemDC, m_hBitmap);
  SelectObject (m_hMyDC, m_hMyBitmap);
	
  m_hBrush	= CreateSolidBrush(Skin.screenfore);
  m_hBrushRepaint = CreateSolidBrush(Skin.screenback);
  if(!m_hBrush)	return;

  SelectObject (m_hMemDC, m_hBrush);
  SelectObject (m_hMyDC, m_hBrush);

}

/***************************************************************
** 
** Responsible to copy bitmap from memory device context to 
** clipboard in Bitmap format
**
***************************************************************/
//
void CVirtualLCD::hpCopyToClipboard(void)
{
	// opens the clipboard for examination and prevents other applications 
	// from modifying the clipboard content
	if ( !OpenClipboard() )
  {
		AfxMessageBox("Cannot open the Clipboard",MB_OK | MB_ICONINFORMATION );
		return;
	}
	// empties the clipboard and frees handles to data in the clipboard
	if( !EmptyClipboard() )
  {
		AfxMessageBox( "Cannot empty the Clipboard",MB_OK | MB_ICONINFORMATION );
		return;
	}
	// places data on the clipboard in a bitmap clipboard format
	if ( ::SetClipboardData( CF_BITMAP, m_hBitmap ) == NULL )
  {
		AfxMessageBox( "Unable to set Clipboard data" ,MB_OK | MB_ICONINFORMATION );
		CloseClipboard();
		return;
	}
	// closes the clipboard
	CloseClipboard();
}

/***************************************************************
** 
** Responsible to copy bitmap from memory device context to 
** clipboard in Text format
**
***************************************************************/
//
void CVirtualLCD::hpCopyToClipboard(CString text)
{
	// opens the clipboard for examination and prevents other applications 
	// from modifying the clipboard content
	if ( !OpenClipboard() )
  {
		AfxMessageBox("Cannot open the Clipboard", MB_OK | MB_ICONINFORMATION );
		return;
	}
	// empties the clipboard and frees handles to data in the clipboard
	if( !EmptyClipboard() )
  {
		AfxMessageBox( "Cannot empty the Clipboard",MB_OK | MB_ICONINFORMATION );
		return;
	}

	LPTSTR  lptstrCopy	= NULL; 
	HGLOBAL hglbCopy	= NULL; 
	
	// Allocate a global memory object for the text. 
	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, text.GetLength()+1); 
	if (hglbCopy == NULL) 
  { 
		CloseClipboard(); 
		return ; 
	} 

	// Lock the handle and copy the text to the buffer. 
	lptstrCopy = (LPTSTR  )GlobalLock(hglbCopy); 
	memset(lptstrCopy, 0, text.GetLength()); 
	memcpy(lptstrCopy, text.GetBuffer(text.GetLength()), text.GetLength()); 
	GlobalUnlock(hglbCopy); 

	// Place the handle on the clipboard. 
	if ( ::SetClipboardData( CF_TEXT, hglbCopy ) == NULL )
  {
		AfxMessageBox( "Unable to set Clipboard data",MB_OK | MB_ICONINFORMATION );
		GlobalFree(hglbCopy);
		CloseClipboard();
		return;
	}
	GlobalFree(hglbCopy);
		
	// closes the clipboard
	CloseClipboard();
}

/***************************************************************
** 
** Responsible to reterive text from clipboard  
**
***************************************************************/
//
CString CVirtualLCD::hpCopyToHP20b(void)
{
	// opens the clipboard for examination and prevents other applications 
	// from modifying the clipboard content
	if ( !OpenClipboard() )
  {
		AfxMessageBox("Cannot open the Clipboard", MB_OK | MB_ICONINFORMATION );
		return " ";
	}
	
	LPTSTR  lptstrCopy	= NULL; 
	HGLOBAL hglbCopy	= NULL; 

	hglbCopy = ::GetClipboardData(CF_TEXT); 
	if (hglbCopy != NULL) 
	{ 
		// Lock the handle and copy the text to the buffer. 
		lptstrCopy = (LPTSTR  )GlobalLock(hglbCopy); 
		if (lptstrCopy != NULL) 
		{
			GlobalUnlock(hglbCopy); 
	    CloseClipboard(); 
			return lptstrCopy;
		}
	}
	// closes the clipboard
	CloseClipboard();
	return " ";
}

/***************************************************************
** 
** The framework calls this member function after each interval 
** specified in the SetTimer member function used to install a timer
**
***************************************************************/
//
void CVirtualLCD::OnTimer(UINT_PTR nIDEvent)
{
	
	if(m_nDefTimerBlinkCur == TIMER_ID_BLINK_CUR)
		On0p5secondTimer();	

	if(m_nDefTimerScrollLines == TIMER_ID_SCROLL_LINES)
	{
		hpStopTimerScrollLines();
		if (On0p2firstTimer()) hpStartTimerScrollLines(TIME_SCROLL_LINES);
	}
	
	CStatic::OnTimer(nIDEvent);
}

/***************************************************************
** 
** Start timer for Cursor Blinking 
**
***************************************************************/
//
void CVirtualLCD::hpStartTimerBlinkCur()
{
	if(!m_nDefTimerBlinkCur) 	m_nDefTimerBlinkCur		=	SetTimer(TIMER_ID_BLINK_CUR,DEF_TIMER_BLINK_CUR,0);
	
}

/***************************************************************
** 
** Stop timer for Cursor Blinking
**
***************************************************************/
//
void CVirtualLCD::hpStopTimerBlinkCur()
{
	if(m_nDefTimerBlinkCur)		{	KillTimer(m_nDefTimerBlinkCur);		m_nDefTimerBlinkCur    = TIMER_DISABLED; }
	
}

/***************************************************************
** 
** Start timer for Scrolling
**
***************************************************************/
//
void CVirtualLCD::hpStartTimerScrollLines(int ms)
{
	if(!m_nDefTimerScrollLines) m_nDefTimerScrollLines  =	SetTimer(TIMER_ID_SCROLL_LINES,ms,0);
}

/***************************************************************
** 
** Stop timer for Scrolling
**
***************************************************************/
//
void CVirtualLCD::hpStopTimerScrollLines()
{
	if(m_nDefTimerScrollLines)	{	KillTimer(m_nDefTimerScrollLines);	m_nDefTimerScrollLines = TIMER_DISABLED; }
}


/***************************************************************
** 
** The framework calls this function when Windows or an application 
** repaint static box
**
***************************************************************/
//
void CVirtualLCD::OnPaint()
{
  CPaintDC dc(this); // device context for painting
  RECT r={0, 0, Skin.screen.right, Skin.screen.bottom};
  if(m_hBrushRepaint){
    FillRect(dc, &r,m_hBrushRepaint);
    UpdateScreenContent();
  }
}
/***************************************************************
** 
** Responsible to reterive text from clipboard  
**
***************************************************************/
//
void CVirtualLCD::OnLButtonUp(UINT nFlags, CPoint point)
{
	hpCopyToClipboard();
	CStatic::OnLButtonUp(nFlags, point);
}
/***************************************************************
** 
** Responsible to reterive current left (x) position
**
***************************************************************/
//
int CVirtualLCD::hpGetXPos()
{
	RECT rc;
	GetWindowRect(&rc);
	return rc.left;	
}
/***************************************************************
** 
** Responsible to reterive current top (y) position
**
***************************************************************/
//
int CVirtualLCD::hpGetYPos()
{
	RECT rc;
	GetWindowRect(&rc);
	return rc.top;	
}
/***************************************************************
** 
** Responsible to reterive current width
**
***************************************************************/
//
int CVirtualLCD::hpGetWidth()
{
	RECT rc;
	GetWindowRect(&rc);
	return (rc.right- rc.left);	
}
/***************************************************************
** 
** Responsible to reterive current Height 
**
***************************************************************/
//
int CVirtualLCD::hpGetHeight()
{
	RECT rc;
	GetWindowRect(&rc);
	return (rc.bottom-rc.top);	
}


void CVirtualLCD::UpdateScreenContent()
{
/*
#pragma warning(disable : 4996) 
  	  HANDLE h= CreateFile("c:\\polys", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
	    for (int i=0; i<400; i++)
	    {
  	    char s[100000]= "";
	      sprintf(s, "POLY=%d,", i);
        for (int p=0; p<table[i].nbPoly; p++)
        {
          strcat(s, "{");
          for (int pt= 0; pt<table[i].poly[p].nbPoints; pt++)
            sprintf(s, "%s%d,%d,", s, table[i].poly[p].points[pt].x, table[i].poly[p].points[pt].y);
          s[strlen(s)-1]='}';
        }
	      sprintf(s, "%s\npoly=%d,", s, i);
        for (int p=0; p<table_LCD[i].nbPoly; p++)
        {
          strcat(s, "{");
          for (int pt= 0; pt<table_LCD[i].poly[p].nbPoints; pt++)
            sprintf(s, "%s%d,%d,", s, table_LCD[i].poly[p].points[pt].x, table_LCD[i].poly[p].points[pt].y);
          s[strlen(s)-1]='}';
        }
        strcat(s, "\n");
        DWORD wrote;
	      WriteFile(h, s, strlen(s), &wrote, NULL);
	    }
	    CloseHandle(h);
	*/
  u64 *e= (u64 *)AT91C_SLCDC_MEM;
  RECT re={0, 0, Skin.HighResScreen.x, Skin.HighResScreen.y};
  FillRect(m_hMemDC, &re, m_hBrushRepaint);

  re.right=Skin.screen.right; re.bottom=Skin.screen.bottom;
  FillRect(m_hMyDC, &re, m_hBrushRepaint);

  SelectObject(m_hMemDC, GetStockObject(DC_PEN));
  SetDCPenColor(m_hMemDC, Skin.screenfore);

  SelectObject(m_hMyDC, GetStockObject(DC_PEN));
  SetDCPenColor(m_hMyDC, Skin.screenfore);

  if (Skin.NbHighResPoly<400 || Skin.NbLowResPoly<400) return;
  SetPolyFillMode(m_hMyDC, WINDING);
  SetPolyFillMode(m_hMemDC, WINDING);
  for (int r=0; r<10; r++)
    for (int c=0; c<40; c++)
      if ((e[r]&((u64)1<<c))!=0)	
      {
        if (Skin.lowres[r*40+c].NbPoly==-1)
        {
          TSourceGraphic *sg= (TSourceGraphic*)&Skin.lowres[r*40+c];
          BitBlt(m_hMyDC, sg->p[0], sg->p[1], sg->p[2], sg->p[3], Skin.dc, sg->p[4], sg->p[5], SRCCOPY);
        } else
          for (int n=0; n<Skin.lowres[r*40+c].NbPoly; n++)
            Polygon(m_hMyDC, Skin.lowres[r*40+c].poly[n].points, Skin.lowres[r*40+c].poly[n].NbPoints);
        if (Skin.highres[r*40+c].NbPoly==-1)
        {
          TSourceGraphic *sg= (TSourceGraphic*)&Skin.highres[r*40+c];
          BitBlt(m_hMemDC, sg->p[0], sg->p[1], sg->p[2], sg->p[3], Skin.dc, sg->p[4], sg->p[5], SRCCOPY);
        } else
          for (int n=0; n<Skin.highres[r*40+c].NbPoly; n++)
            Polygon(m_hMemDC, Skin.highres[r*40+c].poly[n].points, Skin.highres[r*40+c].poly[n].NbPoints);
      }
  HDC hdc= ::GetDC(m_hWnd);
  BitBlt(hdc, 0,0, Skin.screen.right,Skin.screen.bottom, m_hMyDC, 0,0, SRCCOPY);
  ::ReleaseDC(m_hWnd, hdc);
}

void CVirtualLCD::On0p5secondTimer()
{
	WindowsSwapBuffers();
	UpdateScreenContent();
}

bool CVirtualLCD::On0p2firstTimer()
{
  bool r= ScrollTopLine();
	UpdateScreenContent();
  return r;
}

void CVirtualLCD::WindowsSwapBuffers()
{
  for (int i=0; i<20; i++) { int t= SavedLcdData[i]; SavedLcdData[i]= LcdData[i]; LcdData[i]= t; }
}

void CVirtualLCD::WindowsBlink(bool blink)
{
	if (blink)
		hpStartTimerBlinkCur();
	else 
    hpStopTimerBlinkCur(); 
}

// Added by MvC
// In order to avoid including afxwin.h (MFC) which is not available with
// the free Express Edition of Visual C++, here are a few entry points callable
// without including VirtualLCD.h (which needs afxwin.h)

void WindowsBlink(bool blink)
{
    m_VirtualLCD.WindowsBlink( blink );
}

void WindowsSwapBuffers()
{
    m_VirtualLCD.WindowsSwapBuffers();
}


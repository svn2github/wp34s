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

#ifdef wp31s
#define MANUAL "wp31s_Manual.pdf"
#define WEBSITE "http://wp34s.sourceforge.net/"
#define REGKEY "wp31s"
#else
#define MANUAL "HP_20b_Online_Manual.pdf"
#define WEBSITE "http://www.hp.com/calculators"
#define WEBSITE
#define REGKEY "hp20b"
#endif

# include "stdafx.h"
# include "HP20b_c.h"
# include "HP20b_cDlg.h"
# include "application.h"
# include "system.h"
# include "skin.h"
# include "graphics.h"

//#include "atlimage.h"

// The compiler encountered a function that was marked with deprecated.
// The function may no longer be supported in a future release.
// The warning has been disabled for fopen(...) function
# pragma warning(disable : 4996)
# pragma warning(disable : 4800)

# ifdef _DEBUG
#   define new DEBUG_NEW
#   undef THIS_FILE
static char THIS_FILE[] = __FILE__;
# endif

// Added by MvC
// Name of application as global string
extern "C"char *MyName;
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg:
public CDialog
{
  public:
  CAboutDlg();
// Dialog Data
  //{{AFX_DATA(CAboutDlg)
  enum {
    IDD = IDD_ABOUTBOX};
  //}}AFX_DATA

  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAboutDlg)
  protected:
  virtual void DoDataExchange(CDataExchange *pDX);// DDX/DDV support
  virtual         BOOL OnInitDialog();

  //}}AFX_VIRTUAL

// Implementation
  protected:
  //{{AFX_MSG(CAboutDlg)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
  private:
  CString GetVersionInfo();
};
CAboutDlg::CAboutDlg(): CDialog(CAboutDlg::IDD)
{
  //{{AFX_DATA_INIT(CAboutDlg)
  //}}AFX_DATA_INIT
}
void CAboutDlg::DoDataExchange(CDataExchange *pDX)
{
  CDialog        ::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDlg)
  //}}AFX_DATA_MAP
}
/***************************************************************
**
** This member function is called in response to the WM_INITDIALOG message.
**
***************************************************************/
//
BOOL CAboutDlg::OnInitDialog()
{
  SetDlgItemText(IDC_STATIC_VER_INFO, GetVersionInfo());
  SetDlgItemText(IDC_STATIC_MY_NAME, MyName);
  return TRUE;
}
/***************************************************************
**
**  Function to extract the build date from globally defined
**  variable BuildDate in Build.h
**
***************************************************************/
//
CString CAboutDlg::GetVersionInfo()
{
  CString ret_val;
  u64   b = BuildDate & (u64)0xfffffffffffffff;

  int   m, d, y, r;

  r = (int)((b & (u64)0x000000000ffff000) >> 12);

  if ((b & 0xfff) != 0)
    b <<= 4;

  m = (int)((b & (u64)0xff00000000000000) >> 56);
  d = (int)((b & (u64)0x00ff000000000000) >> 48);
  y = (int)((b & (u64)0x0000ffff00000000) >> 32);
  ret_val.Format("Build date %x-%x-%x (%d)", y, m, d, r);
  return ret_val;
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
// No message handlers
//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CHP20b_cDlg dialog
CHP20b_cDlg::CHP20b_cDlg(CWnd *pParent        /*=NULL*/
)

:
CDialog(CHP20b_cDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CHP20b_cDlg)
  m_nHP20bKeyDown = - 1;
  m_Touch_Base = NONE;
  m_bShiftKeyPressed = false;
  m_bHideTitlebar = false;
  //}}AFX_DATA_INIT
  // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
CHP20b_cDlg::~CHP20b_cDlg()
{
}
void CHP20b_cDlg::DoDataExchange(CDataExchange *pDX)
{
  CDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CHP20b_cDlg)
  DDX_Control(pDX, IDC_VIRTUAL_LCD, m_VirtualLCD);
  DDX_Control(pDX, IDC_STATIC_BG, m_Background);
  //}}AFX_DATA_MAP
}
BEGIN_MESSAGE_MAP(CHP20b_cDlg, CDialog)
//{{AFX_MSG_MAP(CHP20b_cDlg)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_WM_CHAR()
ON_WM_DESTROY()
ON_WM_LBUTTONUP()
ON_WM_LBUTTONDOWN()
ON_WM_RBUTTONUP()
ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
ON_COMMAND(ID_HP20b_RESETSTATE, CHP20b_cDlg::OnHP20bResetState)
//}}AFX_MSG_MAP
ON_COMMAND(ID_HP20b_COPYTOCLIPBOARD, CHP20b_cDlg::
     OnHP20bCopytoclipboard)
ON_WM_ACTIVATE()
ON_WM_LBUTTONDBLCLK()
ON_COMMAND(ID_HP20b_EXIT, CHP20b_cDlg::OnHP20bExit)
ON_COMMAND(ID_HP20b_SHOWCAPTION, CHP20b_cDlg::OnHP20bShowTitlebar)
//ON_WM_NCLBUTTONUP()
ON_COMMAND(ID_HELP_ABOUTBOX, CHP20b_cDlg::OnHelpAboutbox)
ON_COMMAND(ID_HELP_MANUAL, CHP20b_cDlg::
     OnHelpHp20bbusinessconsultant)
ON_COMMAND(ID_HELP_WEBSITE, CHP20b_cDlg::OnBuy)
ON_COMMAND(ID_EDIT_COPY_NUMBER, CHP20b_cDlg::OnEditCopyNumber)
ON_COMMAND(ID_EDIT_COPY_TEXTLINE, CHP20b_cDlg::OnEditCopyTextline)
ON_COMMAND(ID_EDIT_PASTE_NUMBER, CHP20b_cDlg::OnEditPasteNumber)
ON_COMMAND(ID_HP20b_SHOWCAPTION_MENU, CHP20b_cDlg::
     OnHP20bShowcaptionMenu)
ON_WM_MOVE()
ON_WM_RBUTTONDOWN()
ON_WM_RBUTTONDBLCLK()
ON_COMMAND(ID_CALCULATOR_ASSIGNASDEFAULTHPCALCULATOR, CHP20b_cDlg::
     OnCalculatorAssignasdefaulthpcalculator)
ON_COMMAND(ID_CALCULATOR_MANAGEHPCALCULATOREMULATORS, CHP20b_cDlg::
     OnCalculatorManagehpcalculatoremulators)
ON_COMMAND(ID_HELP_HP20BEMULATORHELP, CHP20b_cDlg::
     OnHelpHp20bemulatorhelp)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CHP20b_cDlg message handlers

LARGE_INTEGER PerformanceFrequency;
int TestVoltage(int min, int max)
{
  return 30;
}
HANDLE Pipe = 0;
CHP20b_cDlg *dlg;
HANDLE    KeyEvent;
LARGE_INTEGER LastScreenUpdate;
void UpdateDlgScreen(bool force)
{
  if (!force) {
    LARGE_INTEGER a;

    QueryPerformanceCounter(&a);
    if (a.QuadPart - LastScreenUpdate.QuadPart < PerformanceFrequency.QuadPart / 16)
      return;

    LastScreenUpdate = a;
  }
  LastScreenUpdate.QuadPart = 0;
  SendMessage(dlg->m_hWnd, WM_CHAR, '[', 0);   //used to force a screen refresh...
}
int PeekChar();
volatile  u32 *stacktrash;
static void stackTrash()
{
  volatile  u32 table[2048];

  stacktrash = &table[0];
  for (int i = 0; i < 2048; i++)
    table[i] = 0xcdefcdefUL;

}
int stackSize()
{
  volatile  u32 *a = stacktrash;
  int   s = 2048 *4;

  while (*a == 0xcdefcdefUL)
    a++, s -= 4;

  return s;
}
static void retrashto(int largest)
{
  volatile  u32 *a = stacktrash;

  largest = 2048 - largest / 4;
  while (largest--)
    *a++ = 0xcdefcdefUL;

}
unsigned long __stdcall CalculationThread(void *p)
{
  stackTrash();

  while (1) {
    WaitForSingleObject(KeyEvent, INFINITE);
    while (true) {
      int   k;

      if ((k = KeyBuffGetKey()) != - 1) {
        ClearFlag(VirtualKey);
        KeyPress(k);
        continue;
      }
#ifndef wp31s
      LARGE_INTEGER a, b;
      if (CheckCommunication()) {
        // If we received something, we are WAY likely to receive some more
        QueryPerformanceCounter(&a);
        while (true) {
          // so, we loop, waiting for stuff until we have a clear line for 100ms...
          if ( CheckCommunication()) {
            QueryPerformanceCounter(&a);
            continue;
          }
          QueryPerformanceCounter(&b);
          if (b.QuadPart - a.QuadPart > PerformanceFrequency.QuadPart / 8)
            break;

        }
        continue;
      }
#endif
      break;
    }
    LastScreenUpdate.QuadPart = 0;
    UpdateScreen(false);
  }
}

#ifndef wp31s
bool CommunicationPipeConnected = false;
HANDLE    ComunicationNamedPipe;
unsigned long __stdcall CommunicationThread(void *p)
{
  while (true) {
    if (CommunicationPipeConnected) {
      if (!PeekNamedPipe(ComunicationNamedPipe, NULL, NULL, NULL, NULL, NULL)) {
        DWORD er = GetLastError();

        if (er = 109)
           CommunicationPipeConnected = false;

      }
    }
    if (!CommunicationPipeConnected) {
      CommunicationPipeConnected = ConnectNamedPipe( ComunicationNamedPipe, NULL);
      if (!CommunicationPipeConnected) {
        DWORD   er = GetLastError();

        if (er == ERROR_PIPE_CONNECTED)
          CommunicationPipeConnected = true;

        if (er == ERROR_NO_DATA)
          DisconnectNamedPipe( ComunicationNamedPipe);

      }
    }
    if (!CommunicationPipeConnected) {
      Sleep(100);
      continue;
    }
    DWORD bytes;
    if (!PeekNamedPipe(ComunicationNamedPipe, NULL, 1, &bytes, NULL, NULL))
      continue;

    if (bytes == 0) {
      Sleep(100);
      continue;
    }
    SetEvent(KeyEvent);
  }
}
bool Is40b()
{
# ifndef HP20b
  return true;
# else
  return false;
# endif
}

u32 GetChars(u8 *b, u32 nb, u32 timeout)
{
  static int  p1 = 0, p2 = 0;
  static    u8 B[1024];
  LARGE_INTEGER t1, t2;

  if (timeout != 0) {
    QueryPerformanceCounter(&t1);
    t1.QuadPart = t1.QuadPart + timeout *PerformanceFrequency.QuadPart / 1000;
  }
  while (nb != 0) {
    while (p1 != p2) {
      *b++ = B[p2];
      p2 = (p2 + 1) % 1024;
      if (--nb == 0)
        return nb;
    }
    if (!CommunicationPipeConnected)
      return nb;

    DWORD   bytes, w;

    if (p1 < p2)
      w = p2 - p1;
    else
      w = 1024 - p1;

    if (!PeekNamedPipe(ComunicationNamedPipe, NULL, w, &bytes, NULL, NULL))
      return nb;

    if (bytes == 0) {
      if (timeout == 0)
        return nb;

      Sleep(1);
      QueryPerformanceCounter(&t2);
      if (t2.QuadPart > t1.QuadPart)
        return nb;

      continue;
    }
    if (!ReadFile(ComunicationNamedPipe, &B[p1], bytes, &w,
            NULL))
      return nb;

    p1 = (p1 + w) % 1024;
  }
  return 0;
}
// running the first 20% of test system
// before optim:                       690/s in 4:29 at 15609 io in/s and 15819 io out/s
// after better force sent management: 761/s in 3:37 at 19336 io in/s and 19630 io out/s
void SendChars(u8 const *d, u32 size, bool ForceSend)
{
  static int  p1 = 0, p2 = 0;
  static    u8 b[1024];

  while (size) {
    if (!CommunicationPipeConnected) {
      p1 = 0, p2 = 0;
      return;
    }
    int s = min((p1 >= p2)
        ? 1024 - p1
        : p2 - p1 - 1, (int) size);
    if (s == 0) {
      DWORD   w;

      int s2 = p2 > p1
         ? 1024 - p2
         : p1 - p2;
      WriteFile(ComunicationNamedPipe, &b[p2], s2, &w,
          NULL);
      p2 += w;
      if (p2 - 1024 >= 0)
        p2 -= 1024;

    }
    memcpy(&b[p1], d, s);
    d += s;
    size -= s;
    p1 += s;
    if (p1 - 1024 >= 0)
      p1 -= 1024;

  }
  while (ForceSend) {
    if (!CommunicationPipeConnected) {
      p1 = 0, p2 = 0;
      return;
    }
    int s = (p1 >= p2)
      ? p1 - p2
      : 1024 - p2;
    if (s == 0)
      return;

    DWORD   w;

    WriteFile(ComunicationNamedPipe, &b[p2], s, &w, NULL);
    p2 += w;
    if (p2 - 1024 >= 0)
      p2 -= 1024;

  }
}
void SendCharNoForce(u8 c)
{
  SendChars(&c, 1, false);
}
void SendChar(unsigned char c)
{
  SendChars(&c, 1, true);
//  if (!CommunicationPipeConnected) return;
//  DWORD w;
//  WriteFile(ComunicationNamedPipe, &c, 1, &w, NULL);
}
int GetChar()
{
  u8    b;

  if (0 == GetChars(&b, 1, 0))
    return b;

  return - 1;
}
int PeekChar()
{
  if (!CommunicationPipeConnected)
    return - 1;

  DWORD   bytes;

  if (!PeekNamedPipe(ComunicationNamedPipe, NULL, 1, &bytes, NULL,
         NULL))
    return - 1;

  if (bytes == 0)
    return - 1;

  return max(bytes, 8192);
}
int GetChar2(u32 timeout)
{
  u8    C;
  int   c = GetChars(&C, 1, timeout);

  if (c != 0) {
    return - 1;
  }
  return C;
}
#endif

BOOL CHP20b_cDlg::OnInitDialog()
{
  QueryPerformanceFrequency(&PerformanceFrequency);
  LastScreenUpdate.QuadPart = 0;
  CDialog        ::OnInitDialog();

#ifndef wp31s
  CheckMenuForManager();
#endif
  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);
  CMenu   *pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != NULL) {
    CString         strAboutMenu;

    strAboutMenu.LoadString(IDS_ABOUTBOX);
    if (!strAboutMenu.IsEmpty()) {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX,
               strAboutMenu);
    }
  }
  UpdateSkinMenu();
  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);                       // Set big icon
  SetIcon(m_hIcon, FALSE);          // Set small icon

  SetWindowText(MyName);
  dlg = this;
  ((CHP20b_cApp *) AfxGetApp())->m_hwndDialog = m_hWnd;
  initKeyBuffer();
  Init();
# ifdef HP40b
  initFlashCache();
  initEvaluatorData();
# endif
  if (!ReadRegistry())
    LoadSkin(NULL);
                  // load skins
  SetFlag(TestSystem);
  UpdateScreen(true);
  KeyEvent = CreateEvent(NULL, false, false, "");
  unsigned long id;

  CreateThread(NULL, 1024 *16, CalculationThread, NULL, 0, &id);

#ifndef wp31s
  int   i = 0;
  char    name[50];

  do {
    if (i == 0)
      strcpy(name, "\\\\.\\pipe\\hp20b");
    else
      sprintf(name, "\\\\.\\pipe\\hp20b_%d", i);

    ComunicationNamedPipe = CreateNamedPipe(name,
              PIPE_ACCESS_DUPLEX,
              PIPE_TYPE_BYTE |
              PIPE_READMODE_BYTE
              | PIPE_WAIT, 1,
              8192, 2048, 100,
              NULL);
    i++;
  }
  while (INVALID_HANDLE_VALUE == ComunicationNamedPipe)
    ;

  CreateThread(NULL, 0, CommunicationThread, NULL, 0, &id);
#endif

  return TRUE;              // return TRUE  unless you set the focus to a control
}

#ifndef wp31s
/***************************************************************
**
** Function is responsible to remove Test System related menus
**
***************************************************************/
//
void CHP20b_cDlg::CheckMenuForManager()
{
}
#endif

void CHP20b_cDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
    CAboutDlg dlgAbout;

    dlgAbout.DoModal();
  }
  else {
    // If the title bar is hidden, do not accept ALT key
    if (m_bHideTitlebar)
      if (nID == SC_KEYMENU)
        return;
    CDialog        ::OnSysCommand(nID, lParam);
  }
}
/***************************************************************
**
** Override function to filter VK_F10 keystroke. Pressing F10 doesn't generate WM_KEYDOWN.
** F10 is a special key used to activate menu bar.
**
***************************************************************/
//
BOOL CHP20b_cDlg::PreTranslateMessage(MSG *pMsg)
{
  if ((pMsg->wParam - VK_F1 + 1) == 10        /*VK_F10*/
  ) {
    if (pMsg->message == WM_SYSKEYDOWN)
      HP20bKeyDown(VK_F10);
    else if (pMsg->message == WM_SYSKEYUP)
      HP20bKeyUp(VK_F10);

    return 1;
  }
  if (pMsg->message == WM_KEYDOWN) {
    HP20bKeyDown(pMsg->wParam);
    return 1;
  }
  else if (pMsg->message == WM_KEYUP) {
    HP20bKeyUp(pMsg->wParam);
    return 1;
  }
  else if (pMsg->message == WM_COMMAND)
    if (HIWORD(pMsg->wParam) == 0)
      if (SkinCommand(pMsg))
        return 1;

  return CDialog::PreTranslateMessage(pMsg);
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CHP20b_cDlg::OnPaint()
{
  if (IsIconic()) {
    CPaintDC  dc(this);

    // device context for painting

    SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(),
          0);
    // Center icon in client rectangle
    int   cxIcon = GetSystemMetrics(SM_CXICON);
    int   cyIcon = GetSystemMetrics(SM_CYICON);
    CRect   rect;

    GetClientRect(&rect);
    int   x = (rect.Width() - cxIcon + 1) / 2;
    int   y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else {
    CDialog        ::OnPaint();
  }
}
// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHP20b_cDlg::OnQueryDragIcon()
{
  return (HCURSOR) m_hIcon;
}
void CHP20b_cDlg::keypress(int a)
{
  //  if user has clicked the Off MyApplication key
  if (a == KEYON && GetFlag(shift) && ((System.KeyboardMap && (1 << KEYSHIFT)) == 0)) // On Key
  {
    ClearFlag(shift);
    DestroyWindow();        // Close Application
    return;
  }
  m_VirtualLCD.hpStopTimerScrollLines();
  m_VirtualLCD.hpStopTimerBlinkCur();
  System.KeyboardMap |= (u64)1 << a;
#ifndef wp31s
  SendChar(a);
#endif
  AddKeyInBuffer(a);
  SetEvent(KeyEvent);
}
void CHP20b_cDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (nChar != '[')
    keypress(nChar);
  else {
    m_VirtualLCD.UpdateScreenContent();   // copy the graphics on the PC screen
    if (GetOffset() != OffsetNoScroll)
      m_VirtualLCD.hpStartTimerScrollLines(
                   TIME_SCROLLING);
    else
      m_VirtualLCD.hpStopTimerScrollLines();

  }
}
/***************************************************************
**
** Responsible to add the virtual key code of the current pressed key,
** and fire a WM_KEYUP message for the previous key which the user has
** not released.
**
***************************************************************/
//
void CHP20b_cDlg::ForceHP20bKeyUp(WPARAM wKeyCode)
{
  if (m_listKeyCode.size() > 0) {
    list< WPARAM >::iterator iter;
    bool already_exists = false;

    for (iter = m_listKeyCode.begin(); iter != m_listKeyCode.end(); ++iter) {
      if (*iter == wKeyCode) {
        already_exists = true;
        break;
      }
    }
    if (!already_exists) {
      for (iter = m_listKeyCode.begin(); iter !=
            m_listKeyCode.end(); ) {
          if (*iter != VK_LSHIFT && *iter != VK_RSHIFT && *iter != VK_SHIFT)
	    HP20bKeyUp(*iter);
        iter = m_listKeyCode.erase(iter);
      }
      m_listKeyCode.push_back(wKeyCode);
    }
  }
  else
    m_listKeyCode.push_back(wKeyCode);

}

/***************************************************************
**
** Responsible to trap keystrokes for non-system keys. Once trapped,
** calls the keypress(...) function
**
***************************************************************/
//
void CHP20b_cDlg::HP20bKeyDown(WPARAM wKeyCode)
{
  ForceHP20bKeyUp(wKeyCode);
  if (m_Touch_Base == NONE) {
    {
      if (wKeyCode == VK_LSHIFT || wKeyCode == VK_RSHIFT || wKeyCode == VK_SHIFT)
        m_bShiftKeyPressed = true;
      else {
        m_rgnPressedButton = Skin.hpGetKeyRegion( wKeyCode, m_bShiftKeyPressed, &m_nHP20bKeyDown);
        if (m_rgnPressedButton != 0) {
          keypress(m_nHP20bKeyDown);
          HDC hDC = ::GetDC(m_Background.m_hWnd);

          InvertRgn(hDC, m_rgnPressedButton);
          ::ReleaseDC(m_Background.m_hWnd, hDC);
          m_Touch_Base = KEYBOARD;
        }
      }
    }
  }
}
/***************************************************************
**
** Responsible to trap keystrokes for non-system keys. Once trapped,
** calls the keypress(...) function
**
***************************************************************/
//
void CHP20b_cDlg::HP20bKeyUp(WPARAM wKeyCode)
{
  LONG key;
  if (wKeyCode == VK_LSHIFT || wKeyCode == VK_RSHIFT || wKeyCode == VK_SHIFT) {
    m_bShiftKeyPressed = false;
    key = m_nHP20bKeyDown;
  }
  else {
    m_rgnPressedButton = Skin.hpGetKeyRegion(wKeyCode, m_bShiftKeyPressed, &key);
  }
  if (m_rgnPressedButton != 0 && key >= 0 && System.KeyboardMap & ((u64)1 << key)) {
    HDC hDC = ::GetDC(m_Background.m_hWnd);
    InvertRgn(hDC, m_rgnPressedButton);
    ::ReleaseDC(m_Background.m_hWnd, hDC);
    DeleteObject(m_rgnPressedButton);
    System.KeyboardMap &= ~((u64)1 << key);
    m_nHP20bKeyDown = - 1;
    m_rgnPressedButton = 0;
#ifdef wp31s
    keypress(98);
#endif
  }
  m_Touch_Base = NONE;
}
/***************************************************************
**
**  Called to inform the CWnd object that it is being destroyed.
**
***************************************************************/
//
void CHP20b_cDlg::OnDestroy()
{
  CDialog::OnDestroy();

  WriteToRegistry();
  Shutdown();
  if (Pipe != NULL) {
    CloseHandle(Pipe);
    Pipe = NULL;
  }
}
/***************************************************************
**
** Called when the user releases the left mouse button on the button image
** displayed on the calculator. Function is responsible to invert the color
** of the selected button image.
**
***************************************************************/
//
void CHP20b_cDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
  if (NULL != m_rgnPressedButton && MOUSE == m_Touch_Base) {
    HDC   hDC =::GetDC(m_Background.m_hWnd);

    InvertRgn(hDC, m_rgnPressedButton);
    ::ReleaseDC(m_Background.m_hWnd, hDC);
    DeleteObject(m_rgnPressedButton);
    System.KeyboardMap &= ~((u64)1 << m_nCurKeyPadNum);
    m_rgnPressedButton = NULL;
    m_nCurKeyPadNum = - 1;
#ifdef wp31s
    keypress( 98 );
#endif
  }
  if (MOUSE == m_Touch_Base)
    m_Touch_Base = NONE;

  CDialog::OnLButtonUp(nFlags, point);
}
/***************************************************************
**
**  Called when the user presses  the left mouse button on the button image
**  displayed on the calculator. Function is responsible to invert the color
**  of the selected button image.
**
***************************************************************/
//
void CHP20b_cDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  if (m_Touch_Base == NONE) {
    m_rgnPressedButton = Skin.hpGetKeyRegion(&point, &m_nCurKeyPadNum);
    if (NULL != m_rgnPressedButton) {
      if (m_nCurKeyPadNum >= 0) {
        keypress(m_nCurKeyPadNum);
        if (m_Background.m_hWnd == 0)
          return;

        HDC hDC =::GetDC(m_Background.m_hWnd);

        InvertRgn(hDC, m_rgnPressedButton);
        ::ReleaseDC(m_Background.m_hWnd, hDC);
        //track when the mouse pointer leaves a window
        TRACKMOUSEEVENT tme;

        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE;
        _TrackMouseEvent(&tme);
      }
      else {
        DeleteObject(m_rgnPressedButton);
        m_rgnPressedButton = NULL;
        m_nCurKeyPadNum = - 1;
      }
    }
    m_Touch_Base = MOUSE;
  }
  if (NULL == m_rgnPressedButton) {
    //To move HP20b without title bar
    m_Touch_Base = NONE;
    SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, 0);
  }
  CDialog::OnLButtonDown(nFlags, point);
}
/***************************************************************
**
**  Called when the user releases the right mouse button on HP-Logo.
**  This function is responsible to display the short cut menu.
**
***************************************************************/
//
UINT SkinListCounter;
bool SkinList(_TCHAR *fullfilename, _TCHAR *filename, void *p);

void CHP20b_cDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
  LONG code;
  HRGN r = Skin.hpGetKeyRegion(&point, &code);
  if (NULL != r && code == - 1) {
    HDC   hDC =::GetDC(m_Background.m_hWnd);

    InvertRgn(hDC, r);
    ::ReleaseDC(m_Background.m_hWnd, hDC);
    DeleteObject(r);
    CMenu *pMenu = new CMenu;

    pMenu->LoadMenuA(MAKEINTRESOURCE(IDR_MENU1));
    if (m_bHideTitlebar)
      pMenu->CheckMenuItem(ID_HP20b_SHOWCAPTION, MF_BYCOMMAND | MF_CHECKED);
    else
      pMenu->CheckMenuItem(ID_HP20b_SHOWCAPTION, MF_BYCOMMAND | MF_UNCHECKED);

    SkinListCounter = 65535;
    CMenu   *pMenu2 = pMenu->GetSubMenu(0);

    for (int i = 0; i < pMenu2->GetMenuItemCount(); i++)
    {
      CString         str;

      pMenu2->GetMenuStringA(i, str, MF_BYPOSITION);
      if (str.CompareNoCase("Skins") == 0) {
        CMenu   *pSub = pMenu2->GetSubMenu(i);

        while (pSub->GetMenuItemCount())
          pSub->RemoveMenu(0, MF_BYPOSITION);

        Skin.SkinList(SkinList, pSub);
        break;
      }
    }
    ClientToScreen(&point);
    pMenu->GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
    delete pMenu;
  }
  else {
    if (r != NULL) {
      DeleteObject(r);
#ifdef wp31s
      OnLButtonUp(nFlags, point);
#endif
    }
    // Forcing to send SHIFT key to calculator firmware
    System.KeyboardMap &= ~((u64)1 << Skin.mright);
    UpdateScreen(true);
  }
  CDialog::OnRButtonUp(nFlags, point);
}
/***************************************************************
**
**  Called when the user presses the right mouse button on GUI
**  except LCD Area.
**  This function is responsible to send SHIFT key to firmware.
**
***************************************************************/
//
void CHP20b_cDlg::OnRButtonDown(UINT nFlags, CPoint point)
{ 
  LONG code;
  HRGN r = Skin.hpGetKeyRegion(&point, &code);
  if (NULL != r && code == - 1) {
    HDC hDC = ::GetDC(m_Background.m_hWnd);

    InvertRgn(hDC, r);
    ::ReleaseDC(m_Background.m_hWnd, hDC);
    //track when the mouse pointer leaves a window
    TRACKMOUSEEVENT tme;

    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    _TrackMouseEvent(&tme);
    DeleteObject(r);
  }
  else {
    if (r != NULL) {
      DeleteObject(r);
#ifdef wp31s
      if (code != 30 ) {
		// RMB on any other key: press Shift before
		keypress(Skin.mright);
		OnLButtonDown(nFlags, point);
	    System.KeyboardMap &= ~((u64)1 << Skin.mright);
      }
#endif
    }
#ifndef wp31s
    if (Skin.mright != - 1)
      keypress(Skin.mright);
#endif
  }
  CDialog::OnRButtonDown(nFlags, point);
}

/***************************************************************
**
**  Called when the user presses the right mouse button on GUI
**  except LCD Area.
**  This function is responsible to sned SHIFT key to MYAPPLICATION firmware.
**
***************************************************************/
//
void CHP20b_cDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
//  if (Skin.mright != - 1)
//    keypress(Skin.mright);

  CDialog::OnRButtonDblClk(nFlags, point);
}
/***************************************************************
**
**  Called when the mouse pointer leaves a window or hovers over a window for a specified amount of time
**
***************************************************************/
//

LPARAM CHP20b_cDlg::OnMouseLeave(WPARAM wp, LPARAM lp)
{
  SendMessage(WM_LBUTTONUP, 0, 0);
  SendMessage(WM_RBUTTONUP, 0, 0);
  SendMessage(WM_MBUTTONUP, 0, 0);
  return 0;
}
/***************************************************************
**
**  Called when the user presses 'On OFF' sub-menu
**
***************************************************************/
//
void CHP20b_cDlg::OnHP20bOnOFF()
{
  Reset(false);
  UpdateScreen(true);
}
/***************************************************************
**
**  Called when the user presses 'CopyToClipboard' sub-menu
**
***************************************************************/
//
void CHP20b_cDlg::OnHP20bCopytoclipboard()
{
  m_VirtualLCD.hpCopyToClipboard();
}
/***************************************************************
**
**  Called when the user presses 'Reset State' sub-menu
**
***************************************************************/
void CHP20b_cDlg::OnHP20bResetState()
{
  if (AfxMessageBox(
        "Are you sure that you want to reset the calculator?",
        MB_YESNO | MB_ICONINFORMATION) == IDYES)
  {
    Reset(false);
    UpdateScreen(true);
  }
}
/***************************************************************
**
**  Called when the user double-clicks the left mouse button on the button image
**  displayed on the calculator. Function is responsible to invert the color
**  of the selected button image.
**
***************************************************************/
//
void CHP20b_cDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  m_rgnPressedButton = Skin.hpGetKeyRegion(&point, &m_nCurKeyPadNum);
  if (NULL != m_rgnPressedButton) {
    m_Touch_Base = MOUSE;
    keypress(m_nCurKeyPadNum);
    HDC   hDC =::GetDC(m_Background.m_hWnd);

    InvertRgn(hDC, m_rgnPressedButton);
   ::ReleaseDC(m_Background.m_hWnd, hDC);
    System.KeyboardMap &= ~((u64)1 << m_nCurKeyPadNum);
  }
  CDialog::OnLButtonDblClk(nFlags, point);
}
/***************************************************************
**
**  Called when the user presses 'Exit' sub-menu
**
***************************************************************/
void CHP20b_cDlg::OnHP20bExit()
{
  SendMessage(WM_CLOSE, 0, 0);
}
/***************************************************************
**
**  Called when the user presses 'Hide Titlebar' sub-menu
**
***************************************************************/
void CHP20b_cDlg::OnHP20bShowTitlebar()
{
  ShowHP20bTitlebar();
  // check/uncheck main menu
  CMenu *pMenu = GetMenu();

  if (m_bHideTitlebar)
    pMenu->CheckMenuItem(ID_HP20b_SHOWCAPTION, MF_BYCOMMAND | MF_CHECKED);
  else
    pMenu->CheckMenuItem(ID_HP20b_SHOWCAPTION, MF_BYCOMMAND | MF_UNCHECKED);

}
/***************************************************************
**
**  To show/hide the Title bar of the application window.
**  This is also responsible to set the window region.
**
***************************************************************/
//
void CHP20b_cDlg::ShowHP20bTitlebar()
{
  if (m_bHideTitlebar) {
                  // Display title bar
    SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) | WS_CAPTION);
    SetWindowRgn(NULL, true);
  }
  else {
    // Remove title bar
    HRGN rgn;
    RECT r1 = { 0, 0, 0, 0},
         r2 = { 0, 0, 0, 0},
         r3 = { 0, 0, 0, 0};
    AdjustWindowRectEx(&r3, GetWindowLong(m_hWnd, GWL_STYLE)&~WS_CAPTION, false,
                       GetWindowLong(m_hWnd, GWL_EXSTYLE));
    AdjustWindowRectEx(&r2, GetWindowLong(m_hWnd, GWL_STYLE), true,
                       GetWindowLong(m_hWnd, GWL_EXSTYLE));
    AdjustWindowRectEx(&r1, GetWindowLong(m_hWnd, GWL_STYLE), false,
                       GetWindowLong(m_hWnd, GWL_EXSTYLE));
    int h = r1.top - r2.top + (r3.bottom - r3.top) / 2;
    int w = (r1.right - r1.left) / 2;
    POINT   *p = (POINT *) malloc(Skin.regionsize * sizeof(POINT));

    if (p == NULL)
      return;

    for (int i = 0; i < Skin.regionsize; i++) {
      p[i].x = Skin.region[i].x + w;
      p[i].y = Skin.region[i].y + h;
    }
    rgn = CreatePolygonRgn(p, Skin.regionsize, ALTERNATE);
    SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE)& ~WS_CAPTION);
    SetWindowRgn(rgn, TRUE);
    realloc(p, 0);
  }
  m_bHideTitlebar = !m_bHideTitlebar;
}
/***************************************************************
**
**  Called when the user presses 'About' sub-menu
**
***************************************************************/
//
void CHP20b_cDlg::OnHelpAboutbox()
{
  CAboutDlg dlg;

  dlg.DoModal();
}
/***************************************************************
**
**  Called when the user presses 'HP 20b Business Consultant Help' sub-menu
**
***************************************************************/
//

void CHP20b_cDlg::OnHelpHp20bbusinessconsultant()
{
  _TCHAR path[MAX_PATH], buf[MAX_PATH];
  GetModuleFileName(NULL, path, MAX_PATH);
  _TCHAR *b= NULL; _TCHAR *B= path;
  while (*B) if (*B++=='\\') b= B; *b= 0;
  sprintf(buf, "start /b \"Manual\" \"%s%s\"", path, MANUAL);

  system( buf );
}
/***************************************************************
**
**  Called when the user presses 'Buy' sub-menu
**
***************************************************************/
//
void CHP20b_cDlg::OnBuy()
{
  // Open default web browser
  ShellExecute(NULL, "open", WEBSITE, NULL, NULL, SW_SHOWNORMAL);
}
/***************************************************************
**
**  Called when the user presses 'Copy Number' sub-menu
**
***************************************************************/
//
void CHP20b_cDlg::OnEditCopyNumber()
{
  m_VirtualLCD.hpCopyToClipboard(GetBottomLine());
}
/***************************************************************
**
**  Called when the user presses 'Copy Textline' sub-menu
**
***************************************************************/
//
void CHP20b_cDlg::OnEditCopyTextline()
{
#ifdef wp31s
  m_VirtualLCD.hpCopyToClipboardUnicode(GetTopLineW());
#else
  m_VirtualLCD.hpCopyToClipboard(GetTopLine());
#endif
}
/***************************************************************
**
**  Called when the user presses 'Paste Number' sub-menu
**
***************************************************************/
//
struct {
  char    c;
  unsigned int  keys;
}
const keydefs[] =
{
#ifdef wp31s
  { '0', KEY0},
  { '1', KEY1},
  { '2', KEY2},
  { '3', KEY3},
  { '4', KEY4},
  { '5', KEY5},
  { '6', KEY6},
  { '7', KEY7},
  { '8', KEY8},
  { '9', KEY9},
  { 'e', KEYEEX},
  { 'E', KEYEEX},
  { '-', KEYCHS},
  { '.', KEYDOT},
  { ',', KEYDOT},
  { ' ', - 1},
#else
  { '0', KEY0},
  { '1', KEY1},
  { '2', KEY2},
  { '3', KEY3},
  { '4', KEY4},
  { '5', KEY5},
  { '6', KEY6},
  { '7', KEY7},
  { '8', KEY8},
  { '9', KEY9},
  { 'e', 134},
  { 'E', 134},
  { '-', KEYPLUSMOINS},
  { '.', KEYDOT},
  { '+', KEYPLUS},
  { '*', KEYMUL},
  { '/', KEYDIV},
  { '(', KEYOPENP},
  { ')', KEYCLOSEP},
  { ' ', - 1},
  { '%', KEYPERCENT},
  { 'S', KEYSIN},
  { 'C', KEYCOS},
  { 'T', KEYTAN},
  { 's', KEYSQRT},
  { 'L', KEYLN},
  { 'R', KEYRAND},
  { '!', KEYFACT},
  { '^', KEYPOW},
  { 'P', KEYPERM},
  { 'c', KEYCOMB},
  { '=', KEYEQUAL},
  { 'A', KEYANS},
#endif
  { 0, 0}
};
void CHP20b_cDlg::OnEditPasteNumber()
{
  // retrieve clipboard data
  CString val = m_VirtualLCD.hpCopyToHP20b();
  bool pushsign = false;

  val.Trim();
  if (!val.IsEmpty()) {
    // fire keyboard events
    for (int i = 0; i < val.GetLength(); i++) {
      int oo = val.GetAt(i);
      int j = 0;

      while (keydefs[j].c != 0 && keydefs[j].c != oo)
        j++;

      if (keydefs[j].c == oo) {
        int k = keydefs[j].keys;

        while (k > 0) {
          if ((k & 0xff) > KEYSHIFTPLAN) {
            k = k - KEYSHIFTPLAN;
            AddKeyInBuffer(KEYSHIFT);
          }
	  if ((k & 0xff) == KEYCHS && i == 0) {
            pushsign = true;
	  }
          else {
            AddKeyInBuffer(k & 0xff);
            if (pushsign) {
              pushsign = false;
              AddKeyInBuffer(KEYCHS);
	    }
            SetEvent(KeyEvent);
	  }
          k >>= 8;
        }
      }
    }
    UpdateScreen(true);
  }
}
/***************************************************************
**
**  Resonsible to create the registry key and write the window's current
**  coordinates and title bar visibility status to the registry.
**
***************************************************************/
//
void CHP20b_cDlg::WriteToRegistry()
{
  RECT    rect;
  HKEY    hKey, hkResult;
  char    pos[8];

  GetWindowRect(&rect);
  // Open/Create required registry and write values
  if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE", 0,
       KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
    if (RegCreateKeyEx(hKey, "Hewlett-Packard", 0, NULL,
           REG_OPTION_NON_VOLATILE,
           KEY_ALL_ACCESS, NULL, &hkResult,
           NULL) == ERROR_SUCCESS) {
      HKEY    hkResult1;

      if (RegCreateKeyEx(hkResult, REGKEY, 0, NULL,
             REG_OPTION_NON_VOLATILE,
             KEY_ALL_ACCESS, NULL, &
             hkResult1, NULL) ==
          ERROR_SUCCESS) {
        itoa(rect.left, pos, 10);
        RegSetValueEx(hkResult1, "left", 0,
                REG_SZ, (BYTE *) pos,
                lstrlen(pos) + 1);
        itoa(rect.top, pos, 10);
        RegSetValueEx(hkResult1, "top", 0,
                REG_SZ, (BYTE *) pos,
                lstrlen(pos) + 1);
        sprintf(pos, "%d", m_bHideTitlebar);
        RegSetValueEx(hkResult1, "Titlebar", 0,
                REG_SZ, (BYTE *) pos,
                lstrlen(pos) + 1);
        RegSetValueEx(hkResult1, "Skin", 0,
                REG_SZ, (BYTE *) Skin.
                filename, lstrlen(Skin.
                filename) + 1);
      }
      RegCloseKey(hkResult1);
    }
  }
  RegCloseKey(hkResult);
  RegCloseKey(hKey);
}
/***************************************************************
**
** Responsible to read retrieve left, top coordinates of the application
** window and title bar visibility status from the registry.
**
***************************************************************/
//
bool CHP20b_cDlg::ReadRegistry()
{
  int   left, top;
  HKEY    hKey;
  _TCHAR    data[MAX_PATH];
  DWORD   dwBufLen = 8; // 8 Bytes

  // read registy and reterive values
  if (RegOpenKeyEx(HKEY_CURRENT_USER,
       "SOFTWARE\\Hewlett-Packard\\" REGKEY, 0,
       KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
    if (RegQueryValueEx(hKey, "left", NULL, NULL, (LPBYTE)
            data, &dwBufLen) == ERROR_SUCCESS)
      left = atoi(data);
    else
      left = 0;

    dwBufLen = 8;
    if (RegQueryValueEx(hKey, "top", NULL, NULL, (LPBYTE)
            data, &dwBufLen) == ERROR_SUCCESS)
      top = atoi(data);
    else
      top = 0;

    dwBufLen = 8;
    if (RegQueryValueEx(hKey, "Titlebar", NULL, NULL, (
            LPBYTE) data, &dwBufLen) ==
        ERROR_SUCCESS)
      m_bHideTitlebar = atoi(data);
    else
      m_bHideTitlebar = 0;

    dwBufLen = MAX_PATH;
    data[0] = 0;
    RegQueryValueEx(hKey, "Skin", NULL, NULL, (BYTE *) data, &
        dwBufLen);
    LoadSkin(data);
    RegCloseKey(hKey);
    hKey = NULL;
    RECT    rect;

    GetClientRect(&rect);
    if (left < - (rect.right - rect.left))
      left = 0;

    if (top < - (rect.bottom - rect.top))
      top = 0;

    SetWindowPos(NULL, left, top, NULL, NULL, SWP_NOSIZE |
           SWP_NOZORDER | SWP_SHOWWINDOW);
    m_bHideTitlebar = !m_bHideTitlebar;
    ShowHP20bTitlebar();
    //check/uncheck main menu
    CMenu   *pMenu = GetMenu();

    if (m_bHideTitlebar)
      pMenu->CheckMenuItem(ID_HP20b_SHOWCAPTION,
               MF_BYCOMMAND | MF_CHECKED);
    else
      pMenu->CheckMenuItem(ID_HP20b_SHOWCAPTION,
               MF_BYCOMMAND | MF_UNCHECKED);

    return true;
  }
  return false;
}
/***************************************************************
**
**  Called when the user chooses 'Hide Titlebar' sub-menu from the main menu
**
***************************************************************/
//
void CHP20b_cDlg::OnHP20bShowcaptionMenu()
{
  ShowHP20bTitlebar();
  // check/uncheck main menu
  CMenu   *pMenu = GetMenu();

  if (m_bHideTitlebar)
    pMenu->CheckMenuItem(ID_HP20b_SHOWCAPTION, MF_BYCOMMAND | MF_CHECKED);
  else
    pMenu->CheckMenuItem(ID_HP20b_SHOWCAPTION, MF_BYCOMMAND | MF_UNCHECKED);

  AfxMessageBox(
#ifdef wp31s
          "Right click on the wp logo and uncheck 'Hide Titlebar'\r\nto display the titlebar again.\r\n"
          "To close the calculator, turn it OFF with (gold) f + EXIT.",
#else
          "Right click on the HP logo and uncheck 'Hide Titlebar'\r\nto display the titlebar again.\r\n"
          "To close the calculator, turn it OFF with (blue) shift + ON/CE.",
#endif
          MB_OK | MB_ICONINFORMATION);
}
/***************************************************************
**
**  The framework calls this member function after the CWnd object
**      has been moved.
**
***************************************************************/
//
void CHP20b_cDlg::OnMove(int x, int y)
{
  // Changed by MvC: broke minimize/restore from taskbar
  static int  oldYPos = 1;
#if 0
  static FILE *trace;
  if ( trace == NULL ) trace = fopen("wp31strace.dat","w");
  fprintf(trace,"(%d,%d)\n",x,y);
#endif
  CDialog::OnMove(x, y);
  if (m_bHideTitlebar && y > -10000) {
    if (oldYPos < 0 && (y == -1 || y == -5)) {
      int captY = ::GetSystemMetrics(SM_CYCAPTION);
      int newYPos = oldYPos - captY + 2;
      if (newYPos < - (Skin.screen.top + captY))
        newYPos = - (Skin.screen.top + captY);
      SetWindowPos(NULL, x - 3, newYPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
      y = newYPos;
    }
    oldYPos = y;
  }
}
void CHP20b_cDlg::OnCalculatorAssignasdefaulthpcalculator()
{
}
void CHP20b_cDlg::OnCalculatorManagehpcalculatoremulators()
{
}
void CHP20b_cDlg::OnHelpHp20bemulatorhelp()
{
  HINSTANCE h = ShellExecute(NULL, "open", "20bHelpFile.chm", NULL, NULL, SW_SHOWNORMAL);
  // Returns a value greater than 32 if successful, or an error value
  // that is less than or equal to 32 otherwise
  if ((int) h <= 32)
    AfxMessageBox( "The help file 20bHelpFile.chm was not found",
            MB_OK | MB_ICONINFORMATION);

// TODO: Add your command handler code here
}
void CHP20b_cDlg::LoadSkin(char *skin)
{
  Skin.SkinLoad(skin);
  if (Skin.error != 0) {
    char    b[300];

    sprintf(b, "Error while loading skin (%d)", Skin.error);
    MessageBox(b, "Error");
    return;
  }
  m_Background.SetBitmap(Skin.bitmap);
  m_VirtualLCD.hpInitBitmap();
  ::GetWindowRect(m_VirtualLCD.m_hWnd, &m_VirtualLCD.m_rcOrgLCDPos);
  ScreenToClient(&m_VirtualLCD.m_rcOrgLCDPos);
  RECT r1, r2;

  GetWindowRect(&r1);
  r2 = r1;
  r2.bottom = r2.top + Skin.size.y;
  r2.right = r2.left + Skin.size.x;
  AdjustWindowRectEx(&r2, GetWindowLong(m_hWnd, GWL_STYLE), true,
                     GetWindowLong(m_hWnd, GWL_EXSTYLE));
  r1.bottom = r1.top + r2.bottom - r2.top;
  r1.right = r1.left + r2.right - r2.left;
 ::MoveWindow(m_Background.m_hWnd, 0, 0, Skin.size.x, Skin.size.y, true);
 ::MoveWindow(m_VirtualLCD.m_hWnd, Skin.screen.left, Skin.screen.top,
                Skin.screen.right, Skin.screen.bottom, true);
  MoveWindow(&r1, true);
  m_bHideTitlebar = !m_bHideTitlebar;
  ShowHP20bTitlebar();
  UpdateSkinMenu();
  UpdateDlgScreen(false);
}
bool SkinList(_TCHAR *fullfilename, _TCHAR *filename, void *p)
{
  CMenu *pSub = (CMenu *) p;
  MENUITEMINFO  mi;

  mi.cbSize = sizeof(mi);
  mi.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_TYPE;
  mi.fType = MFT_STRING;
  _TCHAR b[MAX_PATH];

  if (Skin.filename != NULL)
    sprintf(b, "%s.skin", Skin.filename);
  else
    b[0] = 0;

  if (filename != NULL && b != NULL && strcmp(filename, b) == 0)
    mi.fState = MFS_ENABLED | MFS_CHECKED | MFS_UNHILITE;
  else
    mi.fState = MFS_ENABLED | MFS_UNCHECKED | MFS_UNHILITE;

  mi.wID = SkinListCounter--;
  mi.hSubMenu = NULL;
  mi.hbmpChecked = NULL;
  mi.hbmpUnchecked = NULL;
  _TCHAR *pt = (_TCHAR *) malloc(strlen(filename) + 1);

  _tcscpy(pt, filename);
  if (strlen(pt) > 5)
    pt[strlen(pt) - 5] = 0;

  mi.dwTypeData = pt;
  mi.cch = strlen(pt);
  pSub->InsertMenuItemA(pSub->GetMenuItemCount(), &mi, true);
  free(pt);
  return false;
}
void CHP20b_cDlg::UpdateSkinMenu()
{
  CMenu *pMenu = GetMenu();

  SkinListCounter = 65535;
  for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
    CString str;

    pMenu->GetMenuStringA(i, str, MF_BYPOSITION);
    if (str.CompareNoCase("Skins") == 0) {
      CMenu *pSub = pMenu->GetSubMenu(i);

      while (pSub->GetMenuItemCount())
        pSub->RemoveMenu(0, MF_BYPOSITION);

      Skin.SkinList(SkinList, pSub);
      break;
    }
  }
  DrawMenuBar();
}
bool CHP20b_cDlg::SkinCommand(MSG *pMsg)
{
  // Remove "Special Keys"  menu
  CMenu *pMenu = GetMenu();

  for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
    CString str;

    pMenu->GetMenuStringA(i, str, MF_BYPOSITION);
    if (str.CompareNoCase("Skins") == 0) {
      CMenu *pSub = pMenu->GetSubMenu(i);

      for (int i = 0; i < pSub->GetMenuItemCount(); i++)
      {
        MENUITEMINFO  mi;
        _TCHAR bb[MAX_PATH];

        mi.cbSize = sizeof(mi);
        mi.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_TYPE;
        mi.dwTypeData = bb;
        mi.cch = MAX_PATH;
        pSub->GetMenuItemInfoA(i, &mi, true);
        if (mi.wID == pMsg->wParam) {
          if ((mi.fState & MFS_CHECKED) ==
              0)
            LoadSkin(mi.dwTypeData);

          return true;
        }
      }
      return false;
    }
  }
  return false;
}

// Added by MvC
// Makes keyboard buffer accessible from outside DLL

void AddKey(int k, bool ifnotfull)
{
  if (!ifnotfull || KeyBuffRoom() >= 2) {
    AddKeyInBuffer(k & 0xff);
    SetEvent(KeyEvent);
  }
}

// Allow exit from application

void ExitEmulator(void)
{
  SendMessage(dlg->m_hWnd, WM_CLOSE, 0, 0);
}




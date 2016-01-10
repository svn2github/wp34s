
#include "skin.h"
#include <string.h>
#pragma warning(disable : 4996)

CSkin Skin;
CSkin::CSkin()
{
  factor=1;
  size.x=0; size.y= 0;
  screen.bottom=0; screen.left=0; screen.right=0; screen.top=0;
  HighResScreen.x=0; HighResScreen.y=0; // size of the high res screen
  regionsize=0;
  screenfore=0; screenback=0;
  mright=-1;
  mmiddle=-1;
  NbKeys=0;
  error=0; // error code, 0 if no loading error...
  filename= NULL;
  Keys= NULL;
  region= NULL;
  bitmap=0;
  b2=0;
  highres= NULL;
  NbHighResPoly= 0;
  lowres= NULL;
  NbLowResPoly= 0;
  dc= CreateCompatibleDC(NULL);
}

CSkin::~CSkin()
{
  DeleteDC(dc);
  SkinDelete();
}

void CSkin::SkinList(TSkinListCallBack *SkinListCallBack, void *p)
{
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  _TCHAR path[MAX_PATH], b2[MAX_PATH], buf[MAX_PATH];
  GetModuleFileName(NULL, path, MAX_PATH);
  _TCHAR *b= NULL; _TCHAR *B= path;
  while (*B) if (*B++=='\\') b= B; *b= 0;
  sprintf(buf, "%s*.skin", path);

  hFind = FindFirstFile(buf, &FindFileData);
  if (hFind == INVALID_HANDLE_VALUE) return;
  while (true)
  {
    sprintf(b2, "%s%s", path, FindFileData.cFileName);
    if (SkinListCallBack(b2, FindFileData.cFileName, p)) break;
    if (!FindNextFile(hFind, &FindFileData)) break;
  }
  FindClose(hFind);
}

static void skipwhite(char **b, DWORD *s)
{
  while (*s && (**b<10 || **b==11 || **b==12 || (**b>13 && **b<=' '))) (*b)++, (*s)--;
}

static bool cmp(char **b, DWORD *size, char const *val)
{
  DWORD s= *size;
  char *B= *b;
  while (*val && s && *val==*B) s--, B++, val++;
  if (*val==0) { *size= s; *b= B; return true; }
  return false;
}

bool CSkin::GetNumber(char **b, DWORD *s, bool hex, LONG *numb, bool *NEG)
{
  LONG factor = this->factor;
  *numb= 0;
  skipwhite(b, s);
  if (!*s) return false;
  bool neg= false;
  if (*s && **b=='-') neg= true, (*s)--, (*b)++;
  if (NEG!=NULL) *NEG=neg;
  if (!*s) return false;
  if (!hex) { if (!(**b>='0' && **b<='9')) return false; }
  else if (!((**b>='0' && **b<='9') || (**b>='A' && **b<='F') || (**b>='a' && **b<='f'))) return false;

  if (!hex)
    while (*s && (**b>='0' && **b<='9')) *numb= ((*numb)*10)+*(*b)++-'0', (*s)--;
  else
    while (*s && ((**b>='0' && **b<='9') || (**b>='A' && **b<='F') || (**b>='a' && **b<='f'))) 
      if (**b>='0' && **b<='9') *numb= ((*numb)*16)+*(*b)++-'0', (*s)--;
      else if (**b>='A' && **b<='F') *numb= ((*numb)*16)+*(*b)++-'A'+10, (*s)--;
      else if (**b>='a' && **b<='f') *numb= ((*numb)*16)+*(*b)++-'a'+10, (*s)--;
  if (neg) *numb= -*numb;
  if (factor > 1) *numb *= factor;
  skipwhite(b, s);
  if (*s && **b==',') (*b)++, (*s)--;
  skipwhite(b, s);
  return true;
}
// This is reading a line in the form
// n,{x1,y1,x2,y2,...xn,yn}[,{x1,y1,x2,y2,...xn,yn}]+ and updating a list of polygon structure
// placing the given polygon at location n in the TPolys table and updating nbpoly.
// the code is blody inefficient, but I could not be bothered at this point, sorry...
bool CSkin::GetPoly(char **f, DWORD *s, int *nbpoly, TPolys **p)
{
  LONG id;
  void *m;
  bool neg;
  if (!GetNumber(f, s, false, &id, &neg)) return false;
  if (neg) id= -id;
  if (factor>1) id /= factor;
  if (id>=*nbpoly)
  {
    m= realloc(*p, (id+1)*sizeof(TPolys));
    if (m==NULL) return false;
    *p= (TPolys*)m;
    while (*nbpoly<=id) { (*p)[*nbpoly].NbPoly=0; (*p)[(*nbpoly)++].poly= NULL; }
  }
  if (neg)
  {
    TSourceGraphic *Ps= (TSourceGraphic*)&(*p)[id];
    Ps->NbPoly=-1;
    if ((Ps->p=(LONG*)malloc(sizeof(LONG)*6))==NULL) return false;
    LONG *P= Ps->p;
    if (!GetNumber(f, s, false, Ps->p)) return false;
    if (!GetNumber(f, s, false, Ps->p+1)) return false;
    if (!GetNumber(f, s, false, Ps->p+2)) return false;
    if (!GetNumber(f, s, false, Ps->p+3)) return false;
    if (!GetNumber(f, s, false, Ps->p+4)) return false;
    if (!GetNumber(f, s, false, Ps->p+5)) return false;
    return true;
  }
  TPolys *Ps= &(*p)[id];
  while (*s && **f=='{')
  {
    (*f)++, (*s)--;
    m= realloc(Ps->poly, (Ps->NbPoly+1)*sizeof(TPoly));
    if (m==NULL) return false;
    Ps->poly= (TPoly*)m;
    TPoly *P= &Ps->poly[Ps->NbPoly++];
    P->NbPoints=0; P->points= NULL;
    while (true)
    {
      m= realloc(P->points, (P->NbPoints+1)*sizeof(TPoly));
      if (m==NULL) return false;
      P->points= (POINT*)m;
      if (!GetNumber(f, s, false, &P->points[P->NbPoints].x)) return false;
      if (!GetNumber(f, s, false, &P->points[P->NbPoints++].y)) return false;
      if (*s && **f=='}') { (*s)--, (*f)++; break; }
      if (!*s || **f<=' ') return false;
    }
    skipwhite(f, s);
  }
  return true;
}

static bool getFirstSkin(_TCHAR *fullfilename, _TCHAR *filename, void *p)
{
  _TCHAR *FILENAME= (_TCHAR*)p;
  _tcscpy(FILENAME, filename);
  if (strlen(FILENAME)>5) FILENAME[strlen(FILENAME)-5]=0;
  return true;
}
#define swap(a, b) { int t= b; b=a; a=t; }
bool CSkin::SkinLoad(_TCHAR *filename)
{
#define Error(a) { error= a; goto error; }
  _TCHAR path[MAX_PATH], buf[MAX_PATH];
  GetModuleFileName(NULL, path, MAX_PATH);
  _TCHAR *b= NULL; _TCHAR *B= path;
  while (*B) if (*B++=='\\') b= B; *b= 0;
  sprintf(buf, "%s%s.skin", path, filename);

  HANDLE h= CreateFile(buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (h==INVALID_HANDLE_VALUE) 
  {
    _TCHAR FILENAME[MAX_PATH]="";
    SkinList(getFirstSkin, FILENAME);
    sprintf(buf, "%s%s.skin", path, FILENAME);
    h= CreateFile(buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (h==INVALID_HANDLE_VALUE) { Skin.error=33; return false; }
    filename= FILENAME;
  }

  DWORD s= SetFilePointer(h, 0, NULL, FILE_END);
  SetFilePointer(h, 0, NULL, FILE_BEGIN);
  char *file= (char*)malloc(s);
  if (file==NULL) { CloseHandle(h); return false; }
  DWORD read;
  if (!ReadFile(h, file, s, &read, NULL)) { CloseHandle(h); return false; }
  CloseHandle(h);
  // file now loaded in the file buffer of lenght s...
  // start scanning!
  char *f= file;
  void *m;
  int keysize= 0;
  SkinDelete();
  if (!(this->filename= (_TCHAR*)malloc((strlen(filename)+1)*sizeof(_TCHAR)))) Error(1)
  _tcscpy(this->filename, filename);
  while (s)
  {
    if (*f<=' ') { f++; s--; continue; }
    if (*f=='#') { f++; s--; while (s-- && *f++!=10); continue; }
    if (cmp(&f, &s, "factor="))
    {
      factor = 1;
      if (!GetNumber(&f, &s, false, &factor)) Error(2)
      continue;
    }
    if (cmp(&f, &s, "picture="))
    {
      char b[MAX_PATH]; _tcscpy(b, path); char *bb= &b[strlen(b)];
      while (s && *f>=' ') *bb++= *f++, s--;
      *bb=0;
      bitmap= (HBITMAP)LoadImage(NULL, b, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
      if (bitmap==NULL) Error(27);
      b2= (HBITMAP)LoadImage(NULL, b, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
      if (b2==NULL) Error(27);
      SelectObject(dc, b2);
      continue;
    }
    if (cmp(&f, &s, "size="))
    {
      if (!GetNumber(&f, &s, false, &size.x)) Error(2)
      if (!GetNumber(&f, &s, false, &size.y)) Error(3)
      continue;
    }
    if (cmp(&f, &s, "SCREEN="))
    {
      if (!GetNumber(&f, &s, false, &HighResScreen.x)) Error(28)
      if (!GetNumber(&f, &s, false, &HighResScreen.y)) Error(29)
      continue;
    }
    if (cmp(&f, &s, "border="))
    {
      int bsize=0;
      if (regionsize!=0) { bsize= regionsize; regionsize= 0; }
      while (true)
      {
        skipwhite(&f, &s);
        if (!s || *f<=' ') break;
        if (bsize<=regionsize)
        {
          bsize+= 100;
          m= realloc(region, bsize*sizeof(POINT));
          if (m==NULL) Error(24);
          region= (POINT*)m;
        }
        if (!GetNumber(&f, &s, false, &region[regionsize].x)) Error(25)
        if (!GetNumber(&f, &s, false, &region[regionsize].y)) Error(26)
        regionsize++;
      }
      region= (POINT*)realloc(region, regionsize*sizeof(POINT));
      continue;
    }
    if (cmp(&f, &s, "screen="))
    {
      if (!GetNumber(&f, &s, false, &screen.left)) Error(6)
      if (!GetNumber(&f, &s, false, &screen.top)) Error(7)
      if (!GetNumber(&f, &s, false, &screen.right)) Error(8)
      if (!GetNumber(&f, &s, false, &screen.bottom)) Error(9)
      continue;
    }
    if (cmp(&f, &s, "screenfore="))
    {
      if (!GetNumber(&f, &s, true, &screenfore)) Error(10)
      if (factor>1) screenfore /= factor;
      continue;
    }
    if (cmp(&f, &s, "screenback="))
    {
      if (!GetNumber(&f, &s, true, &screenback)) Error(11)
      if (factor>1) screenback /= factor;
      continue;
    }
    if (cmp(&f, &s, "key="))
    {
      if (keysize<=NbKeys)
      {
        keysize+= 100;
        m= realloc(Keys, keysize*sizeof(TKey));
        if (m==NULL) Error(14);
        Keys= (TKey*)m;
      }
      for (int i=0; i<NbKeyCodeInKey; i++) Keys[NbKeys].keycodes[i]= -1;
      if (!GetNumber(&f, &s, false, &Keys[NbKeys].code)) Error(16)
      if (factor>1) Keys[NbKeys].code /= factor;
      if (!GetNumber(&f, &s, false, &Keys[NbKeys].r.left)) Error(17)
      if (!GetNumber(&f, &s, false, &Keys[NbKeys].r.top)) Error(18)
      if (!GetNumber(&f, &s, false, &Keys[NbKeys].r.right)) Error(19)
      if (!GetNumber(&f, &s, false, &Keys[NbKeys].r.bottom)) Error(20)
      if (s && *f=='{')
      {
        int kc= 0;
        f++, s--;
        while (true)
        {
          if (s && *f=='}') { s--, f++; break; }
          if (!GetNumber(&f, &s, false, &Keys[NbKeys].keycodes[kc++])) Error(21)
          if (s && *f=='}') { s--, f++; break; }
          if (kc==NbKeyCodeInKey || !s || *f<=' ') Error(22);
        }
        skipwhite(&f, &s);
      }
	  if (Keys[NbKeys].r.left>Keys[NbKeys].r.right) swap(Keys[NbKeys].r.left, Keys[NbKeys].r.right);
	  if (Keys[NbKeys].r.top>Keys[NbKeys].r.bottom) swap(Keys[NbKeys].r.top, Keys[NbKeys].r.bottom);
      Keys[NbKeys].hasSource= false;
      NbKeys++; if (s && *f==',') f++, s--; else continue;
      if (!GetNumber(&f, &s, false, &Keys[NbKeys-1].s.x)) Error(22)
      if (!GetNumber(&f, &s, false, &Keys[NbKeys-1].s.y)) Error(23)
      Keys[NbKeys].hasSource= true;
      continue;
    }
    if (cmp(&f, &s, "mright="))
    {
      if (!GetNumber(&f, &s, false, &mright)) Error(12)
      continue;
    }
    if (cmp(&f, &s, "mmiddle="))
    {
      if (!GetNumber(&f, &s, false, &mmiddle)) Error(13)
      continue;
    }
    if (cmp(&f, &s, "poly="))
    {
      if (!GetPoly(&f, &s, &NbLowResPoly, &lowres)) Error(30)
      continue;
    }
    if (cmp(&f, &s, "POLY="))
    {
      if (!GetPoly(&f, &s, &NbHighResPoly, &highres)) Error(31)
      continue;
    }
    Error(32);
  }
  m= realloc(Keys, NbKeys*sizeof(TKey));
  if (m==NULL) Error(15);
  Keys= (TKey*)m;
  realloc(file, 0); // free memory used by file...
  return true;
error:
  int error= this->error;
  SkinDelete(); // free skin
  this->error= error;
  realloc(file, 0); // free memory used by file...
  return false;
#undef Error
}

void CSkin::SkinDelete()
{
  size.x=0; size.y= 0;
  screen.bottom=0; screen.left=0; screen.right=0; screen.top=0;
  HighResScreen.x=0; HighResScreen.y=0; // size of the high res screen
  regionsize=0;
  screenfore=0; screenback=0;
  NbKeys=0;
  error=0; // error code, 0 if no loading error...

  realloc(filename, 0); filename=NULL;
  realloc(Keys, 0); Keys=NULL;
  realloc(region, 0); region=NULL;
  DeleteObject(bitmap); bitmap=0;
  DeleteObject(b2); b2=0;
  for (int i=0; i<NbHighResPoly; i++)
  {
    for (int j=0; j<highres[i].NbPoly; j++) realloc(highres[i].poly[j].points, 0);
    realloc(highres[i].poly, 0);
  }
  realloc(highres, 0); highres=NULL;
  NbHighResPoly= 0;
  for (int i=0; i<NbLowResPoly; i++)
  {
    for (int j=0; j<lowres[i].NbPoly; j++) realloc(lowres[i].poly[j].points, 0);
    realloc(lowres[i].poly, 0);
  }
  realloc(lowres, 0); lowres=NULL;
  NbLowResPoly= 0;
  mmiddle= -1; mmiddle= -1;
}

HRGN CSkin::hpGetKeyRegion(CPoint *pPtMouse, LONG *pKeyPadNum)
{
  for (int n = 0; n < NbKeys; n++)
    if (pPtMouse->x>=Keys[n].r.left && pPtMouse->x<=Keys[n].r.right && pPtMouse->y>=Keys[n].r.top && pPtMouse->y<=Keys[n].r.bottom)
    {
      if (pKeyPadNum!=NULL) *pKeyPadNum= Keys[n].code;
		  return CreateRoundRectRgn(Keys[n].r.left, Keys[n].r.top, Keys[n].r.right, Keys[n].r.bottom, 6,12);	
    }
	*pKeyPadNum = 0;
  return NULL;
}

HRGN CSkin::hpGetKeyRegion(LONG nKeyPadNum, bool ShiftDown, LONG *code)
{
  if (ShiftDown) nKeyPadNum+= 1000;
  for (int n = 0; n < NbKeys; n++)
    for (int i=0; i<NbKeyCodeInKey; i++)
      if (Keys[n].keycodes[i]==nKeyPadNum)
      {
        if (code!=NULL) *code= Keys[n].code;
		    return CreateRoundRectRgn(Keys[n].r.left, Keys[n].r.top, Keys[n].r.right, Keys[n].r.bottom, 6,12);	
      }
	return NULL;
}

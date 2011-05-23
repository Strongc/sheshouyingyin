#include "StdAfx.h"
#include "TextEdit.h"

void TextEdit::SetTextVCenter()
{
  CRect rc = CRect(0,0,0,0);
  GetClientRect(&rc);

  HDC dc = GetDC();
  CDC pDC;
  pDC.Attach(dc);
  TEXTMETRIC tm;
  pDC.GetTextMetrics(&tm);
  pDC.Detach();
  int nFontHeight = tm.tmHeight + tm.tmExternalLeading;
  int nMargin = (rc.Height() - nFontHeight) / 2;

  rc.DeflateRect(0,nMargin);
  SetRectNP(&rc);
  ReleaseDC(dc);
}
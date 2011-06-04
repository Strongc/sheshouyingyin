#include "StdAfx.h"
#include "TextEdit.h"

TextEdit::TextEdit()
: m_crBKColor(RGB(255, 255, 255))  // white
, m_crTextColor(RGB(0, 0, 0))      // black
, m_brBK(0)
{
  LOGBRUSH lb = {0};
  lb.lbStyle = BS_SOLID;
  lb.lbColor = m_crBKColor;
  m_brBK = ::CreateBrushIndirect(&lb);
}

TextEdit::~TextEdit()
{
  if (m_brBK)
  {
    ::DeleteObject(m_brBK);
    m_brBK = 0;
  }
}

void TextEdit::SetTextColor(int red, int green, int blue)
{
  m_crTextColor = RGB(red, green, blue);
  InvalidateRect(0);
}

COLORREF TextEdit::GetTextColor()
{
  return m_crTextColor;
}

void TextEdit::SetBKColor(int red, int green, int blue)
{
  m_crBKColor = RGB(red, green, blue);
  if (m_brBK)
  {
    ::DeleteObject(m_brBK);
    m_brBK = 0;
  }

  LOGBRUSH lb = {0};
  lb.lbStyle = BS_SOLID;
  lb.lbColor = m_crBKColor;
  m_brBK = ::CreateBrushIndirect(&lb);

  InvalidateRect(0);
}

COLORREF TextEdit::GetBKColor()
{
  return m_crBKColor;
}

void TextEdit::SetTextVCenter()
{
  CRect rc = CRect(0,0,0,0);
  GetClientRect(&rc);

  HDC dc = GetDC()->m_hDC;
  CDC pDC;
  pDC.Attach(dc);
  TEXTMETRIC tm;
  pDC.GetTextMetrics(&tm);
  pDC.Detach();
  int nFontHeight = tm.tmHeight + tm.tmExternalLeading;
  int nMargin = (rc.Height() - nFontHeight) / 2;

  rc.DeflateRect(0,nMargin);
  SetRectNP(&rc);
  ::ReleaseDC(m_hWnd, dc);
}

BEGIN_MESSAGE_MAP(TextEdit, CEdit)
  ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

HBRUSH TextEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
  pDC->SetTextColor(m_crTextColor);
  pDC->SetBkColor(m_crBKColor);

  return m_brBK;
}